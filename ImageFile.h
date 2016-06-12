// ImageFile.h                      11 Jul 94
// Author: Greg Martin, SES, MCMC, USGS

// Defines the abstract class ImageFile


#ifndef _IMAGEFILE_H_
#define _IMAGEFILE_H_

#include "Image/ImageData.h"
#include "Image/Pallette.h"
#include "Image/ImageCommon.h"


class ImageFile : public ImageData
{
   char* filename;

  public:
  
   ImageFile();
   ImageFile(const char* fn, long w, long l);  // create
   ImageFile(const char* fn);                  // open existing
   virtual ~ImageFile();
   void  setFileName(const char* fn);
   char* getFileName();

//  Functions are again pure virtuals...this class is to be abstract

   virtual Pallette* getPallette(void)=0;
   virtual void      setPallette(Pallette* p)=0;

// get/put functions use column-major order, ie (x,y) == (column,row)

   virtual AbstractPixel* getPixel(long x, long y)=0;
   virtual AbstractPixel* getScanline(long row)=0;
   virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2)=0;
   virtual void   putPixel(AbstractPixel* p, long x, long y)=0;
   virtual void   putScanline(AbstractPixel* s, long row)=0;
   virtual void   putRectangle(AbstractPixel* r, long x1, long y1, long x2, long y2)=0;

   virtual void*  getRawPixel(long x, long y)=0;
   virtual void*  getRawScanline(long row)=0;
   virtual void*  getRawRectangle(long x1, long y1, long x2, long y2)=0;
   virtual void   putRawPixel(void* p, long x,long y)=0;
   virtual void   putRawScanline(void* p,long row)=0;
   virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2)=0;
   virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix)=0;
   virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf)=0; 
   virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                     RawRGBPixel* pixbuf)=0;
   
   virtual bool   getOptimalRectangle(int &w, int &h)=0;
};


#endif
