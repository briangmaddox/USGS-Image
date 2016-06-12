// CorePalletteImageData.h               10 Aug 95
// Author: Greg Martin, SES, MCMC, USGS

// Defines the class CorePalletteImageData

#ifndef _COREPALLETTEIMAGEDATA_H_
#define _COREPALLETTEIMAGEDATA_H_

#include "Image/CoreImageData.h"
#include "Image/Pallette.h"
#include "Image/RGBPixel.h"
#include "Image/GreyPixel.h"


class CorePalletteImageData : public CoreImageData
{
   Pallette *pal;

  public:
  
   CorePalletteImageData(long w, long h,unsigned char p=DRG_RGB);
   virtual ~CorePalletteImageData();

   virtual Pallette* getPallette(void);
   virtual void      setPallette(Pallette* p);

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
 protected:

   int getPalletteIndex(RGBPixel *);
   int getPalletteIndex(GreyPixel *);
};

#endif
