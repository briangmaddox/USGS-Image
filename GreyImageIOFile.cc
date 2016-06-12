//  GreyImageIOFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class GreyImageIOFile 

#include "Image/GreyImageIOFile.h"
#include "Image/GreyPixel.h"
#include "Image/GreyPallette.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <string.h>
#include <stdio.h>


GreyImageIOFile::GreyImageIOFile(const char* fn, const char* header):
   ImageIOFile(fn)
{
    setPhotometric(DRG_GREY);
    headerFile = new char[strlen(header)+1];
    strcpy(headerFile,header);
  
    Stream = new fstream((char*)fn,ios::in|ios::out|ios::nocreate);
  
    if (!Stream->good()) 
    {
       gDebug.msg("GreyImageIOFile::GreyImageIOFile(c*,c*): can't open file",1);
       setNoDataBit();
    }
    else
       readHeader();
    setRandomAccessFlags(rpixel|rrow|rrect|wpixel|wrow|wrect);
    unsetHasPallette();
    setBitsPerSample(8);
    setSamplesPerPixel(1);
    gDebug.msg("GreyImageIOFile::GreyImageIOFile(c*,c*): exiting ctor",5);
}

GreyImageIOFile::GreyImageIOFile(const char* fn):
   ImageIOFile(fn)
{
    setPhotometric(DRG_GREY);
    headerFile = new char[strlen(fn)+5];
    strcpy(headerFile,fn);
    strcat(headerFile,".hdr");
  
    Stream = new fstream((char*)fn,ios::in|ios::out|ios::nocreate);
  
    if (!Stream->good()) 
    {
       gDebug.msg("GreyImageIOFile::GreyImageIOFile(c*): can't open file",1);
       setNoDataBit();
    }
    else
       readHeader();
    setRandomAccessFlags(rpixel|rrow|rrect|wpixel|wrow|wrect);
    unsetHasPallette();
    setBitsPerSample(8);
    setSamplesPerPixel(1);
    gDebug.msg("GreyImageIOFile::GreyImageIOFile(c*): exiting ctor",5);
}

GreyImageIOFile::~GreyImageIOFile()
{
    if (good())
       writeHeader();
    else
       gDebug.msg("GreyImageIOFile::~GreyImageIOFile(): can't write header",1);
    if (Stream) delete Stream;
    if (headerFile) delete headerFile;
    gDebug.msg("GreyImageIOFile::~GreyImageIOFile(): exiting dtor",5);
}

void GreyImageIOFile::setPallette(Pallette*)
{
   gDebug.msg("GreyImageIOFile::setPallette(): can't write pallette",1);
   setFailWBit();
}

Pallette* GreyImageIOFile::getPallette(void)
{
  long x, i, j, k, l, found;
  GreyPixel* pixelbuff=NULL;
  GreyPixel* temp = NULL;
  GreyPallette* pal;
  unsigned char* line;

  x = 0;

  Stream->seekg(0);

  for(i=0; i<getHeight(); i++)
  {
    line = (unsigned char*)getRawScanline(i);
    for(j=0; j<getWidth(); j++)
    {
      found = 0;
      for(k=0; k<x; k++)
      {
         if (line[j] == pixelbuff[k].getGrey())
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
    delete line;
  } // for(i)
  pal = new GreyPallette(x, pixelbuff);

  if (pixelbuff) delete[] pixelbuff;

  return((Pallette*)pal);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

AbstractPixel* GreyImageIOFile::getPixel(long x, long y)
{
   if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
   {
      char dmsg[80];
      sprintf(dmsg,"GreyImageIOFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
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

AbstractPixel* GreyImageIOFile::getScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"GreyImageIOFile::getScanline(): scanline %ld out of bounds",row);
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
      pixelbuf[i].setGrey(line[i]);
  
  delete[] line;

  return((AbstractPixel*)pixelbuf);
}

//  THIS IS NOW IN COLUMN MAJOR ORDER

AbstractPixel* GreyImageIOFile::getRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[100];
     sprintf(dmsg,"GreyImageIOFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  GreyPixel *pixelbuf;
  unsigned char *buffer;
  int w,h,n;
  int i;

  w = x2 - x1 + 1;
  h = y2 - y1 + 1;
  n = w*h;

  pixelbuf = new GreyPixel[(x2-x1+1)*(y2-y1+1)];
  buffer = (unsigned char *) getRawRectangle(x1,y1,x2,y2);

  for (i=0; i<n; i++)
      pixelbuf[i].setGrey(buffer[i]);

  delete[] buffer;

  return((AbstractPixel*)pixelbuf);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void* GreyImageIOFile::getRawPixel(long x, long y)
{
  if ((x < 0) || (x > (getWidth()-1)) || (y < 0) || (y > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"GreyImageIOFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

  long seekval;
  unsigned char *pixel;

  pixel = new unsigned char;

  seekval = y*getWidth()+x;
  Stream->seekg(seekval);
  Stream->read(pixel,sizeof(unsigned char));
  if (!Stream->good())
  {
     gDebug.msg("GreyImageIOFile::getRawPixel(): problem reading file",1);
     setFailRBit();
  }

  return pixel;
}


void*  GreyImageIOFile::getRawScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
     char dmsg[80];
     sprintf(dmsg,"GreyImageIOFile::getRawScanline(): scanline %ld out of bounds",row);
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
     gDebug.msg("GreyImageIOFile::getRawScanline(): problem reading file",1);
     setFailRBit();
  }
  
  return(pixelbuf);
}

//  THIS IS NOW IN COLUMN MAJOR ORDER

void*  GreyImageIOFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > (getWidth()-1)) || (y1 < 0) || (y1 > (getHeight()-1)) ||
     (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[120];
     sprintf(dmsg,"GreyImageIOFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
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
    Stream->read(ptr,(sizeof(unsigned char))*length);
    if (!Stream->good())
    {
       gDebug.msg("GreyImageIOFile::getRawRectangle(): problem reading file",1);
       setFailRBit();
    }
  }
  return buffer;
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageIOFile::putPixel(AbstractPixel* p, long x, long y)
{
    unsigned char color;

    if ((x > (getWidth()-1))||(x<0)||(y<0))
    {
       char dmsg[80];
       sprintf(dmsg,"GreyImageIOFile::putPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
    if (y > (getHeight()-1))
       setHeight(y+1);
 
   
    
    color = p->getGrey();

    putRawPixel(&color,x,y);
}
    

    
void GreyImageIOFile::putScanline(AbstractPixel* p, long row)
{
    if (row<0)
    {
       char dmsg[80];
       sprintf(dmsg,"GreyImageIOFile::putScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
    if (row>(getHeight()-1))
       setHeight(row+1);

    long i;
    unsigned char* line;
    GreyPixel* temp = (GreyPixel*)p;

    line = new unsigned char[getWidth()];
    
    
    for (i=0;i<getWidth();i++)
        line[i]   = temp[i].getGrey();

    putRawScanline(line,row);

    delete[] line;
}

//  THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageIOFile::putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2)
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
       sprintf(dmsg,"GreyImageIOFile::putRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       return;
    }
    if (y2>(getHeight()-1))
       setHeight(y2+1);

    int i;
    int w,h,n;
    unsigned char* rect;
    GreyPixel* temp = (GreyPixel*)p;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    n = w*h; 

    rect = new unsigned char[n];
    for (i=0;i<n;i++)
        rect[i] = temp[i].getGrey();
    putRawRectangle(rect,x1,y1,x2,y2);

    delete[] rect;
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageIOFile::putRawPixel(void* p, long x, long y)
{
   if ((x > (getWidth()-1))||(x<0)||(y<0))
   {
      char dmsg[80];
      sprintf(dmsg,"GreyImageIOFile::putRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailWBit();
      return;
   }
   if (y > (getHeight()-1))
   {
      gDebug.msg("GreyImageIOFile::putRawPixel(): extending image height",2);
      setHeight(y+1);
   }

   long seekval;
   unsigned char* pix = (unsigned char*)p;

   seekval = y*getWidth()+x;

   Stream->seekp(seekval);
   Stream->write(pix,sizeof(unsigned char));
   if (!Stream->good())
   {
      gDebug.msg("GreyImageIOFile::putRawPixel(): problem writing file",1);
      setFailWBit();
   }
}

void GreyImageIOFile::putRawScanline(void* p, long row)
{
    if (row<0)
    {
       char dmsg[80];
       sprintf(dmsg,"GreyImageIOFile::putRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
    if (row>(getHeight()-1))
    {
       gDebug.msg("GreyImageIOFile::putRawScanline(): extending image height",2);
       setHeight(row+1);
    }

    long seekval;
    unsigned char* pix = (unsigned char*)p;

    seekval = row*getWidth();

    Stream->seekp(seekval);
    Stream->write(pix,getWidth()*sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("GreyImageIOFile::putRawScanline(): problem writing file",1);
       setFailWBit();
    }
}

//  THIS IS NOW IN COLUMN MAJOR ORDER

void GreyImageIOFile::putRawRectangle(void* p,long x1,long y1,long x2,long y2)
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
       sprintf(dmsg,"GreyImageIOFile::putRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       return;
    }
    if (y2>(getHeight()-1))
    {
       gDebug.msg("GreyImageIOFile::putRawRectangle(): extending image height",2);
       setHeight(y2+1);
    }

    long seekval;
    long i;
    long pixwidth; 
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
           gDebug.msg("GreyImageIOFile::putRawRectangle(): problem writing file",1);
           setFailWBit();
        }
    }
}

void GreyImageIOFile::getRawRGBPixel(long x,long y,RawRGBPixel* pix)
{
   if ((x<0)||(x>(getWidth()-1))||(y<0)||(y>(getHeight()-1)))
   {
       char dmsg[80];
       sprintf(dmsg,"GreyImageIOFile::putRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return;
   }
   unsigned char* temp;

   temp = (unsigned char*) getRawPixel(x,y);
   pix->Red   = *temp;
   pix->Green = *temp;
   pix->Blue  = *temp;
   delete temp;
}

void GreyImageIOFile::getRawRGBScanline(long row,RawRGBPixel* pixbuf)
{
   if ((row<0)||(row>(getHeight()-1)))
   {
       char dmsg[80];
       sprintf(dmsg,"GreyImageIOFile::putRawRGBScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return;
   }

   unsigned char* temp;
   long i,w;

   temp = (unsigned char*) getRawScanline(row);
   w = getWidth();
   for (i=0;i<w;i++)
   {
       pixbuf[i].Red   = temp[i];
       pixbuf[i].Green = temp[i];
       pixbuf[i].Blue  = temp[i];
   }
   delete[] temp;
}

void GreyImageIOFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                        RawRGBPixel* pixbuf)
{

   if ((x1<0)||(x1>(getWidth()-1))||(y1 < 0) || (y1 > (getHeight()-1)) ||
      (x2 < 0) || (x2 > (getWidth()-1)) || (y2 < 0) || (y2 > (getHeight()-1)) ||
      (x1>x2) || (y1>y2))
   {
      char dmsg[120];
      sprintf(dmsg,"GreyImageIOFile::getRawRGBRectangle(): rectangle (%ld,%ld,%ld ,%ld) out of bounds",x1,y1,x2,y2);
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

void GreyImageIOFile::readHeader(void)
{
    ifstream* hdrfile;
    long val1, val2;
    char hdr1[10], hdr2[10];

    val1 = val2 = 0;
    hdr1[0] = hdr2[0] = '\0';

    hdrfile = new ifstream(headerFile,ios::in|ios::nocreate);
 
    if (!hdrfile->good())
    {
       gDebug.msg("GreyImageIOFile::readHeader(): can't open header",1);
       setFailRBit();
       delete hdrfile;
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


void GreyImageIOFile::writeHeader(void)
{
    ofstream* temp;

    temp = new ofstream(headerFile,ios::out|ios::trunc);
    if (!temp->good())
    {
       gDebug.msg("GreyImageIOFile::writeHeader(): can't open header",1);
       setFailWBit();
    }
    else
    { 
       *temp << "Width: " << getWidth() << endl;
       *temp << "Height: " << getHeight() << endl;
    }

    delete temp;
}

bool GreyImageIOFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
