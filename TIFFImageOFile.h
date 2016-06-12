//  TIFFImageOFile.h                    11 Jul 94
//  Author:  Duane Lascoe and Greg Martin

//  Defines the class TIFFImageOFile

//  This class handles the writing of Tagged Image File Format raster
//  files.  Currently, only scanline-oriented images with the Macintosh
//  PackBits (glorified RLE) compression is supported.  Greyscale, and
//  other interpretations will be included Real Soon Now [tm].


#ifndef _TIFFIMAGEOFILE_H_
#define _TIFFIMAGEOFILE_H_

#include "Image/ImageOFile.h"
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#include "Image/Pallette.h"
#include "Image/RGBPixel.h"


class TIFFImageOFile : public ImageOFile
{
protected:
  TIFF* tif;
  unsigned short* red;         // red, green, and blue are for
  unsigned short* green;       // the tiff colormap
  unsigned short* blue;
  unsigned char* buffer;
  unsigned int scan_line;
  unsigned char color_count;

public:
  TIFFImageOFile();
  TIFFImageOFile(const char* tfilename, long tw, long th); 
  // comp - 0 = PackBits 1 = LZW, 2 = None
  TIFFImageOFile(const char* tfilename, long tw, long th, int comp);
  // Added to allow the user to define most of the TIFF fields
  TIFFImageOFile(const char* tfilename, long tw, long th, int samples_pixel,
		 int bits_sample, int comp, int photometric);

  virtual ~TIFFImageOFile();

  bool setTag(int tag, unsigned long val);
  bool setTag(int tag, unsigned short val);
  bool setTag(int tag, float val);
  bool setTag(int tag, char* val);
  bool setTag(int tag, unsigned long* val);
  bool setTag(int tag, unsigned short* val);
  bool setTag(int tag, float* val);
  bool setTag(int tag, double* val);
  bool setTag(int tag, unsigned char** val);
  

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

  // Moved here as there really is a use in allowing them to be called by
  // users
  virtual void updateColormap(RGBPixel* p);
  virtual void writeColormap(void); // output colormap once built
protected:
};

#endif
