//  DOQImageIFile.cc                         Aug 1997
//  Rewritten for new 12/96 Keyword Header DOQ Format:
//           Brian Maddox  SES, MCMC, USGS 16 Aug 1997
//  Implements member functions for the class DOQImageIFile 

#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include <strstream.h>
#include "Image/DOQImageIFile.h"
#include "Image/GreyPixel.h"
#include "Image/GreyPallette.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include "Image/RGBPixel.h"
#include "Image/RGBPallette.h"

DOQImageIFile::DOQImageIFile(const char* fn)
  : ImageIFile(fn)
{

  // Make sure we init some stuff to make sure it's NULLed
  for (int _mycount = 0; _mycount <= 2; _mycount++)
    bandContent[_mycount] = NULL;  // just set so we can test if filled later
  
  for (int _mycount = 0; _mycount <= 4; _mycount++)
  {
    nation[_mycount] = NULL;
    states[_mycount] = NULL;
    sourceImageID[_mycount] = NULL;
    sourceImageDate[_mycount] = NULL;
  }

  quadName = NULL;
  quadrant = NULL;
  prodDate = NULL;
  rasterOrder = NULL;
  bandOrganization = NULL;
  horizontalDatum = NULL;
  horizCoordSystem = NULL;
  horizontalUnits = NULL;
  secHorizDatum = NULL;
  imageSource = NULL;
  sourceDEMDate = NULL;
  agency = NULL;
  producer = NULL;
  productionSystem = NULL;
  compression = NULL;
  standardVersion = NULL;
  metaDataDate = NULL;
  _fileName = NULL;
  RGBOrdered = false;

  char dmsg[80];
  // Open the input stream
  Stream = new ifstream(fn,ios::in|ios::nocreate);

  if (!Stream->good()) // Something bad happened while opening
  {
    sprintf(dmsg,"DOQImageIFile::DOQImageIFile(): cannot open %s.",fn);
    gDebug.msg(dmsg,1);
    setNoDataBit();
  }
  else // could open the file ok so go on.
  {
    // Just store so it's easier to get to later on
    _fileName = new string(fn);

    if (readHeader() ) // Header info parsed so set some stuff
    {
      setWidth(samples);
      setHeight(lines);
      setRandomAccessFlags(rpixel | rrow | rrect);
      setHasPallette();
      setBitsPerSample(getBitsPerPixel());
      setSamplesPerPixel(1);
      if (*bandOrganization == "SINGLE FILE")
	setPhotometric(DRG_GREY);
      else if (*bandOrganization == "BIP")
      {
	setPhotometric(DRG_REAL_RGB);
	if ( (*bandContent[0] == "RED") && (*bandContent[1] == "GREEN")
	     && (*bandContent[2] == "BLUE") )
	  RGBOrdered = true; // this is so we don't waste time down the road
                             // checking if the bytes are in RGB order
      }
    }
    else // Not valid header information so might as well set fail bit
      setFailRBit();
  }

  gDebug.msg("DOQImageIFile::DOQImageIFile(): exiting ctor",5);
}


DOQImageIFile::~DOQImageIFile()
{
  // I test for NULL basically just for gcc for linux because it seg faults 
  // if you try to delete a NULL pointer (even though the C++ standard says
  // you should be able to
  if (Stream != NULL)
    delete Stream;
  if (_fileName != NULL)
    delete _fileName;
  if (quadName != NULL)
    delete quadName;
  if (quadrant != NULL)
    delete quadrant;
  if (prodDate != NULL)
    delete prodDate;
  if (rasterOrder != NULL)
    delete rasterOrder;
  if (bandOrganization != NULL)
    delete bandOrganization;
  for (int count = 0; count <= 2; count++)
    if (bandContent[count] != NULL)
      delete bandContent[count];
  if (horizontalDatum != NULL)
    delete horizontalDatum;
  if (horizontalUnits != NULL)
    delete horizontalUnits;
  if (secHorizDatum != NULL)
    delete secHorizDatum;
  if (horizCoordSystem != NULL)
    delete horizCoordSystem;
  for (int count = 0; count <= 4; count++)
    if (nation[count] != NULL)
      delete nation[count];
  for (int count = 0; count <= 4; count++)
    if (states[count] != NULL)
      delete states[count];
  if (imageSource != NULL)
    delete imageSource;
  for (int count = 0; count <= 4; count++)
    if (sourceImageID[count] != NULL)
      delete sourceImageID[count];
  for (int count = 0; count <= 4; count++)
    if (sourceImageDate[count] != NULL)
      delete sourceImageDate[count];
  if (sourceDEMDate != NULL)
    delete sourceDEMDate;
  if (agency != NULL)
    delete agency;
  if (producer != NULL)
    delete producer;
  if (productionSystem != NULL)
    delete productionSystem;
  if (compression != NULL)
    delete compression;
  if (standardVersion != NULL)
    delete standardVersion;
  if (metaDataDate != NULL)
    delete metaDataDate;
 
  gDebug.msg("DOQImageIFile::DOQImageIFile(): exiting dtor",5);
}



// This function creates a pallette by scanning the file.  Note that this
// is a slow operation and should be avoided, especially with the color
// DOQ data
Pallette* DOQImageIFile::getPallette()
{
  if (!supportedImage()) // make sure we can handle it first
    return NULL;

  long x, i, j, k, l, found;
  unsigned char* line;

  x = 0;

  // Go to the first byte of image data
  Stream->seekg(TopOfImageData);

  if (*bandOrganization == "SINGLE FILE") // greyscale data
  {
    GreyPixel* pixelbuff = NULL;
    GreyPixel* temp = NULL;
    GreyPallette* pal;
    
    for (i=0; i < lines; i++) 
    {
      line = (unsigned char*)getRawScanline(i);
      for (j = 0; j < samples; j++)
      {
	found = 0;
	for (k = 0; k < x; k++) // look and see if the value is already there
	{
	  if ( (line[j] = pixelbuff[k].getGrey()) )
	    found = 1; // in the pallette so flag it
	}
	
	if (!found) // Add to the pallette
	{
	  temp = new GreyPixel[x+1];
	  for (l = 0; l < x; l++)   // create new pallette and copy existing
	    temp[l] = pixelbuff[l]; // entries into it
	  temp[x].setGrey(line[j]); // Set the new entry
	  x++;
	  if (pixelbuff)
	    delete[] pixelbuff;  // Delete the old one if it's there
	  pixelbuff = temp;  // reassign to the pallette pointer
	  temp = NULL;
	} // if(found)
      } // for(j)
      delete[] line;
    } // for(i)
    pal = new GreyPallette(x, pixelbuff); // make the new pallette
    delete [] pixelbuff;
    return((Pallette*)pal);
  }
  
  // If you have a true color DOQ, this routine will take a really really
  // long time to generate a pallete as it has to scan the file, then scan
  // the pallette to see if the color is there, and since you can have
  // 24 bit color....
  else if (*bandOrganization == "BIP")
  {
    RGBPixel* pixelbuff = NULL;
    RGBPixel* temp = NULL;
    RGBPallette* pal = NULL;
    unsigned char red, green, blue;
    
    for (i = 0; i < lines; i++)
    {
      line = (unsigned char*)getRawScanline(i);
      for (j = 0; j < (3 * samples); j += 3)
      {
	found = 0;
	for (k = 0; k < x; k++)
	{ // loop through the line and check if in the pallette
	  red = pixelbuff[k].getRed();
	  green = pixelbuff[k].getGreen();
	  blue = pixelbuff[k].getBlue();
	  if ( (line[j] == red) && (line[j + 1] == green) &&
	       (line[j + 2] == blue) ) // color is there so flag we found it
	    found = 1;                 
	}

	if (!found)  // add to the pallette
	{
	  temp = new RGBPixel[x + 1];
	  for (l = 0; l < x; l++)
	    temp[l] = pixelbuff[l]; // copy the old pallette into the new one
	  temp[x].setRed(line[j]);       // set the values of the new entry
	  temp[x].setGreen(line[j + 1]);
	  temp[x].setBlue(line[j + 2]);
	  x++;
	  if (pixelbuff != NULL)   // delete the old pallette
	    delete [] pixelbuff;
	  pixelbuff = temp;        // set the pointer to the new pallette
	  temp = NULL;
	}
      }
      delete [] line;
    }
    pal = new RGBPallette(x, pixelbuff); // make the new pallette
    delete [] pixelbuff;
    return((Pallette*)pal);
  }

  // This is here so the compiler won't throw out warnings
  return NULL;
}



AbstractPixel* DOQImageIFile::getPixel(long x, long y)
{
  if (!supportedImage()) // check if we can handle
    return NULL;

  // bounds checking of the pixel coordinates
  if ((x < 0) || (x > (samples - 1)) || (y < 0) || (y > (lines - 1)))
  {
    char dmsg[80];
    setFailRBit();
    sprintf(dmsg,"DOQImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",
	    x,y);
    gDebug.msg(dmsg, 1);
    return NULL;
  }
  
  if (*bandOrganization == "SINGLE FILE") // greyscale data
  {
    GreyPixel *pixelbuf;
    long seekval;
    unsigned char pixel;
    
    pixelbuf = new GreyPixel;
    
    seekval = y * samples + x;
    Stream->seekg(seekval + TopOfImageData); // go to the file position
    Stream->read(&pixel, sizeof(unsigned char)); // grab the data
    
    pixelbuf->setGrey((unsigned short)pixel); // set the greyvalue and
    return (AbstractPixel*)pixelbuf;          // return
  }
  else if (*bandOrganization == "BIP")
  {
    RGBPixel* pixel;
    unsigned char* buf;
    // ***********************************************************************
    // Hey Brian, rewrite this so that you don't have extra function call
    // (unless you come up with a real good reason otherwise), signed, youself
    // ***********************************************************************

    // Call getRawBIPPixel so that the RGB triplets will be in the correct
    // order
    buf = (unsigned char*)getRawBIPPixel(x, y);

    pixel = new RGBPixel(buf[0], buf[1], buf[2]);

    delete [] buf;
    return (AbstractPixel*)pixel;
  }

  // Added this to keep the compiler from throwing a warning
  return NULL; // should never get here
}


AbstractPixel* DOQImageIFile::getScanline(long row)
{
  if (!supportedImage()) // can we handle the image type?
    return NULL;

  // bounds checking for the row
  if ((row < 0) || (row > (lines-1)))
  {
    char dmsg[80];
    setFailRBit();
    sprintf(dmsg,"DOQImageIFile::getScanline(): scanline %ld out of bounds",
	    row);
    gDebug.msg(dmsg, 1);
    return NULL;
  }
  
  if (*bandOrganization == "SINGLE FILE") // handle as greyscale data
  {
    unsigned char* line; // hold the input line
    GreyPixel* pixelbuf; // output line
    long seekval; 
    long i;
    
    // allocate memory to the buffers
    pixelbuf = new GreyPixel[samples];
    line = new unsigned char[samples];

    // Goto the file position and grab the data
    seekval = row * samples;
    Stream->seekg(seekval + TopOfImageData);
    Stream->read(line, samples * sizeof(unsigned char));

    for (i = 0; i < samples; i++)
    {
      pixelbuf[i].setGrey(line[i]); // set the grey values in the objects
    }
    delete[] line; // free this memory
    return((AbstractPixel*)pixelbuf);
  }

  else if (*bandOrganization == "BIP") // handle as color data
  {
    unsigned char* line;
    RGBPixel* pixelbuf; // different type of image data
    long i;
   
    pixelbuf = new RGBPixel[samples];

    line = (unsigned char *) getRawScanline(row);
    for (i = 0; i < samples; i++)
    { // go through and set the color values for the objects
      pixelbuf[i].setRed(line[3 * i]);
      pixelbuf[i].setGreen(line[3 * i + 1]);
      pixelbuf[i].setBlue(line[3 * i + 2]);
    }
    delete [] line;
    return((AbstractPixel*)pixelbuf);
  }

  return NULL;
}


AbstractPixel* DOQImageIFile::getRectangle(long x1, long y1, long x2, long y2)
{
  if (!supportedImage()) // can we handle this?
    return NULL;
  
  // bounds checking for the input coordinates
  if ((x1 < 0) || (x1 > samples - 1) || (y1 < 0) || (y1 > lines - 1) ||
      (x2 < 0) || (x2 > samples - 1) || (y2 < 0) || (y2 > lines - 1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[100];
    sprintf(dmsg,"DOQImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds", x1, y1, x2, y2);
    gDebug.msg(dmsg, 1);
    setFailRBit();
    return NULL;
  }
  
  if (*bandOrganization == "SINGLE FILE")
  { // handle as greyscale data
    GreyPixel *pixelbuf;
    unsigned char *buffer;
    long length, x, j, k, seekval;

    // go grab the space for the buffer
    pixelbuf = new GreyPixel[(x2 - x1 + 1) * (y2 - y1 + 1)];
    length = x2 - x1 + 1; 
    k = 0;
    
    for(x = y1; x <= y2; x++)
    { 
      buffer = new unsigned char[length]; // make a temp buffer for input
      seekval = x * samples + x1;
      Stream->seekg(seekval + TopOfImageData);
      Stream->read(buffer, (sizeof(unsigned char))*length); // get the data
      for(j = 0; j < length; j++)
      {
	pixelbuf[k].setGrey(buffer[j]); // set the color values
	k++;
      }
      delete[] buffer;
    }
    return((AbstractPixel*)pixelbuf);
  }
  else if (*bandOrganization == "BIP")
  { // handle as color data
    RGBPixel* pixelbuf;
    unsigned char* rect;
    int w, h, n, i;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    n = w * h;
    
    // grab the area memory
    pixelbuf = new RGBPixel[n];
    rect = (unsigned char*)getRawRectangle(x1, y1, x2, y2);

    for (i = 0; i < n; i++)
    { // set the color values
      pixelbuf[i].setRed(rect[3 * i]);
      pixelbuf[i].setGreen(rect[3 * i + 1]);
      pixelbuf[i].setBlue(rect[3 * i + 2]);
    }

    delete [] rect;
    return((AbstractPixel*)pixelbuf);
  }

  return NULL;
}



void*  DOQImageIFile::getRawPixel(long x, long y)
{
  if (!supportedImage()) 
    return NULL;


  if ((x < 0) || (x > samples-1) || (y < 0) || (y > lines-1))
  {
    char dmsg[80];
    sprintf(dmsg,"DOQImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds"
	    ,x,y);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }

  if (*bandOrganization == "SINGLE FILE")
  { // grey data handling
    long seekval;
    unsigned char* pixel = new unsigned char;

    // seek and read the data
    seekval = y * samples + x;
    Stream->seekg(seekval + TopOfImageData);
    Stream->read(pixel, sizeof(unsigned char));
    if (!Stream->good()) // did we read ok?
    {
      gDebug.msg("DOQImageIFile::getRawPixel: problem reading file", 1);
      setFailRBit();
    }
    return pixel;
  }

  else if (*bandOrganization == "BIP")
  {
    long seekval;
    unsigned char* pixel;
    unsigned char temp[3]; // to hold 3 bytes of RGB data

    pixel = new unsigned char[3];

    // seek and read data
    seekval = 3 * (y * samples + x);
    Stream->seekg(seekval + TopOfImageData);
    Stream->read(temp, 3 * sizeof(unsigned char) );
    if (!Stream->good()) // did we read it ok?
    {
      gDebug.msg("DOQImageIFile::getRawPixel: problem reading file", 1);
      setFailRBit();
    }

    for (int count = 0; count <= 2; count++)
    {
      // This is ugly, but I can't think of a better way to do it right now. 
      // This accounts for different byte orderings in the DOQ while returning
      // a triplet in the "normal"  RGB order
      if (*bandContent[count] == "RED")
	pixel[0] = temp[count];
      else if (*bandContent[count] == "GREEN")
	pixel[1] = temp[count];
      else if (*bandContent[count] == "BLUE")
	pixel[2] = temp[count];
    }
    return pixel;
  }

  return NULL;
}


void*  DOQImageIFile::getRawScanline(long row)
{
  if (!supportedImage())
    return NULL;

  if ((row < 0) || (row > lines-1))
  {
    char dmsg[80];
    sprintf(dmsg,"DOQImageIFile::getRawScanline(): scanline %ld out of bounds",
	    row);
    gDebug.msg(dmsg, 1);
    setFailRBit();
    return NULL;
  }

  // Note, this returns indices for a greyscale DOQ.  To get the true values
  // you'll need to go check the pallette
  if (*bandOrganization == "SINGLE FILE")
  {
    unsigned char* pixelbuf;
    long seekval;
  
    pixelbuf = new unsigned char[samples];
 
    seekval = row * samples;
  
    Stream->seekg(seekval + TopOfImageData);
    Stream->read(pixelbuf, samples * sizeof(unsigned char));
    if (!Stream->good())
    {
      gDebug.msg("DOQImageIFile::getRawScanline: problem reading file", 1);
      setFailRBit();
    }
    return(pixelbuf);
  }
  else if (*bandOrganization == "BIP")
  {
    unsigned char* pixelbuf;
    unsigned char* tempbuff;
    unsigned char* tempchar;
    long _width;

    _width = 3 * samples;

    tempbuff = new unsigned char[_width];
    Stream->seekg(row * _width * sizeof(unsigned char) + TopOfImageData);
    Stream->read(tempbuff, _width * sizeof(unsigned char));
    if (!Stream->good())
    {
      gDebug.msg("DOQImageIFile::getRawScanline: problem readingfile", 1);
      setFailRBit();
    }

    // Ok, if the BIP DOQ is already in R,G,B order, just go ahead and return
    // the buffer we read in so we don't waste time rearranging what doesn't
    // need it
    if (RGBOrdered)
    {
      return tempbuff;
    }

    pixelbuf = new unsigned char[_width];
    for (int foocount = 0; foocount < _width; foocount += 3)
    {
      tempchar = &tempbuff[foocount];
      for (int bandcount = 0; bandcount < 3; bandcount++)
      {
	//*******************************************************************
	// Store the order as an integer and use a switch statement here and
	// See if it's faster than this.  signed, yourself
	//*******************************************************************

	// This is ugly, but I can't think of a better way to do it right now. 
	// This accounts for different byte orderings in the DOQ while
	// returning a triplet in the "normal"  RGB order
	if (*bandContent[bandcount] == "RED")
	{
	  pixelbuf[foocount] = tempchar[bandcount];
	}
	else if (*bandContent[bandcount] == "GREEN")
	{
	  pixelbuf[foocount + 1] = tempchar[bandcount];
	}
	else if (*bandContent[bandcount] == "BLUE")
	{
	  pixelbuf[foocount + 2] = tempchar[bandcount];
	}
      }
    }
    delete [] tempbuff;
    return(pixelbuf);
  }

  return NULL;
}


// This doesn't account for byte ordering yet
void*  DOQImageIFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if (!supportedImage())
    return NULL;

  if ((x1 < 0) || (x1 > samples-1) || (y1 < 0) || (y1 > lines-1) ||
      (x2 < 0) || (x2 > samples-1) || (y2 < 0) || (y2 > lines-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[120];
    sprintf(dmsg,"DOQImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
    gDebug.msg(dmsg,1);
    setFailRBit();
    return NULL;
  }
  
  if (*bandOrganization == "SINGLE FILE")
  {
    unsigned char* buffer;
    unsigned char* ptr;
    long length, x, k, seekval;
  
    length = x2 - x1 + 1;
    k = 0;
    buffer = new unsigned char[(x2 - x1 + 1) * (y2 - y1 + 1)];
  
    for(x=y1; x <= y2; x++)
    {
      seekval = x * samples + x1;
      ptr = &(buffer[(x - y1) * length]);
      Stream->seekg(seekval + TopOfImageData);
      Stream->read(ptr, sizeof(unsigned char)*length);
    }
  }
  else if (*bandOrganization == "BIP")
  {
    unsigned char* buffer;
    unsigned char* ptr;
    long length, y, k, seekval;
    
    length = 3 * (x2 - x1 + 1);
    k = 0;
    buffer = new unsigned char[3 * (x2 - x1 + 1) * (y2 - y1 + 1)];

    for (y = y1; y <= y2; y++)
    {
      seekval = 3 * (y * samples + x1);
      ptr = &(buffer[(y - y1) * length]);
      Stream->seekg(seekval + TopOfImageData);
      Stream->read(ptr, (sizeof(unsigned char) * length));
      if (!Stream->good())
      {
	gDebug.msg("DOQImageIFile::getRawRectangle: problem reading file", 1);
	setFailRBit();
      }
    }
  }

  if (!Stream->good())
  {
    gDebug.msg("DOQImageIFile::getRawRectangle: problem reading file", 1);
    setFailRBit();
  }

  return buffer;
}



void DOQImageIFile::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{

  if ((x < 0) || (x > samples-1) || (y < 0) || (y > lines-1))
  {
    char dmsg[80];
    sprintf(dmsg,"DOQImageIFile::getRawRGBPixel(): pixel (%ld,%ld) out of bounds",x,y);
    gDebug.msg(dmsg, 1);
    setFailRBit();
    return;
  }
    unsigned char* temp;
  
    temp = (unsigned char*) getRawBIPPixel(x,y);
    if (*bandOrganization == "SINGLE FILE")
    {
      pix->Red   = *temp;
      pix->Green = *temp;
      pix->Blue  = *temp;
    }
    else if (*bandOrganization == "BIP")
    {
      pix->Red = temp[0];
      pix->Green = temp[1];
      pix->Blue = temp[2];
    }
    delete temp;
}



void DOQImageIFile::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
  if ((row < 0) || (row > lines-1))
  {
    char dmsg[80];
    sprintf(dmsg,"DOQImageIFile::getRawRGBScanline(): scanline %ld out of bounds",row);
    gDebug.msg(dmsg, 1);
    setFailRBit();
    return;
  }
  if (*bandOrganization == "SINGLE FILE")
  {
    unsigned char* temp;
    long i;
  
    temp = (unsigned char*) getRawScanline(row);
  
    for (i = 0; i < samples; i++)
    {
      pixbuf[i].Red   = temp[i];
      pixbuf[i].Green = temp[i];
      pixbuf[i].Blue  = temp[i];
    }
    delete [] temp;
  }
  else if (*bandOrganization == "BIP")
  {
    unsigned char* temp;
    long i, k, w;
    w = 3 * samples;
    
    temp = (unsigned char*) getRawScanline(row);
    k = 0;
    for (i = 0; i < w; i+=3)
    {
      pixbuf[k].Red = temp[i];
      pixbuf[k].Green = temp[i + 1];
      pixbuf[k].Blue = temp[i + 2];
      k++;
    }
    delete [] temp;
  }
}



void DOQImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                       RawRGBPixel* pixbuf)
{
  if ((x1 < 0) || (x1 > samples-1) || (y1 < 0) || (y1 > lines-1) ||
      (x2 < 0) || (x2 > samples-1) || (y2 < 0) || (y2 > lines-1) ||
      (x1>x2) || (y1>y2))
  {
    char dmsg[120];
    sprintf(dmsg,"DOQImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds", x1, y1, x2, y2);
    gDebug.msg(dmsg, 1);
    setFailRBit();
    return;
  }

  if (*bandOrganization == "SINGLE FILE")
  {
    unsigned char* temp;
    int w, h, l, i;
  
    w = (x2 - x1 + 1);
    h = (y2 - y1 + 1);
    l = w * h;

    temp = (unsigned char*) getRawRectangle(x1, y1, x2, y2);

    for (i = 0; i < l; i++)
    {
      pixbuf[i].Red   = temp[i];
      pixbuf[i].Green = temp[i];
      pixbuf[i].Blue  = temp[i];
    }
    delete [] temp;
  }
  else if (*bandOrganization == "BIP")
  {
    unsigned char* temp;
    long i, k, w, h, l;
    w = (x2 - x1 + 1);
    h = (y2 - y1 + 1);
    l = 3 * h * w;

    temp = (unsigned char*) getRawRectangle(x1, y1, x2, y2);
    k = 0;
    for (i = 0; i < l; i += 3)
    {
      pixbuf[k].Red = temp[i];
      pixbuf[k].Green = temp[i + 1];
      pixbuf[k].Blue = temp[i + 2];
      k++;
    }
    delete[] temp;
  }
}

void* DOQImageIFile::getRawBIPPixel(long x, long y)
{
  // Only to be called by internal functions so they will have already checked
  // to make sure the pixel is within image bounds
  long seekval;
  unsigned char* pixel;
  unsigned char temp[3];
  pixel = new unsigned char[3];

  seekval = 3 * ( (y * samples) + x);
  Stream->seekg(TopOfImageData + seekval);
  Stream->read(temp, 3 * sizeof(unsigned char));
  if (!Stream->good())
  {
    gDebug.msg("DOQImageIFile::getRawBIPPixel(): problem reading file", 1);
    setFailRBit();
  }
  for (int count = 0; count <= 2; count++)
  {
    // This is ugly, but I can't think of a better way to do it right now. 
    // This accounts for different byte orderings in the DOQ while returning
    // a triplet in the "normal"  RGB order
    if (*bandContent[count] == "RED")
      pixel[0] = temp[count];
    else if (*bandContent[count] == "GREEN")
      pixel[1] = temp[count];
    else if (*bandContent[count] == "BLUE")
      pixel[2] = temp[count];
  }
  
  return pixel;
}

bool DOQImageIFile::supportedImage(void)
{
  if (bandOrganization == NULL)
  {
    cerr << "ERROR! Band Organization not specified.  Unable to process image"
	 << " data." << endl;
    return false;
  }


  if ( (*bandOrganization != "SINGLE FILE") && (*bandOrganization != "BIP") )
  {
    cerr << "ERROR! This version of the library only supports image data"
	 << endl;
    cerr << "with BAND_ORGANIZATION set to SINGLE FILE or BIP image data."
	 << endl;
    return false;
  }

  return true; // got here so is hopefully ok
}


bool DOQImageIFile::readBuffer(void)
{
  for (int _myi = 0; _myi <= 79; _myi++)
    buffer[_myi] = '\0'; // Clear the buffer out
  Stream->getline(buffer, 81);
  return true;
}



// Convert the buffer to uppercase chars so we don't have to worry about it
void DOQImageIFile::uppercase(void)
{
  for (int _mycount = 0; _mycount <= 79; _mycount++)
    buffer[_mycount] = toupper(buffer[_mycount]);
}



// Return the number of source images used
int DOQImageIFile::getNumImageSources(void)
{
  int _tempcount = 0;
  
  while (_tempcount <= 4)
  {
    if (sourceImageID[_tempcount] == NULL)
      return _tempcount;
    _tempcount++;
  }

  return _tempcount; // shouldn't get here, for gcc
}



// Scans buffer and returns a numeric value based on what token is in it
int DOQImageIFile::returnToken(void)
{
  // This is ugly, but..  Will worry about a better way later on.
  if ( (position = strstr(buffer, "BEGIN_USGS_DOQ_HEADER") ) != NULL)
    return 1;
  else if ( (position = strstr(buffer, "QUADRANGLE_NAME") ) != NULL)
    return 2;
  else if ( (position = strstr(buffer, "QUADRANT") ) != NULL)
    return 3;
  else if ( (position = strstr(buffer, "WEST_LONGITUDE") ) != NULL)
    return 4;
  else if ( (position = strstr(buffer, "EAST_LONGITUDE") ) != NULL)
    return 5;
  else if ( (position = strstr(buffer, "NORTH_LATITUDE") ) != NULL)
    return 6;
  else if ( (position = strstr(buffer, "SOUTH_LATITUDE") ) != NULL)
    return 7;
  else if ( (position = strstr(buffer, "PRODUCTION_DATE") ) != NULL)
    return 8;
  else if ( (position = strstr(buffer, "RASTER_ORDER") ) != NULL)
    return 9;
  else if ( (position = strstr(buffer, "BAND_ORGANIZATION") ) != NULL)
    return 10;
  else if ( (position = strstr(buffer, "BAND_CONTENT") ) != NULL)
    return 11;
  else if ( (position = strstr(buffer, "BITS_PER_PIXEL") ) != NULL)
    return 12;
  else if ( (position = strstr(buffer, "SAMPLES_AND_LINES") ) != NULL)
    return 13;
  else if ( (position = strstr(buffer, "SECONDARY_HORIZONTAL_DATUM") ) != NULL)
    return 19; // moved here since below would always pick up first
  else if ( (position = strstr(buffer, "HORIZONTAL_DATUM") ) != NULL)
    return 14;
  else if ( (position = strstr(buffer, "HORIZONTAL_COORDINATE_SYSTEM") ) != 
	    NULL)
    return 15;
  else if ( (position = strstr(buffer, "COORDINATE_ZONE") ) != NULL)
    return 16;
  else if ( (position = strstr(buffer, "HORIZONTAL_UNITS") ) != NULL)
    return 17;
  else if ( (position = strstr(buffer, "HORIZONTAL_RESOLUTION") ) != NULL)
    return 18;
  else if ( (position = strstr(buffer, "SECONDARY_XY_ORIGIN") ) != NULL)
    return 21;
  else if ( (position = strstr(buffer, "XY_ORIGIN") ) != NULL)
    return 20;
  else if ( (position = strstr(buffer, "NATION") ) != NULL)
    return 22;
  else if ( (position = strstr(buffer, "STATE") ) != NULL)
    return 23;
  else if ( (position = strstr(buffer, "NW_QUAD_CORNER_XY") ) != NULL)
    return 24;
  else if ( (position = strstr(buffer, "NE_QUAD_CORNER_XY") ) != NULL)
    return 25;
  else if ( (position = strstr(buffer, "SE_QUAD_CORNER_XY") ) != NULL)
    return 26;
  else if ( (position = strstr(buffer, "SW_QUAD_CORNER_XY") ) != NULL)
    return 27;
  else if ( (position = strstr(buffer, "SECONDARY_NW_QUAD_XY") ) != NULL)
    return 28;
  else if ( (position = strstr(buffer, "SECONDARY_NE_QUAD_XY") ) != NULL)
    return 29;
  else if ( (position = strstr(buffer, "SECONDARY_SE_QUAD_XY") ) != NULL)
    return 30;
  else if ( (position = strstr(buffer, "SECONDARY_SW_QUAD_XY") ) != NULL)
    return 31;
  else if ( (position = strstr(buffer, "RMSE_XY") ) != NULL)
    return 32;
  else if ( (position = strstr(buffer, "IMAGE_SOURCE") ) != NULL)
    return 33;
  else if ( (position = strstr(buffer, "SOURCE_IMAGE_ID") ) != NULL)
    return 34;
  else if ( (position = strstr(buffer, "SOURCE_IMAGE_DATE") ) != NULL)
    return 35;
  else if ( (position = strstr(buffer, "SOURCE_DEM_DATE") ) != NULL)
    return 36;
  else if ( (position = strstr(buffer, "AGENCY") ) != NULL)
    return 37;
  else if ( (position = strstr(buffer, "PRODUCER") ) != NULL)
    return 38;
  else if ( (position = strstr(buffer, "PRODUCTION_SYSTEM") ) != NULL)
    return 39;
  else if ( (position = strstr(buffer, "COMPRESSION") ) != NULL)
    return 40;
  else if ( (position = strstr(buffer, "STANDARD_VERSION") ) != NULL)
    return 41;
  else if ( (position = strstr(buffer, "METADATA_DATE") ) != NULL)
    return 42;
  else if ( (position = strstr(buffer, "DATA_FILE_SIZE") ) != NULL)
    return 43;
  else if ( (position = strstr(buffer, "BYTE_COUNT") ) != NULL)
    return 44;
  else if ( (position = strstr(buffer, "END_USGS_HEADER") ) != NULL)
    return 45;
  else
    return 666; // undefined tag
}
  


bool DOQImageIFile::readHeader(void)
{
  int _tempToken;
  readBuffer();
  if (returnToken() != 1)
    return false;
  
  readBuffer();
  while ( (_tempToken = returnToken() ) != 45)
  {
    switch (_tempToken)
    {
    case 2:
      setQuadName(); break;
    case 3:
      setQuadrant(); break;
    case 4:
      setWestLongitude(); break;
    case 5:
      setEastLongitude(); break;
    case 6:
      setNorthLatitude();  break;
    case 7:
      setSouthLatitude();  break;
    case 8:
      setProdDate(); break;
    case 9:
      setRasterOrder();  break;
    case 10:
      setBandOrganization(); break;
    case 11:
      setBandContent(); break;
    case 12:
      setBitsPerPixel(); break;
    case 13:
      setSamplesLines(); break;
    case 14:
      setHorizontalDatum(); break;
    case 15:
      setHorizontalCoordinateSystem(); break;
    case 16:
      setCoordinateZone(); break;
    case 17:
      setHorizontalUnits(); break;
    case 18:
      setHorizontalResolution(); break;
    case 19:
      setSecondaryHorizontalDatum(); break;
    case 20:
      setOrigins(); break;
    case 21:
      setSecondaryOrigins(); break;
    case 22:
      setNation(); break;
    case 23:
      setStates(); break;
    case 24:
      setNWQuadCorners(); break;
    case 25:
      setNEQuadCorners(); break;
    case 26:
      setSEQuadCorners(); break;
    case 27:
      setSWQuadCorners(); break;
    case 28:
      setSecondaryNWQuadCorner(); break;
    case 29:
      setSecondaryNEQuadCorner(); break;
    case 30:
      setSecondarySEQuadCorner(); break;
    case 31:
      setSecondarySWQuadCorner(); break;
    case 32:
      setRMSE(); break;
    case 33:
      setImageSource(); break;
    case 34:
      setSourceImageID(); break;
    case 35:
      setSourceImageDate(); break;
    case 36:
      setSourceDEMDate(); break;
    case 37:
      setAgency(); break;
    case 38:
      setProducer(); break;
    case 39:
      setProductionSystem(); break;
    case 40:
      setCompression(); break;
    case 41:
      setStandardVersion(); break;
    case 42:
      setMetaDataDate(); break;
    case 43:
      setDataSize(); break;
    case 44:
      setByteCount(); break;
    default:
    {
    }
    }
    readBuffer();
  }
  
  TopOfImageData = Stream->tellg();
  return true; // got this far so tis ok
}



bool DOQImageIFile::setQuadName(void)
{
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL) // assume it's a single word type thing
  {
    istrstream is(buffer, 80);
    quadName = new string;
    is >> _temp;
    is >> *quadName;
    return true;
  }
  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  quadName = new string(position);
  return true;
}



bool DOQImageIFile::setQuadrant(void)
{
  uppercase();
  istrstream is(buffer, 80);
  quadrant = new string;
  is >> _temp;  // Skip past tag
  is >> *quadrant;
  return true;
}



bool DOQImageIFile::setWestLongitude(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> westHour >> westMinute >> westSecond;
  return true;
}



bool DOQImageIFile::setEastLongitude(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> eastHour >> eastMinute >> eastSecond;
  return true;
}



bool DOQImageIFile::setNorthLatitude(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> northHour >> northMinute >> northSecond;
  return true;
}



bool DOQImageIFile::setSouthLatitude(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> southHour >> southMinute >> southSecond;
  return true;
}



bool DOQImageIFile::setProdDate(void)
{
  istrstream is(buffer, 80);
  prodDate = new string;
  is >> _temp;
  for (int _mycount = 0; _mycount <= 2; _mycount++)
  {
    is >> _temp;
    *prodDate = *prodDate + " " + _temp;
  }
  return true;
}



bool DOQImageIFile::setRasterOrder(void)
{
  uppercase();
  istrstream is(buffer, 80);
  rasterOrder = new string;
  is >> _temp;
  is >> *rasterOrder;
  return true;
}



bool DOQImageIFile::setBandOrganization(void)
{
  string temp;
  char* _end;
  uppercase();
  istrstream is(buffer, 80);

  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL) // check for other types
  {
    if ( (position = strstr(buffer, "BIP")) != NULL)
    {
      bandOrganization = new string;
      istrstream is(buffer, 80);
      is >> temp;
      is >> *bandOrganization;
      return true;
    }
    else if ( (position = strstr(buffer, "BIL")) != NULL)
    {
      bandOrganization = new string;
      istrstream is(buffer, 80);
      is >> temp;
      is >> *bandOrganization;
      return true;
    }
    else if ( (position = strstr(buffer, "BSQ")) != NULL)
    {
      bandOrganization = new string;
      istrstream is(buffer, 80);
      is >> temp;
      is >> *bandOrganization;
      return true;
    }
    else
    {
      cout << buffer << endl;
      cerr << "WARNING!.  Unable to determine Band Organization!" << endl;
      return false;
    }
  }

  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  bandOrganization = new string(position);

  return true;
}

// Okie, here, have to keep track of whether or not there's already an entry
// in the list.  If so, add it to the next one
bool DOQImageIFile::setBandContent(void)
{
  string _temp;
  istrstream is(buffer, 80);
  uppercase();
  is >> _temp; // grab keyword and ignore
  is >> _temp; // get the actual value of the field
  if (_temp == "BLACK&WHITE")
  {
    bandContent[0] = new string(_temp);
    return true; // don't have to worry about others in bandContent
  }

  else
  {
    if (bandContent[0] == NULL)
    {
      bandContent[0] = new string(_temp);
      return true;
    }
    else if (bandContent[1] == NULL)
    {
      bandContent[1] = new string(_temp);
      return true;
    }
    else if (bandContent[2] == NULL)
    {
      bandContent[2] = new string(_temp);
      return true;
    }
    else // all slots are full, something wierd is going on
      return false;
  }
}


bool DOQImageIFile::setBitsPerPixel(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> bitsPerPixel;
  return true;
}



bool DOQImageIFile::setSamplesLines(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> samples;
  is >> lines;
  return true;
}



bool DOQImageIFile::setHorizontalDatum(void)
{
  uppercase();
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL) // assume it's a single word entry
  {
    istrstream is(buffer, 80);
    horizontalDatum = new string;
    is >> _temp;
    is >> *horizontalDatum;
    return true;
  }

  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  horizontalDatum = new string(position);
  return true;

}



bool DOQImageIFile::setHorizontalCoordinateSystem(void)
{
  uppercase();
  istrstream is(buffer, 80);
  is >> _temp;
  is >> _temp;
  horizCoordSystem = new string(_temp);
  return true;
}



bool DOQImageIFile::setCoordinateZone(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> coordinateZone;
  return true;
}



bool DOQImageIFile::setHorizontalUnits(void)
{
  uppercase();
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL) // assume it's a single word entry
  {
    istrstream is(buffer, 80);
    horizontalUnits = new string;
    is >> _temp;
    is >> *horizontalUnits;
    return true;
  }

  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  horizontalUnits = new string(position);
  return true;

}



bool DOQImageIFile::setHorizontalResolution(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> horizontalResolution;
  return true;
}



bool DOQImageIFile::setSecondaryHorizontalDatum(void)
{
  uppercase();
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL) // assume it's a single word entry
  {
    istrstream is(buffer, 80);
    secHorizDatum = new string;
    is >> _temp;
    is >> *secHorizDatum;
    return true;
  }

  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  secHorizDatum = new string(position);
  return true;
}



bool DOQImageIFile::setOrigins(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> xOrigin >> yOrigin;
  return true;
}



bool DOQImageIFile::setSecondaryOrigins(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  is >> secxOrigin >> secyOrigin;
  return true;
}



// again, keep track of how many are there
bool DOQImageIFile::setNation(void)
{
  uppercase();
  istrstream is(buffer, 80);
  int _where = 0;
  bool flag = false;
  is >> _temp;

  // Go find the next empty slot
  for (int _mycount = 0; _mycount < 5; _mycount++)
  {
    if ( (nation[_mycount] == NULL) && !flag)
    {
      _where = _mycount;
      flag = true;
    }
  }

  nation[_where] = new string;
  is >> *nation[_where];
  return true;
}

bool DOQImageIFile::setStates(void)
{
  uppercase();
  istrstream is(buffer, 80);
  int _where = 0;
  bool flag = false;
  is >> _temp;
  
  // go find the next empty slot
  for (int _mycount = 0; _mycount < 5; _mycount++)
  {
    if ( (states[_mycount] == NULL) && !flag)
    {
      _where = _mycount;
      flag = true;
    }
  }

  states[_where] = new string;
  is >> *states[_where];
  return true;
}



bool DOQImageIFile::setNWQuadCorners(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> NWQuadCornerX >> NWQuadCornerY;
  return true;
}



bool DOQImageIFile::setNEQuadCorners(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> NEQuadCornerX >> NEQuadCornerY;
  return true;
}



bool DOQImageIFile::setSEQuadCorners(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> SEQuadCornerX >> SEQuadCornerY;
  return true;
}



bool DOQImageIFile::setSWQuadCorners(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> SWQuadCornerX >> SWQuadCornerY;
  return true;
}



bool DOQImageIFile::setSecondaryNWQuadCorner(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> SNWQuadCornerX >> SNWQuadCornerY;
  return true;
}



bool DOQImageIFile::setSecondaryNEQuadCorner(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> SNEQuadCornerX >> SNEQuadCornerY;
  return true;
}



bool DOQImageIFile::setSecondarySEQuadCorner(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> SSEQuadCornerX >> SSEQuadCornerY;
  return true;
}



bool DOQImageIFile::setSecondarySWQuadCorner(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> SSWQuadCornerX >> SSWQuadCornerY;
  return true;
}



bool DOQImageIFile::setRMSE(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> rmseXY;
  return true;
}



bool DOQImageIFile::setImageSource(void)
{
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL)
  {
    cerr << "WARNING!  Unable to determine Image Source" << endl;
    return false;
  }
  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  imageSource = new string(position);
  return true;
}



bool DOQImageIFile::setSourceImageID(void)
{
  int _where = 0;
  bool flag = false;

  for (int _mycount = 0; _mycount < 5; _mycount++)
  {
    if ( (sourceImageID[_mycount] == NULL) && !flag)
    {
      _where = _mycount;
      flag = true;
    }
  }

  if (_where == 5) // already full
    return false;
  
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL)
  {
    cerr << "WARNING! Unable to determine Source Image ID" << endl;
    return false;
  }
  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  sourceImageID[_where] = new string(position);
  return true;
}


bool DOQImageIFile::setSourceImageDate(void)
{
  istrstream is(buffer, 80);
  int _where = 0;
  bool flag = false;

  for (int _mycount = 0; _mycount < 5; _mycount++)
  {
    if ( (sourceImageDate[_mycount] == NULL) && !flag)
    {
      _where = _mycount;
      flag = true;
    }
  }
  
  if (_where == 5)
    return false;

  is >> _temp;

  sourceImageDate[_where] = new string;
  is >> *sourceImageDate[_where];
  is >> _temp;
  *sourceImageDate[_where] = *sourceImageDate[_where] + " " + _temp;
  is >> _temp;
  *sourceImageDate[_where] = *sourceImageDate[_where] + " " + _temp;
  return true;
}



bool DOQImageIFile::setSourceDEMDate(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  sourceDEMDate = new string;
  is >> *sourceDEMDate;
  is >> _temp;
  *sourceDEMDate = *sourceDEMDate + " " + _temp;
  is >> _temp;
  *sourceDEMDate = *sourceDEMDate + " " + _temp;
  return true;
}



bool DOQImageIFile::setAgency(void)
{
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL)
  {
    cerr << "WARNING! Unable to set Agency" << endl;
    return false;
  }
  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  agency = new string(position);
  return true;
}



bool DOQImageIFile::setProducer(void)
{
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL)
  {
    cerr << "WARNING! Unable to set Producer" << endl;
    return false;
  }
  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  producer = new string(position);
  return true;
}



bool DOQImageIFile::setProductionSystem(void)
{
  char* _end;
  position = strstr(buffer, "\""); // set pos to first quote
  if (position == NULL)
  {
    cerr << "WARNING! Unable to set Production System" << endl;
    return false;
  }
  position = position + 1; // skip past the first quotation mark
  _end = strstr(position, "\"");
  *_end = '\0'; // set so we can make a string out of the substring
  productionSystem = new string(position);
  return true;
}



bool DOQImageIFile::setCompression(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  compression = new string;
  is >> *compression;
  is >> _temp;
  *compression = *compression + " " + _temp;
  is >> _temp;
  *compression = *compression + " " + _temp;
  return true;
}



bool DOQImageIFile::setStandardVersion(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  standardVersion = new string;
  is >> *standardVersion;
  is >> _temp;
  *standardVersion = *standardVersion + " " + _temp;
  return true;
}



bool DOQImageIFile::setMetaDataDate(void)
{
  istrstream is(buffer, 80);
  is >> _temp;
  metaDataDate = new string;

  is >> *metaDataDate;
  is >> _temp;
  *metaDataDate = *metaDataDate + " " + _temp;
  is >> _temp;
  *metaDataDate = *metaDataDate + " " + _temp;
  return true;
}



bool DOQImageIFile::setDataSize(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> dataSize;
  return true;
}



bool DOQImageIFile::setByteCount(void)
{
  istrstream is(buffer, 80);
  
  is >> _temp;
  is >> byteCount;
  return true;
}



string* DOQImageIFile::getQuadName(void)
{
  if (quadName == NULL)
    return NULL;
  string* retstring = new string(*quadName);
  return retstring;
}



string* DOQImageIFile::getQuadrant(void)
{
  if (quadrant == NULL)
    return NULL;
  string* retstring = new string(*quadrant);
  return retstring;
}


int DOQImageIFile::getWestHour(void)
{
  return westHour;
}



int DOQImageIFile::getWestMinute(void)
{
  return westMinute;
}



float DOQImageIFile::getWestSecond(void)
{
  return westSecond;
}



int DOQImageIFile::getEastHour(void)
{
  return eastHour;
}



int DOQImageIFile::getEastMinute(void)
{
  return eastMinute;
}



float DOQImageIFile::getEastSecond(void)
{
  return eastSecond;
}



int DOQImageIFile::getNorthHour(void)
{
  return northHour;
}



int DOQImageIFile::getNorthMinute(void)
{
  return northMinute;
}



float DOQImageIFile::getNorthSecond(void)
{
  return northSecond;
}



int DOQImageIFile::getSouthHour(void)
{
  return southHour;
}



int DOQImageIFile::getSouthMinute(void)
{
  return southMinute;
}



float DOQImageIFile::getSouthSecond(void)
{
  return southSecond;
}



string* DOQImageIFile::getProdDate(void)
{
  if (prodDate == NULL)
    return NULL;
  string* _mytemp;
  _mytemp = new string(*prodDate);
  return _mytemp;
}



string* DOQImageIFile::getRasterOrder(void)
{
  if (rasterOrder == NULL)
    return NULL;
  string* _retstring = new string(*rasterOrder);
  return _retstring;
}



string* DOQImageIFile::getBandOrganization(void)
{
  if (bandOrganization == NULL)
    return NULL;
  string* _retstring = new string(*bandOrganization);
  return _retstring;;
}



string* DOQImageIFile::getBandContent(int inpos)
{
  if (bandContent[inpos] == NULL)
    return NULL;
  string* _retstring = new string(*bandContent[inpos]);
  return _retstring;
}



int DOQImageIFile::getBitsPerPixel(void)
{
  return bitsPerPixel;
}



long int DOQImageIFile::getSamples(void)
{
  return samples;
}



long int DOQImageIFile::getLines(void)
{
  return lines;
}



string* DOQImageIFile::getHorizontalDatum(void)
{
  if (horizontalDatum == NULL)
    return NULL;
  string* _mytemp = new string(*horizontalDatum);
  return _mytemp;
}



string* DOQImageIFile::getHorizontalCoordinateSystem(void)
{
  if (horizCoordSystem == NULL)
    return NULL;
  string* _mytemp = new string(*horizCoordSystem);
  return _mytemp;
}



int DOQImageIFile::getCoordinateZone(void)
{
  return coordinateZone;
}



string* DOQImageIFile::getHorizontalUnits(void)
{
  if (horizontalUnits == NULL)
    return NULL;
  string* _mytemp = new string(*horizontalUnits);
  return _mytemp;
}



float DOQImageIFile::getHorizontalResolution(void)
{
  return horizontalResolution;
}



string* DOQImageIFile::getSecondaryHorizontalDatum(void)
{
  if (secHorizDatum == NULL)
    return NULL;
  string* _mytemp = new string(*secHorizDatum);
  return _mytemp;
}



double DOQImageIFile::getXOrigin(void)
{
  return xOrigin;
}



double DOQImageIFile::getYOrigin(void)
{
  return yOrigin;
}



double DOQImageIFile::getSecondaryXOrigin(void)
{
  return secxOrigin;
}



double DOQImageIFile::getSecondaryYOrigin(void)
{
  return secyOrigin;
}



int DOQImageIFile::getNumNations(void)
{
  int _mycount = 0;
  while (_mycount != 5)
  {
    if (nation[_mycount] == NULL)
      return (_mycount);
    _mycount++;
  }
  return _mycount; // shouldn't get here, for gcc
}



string* DOQImageIFile::getNation(int nationnumber)
{
  if (nation[nationnumber - 1] == NULL)
    return NULL;
  string* _mytemp = new string(*nation[nationnumber - 1]);
  return _mytemp;
}



int DOQImageIFile::getNumStates(void)
{
  int _mycount = 0;
  while (_mycount != 5)
  {
    if (states[_mycount] == NULL)
      return _mycount;
    _mycount++;
  }
  return _mycount; // shouldn't get here, for gcc
}


string* DOQImageIFile::getState(int statenumber)
{
  if (states[statenumber - 1] == NULL)
    return NULL;
  string* _mystate = new string(*states[statenumber - 1]);
  return _mystate;
}



double DOQImageIFile::getNWQuadX(void)
{
  return NWQuadCornerX;
}



double DOQImageIFile::getNWQuadY(void)
{
  return NWQuadCornerY;
}



double DOQImageIFile::getNEQuadX(void)
{
  return NEQuadCornerX;
}



double DOQImageIFile::getNEQuadY(void)
{
  return NEQuadCornerY;
}



double DOQImageIFile::getSEQuadX(void)
{
  return SEQuadCornerX;
}



double DOQImageIFile::getSEQuadY(void)
{
  return SEQuadCornerY;
}


double DOQImageIFile::getSWQuadX(void)
{
  return SWQuadCornerX;
}


double DOQImageIFile::getSWQuadY(void)
{
  return SWQuadCornerY;
}


double DOQImageIFile::getSecNWQuadX(void)
{
  return SNWQuadCornerX;
}


double DOQImageIFile::getSecNWQuadY(void)
{
  return SNWQuadCornerY;
}


double DOQImageIFile::getSecNEQuadX(void)
{
  return SNEQuadCornerX;
}


double DOQImageIFile::getSecNEQuadY(void)
{
  return SNEQuadCornerY;
}


double DOQImageIFile::getSecSEQuadX(void)
{
  return SSEQuadCornerX;
}


double DOQImageIFile::getSecSEQuadY(void)
{
  return SSEQuadCornerY;
}


double DOQImageIFile::getSecSWQuadX(void)
{
  return SSWQuadCornerX;
}


double DOQImageIFile::getSecSWQuadY(void)
{
  return SSWQuadCornerY;
}


float DOQImageIFile::getRMSE(void)
{
  return rmseXY;
}


string* DOQImageIFile::getImageSource(void)
{
  if (imageSource == NULL)
    return NULL;
  string* _mytemp = new string(*imageSource);
  return _mytemp;
}


string* DOQImageIFile::getSourceImageID(int numimage)
{
  if (sourceImageID[numimage - 1] == NULL)
    return NULL;
  if (numimage <= 5)
  {
    string* _mytemp = new string(*sourceImageID[numimage - 1]);
    return _mytemp;
  }
  else return NULL;
}


string* DOQImageIFile::getSourceImageDate(int numimage)
{
  if ( (numimage <= 5) && (sourceImageDate[numimage - 1] != NULL) )
  {
    string* _mytemp = new string(*sourceImageDate[numimage - 1]);
    return _mytemp;
  }
  else return NULL;
}


string* DOQImageIFile::getSourceDEMDate(void)
{
  if (sourceDEMDate == NULL)
    return NULL;
  string* _mytemp = new string(*sourceDEMDate);
  return _mytemp;
}


string* DOQImageIFile::getAgency(void)
{
  if (agency == NULL)
    return NULL;
  string* _mytemp = new string(*agency);
  return _mytemp;
}


string* DOQImageIFile::getProducer(void)
{
  if (producer == NULL)
    return NULL;
  string* _mytemp = new string(*producer);
  return _mytemp;
}


string* DOQImageIFile::getProductionSystem(void)
{
  if (productionSystem == NULL)
    return NULL;
  string* _mytemp = new string(*productionSystem);
  return _mytemp;
}


string* DOQImageIFile::getCompression(void)
{
  if (compression == NULL)
    return NULL;
  string* _mytemp = new string(*compression);
  return _mytemp;
}


string* DOQImageIFile::getStandardVersion(void)
{
  if (standardVersion == NULL)
    return NULL;
  string* _mytemp = new string(*standardVersion);
  return _mytemp;
}


string* DOQImageIFile::getMetaDataDate(void)
{
  if (metaDataDate == NULL)
    return NULL;
  string* _mytemp = new string(*metaDataDate);
  return _mytemp;
}


long int DOQImageIFile::getDataSize(void)
{
  return dataSize;
}


long int DOQImageIFile::getByteCount(void)
{
  return byteCount;
}


bool DOQImageIFile::getOptimalRectangle(int &w, int &h)
{
   w = samples;
   h = 0;
   return true;
}


