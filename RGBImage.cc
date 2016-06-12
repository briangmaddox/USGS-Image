//  RGBImage.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class RGBImage 


#include "Image/RGBImage.h"
#include "Image/RGBPixel.h"
#include "Image/GreyPixel.h"
#include "Image/ImageCommon.h"
#include "Image/RGBImageIFile.h"
#include "Image/RGBImageOFile.h"
#include "Image/GreyImageIFile.h"
#include "Image/GreyImageOFile.h"
#include "Image/TIFFImageIFile.h"
#include "Image/TIFFImageOFile.h"
#include "Image/DGPImageIFile.h"
#include "Image/DGPImageOFile.h"
#include "Image/CRLImageIFile.h"
#include "Image/DOQImageIFile.h"
#include "Image/PNGImageIFile.h"
#include "Image/PNGImageOFile.h"
#include "Image/GeoTIFFImageIFile.h"
#include "Image/ImageDebug.h"


RGBImage::RGBImage(const char* filename)
{
   unsigned char ftype;
   ImageData* d = NULL;
   bool flag = true;

   ftype = getFileType(filename,1);

   switch (ftype)
   {
      case IMAGE_RGB:      d = new RGBImageIFile(filename);
                           break;
      case IMAGE_GREY:     d = new GreyImageIFile(filename);
                           break;
      case IMAGE_TIFF:     d = new TIFFImageIFile(filename);
                           break;
      case IMAGE_DGP:      d = new DGPImageIFile(filename);
                           break;
      case IMAGE_CRL:      d = new CRLImageIFile(filename);
                           break;
      case IMAGE_DOQ:      d = new DOQImageIFile(filename); 
                           break;
      case IMAGE_GTIF:     d = new GeoTIFFImageIFile(filename); 
                           break;
      case IMAGE_PNG:      d = new PNGImageIFile(filename); 
                           break;
      case IMAGE_GIF:      
      case IMAGE_PBM:      
      case IMAGE_XBM:      
      case IMAGE_BMP:      
      case IMAGE_PCX:      
      case IMAGE_JFIF:     
      case IMAGE_UNKNOWN:  
      case IMAGE_ERROR:    flag = false; 
                           break;
   }  
   if (!flag)
   {
      setNoDataBit();
      gDebug.msg("RGBImage::RGBImage(char*): Can't handle format",1);
   }
   else if (!d->good())
   {
      setNoDataBit();
      gDebug.msg("RGBImage::RGBImage(char*): No data",1);
   }
   else
   {
      setImageData(d);
      setNoBands(d->getSamplesPerPixel());
      setBitsPerBand(d->getBitsPerSample());
      if (d->hasPallette())
         setHasPallette();
      else
         unsetHasPallette();
      setPhotometric(DRG_RGB);
   }
   gDebug.msg("RGBImage::RGBImage(char*): exiting ctor",5);
}

RGBImage::RGBImage(const char* filename, long width, long height)
{
   unsigned char ftype;
   ImageData* d = NULL;
   bool flag = true;

   ftype = getFileType(filename,0);

   switch (ftype)
   {
      case IMAGE_RGB:      d = new RGBImageOFile(filename,width,height);
                           break;
      case IMAGE_GREY:     d = new GreyImageOFile(filename,width,height);
                           break;
      case IMAGE_TIFF:     d = new TIFFImageOFile(filename,width,height);
                           break;
      case IMAGE_DGP:      d = new DGPImageOFile(filename,width,height);
                           break;
      case IMAGE_PNG:      d = new PNGImageOFile(filename,width,height,0);
                           break;
      case IMAGE_CRL:
      case IMAGE_DOQ:
      case IMAGE_GIF:
      case IMAGE_PBM:
      case IMAGE_XBM:
      case IMAGE_BMP:
      case IMAGE_PCX:
      case IMAGE_JFIF:
      case IMAGE_ERROR:
      case IMAGE_UNKNOWN:  flag = false; 
                           break;
   }  
   if (!flag)
   {
      setNoDataBit();
      gDebug.msg("RGBImage::RGBImage(char*,long,long): Can't handle format",1);
   }
   else if (!d->good())
   {
      setNoDataBit();
      gDebug.msg("RGBImage::RGBImage(char*,long,long): No data",1);
   }
   else
   {
      setImageData(d);
      setNoBands(d->getSamplesPerPixel());
      setBitsPerBand(d->getBitsPerSample());
      if (d->hasPallette())
         setHasPallette();
      else
         unsetHasPallette();
      setPhotometric(DRG_RGB);
   }
   gDebug.msg("RGBImage::RGBImage(char*,long,long): exiting ctor",5);
}

RGBImage::RGBImage(ImageData* d):
   Image(d)
{
   setNoBands(d->getSamplesPerPixel());
   setBitsPerBand(d->getBitsPerSample());
   if (d->hasPallette())
      setHasPallette();
   else
      unsetHasPallette();
   setPhotometric(DRG_RGB);
   gDebug.msg("RGBImage::RGBImage(ImageData*): exiting ctor",5);
}


RGBImage::~RGBImage()
{
   gDebug.msg("RGBImage::~RGBImage(): exiting dtor",5);
}


void* RGBImage::getRawPixel(long x, long y)
{
    void *foo = getImageData()->getRawPixel(x,y);
    if (!getImageData()->good())
    {
       setFailRBit();
       gDebug.msg("RGBImage::getRawPixel(): read failed",1);
    }
    return foo;
}

void* RGBImage::getRawScanline(long row)
{
    void *foo = getImageData()->getRawScanline(row);
    if (!getImageData()->good())
    {
       setFailRBit();
       gDebug.msg("RGBImage::getRawScanline(): read failed",1);
    }
    return foo;
}

void* RGBImage::getRawRectangle(long x1, long y1, long x2, long y2)
{
    void *foo = getImageData()->getRawRectangle(x1,y1,x2,y2);
    if (!getImageData()->good())
    {
       setFailRBit();
       gDebug.msg("RGBImage::getRawRectangle(): read failed",1);
    }
    return foo;
}

void RGBImage::putRawPixel(void* p, long x, long y)
{
    getImageData()->putRawPixel(p,x,y);
    if (!getImageData()->good())
    {
       setFailWBit();
       gDebug.msg("RGBImage::putRawPixel(): write failed",1);
    }
}

void RGBImage::putRawScanline(void* p, long row)
{
   getImageData()->putRawScanline(p,row);
   if (!getImageData()->good())
   {
      setFailWBit();
      gDebug.msg("RGBImage::putRawScanline(): write failed",1);
   }
}

void RGBImage::putRawRectangle(void* p,long x1,long y1,long x2, long y2)
{
   getImageData()->putRawRectangle(p,x1,y1,x2,y2);
   if (!getImageData()->good())
   {
      setFailWBit();
      gDebug.msg("RGBImage::putRawRectangle(): write failed",1);
   }
}


AbstractPixel* RGBImage::getPixel(long x, long y)
{
   RGBPixel* pix;
   AbstractPixel* foo;
   
   foo = getImageData()->getPixel(x,y);
 
   if ( getImageData()->getPhotometric() != DRG_RGB)
   {
      gDebug.msg("RGBImage::getPixel(): colorspace is not DRG_RGB",5);
      pix = new RGBPixel(foo->getRed(),foo->getGreen(),foo->getBlue());
      delete foo;
   }
   else
   {
      gDebug.msg("RGBImage::getPixel(): colorspace is DRG_RGB",5);
      pix = (RGBPixel*) foo;
   }
       
   if (!getImageData()->good())
   {
      setFailRBit();
      gDebug.msg("RGBImage::getPixel(): read failed",1);
   }
   return pix;
}

AbstractPixel* RGBImage::getScanline(long row)
{
   RGBPixel* pix = NULL;
   unsigned char photo = getImageData()->getPhotometric();

   if (photo == DRG_RGB)
   { 
      gDebug.msg("RGBImage::getPixel(): colorspace is DRG_RGB",5);
      pix = (RGBPixel*)getImageData()->getScanline(row);
   }
   else if (photo == DRG_GREY)
   {
      GreyPixel* foo;
      long i;
      long width = getWidth();

      gDebug.msg("RGBImage::getScanline(): colorspace is DRG_GREY",5);
   
      pix = new RGBPixel[width];

      foo = (GreyPixel*)getImageData()->getScanline(row);
      for (i=0;i<width;i++)
      {
          pix[i].setRed(foo[i].getRed());
          pix[i].setGreen(foo[i].getGreen());
          pix[i].setBlue(foo[i].getBlue());
      }
      delete[] foo;
   }
   else
      gDebug.msg("RGBImage::getScanline(): colorspace is unknown",1);
 
   if (!getImageData()->good())
   {
      setFailRBit();
      gDebug.msg("RGBImage::getScanline(): read failed",1);
   }
 
   return pix;
      
}

AbstractPixel* RGBImage::getRectangle(long x1, long y1, long x2, long y2)
{
   RGBPixel* pix = NULL;
   unsigned char photo = getImageData()->getPhotometric();

   if (photo == DRG_RGB)
   {
      gDebug.msg("RGBImage::getRectangle(): colorspace is DRG_RGB",5);
      pix = (RGBPixel*)getImageData()->getRectangle(x1,y1,x2,y2);
   }
   else if (photo == DRG_GREY)
   {
      GreyPixel* foo;
      long i;
      long size = (x2-x1+1)*(y2-y1+1);
 
      gDebug.msg("RGBImage::getRectangle(): colorspace is DRG_GREY",5);

      pix = new RGBPixel[size];
   
      foo = (GreyPixel*)getImageData()->getRectangle(x1,y1,x2,y2);
      for (i=0;i<size;i++)
      {
          pix[i].setRed(foo[i].getRed());
          pix[i].setGreen(foo[i].getGreen());
          pix[i].setBlue(foo[i].getBlue());
      }
      delete[] foo;
   }
   else
      gDebug.msg("RGBImage::getRectangle(): colorspace is unknown",1);
   if (!getImageData()->good())
   {
      setFailRBit();
      gDebug.msg("RGBImage::getRectangle(): read failed",1);
   }

   return pix;
}

void RGBImage::putPixel(AbstractPixel* p, long x, long y)
{
   unsigned char photo = getImageData()->getPhotometric();

   if (photo == DRG_RGB)
   {
      gDebug.msg("RGBImage::putPixel(): colorspace is DRG_RGB",5);
      getImageData()->putPixel(p,x,y);
   }
   else if (photo == DRG_GREY)
   {
      GreyPixel* temp = new GreyPixel;
      gDebug.msg("RGBImage::putPixel(): colorspace is DRG_GREY",5);
      temp->setGrey(p->getGrey());
      getImageData()->putPixel(temp,x,y);
      delete temp;
   }
   else 
      gDebug.msg("RGBImage::putPixel(): colorspace is unknown",1);
   if (!getImageData()->good())
   {
      setFailWBit();
      gDebug.msg("RGBImage::putPixel(): write failed",1);
   }
}


void RGBImage::putScanline(AbstractPixel* p, long row)
{
   unsigned char photo = getImageData()->getPhotometric();
 
   if (photo == DRG_RGB)
   {
      gDebug.msg("RGBImage::putScanline(): colorspace is DRG_RGB",5);
      getImageData()->putScanline(p,row);
   }
   else if (photo == DRG_GREY)
   {
      long width = getWidth();
      GreyPixel* temp = new GreyPixel[width];
      RGBPixel* foo = (RGBPixel*)p;
      long i;

      gDebug.msg("RGBImage::putScanline(): colorspace is DRG_GREY",5);

      for (i=0;i<width;i++)
          temp[i].setGrey(foo[i].getGrey());
      getImageData()->putScanline(temp,row);
      delete[] temp;
   }
   else
      gDebug.msg("RGBImage::putScanline(): colorspace is unknown",1);
   if (!getImageData()->good())
   {
      setFailWBit();
      gDebug.msg("RGBImage::putScanline(): write failed",1);
   }
}

void RGBImage::putRectangle(AbstractPixel* p,long x1, long y1, long x2, long y2)
{
   unsigned char photo = getImageData()->getPhotometric();
 
   if (photo == DRG_RGB)
   {
      gDebug.msg("RGBImage::putRectangle(): colorspace is DRG_RGB",5);
      getImageData()->putRectangle(p,x1,y1,x2,y2);
   }
   else if (photo == DRG_GREY)
   {
      long datalength = (x2-x1+1)*(y2-y1+1);
      GreyPixel* temp = new GreyPixel[datalength];
      RGBPixel*  foo  = (RGBPixel*)p;
      long i;

      gDebug.msg("RGBImage::putRectangle(): colorspace is DRG_GREY",5);

      for (i=0;i<datalength;i++)
          temp[i].setGrey(foo[i].getGrey());
      getImageData()->putRectangle(temp,x1,y1,x2,y2);
      delete[] temp;
   }
   else
      gDebug.msg("RGBImage::putRectangle(): colorspace is unknown",1);

   if (!getImageData()->good())
   {
      setFailWBit();
      gDebug.msg("RGBImage::putRectangle(): write failed",1);
   }
}

 

void RGBImage::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
   getImageData()->getRawRGBPixel(x,y,pix);
   if (!getImageData()->good())
   {
      gDebug.msg("RGBImage::getRawRGBPixel(): read failed",1);
      setFailRBit();
   }
}    

void RGBImage::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
   getImageData()->getRawRGBScanline(row,pixbuf);
   if (!getImageData()->good())
   {
      gDebug.msg("RGBImage::getRawRGBScanline(): read failed",1);
      setFailRBit();
   }
}    

void RGBImage::getRawRGBRectangle(long x1,long y1, long x2, long y2,
                                  RawRGBPixel* pixbuf)
{
   getImageData()->getRawRGBRectangle(x1,y1,x2,y2,pixbuf);
   if (!getImageData()->good())
   {
      gDebug.msg("RGBImage::getRawRGBRectangle(): read failed",1);
      setFailRBit();
   }
}    

void RGBImage::setPallette(Pallette* p)
{
   if (getImageData()->hasPallette())
   {
      getImageData()->setPallette(p);
      if (!getImageData()->good())
      {
         gDebug.msg("RGBImage::setPallette(): write failed",1);
         setFailRBit();
      }
   }
   else
      gDebug.msg("RGBImage::setPallette(): can't set pallette, ImageData object
has no pallette.",0);
   gDebug.msg("RGBImage::setPallette(): exiting",5);
}

Pallette* RGBImage::getPallette(void)
{
   gDebug.msg("RGBImage::getPallette(): exiting",5);
   return getImageData()->getPallette();
}
