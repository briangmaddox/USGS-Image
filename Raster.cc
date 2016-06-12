#include <stdio.h>
#include "Image/Raster.h"
#include "mvc/RectProj2D.h"
#include "Image/ImageDebug.h"

Raster::Raster(Image* theimage, 
               double xres, double yres,
               double wx, double wy, int wx_offset, int wy_offset)
{
   // theimage -- pointer to an ImageData that has already been init.
   // xres,yres -- resolution of an image pixel in world units.
   // wx,wy -- World coods of a pixel within the image.
   // wx_offset, wy_offset -- location of the (wx,wy) pixel in
   //    the image.

   double wx1,wy1;  
   double wx2,wy2;

   myimage = theimage;
   mywidth = myimage -> getWidth();
   myheight = myimage -> getHeight();
   myxres = xres;
   myyres = yres;
   mywx = wx;
   mywy = wy;
   mywx_offset = wx_offset;
   mywy_offset = wy_offset;

   myproj = new RectProj2D();

   wx1 = mywx - (mywx_offset * myxres);
   wy1 = mywy + (mywy_offset * myyres);

   wx2 = mywx + ((mywidth - mywx_offset - 1) * myxres);
   wy2 = mywy - ((myheight - mywy_offset - 1) * myyres);

   // But, world space coords are for pixel center, so...
   wx1 = wx1 - (myxres / 2.0);
   wy1 = wy1 + (myyres / 2.0);

   wx2 = wx2 + (myxres / 2.0);
   wy2 = wy2 - (myyres / 2.0);
   
   if (gDebug.getDebugLevel() > 1)
   {
      cerr << "Raster::Raster(): (wx1,wy1) = (";
      cerr.precision(10);
      cerr << wx1 << "," << wy1 << ") (wx2,wy2) = (";
      cerr << wx2 << "," << wy2 << ")" << endl;
   }

   myproj->setWorldSpaceWin(wx1,wy1,wx2,wy2);
   myproj->setDisplayDevWin(0,0,mywidth,myheight);

   gDebug.msg("Raster::Raster(): exiting ctor",5);
}

Raster::~Raster()
{
   delete myproj;
   gDebug.msg("Raster::~Raster(): exiting dtor",5);
}   

Image* Raster::getImage(void)
{
   return(myimage);
}

bool Raster::getPixel(double x, double y, AbstractPixel** thepixel)
{
   // Returns a pixel value for World Space point (x,y).
   // Returns 'true' if (x,y) is in the image.
   //    Sets thepixel to pixel value at (x,y).
   // Returns 'false' if (x,y) is not in the image.
   //    Sets thepixel to NULL.

   int ix,iy;
   bool in_image;
  
   in_image = false;
   *thepixel = NULL;
   ix = myproj->getDisplayDeviceX(x,y);
   iy = myproj->getDisplayDeviceY(x,y);

   if ((ix >= 0) && (ix <= (mywidth-1)) &&
       (iy >= 0) && (iy <= (myheight-1)))
      {
         *thepixel = myimage -> getPixel(ix,iy);
         in_image = true;
      }
   return(in_image);
}

bool Raster::getRawPixel(double x, double y, void** thepixel)
{
   // Returns a pixel value for World Space point (x,y).
   // Returns 'true' if (x,y) is in the image.
   //    Sets thepixel to pixel value at (x,y).
   // Returns 'false' if (x,y) is not in the image.
   //    Sets thepixel to NULL.

   int ix,iy;
   bool in_image;
  
   in_image = false;
   *thepixel = NULL;
   ix = myproj->getDisplayDeviceX(x,y);
   iy = myproj->getDisplayDeviceY(x,y);

   if ((ix >= 0) && (ix <= (mywidth-1)) &&
       (iy >= 0) && (iy <= (myheight-1)))
      {
         *thepixel = myimage -> getRawPixel(ix,iy);
         in_image = true;
      }
   return(in_image);
}


bool Raster::setPixel(double x, double y, AbstractPixel* thepixel)
{
   // If (x,y) is a point in the image, sets the pixel that
   // (x,y) is in to thepixel. Note that (x,y) can be anywhere
   // inside a pixel (not just the center).
   // Returns 'true' if set was successful.
   // Returns 'false' if set failed (usually because (x,y) was
   // outside image.

   int ix,iy;
   bool in_image;

   in_image = false;
   ix = myproj->getDisplayDeviceX(x,y);
   iy = myproj->getDisplayDeviceY(x,y);

   if ((ix >= 0) && (ix <= (mywidth-1)) &&
       (iy >= 0) && (iy <= (myheight-1)))
      {
         myimage->putPixel(thepixel,ix,iy);
         in_image = true;
      }

   return(in_image);
}

double Raster::getUpperLeftX(void)
{
   // Returns X component of world coordinates of top left (0,0)
   // image pixel. Coordinates are for pixel center. 

   double x;

   x = myproj->getWorldSpaceX(0,0);
   x = x + (myxres / 2.0);
   return(x);

}

double Raster::getUpperLeftY(void)
{
   // Returns Y component of world coordinates of top left (0,0)
   // image pixel. Coordinates are for pixel center.

   double y;

   y = myproj->getWorldSpaceY(0,0);
   y = y - (myyres / 2.0);
   return(y);
}

double Raster::getWidth(void)
{
   // Returns the width of the image in world units.

   double x1,x2;

   x1 = myproj->getWorldSpaceX(0,0);
   x2 = myproj->getWorldSpaceX(mywidth-1,0);

   return (x2 - x1 + myxres);
}

double Raster::getHeight(void)
{
   // Returns height of a pixel in world units.

   double y1,y2;

   y1 = myproj->getWorldSpaceY(0,0);
   y2 = myproj->getWorldSpaceY(0,myheight-1);

   return (y1 - y2 + myyres);
}

double Raster::getResolutionX(void)
{
   // Returns width of a pixel in world units.

   return (myxres);
}
 
double Raster::getResolutionY(void)
{
   // Returns height of a pixel in world units.

   return (myyres);
}
