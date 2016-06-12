//  GreyImageOFile.h                  11 Jul 94
//  Author:  Greg Martin, SES, MCMC, USGS

//  Defines the class GreyImageOFile

//  This class handles the writing of raw greyscale raster files.



#ifndef _GREYIMAGEOFILE_H_
#define _GREYIMAGEOFILE_H_

#include "Image/ImageOFile.h"
#include "Image/Pallette.h"
#include "Image/AbstractPixel.h"
#include <fstream.h>


class GreyImageOFile : public ImageOFile
{
   ofstream* Stream;
   char* headerFile;

  public:
  
   GreyImageOFile(const char* fn,const char* header, long w, long l);  // create
   GreyImageOFile(const char* fn, long w, long l);  // create
   virtual ~GreyImageOFile();


// Standard ImageOFile interface

// getPallette allocates memory, so this memory must be deleted
// by the user

   virtual Pallette* getPallette(void);
   virtual void      setPallette(Pallette* p);

//  Note:  The get/put functions all use column-major addressing (ie.
//         (x,y) means (column, row).

   virtual void   putPixel(AbstractPixel* p, long x, long y);
   virtual void   putScanline(AbstractPixel* s, long row);
   virtual void   putRectangle(AbstractPixel* r, long x1, long y1, long x2, long y2);

   virtual void   putRawPixel(void* p, long x,long y);
   virtual void   putRawScanline(void* p,long row);
   virtual void   putRawRectangle(void* p,long x1,long y1,long x2,long y2);

   virtual bool getOptimalRectangle(int &w, int &h);

  protected:
 
   void writeHeader(void);
};

#endif
