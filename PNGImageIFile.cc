//  PNGImageIFile.cc                         12 Jan 96
//  Author:  Greg Martin SES, MCMC, USGS

//  Implements member functions for the class PNGImageIFile 

#include "Image/PNGImageIFile.h"
#include "Image/RGBPixel.h"
#include "Image/GreyPixel.h"
#include "Image/AbstractPixel.h"
#include "Image/ImageCommon.h"
#include "Image/RGBPallette.h"
#include "Image/Pallette.h"
#include <stdlib.h>
#include "Image/ImageDebug.h"
#include <stdio.h>



PNGImageIFile::PNGImageIFile(const char* pfilename):
    ImageIFile(pfilename)
{
   int i;
   RGBPixel *_tmp;

   _png     = new png_struct;
   _pngInfo = new png_info;
   _pngF    = fopen(pfilename,"rb");
   if (_pngF == (FILE *)NULL)
   {
      gDebug.msg("PNGImageIFile::PNGImageIFile(): can't open file",1);
      setNoDataBit(); 
   }
   if (setjmp(_png->jmpbuf))
   {
      png_read_destroy(_png,_pngInfo,(png_info *)0);
      delete _png;
      _png = NULL;
      delete _pngInfo;
      _pngInfo = NULL;
      fclose(_pngF);
      setNoDataBit(); 
      gDebug.msg("PNGImageIFile::PNGImageIFile(): error opening file",1);
   }
   if (good())
   {
      png_info_init(_pngInfo);
      png_read_init(_png);
      png_init_io(_png,_pngF);
      png_read_info(_png,_pngInfo);
      setWidth(_pngInfo->width);
      setHeight(_pngInfo->height);
      if (_pngInfo->color_type == PNG_COLOR_TYPE_PALETTE)
      {
         setHasPallette();
         setPhotometric(DRG_RGB);
         setBitsPerSample(8);
         setSamplesPerPixel(1);
         _pal = new RGBPallette(_pngInfo->num_palette);
         _tmp = new RGBPixel;
         for (i=0;i<_pngInfo->num_palette;i++)
         {
             _tmp->setRed(_pngInfo->palette[i].red);
             _tmp->setGreen(_pngInfo->palette[i].green);
             _tmp->setBlue(_pngInfo->palette[i].blue);
             _pal->setEntry(i,_tmp);
         }
         delete _tmp;
      }
      else if (_pngInfo->color_type == PNG_COLOR_TYPE_GRAY)
      {
         if (_pngInfo->bit_depth < 8)
            png_set_expand(_png);
         setPhotometric(DRG_GREY);
         setBitsPerSample(8);
         setSamplesPerPixel(1);
      }
      else if (_pngInfo->color_type == PNG_COLOR_TYPE_RGB)
      {
         setPhotometric(DRG_RGB);
         setBitsPerSample(8);
         setSamplesPerPixel(3);
      }
      else
      {
         gDebug.msg("PNGImageIFile::PNGImageIFile(): unknown color format",0);
      }
      _rows = new (unsigned char *)[getHeight()];
      for (i=0;i<getHeight();i++)
          _rows[i] = new unsigned char[getWidth()*getSamplesPerPixel()];
      png_read_image(_png,_rows); 
      png_read_end(_png,_pngInfo);
      png_read_destroy(_png,_pngInfo,(png_info *)0);
      delete _png;
      delete _pngInfo;
      fclose(_pngF);
   }
   setRandomAccessFlags(rpixel|rrow|rrect);
   gDebug.msg("PNGImageIFile::PNGImageIFile(): exiting ctor",5);
}   

PNGImageIFile::~PNGImageIFile()
{
   int i;
   if (_rows != NULL)
   {
      for (i=0;i<getHeight();i++)
         if (_rows[i] != NULL) delete[] _rows[i];
      delete[] _rows;
   }
   if (_pal != NULL)
      delete _pal;
}

// *********************************************************************

AbstractPixel* PNGImageIFile::getPixel(long x, long y)
{
   if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
   {
      char dmsg[80];
      sprintf(dmsg,"PNGImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return NULL;
   }

   unsigned char* buf;
   AbstractPixel *pixel = NULL;

   buf = (unsigned char *) getRawPixel(x,y);

   if (hasPallette())
   {
      pixel = new RGBPixel((*_pal)[*buf].getRed(),
                           (*_pal)[*buf].getGreen(),
                           (*_pal)[*buf].getBlue());
   }
   else if (getPhotometric()==DRG_RGB)
   {
      pixel = new RGBPixel(buf[0],buf[1],buf[2]);
   }
   else if (getPhotometric()==DRG_GREY)
   {
      pixel = new GreyPixel(*buf);
   }
 
   delete buf;

   return pixel;            // Return pixel pointer
}

// *********************************************************************

AbstractPixel* PNGImageIFile::getScanline(long row)
{
   if ((row < 0) || (row > getHeight()-1))
   {
      char dmsg[80];
      sprintf(dmsg,"PNGImageIFile::getScanline(): scanline %ld out of bounds",row);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return NULL;
   }
   unsigned char* buf;
   AbstractPixel *line = NULL;
   int i,w;
//   int k;

   w = getWidth();
   
   buf = (unsigned char *) getRawScanline(row);
   if (hasPallette())
   {
      line = new RGBPixel[getWidth()];
      for (i=0;i<w;i++)
      {
         line[i].setRed((*_pal)[buf[i]].getRed());
         line[i].setGreen((*_pal)[buf[i]].getGreen());
         line[i].setBlue((*_pal)[buf[i]].getBlue());
      }
   }
   else if (getPhotometric()==DRG_RGB)
   {
      line = new RGBPixel[getWidth()];
      for (i=0;i<w;i++)
      {
         line[i].setRed(buf[3*i]);
         line[i].setGreen(buf[3*i+1]);
         line[i].setBlue(buf[3*i+2]);
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      line = new GreyPixel[getWidth()];
      for (i=0;i<w;i++)
      {
         line[i].setGrey(buf[i]);
      }
   }
   delete[] buf;

   return line;
}

// *********************************************************************

AbstractPixel* PNGImageIFile::getRectangle(long x1, long y1, long x2, long y2)
{
   if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
   {
      char dmsg[100];
      sprintf(dmsg,"PNGImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return NULL;
   }

   int w, h, n, i;
   unsigned char* buf;
   AbstractPixel *rect = NULL;
   
   w = x2 - x1 + 1;
   h = y2 - y1 + 1;
   n = w*h;

   buf = (unsigned char *) getRawRectangle(x1,y1,x2,y2);

   if (hasPallette())
   {
      rect = new RGBPixel[n];
      for (i=0;i<n;i++)
      {
         rect[i].setRed((*_pal)[buf[i]].getRed());
         rect[i].setGreen((*_pal)[buf[i]].getGreen());
         rect[i].setBlue((*_pal)[buf[i]].getBlue());
      }
   }
   else if (getPhotometric()==DRG_RGB)
   {
      rect = new RGBPixel[n];
      for (i=0;i<n;i++)
      {
         rect[i].setRed(buf[3*i]);
         rect[i].setGreen(buf[3*i+1]);
         rect[i].setBlue(buf[3*i+2]);
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      rect = new GreyPixel[n];
      for (i=0;i<n;i++)
      {
         rect[i].setGrey(buf[i]);
      }
   }

   delete[] buf;

   return rect;
}

// *********************************************************************

void* PNGImageIFile::getRawPixel(long x, long y)
{
    if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"PNGImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    unsigned char* pixel = new unsigned char[getSamplesPerPixel()];

    memcpy(pixel,&(_rows[y][x]),getSamplesPerPixel()); 

    return pixel;
}

// *********************************************************************

void* PNGImageIFile::getRawScanline(long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"PNGImageIFile::getRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    unsigned char* retline = new unsigned char[getWidth()*getSamplesPerPixel()];

    memcpy(retline,_rows[row],getWidth()*getSamplesPerPixel()); 

    return retline;
}

// *********************************************************************

void* PNGImageIFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
    if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
       (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
       (x1>x2) || (y1>y2))
    {
       char dmsg[120];
       sprintf(dmsg,"PNGImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }

    long i,k;
    long rw, rh;
    unsigned char *tptr, *sptr;
    unsigned char* rect = new unsigned char[(x2-x1+1)*(y2-y1+1)];
//    char entry;

    rh = y2-y1+1;
    rw = x2-x1+1; 
    
    k=0;
    for (i=y1;i<=y2;i++)
    {
        sptr = &(_rows[i][x1]);
        tptr = &(rect[k]);
        memcpy(tptr,sptr,rw);
        k+=rw;
    } 
    return rect;
}

void PNGImageIFile::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
   if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
   {
      char dmsg[80];
      sprintf(dmsg,"PNGImageIFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }

   unsigned char* pixel;
 
   pixel = (unsigned char *) getRawPixel(x,y);

   if (hasPallette())
   {
      pix->Red   = (*_pal)[*pixel].getRed();
      pix->Green = (*_pal)[*pixel].getGreen();
      pix->Blue  = (*_pal)[*pixel].getBlue();
   }
   else if (getPhotometric()==DRG_RGB)
   {
      pix->Red   = pixel[0];
      pix->Green = pixel[1];
      pix->Blue  = pixel[2];
   }
   else if (getPhotometric()==DRG_GREY)
   {
      pix->Red   = *pixel;
      pix->Green = *pixel;
      pix->Blue  = *pixel;
   }

   delete pixel;
}

void PNGImageIFile::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
   if ((row < 0) || (row > getHeight()-1))
   {
      char dmsg[80];
      sprintf(dmsg,"PNGImageIFile::getRawRGBScanline(): scanline %ld out of bounds",row);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }
 
   unsigned char* line;
   int i, w;
 //  int k;

   w = getWidth();

   line = (unsigned char *) getRawScanline(row);

   if (hasPallette())
   {
      for (i=0;i<w;i++)
      {
          pixbuf[i].Red =   (*_pal)[line[i]].getRed();
          pixbuf[i].Green = (*_pal)[line[i]].getGreen();
          pixbuf[i].Blue =  (*_pal)[line[i]].getBlue();
      }
   }
   else if (getPhotometric()==DRG_RGB)
   {
      for (i=0;i<w;i++)
      {
         pixbuf[i].Red   = line[3*i];
         pixbuf[i].Green = line[3*i+1];
         pixbuf[i].Blue  = line[3*i+2];
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      for (i=0;i<w;i++)
      {
         pixbuf[i].Red   = line[i];
         pixbuf[i].Green = line[i];
         pixbuf[i].Blue  = line[i];
      }
   }

   delete[] line;
}

void PNGImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                        RawRGBPixel* pixbuf)
{
   if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
   {
      char dmsg[120];
      sprintf(dmsg,"PNGImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
      gDebug.msg(dmsg,1);
      setFailRBit();
      return;
   }

   unsigned char* line;
   int i, w,h,l;
//   int k;

   w = (x2-x1+1);
   h = (y2-y1+1);
   l = w*h;

   line = (unsigned char *) getRawRectangle(x1,y1,x2,y2);

   if (hasPallette())
   {
      for (i=0;i<l;i++)
      {
          pixbuf[i].Red   = (*_pal)[line[i]].getRed();
          pixbuf[i].Green = (*_pal)[line[i]].getGreen();
          pixbuf[i].Blue  = (*_pal)[line[i]].getBlue();
      }
   }
   else if (getPhotometric()==DRG_RGB)
   {
      for (i=0;i<l;i++)
      {
         pixbuf[i].Red   = line[3*i];
         pixbuf[i].Green = line[3*i+1];
         pixbuf[i].Blue  = line[3*i+2];
      }
   }
   else if (getPhotometric()==DRG_GREY)
   {
      for (i=0;i<l;i++)
      {
         pixbuf[i].Red   = line[i];
         pixbuf[i].Green = line[i];
         pixbuf[i].Blue  = line[i];
      }
   }

   delete[] line;
}

// *********************************************************************

Pallette* PNGImageIFile::getPallette(void)
{
   Pallette* pallette;

   if (hasPallette())
      pallette = _pal->copyMe();
   else
      pallette = NULL;

   return pallette; 
}

bool PNGImageIFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
