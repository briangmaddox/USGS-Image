//  ImageIFile.h                 11 Jul 94
//  Author: Greg Martin, SES, MCMC, USGS

//  Defines the abstract class ImageIFile
//  This class forces read only access

#ifndef _IMAGEIFILE_H_
#define _IMAGEIFILE_H_

#include "Image/ImageFile.h"
#include "Image/Pallette.h"


class ImageIFile : public ImageFile
{

  public:
  
   ImageIFile();
   ImageIFile(const char* fn);                  // open existing
   virtual ~ImageIFile();

// get* functions are pure virtuals
// put* functions get defined here (as errors)
// get/put functions use column-major order, ie (x,y) == (column,row)

   virtual Pallette* getPallette(void)=0;
   virtual void      setPallette(Pallette* p);

   virtual AbstractPixel* getPixel(long x, long y)=0;
   virtual AbstractPixel* getScanline(long row)=0;
   virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2)=0;
   virtual void   putPixel(AbstractPixel* p, long x, long y);
   virtual void   putScanline(AbstractPixel* s, long row);
   virtual void   putRectangle(AbstractPixel* r, long x1, long y1, long x2, long y2);

   virtual void*  getRawPixel(long x, long y)=0;
   virtual void*  getRawScanline(long row)=0;
   virtual void*  getRawRectangle(long x1, long y1, long x2, long y2)=0;
   virtual void   putRawPixel(void* p, long x,long y);
   virtual void   putRawScanline(void* p,long row);
   virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2);

   virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix)=0;
   virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf)=0; 
   virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                     RawRGBPixel* pixbuf)=0;
  
   virtual bool   getOptimalRectangle(int &w, int &h)=0;
};

#endif
