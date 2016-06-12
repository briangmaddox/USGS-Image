//  DOQImageIFile.h                   27 Oct 94
//  Authors:  Greg Martin, SES, MCMC, USGS
//            Eric Heien,  SES, MCMC, USGS

//  Defines the class DOQ_OldImageIFile

//  This class handles the reading of raw greyscale raster files


#ifndef _DOQ_OldImageIFile_H_
#define _DOQ_OldImageIFile_H_

#include <fstream.h>
#include <string.h>
#include "Image/ImageIFile.h"
#include "Image/Pallette.h"


class DOQ_OldImageIFile : public ImageIFile
{
    
        ifstream* Stream;      // is called inout in source
        char*     headerFile;
        long TopOfImageData;

        int HeaderDataRecordLength;
        int NumberHeaderRecords;
        int HeaderRecordPad;
        int start;
        char temp;
        char buffer[401];
        char* ret;
	// Record 1   
        char quadname[39];
        char quadrant[3];
        char nation1[3];
        char nation2[3];
        char state1[3];
        char state2[3];
        char state3[3];
        char state4[3];
        char st1co1[4];
        char st1co2[4];
        char st1co3[4];
        char st1co4[4];
        char st1co5[4];
        char st2co1[4];
        char st2co2[4];
        char st2co3[4];
        char st2co4[4];
        char st2co5[4];
        char st3co1[4];
        char st3co2[4];
        char st3co3[4];
        char st3co4[4];

        char st3co5[4];
        char st4co1[4];
        char st4co2[4];
        char st4co3[4];
        char st4co4[4];
        char st4co5[4];
        char reserved1a[3];
        char filler[24];
        char producer[5];
        char reserved1b[2];

        int datorder;
        int numlines;
        int numsamples;
        int bandtyps;
        int elevstor;
        int bestore;
        int vertdat;
        int prhordat;
        int sdhordat;
        double angle;

        int grid;
        int zone;
        int xyunits;

        double prqcorsw[2];
        double prqcornw[2];
        double prqcorne[2];
        double prqcorse[2];



// Record 2

        double prxforma;
        double prxformb;
        double prxformc;
        double prxformd;
        double prxforme;
        double prxformf;
        double prxformx;
        double prxformy;
        double sdqcorsw[2];
        double sdqcornw[2];
        double sdqcorne[2];
        double sdqcorse[2];
        char reserved2[17];


// Record 3

        double sdxforma;
        double sdxformb;
        double sdxformc;
        double sdxformd;
        double sdxforme;
        double sdxformf;
        double sdxformx;
        double sdxformy;

        int prqclssw[2];
        int prqclsnw[2];
        int prqclsne[2];
        int prqclsse[2];
        int sdqclssw[2];
        int sdqclsnw[2];
        int sdqclsne[2];
        int sdqclsse[2];

        double x1y1[2];
        double xnyn[2];
        char reserved3[17];


// Record 4

        int elevunit;

        double minelev;
        double maxelev;
        double xgrndres;
        double ygrndres;
        double zgrndres;
        double xpixres;
        double ypixres;
        double zpixres;

        int maxconin;
        int maxconun;
        int minconin;
        int minconun;
        int susareac;

        double horizacc;
        double vertacc;

        int numhortp;
        int pixproal;

        char prodsyst[25];

        int proddate[3];

        char filmtype[25];
        char photoid[25];

        int moscode;

        char loffflag[3];

        int photodat[3];

        double focallen;

        int flyhgt;

        char scantype[25];

        double xyscres[2];

        double xyssres[2];

        int radres;

        double resmpres;
 
        char compflag;

        int compfilesize;

        char compalgor[21];

        char comptype;

        int compcontrol;

        char reserved4[104];

  public:

   DOQ_OldImageIFile(const char* fn);
   virtual ~DOQ_OldImageIFile();


//  Methods to get at the header data

   char* getQuadName(void);
   char* getQuadrant(void);
   char* getNation1(void);
   char* getNation2(void);
   char* getState1(void);
   char* getState2(void);
   char* getState3(void);
   char* getState4(void);
   char* getSt1co1(void);
   char* getSt1co2(void);
   char* getSt1co3(void);
   char* getSt1co4(void);
   char* getSt1co5(void);
   char* getSt2co1(void);
   char* getSt2co2(void);
   char* getSt2co3(void);
   char* getSt2co4(void);
   char* getSt2co5(void);
   char* getSt3co1(void);
   char* getSt3co2(void);
   char* getSt3co3(void);
   char* getSt3co4(void);
   char* getSt3co5(void);
   char* getSt4co1(void);
   char* getSt4co2(void);
   char* getSt4co3(void);
   char* getSt4co4(void);
   char* getSt4co5(void);
   char* getFiller(void);
   char* getProducer(void);
   int  getDatorder(void);
   int  getNumlines(void);
   int  getNumsamples(void);
   int  getBandtyps(void);
   int  getElevstor(void);
   int  getBestore(void);
   int  getVertdat(void);
   int  getPrhordat(void);
   int  getSdhordat(void);
   double getAngle(void);
   int getGrid(void);
   int getZone(void);
   int getXyunits(void);
   double getPrqcorsw(int i);
   double getPrqcornw(int i);
   double getPrqcorne(int i);
   double getPrqcorse(int i);
   double getPrxforma(void);
   double getPrxformb(void);
   double getPrxformc(void);
   double getPrxformd(void);
   double getPrxforme(void);
   double getPrxformf(void);
   double getPrxformx(void);
   double getPrxformy(void);
   double getSdqcorsw(int i);
   double getSdqcornw(int i);
   double getSdqcorne(int i);
   double getSdqcorse(int i);
   double getSdxforma(void);
   double getSdxformb(void);
   double getSdxformc(void);
   double getSdxformd(void);
   double getSdxforme(void);
   double getSdxformf(void);
   double getSdxformx(void);
   double getSdxformy(void);
   int getPrqclssw(int i);
   int getPrqclsnw(int i);
   int getPrqclsne(int i);
   int getPrqclsse(int i);
   int getSdqclssw(int i);
   int getSdqclsnw(int i);
   int getSdqclsne(int i);
   int getSdqclsse(int i);
   double getX1y1(int i);
   double getXnyn(int i);
   int getElevunit(void);
   double getMinelev(void);
   double getMaxelev(void);
   double getXgrndres(void);
   double getYgrndres(void);
   double getZgrndres(void);
   double getXpixres(void);
   double getYpixres(void);
   double getZpixres(void);
   int    getMaxconin(void);
   int    getMaxconun(void);
   int    getMinconin(void);
   int    getMinconun(void);
   int getSusareac(void);
   double getHorizacc(void);
   double getVertacc(void);
   int getNumhortp(void);
   int  getPixproal(void);
   char* getProdsyst(void);
   int getProddate(int i);
   char* getFilmtype(void);
   char* getPhotoid(void);
   int getMoscode(void);
   char* getLoffflag(void);
   int getPhotodat(int i);
   double getFocallen(void);
   int getFlyhgt(void);
   char* getScantype(void);
   double getXyscres(int i);
   double getXyssres(int i);
   int getRadres(void);
   double getResmpres(void);
   char getCompflag(void);
   int getCompfilesize(void);
   char* getCompalgor(void);
   char getComptype(void);
   int getCompcontrol(void);


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
 
   void readHeader(void);  // Called readHeaderData in the source
};

#endif
