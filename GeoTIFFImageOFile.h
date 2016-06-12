#ifndef _GEOTIFFIMAGEOFILE_H_
#define _GEOTIFFIMAGEOFILE_H_

#include "Image/TIFFImageOFile.h"
#include "geotiff/geotiff.h"
#include "geotiff/geotiffio.h"

class GeoTIFFImageOFile : public TIFFImageOFile
{
  ///
  GTIF *gtif;

public:

  GeoTIFFImageOFile(const char* gtfilename,long w, long h);
  GeoTIFFImageOFile(const char* gtfilename, long w, long h, int compression);
  GeoTIFFImageOFile(const char* gtfilename, long w, long h, int samples_pixel,
		    int bits_sample, int comp, int photometric);
  virtual ~GeoTIFFImageOFile();   


  bool getGeoKeyInfo(geokey_t key, int *size, tagtype_t* type);
  
  bool setTiePoints(double* tp, int num);
  bool setPixelScale(double* ps);
  bool getGeoKey(geokey_t key, unsigned short* sval);
  bool getGeoKey(geokey_t key, double* dval);
  bool getGeoKey(geokey_t key, char* cval);             // Must allocate space
  bool setGeoKey(geokey_t key, unsigned short sval);
  bool setGeoKey(geokey_t key, double dval);
  bool setGeoKey(geokey_t key, char* cval);

  /// Default, handle NAD27 only - please start phasing this call out in favor
  /// of the new one below - BGM
  bool setUSGS_UTM(double x, double y,
		   double xres, double yres,
		   unsigned short utmzone);

  /// Handle any NAD type
  bool setUSGS_UTM(double x, double y,
		   double xres, double yres,
		   unsigned short utmzone,
		   int nadtype);
   
  // This is a hack, sorta.  Handles SPCS output.  Right now, I just pass in
  // the zone.  Am doing it this way since I'm rewriting it now and don't feel
  // like doing anything cool here :)
  bool setUSGS_SPCS(double x, double y,
		    double xres, double yres,
		    unsigned short spcstype, int nadtype);

  char* getKeyName(geokey_t key);
  char* getValueName(geokey_t key, int value);
};

#endif
