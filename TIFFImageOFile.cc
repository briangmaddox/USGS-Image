//  TIFFImageOFile.cc                         11 Jul 94
//  Author:  Greg Martin and Duane Lascoe  SES, MCMC, USGS
//  Updated for enhanced options Brian Maddox SES, MCMC, USGS Aug 1997
//  Implements member functions for the class TIFFImageOFile 

#include "Image/TIFFImageOFile.h"
#include "tiff/tiffio.h"
#include "geotiff/xtiffio.h"
#include "Image/RGBPixel.h"
#include "Image/AbstractPixel.h"
#include "Image/ImageCommon.h"
#include "tiff/tiff.h"
#include "Image/Pallette.h"
#include "Image/RGBPallette.h"
#include <stdlib.h>
#include "Image/ImageDebug.h"
#include <stdio.h>

// Various constructors for TIFF functionality
// Constructor for TIFFImageOFile.h
TIFFImageOFile::TIFFImageOFile(const char* tfilename, long tw, long th)
  : ImageOFile(tfilename,tw,th)
{
  if ((tif = XTIFFOpen(tfilename, "w")) == NULL)
  {
    gDebug.msg("TIFFImageOFile::TIFFImageOFile(c*,l,l): can't open file",1);
    setNoDataBit();
  }
  else
  {
    TIFFSetField(tif,TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,tw);
    TIFFSetField(tif,TIFFTAG_IMAGELENGTH,th);
    TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif,TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif,TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
    TIFFSetField(tif,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
    TIFFSetField(tif,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    TIFFSetField(tif,TIFFTAG_XRESOLUTION, 250.0);
    TIFFSetField(tif,TIFFTAG_YRESOLUTION, 250.0);
    TIFFSetField(tif,TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(tif,TIFFTAG_ROWSPERSTRIP,1);
    setPhotometric(DRG_RGB);
    color_count = 0;
    red    = new unsigned short[256];
    green  = new unsigned short[256];
    blue   = new unsigned short[256];
    buffer = new unsigned char[tw];
  }
  setRandomAccessFlags(wrow);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(1);
  gDebug.msg("TIFFImageOFile::TIFFImageOFile(c*,l,l): exiting ctor",5);
}   

TIFFImageOFile::TIFFImageOFile(const char* tfilename, long tw, 
                               long th, int comp)
  : ImageOFile(tfilename,tw,th)
{
  if ((tif = XTIFFOpen(tfilename, "w")) == NULL)
  {
    gDebug.msg("TIFFImageOFile::TIFFImageOFile(c*,l,l,i): can't open file",1);
    setNoDataBit();
  }
  else
  {
    TIFFSetField(tif,TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,tw);
    TIFFSetField(tif,TIFFTAG_IMAGELENGTH,th);
    TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif,TIFFTAG_BITSPERSAMPLE, 8);
    switch (comp)
    {
    case 0:
      TIFFSetField(tif,TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
      break;
    case 1:
      TIFFSetField(tif,TIFFTAG_COMPRESSION, COMPRESSION_LZW);
      break;
    case 2:
      TIFFSetField(tif,TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      break;
    default:
      TIFFSetField(tif,TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
    }
    TIFFSetField(tif,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
    TIFFSetField(tif,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    TIFFSetField(tif,TIFFTAG_XRESOLUTION, 250.0);
    TIFFSetField(tif,TIFFTAG_YRESOLUTION, 250.0);
    TIFFSetField(tif,TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(tif,TIFFTAG_ROWSPERSTRIP,1);
    setPhotometric(DRG_RGB);
    color_count = 0;
    red    = new unsigned short[256];
    green  = new unsigned short[256];
    blue   = new unsigned short[256];
    buffer = new unsigned char[tw];
  }
  setRandomAccessFlags(wrow);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(1);
  gDebug.msg("TIFFImageOFile::TIFFImageOFile(c*,l,l,i): exiting ctor",5);
}   

// This is just kinda here as an intermediate step so I can write RGB tiffs
// before I do the Image Library rewrite BGM
TIFFImageOFile::TIFFImageOFile(const char* tfilename, long tw, long th, 
			       int samples_pixel, int bits_sample, int comp,
			       int photo_metric)
  : ImageOFile(tfilename, tw, th)
{
  if ((tif = XTIFFOpen(tfilename, "w")) == NULL)
  {
    gDebug.msg("TIFFImageOFile::TIFFImageOFile(c*,l,l,i): can't open file",1);
    setNoDataBit();
  }
  else
  {
    TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, tw);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, th);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples_pixel);
    if (photo_metric == PHOTOMETRIC_RGB)
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits_sample, bits_sample, 
		   bits_sample);
    else
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits_sample);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, comp);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photo_metric);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, 250.0);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, 250.0);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    setPhotometric(DRG_REAL_RGB);
    color_count = 0;
    red = blue = green = NULL;
    buffer = NULL;
  }
  setRandomAccessFlags(wrow);
  setBitsPerSample(bits_sample);
  setSamplesPerPixel(samples_pixel);
  gDebug.msg("TIFFImageOFile::TIFFImageOFile(c*,l,l,i,i,i,i): exiting ctor",5);
}



TIFFImageOFile::~TIFFImageOFile()
{
  if (getPhotometric() != DRG_REAL_RGB)
    if (TIFFSetField(tif,TIFFTAG_COLORMAP,red,green,blue)==0)
      gDebug.msg("TIFFImageIFile::~TIFFImageOFile(): problem writing file",1);
  if (tif!=NULL) XTIFFClose(tif);
  if (red != NULL)
    delete[] red;
  if (green != NULL)
    delete[] green;
  if (blue != NULL)
    delete[] blue;
  if (buffer != NULL)
    delete[] buffer;
  gDebug.msg("TIFFImageOFile::~TIFFImageOFile(): exiting dtor",5);
}

// *********************************************************************

void TIFFImageOFile::updateColormap(RGBPixel* p)
{
  long w; 
  RGBPixel* pixel;
  unsigned char ind;
  unsigned short tred = 0;
  unsigned short tgreen = 0;
  unsigned short tblue = 0;
  static int found = 0;
  unsigned char loop;
  long unsigned int colormatch;
  long wide;

  wide = getWidth();
 

  for(w=0; w<wide; w++)
  {
    pixel = &(p[w]);                   // get first pixel
    ind = 0;
    found = 0;
    tred = pixel->getRed()*256;      // get tiff red of first pixel
    tgreen = pixel->getGreen()*256;  // get tiff green of first pixel
    tblue = pixel->getBlue()*256;    // get tiff blue of first pixel
       
    while ((ind < color_count ) && (!found)) // look for the color in map
    {
      if ((tred  == red[ind]) &&    // test to see if this color
	  (tgreen == green[ind]) && // matches the one in the
          (tblue  == blue[ind]))    // color map at position [ind]
	found = 1;
      ind++;
    }
    if (found)            // color is in the color table
    {
      buffer[w] = --ind;   // put offset into TIFF buffer
    }
    else
      if (ind < 256)   // color not in table and table not full
      {
	red[ind] = tred;      // set red colormap value
	green[ind] = tgreen;  // set green colormap value
	blue[ind] = tblue;    // set blue color map value
	color_count += 1;     // add one to amount of colors
	buffer[w] = ind;
      }
      else               // color not in table and table full
      {
	ind = 0;
	loop = 0;
	// get initial comparison
	colormatch = (tred-red[loop])*(tred-red[loop])+
	  (tgreen-green[loop])*(tgreen-green[loop])+
	  (tblue-blue[loop])*(tblue-blue[loop]);
	  
	  for (loop = 1; loop < color_count; loop++)
	  {  // find the closest color in the table for a match
	    // if a closer match then choose it over the last one.
	    if (((tred-red[loop])*(tred-red[loop])+
	    (tgreen-green[loop])*(tgreen-green[loop])+
	    (tblue-blue[loop])*(tblue-blue[loop])) < (long) colormatch)
	    {
	      colormatch = (tred-red[loop])*(tred-red[loop])+
		(tgreen-green[loop])*(tgreen-green[loop])+
		(tblue-blue[loop])*(tblue-blue[loop]);
		ind = loop; // set current color index to current match
	    }
	  }
	  buffer[w] = ind;  // set buffer position to point at closest match
      }
    // write out current colormap at each step
    /*
      if (TIFFSetField(tif,TIFFTAG_COLORMAP,red,green,blue)==-1) 
      {
      gDebug.msg("TIFFImageIFile::updateColormap(): problem writing file",1);
      setFailWBit();
      }
      */
  }
}

void TIFFImageOFile::writeColormap(void)
{
  if (TIFFSetField(tif, TIFFTAG_COLORMAP, red, green, blue) == 0)
  {
    gDebug.msg("TIFFImageIFile::writeColormap(): problem writing file",1);
    setFailWBit();
  }
}


// *********************************************************************

void TIFFImageOFile::putPixel(AbstractPixel*, long, long) 
{
  gDebug.msg("TIFFImageOFile::putPixel(): Sorry, not implemented with TIFF",1);
  setFailWBit();
}

// *********************************************************************

void TIFFImageOFile::putScanline(AbstractPixel* p, long row)
{
  if ((row < 0) || (row > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageOFile::putScanline(): scanline %ld out of bounds",
	    row);
    gDebug.msg(dmsg,1);
    setFailWBit();
    return;
  }
  
  updateColormap((RGBPixel*)p);   // updates the colormap and fills the buffer
  if (TIFFWriteScanline(tif, buffer, row, 0) == -1)
  {
    gDebug.msg("TIFFImageOFile::putScanline(): problem writing file",1);
    setFailWBit();
  }
}

// *********************************************************************

void TIFFImageOFile::putRectangle(AbstractPixel*, long, long, long, long)
{
  gDebug.msg("TIFFImageOFile::putRectangle(): Sorry, not implemented with TIFF", 1);
  setFailWBit();
}

// *********************************************************************

void TIFFImageOFile::putRawPixel(void*, long, long)
{
  gDebug.msg("TIFFImageOFile::putRawPixel(): Sorry, not implemented with TIFF",
	     1);
  setFailWBit();
}

// *********************************************************************

void TIFFImageOFile::putRawScanline(void* p,long row)
{
  if ((row < 0) || (row > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"TIFFImageOFile::putRawScanline(): scanline %ld out of bounds",row);
    gDebug.msg(dmsg,1);
    setFailWBit();
    return;
  }
  
  unsigned char* foo = (unsigned char*)p;
  if (TIFFWriteScanline(tif,foo,row,0) == -1)
  {
    gDebug.msg("TIFFImageOFile::putRawScanline(): problem writing file",1);
    setFailWBit();
  }
}

// *********************************************************************

void TIFFImageOFile::putRawRectangle(void*,long,long,long,long)
{
  gDebug.msg("TIFFImageOFile::putRawRectangle(): Sorry, not implemented with TIFF",1);
  setFailWBit();
}

// *********************************************************************

Pallette* TIFFImageOFile::getPallette(void)
{
  RGBPallette* pallette = new RGBPallette(256);
  RGBPixel* rgb_pixel = new RGBPixel;
  
  if(TIFFGetField(tif,TIFFTAG_COLORMAP,&red,&green,&blue) == 0)
  {
    gDebug.msg("TIFFImageOFile::getPallette(): can't read pallette",1);
    setFailRBit();
    return NULL;
  }
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

// *********************************************************************

void TIFFImageOFile::setPallette(Pallette* p)
{
  int x;
  RGBPallette* pal = (RGBPallette*)p;
  int no_colors;
  
  no_colors = pal->getNoEntries();

  for (x=0; x<no_colors; x++)
  {
    red[x] = (*pal)[x].getRed()*256; 
    green[x] = (*pal)[x].getGreen()*256;
    blue[x] = (*pal)[x].getBlue()*256;
  }
  if(TIFFSetField(tif,TIFFTAG_COLORMAP,red,green,blue) == 0)
  {
    gDebug.msg("TIFFImageOFile::setPallette(): problem writing file",1);
    setFailWBit();
  }
}


bool TIFFImageOFile::setTag(int tag, unsigned long val)
{
  gDebug.msg("TIFFImageOFile::setTag(): unsigned long version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, unsigned short val)
{
  gDebug.msg("TIFFImageOFile::setTag(): unsigned short version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, float val)
{
  gDebug.msg("TIFFImageOFile::setTag(): float version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, char* val)
{
  gDebug.msg("TIFFImageOFile::setTag(): char* version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, unsigned long* val)
{
  gDebug.msg("TIFFImageOFile::setTag(): unsigned long* version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, unsigned short* val)
{
  gDebug.msg("TIFFImageOFile::setTag(): unsigned short* version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, float* val)
{
  gDebug.msg("TIFFImageOFile::setTag(): float* version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, double* val)
{
  gDebug.msg("TIFFImageOFile::setTag(): double* version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::setTag(int tag, unsigned char** val)
{
  gDebug.msg("TIFFImageOFile::setTag(): char** version called",5);
  return (TIFFSetField(tif,tag,val)==1);
}

bool TIFFImageOFile::getOptimalRectangle(int &w, int &h)
{
  w = 0;
  h = 0;
  return false;
}
