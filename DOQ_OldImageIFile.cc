//  DOQ_OldImageIFile.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS
//           Eric Heien   SES, MCMC, USGS
//  Implements member functions for the class DOQ_OldImageIFile 

#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include "Image/DOQ_OldImageIFile.h"
#include "Image/GreyPixel.h"
#include "Image/GreyPallette.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"


DOQ_OldImageIFile::DOQ_OldImageIFile(const char* fn): 
   ImageIFile(fn)
{

  char dmsg[80];
  HeaderDataRecordLength = 400;
  NumberHeaderRecords = 4;
  setPhotometric(DRG_GREY);
  Stream = new ifstream(fn,ios::in|ios::nocreate);

  if (!Stream->good())
  {
    sprintf(dmsg,"DOQ_OldImageIFile::DOQ_OldImageIFile(): cannot open %s.",fn);
    gDebug.msg(dmsg,1);
    setNoDataBit();
  }
 
  readHeader();
  setWidth(numsamples);
  setHeight(numlines);
  setRandomAccessFlags(rpixel|rrow|rrect);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(1);
  gDebug.msg("DOQ_OldImageIFile::DOQ_OldImageIFile(): exiting ctor",5);
}



DOQ_OldImageIFile::~DOQ_OldImageIFile()
{
  if (Stream) delete Stream;
  gDebug.msg("DOQ_OldImageIFile::DOQ_OldImageIFile(): exiting ctor",5);
}



Pallette* DOQ_OldImageIFile::getPallette()
{
  long x, i, j, k, l, found;
  GreyPixel* pixelbuff = NULL;
  GreyPixel* temp = NULL;
  GreyPallette* pal;
  unsigned char* line;

  x = 0;

  Stream->seekg(TopOfImageData);

  for (i=0; i<getHeight(); i++)
  {
    line = (unsigned char*)getRawScanline(i);
    for (j=0; j<getWidth();j++)
    {
      found = 0;
      for (k=0; k<x; k++)
      {
	if ( (line[j] = pixelbuff[k].getGrey()) )
	  found = 1;
      }
      
      if (!found)
      {
	temp = new GreyPixel[x+1];
	for (l=0; l<x; l++)
	  temp[l] = pixelbuff[l];
	temp[x].setGrey(line[j]);
	x++;
	if (pixelbuff) delete[] pixelbuff;
	pixelbuff = temp;
	temp = NULL;
      } // if(found)
    } // for(j)
    delete[] line;
  } // for(i)
  pal = new GreyPallette(x, pixelbuff);
  
  if (pixelbuff) delete[] pixelbuff;

  return((Pallette*)pal);
}

// THIS IS NOW IN ROW MAJOR ORDER

AbstractPixel* DOQ_OldImageIFile::getPixel(long y, long x)
{
  if ((y < 0) || (y > (getWidth()-1)) || (x < 0) || (x > (getHeight()-1)))
  {
    char dmsg[80];
    setFailRBit();
    sprintf(dmsg,
	    "DOQ_OldImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",
	    x,y);
    gDebug.msg(dmsg,1);
    return NULL;
  }

  GreyPixel *pixelbuf;
  long seekval;
  unsigned char pixel;

  pixelbuf = new GreyPixel;

  seekval = x*getWidth()+y;
  Stream->seekg(seekval+TopOfImageData);
  Stream->read(&pixel,sizeof(unsigned char));

  pixelbuf->setGrey((unsigned short)pixel);

  return (AbstractPixel*)pixelbuf;
}



AbstractPixel* DOQ_OldImageIFile::getScanline(long row)
{
  if ((row < 0) || (row > (getHeight()-1)))
  {
    char dmsg[80];
    setFailRBit();
    sprintf(dmsg,
	    "DOQ_OldImageIFile::getScanline(): scanline %ld out of bounds",
	    row);
    gDebug.msg(dmsg,1);
    return NULL;
  }

  unsigned char* line;
  GreyPixel* pixelbuf;
  long seekval;
  long i;

  pixelbuf = new GreyPixel[getWidth()];
  line = new unsigned char[getWidth()];

  seekval = row*getWidth();
  Stream->seekg(seekval+TopOfImageData);
  Stream->read(line,getWidth()*sizeof(unsigned char));

  for (i=0;i<getWidth();i++)
  {
      pixelbuf[i].setGrey(line[i]);
  }
  delete[] line;

  return((AbstractPixel*)pixelbuf);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

AbstractPixel* DOQ_OldImageIFile::getRectangle(long y1, long x1, long y2,
					       long x2)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[100];
    sprintf(dmsg,"DOQ_OldImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  
  GreyPixel *pixelbuf;
  unsigned char *buffer;
  long length, x, j, k, seekval;

  pixelbuf = new GreyPixel[(x2-x1+1)*(y2-y1+1)];
  length = y2-y1+1;
  k = 0;

  for(x=x1; x<=x2; x++)
  {
    buffer = new unsigned char[length];
    seekval = x*getWidth()+y1;
    Stream->seekg(seekval+TopOfImageData);
    Stream->read(buffer,(sizeof(unsigned char))*length);
    for(j=0; j<length; j++)
    {
      pixelbuf[k].setGrey(buffer[j]);
      k++;
    }
    delete[] buffer;
  }
  return((AbstractPixel*)pixelbuf);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void*  DOQ_OldImageIFile::getRawPixel(long y, long x)
{
  if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,
	    "DOQ_OldImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds",
	    x,y);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  
  long seekval;
  unsigned char* pixel = new unsigned char;
  

  seekval = x*getWidth()+y;
  Stream->seekg(seekval+TopOfImageData);
  Stream->read(pixel,sizeof(unsigned char));

  return pixel;
}



void*  DOQ_OldImageIFile::getRawScanline(long row)
{
  if ((row < 0) || (row > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,
	    "DOQ_OldImageIFile::getRawScanline(): scanline %ld out of bounds",
	    row);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  
  unsigned char* pixelbuf;
  long seekval;
  
  pixelbuf = new unsigned char[getWidth()];
 
  seekval = row*getWidth();
  
  Stream->seekg(seekval+TopOfImageData);
  Stream->read(pixelbuf,getWidth()*sizeof(unsigned char));

  return(pixelbuf);
}

// THIS IS NOW IN COLUMN MAJOR ORDER

void*  DOQ_OldImageIFile::getRawRectangle(long y1, long x1, long y2, long x2)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[120];
    sprintf(dmsg,"DOQ_OldImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }

  unsigned char* buffer;
  unsigned char* ptr;
  long length, x, k, seekval;
  
  length = y2-y1+1;
  k = 0;
  buffer = new unsigned char[(x2-x1+1)*(y2-y1+1)];

  for(x=x1; x<=x2; x++)
  {
    seekval = x*getWidth()+y1;
    ptr = &(buffer[(x-x1)*length]);
    Stream->seekg(seekval+TopOfImageData);
    Stream->read(ptr,sizeof(unsigned char)*length);
  }
  return buffer;
}

void DOQ_OldImageIFile::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
   if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
   {
     char dmsg[80];
     sprintf(dmsg,"DOQ_OldImageIFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return;
   }
   unsigned char* temp;
   
   temp = (unsigned char*) getRawPixel(x,y);
   pix->Red   = *temp;
   pix->Green = *temp;
   pix->Blue  = *temp;
   delete temp;
}

void DOQ_OldImageIFile::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
  if ((row < 0) || (row > getHeight()-1))
  {
    char dmsg[80];
    sprintf(dmsg,"DOQ_OldImageIFile::getRawRGBScanline(): scanline %ld out of bounds",row);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return;
  }
  unsigned char* temp;
  long i,w;

  temp = (unsigned char*) getRawScanline(row);

  w = getWidth();
  for (i=0;i<w;i++)
  {
    pixbuf[i].Red   = temp[i];
    pixbuf[i].Green = temp[i];
    pixbuf[i].Blue  = temp[i];
  }
  delete[] temp;
}

void DOQ_OldImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                       RawRGBPixel* pixbuf)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
      (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[120];
    sprintf(dmsg,"DOQ_OldImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return;
  }
  unsigned char* temp;
  int w,h,l,i;
  
  w = (x2-x1+1);
  h = (y2-y1+1);
  l = w*h;
  
  temp = (unsigned char*) getRawRectangle(x1,y1,x2,y2);
  
  for (i=0;i<l;i++)
  {
    pixbuf[i].Red   = temp[i];
    pixbuf[i].Green = temp[i];
    pixbuf[i].Blue  = temp[i];
  }
  delete[] temp;
}

void DOQ_OldImageIFile::readHeader(void)
{
  // Added this so that the strtod will work correctly.  Previously, it would
  // return numbers in decimal since the floating point in the Record DOQ uses
  // D+XX notation instead of E+XX for it's scientific notation BGM 23 April 1998
  char* _tfoo = NULL;

  Stream->read(buffer,400);
  buffer[400] = '\0';

  // Begin Record 1

  strncpy (quadname,buffer,38);
  quadname[38] = '\0';
  start = 38;
  strncpy (quadrant,buffer+start,2);
  quadrant[2] = '\0';
  start += 2; 
  strncpy (nation1,buffer+start,2);
  nation1[2] = '\0';
  start += 2;
  strncpy (nation2,buffer+start,2);
  nation2[2] = '\0';
  start += 2;
  strncpy (state1,buffer+start,2);
  state1[2] = '\0';
  start += 2;
  strncpy (state2,buffer+start,2); 
  state2[2] = '\0';
  start += 2;
  strncpy (state3,buffer+start,2);
  state3[2] = '\0';
  start += 2;
  strncpy (state4,buffer+start,2);
  state4[2] = '\0';
  start += 2;
  strncpy (st1co1,buffer+start,3);
  st1co1[3] = '\0';
  start += 3;
  strncpy (st1co2,buffer+start,3);
  st1co2[3] = '\0';
  start += 3;
  strncpy (st1co3,buffer+start,3);
  st1co3[3] = '\0';
  start += 3;
  strncpy (st1co4,buffer+start,3);
  st1co4[3] = '\0';
  start += 3;
  strncpy (st1co5,buffer+start,3);
  st1co5[3] = '\0';
  start += 3;
  strncpy (st2co1,buffer+start,3);
  st2co1[3] = '\0';
  start += 3;
  strncpy (st2co2,buffer+start,3);
  st2co2[3] = '\0';
  start += 3;
  strncpy (st2co3,buffer+start,3);
  st2co3[3] = '\0';
  start += 3;
  strncpy (st2co4,buffer+start,3);
  st2co4[3] = '\0';
  start += 3;
  strncpy (st2co5,buffer+start,3);
  st2co5[3] = '\0';
  start += 3;
  strncpy (st3co1,buffer+start,3);
  st3co1[3] = '\0';
  start += 3;
  strncpy (st3co2,buffer+start,3);
  st3co2[3] = '\0';
  start += 3;
  strncpy (st3co3,buffer+start,3);
  st3co3[3] = '\0';
  start += 3;
  strncpy (st3co4,buffer+start,3);
  st3co4[3] = '\0';
  start += 3;
  strncpy (st3co5,buffer+start,3);
  st3co5[3] = '\0';
  start += 3;
  strncpy (st4co1,buffer+start,3);
  st4co1[3] = '\0';
  start += 3;
  strncpy (st4co2,buffer+start,3);
  st4co2[3] = '\0';
  start += 3;
  strncpy (st4co3,buffer+start,3);
  st4co3[3] = '\0';
  start += 3;
  strncpy (st4co4,buffer+start,3);
  st4co4[3] = '\0';
  start += 3;
  strncpy (st4co5,buffer+start,3);
  st4co5[3] = '\0';
  start += 3;
  strncpy (reserved1a,buffer+start,2);
  reserved1a[2] = '\0';
  start += 2;
  strncpy (filler,buffer+start,23);
  filler[23] = '\0';
  start += 23;
  strncpy (producer,buffer+start,4);
  producer[4] = '\0';
  start += 4;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  datorder = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  numlines = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  numsamples = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  bandtyps = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  elevstor = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  bestore = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+2];
  buffer[start+2] = '\0';
  vertdat = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+2];
  buffer[start+2] = '\0';
  prhordat = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+2];
  buffer[start+2] = '\0';
  sdhordat = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  angle = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  grid = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  zone = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  xyunits = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcorsw[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcorsw[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcornw[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcornw[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcorne[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcorne[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcorse[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prqcorse[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  strncpy(reserved1b,buffer+start,1);
  reserved1b[1] = '\0';

  // End Record 1
  HeaderRecordPad = (numsamples-HeaderDataRecordLength); 
  Stream->seekg(HeaderRecordPad,ios::cur);
  Stream->read(buffer,400);
  buffer[400] = '\0';
  start = 0;

  // Begin Record 2

  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxforma = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxformb = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxformc = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxformd = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxforme = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxformf = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxformx = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  prxformy = strtod(buffer+start,&ret); 
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcorsw[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcorsw[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcornw[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcornw[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcorne[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcorne[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcorse[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdqcorse[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  strncpy(reserved2,buffer+start,16);
  reserved2[16] = '\0';

  // End Record 2

  Stream->seekg(HeaderRecordPad,ios::cur);
  Stream->read(buffer,400);
  buffer[400] = '\0';
  start = 0;

  // Begin Record 3

  temp = buffer[start+24];
  buffer[start+24] = '\0';
  sdxforma = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  sdxformb = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  sdxformc = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  sdxformd = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  sdxforme = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  sdxformf = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  sdxformx = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  sdxformy = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclssw[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclssw[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclsnw[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclsnw[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclsne[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclsne[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclsse[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  prqclsse[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclssw[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclssw[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclsnw[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclsnw[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclsne[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclsne[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclsse[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  sdqclsse[1] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  if ( (_tfoo = strstr( (buffer + start), "D+")) != NULL) // found a D, replace
    *_tfoo = 'E';
  x1y1[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  if ( (_tfoo = strstr((buffer + start), "D+")) != NULL) // found a D, replace
    *_tfoo = 'E';
  x1y1[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  xnyn[0] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24];
  buffer[start+24] = '\0';
  xnyn[1] = strtod(buffer+start,&ret);
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+16];
  buffer[start+16] = '\0';
  strncpy (reserved3,buffer+start,16);
  reserved3[16] = '\0'; 

  // End Record 3

  Stream->seekg(HeaderRecordPad,ios::cur);
  Stream->read(buffer,400);
  buffer[400] = '\0';
  start = 0;

  // Begin Record 4

  temp = buffer[start+3];
  buffer[start+3] = '\0';
  elevunit = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;  
  temp = buffer[start+10];  
  buffer[start+10] = '\0';
  minelev = strtod(buffer+start,&ret);
  buffer[start+10] = temp;
  start += 10;  
  temp = buffer[start+10];  
  buffer[start+10] = '\0';
  maxelev = strtod(buffer+start,&ret);
  buffer[start+10] = temp;
  start += 10;  
  temp = buffer[start+12];  
  buffer[start+12] = '\0';
  xgrndres = strtod(buffer+start,&ret);
  buffer[start+12] = temp;
  start += 12;  
  temp = buffer[start+12];  
  buffer[start+12] = '\0';
  ygrndres = strtod(buffer+start,&ret);
  buffer[start+12] = temp;
  start += 12;  
  temp = buffer[start+12];  
  buffer[start+12] = '\0';
  zgrndres = strtod(buffer+start,&ret);
  buffer[start+12] = temp;
  start += 12;  
  temp = buffer[start+12];  
  buffer[start+12] = '\0';
  xpixres = strtod(buffer+start,&ret);
  buffer[start+12] = temp;
  start += 12;  
  temp = buffer[start+12];  
  buffer[start+12] = '\0';
  ypixres = strtod(buffer+start,&ret);
  buffer[start+12] = temp;
  start += 12;  
  temp = buffer[start+12];  
  buffer[start+12] = '\0';
  zpixres = strtod(buffer+start,&ret);
  buffer[start+12] = temp;
  start += 12;  
  temp = buffer[start+5];  
  buffer[start+5] = '\0';
  maxconin = atoi(buffer+start);
  buffer[start+5] = temp;
  start += 5;  
  temp = buffer[start+1];  
  buffer[start+1] = '\0';
  maxconun = atoi(buffer+start);
  buffer[start+1] = temp;
  start ++;  
  temp = buffer[start+5];  
  buffer[start+5] = '\0';
  minconin = atoi(buffer+start);
  buffer[start+5] = temp;
  start += 5;  
  temp = buffer[start+1];  
  buffer[start+1] = '\0';
  minconun = atoi(buffer+start);
  buffer[start+1] = temp;
  start ++;  
  temp = buffer[start+2];  
  buffer[start+2] = '\0';
  susareac = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;  
  temp = buffer[start+6];  
  buffer[start+6] = '\0';
  horizacc = strtod(buffer+start,&ret);
  buffer[start+6] = temp;
  start += 6;  
  temp = buffer[start+6];  
  buffer[start+6] = '\0';
  vertacc = strtod(buffer+start,&ret);
  buffer[start+6] = temp;
  start += 6;  
  temp = buffer[start+4];  
  buffer[start+4] = '\0';
  numhortp = atoi(buffer+start);
  buffer[start+4] = temp;
  start += 4;  
  temp = buffer[start+2];  
  buffer[start+2] = '\0';
  pixproal = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;  
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  strncpy (prodsyst,buffer+start,24);
  prodsyst[24] = '\0';
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  proddate[0] = atoi(buffer+start);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+2];
  buffer[start+2] = '\0';
  proddate[1] = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+2];
  buffer[start+2] = '\0';
  proddate[2] = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  strncpy (filmtype,buffer+start,24);
  filmtype[24] = '\0';
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  strncpy (photoid,buffer+start,24);
  photoid[24] = '\0';
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  moscode = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+2]; 
  buffer[start+2] = '\0';
  strncpy (loffflag,buffer+start,2);
  loffflag[2] = '\0';
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+4];
  buffer[start+4] = '\0';
  photodat[0] = atoi(buffer+start);
  buffer[start+4] = temp;
  start += 4;
  temp = buffer[start+2];
  buffer[start+2] = '\0';
  photodat[1] = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+2];
  buffer[start+2] = '\0';
  photodat[2] = atoi(buffer+start);
  buffer[start+2] = temp;
  start += 2;
  temp = buffer[start+8];
  buffer[start+8] = '\0';
  focallen = strtod(buffer+start,&ret);
  buffer[start+8] = temp;
  start += 8;
  temp = buffer[start+10];
  buffer[start+10] = '\0';
  flyhgt = atoi(buffer+start);
  buffer[start+10] = temp;
  start += 10;
  temp = buffer[start+24]; 
  buffer[start+24] = '\0';
  strncpy (scantype,buffer+start,24);
  scantype[24] = '\0';
  buffer[start+24] = temp;
  start += 24;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  xyscres[0] = strtod(buffer+start,&ret);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  xyscres[1] = strtod(buffer+start,&ret);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  xyssres[0] = strtod(buffer+start,&ret);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  xyssres[1] = strtod(buffer+start,&ret);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+3];
  buffer[start+3] = '\0';
  radres = atoi(buffer+start);
  buffer[start+3] = temp;
  start += 3;
  temp = buffer[start+6];
  buffer[start+6] = '\0';
  resmpres = strtod(buffer+start,&ret);
  buffer[start+6] = temp;
  start += 6;
  temp = buffer[start+1];
  buffer[start+1] = '\0';
  compflag = buffer[start];
  buffer[start+1] = temp;
  start ++;
  temp = buffer[start+9];
  buffer[start+9] = '\0';
  compfilesize = atoi(buffer+start);
  buffer[start+9] = temp;
  start =+ 9;
  temp = buffer[start+20];
  buffer[start+20] = '\0';
  strncpy (compalgor,buffer+start,20);
  compalgor[20] = '\0';
  buffer[start+20] = temp;
  start += 20;
  temp = buffer[start+1];
  buffer[start+1] = '\0';
  comptype = buffer[start];
  buffer[start+1] = temp;
  start ++;
  temp = buffer[start+4];
  buffer[start+4] = '\0';
  compcontrol = atoi (buffer+start); 
  buffer[start+4] = temp;
  start += 4;
  temp = buffer[start+103];
  buffer[start+103] = '\0';
  strncpy (reserved4,buffer+start,103);
  reserved4[103] = '\0';
  buffer[start+103] = temp;

  // End Record 4

  Stream->seekg(HeaderRecordPad,ios::cur);
  TopOfImageData = Stream->tellg();
}


char*   DOQ_OldImageIFile::getQuadName(void)
{  char* temp = new char[strlen(quadname)+1];
   strcpy (temp,quadname);
   return temp;
}

char*   DOQ_OldImageIFile::getQuadrant(void)
{
  char* temp = new char[strlen(quadrant)+1];
  strcpy (temp,quadrant);
  return temp;
}

char*   DOQ_OldImageIFile::getNation1(void)
{
  char* temp = new char[strlen(nation1)+1];
  strcpy (temp,nation1);
  return temp;
}

char*   DOQ_OldImageIFile::getNation2(void)
{
  char* temp = new char[strlen(nation2)+1];
  strcpy (temp,nation2);
  return temp;
}

char*   DOQ_OldImageIFile::getState1(void)
{
  char* temp = new char[strlen(state1)+1];
  strcpy (temp,state1);
  return temp;
}

char*   DOQ_OldImageIFile::getState2(void)
{
  char* temp = new char[strlen(state2)+1];
  strcpy (temp,state2);
  return temp;
}

char*   DOQ_OldImageIFile::getState3(void)
{
  char* temp = new char[strlen(state3)+1];
  strcpy (temp,state3);
  return temp;
}

char*   DOQ_OldImageIFile::getState4(void)
{
  char* temp = new char[strlen(state4)+1];
  strcpy (temp,state4);
  return temp;
}


char*   DOQ_OldImageIFile::getSt1co1(void)
{
  char* temp = new char[strlen(st1co1)+1];
  strcpy (temp,st1co1);
  return temp;
}

char*   DOQ_OldImageIFile::getSt1co2(void)
{
  char* temp = new char[strlen(st1co2)+1];
  strcpy (temp,st1co2);
  return temp;
}

char*   DOQ_OldImageIFile::getSt1co3(void)
{
  char* temp = new char[strlen(st1co4)+1];
  strcpy (temp,st1co4);
  return temp;
}

char*   DOQ_OldImageIFile::getSt1co4(void)
{
  char* temp = new char[strlen(st1co4)+1];
  strcpy (temp,st1co4);
  return temp;
}


char*   DOQ_OldImageIFile::getSt1co5(void)
{
  char* temp = new char[strlen(st1co5)+1];
  strcpy (temp,st1co5);
  return temp;
}


char*   DOQ_OldImageIFile::getSt2co1(void)
{
  char* temp = new char[strlen(st2co1)+1];
  strcpy (temp,st2co1);
  return temp;
}


char*   DOQ_OldImageIFile::getSt2co2(void)
{
  char* temp = new char[strlen(st2co2)+1];
  strcpy (temp,st2co2);
  return temp;
}


char*   DOQ_OldImageIFile::getSt2co3(void)
{
  char* temp = new char[strlen(st2co3)+1];
  strcpy (temp,st2co3);
  return temp;
}


char*   DOQ_OldImageIFile::getSt2co4(void)
{
  char* temp = new char[strlen(st2co4)+1];
  strcpy (temp,st2co4);
  return temp;
}


char*   DOQ_OldImageIFile::getSt2co5(void)
{
  char* temp = new char[strlen(st2co5)+1];
  strcpy (temp,st2co5);
  return temp;
}


char*   DOQ_OldImageIFile::getSt3co1(void)
{
  char* temp = new char[strlen(st3co1)+1];
  strcpy (temp,st3co1);
  return temp;
}


char*   DOQ_OldImageIFile::getSt3co2(void)
{
  char* temp = new char[strlen(st3co2)+1];
  strcpy (temp,st3co2);
  return temp;
}


char*   DOQ_OldImageIFile::getSt3co3(void)
{
  char* temp = new char[strlen(st3co3)+1];
  strcpy (temp,st3co3);
  return temp;
}


char*   DOQ_OldImageIFile::getSt3co4(void)
{
  char* temp = new char[strlen(st3co4)+1];
  strcpy (temp,st3co4);
  return temp;
}


char*   DOQ_OldImageIFile::getSt3co5(void)
{
  char* temp = new char[strlen(st3co5)+1];
  strcpy (temp,st3co5);
  return temp;
}


char*   DOQ_OldImageIFile::getSt4co1(void)
{
  char* temp = new char[strlen(st4co1)+1];
  strcpy (temp,st4co1);
  return temp;
}


char*   DOQ_OldImageIFile::getSt4co2(void)
{
  char* temp = new char[strlen(st4co2)+1];
  strcpy (temp,st4co2);
  return temp;
}


char*   DOQ_OldImageIFile::getSt4co3(void)
{
  char* temp = new char[strlen(st4co3)+1];
  strcpy (temp,st4co3);
  return temp;
}


char*   DOQ_OldImageIFile::getSt4co4(void)
{
  char* temp = new char[strlen(st4co4)+1];
  strcpy (temp,st4co4);
  return temp;
}


char*   DOQ_OldImageIFile::getSt4co5(void)
{
  char* temp = new char[strlen(st4co5)+1];
  strcpy (temp,st4co5);
  return temp;
}

char*   DOQ_OldImageIFile::getFiller(void)
{
  char* temp = new char[strlen(filler)+1];
  strcpy (temp,filler);
  return temp;
}


char*   DOQ_OldImageIFile::getProducer(void)
{
  char*temp = new char[strlen(producer)+1];
  strcpy (temp,producer);
  return temp;
}


int    DOQ_OldImageIFile::getDatorder(void)   {return datorder;}
int    DOQ_OldImageIFile::getNumlines(void)   {return numlines;}
int    DOQ_OldImageIFile::getNumsamples(void) {return numsamples;}
int    DOQ_OldImageIFile::getBandtyps(void)   {return bandtyps;}
int    DOQ_OldImageIFile::getElevstor(void)   {return elevstor;}
int    DOQ_OldImageIFile::getBestore(void)    {return bestore;}
int    DOQ_OldImageIFile::getVertdat(void)    {return vertdat;} 
int    DOQ_OldImageIFile::getPrhordat(void)   {return prhordat;}
int    DOQ_OldImageIFile::getSdhordat(void)   {return sdhordat;}
double DOQ_OldImageIFile::getAngle(void)      {return angle;} 
int    DOQ_OldImageIFile::getGrid(void)       {return grid;}
int    DOQ_OldImageIFile::getZone(void)       {return zone;}
int    DOQ_OldImageIFile::getXyunits(void)    {return xyunits;}
double DOQ_OldImageIFile::getPrqcorsw(int i)  {return prqcorsw[i];}
double DOQ_OldImageIFile::getPrqcornw(int i)  {return prqcornw[i];}
double DOQ_OldImageIFile::getPrqcorne(int i)  {return prqcorne[i];}
double DOQ_OldImageIFile::getPrqcorse(int i)  {return prqcorse[i];}
double DOQ_OldImageIFile::getPrxforma(void)   {return prxforma;}
double DOQ_OldImageIFile::getPrxformb(void)   {return prxformb;}
double DOQ_OldImageIFile::getPrxformc(void)   {return prxformc;}
double DOQ_OldImageIFile::getPrxformd(void)   {return prxformd;}
double DOQ_OldImageIFile::getPrxforme(void)   {return prxforme;}
double DOQ_OldImageIFile::getPrxformf(void)   {return prxformf;}
double DOQ_OldImageIFile::getPrxformx(void)   {return prxformx;}
double DOQ_OldImageIFile::getPrxformy(void)   {return prxformy;}
double DOQ_OldImageIFile::getSdqcorsw(int i)  {return sdqcorsw[i];}
double DOQ_OldImageIFile::getSdqcornw(int i)  {return sdqcornw[i];}
double DOQ_OldImageIFile::getSdqcorne(int i)  {return sdqcorne[i];}
double DOQ_OldImageIFile::getSdqcorse(int i)  {return sdqcorse[i];}
double DOQ_OldImageIFile::getSdxforma(void)   {return sdxforma;}
double DOQ_OldImageIFile::getSdxformb(void)   {return sdxformb;}
double DOQ_OldImageIFile::getSdxformc(void)   {return sdxformc;}
double DOQ_OldImageIFile::getSdxformd(void)   {return sdxformd;}
double DOQ_OldImageIFile::getSdxforme(void)   {return sdxforme;}
double DOQ_OldImageIFile::getSdxformf(void)   {return sdxformf;}
double DOQ_OldImageIFile::getSdxformx(void)   {return sdxformx;}
double DOQ_OldImageIFile::getSdxformy(void)   {return sdxformy;}
int    DOQ_OldImageIFile::getPrqclssw(int i)  {return prqclssw[i];}
int    DOQ_OldImageIFile::getPrqclsnw(int i)  {return prqclsnw[i];}
int    DOQ_OldImageIFile::getPrqclsne(int i)  {return prqclsne[i];}
int    DOQ_OldImageIFile::getPrqclsse(int i)  {return prqclsse[i];}
int    DOQ_OldImageIFile::getSdqclssw(int i)  {return sdqclssw[i];} 
int    DOQ_OldImageIFile::getSdqclsnw(int i)  {return sdqclsnw[i];}
int    DOQ_OldImageIFile::getSdqclsne(int i)  {return sdqclsne[i];}
int    DOQ_OldImageIFile::getSdqclsse(int i)  {return sdqclsse[i];}
double DOQ_OldImageIFile::getX1y1(int i)      {return x1y1[i];}
double DOQ_OldImageIFile::getXnyn(int i)      {return xnyn[i];}
int    DOQ_OldImageIFile::getElevunit(void)   {return elevunit;}
double DOQ_OldImageIFile::getMinelev(void)    {return minelev;}
double DOQ_OldImageIFile::getMaxelev(void)    {return maxelev;}
double DOQ_OldImageIFile::getXgrndres(void)   {return xgrndres;}
double DOQ_OldImageIFile::getYgrndres(void)   {return ygrndres;}
double DOQ_OldImageIFile::getZgrndres(void)   {return zgrndres;}
double DOQ_OldImageIFile::getXpixres(void)    {return xpixres;}
double DOQ_OldImageIFile::getYpixres(void)    {return ypixres;}
double DOQ_OldImageIFile::getZpixres(void)    {return zpixres;}
int    DOQ_OldImageIFile::getMaxconin(void)   {return maxconin;}
int    DOQ_OldImageIFile::getMaxconun(void)   {return maxconun;} 
int    DOQ_OldImageIFile::getMinconin(void)   {return minconin;}
int    DOQ_OldImageIFile::getMinconun(void)   {return minconun;}
int    DOQ_OldImageIFile::getSusareac(void)   {return susareac;}
double DOQ_OldImageIFile::getHorizacc(void)   {return horizacc;}
double DOQ_OldImageIFile::getVertacc(void)    {return vertacc;}
int    DOQ_OldImageIFile::getNumhortp(void)   {return numhortp;}
int    DOQ_OldImageIFile::getPixproal(void)   {return pixproal;}

char*   DOQ_OldImageIFile::getProdsyst(void)   
{
  char* temp = new char[strlen(prodsyst)+1];
  strcpy (temp,prodsyst);
  return temp;
}


int    DOQ_OldImageIFile::getProddate(int i)   {return proddate[i];}
char*   DOQ_OldImageIFile::getFilmtype(void)
{
  char* temp = new char[strlen(filmtype)+1];
  strcpy (temp,filmtype);
  return temp;
}


char*   DOQ_OldImageIFile::getPhotoid(void)
{
  char* temp = new char[strlen(photoid)+1];
  strcpy (temp,photoid);
  return temp;
}


int    DOQ_OldImageIFile::getMoscode(void)   {return moscode;}
char*   DOQ_OldImageIFile::getLoffflag(void)
{
  char* temp = new char[strlen(loffflag)+1];
  strcpy (temp,loffflag);
  return temp;
}


int    DOQ_OldImageIFile::getPhotodat(int i)  {return photodat[i];}
double DOQ_OldImageIFile::getFocallen(void)  {return focallen;} 
int    DOQ_OldImageIFile::getFlyhgt(void)    {return flyhgt;}
char*   DOQ_OldImageIFile::getScantype(void)
{
  char* temp = new char[strlen(scantype)+1];
  strcpy (temp,scantype);
  return temp;
}

double DOQ_OldImageIFile::getXyscres(int i)      {return xyscres[i];}
double DOQ_OldImageIFile::getXyssres(int i)      {return xyssres[i];}
int    DOQ_OldImageIFile::getRadres(void)        {return radres;}
double DOQ_OldImageIFile::getResmpres(void)      {return resmpres;}
char   DOQ_OldImageIFile::getCompflag(void)      {return compflag;}
int    DOQ_OldImageIFile::getCompfilesize(void)  {return compfilesize;}
char*    DOQ_OldImageIFile::getCompalgor(void) 
{
  char* temp = new char[strlen(compalgor)+1];
  strcpy (temp,compalgor);
  return temp;
}

char   DOQ_OldImageIFile::getComptype(void)      {return comptype;}
int    DOQ_OldImageIFile::getCompcontrol(void)   {return compcontrol;}

bool DOQ_OldImageIFile::getOptimalRectangle(int &w, int &h)
{
   w = getWidth();
   h = 0;
   return true;
}
