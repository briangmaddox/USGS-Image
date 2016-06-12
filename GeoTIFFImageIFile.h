#ifndef _GEOTIFFIMAGEIFILE_H_
#define _GEOTIFFIMAGEIFILE_H_

#include "Image/TIFFImageIFile.h"
#include "geotiff/geotiff.h"
#include "geotiff/geotiffio.h"

class GeoTIFFImageIFile : public TIFFImageIFile
{
   GTIF *gtif;

   public:

   GeoTIFFImageIFile(const char* gtfilename);
   virtual ~GeoTIFFImageIFile();   


   bool getGeoKeyInfo(geokey_t key, int *size, tagtype_t* type);

   bool getTiePoints(double **tp, unsigned short *num);
   bool getPixelScale(double **ps);
   
   bool getGeoKey(geokey_t key, unsigned short* sval);
   bool getGeoKey(geokey_t key, double* dval);
   bool getGeoKey(geokey_t key, char* cval);             // Must allocate space
   
   bool getUSGS_UTM(double *x, double *y,
                    double *xres, double *yres,
                    unsigned short *utmzone);
   char* getKeyName(geokey_t key);
   char* getValueName(geokey_t key, int value);
};

#endif
