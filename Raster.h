#ifndef RASTER_H
#define RASTER_H

#include "mvc/Projection2D.h"
#include "Image/Image.h"

class Raster
{
   // Associates world coordinates with an Image.

   private:

   protected:
      Image* myimage;
      Projection2D* myproj;  // World coordinates to *image* coordinates.
      int mywidth, myheight; // Dimensions of image in pixels.
      double myxres, myyres; // Width and height of a pixel in world units.
      double mywx, mywy;     // World coords of a pixel in the image.
      int mywx_offset, mywy_offset;  // Location of that pixel in the image.

   public:

      Raster(Image* theimage, double xres, double yres,
             double wx, double wy, int wx_offset, int wy_offset);
         // theimage -- pointer to an Image that has already been init.
         // xres,yres -- resolution of an image pixel in world units.
         // wx,wy -- World coods of a pixel within the image.
         // wx_offset, wy_offset -- location of the (wx,wy) pixel in
         //    the image.
      ~Raster();

      bool getPixel(double x, double y, AbstractPixel** thepixel);
         // Caller must free space allocated for thepixel.
         // Returns a pixel value for World Space point (x,y).
         // Returns 'true' if (x,y) is in the image.
         //    Sets thepixel to pixel value at (x,y).
         // Returns 'false' if (x,y) is not in the image.
         //    Sets thepixel to NULL.

      bool getRawPixel(double x, double y, void** thepixel);
         // Caller must free space allocated for thepixel.
         // Returns a pixel value for World Space point (x,y).
         // Returns 'true' if (x,y) is in the image.
         //    Sets thepixel to pixel value at (x,y).
         // Returns 'false' if (x,y) is not in the image.
         //    Sets thepixel to NULL.

      bool setPixel(double x, double y, AbstractPixel* thepixel);
         // If (x,y) is a point in the image, sets the pixel that
         // (x,y) is in to thepixel. Note that (x,y) can be anywhere
         // inside a pixel (not just the center).
         // Returns 'true' if set was successful.
         // Returns 'false' if set failed (usually because (x,y) was
         // outside image.

      double getUpperLeftX(void);
         // Returns X component of world coordinates of top left (0,0)
         // image pixel. Coordinates are for pixel center. 

      double getUpperLeftY(void);
         // Returns Y component of world coordinates of top left (0,0)
         // image pixel. Coordinates are for pixel center.

      double getWidth(void);
         // Returns the width of the image in world units.

      double getHeight(void);
         // Returns the height of the image in world units.

      double getResolutionX(void);
         // Returns width of a pixel in world units.
 
      double getResolutionY(void);
         // Returns height of a pixel in world units.
      

      Image* getImage(void);
};

#endif // RASTER_H
