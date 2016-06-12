//  DOQImageIFile.h                   27 Oct 94
//  Authors:  Greg Martin, SES, MCMC, USGS
//            Eric Heien,  SES, MCMC, USGS
//  Seriously updated: Brian Maddox, SES MCMC August 1997
//  There are betas, then there's this thing ;)

//  Defines the class DOQImageIFile

//  This class handles the reading of raw greyscale raster files


#ifndef _DOQIMAGEIFILE_H_
#define _DOQIMAGEIFILE_H_

#include <fstream.h>
#include <string.h>
#include <string>
#include "Image/ImageIFile.h"
#include "Image/Pallette.h"


// This class is huge now.  Might split some of it up eventually
class DOQImageIFile : public ImageIFile
{
    
  ifstream* Stream;      // is called inout in source
  string*   _fileName;
  char*     headerFile;
  long      TopOfImageData;

  char     temp;
  char     buffer[80];
  char*    ret;
  char*    position; // position if found in buffer
  string   _temp; 

  // Identification Section
  string* quadName;
  string* quadrant;
  // neatlines (spread out like this because DOQ's will take a lot of memory
  // reguardless and I didn't want to make a struct since C++ would make a 
  // class out of it and didn't want the extra overhead
  int       westHour;
  int       westMinute;
  float     westSecond;
  int       eastHour;
  int       eastMinute;
  float     eastSecond;
  int       northHour;
  int       northMinute;
  float     northSecond;
  int       southHour;
  int       southMinute;
  float     southSecond;
  string*   prodDate;
  
  // Display Section
  string*    rasterOrder; // 0 = left/right, 1 = top/bottom
  string*    bandOrganization; // 0 = single, 1 = bip, 2 = bsq, 3 = bil
  string*    bandContent[3]; // 0 = black/white, 1 = red, 2 = green, 3 = blue


  int        bitsPerPixel; 
  long int   samples;
  long int   lines;
  // Registration Section
  string*   horizontalDatum;
  string*   horizCoordSystem; // 0 = UTM, 1 = state plane, 2 = geographic
  int       coordinateZone;
  string*   horizontalUnits;
  float     horizontalResolution;
  string*   secHorizDatum;
  double    xOrigin;
  double    yOrigin;
  double    secxOrigin;
  double    secyOrigin;

  // Other Information
  string*       nation[5];
  string*       states[5];  // Up to 4 state entries in the DOQ header
  double        NWQuadCornerX;
  double        NWQuadCornerY;
  double        NEQuadCornerX;
  double        NEQuadCornerY;
  double        SEQuadCornerX;
  double        SEQuadCornerY;
  double        SWQuadCornerX;
  double        SWQuadCornerY;
  // Secondary corner point coordinates
  double        SNWQuadCornerX;
  double        SNWQuadCornerY;
  double        SNEQuadCornerX;
  double        SNEQuadCornerY;
  double        SSEQuadCornerX;
  double        SSEQuadCornerY;
  double        SSWQuadCornerX;
  double        SSWQuadCornerY;
  
  float         rmseXY; // Root Mean Square error
  string*       imageSource;
  string*       sourceImageID[5];
  string*       sourceImageDate[5];
  string*       sourceDEMDate;
  string*       agency;
  string*       producer;
  string*       productionSystem;
  string*       compression;
  string*       standardVersion;
  string*       metaDataDate;
  long int      dataSize;
  long int      byteCount;


  void  uppercase(void); // convert the buffer to lowercase

  // Return false if Band Organization hasn't been specified or if it's 
  // band organization is not single file since we don't handle bils etc yet
  bool  supportedImage(void); 

  // Methods to set data.  Note:  These functions are called after the 
  // whichToken method is called.  Once it returns the token, the parser
  // will call these functions and they will take the buffer and parse out
  // what they need accordingly 
  // Identification section
  bool setQuadName(void);
  bool setQuadrant(void);
  bool setWestLongitude(void);
  bool setEastLongitude(void);
  bool setNorthLatitude(void);
  bool setSouthLatitude(void);
  bool setProdDate(void);

  // Display Section
  bool setRasterOrder(void);
  bool setBandOrganization(void);
  bool setBandContent(void);
  bool setBitsPerPixel(void);
  bool setSamplesLines(void);

  // Registration Section
  bool setHorizontalDatum(void);
  bool setHorizontalCoordinateSystem(void);
  bool setCoordinateZone(void);
  bool setHorizontalUnits(void);
  bool setHorizontalResolution(void);
  bool setSecondaryHorizontalDatum(void);
  bool setOrigins(void); // sets both X and Y
  bool setSecondaryOrigins(void); // sets both X and Y

  // Other Information
  bool setNation(void);
  bool setStates(void); // Can be called up to 4 times
  bool setNWQuadCorners(void); // as usual
  bool setNEQuadCorners(void);
  bool setSEQuadCorners(void);
  bool setSWQuadCorners(void);
  // Secondary Corner Point Coordinates
  bool setSecondaryNWQuadCorner(void);
  bool setSecondaryNEQuadCorner(void);
  bool setSecondarySEQuadCorner(void);
  bool setSecondarySWQuadCorner(void);

  bool setRMSE(void);
  bool setImageSource(void);
  bool setSourceImageID(void);
  bool setSourceImageDate(void);
  bool setSourceDEMDate(void);
  bool setAgency(void);
  bool setProducer(void);
  bool setProductionSystem(void);
  bool setCompression(void);
  bool setStandardVersion(void);
  bool setMetaDataDate(void);
  bool setDataSize(void);
  bool setByteCount(void);

  // Misc functions for the Image interface
  void* getRawBIPPixel(long x, long y);

  // set to true if the triplets for color images are in R,G,B order
  bool RGBOrdered;

public:

  DOQImageIFile(const char* fn);
  virtual ~DOQImageIFile();


  //  Methods to get at the header data
  // Note:  These return memory which must be destroyed by the caller
  // Identification Section
  string* getQuadName(void);
  string* getQuadrant(void);
  int     getWestHour(void);
  int     getWestMinute(void);
  float   getWestSecond(void);
  int     getEastHour(void);
  int     getEastMinute(void);
  float   getEastSecond(void);
  int     getNorthHour(void);
  int     getNorthMinute(void);
  float   getNorthSecond(void);
  int     getSouthHour(void);
  int     getSouthMinute(void);
  float   getSouthSecond(void);
  string* getProdDate(void);

  // Display section
  string*   getRasterOrder(void);
  string*   getBandOrganization(void);
  string*   getBandContent(int inpos);
  int       getBitsPerPixel(void);
  long int  getSamples(void);
  long int  getLines(void);

  // Registration Section
  string* getHorizontalDatum(void);
  string* getHorizontalCoordinateSystem(void);
  int     getCoordinateZone(void);
  string* getHorizontalUnits(void);
  float   getHorizontalResolution(void);
  string* getSecondaryHorizontalDatum(void);
  double  getXOrigin(void);
  double  getYOrigin(void);
  double  getSecondaryXOrigin(void);
  double  getSecondaryYOrigin(void);

  // Other Information
  int     getNumNations(void);
  string* getNation(int nationnumber);
  int     getNumStates(void); // Returns the number of states in storage
  string* getState(int statenumber); // return the selected state
  double  getNWQuadX(void);
  double  getNWQuadY(void);
  double  getNEQuadX(void);
  double  getNEQuadY(void);
  double  getSEQuadX(void);
  double  getSEQuadY(void);
  double  getSWQuadX(void);
  double  getSWQuadY(void);
  // Secondary Corner Point Values
  double  getSecNWQuadX(void);
  double  getSecNWQuadY(void);
  double  getSecNEQuadX(void);
  double  getSecNEQuadY(void);
  double  getSecSEQuadX(void);
  double  getSecSEQuadY(void);
  double  getSecSWQuadX(void);
  double  getSecSWQuadY(void);

  float     getRMSE(void);
  string*   getImageSource(void);
  int       getNumImageSources(void); // returns number for the following 2
  string*   getSourceImageID(int numimage);
  string*   getSourceImageDate(int numimage);
  string*   getSourceDEMDate(void);
  string*   getAgency(void);
  string*   getProducer(void);
  string*   getProductionSystem(void);
  string*   getCompression(void);
  string*   getStandardVersion(void);
  string*   getMetaDataDate(void);
  long int  getDataSize(void);
  long int  getByteCount(void);


  bool readBuffer(void);   // Read buffer from disk
  int  returnToken(void);  // ID tag and return token

  // Standard ImageIFile interface

  // These get functions allocate memory, so the user
  // must delete this memory.


  virtual Pallette* getPallette(void);

  // get/put functions use column-major order, ie. (x,y) == (column,row)

  virtual AbstractPixel* getPixel(long x, long y);
  virtual AbstractPixel* getScanline(long row);
  virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);

  virtual void*  getRawPixel(long x, long y);
  virtual void*  getRawScanline(long row);
  virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);

  virtual void   getRawRGBPixel(long x, long y, RawRGBPixel* pix);
  virtual void   getRawRGBScanline(long row, RawRGBPixel* pixbuf); 
  virtual void   getRawRGBRectangle(long x1, long y1, long x2, long y2,
				    RawRGBPixel* pixbuf);

  virtual bool getOptimalRectangle(int &w, int &h);

protected:
  
  bool readHeader(void);  // Called readHeaderData in the source
};

#endif
