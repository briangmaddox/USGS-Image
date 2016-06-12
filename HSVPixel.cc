//  HSVPixel.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class HSVPixel 


#include "Image/HSVPixel.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <stdio.h>

HSVPixel::HSVPixel(unsigned short h, unsigned short s, unsigned short v)
{
    if ((s==0) && (h < 360))
    {
       char dmsg[100];
       sprintf(dmsg,"HSVPixel::HSVPixel(): Bad HSV value (%d,%d,%d)",h,s,v);
       gDebug.msg(dmsg,0);
    }
    Hue = h;
    Saturation = s;
    Value = v;
}

unsigned short HSVPixel::getRed(void)
{
   unsigned short red = 0;
   double h,s,v;
   double f;
   unsigned char i;

   s = Saturation;
   s = s/255.0;
   v = Value;
   v = v/255.0;

   if (Saturation == 0) 
       red = Value;
   else
   {
      if (Hue == 360)
         h = 0.0;
      else
         h = Hue; 
       
      h = h/60.0;
      i = (unsigned char) h;   
      f = h - i;
 
      switch (i)
      {
          case 0:
          case 5: red = (unsigned short) (255*v); 
                  break;
          case 1: red = (unsigned short) (255*(v*(1.0 - (s*f))));
                  break;
          case 2:
          case 3: red = (unsigned short) (255*(v*(1.0-s)));
                  break;
          case 4: red = (unsigned short) (255*(v*(1.0 - (s * (1.0-f)))));
                  break;
      }
   }  
          
   return red;
}

unsigned short HSVPixel::getGreen(void)
{
   unsigned short green = 0;
   double h,s,v;
   double f;
   unsigned char i;

   s = Saturation;
   s = s/255.0;
   v = Value;
   v = v/255.0;

   if (Saturation == 0) 
       green = Value;
   else
   {
      if (Hue == 360)
         h = 0.0;
      else
         h = Hue; 
       
      h = h/60.0;
      i = (unsigned char) h;   
      f = h - i;
 
      switch (i)
      {
          case 1:
          case 2: green = (unsigned short) (255*v); 
                  break;
          case 3: green = (unsigned short) (255*(v*(1.0 - (s*f))));
                  break;
          case 4:
          case 5: green = (unsigned short) (255*(v*(1.0-s)));
                  break;
          case 0: green = (unsigned short) (255*(v*(1.0 - (s * (1.0-f)))));
                  break;
      }
   }  
          
   return green;
}

unsigned short HSVPixel::getBlue(void)
{
   unsigned short blue = 0;
   double h,s,v;
   double f;
   unsigned char i;

   s = Saturation;
   s = s/255.0;
   v = Value;
   v = v/255.0;

   if (Saturation == 0) 
       blue = Value;
   else
   {
      if (Hue == 360)
         h = 0.0;
      else
         h = Hue; 
       
      h = h/60.0;
      i = (unsigned char) h;   
      f = h - i;
 
      switch (i)
      {
          case 3:
          case 4: blue = (unsigned short) (255*v); 
                  break;
          case 5: blue = (unsigned short) (255*(v*(1.0 - (s*f))));
                  break;
          case 0:
          case 1: blue = (unsigned short) (255*(v*(1.0-s)));
                  break;
          case 2: blue = (unsigned short) (255*(v*(1.0 - (s * (1.0-f)))));
                  break;
      }
   }  
          
   return blue;
}

unsigned short HSVPixel::getGrey(void)
{
    unsigned short red, green, blue;
    red = getRed();
    green = getGreen();
    blue = getBlue();

    return ((21*red+32*green+11*blue)/64);
}


unsigned short HSVPixel::getHue(void)
{
    return Hue;
} 
          

unsigned short HSVPixel::getSaturation(void)
{
    return Saturation;   
}          

unsigned short HSVPixel::getValue(void)
{
    return Value;
}

void HSVPixel::setRed(unsigned short)
{
    gDebug.msg("HSVPixel::setRed(): sorry, not implemented",0);
}

void HSVPixel::setGreen(unsigned short)
{
    gDebug.msg("HSVPixel::setGreen(): sorry, not implemented",0);
}

void HSVPixel::setBlue(unsigned short)
{
    gDebug.msg("HSVPixel::setBlue(): sorry, not implemented",0);
}
      
   
void HSVPixel::setHue(unsigned short h)
{
    Hue = h;
}

void HSVPixel::setSaturation(unsigned short s)
{
    Saturation = s;
}

void HSVPixel::setValue(unsigned short v)
{
    Value = v;
}

void HSVPixel::setGrey(unsigned short grey)
{
    Hue = 512;
    Saturation = 0;
    Value = grey;
}


