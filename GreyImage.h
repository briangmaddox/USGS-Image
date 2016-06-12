//  GreyImage.h                       11 Jul 94
//  Author:  Greg Martin

//  Defines the class GreyImage

//  This class is intended to hide the details of where the
//  data for an image comes from, and present the data asssociated
//  with it as if it were a 1 band (presumably greyscale) image.

//  Note:  This class can have as its ImageData object any
//  kind of image data object, and makes no assumptions about
//  the object.  This means that you can give a RGB file
//  to an GreyImage and it will use it.  However, when you ask
//  for pixels, scanlines, or rectangles, GreyImage assures that
//  GreyPixels are returned, which essentially means that any
//  RGB data is translated into greyscale data.  In return,
//  GreyImage expects you to give it scanlines and rectangles
//  made up of GreyPixels.  If you give it an RGBPixel (or
//  some other object derived from Pixel) it won't work correctly.


#ifndef _GREYIMAGE_H_
#define _GREYIMAGE_H_

#include "Image/AbstractPixel.h"
#include "Image/Image.h"
#include "Image/ImageData.h"
#include "Image/Pallette.h"

class GreyImage : public Image
{
  public:
    GreyImage(const char* filename);  // creates a *ImageIFile*
                                      // attempts to determine the type
                                      // of ImageData from extension,
                                      // magic number, etc...

    GreyImage(const char* filename,   // creates a *ImageOFile*
             long width,              // attempts to determine the type
             long height);            // of ImageData from extension,
                                      // magic number, etc...

    GreyImage(ImageData* d);
    virtual ~GreyImage();

//  Note:  The get/put functions all use column-major addressing (ie.
//         (x,y) means (column,row).  Also, the get functions
//         allocate storage which must be freed by the user. 

    virtual void* getRawPixel(long x, long y);
    virtual void* getRawScanline(long row);
    virtual void* getRawRectangle(long x1, long y1, long x2, long y2);
    virtual void  putRawPixel(void* p, long x, long y);
    virtual void  putRawScanline(void* p, long row);
    virtual void  putRawRectangle(void* p, long x1, long y1, long x2, long y2);
 
    virtual AbstractPixel* getPixel(long x, long y);
    virtual AbstractPixel* getScanline(long row);
    virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);
    virtual void   putPixel(AbstractPixel* p, long x, long y);
    virtual void   putScanline(AbstractPixel* p, long row);
    virtual void   putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2); 

    virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix);
    virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf); 
    virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                      RawRGBPixel* pixbuf);

    virtual Pallette* getPallette(void);
    virtual void      setPallette(Pallette *);
};


#endif
