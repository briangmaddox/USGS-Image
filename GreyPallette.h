// GreyPallette.h                    11 Jul 94
// Author:  Greg Martin  SES, MCMC, USGS

//  Defines the class GreyPallette

#ifndef _GREYPALLETTE_H_
#define _GREYPALLETTE_H_


#include "Image/GreyPixel.h"
#include "Image/Pallette.h"
#include <stdlib.h>

class GreyPallette : public Pallette
{
   GreyPixel* Map;

  public:
 
   GreyPallette();
   GreyPallette(int n,GreyPixel* m=NULL);
   virtual ~GreyPallette();

   GreyPixel* getEntry(int n);
   void   setEntry(int n,GreyPixel* p);
   GreyPixel operator[](int n);
   virtual Pallette* copyMe(void);
};

#endif
