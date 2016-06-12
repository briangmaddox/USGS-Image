//  RGBPallette.h                  11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Defines the class RGBPallette.



#ifndef _RGBPALLETTE_H_
#define _RGBPALLETTE_H_


#include "Image/RGBPixel.h"
#include "Image/Pallette.h"
#include <stdlib.h>

class RGBPallette : public Pallette
{
   RGBPixel* Map;

  public:
 
   RGBPallette();
   RGBPallette(int n,RGBPixel* m=NULL);
   virtual ~RGBPallette();
   RGBPixel* getEntry(int n);
   void   setEntry(int n,RGBPixel* p);
   RGBPixel operator[](int n);
   virtual Pallette* copyMe(void);
};

#endif
