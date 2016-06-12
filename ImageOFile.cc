//  ImageOFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class ImageOFile 


#include "Image/ImageOFile.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdlib.h>

ImageOFile::ImageOFile(void):
   ImageFile()
{
   gDebug.msg("ImageOFile::ImageOFile(): exiting ctor",5);
}

ImageOFile::ImageOFile(const char* fn, long w, long l)
{
   setFileName(fn);
   setWidth(w);
   setHeight(l);
   gDebug.msg("ImageOFile::ImageOFile(c*,l,l): exiting ctor",5);
}

ImageOFile::~ImageOFile()
{
   gDebug.msg("ImageOFile::~ImageOFile(): exiting dtor",5);
}

AbstractPixel* ImageOFile::getPixel(long,long)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getPixel(): can't get pixel",1);
   return NULL;
}

AbstractPixel* ImageOFile::getScanline(long)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getScanline(): can't get scanline",1);
   return NULL;
}

AbstractPixel* ImageOFile::getRectangle(long,long,long,long)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getRectangle(): can't get rectangle",1);
   return NULL;
}

void* ImageOFile::getRawPixel(long,long)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getRawPixel(): can't get pixel",1);
   return NULL;
}

void* ImageOFile::getRawScanline(long)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getRawScanline(): can't get scanline",1);
   return NULL;
}

void* ImageOFile::getRawRectangle(long,long,long,long)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getRawRectangle(): can't get rectangle",1);
   return NULL;
}

void ImageOFile::getRawRGBPixel(long,long,RawRGBPixel*)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getRawRGBPixel(): can't get pixel",1);
}

void ImageOFile::getRawRGBScanline(long,RawRGBPixel*)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getRawRGBScanline(): can't get scanline",1);
}

void ImageOFile::getRawRGBRectangle(long,long,long,long,RawRGBPixel*)
{
   setFailRBit();
   gDebug.msg("ImageOFile::getRawRGBRectangle(): can't get rectangle",1);
}
