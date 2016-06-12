//  RGBImageIFile.cc                         11 Jul 94
//  Author:  Greg Martin and Loren Bonebrake  SES, MCMC, USGS

//  Implements member functions for the class RGBImageIFile 

#include <string.h>
#include <iostream.h>
#include "Image/RGBImageIFile.h"
#include "Image/RGBPixel.h"
#include "Image/RGBPallette.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdio.h>



RGBImageIFile::RGBImageIFile(const char* fn, const char* hdr): 
   ImageIFile(fn)
{
  setPhotometric(DRG_RGB);
  headerFile = new char[strlen(hdr)+1];
  strcpy(headerFile,hdr);
  Stream = new ifstream(fn,ios::in|ios::nocreate);

  if (!Stream->good())
  {
     gDebug.msg("RGBImageIFile::RGBImageIFile(c*,c*): can't open file",1);
     setNoDataBit();
  }
  else
     readHeader();
  setRandomAccessFlags(rpixel|rrow|rrect);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(3);
  gDebug.msg("RGBImageIFile::RGBImageIFile(c*,c*): exiting ctor",5);
}

RGBImageIFile::RGBImageIFile(const char* fn): 
   ImageIFile(fn)
{
  setPhotometric(DRG_RGB);
  headerFile = new char[strlen(fn)+5];
  strcpy(headerFile,fn);
  strcat(headerFile,".hdr");
  Stream = new ifstream(fn,ios::in|ios::nocreate);

  if (!Stream->good())
  {
     gDebug.msg("RGBImageIFile::RGBImageIFile(c*): can't open file",1);
     setNoDataBit();
  }
  else
     readHeader();
  setRandomAccessFlags(rpixel|rrow|rrect);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(3);
  gDebug.msg("RGBImageIFile::RGBImageIFile(c*): exiting ctor",5);
}


RGBImageIFile::~RGBImageIFile()
{
   if (Stream) delete Stream;
   if (headerFile) delete headerFile;
   gDebug.msg("RGBImageIFile::~RGBImageIFile(): exiting dtor",5);
}



Pallette* RGBImageIFile::getPallette()
{
  long wdth, hgth, x, i, j, k, l, found;
  RGBPixel* pixelbuff=NULL;
  RGBPixel* temp = NULL;
  RGBPallette* pal;
  unsigned char* line;
  unsigned char red, green, blue;

  wdth = getWidth();
  hgth = getHeight();
  x = 0;

  Stream->seekg(0);

  for(i=0; i<hgth; i++)
  {
    line = (unsigned char*)getRawScanline(i);
    for(j=0; j<(3*wdth); j+=3)
    {
      found = 0;
      for(k=0; k<x; k++)
      {
        red   = pixelbuff[k].getRed();
        green = pixelbuff[k].getGreen();
        blue  = pixelbuff[k].getBlue();
        if((line[j] == red) && 
           (line[j+1] == green) && 
           (line[j+2] == blue)) found = 1;
      }

      if(!found)
      {
        temp = new RGBPixel[x+1];
        for(l=0; l<x; l++)
          temp[l] = pixelbuff[l];
        temp[x].setRed(line[j]);
        temp[x].setGreen(line[j+1]);
        temp[x].setBlue(line[j+2]);
        x++;
        if (pixelbuff) delete[] pixelbuff;
        pixelbuff = temp;
        temp = NULL;
      } // if(found)
    } // for(j)
    delete line;
  } // for(i)
  pal = new RGBPallette(x, pixelbuff);

  if (pixelbuff) delete[] pixelbuff;

  return((Pallette*)pal);
}



AbstractPixel* RGBImageIFile::getPixel(long x, long y)
{
  if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  RGBPixel *pixel;
  unsigned char* buf;

  buf = (unsigned char *) getRawPixel(x,y);

  pixel = new RGBPixel(buf[0],buf[1],buf[2]);

  delete[] buf;
  return (AbstractPixel*)pixel;
}



AbstractPixel* RGBImageIFile::getScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIFile::getScanline(): scanline %ld out of bounds",row);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* line;
  RGBPixel* pixelbuf;
  int w;
  long i;

  w = getWidth();

  pixelbuf = new RGBPixel[w];

  line = (unsigned char *) getRawScanline(row);

  for (i=0;i<w;i++)
  {
      pixelbuf[i].setRed(line[3*i]);
      pixelbuf[i].setGreen(line[3*i+1]);
      pixelbuf[i].setBlue(line[3*i+2]);
  }

  delete[] line;

  return((AbstractPixel*)pixelbuf);
}



AbstractPixel* RGBImageIFile::getRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[100];
     sprintf(dmsg,"RGBImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  RGBPixel *pixelbuf;
  unsigned char *rect;
  int w, h, n, i;

  w = x2-x1+1;
  h = y2-y1+1;
  n = w*h;

  pixelbuf = new RGBPixel[n];
  rect = (unsigned char *) getRawRectangle(x1,y1,x2,y2);

  for (i=0;i<n;i++)
  {
      pixelbuf[i].setRed(rect[3*i]);
      pixelbuf[i].setGreen(rect[3*i+1]);
      pixelbuf[i].setBlue(rect[3*i+2]);
  }
  
  delete[] rect;

  return((AbstractPixel*)pixelbuf);
}



void*  RGBImageIFile::getRawPixel(long x, long y)
{
  if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  long wdth, seekval;
  unsigned char *pixel;

  pixel = new unsigned char[3];
  wdth = getWidth();

  seekval = 3*((y*wdth)+x);
  Stream->seekg(seekval);
  Stream->read(pixel,3*sizeof(unsigned char));
  if (!Stream->good())
  {
     gDebug.msg("RGBImageIFile::getRawPixel(): problem reading file",1);
     setFailRBit();
  }

  return pixel;
}



void*  RGBImageIFile::getRawScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIFile::getRawScanline(): scanline %ld out of bounds",row);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* pixelbuf;
  long wdth;

  wdth = 3*getWidth();

  pixelbuf = new unsigned char[wdth];

  Stream->seekg(row*wdth*sizeof(unsigned char));
  Stream->read(pixelbuf,wdth*sizeof(unsigned char));
  if (!Stream->good())
  {
     gDebug.msg("RGBImageIFile::getRawScanline(): problem reading file",1);
     setFailRBit();
  }

  return(pixelbuf);
}



void*  RGBImageIFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[120];
     sprintf(dmsg,"RGBImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* buffer;
  unsigned char* ptr;
  long length, wdth, y, k, seekval;

  length = 3*(x2-x1+1);
  wdth = getWidth();
  k = 0;
  buffer = new unsigned char[3*(x2-x1+1)*(y2-y1+1)];

  for(y=y1; y<=y2; y++)
  {
    seekval = 3*(y*wdth+x1);
    ptr = &(buffer[(y-y1)*length]);
    Stream->seekg(seekval);
    Stream->read(ptr,sizeof(unsigned char)*length);
    if (!Stream->good())
    {
       gDebug.msg("RGBImageIFile::getRawRectangle(): problem reading file",1);
       setFailRBit();
    }
  }
  return buffer;
}


void RGBImageIFile::getRawRGBPixel(long x,long y,RawRGBPixel* pix)
{
   if ((x<0)||(x>(getWidth()-1))||(y<0)||(y>(getHeight()-1)))
   {
      char dmsg[100];
      sprintf(dmsg,"RGBImageIFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }
   unsigned char* temp;

   temp = (unsigned char*) getRawPixel(x,y);
   pix->Red   = temp[0];
   pix->Green = temp[1];
   pix->Blue  = temp[2];
   delete[] temp;
}

void RGBImageIFile::getRawRGBScanline(long row,RawRGBPixel* pixbuff)
{
   if ((row<0)||(row>(getHeight()-1)))
   {
      char dmsg[100];
      sprintf(dmsg,"RGBImageIFile::getRawRGBScanline(): scanline %ld out of bounds",row);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }
   unsigned char* temp;
   long i,k,w;
   w = 3*getWidth();

   temp = (unsigned char*) getRawScanline(row);
   k = 0;
   for (i=0;i<w;i+=3)
   {
       pixbuff[k].Red   = temp[i];
       pixbuff[k].Green = temp[i+1];
       pixbuff[k].Blue  = temp[i+2];
       k++;
   }
   delete[] temp;
}

void RGBImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2, 
                                       RawRGBPixel* pixbuff)
{
   if ((x1<0)||(x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
      (x2<0)||(x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
      (x1>x2) || (y1>y2))
   {
      char dmsg[120];
      sprintf(dmsg,"RGBImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }

   unsigned char* temp;
   long i,k,w,h,l;
   w = (x2-x1+1);
   h = (y2-y1+1); 
   l = 3*h*w;

   temp = (unsigned char*) getRawRectangle(x1,y1,x2,y2);
   k = 0;
   for (i=0;i<l;i+=3)
   {
       pixbuff[k].Red   = temp[i];
       pixbuff[k].Green = temp[i+1];
       pixbuff[k].Blue  = temp[i+2];
       k++;
   }
   delete[] temp;
}

void RGBImageIFile::readHeader(void)
{
    ifstream* hdrfile;
    long val1, val2;
    char hdr1[10], hdr2[10];

    val1 = val2 = 0;
    hdr1[0] = hdr2[0] = '\0';

    hdrfile = new ifstream(headerFile,ios::in|ios::nocreate);

    if (!hdrfile->good())
    {
       gDebug.msg("RGBImageIFile::readHeader(): can't open header",1);
       setFailRBit();
    }
    else
    {
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
    }
    delete hdrfile;
}


bool RGBImageIFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
