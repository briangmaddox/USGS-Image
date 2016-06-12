//   AbstractPixel.h                     11 Jul 94
//   Author:  Greg Martin, SES, MCMC, USGS

//   Defines abstract class Pixel


#ifndef _ABSTRACTPIXEL_H_
#define _ABSTRACTPIXEL_H_

class AbstractPixel
{
public:
  virtual unsigned short getRed()=0;
  virtual unsigned short getGreen()=0;
  virtual unsigned short getBlue()=0;

  virtual unsigned short getGrey()=0;

  virtual unsigned short getHue()=0;
  virtual unsigned short getSaturation()=0;
  virtual unsigned short getValue()=0;

  virtual void setRed(unsigned short)=0;
  virtual void setGreen(unsigned short)=0;
  virtual void setBlue(unsigned short)=0;

  virtual void setGrey(unsigned short)=0;
  
  virtual void setHue(unsigned short)=0;
  virtual void setSaturation(unsigned short)=0;
  virtual void setValue(unsigned short)=0;
};


#endif
