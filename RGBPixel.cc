//  RGBPixel.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class RGBPixel 


#include "Image/RGBPixel.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <iostream.h>

RGBPixel::RGBPixel(unsigned short r, unsigned short g, unsigned short b)
{
   Red   = r;
   Green = g; 
   Blue  = b;
}

unsigned short RGBPixel::getRed(void)
{
    return Red;
}


unsigned short RGBPixel::getGreen(void)
{
    return Green;
}

unsigned short RGBPixel::getBlue(void)
{
    return Blue;
}

unsigned short RGBPixel::getGrey(void)
{
    //  This is the standard conversion from RGB to greyscale.
    //  It comes from the transformation from RGB to YIQ
    //  (the television standard).  The grayscale value is
    //  simply the Y (intensity) value from YIQ.

    return ((21*Red+32*Green+11*Blue)/64);
}


unsigned short RGBPixel::getHue(void)
{
    unsigned short max, min;
    double hh = 0;
    unsigned short h;
    double r, g, b, delta;

    if (Red > Green)
    {
       if (Red > Blue)
       {
          max = Red;
          if (Green > Blue)
             min = Blue;
          else
             min = Green;
       }
       else
       {
          max = Blue;
          min = Green;
       }
    } 
    else
    {
       if (Green > Blue)
       {
          max = Green;
          if (Blue > Red)
             min = Red;
          else
             min = Blue;
       }
       else
       {
          max = Blue;
          min = Red;
       }
    }

    r = Red;
    g = Green;
    b = Blue;

    r = r/255;
    g = g/255;
    b = b/255;

    delta = max-min;
    delta = delta/255;
    if (delta != 0)
    {
        if (Red == max)
           hh = (g-b)/delta;
        else if (Green == max)
           hh = 2.0 + (b-r)/delta;
        else if (Blue == max)
           hh = 4.0 + (r-g)/delta;
        hh *= 60.0;
        if (hh<0)
           hh += 360.0;
    }
    else 
       hh = 512.0;

    h = (unsigned short) hh;

    return h;
} 
          

unsigned short RGBPixel::getSaturation(void)
{
    unsigned short max, min;
    unsigned short s;

    if (Red > Green)
    {
       if (Red > Blue)
       {
          max = Red;
          if (Green > Blue)
             min = Blue;
          else
             min = Green;
       }
       else
       {
          max = Blue;
          min = Green;
       }
    } 
    else
    {
       if (Green > Blue)
       {
          max = Green;
          if (Blue > Red)
             min = Red;
          else
             min = Blue;
       }
       else
       {
          max = Blue;
          min = Red;
       }
    }

    if ((max-min)!=0)
       s = ((max-min)*255)/max;
    else
       s = 0;

    return s;
}

unsigned short RGBPixel::getValue(void)
{
    unsigned short max, min;

    if (Red > Green)
    {
       if (Red > Blue)
       {
          max = Red;
          if (Green > Blue)
             min = Blue;
          else
             min = Green;
       }
       else
       {
          max = Blue;
          min = Green;
       }
    } 
    else
    {
       if (Green > Blue)
       {
          max = Green;
          if (Blue > Red)
             min = Red;
          else
             min = Blue;
       }
       else
       {
          max = Blue;
          min = Red;
       }
    }
   
    return max;
}          
      
   
void RGBPixel::setRed(unsigned short r)
{
    Red = r;
}

void RGBPixel::setGreen(unsigned short g)
{
    Green = g;
}

void RGBPixel::setBlue(unsigned short b)
{
    Blue = b;
}

void RGBPixel::setGrey(unsigned short grey)
{
    Red   = grey;
    Green = grey;
    Blue  = grey;
}


void RGBPixel::setHue(unsigned short)
{
    gDebug.msg("RGBPixel::setHue(): Sorry, not implemented",0);
}

void RGBPixel::setSaturation(unsigned short)
{
    gDebug.msg("RGBPixel::setSaturation(): Sorry, not implemented",0);
}

void RGBPixel::setValue(unsigned short)
{
    gDebug.msg("RGBPixel::setValue(): Sorry, not implemented",0);
}
