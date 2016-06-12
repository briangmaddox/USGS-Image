//  RGBImageIFile.h                   11 Jul 94
//  Author:  Greg Martin and Loren Bonebrake, SES, MCMC, USGS

//  Defines the class RGBImageIFile

//  This class handles the reading of raw RGB raster files.

#ifndef _RGBIMAGEIFILE_H_
#define _RGBIMAGEIFILE_H_

#include <fstream.h>
#include "Image/ImageIFile.h"
#include "Image/Pallette.h"


class RGBImageIFile : public ImageIFile
{
  ifstream* Stream;
  char*     headerFile;

  public:

   RGBImageIFile(const char* fn, const char* hdr);
   RGBImageIFile(const char* fn);
   virtual ~RGBImageIFile();

// Standard ImageIFile interface

// These get functions allocate memory, so this memory
// must be deleted by the user. 

   virtual Pallette* getPallette(void);

//  Note:  The get/put functions all use column-major addressing (ie.
//         (x,y) means (column, row).  Also, the get functions

   virtual AbstractPixel* getPixel(long x, long y);
   virtual AbstractPixel* getScanline(long row);
   virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);

   virtual void*  getRawPixel(long x, long y);
   virtual void*  getRawScanline(long row);
   virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);

   virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix);
   virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf); 
   virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                     RawRGBPixel* pixbuf);

   virtual bool getOptimalRectangle(int &w, int &h);

  protected:
 
   void readHeader(void);
};

#endif
