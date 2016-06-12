//  GreyPixel.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class GreyPixel 


#include "Image/GreyPixel.h"


GreyPixel::GreyPixel(unsigned short g)
{
    Grey = g;
}

void GreyPixel::setRed(unsigned short r)
{
    Grey = (21*r + 43*Grey)/64;
}

void GreyPixel::setGreen(unsigned short g)
{
    Grey = (32*g + 32*Grey)/64;
}

void GreyPixel::setBlue(unsigned short b)
{
    Grey = (11*b + 53*Grey)/64;
}

void GreyPixel::setGrey(unsigned short g)
{
    Grey = g;
}

void GreyPixel::setHue(unsigned short)
{
}

void GreyPixel::setSaturation(unsigned short)
{
}

void GreyPixel::setValue(unsigned short v)
{
    Grey = v;
}

unsigned short GreyPixel::getRed(void)
{
    return Grey;
}

unsigned short GreyPixel::getGreen(void)
{
    return Grey;
}

unsigned short GreyPixel::getBlue(void)
{
    return Grey;
}

unsigned short GreyPixel::getGrey(void)
{
   return Grey;
}


unsigned short GreyPixel::getHue(void)
{
   return 512;  // undefined hue...achromatic
}

unsigned short GreyPixel::getSaturation(void)
{
   return 0;    // 0 saturation for achromatic case
}

unsigned short GreyPixel::getValue(void)
{
   return Grey;  // Grey is the value for HSV
}

