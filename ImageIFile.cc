//  ImageIFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class ImageIFile 


#include "Image/ImageIFile.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"

ImageIFile::ImageIFile(void):
   ImageFile()
{
   gDebug.msg("ImageIFile::ImageIFile(): exiting ctor",5);
}

ImageIFile::ImageIFile(const char* fn)
{
   setFileName(fn);
   gDebug.msg("ImageIFile::ImageIFile(c*): exiting ctor",5);
}

ImageIFile::~ImageIFile()
{
   gDebug.msg("ImageIFile::~ImageIFile(): exiting dtor",5);
}


void ImageIFile::setPallette(Pallette*)
{
   setFailWBit();
   gDebug.msg("ImageIFile::setPallette(): can't set pallette",1);
}
    

void ImageIFile::putPixel(AbstractPixel*, long, long)
{
   setFailWBit();
   gDebug.msg("ImageIFile::putPixel(): can't put pixel",1);
}

void ImageIFile::putScanline(AbstractPixel*, long)
{
   setFailWBit();
   gDebug.msg("ImageIFile::putScanline(): can't put scanline",1);
}

void ImageIFile::putRectangle(AbstractPixel*, long, long, long, long)
{
   setFailWBit();
   gDebug.msg("ImageIFile::putRectangle(): can't put rectangle",1);
}

void ImageIFile::putRawPixel(void*,long, long)
{
   setFailWBit();
   gDebug.msg("ImageIFile::putRawPixel(): can't put pixel",1);
}

void ImageIFile::putRawScanline(void*,long)
{
   setFailWBit();
   gDebug.msg("ImageIFile::putRawScanline(): can't put scanline",1);
}

void ImageIFile::putRawRectangle(void*,long,long,long,long)
{
   setFailWBit();
   gDebug.msg("ImageIFile::putRawRectangle(): can't put rectangle",1);
}
