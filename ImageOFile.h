//  ImageOFile.h                       11 Jul 94
//  Author:  Greg Martin, SES, MCMC, USGS

//  Defines the abstract class ImageOFile.h
//  This class forces write-only behaviour


#ifndef _IMAGEOFILE_H_
#define _IMAGEOFILE_H_

#include "Image/ImageFile.h"
#include "Image/Pallette.h"


class ImageOFile : public ImageFile
{
  public:
  
   ImageOFile();
   ImageOFile(const char* fn, long w, long l);  // create
   virtual ~ImageOFile();

// Virtual functions to outline behaviour 

   virtual Pallette* getPallette(void)=0;
   virtual void      setPallette(Pallette* p)=0;

// get/put functions use column major order, ie (x,y) == (column,row)

   virtual AbstractPixel* getPixel(long x, long y);
   virtual AbstractPixel* getScanline(long row);
   virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);
   virtual void   putPixel(AbstractPixel* p, long x, long y)=0;
   virtual void   putScanline(AbstractPixel* s, long row)=0;
   virtual void   putRectangle(AbstractPixel* r, long x1, long y1, long x2, long y2)=0;

   virtual void*  getRawPixel(long x, long y);
   virtual void*  getRawScanline(long row);
   virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);
   virtual void   putRawPixel(void* p, long x,long y)=0;
   virtual void   putRawScanline(void* p,long row)=0;
   virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2)=0;

   virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix);
   virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf); 
   virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                     RawRGBPixel* pixbuf);

   virtual bool   getOptimalRectangle(int &w, int &y)=0;
};

#endif
