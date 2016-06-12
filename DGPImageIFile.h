//  DGPImageIFile.h                11 Jul 94
//  Author:  Greg Martin, SES, MCMC, USGS

//  Defines the DGPImageIFile class

//  This class handles the reading of DGP format raster files.


#ifndef _DGPIMAGEIFILE_H_
#define _DGPIMAGEIFILE_H_


#include <fstream.h>
//#include <stdio.h>
#include "Image/AbstractPixel.h"
#include "Image/Pallette.h"
#include "Image/ImageIFile.h"
#include "Image/RGBPallette.h"

class DGPImageIFile : public ImageIFile
{
    ifstream*    Stream;
//    FILE*        Stream;
    char*        Name;
    char*        Version;
    double       XUpperLeft;
    double       YUpperLeft;
    double       PixelSize;
    RGBPallette* Pal;

    long        TopOfData;

//  These are for a scanline cache
//  hashTable will be as big as the image is high

    char *hashTable;  
    unsigned char* rowCache[125];
    long           rowNumber[125];
    unsigned long  timeStamp[125];
    unsigned long  lastTimeStamp;

  public:

    DGPImageIFile(const char* fn);
    virtual ~DGPImageIFile();

    char* getName();
    char* getVersion();
    double getX();
    double getY();
    double getPixelSize();


//  Standard ImageIFile interface
//  These get functions allocate memory; this memory must
//  be deleted by the user.

    virtual Pallette* getPallette(void);
    
// get/put functions use column-major order, ie. (x,y) == (column,row)

    virtual AbstractPixel* getPixel(long x, long y);
    virtual AbstractPixel* getScanline(long row);
    virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);

    virtual void*  getRawPixel(long x, long y);
    virtual void*  getRawScanline(long row);
    virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);

    virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix);
    virtual void   getRawRGBScanline(long row, RawRGBPixel* pix); 
    virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                      RawRGBPixel* pixbuf);
    virtual bool getOptimalRectangle(int &w, int &h);

  protected:
    
    void readHeader(void);
    char cacheScanline(long row);
};

#endif
