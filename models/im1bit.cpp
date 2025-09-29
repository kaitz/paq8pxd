#include "im1bit.hpp"
//////////////////////////// im1bitModel /////////////////////////////////
// Model for 1-bit image data

  im1bitModel1::im1bitModel1( BlockData& bd,U32 val): x(bd),buf(bd.buf),r0(0),r1(0),r2(0),r3(0), 
    t(0x23000),N(11), cxt(N),t1(65536/2) {
   sm=new StateMap[N];
   cp=t1[0]+1;
   }
int im1bitModel1::p(Mixer& m,int w,int val2)  {
  // update the model
  int i;
  for (i=0; i<N; i++)
    t[cxt[i]]=nex(t[cxt[i]],x.y);
  //count run
  if (cp[0]==0 || cp[1]!=x.y) cp[0]=1, cp[1]=x.y;
  else if (cp[0]<255) ++cp[0];
  cp=t1[x.c4]+1;
  // update the contexts (pixels surrounding the predicted one)
  r0+=r0+x.y;
  r1+=r1+((x.buf(w-1)>>(7-x.bpos))&1);
  r2+=r2+((x.buf(w+w-1)>>(7-x.bpos))&1);
  r3+=r3+((x.buf(w+w+w-1)>>(7-x.bpos))&1);
  cxt[0]=(r0&0x7)|(r1>>4&0x38)|(r2>>3&0xc0);
  cxt[1]=0x100+   ((r0&1)|(r1>>4&0x3e)|(r2>>2&0x40)|(r3>>1&0x80));
  cxt[2]=0x200+   ((r0&1)|(r1>>4&0x1d)|(r2>>1&0x60)|(r3&0xC0));
  cxt[3]=0x300+   ((r0&1)|((r0<<1)&4)|((r1>>1)&0xF0)|((r2>>3)&0xA));//
  cxt[4]=0x400+   ((r0>>4&0x2AC)|(r1&0xA4)|(r2&0x349)|(!(r3&0x14D)));
  cxt[5]=0x800+   ((r0&1)|((r1>>4)&0xE)|((r2>>1)&0x70)|((r3<<2)&0x380));//
  cxt[6]=0xC00+   (((r1&0x30)^(r3&0x0c0c))|(r0&3));
  cxt[7]=0x1000+  ((!(r0&0x444))|(r1&0xC0C)|(r2&0xAE3)|(r3&0x51C));
  cxt[8]=0x2000+  ((r0&7)|((r1>>1)&0x3F8)|((r2<<5)&0xC00));//
  cxt[9]=0x3000+  ((r0&0x3f)^(r1&0x3ffe)^(r2<<2&0x7f00)^(r3<<5&0xf800));
  cxt[10]=0x13000+((r0&0x3e)^(r1&0x0c0c)^(r2&0xc800));
 
  // predict
  for (i=0; i<N; i++) m.add(stretch(sm[i].p1(t[cxt[i]],x.y)));
  //run
  if (cp[1]==x.y)
      m.add(((cp[1]&1)*2-1)*ilog(cp[0]+1)*8);
  else
      m.add(0);
  m.set((r0&0x7)|(r1>>4&0x38)|(r2>>3&0xc0), 256);
  m.set(((r1&0x30)^(r3&0x0c))|(r0&3),256);
  m.set((r0&1)|(r1>>4&0x3e)|(r2>>2&0x40)|(r3>>1&0x80), 256);
  m.set((r0&0x3e)^((r1>>8)&0x0c)^((r2>>8)&0xc8),256);
  m.set(cp[0],256);
  return 0;
}
