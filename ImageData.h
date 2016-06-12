// ImageData.h                   11 Jul 94
// Author: Greg Martin, SES, MCMC, USGS

// Defines the abstract class ImageData


#ifndef _IMAGEDATA_H_
#define _IMAGEDATA_H_

#include "Image/AbstractPixel.h"
#include "Image/Pallette.h"
#include "Image/ImageCommon.h"


class ImageData
{
  long Width;
  long Height;
  int  bitsPerSample;
  int  samplesPerPixel;
  // Uses values in ImageCommon.h
  unsigned char Photometric;
 
  enum IDState {
    goodbit=0,
    nodatabit=1,
    failrbit=2,
    failwbit=4
  };

public:
  enum RanAccess {
    none=0, 
    rpixel=1,
    rrow=2,
    rrect=4,
    wpixel=8,
    wrow=16,
    wrect=32
  };
                     
private:

  unsigned int RandomAccessFlags;
  unsigned int State;
  bool _hasPallette;


public:
  
  ImageData();
  ImageData(long w, long h);
  virtual ~ImageData();
  
  bool good(void);
  bool bad(void);
  bool noData(void);
  bool failedRead(void);
  bool failedWrite(void);
  void clear(void);

protected:
  void setNoDataBit(void);
  void setFailRBit(void);
  void setFailWBit(void);
  void setRandomAccessFlags(unsigned int);
  void setBitsPerSample(int);
  void setSamplesPerPixel(int);
  void setPhotometric(unsigned char p);
  void setHasPallette(void);
  void unsetHasPallette(void);

public:

  bool hasPallette(void);
  unsigned int getRandomAccessFlags(void);
  bool canReadPixel(void);
  bool canReadScanline(void);
  bool canReadRectangle(void);
  bool canWritePixel(void);
  bool canWriteScanline(void);
  bool canWriteRectangle(void);
  long getWidth();
  long getHeight();
  void setWidth(long w);
  void setHeight(long h);
  int  getBitsPerSample();
  int  getSamplesPerPixel();
  unsigned char getPhotometric(void);

  // Virtual functions to define behaviour of children objects
  virtual Pallette* getPallette(void)=0;
  virtual void      setPallette(Pallette* p)=0;

  // get/put functions use column major order...ie, (x,y) == (column,row)

  virtual void*  getRawPixel(long x, long y)=0;
  virtual void*  getRawScanline(long row)=0;
  virtual void*  getRawRectangle(long x1, long y1, long x2, long y2)=0;
  virtual void   putRawPixel(void* p, long x,long y)=0;
  virtual void   putRawScanline(void* p,long row)=0;
  virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2)=0;

  virtual AbstractPixel* getPixel(long x, long y)=0;
  virtual AbstractPixel* getScanline(long row)=0;
  virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2)=0;
  virtual void   putPixel(AbstractPixel* p, long x, long y)=0;
  virtual void   putScanline(AbstractPixel* s, long row)=0;
  virtual void   putRectangle(AbstractPixel* r, long x1, long y1, long x2, long y2)=0;

  virtual void getRawRGBPixel(long x, long y, RawRGBPixel* pix)=0;
  virtual void getRawRGBScanline(long row, RawRGBPixel* pixbuff)=0; 
  virtual void getRawRGBRectangle(long x1, long y1, long x2, long y2,
				  RawRGBPixel* pixbuf)=0;

  virtual bool getOptimalRectangle(int &w, int &h)=0;
};

#endif
