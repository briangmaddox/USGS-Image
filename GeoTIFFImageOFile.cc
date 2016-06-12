#include "Image/UTMN.h"
#include "Image/GeoTIFFImageOFile.h"
#include "Image/ImageDebug.h"
#include "geotiff/xtiffio.h"


GeoTIFFImageOFile::GeoTIFFImageOFile(const char* gtfilename,long w, long h):
TIFFImageOFile(gtfilename,w,h)
{
   gtif = GTIFNew(tif);
   gDebug.msg("GeoTIFFImageOFile::GeoTIFFImageOFile: exiting ctor",5);
}


GeoTIFFImageOFile::GeoTIFFImageOFile(const char* gtfilename,long w, long h,
				     int compression)
  : TIFFImageOFile(gtfilename,w,h,compression)
{
  gtif = GTIFNew(tif);
  gDebug.msg("GeoTIFFImageOFile::GeoTIFFImageOFile(c,w,h,c): exiting ctor",5);
}


GeoTIFFImageOFile::GeoTIFFImageOFile(const char* gtfilename, long w, long h,
				     int samples_pixel, int bits_sample,
				     int comp, int photometric)
  : TIFFImageOFile(gtfilename, w, h, samples_pixel, bits_sample, comp,
		   photometric)
{
  gtif = GTIFNew(tif);
  gDebug.msg("GeoTIFFImageOFile::GeoTIFFImageOFile(c,l,l,i,i,i,i): exiting ctor", 5);
}


GeoTIFFImageOFile::~GeoTIFFImageOFile()
{
   GTIFWriteKeys(gtif);
   GTIFFree(gtif);
   gDebug.msg("GeoTIFFImageOFile::~GeoTIFFImageOFile: exiting dtor",5);
}

// The old function. Please stop using this one!!!!
bool GeoTIFFImageOFile::setUSGS_UTM(double x, double y,
                                    double xres, double yres, 
                                    unsigned short utmzone)
{
   double tp[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
   double res[3] = { 0.0, 0.0, 0.0 };
   char cit[100];
   bool r;
   unsigned short pcstype;

   tp[3] = x;
   tp[4] = y;
   setTiePoints(tp,6);
   res[0] = xres;
   res[1] = yres;
   setPixelScale(res);
   r = setGeoKey(GTModelTypeGeoKey,(unsigned short)ModelProjected);
   if (!r) return r;
   r = setGeoKey(GTRasterTypeGeoKey,(unsigned short)RasterPixelIsArea);
   if (!r) return r;

   pcstype = UTMN + utmzone;
   r = setGeoKey(ProjectedCSTypeGeoKey,pcstype);
   if (!r) return r;
   sprintf(cit,"UTM Zone %d N with NAD27",utmzone);
   r = setGeoKey(PCSCitationGeoKey,cit);
   return r;
}
   

// The new function.  Please use this one!!!
bool GeoTIFFImageOFile::setUSGS_UTM(double x, double y,
                                    double xres, double yres, 
                                    unsigned short utmzone,
				    int nadtype)
{
   double tp[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
   double res[3] = { 0.0, 0.0, 0.0 };
   char cit[100];
   bool r;
   unsigned short pcstype;

   tp[3] = x;
   tp[4] = y;
   setTiePoints(tp,6);
   res[0] = xres;
   res[1] = yres;
   setPixelScale(res);
   r = setGeoKey(GTModelTypeGeoKey,(unsigned short)ModelProjected);
   if (!r) return r;
   r = setGeoKey(GTRasterTypeGeoKey,(unsigned short)RasterPixelIsArea);
   if (!r) return r;

   // Okie, these two switch statements handle the NAD encoding that we
   // currently support.  By default, they go back to NAD27.  Note that if
   // any NAD types are added, tif2usgsdrg.cc must be modified also so that
   // it will know that there's a new supported NAD (heh heh).  BGM 26 Nov 1996
   switch(nadtype)
   {
     case 1927:   // Use the NAD27 code
     {
       pcstype = UTMN + utmzone;
       break;
     }
     case 1983:   // NAD83 code
     {
       pcstype = UTMN83 + utmzone;
       break;
     }
     default:   // Default to NAD27
     {
       pcstype = UTMN + utmzone;
     }
   }
   //   cout << flush << "Using " << pcstype << " as geokey" << endl;
   r = setGeoKey(ProjectedCSTypeGeoKey,pcstype);
   if (!r) return r;

   switch(nadtype)
   {
      case 1927:
      {
	sprintf(cit,"UTM Zone %d N with NAD27",utmzone);
	break;
      }
      case 1983:
      {
	sprintf(cit,"UTM Zone %d N with NAD83",utmzone);
	break;
      }
      default:
      {
	sprintf(cit,"UTM Zone %d N with NAD27",utmzone);
      }
   }
   r = setGeoKey(PCSCitationGeoKey,cit);
   return r;
}

bool GeoTIFFImageOFile::setUSGS_SPCS(double x, double y,
				     double xres, double yres, 
				     unsigned short spcstype, int nadtype)
{
   double tp[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
   double res[3] = { 0.0, 0.0, 0.0 };
   char cit[100];
   bool r;
   unsigned short pcstype;

   tp[3] = x;
   tp[4] = y;
   setTiePoints(tp,6);
   res[0] = xres;
   res[1] = yres;
   setPixelScale(res);
   r = setGeoKey(GTModelTypeGeoKey,(unsigned short)ModelProjected);
   if (!r) return r;
   r = setGeoKey(GTRasterTypeGeoKey,(unsigned short)RasterPixelIsArea);
   if (!r) return r;

   r = setGeoKey(ProjectedCSTypeGeoKey,spcstype);
   if (!r) return r;

   return r;
}

bool GeoTIFFImageOFile::setTiePoints(double* tp, int num)
{
   TIFFSetField(tif,TIFFTAG_GEOTIEPOINTS,num,tp);
   return true;
}

bool GeoTIFFImageOFile::setPixelScale(double* ps)
{
   TIFFSetField(tif,TIFFTAG_GEOPIXELSCALE,3,ps);
   return true;
}

bool GeoTIFFImageOFile::getGeoKeyInfo(geokey_t key, int *size, tagtype_t* type)
{
   int r;

   r = GTIFKeyInfo(gtif,key,size,type);
   return (r!=0); 
}


bool GeoTIFFImageOFile::setGeoKey(geokey_t key, unsigned short sval)
{
   int r;

   r = GTIFKeySet(gtif,key,TYPE_SHORT,1,sval);

   return (r!=0);
}


bool GeoTIFFImageOFile::setGeoKey(geokey_t key, double dval)
{
   int r;

   r = GTIFKeySet(gtif,key,TYPE_DOUBLE,1,dval);

   return (r!=0);
}


bool GeoTIFFImageOFile::setGeoKey(geokey_t key, char* cval)
{
   int r;

   r = GTIFKeySet(gtif,key,TYPE_ASCII,0,cval);

   return (r!=0);
}

bool GeoTIFFImageOFile::getGeoKey(geokey_t key, unsigned short* sval)
{
   int r;

   r = GTIFKeyGet(gtif,key,sval,0,1);

   return (r!=0);
}


bool GeoTIFFImageOFile::getGeoKey(geokey_t key, double* dval)
{
   int r;

   r = GTIFKeyGet(gtif,key,dval,0,1);

   return (r!=0);
}

bool GeoTIFFImageOFile::getGeoKey(geokey_t key, char* cval)
{
   int r;

   r = GTIFKeyGet(gtif,key,cval,0,0);

   return (r!=0);
}


char* GeoTIFFImageOFile::getKeyName(geokey_t key)
{
   return GTIFKeyName(key);
}


char* GeoTIFFImageOFile::getValueName(geokey_t key, int value)
{
   return GTIFValueName(key,value);
}


