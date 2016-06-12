//  Image.h                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS


#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "Image/AbstractPixel.h"
#include "Image/ImageData.h"
#include "Image/ImageCommon.h"
#include "Image/Pallette.h"

/*
  Defines the abstract class Image.

  Image is the class that will be used to pass Image objects
  back and forth between functions/classes which operate upon
  Images.  Image is an abstract class, which just defines 
  behaviour for classes derived from Image.  All classes derived
  from Image are required to provide the virtual functions
  outlined in Image.
  */
class Image
{
  ImageData* Data;
  unsigned char Photometric;
  int NoBands;
  unsigned char BitsPerBand;  
  bool _hasPallette;
   
  enum ImageState { goodbit=0, nodatabit=1, failrbit=2, failwbit=4 };

public:

  enum RandAccess {
    none=0,
    rpixel=1,
    rrow=2,
    rrect=4,
    wpixel=8,
    wrow=16,
    wrect=32
  };
private:

  unsigned int State;
  unsigned int RandomAccessFlags;
  int FileType;
  
public:

  Image();
  Image(ImageData* d);
  virtual ~Image();

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
  void setHasPallette(void);
  void unsetHasPallette(void);
   

public:

  //  Note:  in the get/put functions, column major order is used.
  //         in other words, (x,y) refers to (column,row).
  //         Also, in all of the classes derived from Image,
  //         the get functions allocate storage, which must
  //         be freed by the user of the classes.
    
  virtual void* getRawPixel(long x, long y)=0;
  virtual void* getRawScanline(long row)=0;
  virtual void* getRawRectangle(long x1, long y1, long x2, long y2)=0;
  virtual void  putRawPixel(void* pixel, long x, long y)=0;
  virtual void  putRawScanline(void* pixel, long row)=0;
  virtual void  putRawRectangle(void* pixel,long x1,long y1,long x2,long y2)=0;
    
  virtual AbstractPixel* getPixel(long x, long y)=0;
  virtual AbstractPixel* getScanline(long row)=0;
  virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2)=0;
  virtual void   putPixel(AbstractPixel* p, long x, long y)=0;
  virtual void   putScanline(AbstractPixel* s, long row)=0;
  virtual void   putRectangle(AbstractPixel* r, long x1, long y1, long x2, long y2)=0;
  
  virtual void getRawRGBPixel(long x, long y, RawRGBPixel* pix)=0;
  virtual void getRawRGBScanline(long row, RawRGBPixel* pixbuf)=0;
  virtual void getRawRGBRectangle(long x1,long y1, long x1, long y2,
				  RawRGBPixel* pixbuf)=0;

  virtual Pallette* getPallette(void)=0;
  virtual void      setPallette(Pallette *)=0;


  int  getNoBands();
  int  getBitsPerBand();
  long getWidth();
  long getHeight(); 
  void setNoBands(int n);
  void setBitsPerBand(int b);
  void setWidth(long w);
  void setHeight(long h);
  unsigned char getFileType(const char* filename, int file_exists);
  unsigned char getFileType(void);
  ImageData* getImageData(void);
  unsigned char getPhotometric(void);

  bool hasPallette(void);
  bool canReadPixel(void);
  bool canReadScanline(void);
  bool canReadRectangle(void);
  bool canWritePixel(void);
  bool canWriteScanline(void);
  bool canWriteRectangle(void);
  
protected:

  void setPhotometric(unsigned char);
  void setImageData(ImageData* d);
};


#endif
