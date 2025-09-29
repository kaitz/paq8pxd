#include "im4bit.hpp"
extern U8 level;
//////////////////////////// im4bitModel /////////////////////////////////

// Model for 4-bit image data

 im4bitModel1::im4bitModel1( BlockData& bd,U32 val ): x(bd),buf(bd.buf),t( CMlimit((level>14?MEM()/2:MEM())/8) ),S(14),cp(S),map(16), WW(0), W(0), NWW(0), NW(0), N(0), NE(0),
  NEE(0), NNWW(0), NNW(0), NN(0), NNE(0), NNEE(0),col(0), line(0), run(0), prevColor(0), px(0) {
   sm=new StateMap[S];
   for (int i=0;i<S;i++)
      cp[i]=t[263*i]+1;
   }

   
int im4bitModel1::p(Mixer& m,int w,int val2)  {
  int i;
  if (x.blpos==1){//helps only on bigger files+1024kb
      //t.reset();
      for (i=0;i<S;i++)
      cp[i]=t[263*i]+1;
  }
  for (i=0;i<S;i++)
    *cp[i]=nex(*cp[i],x.y);

  if (x.bpos==0 || x.bpos==4){
      WW=W, NWW=NW, NW=N, N=NE, NE=NEE, NNWW=NWW, NNW=NN, NN=NNE, NNE=NNEE;
      if (x.bpos==0)
        {
        W=x.c4&0xF, NEE=buf(w-1)>>4, NNEE=buf(w*2-1)>>4;}
      else
       {
        W=x.c0&0xF, NEE=buf(w-1)&0xF, NNEE=buf(w*2-1)&0xF; }
      run=(W!=WW || col==0)?(prevColor=WW,0):min(0xFFF,run+1);
      px=1, i=0;

      cp[i++]=t[hash(W,NW,N)]+1;
      cp[i++]=t[hash(N, min(0xFFF, col/8))]+1;
      cp[i++]=t[hash(W,NW,N,NN,NE)]+1;
      cp[i++]=t[hash(W, N, NE+NNE*16, NEE+NNEE*16)]+1;
      cp[i++]=t[hash(W, N, NW+NNW*16, NWW+NNWW*16)]+1;
      cp[i++]=t[hash(W, ilog2(run+1), prevColor, col/max(1,w/2) )]+1;
      cp[i++]=t[hash(NE, min(0x3FF, (col+line)/max(1,w*8)))]+1;
      cp[i++]=t[hash(NW, (col-line)/max(1,w*8))]+1;
      cp[i++]=t[hash(WW*16+W,NN*16+N,NNWW*16+NW)]+1;
      cp[i++]=t[hash(i,N,NN)]+1;
      cp[i++]=t[hash(i,W,WW)]+1;
      cp[i++]=t[hash(i,W,NE)]+1;
      cp[i++]=t[hash(i,WW,NN,NEE)]+1;
      cp[i++]=t[-1]+1;
      
      col++;
      if(col==w*2){col=0;line++;}
        x.Image.pixels.W = W;
        x.Image.pixels.N = N;
        x.Image.pixels.NN = NN;
        x.Image.pixels.WW = WW;
        x.Image.ctx = W*16+px;
  }
  else{
    px+=px+x.y;
    int j=(x.y+1)<<(x.bpos&3);
    for (i=0;i<S;i++)
      cp[i]+=j;
  }
  // predict
  for (int i=0; i<S; i++) {
    const U8 s = *cp[i];
    const int n0=-!nex(s, 2), n1=-!nex(s, 3);
    const int p1 = sm[i].p1(s,x.y);
    const int st = stretch(p1)>>1;
    m.add(st);
    m.add((p1-2048)>>3);
    m.add(st*abs(n1-n0));
  }
  m.add(stretch(map.p(px,x.y)));
 
  m.set(W*16+px, 256);
  m.set(min(31,col/max(1,w/16))+N*32, 512);
  m.set((x.bpos&3)+4*W+64*min(7,ilog2(run+1)), 512);
  m.set(W+NE*16+(x.bpos&3)*256, 1024);
  m.set(px, 16);
  m.set(0,1);
  return 0;
}

