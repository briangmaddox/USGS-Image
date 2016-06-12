//  CRLImageIFile.h                  11 Jul 94
//  Authors:  Rob Fugina, Greg Martin, and Loren Bonebrake, SES, MCMC, USGS

//  Defines the class CRLImageIFile

//  This class handles the reading of Intergraph Tiled format raster
//  files, usually named <file>.crl

//  The order for the x,y components stands for (column, row)


#ifndef _CRLIMAGEIFILE_H_
#define _CRLIMAGEIFILE_H_


#include <stdio.h>
#include "Image/ImageIFile.h"
#include "Image/Pallette.h"
#include "Image/RGBPallette.h"

//  Some utility structures used by the CRLImageIFile class

typedef struct
{
   unsigned char slot;
   unsigned char red;
   unsigned char green;
   unsigned char blue;
} ColorCell;


struct Tile
{
   long xorg;
   long yorg;
   long height;
   long width;
   unsigned char *data;
   ~Tile();
};

typedef struct
{
   long offset;
   long alloc;
   long used;
} DirEntry;

#define CACHE_SIZE 50

class CRLImageIFile : public ImageIFile
{
  // data members

  FILE* file;
  long dir_offset;
  ColorCell *co_map;
  int co_map_entries;
  DirEntry *tile_dir;
  RGBPallette* pallette;


  int tiled;
  int topOfData;
  int *s_off;


  Tile** cache;
  long* when;
  long* tile;
  long tiletime;
  int CacheSize;


  short *hashTable;
  unsigned char **rowCache;
  int           *rowNumber;
  unsigned int  *timeStamp;
  unsigned int   lastTimeStamp;
  
  public :

  // constructors and destructor

  CRLImageIFile(const char* f_name);
  virtual ~CRLImageIFile();

  protected :

  // methods

  int  readWord();
  long readDWord();
  void readColorMap();
  void readTileDir();
  long getTileSize(int tile_num);
  void readScanLine(unsigned char *scan);
   
  // access to file parts

  long getFileWidth();
  long getFileHeight();


  const ColorCell* getColorMap();
  void setupPallette(void);

  Tile* getTile(int tile_num);
  
  void getScanlineOffsets(void);

  short cacheScanline(int);                

  public:

  long getMaxTileWidth();
  long getMaxTileHeight();

  long getTileWidth(int tile_num);
  long getTileHeight(int tile_num);

  int getTilesWide();
  int getTilesHigh();
  int getNumTiles();


  /* Standard ImageIFile interface.  These functions allocate memory,
     so the memory must be deleted by the user.
     */
  virtual void*  getRawPixel(long x, long y);
  virtual void*  getRawScanline(long row);
  virtual void*  getRawRectangle(long x1, long y1, long x2, long y2);

  virtual AbstractPixel* getPixel(long x, long y);
  virtual AbstractPixel* getScanline(long row);
  virtual AbstractPixel* getRectangle(long x1, long y1, long x2, long y2);
  
  virtual void getRawRGBPixel(long x, long y, RawRGBPixel* pix);
  virtual void getRawRGBScanline(long row, RawRGBPixel* pixbuf);
  virtual void getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                   RawRGBPixel* pixbuf);

  virtual Pallette* getPallette();

  virtual bool getOptimalRectangle(int &w, int &h);
};

#endif




