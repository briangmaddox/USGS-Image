//  CRLImageIFile.cc                         11 Jul 94
//  Authors:  Rob Fugina, Loren Bonebrake, and Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class CRLImageIFile 

#include "Image/CRLImageIFile.h"
#include "Image/RGBPixel.h"
#include "Image/RGBPallette.h"
#include "Image/ImageCommon.h"
#include "Image/ImageDebug.h"
#include <iostream.h>
#include <iomanip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


Tile::~Tile()
{
    if (data) delete data;
    gDebug.msg("Tile::~Tile(): exiting dtor",5);
}


// **********************************************************************

// Function  : CRLImageIFile
// Purpose   : constructor for the CRLImageIFile class with a file name.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

CRLImageIFile::CRLImageIFile(const char *f_name)
{
  short i;
  unsigned int data_type;

  setPhotometric(DRG_RGB);
  setHasPallette();
  setBitsPerSample(8);
  setSamplesPerPixel(1);

  if (!(file = fopen(f_name, "rb")))
  {
     setNoDataBit();
     gDebug.msg("CRLImageIFile::CRLImageIFile(): couldn't open file",2);
  }
  else
  {
     co_map = NULL;
     tile_dir = NULL;
   
     fseek(file,4,SEEK_SET);
     data_type = readWord();
     if (data_type == 65)
     {
        tiled = 1;
        gDebug.msg("CRLImageIFile::CRLImageIFile(): file is tiled",5);
     }
     else if (data_type == 10)
     {
        tiled = 0;
        gDebug.msg("CRLImageIFile::CRLImageIFile(): file is not tiled",5);
     }
     else
     {
        setNoDataBit();
        gDebug.msg("CRLImageIFile::CRLImageIFile(): file is not a CRL.",1);
     }
     
     if (good())
     {
        if (tiled)
        {
           readColorMap();
           readTileDir();
           setupPallette();
           setWidth(getFileWidth());
           setHeight(getFileHeight());
   
           CacheSize = getTilesWide();
           cache = new Tile*[CacheSize];
           when  = new long[CacheSize];
           tile  = new long[CacheSize];
           tiletime = 0;
   
           for (i=0;i<CacheSize;i++)
           {
              cache[i] = NULL;
              when[i]  = -1;
              tile[i]  = -1;
           }
        }
        else
        {
           fseek(file,2,SEEK_SET);
           topOfData = readWord();
           topOfData = (topOfData+2)/256;
           topOfData = topOfData*512;
           readColorMap();
           setupPallette();
           setWidth(getFileWidth());
           setHeight(getFileHeight());
           getScanlineOffsets();
           hashTable = new short[getHeight()];
           rowCache = new unsigned char *[CACHE_SIZE];
           rowNumber = new int[CACHE_SIZE];
           timeStamp = new unsigned int[CACHE_SIZE];
           lastTimeStamp=0;
           for (i=0;i<getHeight();i++)
               hashTable[i] = (short) -1;
           for (i=0;i<CACHE_SIZE;i++)
           {
               rowCache[i] = NULL;
               timeStamp[i] = 0;
               rowNumber[i] = -1;
           }
        }
     }
  }
  setRandomAccessFlags(rpixel|rrow|rrect);
  gDebug.msg("CRLImageIFile::CRLImageIFile(c*): exiting ctor",5);
}

// **********************************************************************

// Function  : ~CRLImageIFile
// Purpose   : a destructor for the CRLImageIFile class.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

CRLImageIFile::~CRLImageIFile()
{
  char i;

  if (file) fclose(file);
  if (co_map) delete[] co_map;
  if (tiled)
  {
     if (tile_dir) delete[] tile_dir;
     if (cache)
        for (i=0;i<CacheSize;i++)
           if (cache[i])  delete cache[i];
     if (when) delete[] when;
     if (tile) delete[] tile;
  }
  else
     if (s_off) delete[] s_off;
 
  gDebug.msg("CRLImageIFile::~CRLImageIFile(): exiting dtor",5);
}

// **********************************************************************

// Function  : getFileWidth
// Purpose   : returns file with in pixels.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::getFileWidth()
{
  fseek(file, 184, SEEK_SET);
  return readDWord();
}

// **********************************************************************

// Function  : getFileHeight
// Purpose   : returns file height in pixels.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::getFileHeight()
{
  fseek(file, 188, SEEK_SET);
  return readDWord();
}

// **********************************************************************

// Function  : getMaxTileWidth
// Purpose   : returns max tile with in pixels.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::getMaxTileWidth()
{
  if (tiled)
  {
     fseek(file, dir_offset+120, SEEK_SET);
     return readDWord();
  }
  else
  {
     gDebug.msg("CRLImageIFile::getMaxTileWidth(): file is not tiled",2);
     return 0;
  }
}

// **********************************************************************

// Function  : getMaxTileHeight
// Purpose   : returns max tile height in pixels.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::getMaxTileHeight()
{
  return getMaxTileWidth();	// because full tiles are square!
}

// **********************************************************************

// Function  : getTileWidth
// Purpose   : returns tile width in pixels.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::getTileWidth(int tile_num)
{
  long retval;
  int tile_pos;

  if (tiled)
  {
     tile_pos = ( (tile_num - 1) % getTilesWide() ) + 1;
   
     if (tile_pos == getTilesWide())
     {
       // remainder in last column
       retval = ((getFileWidth() - 1) % getMaxTileWidth()) + 1;
     }
     else
     {
       // not in last column
       retval = getMaxTileWidth();
     }
  } 
  else 
  {
     retval = 0;
     gDebug.msg("CRLImageIFile::getTileWidth(): file is not tiled",2);
  }
  return retval;
}

// **********************************************************************

// Function  : getTileHeight
// Purpose   : returns tile height in pixels.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::getTileHeight(int tile_num)
{
  long retval;

  if (tiled)
  {
     if ( tile_num <= (getTilesWide() * (getTilesHigh()-1)) )
     {
       // not in last row
       retval = getMaxTileHeight();
     }
     else
     {
       // remainder in last row
       retval = ((getFileHeight() - 1) % getMaxTileHeight()) + 1;
     }
  }
  else
  {
     retval = 0;
     gDebug.msg("CRLImageIFile::getTileHeight(): file is not tiled",2);
  }
  return retval;
}

// **********************************************************************

// Function  : getTilesWide
// Purpose   : returns file width in tiles.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

int CRLImageIFile::getTilesWide()
{
  if (tiled)
     return ( (getFileWidth() + getMaxTileWidth() - 1) / getMaxTileWidth() );
  else
  {
     gDebug.msg("CRLImageIFile::getTilesWide(): file is not tiled",2);
     return 0;
  }
}

// **********************************************************************

// Function  : getTilesHigh
// Purpose   : returns file height in tiles.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

int CRLImageIFile::getTilesHigh()
{
  if (tiled)
     return ( (getFileHeight() + getMaxTileHeight() - 1) / getMaxTileHeight() );
  else
  {
     gDebug.msg("CRLImageIFile::getTilesHigh(): file is not tiled",2);
     return 0;
  }
}

// **********************************************************************

// Function  : getNumTiles
// Purpose   : returns total number of tiles.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

int CRLImageIFile::getNumTiles()
{
  if (tiled)
     return ( getTilesWide() * getTilesHigh() );
  else
  {
     gDebug.msg("CRLImageIFile::getNumTiles(): file is not tiled",2);
     return 0;
  }
}

// **********************************************************************

// Function  : getColorMap
// Purpose   : returns pointer to color map (array).
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

const ColorCell* CRLImageIFile::getColorMap()
{
  return co_map;
}

// **********************************************************************

// Function  : getTile
// Purpose   : returns pointer to tile.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

Tile* CRLImageIFile::getTile(int tileno)
{
  // user must DELETE memory pointed to by return value as required
  Tile* tilebuf;
  long theight = getTileHeight(tileno);
  long t = 0, f = 0, g = 0;


   if (tiled)
   {
      f = 0;
      t = 0;
      while ((t<CacheSize) && (!f))
      {
         f = (tileno == tile[t]);
         t++;
      }
      if (f != 0)
      {
        tilebuf = cache[t-1];
        when[t-1] = tiletime;
        tiletime++;
      }
      else
      { 
        f= 2147483647;
        for (t=0;t<CacheSize;t++)
            if (f>when[t]) 
            {
               f = when[t];
               g = t;
            }
        if (cache[g] != NULL) delete cache[g];
        tilebuf = new Tile;
        tilebuf->xorg = ((tileno-1) % getTilesWide()) * getMaxTileWidth();
        tilebuf->yorg = ((tileno-1) / getTilesHigh()) * getMaxTileHeight();
        tilebuf->width = getTileWidth(tileno);
        tilebuf->height = getTileHeight(tileno);
   
        tilebuf->data = new unsigned char[getTileSize(tileno)];
   
        fseek(file, tile_dir[tileno].offset, SEEK_SET);

        for (int i = 0; i < theight; i++)
        {
          readScanLine( tilebuf->data + i * tilebuf->width );
        }
        cache[g] = tilebuf;
        when[g]  = tiletime+1;
        tiletime++;
        tile[g]  = tileno;
      } 
   }
   else
   {
      tilebuf = NULL;
      gDebug.msg("CRLImageIFile::getTile(): file is not tiled",2);
   }
   return tilebuf;
}

// **********************************************************************

// Function  : readWord
// Purpose   : reads and inverts word.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

int CRLImageIFile::readWord()
{
  int byte1, byte2;

  byte2 = fgetc(file);
  byte1 = fgetc(file);
 
  return ( (byte1 << 8) + byte2 );
}

// **********************************************************************

// Function  : readDWord
// Purpose   : reads and inverts double-word.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::readDWord()
{
  long byte1, byte2, byte3, byte4;

  byte4 = fgetc(file);
  byte3 = fgetc(file);
  byte2 = fgetc(file);
  byte1 = fgetc(file);

  return ((byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4);
}

// **********************************************************************

// Function  : getTileSize
// Purpose   : returns number of pixels in tile.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

long CRLImageIFile::getTileSize(int tile_num)
{
  if (tiled)
     return (getTileWidth(tile_num) * getTileHeight(tile_num));
  else
     return 0;
}

// **********************************************************************

// Function  : readScanLine
// Purpose   : reads a single scan line.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

void CRLImageIFile::readScanLine(unsigned char *scan)
{
  int runs, i;
  int color, pixels;
  unsigned char* tptr;
  int lineNum;
  unsigned char *buf;

  if (readWord() != 0x5900) 
  {
    char dmsg[80];
    setNoDataBit();
    
    sprintf(dmsg,
            "CRLImageIFile::readScanLine(): code read at %ld was not 0x5900",
            ftell(file)-2);
    gDebug.msg(dmsg,0);
    gDebug.msg("as expected for start-of-scanline",0);
  }

  // number of run lengths
  runs = ((readWord() - 2) / 2);

  // discard line number
  lineNum = readWord();

  buf = new unsigned char[runs*4];
  // pixel offset
  if (readWord())
  {
    setNoDataBit();
    gDebug.msg("CRLImageIFile::readScanLine(): continuation line found, can't read this",0);
  }

  // read rle data and stuff into character array
  fread(buf,1,runs*4,file);
  tptr = scan;
  for (i=0; i<runs; ++i)
  {
    color = (buf[4*i+1] << 8) + buf[4*i];
    pixels = (buf[4*i+3] << 8) + buf[4*i+2];
    memset(tptr,color,pixels);
    tptr += pixels;
  }
  delete[] buf;

  return;
}

// **********************************************************************

// Function  : readColorMap
// Purpose   : puts colormap at co_map
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

void CRLImageIFile::readColorMap()
{

  int ctv, cte, i;

  if (co_map) delete[] co_map;

  fseek(file,532,SEEK_SET);

  // color-table type
  ctv = fgetc(file);

  switch (ctv)
  {
    // no color table
    case 0: 
    {
      gDebug.msg("CRLImageIFile::readColorMap(): no color table found",0);
      break;
    }

    // IGDS color table
    case 1:
    {
      gDebug.msg("CRLImageIFile::readColorMap(): Cannot read IGDS color table.",0);
      break;
    }

    // Environ-V color table
    case 2:
    {
      gDebug.msg("CRLImageIFile::readColorMap(): type is Environ-V",2);
      fseek(file, 0x218, SEEK_SET);

      // number of color table entries
      cte = readDWord();

      co_map = new ColorCell[cte];
      co_map_entries = cte;

      fseek(file, 0x400, SEEK_SET);

      for (i = 0; i < cte; i++)
      {
        co_map[i].slot = readWord();
        co_map[i].red = readWord() >> 8;
        co_map[i].green = readWord() >> 8;
        co_map[i].blue = readWord() >> 8;
      }

      break;
    }

    // unknown color table type
    default:
    {
      gDebug.msg("CRLImageIFile::readColorMap(): Unknown color table type.",1);
    }
 }

 return;
}

// **********************************************************************

// Function  : readTileDir
// Purpose   : puts tile directory at tile_dir.
// Author    : Rob Fugina and Loren Bonebrake
// Date      : June 6, 1994

void CRLImageIFile::readTileDir()
{

  if (tiled)
  {
     fseek(file, 2, SEEK_SET);
     dir_offset = 2 * (readWord() + 2);
   
     if (tile_dir) delete[] tile_dir;
     int numTiles = getTilesHigh() * getTilesWide();
     tile_dir = new DirEntry[numTiles+1];
   
     fseek(file, dir_offset+128, SEEK_SET);
   
     for (int i = 1; i <= numTiles; i++)
     {
       tile_dir[i].offset = dir_offset + readDWord();
       tile_dir[i].alloc = readDWord();
       tile_dir[i].used = readDWord();
     }
  }
  else
     gDebug.msg("CRLImageIFile::readTileDir(): file is not tiled",2);

  return;
}

// **********************************************************************

// Function  : getPixel
// Purpose   : returns a AbstractPixel* to a single pixel.
// Author    : Rob Fugina and Loren Bonebrake and Greg Martin
// Date      : June 6, 1994

//  THIS IS NOW IN COLUMN MAJOR ORDER. 

AbstractPixel* CRLImageIFile::getPixel(long x, long y)
{
  if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
  {
     char dmsg[80];
     setFailRBit();
     sprintf(dmsg,"CRLImageIFile::getPixel(): pixel (%ld,%ld) out of bounds",x,y);
     gDebug.msg(dmsg,1);
     return NULL;
  }
 
  unsigned short r,g,b;
  RGBPixel* pixelbuf;
  unsigned char* foo;

  foo = (unsigned char *) getRawPixel(x,y);
  

  r = (*pallette)[*foo].getRed();
  g = (*pallette)[*foo].getGreen();
  b = (*pallette)[*foo].getBlue();

  pixelbuf = new RGBPixel(r,g,b);
  
  delete foo;

  return((AbstractPixel*)pixelbuf);
}

// **********************************************************************

// Function  : getScanline
// Purpose   : read a whole scan line from the file.
// Author    : Rob Fugina and Loren Bonebrake and Greg Martin
// Date      : June 6, 1994


AbstractPixel* CRLImageIFile::getScanline(long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       setFailRBit();
       sprintf(dmsg,"CRLImageIFile::getScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       return NULL;
    }

    RGBPixel* pixbuf;
    unsigned char* buf;
    int i,w;
    pixbuf = new RGBPixel[getWidth()];

    w = getWidth();
    buf = (unsigned char*)getRawScanline(row);

    for (i=0;i<w;i++)
    {
        pixbuf[i].setRed((*pallette)[buf[i]].getRed()); 
        pixbuf[i].setGreen((*pallette)[buf[i]].getGreen()); 
        pixbuf[i].setBlue((*pallette)[buf[i]].getBlue()); 
    }
    delete[] buf;
       
    return (AbstractPixel*)pixbuf;
}

// **********************************************************************

// Function  : getRectangle
// Purpose   : returns a rectangluar pixel.
// Author    : Rob Fugina and Loren Bonebrake and Greg Martin
// Date      : June 6, 1994
// Note      : This function keeps a cache of tiles

//  THIS IS NOW IN COLUMN MAJOR ORDER

AbstractPixel* CRLImageIFile::getRectangle(long x1, long y1, long x2, long y2)
{
   if ((y1 < 0) || (y1 > getWidth()-1) || (x1 < 0) || (x1 > getHeight()-1) ||
      (y2 < 0) || (y2 > getWidth()-1) || (x2 < 0) || (x2 > getHeight()-1) ||
      (x1>x2) || (y1>y2))
   {
      char dmsg[80];
      setFailRBit();
      sprintf(dmsg,"CRLImageIFile::getRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
      gDebug.msg(dmsg,1); 
      return NULL;
   }

   RGBPixel* pixbuf;
   unsigned char *buf;
   int i;
   int l = (x2-x1+1)*(y2-y1+1);

   pixbuf = new RGBPixel[l];
 
   buf = (unsigned char *) getRawRectangle(x1,y1,x2,y2);
   for (i=0;i<l;i++)
   {
       pixbuf[i].setRed((*pallette)[buf[i]].getRed());
       pixbuf[i].setGreen((*pallette)[buf[i]].getGreen());
       pixbuf[i].setBlue((*pallette)[buf[i]].getBlue());
   }
   delete[] buf;

   return((AbstractPixel*)pixbuf);
}

void CRLImageIFile::setupPallette()
{
    int colors=0; 
    int stop;
    int last=-1;
    RGBPixel* foo;
    char used[256];

    for (colors=0;colors<256;colors++) 
    {      
        used[colors] = 0;
        if (colors<co_map_entries)
        {
           if ((int)co_map[colors].slot > last)
           {
              last = co_map[colors].slot;
           }
        }
    }

    pallette = new RGBPallette(last+1); 
    
    colors = 0;
    stop = 0;
    
    while (colors<co_map_entries)
    {
        if (used[co_map[colors].slot] == 0) 
        {
           foo = new RGBPixel(co_map[colors].red,
                              co_map[colors].green,
                              co_map[colors].blue); 

           pallette->setEntry(co_map[colors].slot,foo);
           delete foo;
           used[co_map[colors].slot] = 1;
        }
        colors++;
    }
} 


// **********************************************************************

// Function  : getPallette
// Purpose   : returns a Pallette of the colormap.
// Author    : Rob Fugina and Loren Bonebrake and Greg Martin
// Date      : June 6, 1994

Pallette* CRLImageIFile::getPallette()
{
  RGBPallette* pal;
  RGBPixel* pixel;
  int i;

  pal = new RGBPallette(pallette->getNoEntries());

  for(i=0;i<pallette->getNoEntries();i++)
  {
       pixel = new RGBPixel;
       pixel->setRed((*pallette)[i].getRed());
       pixel->setGreen((*pallette)[i].getGreen());
       pixel->setBlue((*pallette)[i].getBlue());
       pal->setEntry(i, pixel);
       delete pixel;
  }

  return ((Pallette*)pal);
}

// **********************************************************************

//  THIS IS NOW IN COLUMN MAJOR ORDER

void* CRLImageIFile::getRawPixel(long x, long y)
{
  if ((x < 0) || (x > getWidth()-1) || (y < 0) || (y > getHeight()-1))
  {
     char dmsg[80];
     sprintf(dmsg,"CRLImageIFile::getRawPixel(): pixel (%ld,%ld) out of bounds",x,y);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }
 
  unsigned char* pixel;
  pixel = new unsigned char;

  if (tiled)
  {
     long localx, localy, tilex, tiley, tileno, width, height, wide;
     long twidth;
     Tile* tilebuf;
     width = getMaxTileWidth();
     height = getMaxTileHeight();
     wide = getTilesWide();
   
     localy = y % height;
     localx = x % width;
     tiley = y / height;
     tilex = x / width;
   
     tileno = (tiley*wide)+tilex+1;
   
     tilebuf = getTile(tileno);
     twidth  = getTileWidth(tileno);
     
     *pixel = tilebuf->data[localy*twidth+localx];
  }
  else
  {
     short entry;
     unsigned char* line;
     entry = cacheScanline(x);
     line = rowCache[entry];
     *pixel = line[x];
  }

  return pixel;
}

// **********************************************************************

void* CRLImageIFile::getRawScanline(long row)
{
    if ((row < 0) || (row > getHeight()-1))
    {
       char dmsg[80];
       sprintf(dmsg,"CRLImageIFile::getRawScanline(): scanline %ld out of bounds",row);
       gDebug.msg(dmsg,1);
       setFailRBit();
       return NULL;
    }
    unsigned char* pixbuf;
    pixbuf = new unsigned char[getWidth()];

    if (tiled)
    {
       Tile* tilebuf = NULL;
       long iwidth, width, twidth, k, j, localy, tiley;
       long tileno, height, wide;
       unsigned char *sptr, *tptr;
   
       height = getMaxTileHeight();
       
       localy = row % height;
       tiley  = row / height;
   
       iwidth = getWidth();
       width  = getMaxTileWidth();
       wide   = getTilesWide();
   
       k = 0;
   
       for (j=0;j<wide;j++)
       {
          tileno = tiley*wide+j+1;

          tilebuf = getTile(tileno);
          twidth = getTileWidth(tileno);
          tptr = &(pixbuf[k]);
          sptr = &(tilebuf->data[localy*twidth]);
          memcpy(tptr,sptr,twidth);
          k+=twidth;
       }
    }
    else
    {
       short entry;
       unsigned char* line;
       int w = getWidth();
       entry = cacheScanline(row);
       line = rowCache[entry];
       memcpy(pixbuf,line,w);
    }

    return pixbuf;
}


// **********************************************************************

//  THIS IS NOW IN COLUMN MAJOR ORDER

void* CRLImageIFile::getRawRectangle(long x1, long y1, long x2, long y2)
{
  if ((x1 < 0) || (x1 > getWidth()-1) || (y1 < 0) || (y1 > getHeight()-1) ||
     (x2 < 0) || (x2 > getWidth()-1) || (y2 < 0) || (y2 > getHeight()-1) ||
     (y1>y2) || (x1>x2))
  {
     char dmsg[100];
     sprintf(dmsg,"CRLImageIFile::getRawRectangle(): rectangle (%ld,%ld,%ld,%ld) out of bounds",x1,y1,x2,y2);
     gDebug.msg(dmsg,1);
     setFailRBit();
     return NULL;
  }

    unsigned char* pixbuf;
    pixbuf = new unsigned char[(x2-x1+1)*(y2-y1+1)];

    if (tiled)
    {
       Tile* tilebuf = NULL;
       long height, width, wide, tilex, tiley, tileno, oldtile, localx, localy;
       long imagex = 0, imagey = 0, k = 0, tilewidth = 0; 
   
       height = getMaxTileHeight();
       width  = getMaxTileWidth();
       wide   = getTilesWide();
   
       oldtile = 0;
       k = 0;
       for (imagey=y1;imagey<=y2;imagey++)
       {
           tiley = imagey/height;
           localy = imagey % height;
           for (imagex=x1;imagex<=x2;imagex++)
           {
               tilex = imagex/width;
               tileno = tiley*wide + tilex+1;
               localx = imagex % width;
               if (oldtile != tileno)
               {
                  oldtile = tileno;
                  tilebuf=getTile(tileno);
                  tilewidth = getTileWidth(tileno);
               }      
               pixbuf[k] = tilebuf->data[localy*tilewidth+localx];
               k++;
           }
       }
    }
    else
    {
       int y;
       unsigned char *sptr, *tptr;
       unsigned char *buf;
       buf = new unsigned char[getWidth()];
       tptr = buf;
       for (y=y1;y<=y2;y++)
       {
           fseek(file,s_off[y],SEEK_SET);
           readScanLine(buf);
           sptr = &(buf[x1]);
           memcpy(tptr,sptr,x2-x1+1);
           tptr += x2-x1+1;
       }
       delete[] buf;
    }
           
    return pixbuf;
}

// **********************************************************************

void CRLImageIFile::getRawRGBPixel(long x, long y, RawRGBPixel* pix)
{
   unsigned char* foo;

   foo = (unsigned char *)getRawPixel(x,y);
   if (good())
   {
      pix->Red   = (*pallette)[foo[x]].getRed();  
      pix->Green = (*pallette)[foo[x]].getGreen();  
      pix->Blue  = (*pallette)[foo[x]].getBlue();  
   }
   
}

void CRLImageIFile::getRawRGBScanline(long row, RawRGBPixel* pixbuf)
{
   int i;
   int w = getWidth();
   unsigned char *buf;
  
   buf = (unsigned char *) getRawScanline(row);
   for (i=0;i<w;i++)
   {
        pixbuf[i].Red   = (*pallette)[buf[i]].getRed();
        pixbuf[i].Green = (*pallette)[buf[i]].getGreen();
        pixbuf[i].Blue  = (*pallette)[buf[i]].getBlue();
   }
}

void CRLImageIFile::getRawRGBRectangle(long x1, long y1, long x2, long y2,
                                       RawRGBPixel* pixbuf)
{
   int i;
   int h = (y2 - y1 + 1);
   int w = (x2 - x1 + 1);
   int l = w*h;
   unsigned char *buf;
  
   buf = (unsigned char *) getRawRectangle(x1,y1,x2,y2);
   for (i=0;i<l;i++)
   {
        pixbuf[i].Red   = (*pallette)[buf[i]].getRed();
        pixbuf[i].Green = (*pallette)[buf[i]].getGreen();
        pixbuf[i].Blue  = (*pallette)[buf[i]].getBlue();
   }
}

void CRLImageIFile::getScanlineOffsets(void)
{
   int curline;
   int next;
   int id;
   int wtf;
   int height = getHeight();

   next = topOfData;
   s_off = new int[getHeight()]; 
   s_off[0] = topOfData;
   curline = 1;

   fseek(file,topOfData,SEEK_SET);
   while (curline < height)
   {
         id = readWord();
         wtf = readWord();
         if (id != 0x5900)
         {
            setNoDataBit();
            gDebug.msg("CRLImageIFile::getPixel(): Invalid scanline header",1);
         }
         next += (wtf*2)+4;
         fseek(file,next,SEEK_SET); 
         s_off[curline] = next;
         curline++;
   }
}


short CRLImageIFile::cacheScanline(int row)
{
   int i;
   short entry;
   unsigned int min;

   if (hashTable[row] == -1)
   {
      min = 0;
      entry = 0;
      for (i=1;i<50;i++)
          if (timeStamp[i] < min )
          {
             min = timeStamp[i];
             entry = i;
          }
      lastTimeStamp++;
      if (rowCache[entry] == NULL)
         rowCache[entry] = new unsigned char[getWidth()];
      if (rowNumber[entry] != -1)
         hashTable[rowNumber[entry]] = -1;
      timeStamp[entry] = lastTimeStamp;
      hashTable[row] = entry;
      rowNumber[entry] = row;

      fseek(file,s_off[row],SEEK_SET);
      readScanLine(rowCache[entry]);
   }
   else
      entry = hashTable[row];

   return entry;
}

bool CRLImageIFile::getOptimalRectangle(int &w, int &h)
{
   if (tiled)
   {
      w = getMaxTileWidth();
      h = getMaxTileHeight();
   }
   else
   {
      w = getWidth();
      h = 0;
   }
   return true;
}
