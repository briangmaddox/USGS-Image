//  TIFFImageIFile.h                    11 Jul 94
//  Authors:  Greg Martin and Duane Lascoe  SES, MCMC, USGS

//  Defines the class TIFFImageIFile

//  This class handles reading Tagged Image File Format raster
//  files.  Currently, it handles only scanline oriented files.
//  Tiled files will be included Real Soon Now [tm].


#ifndef _TIFFIMAGEIFILE_H_
#define _TIFFIMAGEIFILE_H_


#include "Image/ImageIFile.h"
#include "tiff/tiffio.h"
#include "tiff/tiff.h"
#include "Image/Pallette.h"



class TIFFImageIFile : public ImageIFile
{
  protected:
    TIFF* tif;
    unsigned short* red;         // red, green, and blue are for
    unsigned short* green;       // the tiff colormap
    unsigned short* blue;
    unsigned short bits_per_sample;

//  These are for a scanline cache
//  hashTable will be as big as the image is high

    char *hashTable;
    unsigned char* rowCache[20];
    long           rowNumber[20];
    unsigned long  timeStamp[20];
    unsigned long  lastTimeStamp;

  public:
    TIFFImageIFile(const char* tfilename); 
    virtual ~TIFFImageIFile(); 

    TIFF* getTif(void);
    bool getTag(int tag, unsigned long* val);
    bool getTag(int tag, unsigned short* val);
    bool getTag(int tag, float* val);
    bool getTag(int tag, double *val);
    bool getTag(int tag, char** val);
    bool getTag(int tag, unsigned long** val);
    bool getTag(int tag, unsigned short** val);
    bool getTag(int tag, float** val);
    bool getTag(int tag, double** val);
    bool getTag(int tag, unsigned char*** val);

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
   
  protected:
    char cacheScanline(long xx);

};

#endif
