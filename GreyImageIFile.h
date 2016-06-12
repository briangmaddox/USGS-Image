//  GreyImageIFile.h                   11 Jul 94
//  Author:  Greg Martin, SES, MCMC, USGS

//  Defines the class GreyImageIFile

//  This class handles the reading of raw greyscale raster files


#ifndef _GREYIMAGEIFILE_H_
#define _GREYIMAGEIFILE_H_

#include <fstream.h>
#include "Image/ImageIFile.h"
#include "Image/Pallette.h"


class GreyImageIFile : public ImageIFile
{
  ifstream* Stream;
  char*     headerFile;

  public:

   GreyImageIFile(const char* fn, const char* hdr);
   GreyImageIFile(const char* fn);
   virtual ~GreyImageIFile();

// Standard ImageIFile interface

// These get functions allocate memory, so the user
// must delete this memory.


   virtual Pallette* getPallette(void);

// get/put functions use column-major order, ie. (x,y) == (column,row)

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
