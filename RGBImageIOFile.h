//  RGBImageIOFile.h                  11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Defines the class RGBImageIOFile

//  This class handles read/write access to a raw RGB raster file


#ifndef _RGBIMAGEIOFILE_H_
#define _RGBIMAGEIOFILE_H_


#include "Image/ImageIOFile.h"
#include "Image/AbstractPixel.h"
#include "Image/Pallette.h"
#include <fstream.h>

class RGBImageIOFile : public ImageIOFile
{
    fstream* Stream;
    char* headerFile;

  public:
    RGBImageIOFile(const char* fn,const char* header);  // file must exist
    RGBImageIOFile(const char* fn);  // file must exist
    virtual ~RGBImageIOFile();

//  Standard ImageIOFile interface.

//  the get functions allocate storage, so the user must
//  delete this storage.

    virtual Pallette* getPallette(void);
    virtual void      setPallette(Pallette* p);

//  Note:  The get/put functions all use column-major addressing (ie.
//         (x,y) means (column, row).  Also, the get functions
    
    virtual AbstractPixel* getPixel(long x, long y);
    virtual AbstractPixel* getScanline(long row);
    virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);
    virtual void   putPixel(AbstractPixel* p, long x, long y);
    virtual void   putScanline(AbstractPixel* p, long row);
    virtual void   putRectangle(AbstractPixel* p,long x1,long y1,long x2,long y2);

    virtual void*  getRawPixel(long x, long y);
    virtual void*  getRawScanline(long row);
    virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);
    virtual void   putRawPixel(void* p, long x, long y);
    virtual void   putRawScanline(void* p, long row);
    virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2);

    virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix);
    virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf); 
    virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                      RawRGBPixel* pixbuf);

    virtual bool getOptimalRectangle(int &w, int &h);

  protected:
    void readHeader(void);
    void writeHeader(void);
};

#endif
