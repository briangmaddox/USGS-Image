//  RGBPallette.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class RGBPallette 


#include "Image/RGBPallette.h"
#include "Image/RGBPixel.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdlib.h>


RGBPallette::RGBPallette(void)
:Pallette(0)
{
   Map=NULL;
   gDebug.msg("RGBPallette::RGBPallette(): exiting ctor",5); 
}


RGBPallette::RGBPallette(int n, RGBPixel* m)
:Pallette(n)
{
   int i;
   char dmsg[80];
   sprintf(dmsg,"RGBPallette::RGBPallette(i,RP*): pallette has %d entries",n);
   gDebug.msg(dmsg,2);

   
   Map = new RGBPixel[getNoEntries()];
   if (m != NULL)
   {
       for (i=0;i<getNoEntries();i++)
       {
           Map[i].setRed(m[i].getRed());
           Map[i].setGreen(m[i].getGreen());
           Map[i].setBlue(m[i].getBlue());
       }
   }
   gDebug.msg("RGBPallette::RGBPallette(i,RP*): exiting ctor",5); 
}

RGBPallette::~RGBPallette()
{
   if (Map != NULL)
      delete[] Map;
   gDebug.msg("RGBPallette::~RGBPallette(): exiting dtor",5); 
}

RGBPixel* RGBPallette::getEntry(int n)
{
    RGBPixel* temp = new RGBPixel(Map[n].getRed(),
                                  Map[n].getGreen(),
                                  Map[n].getBlue());
    return temp;
}

void RGBPallette::setEntry(int n, RGBPixel* p)
{
    if (n<getNoEntries())
    {
       Map[n].setRed(p->getRed()); 
       Map[n].setGreen(p->getGreen()); 
       Map[n].setBlue(p->getBlue()); 
    }
    else
       gDebug.msg("RGBPallette::setEntry(): entry beyond bounds",1);
}
    

RGBPixel RGBPallette::operator[](int n)
{
    return Map[n];
}

Pallette* RGBPallette::copyMe(void)
{
   RGBPallette* newpal;
   RGBPixel* pix;
   int i;

   newpal = new RGBPallette(getNoEntries(),NULL);
 
   for (i=0;i<getNoEntries();i++)
   {
       pix = (RGBPixel *) getEntry(i);
       newpal->setEntry(i,pix); 
       delete pix;
   }
   
   return ((Pallette*)newpal);
}
    
