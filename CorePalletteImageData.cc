#include "Image/CorePalletteImageData.h"
#include "Image/RGBPallette.h"
#include "Image/GreyPallette.h"
#include "Image/RGBPixel.h"
#include "Image/GreyPixel.h"
#include "Image/ImageDebug.h"

  
CorePalletteImageData::CorePalletteImageData(long w, long h,unsigned char p):
CoreImageData(w,h,p,1)
{
   pal = NULL;
   gDebug.msg("CorePalletteImageData::CorePalletteImageData(): exiting ctor",5);
}

CorePalletteImageData::~CorePalletteImageData()
{
   if (pal != NULL) delete pal;
   gDebug.msg("CorePalletteImageData::~CorePalletteImageData(): exiting dtor",5);
}


Pallette* CorePalletteImageData::getPallette(void)
{
   Pallette* tmp = NULL;
   //   int i;
   if (pal != NULL)
      tmp = pal->copyMe();
   else
      tmp = NULL;

   return tmp;
}

void CorePalletteImageData::setPallette(Pallette* p)
{
   if (pal!=NULL) delete pal;
   pal = p->copyMe();
}


AbstractPixel* CorePalletteImageData::getPixel(long x, long y)
{
   AbstractPixel* pix = NULL;
   unsigned char* pixdata = (unsigned char *) getRawPixel(x,y);
 
   if (pal!=NULL)
   {
      if (getPhotometric()==DRG_RGB)
      {
         RGBPallette* tmp = (RGBPallette *)pal;
         pix = new RGBPixel((*tmp)[*pixdata].getRed(),
                            (*tmp)[*pixdata].getGreen(),
                            (*tmp)[*pixdata].getBlue());
      }
      else if (getPhotometric()==DRG_GREY)
      {
         GreyPallette* tmp = (GreyPallette *)pal;
         pix = new GreyPixel((*tmp)[*pixdata].getGrey());
      }
   }

   delete[] pixdata;
   return pix;
}

AbstractPixel* CorePalletteImageData::getScanline(long row)
{
   int i;
   AbstractPixel* pix = NULL;
   unsigned char* pixdata = (unsigned char *) getRawScanline(row);
   if (pal != NULL)
   {
      if (getPhotometric()==DRG_RGB)
      {
         RGBPallette* tmp = (RGBPallette *)pal;
         pix = new RGBPixel[getWidth()];
         for (i=0;i<getWidth();i++)
         {
             ((RGBPixel *)pix)[i].setRed((*tmp)[pixdata[i]].getRed());
             ((RGBPixel *)pix)[i].setGreen((*tmp)[pixdata[i]].getGreen());
             ((RGBPixel *)pix)[i].setBlue((*tmp)[pixdata[i]].getBlue());
         }
      }
      else if (getPhotometric()==DRG_GREY)
      {
         GreyPallette* tmp = (GreyPallette *)pal;
         pix = new GreyPixel[getWidth()];
         for (i=0;i<getWidth();i++)
             ((GreyPixel *)pix)[i].setGrey((*tmp)[pixdata[i]].getGrey());
      }
   } 
         
   delete[] pixdata; 
   return pix;      
}

AbstractPixel* CorePalletteImageData::getRectangle(long x1, long y1, 
                                                   long x2, long y2)
{
   int i;
   int l = (x2-x1+1)*(y2-y1+1);
   AbstractPixel *pix = NULL;
   unsigned char *pixdata = (unsigned char *) getRawRectangle(x1,y1,x2,y2);
   if (pal != NULL)
   {
      if (getPhotometric()==DRG_RGB)
      { 
         RGBPallette* tmp = (RGBPallette *)pal;
         pix = new RGBPixel[l];
         for (i=0;i<l;i++)
         {
             ((RGBPixel *)pix)[i].setRed((*tmp)[pixdata[i]].getRed());
             ((RGBPixel *)pix)[i].setGreen((*tmp)[pixdata[i]].getGreen());
             ((RGBPixel *)pix)[i].setBlue((*tmp)[pixdata[i]].getBlue());
         }
      }
      else if (getPhotometric()==DRG_GREY) 
      {
         GreyPallette* tmp = (GreyPallette *)pal;
         pix = new GreyPixel[l]; 
         for (i=0;i<l;i++)
             ((GreyPixel *)pix)[i].setGrey((*tmp)[pixdata[i]].getGrey());
      }
   }
   
   delete[] pixdata; 
   return pix;
  
}

void CorePalletteImageData::putPixel(AbstractPixel* p, long x, long y)
{
/*
   int index;
   if (getPhotometric()==DRG_RGB)
   {
      index = getPalletteIndex((RGBPixel *)p);
   }
   else if (getPhotometric()==DRG_GREY)
   {
      index = getPalletteIndex((GreyPixel *)p);
   }
   else
   {
      setFailWBit();
      return;
   }
   putRawPixel(&index,x,y);
*/
}

void CorePalletteImageData::putScanline(AbstractPixel* s, long row)
{
/*
   int i;
   unsigned char *line = new unsigned char[getWidth()];

   if (getPhotometric()==DRG_RGB)
   {
      RGBPixel *pix = (RGBPixel *)p;
      for (i=0;i<getWidth();i++)
          line[i] = getPalletteIndex(&(pix[i]));
   }
   else if (getPhotometric()==DRG_GREY)
   {
      GreyPixel *pix = (GreyPixel *)p;
      for (i=0;i<getWidth();i++)
          line[i] = getPalletteIndex(&(pix[i]));
   }
   else
   {
      setFailWBit();
      delete[] line;
      return;
   }

   putRawScanline(line,row);
   delete[] line;
*/
}
      
          
void CorePalletteImageData::putRectangle(AbstractPixel* r, long x1, long y1, 
                                 long x2, long y2)
{
/*
   int i;
   int l = (x2-x1+1)*(y2-y1+1);
   unsigned char *rect = new unsigned char[l];

   if (getPhotometric()==DRG_RGB)
   {
      RGBPixel *pix = (RGBPixel *)r;
      for (i=0;i<l;i++)
          rect[i] = getPalletteIndex(&(pix[i]));
   }
   else if (getPhotometric()==DRG_GREY)
   {
      GreyPixel *pix = (GreyPixel *)r;
      for (i=0;i<l;i++)
          rect[i] = getPalletteIndex(&(pix[i]));
   }
   else
   {
      setFailWBit();
      delete[] rect;
      return;
   }

   putRawRectangle(rect,x1,y1,x2,y2);
   delete[] rect;
*/
}


void CorePalletteImageData::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
   unsigned char *pixel;
  
   pixel = (unsigned char *) getRawPixel(x,y);

   if (getPhotometric()==DRG_RGB)
   {
      pix->Red   = (*((RGBPallette *)pal))[*pixel].getRed();
      pix->Green = (*((RGBPallette *)pal))[*pixel].getGreen();
      pix->Blue  = (*((RGBPallette *)pal))[*pixel].getBlue();
   }
   else if (getPhotometric()==DRG_GREY)
   {
      pix->Red   = (*((GreyPallette *)pal))[*pixel].getRed();
      pix->Green = (*((GreyPallette *)pal))[*pixel].getGreen();
      pix->Blue  = (*((GreyPallette *)pal))[*pixel].getBlue();
   }
  
   delete pixel;
}

void CorePalletteImageData::getRawRGBScanline(long row, RawRGBPixel* pix)
{
   int i;
   unsigned char *line;

   line = (unsigned char *) getRawScanline(row);
   if (getPhotometric()==DRG_RGB)
   {
      for (i=0;i<getWidth();i++)
      {
         pix[i].Red   = (*((RGBPallette *)pal))[line[i]].getRed();
         pix[i].Green = (*((RGBPallette *)pal))[line[i]].getGreen();
         pix[i].Blue  = (*((RGBPallette *)pal))[line[i]].getBlue();
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      for (i=0;i<getWidth();i++)
      {
         pix[i].Red   = (*((GreyPallette *)pal))[line[i]].getRed();
         pix[i].Green = (*((GreyPallette *)pal))[line[i]].getGreen();
         pix[i].Blue  = (*((GreyPallette *)pal))[line[i]].getBlue();
      }
   }

   delete[] line;
}

void CorePalletteImageData::getRawRGBRectangle(long x1, long y1,
                                               long x2, long y2,
                                               RawRGBPixel* pix)
{
   int i;
   int l = (x2-x1+1)*(y2-y1+1);
   unsigned char *rect;

   rect = (unsigned char *) getRawRectangle(x1,y1,x2,y2);

   if (getPhotometric()==DRG_RGB)
   {
      for (i=0;i<l;i++)
      {
         pix[i].Red   = (*((RGBPallette *)pal))[rect[i]].getRed();
         pix[i].Green = (*((RGBPallette *)pal))[rect[i]].getGreen();
         pix[i].Blue  = (*((RGBPallette *)pal))[rect[i]].getBlue();
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      for (i=0;i<l;i++)
      {
         pix[i].Red   = (*((GreyPallette *)pal))[rect[i]].getRed();
         pix[i].Green = (*((GreyPallette *)pal))[rect[i]].getGreen();
         pix[i].Blue  = (*((GreyPallette *)pal))[rect[i]].getBlue();
      }
   }
  
   delete[] rect;
}

int CorePalletteImageData::getPalletteIndex(RGBPixel *p)
{
/*
   int i;
   int index;
   

   for (i=0;i<pal->getNoEntries();i++)
*/
  return -100;
}

int CorePalletteImageData::getPalletteIndex(GreyPixel *p)
{
  return -100;
}
