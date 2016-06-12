#define INCINDEBUGCC
#include "Image/ImageDebug.h"

_ImageDebug::_ImageDebug()
{
   setDebugLevel(0);
}

_ImageDebug::_ImageDebug(int l)
{
   l = 0;
   setDebugLevel(0);
}

void _ImageDebug::setDebugLevel(int l)
{
   debugLevel = l;
}

int _ImageDebug::getDebugLevel(void)
{
   return debugLevel;
}

void _ImageDebug::msg(const char* msg, int l)
{
  /*  if (l<=debugLevel)
    cerr << msg << endl;
  */
}

_ImageDebug gDebug;

