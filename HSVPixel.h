//   HSVPixel.h                     11 Jul 94
//   Author:  Greg Martin, SES, MCMC, USGS



#ifndef _HSVPIXEL_H_
#define _HSVPIXEL_H_

#include "Image/AbstractPixel.h"
/*
  Defines abstract class HSVPixel
  */
class HSVPixel : public AbstractPixel
{
  unsigned short Hue;
  unsigned short Saturation;
  unsigned short Value;

public:
  HSVPixel(void) {};
  HSVPixel(unsigned short h, unsigned short s, unsigned short v);
  virtual ~HSVPixel() {};

  virtual unsigned short getRed();
  virtual unsigned short getGreen();
  virtual unsigned short getBlue();

  virtual unsigned short getGrey();

  virtual unsigned short getHue();
  virtual unsigned short getSaturation();
  virtual unsigned short getValue();

  virtual void setRed(unsigned short);
  virtual void setGreen(unsigned short);
  virtual void setBlue(unsigned short);

  virtual void setGrey(unsigned short);

  virtual void setHue(unsigned short);
  virtual void setSaturation(unsigned short);
  virtual void setValue(unsigned short);
};


#endif
