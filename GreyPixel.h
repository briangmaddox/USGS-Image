//  GreyPixel.h                 11 Jul 94
//  Author: Greg Martin, SES, MCMC, USGS

//  Defines the class GreyPixel


#ifndef _GREYPIXEL_H_
#define _GREYPIXEL_H_

#include "Image/AbstractPixel.h"

class GreyPixel : public AbstractPixel
{
  unsigned short Grey;

public:
  GreyPixel() {};
  GreyPixel(unsigned short g);
  virtual ~GreyPixel() {};

  /*
    These return the intensities as if the grey value came from
    only the color asked for
    */
  virtual unsigned short getRed();
  virtual unsigned short getGreen();
  virtual unsigned short getBlue();

  // This works as expected
  virtual unsigned short getGrey();

  virtual unsigned short getHue();
  virtual unsigned short getSaturation();
  virtual unsigned short getValue();


  // sets 11/32 of the grey
  virtual void setRed(unsigned short);
  // sets 1/2 of the grey
  virtual void setGreen(unsigned short);
  // sets 5/32 of the grey
  virtual void setBlue(unsigned short);

  // This works as expected
  virtual void setGrey(unsigned short);

  virtual void setHue(unsigned short);
  virtual void setSaturation(unsigned short);
  virtual void setValue(unsigned short);
};


#endif



