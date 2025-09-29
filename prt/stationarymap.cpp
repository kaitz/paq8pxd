#include "stationarymap.hpp"
/*
  Map for modelling contexts of (nearly-)stationary data.
  The context is looked up directly. For each bit modelled, a 32bit element stores
  a 22 bit prediction and a 10 bit adaptation rate offset.

  - BitsOfContext: How many bits to use for each context. Higher bits are discarded.
  - InputBits: How many bits [1..8] of input are to be modelled for each context.
    New contexts must be set at those intervals.
  - Rate: Initial adaptation rate offset [0..1023]. Lower offsets mean faster adaptation.
    Will be increased on every occurrence until the higher bound is reached.

    Uses (2^(BitsOfContext+2))*((2^InputBits)-1) bytes of memory.
*/


extern int dt[1024];  // i -> 16K/(i+i+3)

StationaryMap::StationaryMap(int BitsOfContext, int InputBits, int Rate): Data((1ull<<BitsOfContext)*((1ull<<InputBits)-1)), Context(0), Mask((1<<BitsOfContext)-1), Stride((1<<InputBits)-1), bCount(0), bTotal(InputBits), B(0) {
    assert(InputBits>0 && InputBits<=8);
    assert(BitsOfContext+InputBits<=24);
    Reset(Rate);
    cp=&Data[0];
  }
  void  StationaryMap::set(U32 ctx) {
    Context = (ctx&Mask)*Stride;
    bCount=B=0;
  }
    void  StationaryMap::Reset( int Rate){
    for (U32 i=0; i<Data.size(); ++i)
      Data[i]=(1<<31)|min(1023,Rate);
  }
  int  StationaryMap::mix(Mixer& m, const int Multiplier, const int Divisor, const U16 Limit) {
    // update
    int Prediction,Error ;
    U32 p0=cp[0];
#ifdef SM  
     int n=p0&1023, pr=p0>>13;  // count, prediction
     p0+=(n<Limit);     
     p0+=(((m.x.y<<19)-pr))*dt[n]&0xfffffc00;
#else
     int n=p0&1023, pr=p0>>10;  // count, prediction
     p0+=(n<Limit);     
     p0+=(((m.x.y<<22)-pr)>>3)*dt[n]&0xfffffc00;
#endif    
     cp[0]=p0;

    // predict
    B+=(m.x.y && B>0);
    cp=&Data[Context+B];
    pr = (*cp)>>20;
    m.add((stretch(pr)*Multiplier)/Divisor);
    m.add(((pr-2048)*Multiplier)/(Divisor*2));
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
    return pr;
  }
  int  StationaryMap::mix1(Mixer& m, const int Multiplier, const int Divisor, const U16 Limit) {
    // update
    int Prediction,Error;
    U32 p0=cp[0];
#ifdef SM
     int pr=p0>>13;  // count, prediction
     if (m.x.count>0x4ffff)
       p0+=(((m.x.y<<19)-pr));
     else {
       int n=p0&1023;
       p0+=(n<Limit);     
       p0+=(((m.x.y<<19)-pr))*dt[n]&0xfffffc00;
     }
#else
     int n=p0&1023, pr=p0>>10;  // count, prediction
     p0+=(n<Limit);     
     p0+=(((m.x.y<<22)-pr)>>3)*dt[n]&0xfffffc00;
#endif    
     cp[0]=p0;

    // predict
    B+=(m.x.y && B>0);
    cp=&Data[Context+B];
    pr = (*cp)>>20;
    m.add((stretch(pr)*Multiplier)/Divisor);
    m.add(((pr-2048)*Multiplier)/(Divisor*2));
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
    return pr;
  }

