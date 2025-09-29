#include "indirect.hpp"
//////////////////////////// indirectModel /////////////////////

// The context is a byte string history that occurs within a
// 1 or 2 byte context.
extern U8 level;
  indirectModel1::indirectModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf),N(15+1+9-13),mem(0x1000000*(level>9?1:1)),

  cm2((mem/4),4,M_INDIRECT),
  cmt((mem/2),3,M_INDIRECT),
    cm0((mem/2),3,M_INDIRECT), 
    cma((mem/4),3,M_INDIRECT), 
    cmc((mem/4),3,M_INDIRECT), 
    cm3((mem/4),3,M_INDIRECT), 
    cm4((mem/4),3,M_INDIRECT), 
    cm5((mem/4),3,M_INDIRECT), 
   t1(256),
   t2(0x10000), t3(0x8000),t4(0x8000),t5(256*256),
   iCtx{16,8 },
   chars4(0){
  }
 
int indirectModel1::p(Mixer& m,int val1,int val2){
  int j=0;
  if (!x.bpos) {
    U32 d=x.c4&0xffff;
    U8 c=d&255;
    U32 d2=(x.buf(1)&31)+   32*(x.buf(2)&31)+   1024*(x.buf(3)&31);
    U32 d3=(x.buf(1)>>3&31)+32*(x.buf(3)>>3&31)+1024*(x.buf(4)>>3&31);
    
    U32& r1=t1[d>>8];
    r1=r1<<8|c;
    U16& r2=t2[x.c4>>8&0xffff];
    r2=r2<<8|c;
    U16& r3=t3[(x.buf(2)&31)+32*(x.buf(3)&31)+1024*(x.buf(4)&31)];
    r3=r3<<8|c;
    U16& r4=t4[(x.buf(2)>>3&31)+32*(x.buf(4)>>3&31)+1024*(x.buf(5)>>3&31)];
    r4=r4<<8|c;
    const U32 t=c|t1[c]<<8;
    const U32 t0=d|t2[d]<<16;
    const U32 ta=d2|t3[d2]<<16;
    const U32 tc=d3|t4[d3]<<16;
    const U8 pc=tolower(U8(x.c4>>8));
    iCtx+=(c=tolower(c)), iCtx=(pc<<8)|c;
    
    const U32 ctx0=iCtx(), mask=(U8(t1[c])==U8(t2[d]))|
                               ((U8(t1[c])==U8(t3[d2]))<<1)|
                               ((U8(t1[c])==U8(t4[d3]))<<2)|
                               ((U8(t1[c])==U8(ctx0))<<3);
    cmt.set(t);
    cmt.set( (t&0xff00)| mask);
    cmt.set(t&0xffff);
    
    cm0.set(t0);
    cm0.set(t0&0xffffff);
    cm0.set((t0<<9)|x.frstchar);
    
    cma.set(ta); 
    cma.set(ta&0xffffff);
    cma.set((ta<<9)|x.frstchar);
    
    cmc.set(tc);
    cmc.set((tc<<9)|x.frstchar);
    cmc.set(buf(1) + ((32 * x.tt) & 0x1FFFFF00));
    
     // context with 2 characters converted to lowercase (context table: t5, byte history: h5)
    U32 c1=c;
    c = tolower(c1);
    chars4 = chars4 << 8 | c;

    const U32 h5 = t5[chars4 & 0xffff];
    U32& r5 = t5[(chars4 >> 8) & 0xffff];
    r5 = r5 << 8 | c;

    cm2.set( (h5 & 0xff) | (c << 8)); //last char + 1 history char
    cm2.set(  h5 & 0xffff); //2 history chars
    cm2.set(  h5 & 0xffffff); //3 history chars
    cm2.set(  h5 ); //4 history chars

    // large contexts (with hashtable)
    //
    U32 context, h6;

    iCtxLarge.set( x.c4 >> 8, c1);
    context = x.c4 & 0xffffff; //24 bits
    h6 = iCtxLarge.get( context);
    cm3.set(( h6 & 0xff)<<8+(context & 0xff));
    cm3.set( ( h6 & 0xff) + (((32 * x.tt) & 0x1FFFFF00)));
    cm3.set( h6<<8+ (context & 0xff)); //without context is also OK (not for news and obj2 however)

    iCtxLarge.set( chars4 >> 8, c1);
    context = chars4 & 0xffffff; //24 bits lowercase
    h6 = iCtxLarge.get( context);
    cm4.set((( h6 & 0xff)<<24)+  (context &0xffff));
    cm4.set((h6 & 0xffff)+ context);
    cm4.set( h6 );

    iCtxLarge.set( buf(5) << 24 | x.c4 >> 8, c1);
    context = x.c4; //32 bits
    h6 = iCtxLarge.get( context);
    cm5.set(( h6 & 0xff)<<16+  (context &0xffff));
    cm5.set(( h6 & 0xffff)<<8+(context &0xff));
    cm5.set(  h6 );
  }
  
  cm2.mix(m);
  cm3.mix(m);
  cm4.mix(m);
  cm5.mix(m);
   cmt.mix(m);
    cm0.mix(m);
    cma.mix(m); 
    cmc.mix(m); 
  return 0;
}
