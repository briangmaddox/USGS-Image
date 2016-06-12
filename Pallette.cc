//  Pallette.cc                         11 Jul 94
//  Author:  Greg Martin  SES, MCMC, USGS

//  Implements member functions for the class Pallette 


#include "Image/Pallette.h"
#include "Image/ImageDebug.h"
#include <iostream.h>

Pallette::Pallette(void)
{
    NoEntries = 0;
    gDebug.msg("Pallette::Pallette(): exiting ctor",5);
}

Pallette::Pallette(int n)
{
    NoEntries = n;
    gDebug.msg("Pallette::Pallette(i): exiting ctor",5);
}

Pallette::~Pallette()
{
    gDebug.msg("Pallette::~Pallette(): exiting dtor",5);
}

int Pallette::getNoEntries(void)
{
    return NoEntries;
}

void Pallette::setNoEntries(int n)
{
    NoEntries = n;
    gDebug.msg("Pallette::setNoEntries(): NoEntries set",5);
}
