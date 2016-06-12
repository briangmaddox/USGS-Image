//  GreyImage.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class GreyImage 

#include "Image/GreyImage.h"
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


GreyImage::GreyImage(const char* filename)
{
   unsigned char ftype;
   ImageData* d=NULL;
   bool flag=true;

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
      case IMAGE_ERROR:    flag=false; 
                           break;
   }
   if (!flag)
   {
      setNoDataBit();
      gDebug.msg("GreyImage::GreyImage(c*): Can't handle format",1);
   }
   else if (!d->good())
   {
      setNoDataBit();
      gDebug.msg("GreyImage::GreyImage(c*): No data",1);
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
      setPhotometric(DRG_GREY);
   }
   gDebug.msg("GreyImage::GreyImage(c*): exiting ctor",5);
}

GreyImage::GreyImage(const char* filename, long width, long height)
{
   unsigned char ftype;
   ImageData* d=NULL;
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
      gDebug.msg("GreyImage::GreyImage(c*,l,l): Can't handle format",1);
   }
   else if (!d->good())
   {
      setNoDataBit();
      gDebug.msg("GreyImage::GreyImage(c*,l,l): No data",1);
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
      setPhotometric(DRG_GREY);
   }
   gDebug.msg("GreyImage::GreyImage(c*,l,l): exiting ctor",5);
}

GreyImage::GreyImage(ImageData* d):
   Image(d)
{
   setNoBands(d->getSamplesPerPixel());
   setBitsPerBand(d->getBitsPerSample());
   if (d->hasPallette())
      setHasPallette();
   else
      unsetHasPallette();
   setPhotometric(DRG_GREY);
   gDebug.msg("GreyImage::GreyImage(ID*): exiting ctor",5);
}


GreyImage::~GreyImage()
{
   gDebug.msg("GreyImage::~GreyImage(): exiting dtor",5);
}


void* GreyImage::getRawPixel(long x, long y)
{
   void* foo = getImageData()->getRawPixel(x,y);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getRawPixel(): read failed",1);
      setFailRBit();
   }
   return foo;
}

void* GreyImage::getRawScanline(long row)
{
   void *foo = getImageData()->getRawScanline(row);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getRawScanline(): read failed",1);
      setFailRBit();
   }
   return foo;
}

void* GreyImage::getRawRectangle(long x1, long y1, long x2, long y2)
{
   void *foo = getImageData()->getRawRectangle(x1,y1,x2,y2);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getRawRectangle(): read failed",1);
      setFailRBit();
   }
   return foo;
}

void GreyImage::putRawPixel(void* p, long x, long y)
{
   getImageData()->putRawPixel(p,x,y);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::putRawPixel(): write failed",1);
      setFailWBit();
   }
}

void GreyImage::putRawScanline(void* p, long row)
{
   getImageData()->putRawScanline(p,row);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::putRawScanline(): write failed",1);
      setFailWBit();
   }
}

void GreyImage::putRawRectangle(void* p,long x1,long y1,long x2, long y2)
{
   getImageData()->putRawRectangle(p,x1,y1,x2,y2);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::putRawRectangle(): write failed",1);
      setFailWBit();
   }
}


AbstractPixel* GreyImage::getPixel(long x, long y)
{
   GreyPixel* pix;
   unsigned char photo = getImageData()->getPhotometric();

   if (photo == DRG_GREY)
   {
      gDebug.msg("GreyImage::getPixel(): colorspace is DRG_GREY",5);
      pix = (GreyPixel*)getImageData()->getPixel(x,y);
   }
   else
   {
      AbstractPixel* foo;

      gDebug.msg("GreyImage::getPixel(): colorspace is not DRG_GREY",5);

      foo = getImageData()->getPixel(x,y);
      pix = new GreyPixel(foo->getGrey());
    
      delete foo;
   }
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getPixel(): read failed",1);
      setFailRBit();
   }

   return pix;
}

AbstractPixel* GreyImage::getScanline(long row)
{
   GreyPixel* pix = NULL;
   unsigned char photo = getImageData()->getPhotometric();
   
   if (photo == DRG_GREY)
   {
      gDebug.msg("GreyImage::getScanline(): colorspace is DRG_GREY",5);
      pix = (GreyPixel*)getImageData()->getScanline(row);
   }
   else if (photo == DRG_RGB)
   {
      RGBPixel* foo;
      long width = getWidth();
      long i;

      gDebug.msg("GreyImage::getScanline(): colorspace is DRG_RGB",5);

      foo = (RGBPixel*)getImageData()->getScanline(row);
      pix = new GreyPixel[width];
      for (i=0;i<width;i++)
          pix[i].setGrey(foo[i].getGrey());
   
      delete[] foo;
   }
   else
      gDebug.msg("GreyImage::getScanline(): colorspace is unknown",1);

   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getScanline(): read failed",1);
      setFailRBit();
   }
   return pix;
}

AbstractPixel* GreyImage::getRectangle(long x1, long y1, long x2, long y2)
{
   GreyPixel* pix = NULL;
   unsigned char photo = getImageData()->getPhotometric();

   if (photo == DRG_GREY)
   {
      gDebug.msg("GreyImage::getRectangle(): colorspace is DRG_GREY",5);
      pix = (GreyPixel*)getImageData()->getRectangle(x1,y1,x2,y2);
   }
   else if (photo == DRG_RGB)
   {
      RGBPixel* foo;
      long size = (x2-x1+1)*(y2-y1+1);
      long i;

      gDebug.msg("GreyImage::getRectangle(): colorspace is DRG_RGB",5);

      foo = (RGBPixel*)getImageData()->getRectangle(x1,y1,x2,y2);

      pix = new GreyPixel[size];
      for (i=0;i<size;i++)
          pix[i].setGrey(foo[i].getGrey());

      delete[] foo;
   }
   else
      gDebug.msg("GreyImage::getRectangle(): colorspace is unknown",1);

   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getRectangle(): read failed",1);
      setFailRBit();
   }

   return pix;
}

void GreyImage::putPixel(AbstractPixel* p, long x, long y)
{
   unsigned char photo = getImageData()->getPhotometric();

   if (photo == DRG_GREY)
   {
      gDebug.msg("GreyImage::putPixel(): colorspace is DRG_GREY",5);
      getImageData()->putPixel(p,x,y);
   }
   else if (photo == DRG_RGB)
   {
      RGBPixel* temp = new RGBPixel;

      gDebug.msg("GreyImage::putPixel(): colorspace is DRG_RGB",5);

      temp->setGrey(p->getGrey());
      getImageData()->putPixel(temp,x,y);
      delete temp;
   }
   else
      gDebug.msg("GreyImage::putPixel(): colorspace is unknown",1);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::putPixel(): write failed",1);
      setFailWBit();
   }
}

void GreyImage::putScanline(AbstractPixel* p, long row)
{
   unsigned char photo = getImageData()->getPhotometric();
   
   if (photo == DRG_GREY)
   {
      gDebug.msg("GreyImage::putScanline(): colorspace is DRG_GREY",5);
      getImageData()->putScanline(p,row);
   }
   else if (photo == DRG_RGB)
   {
      RGBPixel* temp = new RGBPixel[getWidth()];
      GreyPixel* foo = (GreyPixel*)p;
      long i;

      gDebug.msg("GreyImage::putScanline(): colorspace is DRG_RGB",5);

      for (i=0;i<getWidth();i++)
          temp[i].setGrey(foo[i].getGrey());

      getImageData()->putScanline(temp,row);
      delete[] temp;
   }
   else
      gDebug.msg("GreyImage::putScanline(): colorspace is unknown",1);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::putScanline(): write failed",1);
      setFailWBit();
   }
}

void GreyImage::putRectangle(AbstractPixel* p,long x1, long y1, long x2, long y2)
{
   unsigned char photo = getImageData()->getPhotometric();

   if (photo == DRG_GREY)
   {
      gDebug.msg("GreyImage::putRectangle(): colorspace is DRG_GREY",5);
      getImageData()->putRectangle(p,x1,y1,x2,y2);
   }
   else if (photo == DRG_RGB)
   {
      long datalength = (x2-x1+1)*(y2-y1+1);
      RGBPixel* temp  = new RGBPixel[datalength];
      GreyPixel* foo  = (GreyPixel*)p;
      long i;

      gDebug.msg("GreyImage::putRectangle(): colorspace is DRG_RGB",5);

      for (i=0;i<datalength;i++)     
          temp[i].setGrey(foo[i].getGrey());
      getImageData()->putRectangle(temp,x1,y1,x2,y2);
      delete[] temp;
   }
   else
      gDebug.msg("GreyImage::putRectangle(): colorspace is unknown",1);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::putRectangle(): read failed",1);
      setFailWBit();
   }
}
 

void GreyImage::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
   getImageData()->getRawRGBPixel(x,y,pix);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getRawRGBPixel(): read failed",1);
      setFailRBit();
   }
}
    
void GreyImage::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
   getImageData()->getRawRGBScanline(row,pixbuf);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getRawRGBScanline(): read failed",1);
      setFailRBit();
   }
}

void GreyImage::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                   RawRGBPixel* pixbuf)
{
   getImageData()->getRawRGBRectangle(x1,y1,x2,y2,pixbuf);
   if (!getImageData()->good())
   {
      gDebug.msg("GreyImage::getRawRGBRectangle(): read failed",1);
      setFailRBit();
   }
}

void GreyImage::setPallette(Pallette* p)
{
   if (getImageData()->hasPallette())
   {
      getImageData()->setPallette(p);
      if (!getImageData()->good())
      {
         gDebug.msg("GreyImage::setPallette(): write failed",1);
         setFailRBit();
      }
   }
   else
      gDebug.msg("GreyImage::setPallette(): can't set pallette, ImageData has no
 pallette.",0);
   gDebug.msg("GreyImage::setPallette(): exiting",5);
}

Pallette* GreyImage::getPallette(void)
{
   gDebug.msg("GreyImage::getPallette(): exiting",5);
   return getImageData()->getPallette();
}
