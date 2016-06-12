//  DGPImageIFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class DGPImageIFile 

#include "Image/DGPImageIFile.h"
#include "Image/RGBPallette.h"
#include "Image/RGBPixel.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


DGPImageIFile::DGPImageIFile(const char* fn)
{
    long i;

    setPhotometric(DRG_RGB);
    clear();
    Stream = new ifstream(fn,ios::in|ios::nocreate);
    if (!Stream->good())
    {
       setNoDataBit();   
       gDebug.msg("DGPImageIFile::DGPImageIFile(): can't open file",1);
    }
    else
    {
       readHeader();

       hashTable = new char[getHeight()];
       for (i=0;i<getHeight();i++)
           hashTable[i] = (char) -1;
       for (i=0;i<50;i++)
       {
           rowCache[i] = NULL;
           timeStamp[i] = 0;
           rowNumber[i] = -1;
       }
       lastTimeStamp = 0;    
    }
    gDebug.msg("DGPImageIFile::DGPImageIFile(): exiting ctor",5);
    setRandomAccessFlags(rpixel|rrow|rrect);
    setHasPallette();
    setBitsPerSample(8);
    setSamplesPerPixel(1);
}


DGPImageIFile::~DGPImageIFile()
{
    int i;
    if (Stream!=NULL) delete Stream;
    if (Name!=NULL) delete[] Name;
    if (Version!=NULL) delete[] Version;
    if (Pal!=NULL) delete Pal;
    if (hashTable!=NULL) delete[] hashTable;
    for (i=0;i<50;i++)
        if (rowCache[i]!=NULL) delete[] rowCache[i];
    gDebug.msg("DGPImageIFile::~DGPImageIFile(): exiting dtor",5);
}

char* DGPImageIFile::getName(void)
{
    char* foo;
    foo = new char[strlen(Name)+1];
    strcpy(foo,Name);
    
    return foo;
}

char* DGPImageIFile::getVersion(void)
{
    char* foo;
    foo = new char[strlen(Version)+1];
    strcpy(foo,Version);
    
    return foo;
}

double DGPImageIFile::getX(void)
{
    return XUpperLeft;
}

double DGPImageIFile::getY(void)
{
    return YUpperLeft;
}

double DGPImageIFile::getPixelSize(void)
{
    return PixelSize;
}

Pallette* DGPImageIFile::getPallette(void)
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
 
AbstractPixel* DGPImageIFile::getPixel(long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
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

AbstractPixel* DGPImageIFile::getScanline(long row)
{
   if ((row < 0) || (row > getHeight()-1))
   {
      char dmsg[80];
      sprintf(dmsg,"DGPImageIFile::getScanline(): scanline %ld out of bounds",row);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return NULL;
   }

   RGBPixel* temp;
   long i;
   unsigned char* pix;

   
   pix = (unsigned char *) getRawScanline(row);
   
   temp = new RGBPixel[getWidth()];
  
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

AbstractPixel* DGPImageIFile::getRectangle(long x1, long y1, long x2, long y2)
{

  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
     (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[100];
     sprintf(dmsg,"DGPImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

    RGBPixel* temp;
    long w;
    long h;
    unsigned char* pix;
    long i,n;

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

void* DGPImageIFile::getRawPixel(long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }
   
    char entry;
    unsigned char* line;
    unsigned char* temp = new unsigned char;
    
    entry = cacheScanline(y);
    line = rowCache[entry];
    *temp = line[x];

    return temp;
}


void* DGPImageIFile::getRawScanline(long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIFile::getRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    char entry;
    unsigned char* line;
    unsigned char* temp;

    temp = new unsigned char[getWidth()];

    entry = cacheScanline(row);
    line = rowCache[entry];
    
    memcpy(temp,line,getWidth());

    return temp;
}

void* DGPImageIFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
     (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
     (x1>x2) || (y1>y2))
  {
     char dmsg[120];
     sprintf(dmsg,"DGPImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
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

    height = y2-y1+1;
    width = x2-x1+1;

    temp = new unsigned char[width*height];

    for (i=y1;i<=y2;i++)
    {
        ImageOffset = i*getWidth()+x1;
        Stream->seekg(TopOfData+ImageOffset);
        ptr = &(temp[(i-y1)*width]);
        Stream->read(ptr,width*sizeof(unsigned char));
        if (!Stream->good())
           setFailRBit();
    }

    return temp;
}
      

void DGPImageIFile::readHeader(void)
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

    Stream->read((char*)&XUpperLeft,sizeof(double));
    TopOfData += sizeof(double);
    Stream->read((char*)&YUpperLeft,sizeof(double));
    TopOfData += sizeof(double);
    Stream->read((char*)&PixelSize,sizeof(double));
    TopOfData += sizeof(double);
    Stream->read((char*)&width,sizeof(long));
    TopOfData += sizeof(long);
    Stream->read((char*)&height,sizeof(long));
    TopOfData += sizeof(long);
    Stream->read((char*)&num_colors,sizeof(unsigned long));
    TopOfData += sizeof(long);
    Stream->read((char*)&c_map,num_colors*3*sizeof(unsigned char));
    TopOfData += num_colors*3*sizeof(unsigned char);

    if (!Stream->good())
    {
       gDebug.msg("DGPImageIFile::readHeader(): problem reading header",1);
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

     
    
    
void DGPImageIFile::getRawRGBPixel(long x, long y,RawRGBPixel* pix)
{

    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
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


void DGPImageIFile::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{

    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"DGPImageIFile::getRawRGBScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return;
    }

    int i;
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

void DGPImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                       RawRGBPixel* pixbuf)
{

    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[100];
       sprintf(dmsg,"DGPImageIFile::getRawRGBRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
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

char DGPImageIFile::cacheScanline(long row)
{
   long ImageOffset;
   int i;
   char entry; 
   unsigned long min;

   if (hashTable[row] == -1)
   {
      min = 0;
      entry = 0;
      for (i=1;i<50;i++)
          if (timeStamp[i] < min )
          {
             min = timeStamp[i];
             entry = i;
          }
      lastTimeStamp++;
      if (rowCache[entry] == NULL) 
         rowCache[entry] = new unsigned char[getWidth()];
      if (rowNumber[entry] != -1)
         hashTable[rowNumber[entry]] = -1; 
      timeStamp[entry] = lastTimeStamp;
      hashTable[row] = entry;
      rowNumber[entry] = row;

      ImageOffset = row*getWidth();
      Stream->seekg(TopOfData+ImageOffset);
      Stream->read(rowCache[entry],getWidth()*sizeof(unsigned char));
      if (!Stream->good())
      {
         gDebug.msg("DGPImageIFile::cacheScanline(): problem reading scanline",1);
         setFailRBit();
      }
   }
   else
      entry = hashTable[row];

   return entry;
}


bool DGPImageIFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
