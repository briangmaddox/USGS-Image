//  DGPImageIOFile.cc                         13 Oct 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class DGPImageIFile 

#include "Image/DGPImageIOFile.h"
#include "Image/RGBPallette.h"
#include "Image/RGBPixel.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>


DGPImageIOFile::DGPImageIOFile(const char* fn)
{
    setPhotometric(DRG_RGB);
    Stream = new fstream((char *)fn,ios::in|ios::out|ios::nocreate);
    if (!Stream->good())
    {
       gDebug.msg("DGPImageIOFile::DGPImageIOFile(): can't open file",1);
       setNoDataBit();
    }
    else
    {
       readHeader();
       NoColors = Pal->getNoEntries();
       ColorsFound = NoColors;
    }
    gDebug.msg("DGPImageIOFile::DGPImageIOFile(): exiting ctor",5);
    setRandomAccessFlags(rpixel|rrow|rrect|wpixel|wrow|wrect);
    setHasPallette();
    setBitsPerSample(8);
    setSamplesPerPixel(1);
}


DGPImageIOFile::~DGPImageIOFile()
{
    if (Stream!=NULL) delete Stream;
    if (Name!=NULL) delete[] Name;
    if (Version!=NULL) delete[] Version;
    if (Pal!=NULL) delete Pal;
    gDebug.msg("DGPImageIOFile::~DGPImageIOFile(): exiting dtor",5);
}

char* DGPImageIOFile::getName(void)
{
    char* foo;
    foo = new char[strlen(Name)+1];
    strcpy(foo,Name);
  
    return foo;
}

char* DGPImageIOFile::getVersion(void)
{
    char* foo;
    foo = new char[strlen(Version)+1];
    strcpy(foo,Version);
    
    return foo;
}

double DGPImageIOFile::getX(void)
{
    return XUpperLeft;
}

double DGPImageIOFile::getY(void)
{
    return YUpperLeft;
}

double DGPImageIOFile::getPixelSize(void)
{
    return PixelSize;
}

Pallette* DGPImageIOFile::getPallette(void)
{
    RGBPallette* foo = new RGBPallette(Pal->getNoEntries());
    RGBPixel* foobar;
    long i;

    for (i=0; i<Pal->getNoEntries(); i++)
    {
        foobar = Pal->getEntry(i);
        foo->setEntry(i,foobar);
        delete foobar;
    }  
   
    return (Pallette*)foo;
}

// THIS IS NOW IN COLUMN MAJOR ORDER
 
AbstractPixel* DGPImageIOFile::getPixel(long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    RGBPixel* temp;
    unsigned char *pix;

    pix = (unsigned char *) getRawPixel(x,y);
 
    temp = new RGBPixel((*Pal)[*pix].getRed(),
                        (*Pal)[*pix].getGreen(),
                        (*Pal)[*pix].getBlue());

    delete pix;
    return (RGBPixel*)temp;
}

AbstractPixel* DGPImageIOFile::getScanline(long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::getScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    RGBPixel* temp;
    unsigned char* pix;
    int i;


    temp = new RGBPixel[getWidth()];
   
    pix = (unsigned char *) getRawScanline(row);
    for (i=0; i<getWidth(); i++)
    {
        temp[i].setRed((*Pal)[pix[i]].getRed());
        temp[i].setGreen((*Pal)[pix[i]].getGreen());
        temp[i].setBlue((*Pal)[pix[i]].getBlue());
    }
    delete[] pix;

    return (RGBPixel*)temp;
}

// THIS IS NOW IN COLUMN MAJOR ORDER

AbstractPixel* DGPImageIOFile::getRectangle(long x1, long y1, long x2, long y2)
{
    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[100];
       sprintf(dmsg,"DGPImageIOFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    RGBPixel* temp;
    int w;
    int h;
    int n; 
    unsigned char* pix;
    long i;

    h = y2-y1+1;
    w = x2-x1+1;
    n = w*h;
    temp = new RGBPixel[n];

    pix = (unsigned char *) getRawRectangle(x1,y1,x2,y2);
    for (i=0;i<n;i++)
    {
        temp[i].setRed((*Pal)[pix[i]].getRed()); 
        temp[i].setGreen((*Pal)[pix[i]].getGreen()); 
        temp[i].setBlue((*Pal)[pix[i]].getBlue()); 
    }

    delete[] pix;
    return (RGBPixel*)temp;
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void* DGPImageIOFile::getRawPixel(long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    unsigned char* temp = new unsigned char;
    long ImageOffset;

    ImageOffset = y*getWidth()+x;

    Stream->seekg(TopOfData+ImageOffset);
    Stream->read(temp,sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("DGPImageIOFile::getRawPixel(): problem reading file",2);
       setFailRBit();
    }

    return temp;
}


void* DGPImageIOFile::getRawScanline(long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::getRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    unsigned char* temp;
    long ImageOffset;

    ImageOffset = row*getWidth();
    temp = new unsigned char[getWidth()];

    Stream->seekg(TopOfData+ImageOffset);
    Stream->read(temp,getWidth()*sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("DGPImageIOFile::getRawScanline(): problem reading file",2);
       setFailRBit();
    }

    return temp;
}

void* DGPImageIOFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[120];
       sprintf(dmsg,"DGPImageIOFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    unsigned char* temp;
    long width;
    long height;
    long ImageOffset;
    unsigned char* ptr;
    long i;

    width = x2-x1+1;
    height = y2-y1+1;

    temp = new unsigned char[width*height];

    for (i=y1;i<=y2;i++)
    {
        ImageOffset = i*getWidth()+x1;
        Stream->seekg(TopOfData+ImageOffset);
        ptr = &(temp[(i-y1)*width]);
        Stream->read(ptr,width*sizeof(unsigned char));
        if (!Stream->good())
        {
           gDebug.msg("DGPImageIOFile::getRawRectangle(): problem reading file",2);
           setFailRBit();
        }
    }

    return temp;
}
      

void DGPImageIOFile::readHeader(void)
{
    char foo;
    int len1, len2;
    long width, height;
    unsigned long num_colors;
    unsigned char c_map[256*3];
    unsigned long i;
    RGBPixel* foobar;

    Stream->seekg(0);
    TopOfData = 0;

    len1 = 0;
    len2 = 0;

    Stream->read(&foo,sizeof(char));

    while (foo!=0)
    {
       len1++;
       Stream->read(&foo,sizeof(char));
    }
    Stream->read(&foo,sizeof(char));

    while (foo!=0)
    {
       len2++;
       Stream->read(&foo,sizeof(char));
    }


    Stream->seekg(0);
  
    Name = new char[len1+1];

    Stream->read(Name,sizeof(char)*(len1+1));

    Version = new char[len2+1];

    Stream->read(Version,sizeof(char)*(len2)+1);

    TopOfData = len1+len2+2;
    TopOfColormap = TopOfData;

    Stream->read((char*)&XUpperLeft,sizeof(double));
    TopOfData += sizeof(double);
    TopOfColormap += sizeof(double);
    Stream->read((char*)&YUpperLeft,sizeof(double));
    TopOfData += sizeof(double);
    TopOfColormap += sizeof(double);
    Stream->read((char*)&PixelSize,sizeof(double));
    TopOfData += sizeof(double);
    TopOfColormap += sizeof(double);
    Stream->read((char*)&width,sizeof(long));
    TopOfData += sizeof(long);
    TopOfColormap += sizeof(long);
    Stream->read((char*)&height,sizeof(long));
    TopOfData += sizeof(long);
    TopOfColormap += sizeof(long);
    Stream->read((char*)&num_colors,sizeof(unsigned long));
    TopOfData += sizeof(long);
    TopOfColormap += sizeof(long);
    Stream->read((char*)&c_map,num_colors*3*sizeof(unsigned char));
    TopOfData += num_colors*3*sizeof(unsigned char);

    if (!Stream->good())
    {
       gDebug.msg("DGPImageIOFile::readHeader(): problem reading header",1);
       setFailRBit();
    }

    setWidth(width);
    setHeight(height);
    
    Pal = new RGBPallette(num_colors);

    for (i=0;i<num_colors;i++)
    {
        foobar = new RGBPixel(c_map[3*i],c_map[3*i+1],c_map[3*i+2]);
        Pal->setEntry(i,foobar);
        delete foobar;
    }
}

     
void DGPImageIOFile::setPallette(Pallette* p)
{
   unsigned long i;
   RGBPixel* foo;
   RGBPallette* tpal = (RGBPallette*)p;

   if (p->getNoEntries() > NoColors)
   {
      gDebug.msg("DGPImageIOFile::setPallette(): can't grow colormap",1);
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

void DGPImageIOFile::putRawPixel(void* p, long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::putRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    unsigned char* ptr;
    long seekval;

    seekval = y*getWidth()+x;

    Stream->seekp(seekval+TopOfData);
    ptr = (unsigned char*)p;

    Stream->write(ptr,sizeof(unsigned char));
    if (!Stream->good())
    {
       gDebug.msg("DGPImageIOFile::putRawPixel(): problem writing file",2);
       setFailWBit();
    }
}

void DGPImageIOFile::putRawScanline(void* p, long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::putRawScanline(): scanline %ld out of bounds",row);
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
       gDebug.msg("DGPImageIOFile::putRawScanline(): problem writing file",2);
       setFailWBit();
    }
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void DGPImageIOFile::putRawRectangle(void* p, long x1, long y1, long x2, long y2)
{
    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[100];
       sprintf(dmsg,"DGPImageIOFile::putRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
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
           gDebug.msg("DGPImageIOFile::putRawRectangle(): problem writing file",2);
           setFailWBit();
        }
    }
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void DGPImageIOFile::putPixel(AbstractPixel* p, long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::putPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }
 

    unsigned char index;

    index = getPalletteIndex((RGBPixel*)p);

    putRawPixel(&index,x,y);
}

void DGPImageIOFile::putScanline(AbstractPixel* p, long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::putScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    long i;
    RGBPixel* pixbuf = (RGBPixel*)p;
    unsigned char* line = new unsigned char[getWidth()];

    for (i=0;i<getWidth();i++)
    {
        line[i] = getPalletteIndex(&(pixbuf[i]));
    }

    putRawScanline(line,row);
    
    delete[] line;
   
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void DGPImageIOFile::putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2)
{
    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[120];
       sprintf(dmsg,"DGPImageIOFile::putRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailWBit();
       return;
    }

    int i;
    int w,h;
    int n;
 
    RGBPixel* pixbuf = (RGBPixel*)p;
    unsigned char* rect;

    w = x2-x1+1;
    h = y2-y1+1;
    n = w*h;
   
    rect = new unsigned char[n];
    
    for (i=0;i<n;i++)
        rect[i] = getPalletteIndex(&(pixbuf[i]));

    putRawRectangle(rect,x1,y1,x2,y2);
    delete[] rect;
}
 


void DGPImageIOFile::writeHeader(void)
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
        gDebug.msg("DGPImageIOFile::writeHeader(): no pallette",1);
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
       gDebug.msg("DGPImageIOFile::writeHeader(): problem writing file",1);
       setFailWBit();
    }
}


unsigned char DGPImageIOFile::getPalletteIndex(RGBPixel* p)
{
    unsigned long i;
    int found;
    unsigned char index = 0;
    long r, g, b;
    long dist,mindist;

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

void DGPImageIOFile::writePallette(void)
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
        gDebug.msg("DGPImageIOFile::writePallette(): no pallette",1);
        return;
    }
    
    Stream->seekp(TopOfColormap);

    Stream->write(c_map,sizeof(unsigned char)*3*NoColors);
    if (!Stream->good())
    {
       gDebug.msg("DGPImageIOFile::writePallette(): problem writing file",1);
       setFailWBit();
    }

    delete[] c_map;
}
        



    
void DGPImageIOFile::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return;
    }

    unsigned char *pixel;

 
    pixel = (unsigned char *) getRawPixel(x,y);

    pix->Red   = (*Pal)[*pixel].getRed();
    pix->Green = (*Pal)[*pixel].getGreen();
    pix->Blue  = (*Pal)[*pixel].getBlue();
    delete pixel;
}

void DGPImageIOFile::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIOFile::getRawRGBScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return;
    }

    long i;
    unsigned char* pix;

    pix = (unsigned char *) getRawScanline(row);
    
    for (i=0; i<getWidth(); i++)
    {
        pixbuf[i].Red   = (*Pal)[pix[i]].getRed();
        pixbuf[i].Green = (*Pal)[pix[i]].getGreen();
        pixbuf[i].Blue  = (*Pal)[pix[i]].getBlue();
    }
    delete[] pix;
}

void DGPImageIOFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                       RawRGBPixel* pixbuf)
{

    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[100];
       sprintf(dmsg,"DGPImageIFile::getRawRGBRectangle(): rectangle (%ld,%ld,%ld
,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return;
    }

    int i;
    unsigned char* pix;

    pix = (unsigned char *) getRawRectangle(x1,y1,x2,y2);

    for (i=0; i<getWidth(); i++)
    {
        pixbuf[i].Red   = (*Pal)[pix[i]].getRed();
        pixbuf[i].Green = (*Pal)[pix[i]].getGreen();
        pixbuf[i].Blue  = (*Pal)[pix[i]].getBlue();
    }
    delete[] pix;
}

bool DGPImageIOFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
