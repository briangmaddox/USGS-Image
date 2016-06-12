//  ImageIOFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class ImageIOFile 


#include "Image/ImageIOFile.h"
#include "Image/ImageDebug.h"

ImageIOFile::ImageIOFile(void):
   ImageFile()
{
   gDebug.msg("ImageIOFile::ImageIOFile(): exiting ctor",5);
} 

ImageIOFile::ImageIOFile(const char* fn):
   ImageFile(fn)
{
   gDebug.msg("ImageIOFile::ImageIOFile(c*): exiting ctor",5);
}


ImageIOFile::~ImageIOFile()
{
   gDebug.msg("ImageIOFile::~ImageIOFile(): exiting dtor",5);
}
