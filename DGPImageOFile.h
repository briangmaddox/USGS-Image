//  DGPImageOFile.h                   11 Jul 94
//  Author:  Greg Martin         

//  Defines the class DGPImageOFile.h

//  This class handles the writing of DGP format raster files.


#ifndef _DGPIMAGEOFILE_H_
#define _DGPIMAGEOFILE_H_


#include "Image/ImageOFile.h"
#include "Image/Pallette.h"
#include "Image/RGBPixel.h"
#include "Image/RGBPallette.h"
#include <fstream.h>

class DGPImageOFile : public ImageOFile
{
 private:

   ofstream*      Stream;
   char*          Version;
   char*          Name;
   double         XUpperLeft;
   double         YUpperLeft;
   double         PixelSize;
   unsigned long  NoColors;
   unsigned long  ColorsFound;
   RGBPallette*   Pal;

   long TopOfColormap;
   long TopOfData;


 public:

   DGPImageOFile(const char* fn, long w, long h);
   DGPImageOFile(const char* fn, long w, long h, unsigned long c,
                 double x, double y, double p);
   virtual ~DGPImageOFile();


// Standard ImageOFile interface.
// getPallette allocates memory, so the user must
// delete this memory.

   virtual Pallette* getPallette(void);
   virtual void setPallette(Pallette* p);

// get/put functions use column-major order, ie. (x,y) == (column,row)

   virtual void putRawPixel(void* p, long x, long y);
   virtual void putRawScanline(void* p, long row);
   virtual void putRawRectangle(void* p, long x1, long y1, long x2, long y2);

   virtual void putPixel(AbstractPixel* p, long x, long y);
   virtual void putScanline(AbstractPixel* p, long row);
   virtual void putRectangle(AbstractPixel* p, long x1, long y1, long x2, long y2);

   virtual bool getOptimalRectangle(int &w, int &h);

 protected:
  
   void writeHeader(void);
   unsigned char getPalletteIndex(RGBPixel* p);
   void initializePallette(void);
   void writePallette(void);

};
#endif
