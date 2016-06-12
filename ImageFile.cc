//  ImageFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class ImageFile 


#include "Image/ImageFile.h"
#include "Image/ImageDebug.h"
#include <stdlib.h>
#include <string.h>
#include <iostream.h>


ImageFile::ImageFile(void):
   ImageData()
{
   filename = NULL;
   gDebug.msg("ImageFile::ImageFile(): exiting ctor",5);
}

ImageFile::ImageFile(const char* fn, long w, long l)
{
   setWidth(w);
   setHeight(l);
   filename = new char[strlen(fn)+1];
   strcpy(filename,fn);
   gDebug.msg("ImageFile::ImageFile(c*,l,l): exiting ctor",5);
}

ImageFile::ImageFile(const char* fn)
{
   filename = new char[strlen(fn)+1];
   strcpy(filename,fn);
   gDebug.msg("ImageFile::ImageFile(c*): exiting ctor",5);
}
    
ImageFile::~ImageFile()
{
   if (filename != NULL)
      delete filename;
   gDebug.msg("ImageFile::~ImageFile(): exiting dtor",5);
}


void ImageFile::setFileName(const char* fn)
{
    if (filename != NULL)
       delete filename;
   filename = new char[strlen(fn)+1];
   strcpy(filename,fn);
   gDebug.msg("ImageFile::setFileName(): filename set",5);
}

char* ImageFile::getFileName(void)
{
    char* foo = new char[strlen(filename)+1];
    strcpy(foo,filename);
    return foo;
}
