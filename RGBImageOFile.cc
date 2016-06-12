//  RGBImageOFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class RGBImageOFile 

#include "Image/RGBImageOFile.h"
#include "Image/ImageCommon.h"
#include "Image/RGBPixel.h"
#include <string.h>
#include "Image/ImageDebug.h"
#include <stdio.h>



RGBImageOFile::RGBImageOFile(const char* fn, const char* header, 
                             long w, long l):
   ImageOFile(fn,w,l)
{
   setPhotometric(DRG_RGB);
   headerFile = new char[strlen(header)+1];
   strcpy(headerFile,header);

   Stream = new ofstream(fn,ios::out|ios::trunc);
   if (!Stream->good())
   {
      gDebug.msg("RGBImageOFile::RGBImageOFile(c*,c*,l,l): can't open file",1);
      setNoDataBit();
   }
   else
      writeHeader();
   setRandomAccessFlags(wpixel|wrow|wrect);
   unsetHasPallette();
   setBitsPerSample(8);
   setSamplesPerPixel(3);
   gDebug.msg("RGBImageOFile::RGBImageOFile(c*,c*,l,l): exiting ctor",5);
}

RGBImageOFile::RGBImageOFile(const char* fn, long w, long l):
   ImageOFile(fn,w,l)
{
   setPhotometric(DRG_RGB);
   headerFile = new char[strlen(fn)+5];
   strcpy(headerFile,fn);
   strcat(headerFile,".hdr");

   Stream = new ofstream(fn,ios::out|ios::trunc);
   if (!Stream->good())
   {
      gDebug.msg("RGBImageOFile::RGBImageOFile(c*,l,l): can't open file",1);
      setNoDataBit();
   }
   else
      writeHeader();
   setRandomAccessFlags(wpixel|wrow|wrect);
   unsetHasPallette();
   setBitsPerSample(8);
   setSamplesPerPixel(3);
   gDebug.msg("RGBImageOFile::RGBImageOFile(c*,l,l): exiting ctor",5);
}

RGBImageOFile::~RGBImageOFile()
{
   if (headerFile) delete[] headerFile;
   if (Stream) delete Stream;
   gDebug.msg("RGBImageOFile::~RGBImageOFile(): exiting dtor",5);
}


void RGBImageOFile::setPallette(Pallette*)
{
    gDebug.msg("RGBImageOFile::setPallette(): can't set pallette",1);
    setFailWBit();
}

Pallette* RGBImageOFile::getPallette(void)
{
    gDebug.msg("RGBImageOFile::getPallette(): no pallette",1);
    setFailRBit();
    return NULL;
}
    

void RGBImageOFile::putPixel(AbstractPixel* p, long x, long y)
{
    if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
    {
       char dmsg[80];
       sprintf(dmsg,"RGBImageOFile::putPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    unsigned char colors[3];
 
    colors[0] = p->getRed();
    colors[1] = p->getGreen();
    colors[2] = p->getBlue();

    putRawPixel(colors,x,y);
}
    

    
void RGBImageOFile::putScanline(AbstractPixel* p, long row)
{
    if ((row < 0) || (row > (getHeight()-1)))
    {
       char dmsg[80];
       sprintf(dmsg,"RGBImageOFile::putScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    int i;
    int w;
    unsigned char* line;
    RGBPixel* temp = (RGBPixel*)p;

    w = getWidth();
    line = new unsigned char[3*w];
    
    
    for (i=0;i<w;i++)
    {
        line[3*i]   = temp[i].getRed();
        line[3*i+1] = temp[i].getGreen();
        line[3*i+2] = temp[i].getBlue();
    }

    putRawScanline(line,row);

    delete[] line;
}


void RGBImageOFile::putRectangle(AbstractPixel* p, long x1, long y1,
                                                   long x2, long y2)
{
    if ((x1<0)||(x1>(getWidth()-1))||(y1<0)||(y1>(getHeight()-1))||
        (x2 < 0)||(x2>(getWidth()-1))||(y2<0)||(y2>(getHeight()-1))||
        (x1>x2) || (y1>y2))
    {
       char dmsg[100];
       sprintf(dmsg,"RGBImageOFile::putRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    int w,h,n,i;
    unsigned char* rect;
    RGBPixel* temp = (RGBPixel*)p;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    n = w*h;

    rect = new unsigned char[3*n];
    
    for (i=0;i<n;i++)
    {
        rect[3*i]   = temp[i].getRed(); 
        rect[3*i+1] = temp[i].getGreen(); 
        rect[3*i+2] = temp[i].getBlue(); 
    }
    
    putRawRectangle(rect,x1,y1,x2,y2); 

    delete[] rect;
}

void RGBImageOFile::putRawPixel(void* p, long x, long y)
{
   if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
   {
      char dmsg[80];
      sprintf(dmsg,"RGBImageOFile::putRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailWBit();
      return;
   }

   long seekval;
   unsigned char* pix = (unsigned char*)p;

   seekval = 3*(y*getWidth()+x);

   Stream->seekp(seekval);
   Stream->write(pix,3*sizeof(unsigned char));
   if (!Stream->good())
   {
      gDebug.msg("RGBImageOFile::putRawPixel(): problem writing file",1);
      setFailWBit();
   }
}

void RGBImageOFile::putRawScanline(void* p, long row)
{
    if ((row < 0) || (row > getHeight()))
    {
       char dmsg[80];
       sprintf(dmsg,"RGBImageOFile::putRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    long seekval;
    unsigned char* pix = (unsigned char*)p;

    seekval = 3*row*getWidth();

    Stream->seekp(seekval);
    Stream->write(pix,getWidth()*3*sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("RGBImageOFile::putRawScanline(): problem writing file",1);
       setFailWBit();
    }
}

void RGBImageOFile::putRawRectangle(void* p,long x1,long y1,long x2,long y2)
{
    if ((x1<0)||(x1>(getWidth()-1))||(y1<0)||(y1>(getHeight()-1))||
        (x2<0)||(x2>(getWidth()-1))||(y2<0)||(y2>(getHeight()-1))||
        (x1>x2)||(y1>y2))
    {
       char dmsg[120];
       sprintf(dmsg,"RGBImageOFile::putRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    long seekval;
    long i;
    long width,pixwidth;;
    unsigned char* line;
    unsigned char* pix = (unsigned char*)p;

    pixwidth = x2-x1+1;
    width = 3*pixwidth;

    for (i=y1;i<=y2;i++)
    {
        line = &(pix[(i-y1)*width]); 
        seekval = 3*(i*getWidth()+x1);
        Stream->seekp(seekval);
        Stream->write(line,width*sizeof(unsigned char));
        if (!Stream->good())
        {
           gDebug.msg("RGBImageOFile::putRawRectangle(): problem writing file",1);
           setFailWBit();
        }
    }
}

void RGBImageOFile::writeHeader(void)
{
    ofstream* temp;

    temp = new ofstream(headerFile,ios::out|ios::trunc);
  
    if (!temp->good())
    {
       gDebug.msg("RGBImageOFile::writeHeader(): can't open header",1);
       setFailWBit();
    }
    else
    {
       *temp << "Width: " << getWidth() << endl;
       *temp << "Height: " << getHeight() << endl;
    }

    delete temp;
}

bool RGBImageOFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
