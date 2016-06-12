//  DGPImageOFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class DGPImageOFile 

#include "Image/DGPImageOFile.h"
#include "Image/Pallette.h"
#include "Image/RGBPixel.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdio.h>
#include <fstream.h>
#include <stdlib.h>
#include <string.h>


DGPImageOFile::DGPImageOFile(const char* fn, long w, long h):
    ImageOFile(fn,w,h)
{
   Stream = new ofstream(fn,ios::out | ios::trunc);
   if (!Stream->good()) 
   {
      gDebug.msg("DGPImageOFile::DGPImageOFile(c*,l,l): can't open file",1);
      setNoDataBit();
   }
   else
   {
      setPhotometric(DRG_RGB);
   
      Name = new char[strlen("DGP")+1];
      strcpy(Name,"DGP");
      Version = new char[strlen("1.0")+1];
      strcpy(Version,"1.0");
      XUpperLeft = 0.0;
      YUpperLeft = 0.0;
      PixelSize = 1;
      NoColors = 256;
      ColorsFound = 0;
      initializePallette();
      writeHeader();
   }
   setRandomAccessFlags(wpixel|wrow|wrect);
   setHasPallette();
   setBitsPerSample(8);
   setSamplesPerPixel(1);
   gDebug.msg("DGPImageOFile::DGPImageOFile(): exiting ctor",5);
}

DGPImageOFile::DGPImageOFile(const char* fn, long w, long h,
                             unsigned long c, double x, double y, 
                             double p):
    ImageOFile(fn,w,h)
{
   Stream = new ofstream(fn,ios::out | ios::trunc);
   if (!Stream->good())
   {
      gDebug.msg("DGPImageOFile::DGPImageOFile(c*,l,l,ul,d,d,d): can't open file",1);
      setNoDataBit();
   }
   else
   {
      setPhotometric(DRG_RGB);
   
      Name = new char[strlen("DGP")+1];
      strcpy(Name,"DGP");
      Version = new char[strlen("1.0")+1];
      strcpy(Version,"1.0");
      XUpperLeft = x;
      YUpperLeft = y;
      PixelSize = p;
      NoColors = c;
      ColorsFound = 0;
      initializePallette();
      writeHeader();
   }
   setRandomAccessFlags(wpixel|wrow|wrect);
   setHasPallette();
   setBitsPerSample(8);
   setSamplesPerPixel(1);
   gDebug.msg("DGPImageOFile::DGPImageOFile(): exiting ctor",5);
}

DGPImageOFile::~DGPImageOFile()
{
   if (Stream!=NULL) delete Stream;
   if (Version!=NULL) delete[] Version;
   if (Name!=NULL) delete[] Name;
   if (Pal!=NULL) delete Pal;
   gDebug.msg("DGPImageOFile::~DGPImageOFile(): exiting dtor",5);
}

Pallette* DGPImageOFile::getPallette(void)
{
    unsigned long i;
    RGBPallette* foo = new RGBPallette(Pal->getNoEntries());
    RGBPixel* foobar;

    for (i = 0; i < Pal->getNoEntries(); i++)
    {
        foobar = (RGBPixel*) Pal->getEntry(i);
        foo->setEntry(i,foobar);
        delete foobar;
    }
 
    return (Pallette*) foo; 
}
    
    

void DGPImageOFile::setPallette(Pallette* p)
{
   unsigned long i;
   RGBPixel* foo;
   RGBPallette* tpal = (RGBPallette*)p;

   if (p->getNoEntries() > NoColors)
   {
      gDebug.msg("DGPImageOFile::setPallette(): Can't grow colors",1);
      setFailWBit();
      return;
   }

   ColorsFound = tpal->getNoEntries();

   for (i=0;i<p->getNoEntries();i++)
   {
       foo = tpal->getEntry(i);
       Pal->setEntry(i,foo);
       delete foo;
   } 
   writePallette();
  
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void DGPImageOFile::putRawPixel(void* p, long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageOFile::putRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    unsigned char* ptr;
    long seekval;

    seekval = x*getWidth()+y;

    Stream->seekp(seekval+TopOfData);
    ptr = (unsigned char*)p;

    Stream->write(ptr,sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("DGPImageOFile::putRawPixel(): problem writing file",1);
       setFailWBit();
    }
}

void DGPImageOFile::putRawScanline(void* p, long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageOFile::putRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    unsigned char* ptr;
    long seekval;

    seekval = row*getWidth();

    Stream->seekp(seekval+TopOfData);
    ptr = (unsigned char*)p;

    Stream->write(ptr,getWidth()*sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("DGPImageOFile::putRawScanline(): problem writing file",1);
       setFailWBit();
    }
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void DGPImageOFile::putRawRectangle(void* p, long x1, long y1, long x2, long y2)
{
    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[100];
       sprintf(dmsg,"DGPImageOFile::putRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    long width;
    long seekval;
    long i;
    unsigned char* ptr;
    unsigned char* pixbuf = (unsigned char*)p;

    width = x2-x1+1;

    for (i=y1;i<=y2;i++)
    {
        seekval = i*getWidth()+x1;
        Stream->seekp(seekval+TopOfData);
        ptr = &(pixbuf[(i-y1)*width]);
        Stream->write(ptr,width*sizeof(unsigned char));
        if (!Stream->good())
        {
           gDebug.msg("DGPImageOFile::putRawRectangle(): problem writing file",1);
           setFailWBit();
        }
    }
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void DGPImageOFile::putPixel(AbstractPixel* p, long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageOFile::putPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    unsigned char index;

    index = getPalletteIndex((RGBPixel*)p);

    putRawPixel(&index,x,y);
}

void DGPImageOFile::putScanline(AbstractPixel* p, long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageOFile::putScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    long i;
    RGBPixel* pixbuf = (RGBPixel*)p;
    unsigned char* line = new unsigned char[getWidth()];

    for (i=0;i<getWidth();i++)
        line[i] = getPalletteIndex(&(pixbuf[i]));

    putRawScanline(line,row);
    delete[] line;
   
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void DGPImageOFile::putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2)
{
    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[120];
       sprintf(dmsg,"DGPImageOFile::putRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    int i;
    int w,h,n;
    RGBPixel* pixbuf = (RGBPixel*)p;
    unsigned char* rect;

    h = y2-y1+1;
    w = x2-x1+1;
    n = w*h;

    rect = new unsigned char[n];
   
    for (i=0;i<n;i++)
        rect[i] = getPalletteIndex(&(pixbuf[i]));
 
    putRawRectangle(rect,x1,y1,x2,y2);
    
    delete[] rect;
}
 


void DGPImageOFile::initializePallette(void)
{
    unsigned long i;
    RGBPixel* foo = new RGBPixel(0,0,0);

    Pal = new RGBPallette(NoColors);

    for (i=0;i<NoColors;i++)
    {
        Pal->setEntry(i,foo);
    }
    delete foo;
}
    


void DGPImageOFile::writeHeader(void)
{
    long width, height;
    unsigned long i;
    unsigned char* c_map = new unsigned char[3*NoColors];

    if (Pal)
    {
        for(i=0;i<NoColors;i++)
        {
            c_map[3*i]   = (*Pal)[i].getRed();
            c_map[3*i+1] = (*Pal)[i].getGreen();
            c_map[3*i+2] = (*Pal)[i].getBlue();
        }
    }
    else
    {
        gDebug.msg("DGPImageOFile::writeHeader(): no pallette",1);
        setFailWBit();
        return;
    }
 
   
    width=getWidth();
    height=getHeight();

    Stream->seekp(0);
    TopOfData=0;
    TopOfColormap=0;

    Stream->write(Name,(strlen(Name)+1)*sizeof(char));
    TopOfData += strlen(Name) + 1;
    TopOfColormap += strlen(Name) + 1;
    Stream->write(Version,(strlen(Version)+1)*sizeof(char));
    TopOfData += strlen(Version) + 1;
    TopOfColormap += strlen(Version) + 1;
    Stream->write((char*)&XUpperLeft,sizeof(double));
    TopOfData += sizeof(double); 
    TopOfColormap += sizeof(double); 
    Stream->write((char*)&YUpperLeft,sizeof(double));
    TopOfData += sizeof(double); 
    TopOfColormap += sizeof(double); 
    Stream->write((char*)&PixelSize,sizeof(double)); 
    TopOfData += sizeof(double); 
    TopOfColormap += sizeof(double); 
    Stream->write((char*)&width,sizeof(long));
    TopOfData += sizeof(long); 
    TopOfColormap += sizeof(long); 
    Stream->write((char*)&height,sizeof(long));  
    TopOfData += sizeof(long); 
    TopOfColormap += sizeof(long); 
    Stream->write((char*)&NoColors,sizeof(unsigned long));
    TopOfData += sizeof(unsigned long); 
    TopOfColormap += sizeof(unsigned long); 
    Stream->write(c_map,sizeof(unsigned char)*3*NoColors);
    TopOfData += 3*NoColors*sizeof(unsigned char);
    if (!Stream->good())
    {
       gDebug.msg("DGPImageOFile::writeHeader(): problem writing file",1);
       setFailWBit();
    }
    delete[] c_map;
}


unsigned char DGPImageOFile::getPalletteIndex(RGBPixel* p)
{
    unsigned long i;
    int found;
    unsigned char index = 0;
    long r, g, b;
    long dist, mindist;

    found = 0;
    i = 0;

    while ((i<ColorsFound) && (!found))
    {
        found = ((p->getRed() == (*Pal)[i].getRed())&&    
                 (p->getGreen() == (*Pal)[i].getGreen())&&    
                 (p->getBlue() == (*Pal)[i].getBlue()));
        i++;
    }    

    if (found)
    {
        index = --i;
    }
    else if (i<NoColors)
    {
        Pal->setEntry(i,p);
        index = i;
        ColorsFound++;
        writePallette();
    }
    else
    {
        mindist = 2147483648;
        for (i=0;i<ColorsFound;i++)
        {
            r = p->getRed() - (*Pal)[i].getRed();  
            g = p->getGreen() - (*Pal)[i].getGreen();  
            b = p->getBlue() - (*Pal)[i].getBlue();  
            dist = r*r + g*g + b*b;
            if (dist < mindist)
            {
                mindist = dist;
                index = i;
            }
        }
    }

    return index; 

}

void DGPImageOFile::writePallette(void)
{
    unsigned long i;
    unsigned char* c_map;

    c_map = new unsigned char[3*NoColors];

    if (Pal)
    {
        for(i=0;i<NoColors;i++)
        {
            c_map[3*i]   = (*Pal)[i].getRed();
            c_map[3*i+1] = (*Pal)[i].getGreen();
            c_map[3*i+2] = (*Pal)[i].getBlue();
        }
    }
    else
    {
        gDebug.msg("DGPImageOFile::writePallette(): no pallette",1);
        setFailWBit();
        return;
    }
    
    Stream->seekp(TopOfColormap);

    Stream->write(c_map,sizeof(unsigned char)*3*NoColors);
    if (!Stream->good())
    {
       gDebug.msg("DGPImageOFile::writePallette(): problem writing file",1);
       setFailWBit();
    }

    delete[] c_map;
}

bool DGPImageOFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
