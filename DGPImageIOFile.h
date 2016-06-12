//  DGPImageIOFile.h                13 Oct 94
//  Author:  Greg Martin, SES, MCMC, USGS

//  Defines the DGPImageIFile class

//  This class handles the reading of DGP format raster files.


#ifndef _DGPIMAGEIOFILE_H_
#define _DGPIMAGEIOFILE_H_


#include <fstream.h>
#include "Image/AbstractPixel.h"
#include "Image/Pallette.h"
#include "Image/ImageIOFile.h"
#include "Image/RGBPallette.h"

class DGPImageIOFile : public ImageIOFile
{
    fstream*    Stream;
    char*        Name;
    char*        Version;
    double       XUpperLeft;
    double       YUpperLeft;
    double       PixelSize;
    unsigned long NoColors;
    unsigned long ColorsFound;
    RGBPallette* Pal;

    long        TopOfColormap;
    long        TopOfData;

  public:

    DGPImageIOFile(const char* fn);
    virtual ~DGPImageIOFile();

    char* getName();
    char* getVersion();
    double getX();
    double getY();
    double getPixelSize();


//  Standard ImageIOFile interface
//  These get functions allocate memory; this memory must
//  be deleted by the user.

    virtual Pallette* getPallette(void);
    virtual void setPallette(Pallette* p);
    
// get/put functions use column-major order, ie. (x,y) == (column,row)

    virtual AbstractPixel* getPixel(long x, long y);
    virtual AbstractPixel* getScanline(long row);
    virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);

    virtual void*  getRawPixel(long x, long y);
    virtual void*  getRawScanline(long row);
    virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);

    virtual void putRawPixel(void* p, long x, long y);
    virtual void putRawScanline(void* p, long row);
    virtual void putRawRectangle(void* p, long x1, long y1, long x2, long y2);

    virtual void putPixel(AbstractPixel* p, long x, long y);
    virtual void putScanline(AbstractPixel* p, long row);
    virtual void putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2);

    virtual void getRawRGBPixel(long x, long y, RawRGBPixel* pix);
    virtual void getRawRGBScanline(long row, RawRGBPixel* pixbuf); 
    virtual void getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                    RawRGBPixel* pixbuf);
    virtual bool getOptimalRectangle(int &w, int &h);

  protected:
    
    void readHeader(void);
    void writeHeader(void);
    unsigned char getPalletteIndex(RGBPixel* p);
    void writePallette(void);
};

#endif
