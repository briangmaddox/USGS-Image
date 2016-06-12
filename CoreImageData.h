// CoreImageData.h                   13 Jul 95
// Author: Greg Martin, SES, MCMC, USGS

// Defines the class CoreImageData

#ifndef _COREIMAGEDATA_H_
#define _COREIMAGEDATA_H_

#include "Image/ImageData.h"


class CoreImageData : public ImageData
{
   unsigned char* data;
   int            quantum;        // 1 for pallette or grey, 3 for RGB

  public:
  
   CoreImageData(long w, long h,unsigned char p=DRG_RGB,int q=3);
   virtual ~CoreImageData();

   inline int        getQuantum(void);

   virtual Pallette* getPallette(void);
   virtual void      setPallette(Pallette* p);

   virtual void*  getRawPixel(long x, long y);
   virtual void*  getRawScanline(long row);
   virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);
   virtual void   putRawPixel(void* p, long x,long y);
   virtual void   putRawScanline(void* p,long row);
   virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2);

   virtual AbstractPixel* getPixel(long x, long y);
   virtual AbstractPixel* getScanline(long row);
   virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);
   virtual void   putPixel(AbstractPixel* p, long x, long y);
   virtual void   putScanline(AbstractPixel* s, long row);
   virtual void   putRectangle(AbstractPixel* r, long x1, long y1, long x2, long y2);

   virtual void getRawRGBPixel(long x, long y, RawRGBPixel* pix);
   virtual void getRawRGBScanline(long row, RawRGBPixel* pixbuff);
   virtual void getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                   RawRGBPixel* pixbuf);

   virtual bool getOptimalRectangle(int &w, int &h);
};

#endif
