//  TIFFImageIFile.cc                         11 Jul 94
//  Author:  Greg Martin and Duane Lascoe  SES, MCMC, USGS
//  Altered somewhat by Brian Maddox to allow it to read in raw RGB Tiffs

//  Implements member functions for the class TIFFImageIFile 

#include "Image/TIFFImageIFile.h"
#include "tiff/tiffio.h"
#include "tiff/tiff.h"
#include "geotiff/xtiffio.h"
#include "Image/RGBPixel.h"
#include "Image/AbstractPixel.h"
#include "Image/ImageCommon.h"
#include "Image/RGBPallette.h"
#include "Image/Pallette.h"
#include <stdlib.h>
#include "Image/ImageDebug.h"
#include <stdio.h>



// Constructor for TIFFImageIFile.h
// It will open a file for read and get Height, Width and the colormap
// Altered here to determine if we're dealing with a RGB tiff for a 
// pallette indexed tiff
TIFFImageIFile::TIFFImageIFile(const char* tfilename)
  : ImageIFile(tfilename)
{
  unsigned long twidth, theight;
  unsigned short i_photometric, samples_pixel;
  long i;

  if ((tif = XTIFFOpen(tfilename, "r")) == NULL)
  {
    gDebug.msg("TIFFImageIFile::TIFFImageIFile(): can't open file",1);
    setNoDataBit();
  }
  else
  {
    //red   = new unsigned short[256];
    //green = new unsigned short[256];
    //blue  = new unsigned short[256]; 
    if (TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&twidth) == 0)
      setFailRBit();
    if (TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&theight) == 0)
      setFailRBit();
    if (TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &i_photometric) == 0)
      setFailRBit();
    if (i_photometric != PHOTOMETRIC_RGB)
    {
      if (TIFFGetField(tif,TIFFTAG_COLORMAP,&red,&green,&blue) == 0)
	setFailRBit();
    }
    if (TIFFGetField(tif,TIFFTAG_BITSPERSAMPLE,&bits_per_sample) == 0)
      setFailRBit();
    if (TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_pixel) == 0)
      setFailRBit();

    if (good())
    {
      setWidth(twidth);
      setHeight(theight);
      if (i_photometric == PHOTOMETRIC_RGB)
	setPhotometric(DRG_REAL_RGB);
      else
      {
	setPhotometric(DRG_RGB);
	setHasPallette();
      }
      setBitsPerSample(bits_per_sample);
      setSamplesPerPixel(1);

      hashTable = new char[getHeight()];
      for (i=0;i<getHeight();i++)
	hashTable[i] = (char) -1;
      for (i=0;i<20;i++)
      {
	if (i_photometric == PHOTOMETRIC_RGB)
	  rowCache[i] = new unsigned char[3 * getWidth()];
	else
	  rowCache[i] = new unsigned char[getWidth()];
	timeStamp[i] = 0;
	rowNumber[i] = -1;
      }
      lastTimeStamp = 0;
    }
  }
  setRandomAccessFlags(rpixel|rrow|rrect);
  gDebug.msg("TIFFImageIFile::TIFFImageIFile(): exiting ctor",5);
}   

TIFFImageIFile::~TIFFImageIFile()
{
  int i;
  XTIFFClose(tif);
  //delete[] red;
  //delete[] blue;
  //delete[] green;
  if (hashTable!=NULL)
    delete[] hashTable;
  for (i=0;i<20;i++)
    if (rowCache[i]!=NULL)
      delete[] rowCache[i];
  gDebug.msg("TIFFImageIFile::~TIFFImageIFile(): exiting dtor",5);
}

// *********************************************************************

// Read Scanline and if not exists or problem dump error msg and exit
char TIFFImageIFile::cacheScanline(long row)
{
  int i;
  char entry;
  unsigned long min;

  if (hashTable[row] == -1)
  {
    min = 0;
    entry = 0;
    for (i=0;i<20;i++)
      if (timeStamp[i] < min )
      {
	min = timeStamp[i];
	entry = i;
      }
    lastTimeStamp++;
    
    if (rowNumber[entry] != -1)
      hashTable[rowNumber[entry]] = -1;
    timeStamp[entry] = lastTimeStamp;
    hashTable[row] = entry;
    rowNumber[entry] = row;
    
    if (TIFFReadScanline(tif, rowCache[entry], (unsigned int)row) == -1 ) 
    {
      gDebug.msg("TIFFImageIFile::cacheScanline(): problem reading file",1);
      setFailRBit();
    }
  }
  else
    entry = hashTable[row];
  
  return entry;
}

// *********************************************************************

AbstractPixel* TIFFImageIFile::getPixel(long x, long y)
{
  if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }

  unsigned char* buf;
  RGBPixel *pixel;

  buf = (unsigned char *) getRawPixel(x,y);

  pixel = new RGBPixel(red[*buf]/256,green[*buf]/256,blue[*buf]/256);
 
  delete buf;

  return (AbstractPixel*) pixel;            // Return pixel pointer
}

// *********************************************************************

AbstractPixel* TIFFImageIFile::getScanline(long row)
{
  if ((row < 0) || (row > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageIFile::getScanline(): scanline %ld out of bounds",row);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  unsigned char* buf;
  RGBPixel *line;
  int i,w;
  int k;

  w = getWidth();
   
  line = new RGBPixel[w];
  buf = (unsigned char *) getRawScanline(row);
  if (bits_per_sample == 8)
  {
    for (i=0;i<w;i++)
    {
      line[i].setRed(red[buf[i]]/256);
      line[i].setGreen(green[buf[i]]/256);
      line[i].setBlue(blue[buf[i]]/256);
    }
  }
  else if (bits_per_sample == 4)
  {
    k = 0;
    for (i=0;i<w-1;i+=2) 
    {
      line[i].setRed(red[(buf[k]>>4)]/256);
      line[i].setGreen(green[(buf[k]>>4)]/256);
      line[i].setBlue(blue[(buf[k]>>4)]/256);
      line[i+1].setRed(red[(buf[k]&0x0f)]/256);
      line[i+1].setGreen(green[(buf[k]&0x0f)]/256);
      line[i+1].setBlue(blue[(buf[k]&0x0f)]/256);
      k++;
    }
  }
  delete[] buf;
  
  return (AbstractPixel*) line;
}

// *********************************************************************

AbstractPixel* TIFFImageIFile::getRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[100];
    sprintf(dmsg,"TIFFImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  
  int w, h, n, i,k;
  unsigned char* buf;
  RGBPixel *rect;
  
  w = x2 - x1 + 1;
  h = y2 - y1 + 1;
  n = w*h;
  
  rect = new RGBPixel[n];
  
  buf = (unsigned char *) getRawRectangle(x1,y1,x2,y2);
  
  if (bits_per_sample == 8)
  {
    for (i=0;i<n;i++)
    {
      rect[i].setRed(red[buf[i]]/256);
      rect[i].setGreen(green[buf[i]]/256);
      rect[i].setBlue(blue[buf[i]]/256);
    }
  }
  else if (bits_per_sample == 4)
  {
    k = 0;
    for (i=0;i<n-1;i+=2)
    {
      rect[i].setRed(red[(buf[k]>>4)]/256);
      rect[i].setGreen(green[(buf[k]>>4)]/256);
      rect[i].setBlue(blue[(buf[k]>>4)]/256);
      rect[i+1].setRed(red[(buf[k]&0x0f)]/256);
      rect[i+1].setGreen(green[(buf[k]&0x0f)]/256);
      rect[i+1].setBlue(blue[(buf[k]&0x0f)]/256);
      k++;
    }
  }
  
  delete[] buf;
  
  return rect;
}

// *********************************************************************

void* TIFFImageIFile::getRawPixel(long x, long y)
{
  if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  
  unsigned char* line;
  unsigned char* pixel = new unsigned char;
  char entry;
  int k,r;
  
  entry = cacheScanline(y);
  line = rowCache[entry];
  if (bits_per_sample == 8)
    *pixel = line[x]; 
  else if (bits_per_sample == 4)   /* return value in bits 0-3 */
  {
    k = x/2;
    r = x % 2;
    if (r==0)
      *pixel = line[k] >> 4;
    else
      *pixel = line[k] & 0x0f;
  }
  
  return pixel;
}

// *********************************************************************
// Note: This is the only function which will return raw RGB TIFF scanlines
// until I have time to rewrite portions of the Image library
// BGM 25 Sept 1997
void* TIFFImageIFile::getRawScanline(long row)
{
  if ((row < 0) || (row > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageIFile::getRawScanline(): scanline %ld out of bounds",row);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }

  if (getPhotometric() == DRG_REAL_RGB) // return a RGB scanline
  {
    unsigned char* retline = new unsigned char[3 * getWidth()];
    if (TIFFReadScanline(tif, retline, row) == -1)
    {
      gDebug.msg("TIFFImageIFile::getRawScanline(): problem reading file", 1);
      setFailRBit();
    }
    return retline;
  }

  unsigned char* retline = new unsigned char[getWidth()];
  unsigned char* line;
  char entry;
  
  entry = cacheScanline(row);
  line = rowCache[entry];
  memcpy(retline,line,getWidth()); 
  
  return retline;
}

// *********************************************************************

void* TIFFImageIFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[120];
    sprintf(dmsg,"TIFFImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  
  long i,k;
  long rw, rh;
  unsigned char *tptr, *sptr;
  unsigned char* rect = new unsigned char[(x2-x1+1)*(y2-y1+1)];
  unsigned char* line;
  char entry;
  
  rh = y2-y1+1;
  rw = x2-x1+1; 
    
  k=0;
  for (i=y1;i<=y2;i++)
  {
    entry = cacheScanline(i);
    line = rowCache[entry];
    sptr = &(line[x1]);
    tptr = &(rect[k]);
    memcpy(tptr,sptr,rw);
    k+=rw;
  } 
  return rect;
}

void TIFFImageIFile::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
  if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageIFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return;
  }

  unsigned char* pixel;
 
  pixel = (unsigned char *) getRawPixel(x,y);

  pix->Red   = red[*pixel]/256;
  pix->Green = green[*pixel]/256;
  pix->Blue  = blue[*pixel]/256;
  
  delete pixel;
}

void TIFFImageIFile::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
  if ((row < 0) || (row > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageIFile::getRawRGBScanline(): scanline %ld out of bounds",row);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return;
  }
 
  unsigned char* line;
  int i, w;
  int k;

  w = getWidth();

  line = (unsigned char *) getRawScanline(row);

  if (bits_per_sample == 8)
  {
    for (i=0;i<w;i++)
    {
      pixbuf[i].Red =   red[line[i]]/256;
      pixbuf[i].Green = green[line[i]]/256;
      pixbuf[i].Blue =  blue[line[i]]/256;
    }
  }
  else if (bits_per_sample == 4)
  {
    k = 0;
    for (i=0;i<w-1;i+=2);
    {
      pixbuf[i].Red =   red[(line[k] >> 4)]/256;
      pixbuf[i].Green = green[(line[k] >> 4)]/256;
      pixbuf[i].Blue =  blue[(line[k] >> 4)]/256;
      pixbuf[i+1].Red =   red[(line[k]&0x0f)]/256;
      pixbuf[i+1].Green = green[(line[k]&0x0f)]/256;
      pixbuf[i+1].Blue =  blue[(line[k]&0x0f)]/256;
      k++;
    }
  }

  delete[] line;
}

void TIFFImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                        RawRGBPixel* pixbuf)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[120];
    sprintf(dmsg,"TIFFImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return;
  }

  unsigned char* line;
  int i, w,h,l;
  int k;
  
  w = (x2-x1+1);
  h = (y2-y1+1);
  l = w*h;

  line = (unsigned char *) getRawRectangle(x1,y1,x2,y2);

  if (bits_per_sample == 8)
  {
    for (i=0;i<l;i++)
    {
      pixbuf[i].Red =   red[line[i]]/256;
      pixbuf[i].Green = green[line[i]]/256;
      pixbuf[i].Blue =  blue[line[i]]/256;
    }
  }
  else if (bits_per_sample == 4)
  {
    k = 0;
    for (i=0;i<l-1;i+=2);
    {
      pixbuf[i].Red =   red[(line[k] >> 4)]/256;
      pixbuf[i].Green = green[(line[k] >> 4)]/256;
      pixbuf[i].Blue =  blue[(line[k] >> 4)]/256;
      pixbuf[i+1].Red =   red[(line[k]&0x0f)]/256;
      pixbuf[i+1].Green = green[(line[k]&0x0f)]/256;
      pixbuf[i+1].Blue =  blue[(line[k]&0x0f)]/256;
      k++;
    }
  }
  
  delete[] line;
}

// *********************************************************************

Pallette* TIFFImageIFile::getPallette(void)
{
  RGBPallette* pallette = new RGBPallette(256);
  RGBPixel* rgb_pixel = new RGBPixel;
  for(int x=0; x<256; x++)
  {
    rgb_pixel->setRed(red[x]/256);
    rgb_pixel->setGreen(green[x]/256);
    rgb_pixel->setBlue(blue[x]/256);

    pallette->setEntry(x,rgb_pixel);
  }
  delete rgb_pixel;
  return (Pallette*)pallette; 
}

TIFF* TIFFImageIFile::getTif(void)
{
  return tif;
}

bool TIFFImageIFile::getTag(int tag, unsigned long* val)
{
  gDebug.msg("TIFFImageIFile::getTag(): unsigned long* version called",5);
  return (TIFFGetField(tif,tag,val) == 1);
}

bool TIFFImageIFile::getTag(int tag, unsigned short* val)
{
  gDebug.msg("TIFFImageIFile::getTag(): unsigned short* version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, float* val)
{
  gDebug.msg("TIFFImageIFile::getTag(): float* version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, double* val)
{
  gDebug.msg("TIFFImageIFile::getTag(): double* version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, double** val)
{
  gDebug.msg("TIFFImageIFile::getTag(): double** version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, char** val)
{
  gDebug.msg("TIFFImageIFile::getTag(): char** version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, unsigned long** val)
{
  gDebug.msg("TIFFImageIFile::getTag(): unsigned long** version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, unsigned short** val)
{
  gDebug.msg("TIFFImageIFile::getTag(): unsigned short** version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, float** val)
{
  gDebug.msg("TIFFImageIFile::getTag(): float** version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getTag(int tag, unsigned char*** val)
{
  gDebug.msg("TIFFImageIFile::getTag(): unsigned char*** version called",5);
  return (TIFFGetField(tif,tag,val)==1);
}

bool TIFFImageIFile::getOptimalRectangle(int &w, int &h)
{
  w = getWidth();
  h = 0;
  return true;
}
