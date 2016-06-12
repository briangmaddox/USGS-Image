#include "Image/CoreImageData.h"
#include "Image/RGBPallette.h"
#include "Image/GreyPallette.h"
#include "Image/RGBPixel.h"
#include "Image/GreyPixel.h"
#include "Image/ImageDebug.h"

  
CoreImageData::CoreImageData(long w, long h,unsigned char p, int q)
{
   quantum = q;
   unsetHasPallette();
   setBitsPerSample(8);
   setSamplesPerPixel(3);
   setPhotometric(p);
   setWidth(w);
   setHeight(h);

   if (gDebug.getDebugLevel() > 3)
   {
      cout << "CoreImageData::CoreImageData(): quantum is ";
      cout << quantum << endl;
      cout << "                                photometric is ";
      cout << (int) p << endl;
   }
   data = new unsigned char[getQuantum()*w*h];
   setRandomAccessFlags(rpixel|rrow|rrect|wpixel|wrow|wrect);
   gDebug.msg("CoreImageData::CoreImageData(): exiting ctor",5);
}

CoreImageData::~CoreImageData()
{
   if (data!=NULL) delete[] data;
   gDebug.msg("CoreImageData::~CoreImageData(): exiting dtor",5);
}

int CoreImageData::getQuantum(void)
{
   return quantum;
}

Pallette* CoreImageData::getPallette(void)
{
   int l = getWidth()*getHeight();
   int i;
   int k,x, found;
   RGBPixel* rgbbuf = NULL;
   RGBPixel* rgbtemp = NULL;
   GreyPixel* greybuf = NULL;
   GreyPixel* greytemp = NULL;
   Pallette* pal = NULL;
   unsigned char red,green,blue,grey;
   unsigned char photo = getPhotometric();

   x = 0;
   for (i=0;i<l;i++)
   {
       found = 0; 
       for (k=0;k<x;k++)
       {
           if (photo==DRG_RGB)
           {
              red   = rgbbuf[k].getRed();
              green = rgbbuf[k].getGreen();
              blue  = rgbbuf[k].getBlue();
              if ((data[3*i]==red)&&
                  (data[3*i+1]==green)&&
                  (data[3*i+2]==blue))     found = 1;
           }
           else if (photo==DRG_GREY)
           {
              grey = greybuf[k].getGrey();
              if (data[i] == grey) found = 1;
           }
           else
           {
              gDebug.msg("CoreImageData::getPallette(): can't handle format",1);
              setNoDataBit();
              return NULL;
           }
       }
       if (!found)
       {
          if (photo==DRG_RGB)
          {
             rgbtemp = new RGBPixel[x+1];
             for (k=0;k<x;x++)
                  rgbtemp[k] = rgbbuf[k];
             rgbtemp[x].setRed(data[3*i]); 
             rgbtemp[x].setGreen(data[3*i+1]); 
             rgbtemp[x].setBlue(data[3*i+2]); 
             if (rgbbuf) delete[] rgbbuf;
             rgbbuf = rgbtemp;
             rgbtemp = NULL;
          }
          else if (photo==DRG_GREY)
          {
             greytemp = new GreyPixel[x+1];
             for (k=0;k<x;k++)
                 greytemp[k] = greybuf[k];
             greytemp[x].setGrey(data[i]);
             if (greybuf) delete[] greybuf;
             greybuf = greytemp;
             greytemp = NULL;
          }
       }
   }
   if (photo == DRG_RGB)
   {
      pal = new RGBPallette(x,rgbbuf);
      if (rgbbuf) delete[] rgbbuf;
   }
   else if (photo == DRG_GREY)
   {
      pal = new GreyPallette(x,greybuf);
      if (greybuf) delete[] greybuf;
   }

   return pal;
}

void CoreImageData::setPallette(Pallette* p)
{
   gDebug.msg("CoreImageData::setPallette(): can't do this",1);
}


void* CoreImageData::getRawPixel(long x, long y)
{
   unsigned char* pix;

   pix = new unsigned char[getQuantum()];
   memcpy(pix,&data[getQuantum()*(y*getWidth()+x)],getQuantum());
    
   return pix;
}

void* CoreImageData::getRawScanline(long row)
{
   unsigned char* line;

   line = new unsigned char[getQuantum()*getWidth()];
   memcpy(line,&data[getQuantum()*row*getWidth()],getQuantum()*getWidth());

   return line;
}

void* CoreImageData::getRawRectangle(long x1, long y1, long x2, long y2)
{
   unsigned char* rect;
   int w,h;
   int i;
   w = x2-x1+1;
   h = y2-y1+1;

   rect = new unsigned char[getQuantum()*w*h];
   if ((w==getWidth())&&(h==getHeight()))
      memcpy(rect,data,getQuantum()*w*h);
   else if (w==getWidth())
      memcpy(rect,&(data[y1*getWidth()*getQuantum()]),getQuantum()*w*h);
   else
   {
      for (i=y1;i<=y2;i++)
         memcpy(&(rect[(i-y1)*getQuantum()*w]),
                &(data[(i*getWidth()+x1)*getQuantum()]),
                w*getQuantum());
   }

   return rect;
}

void CoreImageData::putRawPixel(void* p, long x,long y)
{
  //   unsigned char* pix = (unsigned char *) p;

   memcpy(&data[getQuantum()*(y*getWidth()+x)],p,getQuantum());
}

void CoreImageData::putRawScanline(void* p,long row)
{
  //   unsigned char* line = (unsigned char *) p;
  memcpy(&data[getQuantum()*row*getWidth()],p,getQuantum()*getWidth());
}

void CoreImageData::putRawRectangle(void* p,long x1,long y1,long x2,long y2)
{
   unsigned char* rect=(unsigned char*) p;
   int w,h,i;
   int y;
   w = x2-x1+1;
   h = y2-y1+1;

   if ((w==getWidth())&&(h==getHeight()))
      memcpy(data,rect,getQuantum()*w*h);
   else if (w==getWidth())
      memcpy(&(data[y1*getWidth()*getQuantum()]),rect,getQuantum()*w*h);
   else
      for (i=0,y=y1;i<h;i++,y++)
          memcpy(&(data[getQuantum()*(y*getWidth()+x1)]),
                 &(rect[getQuantum()*i*w]),
                 getQuantum()*w);
}


AbstractPixel* CoreImageData::getPixel(long x, long y)
{
   AbstractPixel* pix = NULL;
   if ((getPhotometric()==DRG_RGB)&&(getQuantum()==3))
   {
      pix = new RGBPixel(data[3*(y*getWidth()+x)],
                         data[3*(y*getWidth()+x)+1],
                         data[3*(y*getWidth()+x)+2]);
   }
   else if ((getPhotometric()==DRG_GREY)&& (getQuantum()==1))
      pix = new GreyPixel(data[y*getWidth()+x]);

   return pix;
}

AbstractPixel* CoreImageData::getScanline(long row)
{
   int i;
   AbstractPixel* pix = NULL;
   if ((getPhotometric()==DRG_RGB) && (getQuantum()==3))
   {
      RGBPixel* tmp;
      tmp = new RGBPixel[getWidth()];
      pix = tmp;
      for (i=0;i<getWidth();i++)
      {
          tmp[i].setRed(data[3*(row*getWidth()+i)]);
          tmp[i].setGreen(data[3*(row*getWidth()+i)+1]);
          tmp[i].setBlue(data[3*(row*getWidth()+i)+2]);
      }
   }
   else if ((getPhotometric()==DRG_GREY) && (getQuantum()==1))
   {
      GreyPixel* tmp;
      tmp = new GreyPixel[getWidth()];
      pix = tmp;
      for (i=0;i<getWidth();i++)
          tmp[i].setGrey(data[row*getWidth()+i]);
   }
         
   return pix;      
}

AbstractPixel* CoreImageData::getRectangle(long x1, long y1, long x2, long y2)
{
   int l = (x2-x1+1)*(y2-y1+1);
   int i;
   AbstractPixel *pix = NULL;
   unsigned char* tmpdata;
   tmpdata = (unsigned char*) getRawRectangle(x1,y1,x2,y2);
   if ((getPhotometric()==DRG_RGB) && (getQuantum()==3))
   {
      RGBPixel* tmp;
      tmp = new RGBPixel[l];
      pix = tmp;
      for (i=0;i<l;i++)
      {
          tmp[i].setRed(tmpdata[3*i]);
          tmp[i].setGreen(tmpdata[3*i+1]);
          tmp[i].setBlue(tmpdata[3*i+2]);
      }
   }
   else if ((getPhotometric()==DRG_GREY) && (getQuantum()==1))
   {
      GreyPixel* tmp;
      tmp = new GreyPixel[getWidth()];
      pix = tmp;
      for (i=0;i<l;i++)
          tmp[i].setGrey(tmpdata[i]);
   }
   return pix; 
}

void CoreImageData::putPixel(AbstractPixel* p, long x, long y)
{
   if ((getPhotometric()==DRG_RGB)&&(getQuantum()==3))
   {
      data[3*(y*getWidth()+x)]   = p->getRed();
      data[3*(y*getWidth()+x)+1] = p->getGreen();
      data[3*(y*getWidth()+x)+2] = p->getBlue();
   }
   else if ((getPhotometric()==DRG_GREY)&&(getQuantum()==1))
      data[y*getWidth()+x] = p->getGrey(); 
}

void CoreImageData::putScanline(AbstractPixel* s, long row)
{
   int i;
   if ((getPhotometric()==DRG_RGB)&&(getQuantum()==3))
   {
      RGBPixel* tmp = (RGBPixel *) s;
      for (i=0;i<getWidth();i++)
      {
          data[3*(row*getWidth()+i)]   = tmp[i].getRed(); 
          data[3*(row*getWidth()+i)+1] = tmp[i].getGreen(); 
          data[3*(row*getWidth()+i)+2] = tmp[i].getBlue(); 
      }
   }
   else if ((getPhotometric()==DRG_GREY)&&(getQuantum()==1))
   {
      GreyPixel *tmp = (GreyPixel *) s;
      for (i=0;i<getWidth();i++)
          data[row*getWidth()+i] = tmp[i].getGrey();
   }
}

void CoreImageData::putRectangle(AbstractPixel* r, long x1, long y1, 
                                 long x2, long y2)
{
   int i,w,h,l;
   unsigned char* rect;
   w = (x2-x1+1);
   h = (y2-y1+1);
   l = w*h;
   if ((getPhotometric()==DRG_RGB)&&(getQuantum()==3))
   {
      RGBPixel* tmp = (RGBPixel *) r;
      rect = new unsigned char[3*l];
      for (i=0;i<l;i++)
      {
          rect[3*i]   = tmp[i].getRed();
          rect[3*i+1] = tmp[i].getGreen();
          rect[3*i+2] = tmp[i].getBlue();
      }
      putRawRectangle(rect,x1,y1,x2,y2);
      delete[] rect;
   }
   else if ((getPhotometric()==DRG_GREY)&&(getQuantum()==1))
   {
      GreyPixel* tmp = (GreyPixel *) r;
      rect = new unsigned char[l];
      for (i=0;i<l;i++)
          rect[i] = tmp[i].getGrey();
      putRawRectangle(rect,x1,y1,x2,y2);
      delete[] rect;
   }
}


void CoreImageData::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
   if ((getPhotometric()==DRG_RGB)&&(getQuantum()==3))
   {
      pix->Red   = data[3*(y*getWidth()+x)];
      pix->Green = data[3*(y*getWidth()+x)+1];
      pix->Blue  = data[3*(y*getWidth()+x)+2];
   }
   else if ((getPhotometric()==DRG_GREY)&&(getQuantum()==1))
   {
      pix->Red   = data[y*getWidth()+x];
      pix->Green = data[y*getWidth()+x];
      pix->Blue  = data[y*getWidth()+x];
   } 
}

void CoreImageData::getRawRGBScanline(long row, RawRGBPixel* pix)
{
   int i;
   for (i=0;i<getWidth();i++)
   {
      if ((getPhotometric()==DRG_RGB)&&(getQuantum()==3))
      {
         pix[i].Red   = data[3*(row*getWidth()+i)];
         pix[i].Green = data[3*(row*getWidth()+i)+1];
         pix[i].Blue  = data[3*(row*getWidth()+i)+2];
      }
      else if ((getPhotometric()==DRG_GREY)&&(getQuantum()==1))
      {
         pix[i].Red   = data[row*getWidth()+i]; 
         pix[i].Green = data[row*getWidth()+i];
         pix[i].Blue  = data[row*getWidth()+i];
      }
   }
}

void CoreImageData::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                       RawRGBPixel* pix)
{
   unsigned char* rect;
   int w,h,i,l;
   w = (x2-x1+1);
   h = (y2-y1+1);
   l = w*h;
   rect = (unsigned char *) getRawRectangle(x1,y1,x2,y2);
   for (i=0;i<l;i++)
   {
      if ((getPhotometric()==DRG_RGB)&&(getQuantum()==3))
      {
         pix[i].Red   = data[3*l];
         pix[i].Green = data[3*l+1];
         pix[i].Blue  = data[3*l+2];
      }
      else if ((getPhotometric()==DRG_GREY)&&(getQuantum()==1))
      {
         pix[i].Red   = data[l];
         pix[i].Green = data[l];
         pix[i].Blue  = data[l];
      }
   }
}

bool CoreImageData::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
