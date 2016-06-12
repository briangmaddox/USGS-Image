//  RGBImage.h                       11 Jul 94
//  Author:  Greg Martin

//  Defines the class RGBImage

//  RGBImage is intended to hide the details of where the
//  data for an image comes from, and present the image data
//  associated with it as a 3 band (presumably color)
//  image.

//  Note:  This class can have as its ImageData object any
//  kind of image data object, and makes no assumptions about
//  the object.  This means that you can give a greyscale file
//  to an RGBImage and it will use it.  However, when you ask
//  for pixels, scanlines, or rectangles, RGBImage assures that
//  RGBPixels are returned, which essentially means that any 
//  greyscale data is translated into RGB data.  In return,
//  RGBImage expects you to give it scanlines and rectangles
//  made up of RGBPixels.  If you give it a GreyPixel (or 
//  some other object derived from Pixel) it won't work correctly.
//  Also, if you attach a greyscale ImageData object to an
//  RGBImage, it won't automatically colorize it.  This is quite
//  impossible, although it is easy to go from color to gray. 

#ifndef _RGBIMAGE_H_
#define _RGBIMAGE_H_

#include "Image/AbstractPixel.h"
#include "Image/Image.h"
#include "Image/ImageData.h"
#include "Image/Pallette.h"

class RGBImage : public Image
{
  public:
    
    RGBImage(const char* filename);   // creates a *ImageIFile* 
                                      // attempts to determine the type
                                      // of ImageData from extension,
                                      // magic number, etc...

    RGBImage(const char* filename,    // creates a *ImageOFile* 
             long width,              // attempts to determine the type
             long height);            // of ImageData from extension,
                                      // magic number, etc...

    RGBImage(ImageData* d);
    virtual ~RGBImage();

// Note:  These functions all use column-major order, as do all
//        get/put functions associated with the Image classes.
//        Also, the get functions allocate storage which must
//        be freed by the user.

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
