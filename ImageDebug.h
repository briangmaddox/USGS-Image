#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <string.h>
#include <iostream.h>
#include <stdio.h>


class _ImageDebug
{
  int debugLevel;

public:
    
  _ImageDebug();
  _ImageDebug(int);

  void setDebugLevel(int);
  int  getDebugLevel(void);
  void msg(const char*,int);
};

#ifndef INCINDEBUGCC
extern _ImageDebug gDebug;
#endif

#endif
