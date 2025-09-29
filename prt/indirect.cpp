#include "indirect.hpp"

IndirectMap::IndirectMap(int BitsOfContext, int InputBits,int s,int l): 
  Data((1ull<<BitsOfContext)*((1ull<<InputBits)-1)), mask((1<<BitsOfContext)-1),
   maskbits(BitsOfContext), stride((1<<InputBits)-1), Context(0),
    bCount(0), bTotal(InputBits), B(0),scale(s),Limit(l) {
    assert(InputBits>0 && InputBits<=8);
    assert(BitsOfContext+InputBits<=24);
    cp=&Data[0];
  }
  void IndirectMap::set_direct(const U32 ctx) {
    Context = (ctx&mask)*stride;
    bCount=B=0;
  }
  void IndirectMap::set(const U64 ctx) {
    Context = (finalize64(ctx,maskbits)&mask)*stride;
    bCount=B=0;
  }
  void IndirectMap::mix(Mixer& m, const int Multiplier, const int Divisor, const U16 Limit) {
    // update
    //*cp = nex(*cp, m.x.y);
    int ns=nex(*cp, m.x.y);
      if (ns>=204 && rnd() << ((452-ns)>>3)) ns-=4;  // probabilistic increment
      *cp=ns;
    // predict
    B+=(m.x.y && B>0);
    cp=&Data[Context+B];
    const U8 state = *cp;
    const int p1 = Map.p(state,m.x.y, Limit);
    m.add((stretch(p1)*Multiplier)/Divisor);
    m.add(((p1-2048)*Multiplier)/(Divisor*2));
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
  }
  void IndirectMap::mix1(Mixer& m) {
    // update
    //*cp = nex(*cp, m.x.y);
    int ns=nex(*cp, m.x.y);
      if (ns>=204 && rnd() << ((452-ns)>>3)) ns-=4;  // probabilistic increment
      *cp=ns;
    // predict
    B+=(m.x.y && B>0);
    cp=&Data[Context+B];
    const U8 state = *cp;
    const int p1 = Map.p1(state,m.x.y, Limit);
    m.add((stretch(p1)*scale  )>> 8U);
    m.add(((p1-2048)*scale  )>> 9U);
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
  }
  void IndirectMap::mix2(Mixer& m) {
    // update
    //*cp = nex(*cp, m.x.y);
    int ns=nex(*cp, m.x.y);
      if (ns>=204 && rnd() << ((452-ns)>>3)) ns-=4;  // probabilistic increment
      *cp=ns;
    // predict
    B+=(m.x.y && B>0);
    cp=&Data[Context+B];
    const U8 state = *cp;
    const int p1 = Map.p2(state,m.x.y, Limit);
    m.add((stretch(p1)*scale  )>> 8U);
    m.add(((p1-2048)*scale  )>> 9U);
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
  }

