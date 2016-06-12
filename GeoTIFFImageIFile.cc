#include "Image/UTMN.h"
#include "Image/GeoTIFFImageIFile.h"
#include "Image/ImageDebug.h"
#include "geotiff/xtiffio.h"


GeoTIFFImageIFile::GeoTIFFImageIFile(const char* gtfilename):
TIFFImageIFile(gtfilename)
{
   gtif = GTIFNew(tif);
   gDebug.msg("GeoTIFFImageIFile::GeoTIFFImageIFile(): exiting ctor",5);
}


GeoTIFFImageIFile::~GeoTIFFImageIFile()
{
   GTIFFree(gtif);
   gDebug.msg("GeoTIFFImageIFile::~GeoTIFFImageIFile(): exiting dtor",5);
}


bool GeoTIFFImageIFile::getGeoKeyInfo(geokey_t key, int *size, tagtype_t* type)
{
   int r;

   r = GTIFKeyInfo(gtif,key,size,type);
   return (r!=0); 
}

bool GeoTIFFImageIFile::getTiePoints(double** tp, unsigned short* num)
{
   double *points;
   int i;
  
   TIFFGetField(tif,TIFFTAG_GEOTIEPOINTS,num,&points);
   *tp = new double[*num];
   for (i=0;i<*num;i++) 
       (*tp)[i] = points[i];  
   return true;
}

bool GeoTIFFImageIFile::getPixelScale(double** ps)
{
   unsigned short scount=3;
   double *scales;
   int i;

   TIFFGetField(tif,TIFFTAG_GEOPIXELSCALE,&scount,&scales);
   *ps = new double[scount];
   for (i=0;i<scount;i++)
      (*ps)[i] = scales[i];
   return true;
}

bool GeoTIFFImageIFile::getUSGS_UTM(double *x, double *y,
                                    double *xres, double *yres,
                                    unsigned short *utmzone)
{
   bool r;
   double *tp;
   double *res;
   unsigned short dummy;
   unsigned short scount=6;
   char str[100];

   r = getGeoKey(GTModelTypeGeoKey,&dummy);
   if (!r) return r;
   if (dummy != (unsigned short)ModelProjected) return false;
   r = getGeoKey(GTRasterTypeGeoKey,&dummy);
   if (!r) return r;
   if (dummy != (unsigned short)RasterPixelIsArea) return false;
   r = getGeoKey(ProjectedCSTypeGeoKey,&dummy);
   if (!r) return r;
//   if (((dummy - UTMN) <= 60) && ((dummy - UTMN) >= 1)) 
//      *utmzone = dummy - UTMN;
//   else if (((dummy - UTMS) <= 60) && ((dummy - UTMS) >= 1))
//      *utmzone = dummy - UTMS;
//   else 
//      return false;  

// Below is the WRONG way of doing this. This is a cludge to replace
// Greg's cludge, which was even more wrong. This will work OK for
// NAD27 and WGS84 zones inside the US, but it may die unexpectedly
// if an attempt is made to check date from outside the US.
   *utmzone = dummy - ((dummy/100)*100);

   r = getGeoKey(PCSCitationGeoKey,str);
   if (!r) return r;
   str[3] = (char)0;
   if (strcmp(str,"UTM") != 0) return false;
   r = getTiePoints(&tp,&scount);
   if (!r) return r;
   *x = tp[3]; 
   *y = tp[4];
   r = getPixelScale(&res);
   if (!r) return r;
   *xres = res[0];
   *yres = res[1];

   delete[] tp;
   delete[] res;
   return true;
}


bool GeoTIFFImageIFile::getGeoKey(geokey_t key, unsigned short* sval)
{
   int r;

   r = GTIFKeyGet(gtif,key,sval,0,1);

   return (r!=0);
}


bool GeoTIFFImageIFile::getGeoKey(geokey_t key, double* dval)
{
   int r;

   r = GTIFKeyGet(gtif,key,dval,0,1);

   return (r!=0);
}


bool GeoTIFFImageIFile::getGeoKey(geokey_t key, char* cval)
{
   int r;

   r = GTIFKeyGet(gtif,key,cval,0,0);

   return (r!=0);
}


char* GeoTIFFImageIFile::getKeyName(geokey_t key)
{
   return GTIFKeyName(key);
}


char* GeoTIFFImageIFile::getValueName(geokey_t key, int value)
{
   return GTIFValueName(key,value);
}


