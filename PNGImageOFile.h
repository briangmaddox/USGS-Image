//  PNGImageOFile.h                    21 Jan 96
//  Author:  Greg Martin

//  Defines the class PNGImageOFile

//  This class handles the writing of Portable Network Graphics  raster
//  files.  


#ifndef _PNGIMAGEOFILE_H_
#define _PNGIMAGEOFILE_H_

#include <string.h>
#include <stdio.h>
#include "Image/ImageOFile.h"
extern "C"
{
#include "png/png.h"
}
#include "Image/Pallette.h"
#include "Image/RGBPallette.h"
#include "Image/RGBPixel.h"


class PNGImageOFile : public ImageOFile
{
  protected:
    png_struct      *_png;
    png_info        *_pngInfo;
    FILE            *_pngF;

    RGBPallette     *_pal;
    int              _colorsFound;
    unsigned char  **_rows;

  public:
    PNGImageOFile();
    PNGImageOFile(const char* pfilename, long pw, long ph, int ptype); 

    virtual ~PNGImageOFile();

// Standard ImageOFile interface
// getPallette allocates storage which the user must delete

   virtual Pallette* getPallette(void);
   virtual void setPallette(Pallette* p);

//  Note:  The get/put functions all use column-major addressing (ie.
//         (x,y) means (column, row).  Also, the get functions

   virtual void putRawPixel(void* p, long x,long y);
   virtual void putRawScanline(void* p,long row);
   virtual void putRawRectangle(void* p,long x1,long y1,long x2,long y2);


   virtual void putPixel(AbstractPixel* p, long x, long y);
   virtual void putScanline(AbstractPixel* p, long row);
   virtual void putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2);

   virtual bool getOptimalRectangle(int &w, int &h);

  protected:

   unsigned char getPalletteIndex(RGBPixel *foo);
};

#endif
