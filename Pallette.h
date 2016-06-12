//  Pallette.h                     11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Defines the abstract class Pallette

#ifndef _PALLETTE_H_
#define _PALLETTE_H_


class Pallette
{
   int NoEntries;

  public:
   
   Pallette();
   Pallette(int n);
   virtual ~Pallette();
   int  getNoEntries();
   void setNoEntries(int n);
   virtual Pallette* copyMe(void)=0;
};

#endif
