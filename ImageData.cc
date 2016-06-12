//  ImageData.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class ImageData 


#include "Image/ImageData.h"
#include "Image/AbstractPixel.h"
#include "Image/ImageDebug.h"
#include <iostream.h>


ImageData::ImageData(void)
{
    Width = 0;
    Height = 0;
    State = goodbit;
    bitsPerSample = 0;
    samplesPerPixel = 0;
    _hasPallette = false;
    RandomAccessFlags = none;
    gDebug.msg("ImageData::ImageData(): exiting ctor",5);
}

ImageData::ImageData(long w, long h)
{
    Width = w;
    Height = h;
    State = goodbit;
    bitsPerSample = 0;
    samplesPerPixel = 0;
    _hasPallette = false;
    RandomAccessFlags = none;
    gDebug.msg("ImageData::ImageData(long,long): exiting ctor",5);
}

ImageData::~ImageData()
{
    gDebug.msg("ImageData::~ImageData(): exiting dtor",5);
}

void ImageData::setWidth(long w)
{
    Width = w;
}

void ImageData::setHeight(long h)
{
    Height = h;
}

void ImageData::setBitsPerSample(int b)
{
    bitsPerSample = b;
}

void ImageData::setSamplesPerPixel(int s)
{
    samplesPerPixel = s;
}

void ImageData::setPhotometric(unsigned char p)
{
    Photometric = p;
}

long ImageData::getWidth(void)
{
    return Width;
}

long ImageData::getHeight(void)
{
    return Height;
}

int ImageData::getBitsPerSample(void)
{
    return bitsPerSample;
}

int ImageData::getSamplesPerPixel(void)
{
    return samplesPerPixel;
}

unsigned char ImageData::getPhotometric(void)
{
    return Photometric;
}

void ImageData::setHasPallette()
{
   _hasPallette = true;
}

void ImageData::unsetHasPallette()
{
   _hasPallette = false;
}

bool ImageData::hasPallette()
{
   return _hasPallette;
}

bool ImageData::good(void)
{
   return (State == goodbit);
}

bool ImageData::bad(void)
{
   return (State != goodbit);
}

bool ImageData::noData(void)
{
   return ((State & nodatabit) != 0);
}

bool ImageData::failedRead(void)
{
   return ((State & failrbit) != 0);
}

bool ImageData::failedWrite(void)
{
   return ((State & failwbit) != 0);
}

void ImageData::clear(void)
{
   State = goodbit;
}

void ImageData::setNoDataBit(void)
{
   State = State | nodatabit;
}

void ImageData::setFailRBit(void)
{
   State = State | failrbit;
}

void ImageData::setFailWBit(void)
{
   State = State | failwbit;
}

void ImageData::setRandomAccessFlags(unsigned int f)
{
   RandomAccessFlags = f;
}

unsigned int ImageData::getRandomAccessFlags(void)
{
   return RandomAccessFlags;
}

bool ImageData::canReadPixel(void)
{
   return ((RandomAccessFlags & rpixel)!=0);
}

bool ImageData::canReadScanline(void)
{
   return ((RandomAccessFlags & rrow)!=0);
}

bool ImageData::canReadRectangle(void)
{
   return ((RandomAccessFlags & rrect)!=0);
}

bool ImageData::canWritePixel(void)
{
   return ((RandomAccessFlags & wpixel)!=0);
}

bool ImageData::canWriteScanline(void)
{
   return ((RandomAccessFlags & wrow)!=0);
}

bool ImageData::canWriteRectangle(void)
{
   return ((RandomAccessFlags & wrect)!=0);
}

