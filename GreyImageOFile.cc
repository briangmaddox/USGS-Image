//  GreyImageOFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class GreyImageOFile 

#include "Image/GreyImageOFile.h"
#include "Image/ImageCommon.h"
#include "Image/AbstractPixel.h"
#include "Image/GreyPixel.h"
#include "Image/ImageDebug.h"
#include <stdio.h>
#include <string.h>



GreyImageOFile::GreyImageOFile(const char* fn, const char* header, 
                             long w, long l):
   ImageOFile(fn,w,l)
{
   setPhotometric(DRG_GREY);
   headerFile = new char[strlen(header)+1];
   strcpy(headerFile,header);

   Stream = new ofstream(fn,ios::out|ios::trunc);
   if (!Stream->good())
   {
      gDebug.msg("GreyImageOFile::GreyImageOFile(c*,c*,l,l): can't open file",1);
      setNoDataBit();
   }
   else
      writeHeader();
   setRandomAccessFlags(wpixel|wrow|wrect);
   unsetHasPallette();
   setBitsPerSample(8);
   setSamplesPerPixel(1);
   gDebug.msg("GreyImageOFile::GreyImageOFile(c*,c*,l,l): exiting ctor",5);
}

GreyImageOFile::GreyImageOFile(const char* fn, long w, long l):
   ImageOFile(fn,w,l)
{
   setPhotometric(DRG_GREY);
   headerFile = new char[strlen(fn)+5];
   strcpy(headerFile,fn);
   strcat(headerFile,".hdr");

   Stream = new ofstream(fn,ios::out|ios::trunc);
   if (!Stream->good())
   {
      gDebug.msg("GreyImageOFile::GreyImageOFile(c*,l,l): can't open file",1);
      setNoDataBit();
   }
   else
      writeHeader();
   setRandomAccessFlags(wpixel|wrow|wrect);
   unsetHasPallette();
   setBitsPerSample(8);
   setSamplesPerPixel(1);
   gDebug.msg("GreyImageOFile::GreyImageOFile(c*,l,l): exiting ctor",5);
}

GreyImageOFile::~GreyImageOFile()
{
   if (headerFile) delete headerFile;
   if (Stream) delete Stream;
   gDebug.msg("GreyImageOFile::~GreyImageOFile(): exiting dtor",5);
}


void GreyImageOFile::setPallette(Pallette*)
{
   gDebug.msg("GreyImageOFile::setPallette(): can't set pallette",1);
   setFailWBit();
}

Pallette* GreyImageOFile::getPallette(void)
{
    gDebug.msg("GreyImageOFile::getPallette(): no pallette",1);
    setFailRBit();
    return NULL;
}
    
// THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageOFile::putPixel(AbstractPixel* p, long x, long y)
{
    if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
    {
       char dmsg[80];
       sprintf(dmsg,"GreyImageOFile::putPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    unsigned char color;
    GreyPixel* temp = (GreyPixel*) p;
 
    color = temp->getGrey();

    putRawPixel(&color,x,y);
}
    

    
void GreyImageOFile::putScanline(AbstractPixel* p, long row)
{
    if ((row < 0) || (row > (getHeight()-1)))
    {
       char dmsg[80];
       sprintf(dmsg,"GreyImageOFile::putScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    long i;
    unsigned char* line;
    GreyPixel* temp = (GreyPixel*)p;

    line = new unsigned char[getWidth()];
    
    
    for (i=0;i<getWidth();i++)
        line[i]   = temp[i].getGrey();

    putRawScanline(line,row);

    delete[] line;
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageOFile::putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[100];
     sprintf(dmsg,"GreyImageOFile::putRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailWBit();
     return;
  }

    int w, h, n, i;
    unsigned char* rect;
    GreyPixel* temp = (GreyPixel*)p;

    w = x2-x1+1;
    h = y2-y1+1;
    n = w*h;

    rect = new unsigned char[n];

    for (i=0;i<n;i++)
        rect[i] = temp[i].getGrey();

    putRawRectangle(rect,x1,y1,x2,y2); 

}

// THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageOFile::putRawPixel(void* p, long x, long y)
{
   if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
   {
      char dmsg[80];
      sprintf(dmsg,"GreyImageOFile::putRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailWBit();
      return;
   }

   long seekval;
   unsigned char* pix = (unsigned char*)p;

   seekval = y*getWidth()+x;

   Stream->seekp(seekval);
   Stream->write(pix,sizeof(unsigned char));
   if (!Stream->good())
   {
      gDebug.msg("GreyImageOFile::putRawPixel(): problem writing file",1);
      setFailWBit();
   }
}

void GreyImageOFile::putRawScanline(void* p, long row)
{
    if ((row < 0) || (row > getHeight()))
    {
       char dmsg[80];
       sprintf(dmsg,"GreyImageOFile::putRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    long seekval;
    unsigned char* pix = (unsigned char*)p;

    seekval = row*getWidth();

    Stream->seekp(seekval);
    Stream->write(pix,getWidth()*sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("GreyImageOFile::putRawScanline(): problem writing file",1);
       setFailWBit();
    }
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageOFile::putRawRectangle(void* p,long x1,long y1,long x2,long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[120];
     sprintf(dmsg,"GreyImageOFile::putRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailWBit();
     return;
  }

    long seekval;
    long i;
    long pixwidth;;
    unsigned char* line;
    unsigned char* pix = (unsigned char*)p;

    pixwidth = x2-x1+1;

    for (i=y1;i<=y2;i++)
    {
        line = &(pix[(i-y1)*pixwidth]); 
        seekval = i*getWidth()+x1;
        Stream->seekp(seekval);
        Stream->write(line,pixwidth*sizeof(unsigned char));
        if (!Stream->good())
        {
           gDebug.msg("GreyImageOFile::putRawRectangle(): problem writing file",1);
           setFailWBit();
        }
    }
}

void GreyImageOFile::writeHeader(void)
{
    ofstream* temp;

    temp = new ofstream(headerFile,ios::out|ios::trunc);
    if (!temp->good())
    {
       gDebug.msg("GreyImageOFile::writeHeader(): can't open header",1);
       setFailWBit();
    }
    else
    {
       *temp << "Width: " << getWidth() << endl;
       *temp << "Height: " << getHeight() << endl;
    }
    delete temp;
}

bool GreyImageOFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
