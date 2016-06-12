//  RGBPixel.h                11 Jul 94
//  Author: Greg Martin, SES, MCMC, USGS

//  Defines class RGBPixel


#ifndef _RGBPIXEL_H_
#define _RGBPIXEL_H_

#include "Image/AbstractPixel.h"


class RGBPixel : public AbstractPixel
{
      unsigned short Red;
      unsigned short Green;
      unsigned short Blue;
   
   public:
      RGBPixel(void) {};
      RGBPixel(unsigned short r, unsigned short g, unsigned short b);
      virtual ~RGBPixel() {};

      virtual unsigned short getRed();         // These three work as expected
      virtual unsigned short getGreen();
      virtual unsigned short getBlue();

      virtual unsigned short getGrey();        // This uses the standard
                                               // transformation from RGB to
                                               // the Y (intensity) component
                                               // of the YIQ colorspace.


      virtual unsigned short getHue();
      virtual unsigned short getSaturation();
      virtual unsigned short getValue();

      virtual void setRed(unsigned short);     // These three work as expected
      virtual void setGreen(unsigned short);
      virtual void setBlue(unsigned short);

      virtual void setGrey(unsigned short);    // Sets Red = Green = Blue


      virtual void setHue(unsigned short);
      virtual void setSaturation(unsigned short);
      virtual void setValue(unsigned short);
};


#endif
