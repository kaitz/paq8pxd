#include "sparsey.hpp"
//////////////////////////// sparseModel ///////////////////////

// Model order 1-2 contexts with gaps.

  sparseModely::sparseModely(BlockData& bd,U32 val):x(bd),buf(bd.buf), N(44),cm(CMlimit(MEM()*2), N,M_SPARSE_Y),ctx(0) {
  }

  int sparseModely::p(Mixer& m,int seenbefore,int howmany){//match order
  int j=0;
  if (x.bpos==0) {
    //context for 4-byte structures and 
    //positions of zeroes in the last 16 bytes
    ctx <<= 1;
    ctx |= (x.c4 & 0xff) == x.buf(5); //column matches in a 4-byte fixed structure
    ctx <<= 1;
    ctx |= (x.c4 & 0xff) == 0; //zeroes
    cm.set(hash(j++, ctx)); // calgary/obj2, calgary/pic, cantenbury/kennedy.xls, cantenbury/sum, etc.
    //special model for some 4-byte fixed length structures
    cm.set(hash(j++, x.c4 & 0xffe00000 | (ctx&0xff))); 
    cm.set(hash(j++,seenbefore));
    cm.set(hash(j++,howmany==-1?0:howmany));
    cm.set(hash(j++,buf(1)|buf(5)<<8));
    cm.set(hash(j++,buf(1)|buf(6)<<8));
    cm.set(hash(j++,buf(3)|buf(6)<<8));
    cm.set(hash(j++,buf(4)|buf(8)<<8));
    cm.set(hash(j++,buf(1)|buf(3)<<8|buf(5)<<16));
    cm.set(hash(j++,buf(2)|buf(4)<<8|buf(6)<<16));
    if (x.c4==0){
        for (int i=0; i<13; ++i) cm.set(); j++;
    }else{
    cm.set(hash(j++,x.c4&0x00f0f0ff));
    cm.set(hash(j++,x.c4&0x00ff00ff));
    cm.set(hash(j++,x.c4&0xff0000ff));
    cm.set(hash(j++,x.c4&0x0f0f0f0f));
    cm.set(hash(j++,x.c4&0x0000f8f8)); 
    cm.set(hash(j++,x.c4&0x00f8f8f8));
    cm.set(hash(j++,x.c4&0xf8f8f8f8));
    cm.set(hash(j++,x.c4&0x00e0e0e0));
    cm.set(hash(j++,x.c4&0xe0e0e0e0));
    cm.set(hash(j++,x.c4&0x810000c1));
    cm.set(hash(j++,x.c4&0xC3CCC38C));
    cm.set(hash(j++,x.c4&0x0081CC81));
    cm.set(hash(j++,x.c4&0x00c10081));
    }
    for (int i=1; i<8; ++i) {
      cm.set(hash(j++,seenbefore|buf(i)<<8)); 
      cm.set(hash(j++,(buf(i+2)<<8)|buf(i+1)));
      cm.set(hash(j++,(buf(i+3)<<8)|buf(i+1)));
    }
  }
  if (howmany==-1) return 1;
  cm.mix(m);
   m.set((x.blpos & 3)<<8 | (ctx&0xff), 4 * 256);
  return 0;
}
