#include "chart.hpp"

///////////////////////////// chartModel ////////////////////////////

  chartModel::chartModel(BlockData& bd,U32 val):x(bd),buf(bd.buf),cm(CMlimit(MEM()*4),30,M_CHART),cn(CMlimit(MEM()),20,M_CHART), 
  chart(32), indirect(2048), indirect2(256), indirect3(0x10000) 
 {   
 }

int chartModel::p(Mixer& m,int val1,int val2){
   if (!x.bpos){
       U32 c4=x.c4;
    const
      U32 w = c4&65535,
      w0 = c4&16777215,
      w1 = c4&255,
      w2 = c4<<8&65280,
      w3 = c4<<8&16711680,
      w4 = c4<<8&4278190080,
      a[3]={c4>>5&460551,    c4>>2&460551,   c4&197379},
      b[3]={c4>>23&448|      c4>>18&56|      c4>>13&7,
            c4>>20&448|      c4>>15&56|      c4>>10&7,
            c4>>18&48 |      c4>>12&12|      c4>>8&3},
      d[3]={c4>>15&448|      c4>>10&56|      c4>>5&7,
            c4>>12&448|      c4>>7&56 |      c4>>2&7,
            c4>>10&48 |      c4>>4&12 |      c4&3},
      c[3]={c4&255,          c4>>8&255,      c4>>16&255};
U32 p=0;
    for (
      int i=0,
      j=0,
      f=b[0],
      e=a[0];

    i<3;
    ++i
      
    ){
         j=(i<<9);
      f=j|b[i];
      
      e=a[i];
      indirect[f&0x7FF]=w1; // <--Update indirect
      const int g =indirect[(j|d[i])&0x7FF];
      chart[(i<<3|e>>16&255)&31]=w0; // <--Fix chart
      chart[(i<<3|e>>8&255)&31]=w<<8; // <--Update chart
      cn.set(U32(e&7|(((e>>8)&7)<<3)|(((e>>16)&7)<<6)));// low 3 bits of last 3 bytes
      //Maps[p++].set(g);
      //cn.set(e); // <--Model previous/current/next slot
      //cn.set(g); // <--Guesses next "c4&0xFF"
      cn.set(U32(w2|g)); // <--Guesses next "c4&0xFFFF"
      cn.set(U32(w3|g)); // <--Guesses next "c4&0xFF00FF"
      cn.set(U32(w4|g)); // <--Guesses next "c4&0xFF0000FF"
      //cm.set(c[i]); // <--Models buf(1,2,3)
    }

    indirect2[buf(2)]=buf(1);
    int g=indirect2[buf(1)];
    cn.set(U32(g)); // <--Guesses next "c4&0xFF"
    cn.set(U32(w2|g)); // <--Guesses next "c4&0xFFFF"
    cn.set(U32(w3|g)); // <--Guesses next "c4&0xFF00FF"
    cn.set(U32(w4|g)); // <--Guesses next "c4&0xFF0000FF"

    indirect3[buf(3)<<8|buf(2)]=buf(1);
    g=indirect3[buf(2)<<8|buf(1)];
    cn.set(U32(g)); // <--Guesses next "c4&0xFF"
    cn.set(U32(w2|g)); // <--Guesses next "c4&0xFFFF"
    cn.set(U32(w3|g)); // <--Guesses next "c4&0xFF00FF"
    cn.set(U32(w4|g)); // <--Guesses next "c4&0xFF0000FF"


    for (
      int
      i=10,
      s=0,
      e=a[0],
      k=chart[0];

    i<20;
    s=++i>>3,
      e=a[s],
      k=chart[i]
    ){ // k e
      cm.set(U32(k<<s)); // 111 000
      cm.set(hash(e,k,s)); // 111 111
      cm.set((hash(e&255,k>>16)^(k&255))<<s); // 101 001
    }
  }
  cn.mix(m);
  cm.mix(m);
  return 0;
}

