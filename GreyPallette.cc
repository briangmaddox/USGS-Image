//  GreyPallette.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class GreyPallette 


#include "Image/GreyPallette.h"
#include "Image/GreyPixel.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdio.h>
#include <stdlib.h>


GreyPallette::GreyPallette(void)
:Pallette(0)
{
    Map=NULL;
}


GreyPallette::GreyPallette(int n, GreyPixel* m)
:Pallette(n)
{
    int i;

    Map = new GreyPixel[getNoEntries()];
    if (m != NULL)
    {
        for (i=0;i<getNoEntries();i++)
            Map[i].setGrey(m[i].getGrey());
    }
}

GreyPallette::~GreyPallette()
{
    if (Map != NULL)
       delete Map;
}

GreyPixel* GreyPallette::getEntry(int n)
{
    GreyPixel* temp = new GreyPixel(Map[n].getGrey());

    return temp;
}

void GreyPallette::setEntry(int n, GreyPixel* p)
{
    if (n<getNoEntries())
    {
       Map[n].setRed(p->getGrey()); 
    }
    else
    {
       char dmsg[80];
       sprintf(dmsg,"GreyPallette::setEntry(): pallette too small for %d entries",n);
       gDebug.msg(dmsg,0);
    }
}
    

GreyPixel GreyPallette::operator[](int n)
{
    return Map[n];
}
    

Pallette* GreyPallette::copyMe(void)
{
   GreyPallette* newpal;
   int i;

   newpal = new GreyPallette(getNoEntries(),NULL);

   for (i=0;i<getNoEntries();i++)
       newpal->setEntry(i,getEntry(i));

   return ((Pallette*)newpal);
}
