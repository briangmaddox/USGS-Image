//  PNGImageIFile.h                    12 Jan 96
//  Authors:  Greg Martin SES, MCMC, USGS

//  Defines the class PNGImageIFile

//  This class handles reading Portable Network Graphics  raster
//  files.  


#ifndef _PNGIMAGEIFILE_H_
#define _PNGIMAGEIFILE_H_


#include <string.h>
#include <stdio.h>
#include "Image/ImageIFile.h"
extern "C"
{
#include "png/png.h"
}
#include "Image/Pallette.h"
#include "Image/RGBPallette.h"



class PNGImageIFile : public ImageIFile
{
  protected:
    png_struct    *_png;
    png_info      *_pngInfo;
    FILE          *_pngF;

    RGBPallette   *_pal;
    unsigned char **_rows;

  public:
    PNGImageIFile(const char* pfilename); 
    virtual ~PNGImageIFile(); 

//  Standard ImageIFile interface
//  the get functions allocate storage which the user must delete

    virtual Pallette* getPallette(void);

//  Note:  The get/put functions all use column-major addressing (ie.
//         (x,y) means (column, row).  Also, the get functions

    virtual void*  getRawPixel(long x, long y);
    virtual void*  getRawScanline(long row);
    virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);

    virtual AbstractPixel* getPixel(long x, long y);
    virtual AbstractPixel* getScanline(long row);
    virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);

    virtual void getRawRGBPixel(long x, long y, RawRGBPixel* pix);
    virtual void getRawRGBScanline(long row, RawRGBPixel* pixbuf); 
    virtual void getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                    RawRGBPixel* pixbuf);

    virtual bool getOptimalRectangle(int &w, int &h);

};

#endif
