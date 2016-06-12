//  ImageCommon.h                       11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Defines some macros for error messages and constants.

#ifndef _IMAGE_COMMON_H_
#define _IMAGE_COMMON_H_


#include <iostream.h>
#include <stdlib.h>


// Here are some macros defining constants for the
// purpose of identifying photometric interpretation
// of an image.

#define DRG_BILEVEL   0
#define DRG_GREY      1
#define DRG_GRAY      1
#define DRG_RGB       2
#define DRG_HSV       3
#define DRG_HLS       4
#define DRG_CMY       5
#define DRG_YIQ       6 
#define DRG_REAL_RGB  7

//  Here are some macros defining constants to represent
//  image file types which are understood by Image4.0
//  These are returned by Image::getFileType()

#define IMAGE_RGB     0
#define IMAGE_GREY    1
#define IMAGE_TIFF    2
#define IMAGE_DGP     3
#define IMAGE_CRL     4
#define IMAGE_DOQ     5
#define IMAGE_GIF     6
#define IMAGE_PBM     7
#define IMAGE_XBM     8
#define IMAGE_BMP     9
#define IMAGE_PCX     10 
#define IMAGE_JFIF    11 
#define IMAGE_GTIF    12
#define IMAGE_PNG     13
#define IMAGE_ERROR   254
#define IMAGE_UNKNOWN 255 



//  Here are some macros defined for use as error messages
//  in the DRG related programs.


#if 0

#define ABORTERRMSG(msg) { cerr << "Error: " << msg << " in line " << __LINE__ << " in file " << __FILE__ << ".  Aborting..." << endl; abort(); } 

#define EXITERRMSG(msg) { cerr << "Error: " << msg << " in line " << __LINE__ << " in file " << __FILE__ << ".  Exiting..." << endl; exit(1); }

#define ERRMSG(msg) { cerr << "Error: " << msg << " in line " << __LINE__ << " in file " << __FILE__ << ".  Continuing..." << endl; }

#endif
//  Struct RawRGBPixel for efficiency enhancements.  Added v4.2

struct RawRGBPixel
{
   unsigned char Red;
   unsigned char Green;
   unsigned char Blue;
};

#endif

