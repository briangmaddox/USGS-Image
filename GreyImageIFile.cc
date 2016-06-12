//  GreyImageIFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class GreyImageIFile 

#include <string.h>
#include <iostream.h>
#include "Image/GreyImageIFile.h"
#include "Image/GreyPixel.h"
#include "Image/GreyPallette.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdio.h>



GreyImageIFile::GreyImageIFile(const char* fn, const char* hdr): 
   ImageIFile(fn)
{

  setPhotometric(DRG_GREY);
  headerFile = new char[strlen(hdr)+1];
  strcpy(headerFile,hdr);
  Stream = new ifstream(fn,ios::in|ios::nocreate);

  if (!Stream->good())
  {
     gDebug.msg("GreyImageIFile::GreyImageIFile(c*,c*): can't open file",1);
     setNoDataBit();
  }
  else
     readHeader();
  setRandomAccessFlags(rpixel|rrow|rrect);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(1);
  gDebug.msg("GreyImageIFile::GreyImageIFile(c*,c*): exiting ctor",5);
}

GreyImageIFile::GreyImageIFile(const char* fn): 
   ImageIFile(fn)
{

  setPhotometric(DRG_GREY);
  headerFile = new char[strlen(fn)+5];
  strcpy(headerFile,fn);
  strcat(headerFile,".hdr");
  Stream = new ifstream(fn,ios::in|ios::nocreate);

  if (!Stream->good())
  {
     gDebug.msg("GreyImageIFile::GreyImageIFile(c*): can't open file",1);
     setNoDataBit();
  }
  else
     readHeader();
  setRandomAccessFlags(rpixel|rrow|rrect);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(1);
  gDebug.msg("GreyImageIFile::GreyImageIFile(c*): exiting ctor",5);
}



GreyImageIFile::~GreyImageIFile()
{
    if (Stream) delete Stream;
    if (headerFile) delete headerFile;
    gDebug.msg("GreyImageIFile::~GreyImageIFile(): exiting dtor",5);
}



Pallette* GreyImageIFile::getPallette()
{
  long x, i, j, k, l, found;
  GreyPixel* pixelbuff = NULL;
  GreyPixel* temp = NULL;
  GreyPallette* pal;
  unsigned char* line;

  x = 0;

  Stream->seekg(0);

  for(i=0; i<getHeight(); i++)
  {
    line = (unsigned char*)getRawScanline(i);
    for(j=0; j<getWidth();j++)
    {
      found = 0;
      for(k=0; k<x; k++)
      {
          if ( (line[j] = pixelbuff[k].getGrey()) )
             found = 1;
      }

      if(!found)
      {
        temp = new GreyPixel[x+1];
        for(l=0; l<x; l++)
          temp[l] = pixelbuff[l];
        temp[x].setGrey(line[j]);
        x++;
        if (pixelbuff) delete[] pixelbuff;
        pixelbuff = temp;
        temp = NULL;
      } // if(found)
    } // for(j)
    delete[] line;
  } // for(i)
  pal = new GreyPallette(x, pixelbuff);

  if (pixelbuff) delete[] pixelbuff;

  return((Pallette*)pal);
}

// THIS IS NOW IN ROW MAJOR ORDER

AbstractPixel* GreyImageIFile::getPixel(long x, long y)
{
   if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
   {
      char dmsg[80];
      sprintf(dmsg,"GreyImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return NULL;
   }

   GreyPixel *pixelbuf;
   unsigned char *pixel;

   pixelbuf = new GreyPixel;

   pixel = (unsigned char *) getRawPixel(x,y);

   pixelbuf->setGrey(*pixel);

   delete pixel;

   return (AbstractPixel*)pixelbuf;
}



AbstractPixel* GreyImageIFile::getScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"GreyImageIFile::getScanline(): scanline %ld out of bounds",row);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* line;
  GreyPixel* pixelbuf;
  long i;

  pixelbuf = new GreyPixel[getWidth()];

  line = (unsigned char *) getRawScanline(row);

  for (i=0;i<getWidth();i++)
  {
      pixelbuf[i].setGrey(line[i]);
  }
  delete[] line;

  return((AbstractPixel*)pixelbuf);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

AbstractPixel* GreyImageIFile::getRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[100];
     sprintf(dmsg,"GreyImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  GreyPixel *pixelbuf;
  unsigned char *buffer;
  int w, h, n;
  int i;

  w = x2 - x1 + 1;
  h = y2 - y1 + 1;
  n = w*h;

  buffer = (unsigned char *) getRawRectangle(x1,y1,x2,y2);
  pixelbuf = new GreyPixel[n];

  for (i=0;i<n;i++)
      pixelbuf[i].setGrey(buffer[i]); 

  delete[] buffer;

  return((AbstractPixel*)pixelbuf);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void*  GreyImageIFile::getRawPixel(long x, long y)
{
   if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
   {
      char dmsg[80];
      sprintf(dmsg,"GreyImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return NULL;
   }

   long seekval;
   unsigned char* pixel = new unsigned char;


   seekval = y*getWidth()+x;
   Stream->seekg(seekval);
   Stream->read(pixel,sizeof(unsigned char));
   if (!Stream->good())
   {
      gDebug.msg("GreyImageIFile::getRawPixel(): problem reading file",1);
      setFailRBit();
   }

   return pixel;
}



void*  GreyImageIFile::getRawScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"GreyImageIFile::getRawScanline(): scanline %ld out of bounds",row);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* pixelbuf;
  long seekval;

  pixelbuf = new unsigned char[getWidth()];
 
  seekval = row*getWidth();
  
  Stream->seekg(seekval);
  Stream->read(pixelbuf,getWidth()*sizeof(unsigned char));
  if (!Stream->good())
  {
     gDebug.msg("GreyImageIFile::getRawScanline(): problem reading file",1);
     setFailRBit();
  }

  return(pixelbuf);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void*  GreyImageIFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[120];
     sprintf(dmsg,"GreyImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* buffer;
  unsigned char* ptr;
  long length, y, k, seekval;

  length = x2-x1+1;
  k = 0;
  buffer = new unsigned char[(x2-x1+1)*(y2-y1+1)];

  for(y=y1; y<=y2; y++)
  {
    seekval = y*getWidth()+x1;
    ptr = &(buffer[(y-y1)*length]);
    Stream->seekg(seekval);
    Stream->read(ptr,sizeof(unsigned char)*length);
    if (!Stream->good())
    {
       gDebug.msg("GreyImageIFile::getRawRectangle(): problem reading file",1);
       setFailRBit();
    }
  }
  return buffer;
}

void GreyImageIFile::getRawRGBPixel(long x,long y,RawRGBPixel* pix)
{
   if ((x<0)||(x>(getWidth()-1))||(y<0)||(y>(getHeight()-1)))
   {
      char dmsg[80];
      sprintf(dmsg,"GreyImageIFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }
   unsigned char* temp;

   temp = (unsigned char*) getRawPixel(x,y);
   if (good())
   {
      pix->Red   = *temp;
      pix->Green = *temp;
      pix->Blue  = *temp;
   }
   if (temp!=NULL) delete temp;
}

void GreyImageIFile::getRawRGBScanline(long row,RawRGBPixel* pixbuf)
{
   if ((row<0)||(row>(getHeight()-1)))
   {
      char dmsg[80];
      sprintf(dmsg,"GreyImageIFile::getRawRGBScanline(): scanline %ld out of bounds",row);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }
   unsigned char* temp;
   long i,w; 

   temp = (unsigned char*) getRawScanline(row);

   if (good())
   {

      w = getWidth();
      for (i=0;i<w;i++)
      {
          pixbuf[i].Red   = temp[i];
          pixbuf[i].Green = temp[i];
          pixbuf[i].Blue  = temp[i];
      }
   }
   if (temp!=NULL) delete[] temp;
}

void GreyImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                        RawRGBPixel* pixbuf)
{

   if ((x1<0)||(x1>(getWidth()-1))||(y1 < 0) || (y1 > (getHeight()-1)) ||
      (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
      (x1>x2) || (y1>y2))
   {
      char dmsg[120];
      sprintf(dmsg,"GreyImageIFile::getRawRGBRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }
   unsigned char* temp;
   long i,h,w,l;

   w = (x2-x1+1);
   h = (y2-y1+1);
   l = w*h;

   temp = (unsigned char*) getRawRectangle(x1,y1,x2,y2);

   if (good())
   {

      w = getWidth();
      for (i=0;i<l;i++)
      {
          pixbuf[i].Red   = temp[i];
          pixbuf[i].Green = temp[i];
          pixbuf[i].Blue  = temp[i];
      }
   }
   if (temp!=NULL) delete[] temp;
}

void GreyImageIFile::readHeader(void)
{
    ifstream* hdrfile;
    long val1, val2;
    char hdr1[10], hdr2[10];

    val1 = val2 = 0;
    hdr1[0] = hdr2[0] = '\0';

    hdrfile = new ifstream(headerFile,ios::in|ios::nocreate);

    if (!hdrfile->good())
    {
       gDebug.msg("GreyImageIFile::readHeader(): can't open file",1);
       setFailRBit();
       return;
    }

    *hdrfile >> hdr1 >> val1;
    *hdrfile >> hdr2 >> val2;

    if (!strcmp(hdr1,"width:")) setWidth(val1);
    if (!strcmp(hdr1,"Width:")) setWidth(val1);
    if (!strcmp(hdr1,"WIDTH:")) setWidth(val1);
    if (!strcmp(hdr2,"height:")) setHeight(val2);
    if (!strcmp(hdr2,"Height:")) setHeight(val2);
    if (!strcmp(hdr2,"HEIGHT:")) setHeight(val2);
    if (!strcmp(hdr2,"width:")) setWidth(val2);
    if (!strcmp(hdr2,"Width:")) setWidth(val2);
    if (!strcmp(hdr2,"WIDTH:")) setWidth(val2);
    if (!strcmp(hdr1,"height:")) setHeight(val1);
    if (!strcmp(hdr1,"Height:")) setHeight(val1);
    if (!strcmp(hdr1,"HEIGHT:")) setHeight(val1);

    delete hdrfile;
}

bool GreyImageIFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
