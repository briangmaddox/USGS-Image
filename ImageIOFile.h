//  ImageIOFile.h                    11 Jul 94
//  Author: Greg Martin, SES, MCMC, USGS

//  Defines the abstract class ImageIOFile
//  This class allows read/write behaviour


#ifndef _IMAGEIOFILE_H_
#define _IMAGEIOFILE_H_


#include "Image/ImageFile.h"
#include "Image/AbstractPixel.h"
#include "Image/Pallette.h"


class ImageIOFile : public ImageFile
{

  public:
    ImageIOFile();
    ImageIOFile(const char* fn);
    virtual ~ImageIOFile();


//  Virtual functions to outline behaviour

    virtual Pallette* getPallette(void)=0;
    virtual void      setPallette(Pallette* p)=0;

// get/put functions use column major order, ie (x,y) == (column,row)
    
    virtual AbstractPixel* getPixel(long x, long y)=0;
    virtual AbstractPixel* getScanline(long row)=0;
    virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2)=0;
    virtual void   putPixel(AbstractPixel* p, long x, long y)=0;
    virtual void   putScanline(AbstractPixel* p, long row)=0;
    virtual void   putRectangle(AbstractPixel* p,long x1,long y1,long x2,long y2)=0;

    virtual void*  getRawPixel(long x, long y)=0;
    virtual void*  getRawScanline(long row)=0;
    virtual void*  getRawRectangle(long x1, long y1, long x2, long y2)=0;
    virtual void   putRawPixel(void* p, long x, long y)=0;
    virtual void   putRawScanline(void* p, long row)=0;
    virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2)=0;

    virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix)=0;
    virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf)=0; 
    virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                      RawRGBPixel* pixbuf)=0;
 
    virtual bool   getOptimalRectangle(int &w, int &h)=0;
};

#endif
