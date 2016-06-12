//  RGBImageIOFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class RGBImageIOFile 

#include "Image/RGBImageIOFile.h"
#include "Image/RGBPixel.h"
#include "Image/RGBPallette.h"
#include "Image/ImageCommon.h"
#include <string.h>
#include "Image/ImageDebug.h"
#include <stdio.h>


RGBImageIOFile::RGBImageIOFile(const char* fn, const char* header):
   ImageIOFile(fn)
{

    setPhotometric(DRG_RGB);
    headerFile = new char[strlen(header)+1];
    strcpy(headerFile,header);
  
    Stream = new fstream((char*)fn,ios::in|ios::out|ios::nocreate);
  
    if (!Stream->good()) 
    {
       gDebug.msg("RGBImageIOFile::RGBImageIOFile(c*,c*): can't open file",1);
       setNoDataBit();
    }
    else
       readHeader();
    setRandomAccessFlags(rpixel|rrow|rrect|wpixel|wrow|wrect);
    unsetHasPallette();
    setBitsPerSample(8);
    setSamplesPerPixel(3);
    gDebug.msg("RGBImageIOFile::RGBImageIOFile(c*,c*): exiting ctor",5);
}

RGBImageIOFile::RGBImageIOFile(const char* fn):
   ImageIOFile(fn)
{

    setPhotometric(DRG_RGB);
    headerFile = new char[strlen(fn)+5];
    strcpy(headerFile,fn);
    strcat(headerFile,".hdr"); 
  
    Stream = new fstream((char*)fn,ios::in|ios::out|ios::nocreate);
  
    if (!Stream->good()) 
    {
       gDebug.msg("RGBImageIOFile::RGBImageIOFile(c*): can't open file",1);
       setNoDataBit();
    }
    else
       readHeader();
    setRandomAccessFlags(rpixel|rrow|rrect|wpixel|wrow|wrect);
    unsetHasPallette();
    setBitsPerSample(8);
    setSamplesPerPixel(3);
    gDebug.msg("RGBImageIOFile::RGBImageIOFile(c*): exiting ctor",5);
}

RGBImageIOFile::~RGBImageIOFile()
{
    if (good())
       writeHeader();
    else
       gDebug.msg("RGBImageIOFile::~RGBImageIOFile(): can't write header",2);
    if (Stream) delete Stream;
    if (headerFile) delete headerFile;
    gDebug.msg("RGBImageIOFile::~RGBImageIOFile(): exiting dtor",5);
}

void RGBImageIOFile::setPallette(Pallette*)
{
    gDebug.msg("RGBImageIOFile::setPallette(): can't set pallette",1);
    setFailWBit();
}

Pallette* RGBImageIOFile::getPallette(void)
{
  long x, i, j, k, l, found;
  RGBPixel* pixelbuff=NULL;
  RGBPixel* temp = NULL;
  RGBPallette* pal;
  unsigned char* line;
  unsigned char red, green, blue;

  x = 0;

  Stream->seekg(0);

  for(i=0; i<getHeight(); i++)
  {
    line = (unsigned char*)getRawScanline(i);
    for(j=0; j<(3*getWidth()); j+=3)
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

AbstractPixel* RGBImageIOFile::getPixel(long x, long y)
{
  if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIOFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y); 
     gDebug.msg(dmsg,1);
     setFailRBit();  
     return NULL;
  }

  RGBPixel *pixel;
  unsigned char *buf;

  buf = (unsigned char *) getRawPixel(x,y);

  pixel = new RGBPixel(buf[0],buf[1],buf[2]);

  delete[] buf;

  return (AbstractPixel*) pixel;
}

AbstractPixel* RGBImageIOFile::getScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIOFile::getScanline(): scanline %ld out of bounds",row); 
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* line;
  RGBPixel* pixelbuf;
  long i;

  pixelbuf = new RGBPixel[getWidth()];
 
  line = (unsigned char *) getRawScanline(row);

  for (i=0;i<getWidth();i++)
  {
      pixelbuf[i].setRed(line[3*i]);
      pixelbuf[i].setGreen(line[3*i+1]);
      pixelbuf[i].setBlue(line[3*i+2]);
  }

  delete[] line;

  return((AbstractPixel*)pixelbuf);
}

AbstractPixel* RGBImageIOFile::getRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[100];
     sprintf(dmsg,"RGBImageIOFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2); 
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  RGBPixel *pixelbuf;
  unsigned char *rect;
  int w, h, n, i;

  w = x2 - x1 + 1;
  h = y2 - y1 + 1;
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


void* RGBImageIOFile::getRawPixel(long x, long y)
{
  if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIOFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y); 
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  long seekval;
  unsigned char *pixel;

  pixel = new unsigned char[3];

  seekval = 3*((y*getWidth())+x);
  Stream->seekg(seekval);
  Stream->read(pixel,3*sizeof(unsigned char));
  if (!Stream->good())
  {
     gDebug.msg("RGBImageIOFile::getRawPixel(): problem reading file",1);
     setFailRBit();
  }

  return pixel;
}


void*  RGBImageIOFile::getRawScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"RGBImageIOFile::getRawScanline(): scanline %ld out of bounds",row); 
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* pixelbuf;
  long seekval;

  pixelbuf = new unsigned char[3*getWidth()];

  seekval = 3*row*getWidth();
  Stream->seekg(seekval);
  Stream->read(pixelbuf,3*getWidth()*sizeof(unsigned char));
  if (!Stream->good())
  {
     gDebug.msg("RGBImageIOFile::getRawScanline(): problem reading file",1);
     setFailRBit();
  }

  return(pixelbuf);
}


void*  RGBImageIOFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[120];
     sprintf(dmsg,"RGBImageIOFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2); 
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  unsigned char* buffer;
  unsigned char* ptr;
  long length, y, k, seekval;

  length = 3*(x2-x1+1);
  k = 0;
  buffer = new unsigned char[3*(x2-x1+1)*(y2-y1+1)];

  for(y=y1; y<=y2; y++)
  {
    seekval = 3*(y*getWidth()+x1);
    ptr = &(buffer[(y-y1)*length]);
    Stream->seekg(seekval);
    Stream->read(ptr,(sizeof(unsigned char))*length);
    if (!Stream->good())
    {
       gDebug.msg("RGBImageIOFile::getRawRectangle(): problem reading file",1);
       setFailRBit(); 
    }
  }
  return buffer;
}


void RGBImageIOFile::putPixel(AbstractPixel* p, long x, long y)
{
    unsigned char colors[3];

    if ((x > (getWidth()-1))||(x<0)||(y<0))
    {
       char dmsg[80];
       sprintf(dmsg,"RGBImageIOFile::putPixel(): pixel (%ld,%ld) out of bounds",x,y); 
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
    if (y > (getHeight()-1))
       setHeight(y+1);
 
    colors[0] = p->getRed();
    colors[1] = p->getGreen();
    colors[2] = p->getBlue();

    putRawPixel(colors,x,y);
}
    

    
void RGBImageIOFile::putScanline(AbstractPixel* p, long row)
{
    if (row<0)
    {
       char dmsg[80];
       sprintf(dmsg,"RGBImageIOFile::putScanline(): scanline %ld out of bounds",row); 
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
    if (row>(getHeight()-1))
       setHeight(row+1);

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


void RGBImageIOFile::putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2)
{
    if ((x1<0) || (x2<0) || (y1<0) || (y2<0))
       setFailWBit();
    if ((x2<x1) || (y2<y1))
       setFailWBit();
    if ((x1>(getWidth()-1))||(x2>(getWidth())-1))
       setFailWBit();
    if (!good())
    {
       char dmsg[100];
       sprintf(dmsg,"RGBImageIOFile::putRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2); 
       gDebug.msg(dmsg,1);
       return;
    }
    if (y2>(getHeight()-1))
       setHeight(y2+1);

    RGBPixel* temp = (RGBPixel*)p;
    unsigned char *rect;
    int w,h,n,i;

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

void RGBImageIOFile::putRawPixel(void* p, long x, long y)
{
    if ((x > (getWidth()-1))||(x<0)||(y<0))
    {
       char dmsg[80];
       sprintf(dmsg,"RGBImageIOFile::putRawPixel(): pixel (%ld,%ld) out of bounds",x,y); 
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
    if (y > (getHeight()-1))
    {
       gDebug.msg("RGBImageIOFile::putRawPixel(): extending file height",2);
       setHeight(y+1);
    }

   long seekval;
   unsigned char* pix = (unsigned char*)p;

   seekval = 3*(y*getWidth()+x);

   Stream->seekp(seekval);
   Stream->write(pix,3*sizeof(unsigned char));
   if (!Stream->good())
   {
      gDebug.msg("RGBImageIOFile::putRawPixel(): problem writing file",1);
      setFailWBit();
   }
}

void RGBImageIOFile::putRawScanline(void* p, long row)
{
    if (row<0)
    {
       char dmsg[80];
       sprintf(dmsg,"RGBImageIOFile::putRawScanline(): scanline %ld out of bounds",row); 
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
    if (row>(getHeight()-1))
    {
       gDebug.msg("RGBImageIOFile::putRawScanline(): extending file height",2);
       setHeight(row+1);
    }

    long seekval;
    unsigned char* pix = (unsigned char*)p;

    seekval = 3*row*getWidth();

    Stream->seekp(seekval);
    Stream->write(pix,getWidth()*3*sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("RGBImageIOFile::putRawScanline(): problem writing file",1);
       setFailWBit();
    }
}

void RGBImageIOFile::putRawRectangle(void* p,long x1,long y1,long x2,long y2)
{
    if ((x1<0) || (x2<0) || (y1<0) || (y2<0))
       setFailWBit();
    if ((x2<x1) || (y2<y1))
       setFailWBit();
    if ((x1>(getWidth()-1))||(x2>(getWidth()-1)))
       setFailWBit();
    if (!good())
    {
       char dmsg[120];
       sprintf(dmsg,"RGBImageIOFile::putRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2); 
       gDebug.msg(dmsg,1);
       return;
    }
    if (y2>(getHeight()-1))
    {
       gDebug.msg("RGBImageIOFile::putRawRectangle(): extending file height",2);
       setHeight(y2+1);
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
           gDebug.msg("RGBImageIOFile::putRawRectangle(): problem writing file",1);
           setFailWBit();
        }
    }
}

void RGBImageIOFile::getRawRGBPixel(long x,long y,RawRGBPixel* pix)
{
   if ((x<0)||(x>(getWidth()-1))||(y<0)||(y>(getHeight()-1)))
   {
       char dmsg[80];
       sprintf(dmsg,"RGBImageIOFile::putRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y); 
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

void RGBImageIOFile::getRawRGBScanline(long row,RawRGBPixel* pixbuf)
{
   if ((row<0)||(row>(getHeight()-1)))
   {
       char dmsg[80];
       sprintf(dmsg,"RGBImageIOFile::putRawRGBScanline(): scanline %ld out of bounds",row); 
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
       pixbuf[k].Red   = temp[i];
       pixbuf[k].Green = temp[i+1];
       pixbuf[k].Blue  = temp[i+2];
       k++;
   }
   delete[] temp;
}

void RGBImageIOFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                       RawRGBPixel* pixbuff)
{
   if ((x1<0)||(x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
      (x2<0)||(x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
      (x1>x2) || (y1>y2))
   {
      char dmsg[120];
      sprintf(dmsg,"RGBImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld
) out of bounds",x1,y1,x2,y2);
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

void RGBImageIOFile::readHeader(void)
{
    ifstream* hdrfile;
    long val1, val2;
    char hdr1[10], hdr2[10];

    val1 = val2 = 0;
    hdr1[0] = hdr2[0] = '\0';

    hdrfile = new ifstream(headerFile,ios::in|ios::nocreate);
 
    if (!hdrfile->good())
    {
       gDebug.msg("RGBImageIOFile::readHeader(): can't open header",1);
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


void RGBImageIOFile::writeHeader(void)
{
    ofstream* temp;

    temp = new ofstream(headerFile,ios::out|ios::trunc);
    
    if (!temp->good())
    {
       gDebug.msg("RGBImageIOFile::writeHeader(): can't open header",1);
       setFailWBit();
    }
    else
    {
       *temp << "Width: " << getWidth() << endl;
       *temp << "Height: " << getHeight() << endl;
    }
    delete temp;
}

bool RGBImageIOFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
