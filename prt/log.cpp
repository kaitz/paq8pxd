#include "log.hpp"
///////////////////////////// ilog //////////////////////////////

// ilog(x) = round(log2(x) * 16), 0 <= x < 64K
class Ilog {
  Array<U8> t;
public:
  int operator()(U16 x) const {return t[x];}
  Ilog();
   int llog(U32 x) {
  if (x>=0x1000000)
    return 256+t[x>>16];
  else if (x>=0x10000)
    return 128+t[x>>8];
  else
    return t[x];
}
} ilog1;


// Compute lookup table by numerical integration of 1/x
Ilog::Ilog(): t(65536) {
  U32 x=14155776;
  for (int i=2; i<65536; ++i) {
    x+=774541002/(i*2-1);  // numerator is 2^29/ln 2
    t[i]=x>>24;
  }
}

int ilog(U16 x)  {return ilog1(x);}
int llog(U32 x)  {return ilog1.llog(x);}
