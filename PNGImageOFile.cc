
//  PNGImageOFile.cc                         30 Jan 96
//  Author:  Greg Martin SES, MCMC, USGS
//  Implements member functions for the class PNGImageOFile 

#include "Image/PNGImageOFile.h"
#include "Image/RGBPixel.h"
#include "Image/GreyPixel.h"
#include "Image/AbstractPixel.h"
#include "Image/ImageCommon.h"
#include "Image/Pallette.h"
#include "Image/RGBPallette.h"
#include <stdlib.h>
#include "Image/ImageDebug.h"
#include <stdio.h>


// Constructor for PNGImageOFile.h
PNGImageOFile::PNGImageOFile(const char* pfilename, long pw, long ph,
                             int ptype):
   ImageOFile(pfilename,pw,ph)
{
   int i;

   _png = new png_struct;
   _pngInfo = new png_info;
   _pngF = fopen(pfilename,"wb");
   _rows = NULL;
   if (_pngF == (FILE *)NULL)
   {
      gDebug.msg("PNGImageOFile::PNGImageOFile(): can't open file",1);
      setNoDataBit();
   }
   if (setjmp(_png->jmpbuf))
   {
      png_write_destroy(_png);
      delete _png;
      _png = NULL;
      delete _pngInfo;
      _pngInfo = NULL;
      fclose(_pngF);
      setNoDataBit();
      gDebug.msg("PNGImageOFile::PNGImageOFile(): error opening file",1);
   }
   if (good())
   {
      png_info_init(_pngInfo);
      png_write_init(_png);
      png_init_io(_png,_pngF);
     
      _pngInfo->width = getWidth();
      _pngInfo->height = getHeight();
      _pngInfo->bit_depth = 8;
      _pngInfo->compression_type = 0;
      _pngInfo->filter_type = 0;
      _pngInfo->interlace_type = 0;
      _pngInfo->valid = 0;

      if (ptype==0)        // pallette color
      {
         _pngInfo->color_type = PNG_COLOR_TYPE_PALETTE;
         _pngInfo->valid |= PNG_INFO_PLTE;
         setHasPallette();
         setSamplesPerPixel(1);
         setBitsPerSample(8);
         _colorsFound = 0;
         _pngInfo->palette = new png_color[256];
         _pngInfo->num_palette = 256;
      }
      else if (ptype == 1) // RGB color
      {
         _pngInfo->color_type = PNG_COLOR_TYPE_RGB;
         unsetHasPallette();
         setSamplesPerPixel(3);
         setBitsPerSample(8);
      }
      else if (ptype == 2) // Greyscale
      {
         _pngInfo->color_type = PNG_COLOR_TYPE_GRAY;
         unsetHasPallette();
         setSamplesPerPixel(1);
         setBitsPerSample(8);
      }
      else                 // dunno...
      {
         setNoDataBit();
      }
      _rows = new (unsigned char *)[getHeight()];
      for (i=0;i<getHeight();i++)
         _rows[i] = new unsigned char[getWidth()*getSamplesPerPixel()];
   } 
   setRandomAccessFlags(wpixel|wrow|wrect);
   gDebug.msg("PNGImageOFile::PNGImageOFile(c*,l,l): exiting ctor",5);
}   


PNGImageOFile::~PNGImageOFile()
{
   int i;
   if (hasPallette())
   {
      for (i=0;i<_pal->getNoEntries();i++)
      {
         _pngInfo->palette[i].red = (*_pal)[i].getRed();
         _pngInfo->palette[i].green = (*_pal)[i].getGreen();
         _pngInfo->palette[i].blue = (*_pal)[i].getBlue();
      }
   }
   png_write_info(_png,_pngInfo);
   png_write_image(_png,_rows);
   png_write_end(_png,_pngInfo);
   png_write_destroy(_png);
   if (hasPallette())
   {
      if (_pngInfo!=NULL)
      {
         if (_pngInfo->palette!=NULL) delete[] _pngInfo->palette;
         delete _pngInfo;
      }
   }
   if (_png!=NULL)
      delete _png;
   fclose(_pngF);
   if (_rows!=NULL)
   {
      for (i=0;i<getHeight();i++)
          delete[] _rows[i];
      delete[] _rows;
   }
   gDebug.msg("PNGImageOFile::~PNGImageOFile(): exiting dtor",5);
}


void PNGImageOFile::putPixel(AbstractPixel* p, long x, long y) 
{
   unsigned char *pix = NULL;
   if (getPhotometric()==DRG_RGB)
   {
      if (hasPallette())
      {
         pix = new unsigned char;
         *pix = getPalletteIndex((RGBPixel *)p);
      }
      else
      {
         pix = new unsigned char[3];
         pix[0] = p->getRed();
         pix[1] = p->getGreen();
         pix[2] = p->getBlue();
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      pix = new unsigned char;
      *pix = p->getGrey();
   }
   putRawPixel(pix,x,y);
   delete[] pix;
}


void PNGImageOFile::putScanline(AbstractPixel* p, long row)
{
   unsigned char *line = NULL;
   int i;
   if ((row < 0) || (row > getHeight()-1))
   {
      char dmsg[80];
      sprintf(dmsg,"PNGImageOFile::putScanline(): scanline %ld out of bounds",row);
      gDebug.msg(dmsg,1);
      setFailWBit();
      return;
   }
   if (getPhotometric()==DRG_RGB)
   {
      RGBPixel *tmp = (RGBPixel *)p;
      if (hasPallette())
      {
         line = new unsigned char[getWidth()];
         for (i=0;i<getWidth();i++)
            line[i] = getPalletteIndex(&tmp[i]); 
      }
      else
      {
         line = new unsigned char[3*getWidth()];
         for (i=0;i<getWidth();i++)
         {
            line[3*i]   = tmp[i].getRed();
            line[3*i+1] = tmp[i].getBlue();
            line[3*i+2] = tmp[i].getGreen();
         }
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      GreyPixel *tmp = (GreyPixel *)p;
      line = new unsigned char[getWidth()];
      for (i=0;i<getWidth();i++)
         line[i] = tmp[i].getGrey();
   }
   putRawScanline(line,row);
   delete[] line;
}

void PNGImageOFile::putRectangle(AbstractPixel* p, long x1, long y1,
                                 long x2, long y2)
{
   int i;
   int w,h,l;
   unsigned char *rect = NULL; 
   w = x2 - x1 + 1;
   h = y2 - y1 + 1;
   l = w*h;
   if (getPhotometric()==DRG_RGB)
   {
      RGBPixel *tmp = (RGBPixel *)p;
      if (hasPallette())
      {
         rect = new unsigned char[l];
         for (i=0;i<l;i++)
            rect[i] = getPalletteIndex(&tmp[i]); 
      }
      else
      {
         rect = new unsigned char[3*l];
         for (i=0;i<l;i++)
         {
            rect[3*i]   = tmp[i].getRed();
            rect[3*i+1] = tmp[i].getBlue();
            rect[3*i+2] = tmp[i].getGreen();
         }
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      GreyPixel *tmp = (GreyPixel *)p;
      rect = new unsigned char[l];
      for (i=0;i<l;i++)
         rect[i] = tmp[i].getGrey();
   }
   putRawRectangle(rect,x1,y1,x2,y2);
   delete[] rect;
}


void PNGImageOFile::putRawPixel(void* p, long x, long y)
{
   unsigned char *tmp = &_rows[y][x*getSamplesPerPixel()];
   memcpy(tmp,p,getSamplesPerPixel());
}


void PNGImageOFile::putRawScanline(void* p,long row)
{
   if ((row < 0) || (row > getHeight()-1))
   {
      char dmsg[80];
      sprintf(dmsg,"PNGImageOFile::putRawScanline(): scanline %ld out of bounds",row);
      gDebug.msg(dmsg,1);
      setFailWBit();
      return;
   }
   memcpy(_rows[row],p,getWidth()*getSamplesPerPixel());
   
}


void PNGImageOFile::putRawRectangle(void* p,long x1,long y1,long x2,long y2)
{
   int w,h;
   int i;
   w = x2 - x1 + 1;
   h = y2 - y1 + 1;

   for (i=0;i<h;i++)
      memcpy(&_rows[y2+i][x2],p,w*getSamplesPerPixel());
}


Pallette* PNGImageOFile::getPallette(void)
{
   Pallette *ret;
   if (hasPallette())
      ret = _pal->copyMe();
   else
      ret = NULL;
   
   return ret;
}


void PNGImageOFile::setPallette(Pallette* p)
{
   if (hasPallette())
   {
      if (_pal!=NULL) delete _pal;
      _pal = (RGBPallette *)p->copyMe();
   }
}
 


bool PNGImageOFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}

unsigned char PNGImageOFile::getPalletteIndex(RGBPixel *p)
{
   int i;
   bool foundIt;
   unsigned char index = 0;

   foundIt = false;
   for (i=0;(i<_colorsFound)&&!foundIt;i++)
   {
      if (((*_pal)[i].getRed() == p->getRed())&&
          ((*_pal)[i].getGreen() == p->getGreen())&&
          ((*_pal)[i].getBlue() == p->getBlue()))
      {
         foundIt = true;
         index = i;
      }
   }
   if (!foundIt)
   {
      if ((_colorsFound+1)<_pal->getNoEntries())
      {
         _pal->setEntry(_colorsFound,p);
         index = _colorsFound++;
      }
      else
      {
         // pallette is full, fix this later
         index = 0;
      }
   }
   
   return index;
}
