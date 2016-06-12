//  Image.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class Image 


#include "Image/Image.h"
#include "Image/ImageCommon.h"
#include "Image/AbstractPixel.h"
#include "Image/ImageData.h"
#include "Image/ImageDebug.h"
#include "Image/TIFFImageIFile.h"
#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>

Image::Image()
{
   Data = NULL;
   NoBands = 0;
   BitsPerBand = 0;
   FileType = IMAGE_UNKNOWN;
   clear();
   RandomAccessFlags = none;
   _hasPallette = false;
   gDebug.msg("Image::Image(): exiting ctor",5);
}
    
Image::Image(ImageData* data)
{
   Data = data;
   NoBands = 0;
   BitsPerBand = 0;
   clear();
   RandomAccessFlags = data->getRandomAccessFlags();
   _hasPallette = false;
   gDebug.msg("Image::Image(ImageData*): exiting ctor",5);
}

Image::~Image()
{
   if (Data != NULL)
      delete Data;
   gDebug.msg("Image::~Image(): exiting dtor",5);
}


int Image::getNoBands(void)
{
    return NoBands;
}

int Image::getBitsPerBand(void)
{
    return BitsPerBand;
}

long Image::getWidth(void)
{
    return Data->getWidth();
}

long Image::getHeight(void)
{
    return Data->getHeight();
}


void Image::setNoBands(int n)
{
    NoBands = n;
}

void Image::setBitsPerBand(int n)
{
    BitsPerBand = n;
}

void Image::setWidth(long w)
{
    Data->setWidth(w);
}

void Image::setHeight(long h)
{
    Data->setHeight(h);
}

void Image::setHasPallette(void)
{
    _hasPallette = true;
}

void Image::unsetHasPallette(void)
{
   _hasPallette = false;
}

bool Image::hasPallette(void)
{
   return _hasPallette;
}


ImageData* Image::getImageData(void)
{
    return Data;
}

void Image::setImageData(ImageData* d)
{
    if (Data) delete Data;
    Data = d;
    setRandomAccessFlags(d->getRandomAccessFlags());
    gDebug.msg("Image::setImageData(): Data set",5);
}



unsigned char Image::getFileType(const char* filename,int file_exists)
{
  FILE *fp;
  unsigned char magicno[8];    /* first 8 bytes of file */
  unsigned char file_type;
  char* foo;
  int c;

  if (!filename)
  {
     gDebug.msg("Image::getFileType(): filename is NULL",1);
     return IMAGE_ERROR;   /* shouldn't happen */
  }

  if (file_exists)
  {
     fp = fopen(filename, "r");
   
     if (fp == NULL) return IMAGE_ERROR;
   
     c = fread(magicno,8,1,fp);
     fclose(fp);
   
     if (c!=1) return IMAGE_UNKNOWN;    /* files less than 8 bytes long... */

     char dmsg[80];
     sprintf(dmsg,"Image::getFileType(): magic nos are %d %d %d %d %d %d %d %d",
             (int)magicno[0],
             (int)magicno[1],
             (int)magicno[2],
             (int)magicno[3],
             (int)magicno[4],
             (int)magicno[5],
             (int)magicno[6],
             (int)magicno[7]);
     gDebug.msg(dmsg,1);
  }

  file_type = IMAGE_UNKNOWN;

  for(c=(strlen(filename) - 1);filename[c]!='.';c--);
  foo = (char *) &(filename[c+1]);
  if ((strcmp(foo,"rgb")==0)||
      (strcmp(foo,"RGB")==0))     file_type = IMAGE_RGB;

  if ((strcmp(foo,"grey")==0)||
      (strcmp(foo,"GREY")==0))     file_type = IMAGE_GREY;

  if ((strcmp(foo,"doq")==0)||
      (strcmp(foo,"DOQ")==0))     file_type = IMAGE_DOQ;

  if ((file_type == IMAGE_UNKNOWN)&&(file_exists))
  {
     if (strncmp((char *)magicno,"GIF87a",6)==0 ||
         strncmp((char *)magicno,"GIF89a",6)==0) file_type = IMAGE_GIF;
   
     else if (magicno[0] == 'P' && magicno[1]>='1' &&
              magicno[1]<='6') file_type = IMAGE_PBM;
   
     else if (strncmp((char *)magicno,"#define",7)==0) file_type = IMAGE_XBM;
   
     else if (magicno[0] == 'B' && magicno[1] == 'M') file_type = IMAGE_BMP;
   
     else if (magicno[0]==0x0a && magicno[1] <= 5) file_type = IMAGE_PCX;

     else if (magicno[0]==0xff && magicno[1]==0xd8 &&
              magicno[2]==0xff) file_type = IMAGE_JFIF;
   
     else if ((magicno[0]=='M' && magicno[1]=='M') ||
              (magicno[0]=='I' && magicno[1]=='I')) file_type = IMAGE_TIFF;
     
     else if (strncmp((char *)magicno,"DGP",3)==0) file_type = IMAGE_DGP;
     
     else if (magicno[0]==0x08 && magicno[1]==0x09 &&
              magicno[4]==0x41 && magicno[5]==0x00)   file_type = IMAGE_CRL;
     else if (strncmp((char *)magicno,"PNG",3)==0) file_type = IMAGE_PNG;
     if (file_type==IMAGE_TIFF)
     {
        TIFFImageIFile* fin = new TIFFImageIFile(filename);
        unsigned short scount=0;
        double *t;
        if (fin->good())
           if (TIFFGetField(fin->getTif(),33550,&scount,&t))
              file_type = IMAGE_GTIF;
        delete fin;
     }
  } 
  if ((file_type == IMAGE_UNKNOWN)&&(!file_exists))
  {
     if ((strcmp(foo,"tiff")==0) ||
         (strcmp(foo,"tif") ==0) ||
         (strcmp(foo,"TIFF")==0) ||
         (strcmp(foo,"TIF") ==0))      file_type = IMAGE_TIFF;

     else if ((strcmp(foo,"gif")==0) ||
              (strcmp(foo,"GIF")==0))       file_type = IMAGE_GIF; 

     else if ((strcmp(foo,"pcx")==0) ||
              (strcmp(foo,"PCX")==0))       file_type = IMAGE_PCX; 

     else if ((strcmp(foo,"xbm")==0) ||
              (strcmp(foo,"XBM")==0))       file_type = IMAGE_XBM; 

     else if ((strcmp(foo,"pbm")==0) ||
              (strcmp(foo,"PBM")==0))       file_type = IMAGE_PBM; 

     else if ((strcmp(foo,"bmp")==0) ||
              (strcmp(foo,"BMP")==0))       file_type = IMAGE_BMP; 

     else if ((strcmp(foo,"jpeg")==0) ||
              (strcmp(foo,"JPEG")==0))       file_type = IMAGE_JFIF; 

     else if ((strcmp(foo,"crl")==0) ||
              (strcmp(foo,"CRL")==0))       file_type = IMAGE_CRL; 

     else if ((strcmp(foo,"dgp")==0) ||
              (strcmp(foo,"DGP")==0))       file_type = IMAGE_DGP; 

     else if ((strcmp(foo,"png")==0) ||
              (strcmp(foo,"PNG")==0))       file_type = IMAGE_PNG; 
  }

  FileType = file_type;

  return file_type;
}

unsigned char Image::getFileType(void)
{
   return FileType;
}

bool Image::good(void)
{
    return (State == goodbit);
}

bool Image::bad(void)
{
    return (State != goodbit);
}

bool Image::noData(void)
{
    return ((State&nodatabit)!=0);
}

bool Image::failedRead(void)
{
    return ((State&failrbit)!=0);
}

bool Image::failedWrite(void)
{
    return ((State&failwbit)!=0);
}

void Image::clear(void)
{
    State = goodbit;
}

void Image::setNoDataBit(void)
{
    State = State | nodatabit;
}

void Image::setFailRBit(void)
{
    State = State | failrbit;
}

void Image::setFailWBit(void)
{
    State = State | failwbit;
}

void Image::setRandomAccessFlags(unsigned int f)
{
    RandomAccessFlags = f;
}

bool Image::canReadPixel(void)
{
   return ((RandomAccessFlags & rpixel) != 0);
}

bool Image::canReadScanline(void)
{
   return ((RandomAccessFlags & rrow) != 0);
}

bool Image::canReadRectangle(void)
{
   return ((RandomAccessFlags & rrect) != 0);
}


bool Image::canWritePixel(void)
{
   return ((RandomAccessFlags & wpixel) != 0);
}


bool Image::canWriteScanline(void)
{
   return ((RandomAccessFlags & wrow) != 0);
}

bool Image::canWriteRectangle(void)
{
   return ((RandomAccessFlags & wrect) != 0);
}

void Image::setPhotometric(unsigned char p)
{
   Photometric = p;
}

unsigned char Image::getPhotometric(void)
{
   return Photometric;
}
