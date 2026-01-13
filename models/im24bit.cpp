#include "im24bit.hpp"

//////////////////////////// im24bitModel /////////////////////////////////
// Model for 24-bit image data

  im24bitModel1::im24bitModel1(BlockData& bd): nOLS(6),inpts(47),cm(CMlimit(MEM()*4), inpts,M_IM24,
  CM_RUN1+
  CM_RUN0+
  CM_MAIN1+
  CM_MAIN2+
  CM_MAIN3+
  CM_MAIN4+
  CM_M12
  ), col(0) ,color(-1),stride(3), padding(0), x(0),xx(bd),
   buf(bd.buf), buffer(0x100000),WWW(0), WW(0), W(0),NWW(0),NW(0) ,N(0), NE(0), NEE(0), NNWW(0), NNW(0),
   NN(0), NNE(0), NNEE(0), NNN(0), px(0),filter(0),  w(0), line(0), isPNG(0),R1(0), R2(0),filterOn(false),
   c4(bd.c4),c0(bd.c0),bpos(bd.bpos),lastWasPNG(0), WWp1(0), Wp1(0), p1(0), NWp1(0),
   Np1(0), NEp1(0), NNp1(0),p2(0),lastw(0),lastpos(0),curpos(0), MapCtxs(n2Maps1), SCMapCtxs(nSCMaps-1), pOLS(nOLS),mixCxt(13){
  
    columns[0] = 1, columns[1]=1;
    column[0]=0,column[1]=0;
    ctx[0]=0,ctx[1]=0;
    }
   
  
  int im24bitModel1::p(Mixer& m,int info,int val2){
  if (!bpos) {
    if (xx.blpos==1  ){
      const int alpha=xx.filetype==IMAGE32?1:xx.filetype==IMPNG32?1:0;
      stride = 3+alpha;
      lastpos=curpos;
      curpos=buf.pos;
      lastw=w;
      w = info&0xFFFFFF;
      
      isPNG =(xx.filetype==IMPNG24?1:xx.filetype==IMPNG32?1:0);
      padding = w%stride;
      
      x =1; color = line =px =0;
       filterOn = false;
      columns[0] = max(1,w/max(1,ilog2(w)*3));
      columns[1] = max(1,columns[0]/max(1,ilog2(columns[0])));
      if ( lastWasPNG!=isPNG){
        for (int i=0;i<n2Maps;i++)
          Map[i].Reset();
          xx.count=0;
      }
      lastWasPNG = isPNG;
      buffer.Fill(0x7F);
    }
    else{
      x++;
      if(x>=w+isPNG){x=0;line++;}
    }

    if (x==1 && isPNG)
      filter = (U8)c4;
    else{
      
           if (x+padding<w) {
        color++;
        if (color>=stride) color=0;
      }
      else {
        if (padding>0) color=stride;
        else color=0;
      }
      if (isPNG){
          U8 B = (U8)c4;
        switch (filter){
          case 1: {
            buffer.Add((U8)( B + buffer(stride)*(x>stride+1 || !x) ) );
            filterOn = x>stride;
            px = buffer(stride);
            break;
          }
          case 2: {
            buffer.Add((U8)( B + buffer(w)*(filterOn=(line>0)) ) );
            px = buffer(w);
            break;
          }
          case 3: {
            buffer.Add((U8)( B + (buffer(w)*(line>0) + buffer(stride)*(x>stride+1 || !x))/2 ) );
            filterOn = (x>stride || line>0);
            px = (buffer(stride)*(x>stride)+buffer(w)*(line>0))/2;
            break;
          }
          case 4: {
            buffer.Add((U8)( B + Paeth(buffer(stride)*(x>stride+1 || !x), buffer(w)*(line>0), buffer(w+stride)*(line>0 && (x>stride+1 || !x))) ) );
            filterOn = (x>stride || line>0);
            px = Paeth(buffer(stride)*(x>stride),buffer(w)*(line>0),buffer(w+stride)*(x>stride && line>0));
            break;
          }
          default: buffer.Add(B);
            filterOn = false;
            px = 0;
        }
         if(!filterOn)px=0;
      }
      else
        buffer.Add(c4 & 0xFF);
    }

    if (x || !isPNG){
      int i=color<<5;
      column[0]=(x-isPNG)/columns[0];
      column[1]=(x-isPNG)/columns[1];
       WWWWWW=buffer(6*stride), WWWWW=buffer(5*stride), WWWW=buffer(4*stride), WWW=buffer(3*stride), WW=buffer(2*stride), W=buffer(stride);
      NWWWW=buffer(w+4*stride), NWWW=buffer(w+3*stride), NWW=buffer(w+2*stride), NW=buffer(w+stride), N=buffer(w), NE=buffer(w-stride), NEE=buffer(w-2*stride), NEEE=buffer(w-3*stride), NEEEE=buffer(w-4*stride);
      NNWWW=buffer(w*2+stride*3), NNWW=buffer((w+stride)*2), NNW=buffer(w*2+stride), NN=buffer(w*2), NNE=buffer(w*2-stride), NNEE=buffer((w-stride)*2), NNEEE=buffer(w*2-stride*3);
      NNNWW=buffer(w*3+stride*2), NNNW=buffer(w*3+stride), NNN=buffer(w*3), NNNE=buffer(w*3-stride), NNNEE=buffer(w*3-stride*2);
      NNNNW=buffer(w*4+stride), NNNN=buffer(w*4), NNNNE=buffer(w*4-stride);
      NNNNN=buffer(w*5);
      NNNNNN=buffer(w*6);
      WWp1=buffer(stride*2+1), Wp1=buffer(stride+1), p1=buffer(1), NWp1=buffer(w+stride+1), Np1=buffer(w+1), NEp1=buffer(w-stride+1), NNp1=buffer(w*2+1);
      WWp2=buffer(stride*2+2), Wp2=buffer(stride+2), p2=buffer(2), NWp2=buffer(w+stride+2), Np2=buffer(w+2), NEp2=buffer(w-stride+2), NNp2=buffer(w*2+2);
       
      int j = 0;
      MapCtxs[j++] = Clamp4(N+p1-Np1, W, NW, N, NE);
      MapCtxs[j++] = Clamp4(N+p2-Np2, W, NW, N, NE);
      MapCtxs[j++] = (W+Clamp4(NE*3-NNE*3+NNNE, W, N, NE, NEE))/2;
      MapCtxs[j++] = Clamp4((W+Clip(NE*2-NNE))/2, W, NW, N, NE);
      MapCtxs[j++] = (W+NEE)/2;
      MapCtxs[j++] = Clip((WWW-4*WW+6*W+Clip(NE*4-NNE*6+NNNE*4-NNNNE))/4);
      MapCtxs[j++] = Clip((-WWWW+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+NNNE*4-NNNNE, N, NE, NEE, NEEE))/5);
      MapCtxs[j++] = Clip((-4*WW+15*W+10*Clip(NE*3-NNE*3+NNNE)-Clip(NEEE*3-NNEEE*3+buffer(w*3-3*stride)))/20);
      MapCtxs[j++] = Clip((-3*WW+8*W+Clamp4(NEE*3-NNEE*3+NNNEE, NE, NEE, NEEE, NEEEE))/6);
      MapCtxs[j++] = Clip((W+Clip(NE*2-NNE))/2+p1-(Wp1+Clip(NEp1*2-buffer(w*2-stride+1)))/2);
      MapCtxs[j++] = Clip((W+Clip(NE*2-NNE))/2+p2-(Wp2+Clip(NEp2*2-buffer(w*2-stride+2)))/2);
      MapCtxs[j++] = Clip((-3*WW+8*W+Clip(NEE*2-NNEE))/6+p1-(-3*WWp1+8*Wp1+Clip(buffer(w-stride*2+1)*2-buffer(w*2-stride*2+1)))/6);
      MapCtxs[j++] = Clip((-3*WW+8*W+Clip(NEE*2-NNEE))/6+p2-(-3*WWp2+8*Wp2+Clip(buffer(w-stride*2+2)*2-buffer(w*2-stride*2+2)))/6);
      MapCtxs[j++] = Clip((W+NEE)/2+p1-(Wp1+buffer(w-stride*2+1))/2);
      MapCtxs[j++] = Clip((W+NEE)/2+p2-(Wp2+buffer(w-stride*2+2))/2);
      MapCtxs[j++] = Clip((WW+Clip(NEE*2-NNEE))/2+p1-(WWp1+Clip(buffer(w-stride*2+1)*2-buffer(w*2-stride*2+1)))/2);
      MapCtxs[j++] = Clip((WW+Clip(NEE*2-NNEE))/2+p2-(WWp2+Clip(buffer(w-stride*2+2)*2-buffer(w*2-stride*2+2)))/2);
      MapCtxs[j++] = Clip(WW+NEE-N+p1-Clip(WWp1+buffer(w-stride*2+1)-Np1));
      MapCtxs[j++] = Clip(WW+NEE-N+p2-Clip(WWp2+buffer(w-stride*2+2)-Np2));
      MapCtxs[j++] = Clip(W+N-NW);
      MapCtxs[j++] = Clip(W+N-NW+p1-Clip(Wp1+Np1-NWp1));
      MapCtxs[j++] = Clip(W+N-NW+p2-Clip(Wp2+Np2-NWp2));
      MapCtxs[j++] = Clip(W+NE-N);
      MapCtxs[j++] = Clip(N+NW-NNW);
      MapCtxs[j++] = Clip(N+NW-NNW+p1-Clip(Np1+NWp1-buffer(w*2+stride+1)));
      MapCtxs[j++] = Clip(N+NW-NNW+p2-Clip(Np2+NWp2-buffer(w*2+stride+2)));
      MapCtxs[j++] = Clip(N+NE-NNE);
      MapCtxs[j++] = Clip(N+NE-NNE+p1-Clip(Np1+NEp1-buffer(w*2-stride+1)));
      MapCtxs[j++] = Clip(N+NE-NNE+p2-Clip(Np2+NEp2-buffer(w*2-stride+2)));
      MapCtxs[j++] = Clip(N+NN-NNN);
      MapCtxs[j++] = Clip(N+NN-NNN+p1-Clip(Np1+NNp1-buffer(w*3+1)));
      MapCtxs[j++] = Clip(N+NN-NNN+p2-Clip(Np2+NNp2-buffer(w*3+2)));
      MapCtxs[j++] = Clip(W+WW-WWW);
      MapCtxs[j++] = Clip(W+WW-WWW+p1-Clip(Wp1+WWp1-buffer(stride*3+1)));
      MapCtxs[j++] = Clip(W+WW-WWW+p2-Clip(Wp2+WWp2-buffer(stride*3+2)));
      MapCtxs[j++] = Clip(W+NEE-NE);
      MapCtxs[j++] = Clip(W+NEE-NE+p1-Clip(Wp1+buffer(w-stride*2+1)-NEp1));
      MapCtxs[j++] = Clip(W+NEE-NE+p2-Clip(Wp2+buffer(w-stride*2+2)-NEp2));
      MapCtxs[j++] = Clip(NN+p1-NNp1);
      MapCtxs[j++] = Clip(NN+p2-NNp2);
      MapCtxs[j++] = Clip(NN+W-NNW);
      MapCtxs[j++] = Clip(NN+W-NNW+p1-Clip(NNp1+Wp1-buffer(w*2+stride+1)));
      MapCtxs[j++] = Clip(NN+W-NNW+p2-Clip(NNp2+Wp2-buffer(w*2+stride+2)));
      MapCtxs[j++] = Clip(NN+NW-NNNW);
      MapCtxs[j++] = Clip(NN+NW-NNNW+p1-Clip(NNp1+NWp1-buffer(w*3+stride+1)));
      MapCtxs[j++] = Clip(NN+NW-NNNW+p2-Clip(NNp2+NWp2-buffer(w*3+stride+2)));
      MapCtxs[j++] = Clip(NN+NE-NNNE);
      MapCtxs[j++] = Clip(NN+NE-NNNE+p1-Clip(NNp1+NEp1-buffer(w*3-stride+1)));
      MapCtxs[j++] = Clip(NN+NE-NNNE+p2-Clip(NNp2+NEp2-buffer(w*3-stride+2)));
      MapCtxs[j++] = Clip(NN+NNNN-NNNNNN);
      MapCtxs[j++] = Clip(NN+NNNN-NNNNNN+p1-Clip(NNp1+buffer(w*4+1)-buffer(w*6+1)));
      MapCtxs[j++] = Clip(NN+NNNN-NNNNNN+p2-Clip(NNp2+buffer(w*4+2)-buffer(w*6+2)));
      MapCtxs[j++] = Clip(WW+p1-WWp1);
      MapCtxs[j++] = Clip(WW+p2-WWp2);
      MapCtxs[j++] = Clip(WW+WWWW-WWWWWW);
      MapCtxs[j++] = Clip(WW+WWWW-WWWWWW+p1-Clip(WWp1+buffer(stride*4+1)-buffer(stride*6+1)));
      MapCtxs[j++] = Clip(WW+WWWW-WWWWWW+p2-Clip(WWp2+buffer(stride*4+2)-buffer(stride*6+2)));
      MapCtxs[j++] = Clip(N*2-NN+p1-Clip(Np1*2-NNp1));
      MapCtxs[j++] = Clip(N*2-NN+p2-Clip(Np2*2-NNp2));
      MapCtxs[j++] = Clip(W*2-WW+p1-Clip(Wp1*2-WWp1));
      MapCtxs[j++] = Clip(W*2-WW+p2-Clip(Wp2*2-WWp2));
      MapCtxs[j++] = Clip(N*3-NN*3+NNN);
      MapCtxs[j++] = Clamp4(N*3-NN*3+NNN, W, NW, N, NE);
      MapCtxs[j++] = Clamp4(W*3-WW*3+WWW, W, NW, N, NE);
      MapCtxs[j++] = Clamp4(N*2-NN, W, NW, N, NE);
      MapCtxs[j++] = Clip((NNNNN-6*NNNN+15*NNN-20*NN+15*N+Clamp4(W*4-NWW*6+NNWWW*4-buffer(w*3+4*stride), W, NW, N, NN))/6);
      MapCtxs[j++] = Clip((buffer(w*3-3*stride)-4*NNEE+6*NE+Clip(W*4-NW*6+NNW*4-NNNW))/4);
      MapCtxs[j++] = Clip(((N+3*NW)/4)*3-((NNW+NNWW)/2)*3+(NNNWW*3+buffer(w*3+3*stride))/4);
      MapCtxs[j++] = Clip((W*2+NW)-(WW+2*NWW)+NWWW);
      MapCtxs[j++] = (Clip(W*2-NW)+Clip(W*2-NWW)+N+NE)/4;
      MapCtxs[j++] = NNNNNN;
      MapCtxs[j++] = (NEEEE+buffer(w-6*stride))/2;
      MapCtxs[j++] = (WWWWWW+WWWW)/2;
      MapCtxs[j++] = ((W+N)*3-NW*2)/4;
      MapCtxs[j++] = N;
      MapCtxs[j++] = NN;
      j = 0;
      SCMapCtxs[j++] = N+p1-Np1;
      SCMapCtxs[j++] = N+p2-Np2;
      SCMapCtxs[j++] = W+p1-Wp1;
      SCMapCtxs[j++] = W+p2-Wp2;
      SCMapCtxs[j++] = NW+p1-NWp1;
      SCMapCtxs[j++] = NW+p2-NWp2;
      SCMapCtxs[j++] = NE+p1-NEp1;
      SCMapCtxs[j++] = NE+p2-NEp2;
      SCMapCtxs[j++] = NN+p1-NNp1;
      SCMapCtxs[j++] = NN+p2-NNp2;
      SCMapCtxs[j++] = WW+p1-WWp1;
      SCMapCtxs[j++] = WW+p2-WWp2;
      SCMapCtxs[j++] = W+N-NW;
      SCMapCtxs[j++] = W+N-NW+p1-Wp1-Np1+NWp1;
      SCMapCtxs[j++] = W+N-NW+p2-Wp2-Np2+NWp2;
      SCMapCtxs[j++] = W+NE-N;
      SCMapCtxs[j++] = W+NE-N+p1-Wp1-NEp1+Np1;
      SCMapCtxs[j++] = W+NE-N+p2-Wp2-NEp2+Np2;
      SCMapCtxs[j++] = W+NEE-NE;
      SCMapCtxs[j++] = W+NEE-NE+p1-Wp1-buffer(w-stride*2+1)+NEp1;
      SCMapCtxs[j++] = W+NEE-NE+p2-Wp2-buffer(w-stride*2+2)+NEp2;
      SCMapCtxs[j++] = N+NN-NNN;
      SCMapCtxs[j++] = N+NN-NNN+p1-Np1-NNp1+buffer(w*3+1);
      SCMapCtxs[j++] = N+NN-NNN+p2-Np2-NNp2+buffer(w*3+2);
      SCMapCtxs[j++] = N+NE-NNE;
      SCMapCtxs[j++] = N+NE-NNE+p1-Np1-NEp1+buffer(w*2-stride+1);
      SCMapCtxs[j++] = N+NE-NNE+p2-Np2-NEp2+buffer(w*2-stride+2);
      SCMapCtxs[j++] = N+NW-NNW;
      SCMapCtxs[j++] = N+NW-NNW+p1-Np1-NWp1+buffer(w*2+stride+1);
      SCMapCtxs[j++] = N+NW-NNW+p2-Np2-NWp2+buffer(w*2+stride+2);
      SCMapCtxs[j++] = NE+NW-NN;
      SCMapCtxs[j++] = NE+NW-NN+p1-NEp1-NWp1+NNp1;
      SCMapCtxs[j++] = NE+NW-NN+p2-NEp2-NWp2+NNp2;
      SCMapCtxs[j++] = NW+W-NWW;
      SCMapCtxs[j++] = NW+W-NWW+p1-NWp1-Wp1+buffer(w+stride*2+1);
      SCMapCtxs[j++] = NW+W-NWW+p2-NWp2-Wp2+buffer(w+stride*2+2);
      SCMapCtxs[j++] = W*2-WW;
      SCMapCtxs[j++] = W*2-WW+p1-Wp1*2+WWp1;
      SCMapCtxs[j++] = W*2-WW+p2-Wp2*2+WWp2;
      SCMapCtxs[j++] = N*2-NN;
      SCMapCtxs[j++] = N*2-NN+p1-Np1*2+NNp1;
      SCMapCtxs[j++] = N*2-NN+p2-Np2*2+NNp2;
      SCMapCtxs[j++] = NW*2-NNWW;
      SCMapCtxs[j++] = NW*2-NNWW+p1-NWp1*2+buffer(w*2+stride*2+1);
      SCMapCtxs[j++] = NW*2-NNWW+p2-NWp2*2+buffer(w*2+stride*2+2);
      SCMapCtxs[j++] = NE*2-NNEE;
      SCMapCtxs[j++] = NE*2-NNEE+p1-NEp1*2+buffer(w*2-stride*2+1);
      SCMapCtxs[j++] = NE*2-NNEE+p2-NEp2*2+buffer(w*2-stride*2+2);
      SCMapCtxs[j++] = N*3-NN*3+NNN+p1-Np1*3+NNp1*3-buffer(w*3+1);
      SCMapCtxs[j++] = N*3-NN*3+NNN+p2-Np2*3+NNp2*3-buffer(w*3+2);
      SCMapCtxs[j++] = N*3-NN*3+NNN;
      SCMapCtxs[j++] = (W+NE*2-NNE)/2;
      SCMapCtxs[j++] = (W+NE*3-NNE*3+NNNE)/2;
      SCMapCtxs[j++] = (W+NE*2-NNE)/2+p1-(Wp1+NEp1*2-buffer(w*2-stride+1))/2;
      SCMapCtxs[j++] = (W+NE*2-NNE)/2+p2-(Wp2+NEp2*2-buffer(w*2-stride+2))/2;
      SCMapCtxs[j++] = NNE+NE-NNNE;
      SCMapCtxs[j++] = NNE+W-NN;
      SCMapCtxs[j++] = NNW+W-NNWW;
      j = 0;
      
      for (int k=(color>0)?color-1:stride-1; j<nOLS; j++) {
//          printf("k %d, color %d j %d \n",k,color,j);
        pOLS[j] = Clip(floor(ols[j][color].Predict(ols_ctxs[j])));
        ols[j][k].Update(p1);
      }
      //if (val2==1) {if (++col>=stride*8) col=0;return 1;
      //}
      if (!isPNG){
         
        int mean=W+NW+N+NE;
        const int var=(W*W+NW*NW+N*N+NE*NE-mean*mean/4)>>2;
        mean>>=2;
        const int logvar=ilog(var);

        cm.set(hash(++i,(N+1)>>1, LogMeanDiffQt(N,Clip(NN*2-NNN))));
        cm.set(hash(++i,(W+1)>>1, LogMeanDiffQt(W,Clip(WW*2-WWW))));
        cm.set(hash(++i,Clamp4(W+N-NW,W,NW,N,NE), LogMeanDiffQt(Clip(N+NE-NNE), Clip(N+NW-NNW))));
        cm.set(hash(++i,(NNN+N+4)/8, Clip(N*3-NN*3+NNN)>>1 ));
        cm.set(hash(++i,(WWW+W+4)/8, Clip(W*3-WW*3+WWW)>>1 ));
        cm.set(hash(++i,color, (W+Clip(NE*3-NNE*3+NNNE))/4, LogMeanDiffQt(N,(NW+NE)/2)));
        cm.set(hash(++i,color, Clip((-WWWW+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+NNNE*4-NNNNE,N,NE,NEE, NEEE))/5)/4));
        cm.set(hash(++i,Clip(NEE+N-NNEE), LogMeanDiffQt(W,Clip(NW+NE-NNE))));
        cm.set(hash(++i,Clip(NN+W-NNW), LogMeanDiffQt(W,Clip(NNW+WW-NNWW))));
        cm.set(hash(++i,color, p1));
        cm.set(hash(++i,color, p2));
        cm.set(hash(++i,color, Clip(W+N-NW)/2, Clip(W+p1-Wp1)/2));
        cm.set(hash(++i,Clip(N*2-NN)/2, LogMeanDiffQt(N,Clip(NN*2-NNN))));
        cm.set(hash(++i,Clip(W*2-WW)/2, LogMeanDiffQt(W,Clip(WW*2-WWW))));
        cm.set(hash(++i,        Clamp4(N*3-NN*3+NNN, W, NW, N, NE)/2));
        cm.set(hash(++i,        Clamp4(W*3-WW*3+WWW, W, N, NE, NEE)/2));
        cm.set(hash(++i, color, LogMeanDiffQt(W,Wp1), Clamp4((p1*W)/(Wp1<1?1:Wp1),W,N,NE,NEE))); //using max(1,Wp1) results in division by zero in VC2015
        cm.set(hash(++i, color, Clamp4(N+p2-Np2,W,NW,N,NE)));
        cm.set(hash(++i, color, Clip(W+N-NW), column[0]));
        cm.set(hash(++i, color, Clip(N*2-NN), LogMeanDiffQt(W,Clip(NW*2-NNW))));
        cm.set(hash(++i, color, Clip(W*2-WW), LogMeanDiffQt(N,Clip(NW*2-NWW))));
        cm.set(hash(++i, (W+NEE)/2, LogMeanDiffQt(W,(WW+NE)/2) ));
        cm.set(hash(++i,        (Clamp4(Clip(W*2-WW)+Clip(N*2-NN)-Clip(NW*2-NNWW), W, NW, N, NE))));
        cm.set(hash(++i, color, W, p2 ));
        cm.set(hash(++i, N, NN&0x1F, NNN&0x1F ));
        cm.set(hash(++i, W, WW&0x1F, WWW&0x1F ));
        cm.set(hash(++i, color, N, column[0] ));
        cm.set(hash(++i, color, Clip(W+NEE-NE), LogMeanDiffQt(W,Clip(WW+NE-N))));
        cm.set(hash(++i,NN, NNNN&0x1F, NNNNNN&0x1F, column[1]));
        cm.set(hash(++i,WW, WWWW&0x1F, WWWWWW&0x1F, column[1]));
        cm.set(hash(++i,NNN, NNNNNN&0x1F, buffer(w*9)&0x1F, column[1]));
        cm.set(hash(++i,  color,column[1]));
        
        cm.set(hash(++i, color, W, LogMeanDiffQt(W,WW)));
        cm.set(hash(++i, color, W, p1));
        cm.set(hash(++i, color, W/4, LogMeanDiffQt(W,p1), LogMeanDiffQt(W,p2) ));
        cm.set(hash(++i, color, N, LogMeanDiffQt(N,NN)));
        cm.set(hash(++i, color, N, p1));
        cm.set(hash(++i, color, N/4, LogMeanDiffQt(N,p1), LogMeanDiffQt(N,p2) ));
        cm.set(hash(++i, color, (W+N)>>3, p1>>4, p2>>4));
        cm.set(hash(++i, color, p1/2, p2/2));
        cm.set(hash(++i, color, W, p1-Wp1));
        cm.set(hash(++i, color, W+p1-Wp1));
        cm.set(hash(++i, color, N, p1-Np1));
        cm.set(hash(++i, color, N+p1-Np1));
        cm.set(hash(++i, color, NNNE, NNNEE)); //buf(w*3-stride),buf(w*3-stride*2)
        cm.set(hash(++i, color, NNNW, NNNWW ));//buf(w*3+stride), buf(w*3+stride*2)

        cm.set(hash(++i, mean, logvar>>4));

        ctx[0] = (min(color,stride-1)<<9)|((abs(W-N)>3)<<8)|((W>N)<<7)|((W>NW)<<6)|((abs(N-NW)>3)<<5)|((N>NW)<<4)|((abs(N-NE)>3)<<3)|((N>NE)<<2)|((W>WW)<<1)|(N>NN);
        ctx[1] = ((LogMeanDiffQt(p1,Clip(Np1+NEp1-buffer(w*2-stride+1)))>>1)<<5)|((LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))>>1)<<2)|min(color,stride-1);
      }
      else{
        i|=(filterOn)?((0x100|filter)<<8):0;
        int residuals[5] = { ((int8_t)buf(stride+(x<=stride)))+128,
                             ((int8_t)buf(1+(x<2)))+128,
                             ((int8_t)buf(stride+1+(x<=stride)))+128,
                             ((int8_t)buf(2+(x<3)))+128,
                             ((int8_t)buf(stride+2+(x<=stride)))+128
                           };
        R1 = (residuals[1]*residuals[0])/max(1,residuals[2]);
        R2 = (residuals[3]*residuals[0])/max(1,residuals[4]);
       
        cm.set(hash(++i, Clip(W+N-NW)-px, Clip(W+p1-Wp1)-px, R1));
        cm.set(hash(++i, Clip(W+N-NW)-px, LogMeanDiffQt(p1, Clip(Wp1+Np1-NWp1))));
        cm.set(hash(++i, Clip(W+N-NW)-px, LogMeanDiffQt(p2, Clip(Wp2+Np2-NWp2)), R2/4));
        cm.set(hash(++i, Clip(W+N-NW)-px, Clip(N+NE-NNE)-Clip(N+NW-NNW)));
        cm.set(hash(++i, Clip(W+N-NW+p1-(Wp1+Np1-NWp1)), px, R1));
        cm.set(hash(++i, Clamp4(W+N-NW, W, NW, N, NE)-px, column[0]));
        cm.set(hash(i>>8, Clamp4(W+N-NW, W, NW, N, NE)/8, px));
        cm.set(hash(++i, N-px, Clip(N+p1-Np1)-px));
        cm.set(hash(++i, Clip(W+p1-Wp1)-px, R1));
        cm.set(hash(++i, Clip(N+p1-Np1)-px));
        cm.set(hash(++i, Clip(N+p1-Np1)-px, Clip(N+p2-Np2)-px));
        cm.set(hash(++i, Clip(W+p1-Wp1)-px, Clip(W+p2-Wp2)-px));
        cm.set(hash(++i, Clip(NW+p1-NWp1)-px));
        cm.set(hash(++i, Clip(NW+p1-NWp1)-px, column[0]));
        cm.set(hash(++i, Clip(NE+p1-NEp1)-px, column[0]));
        cm.set(hash(++i, Clip(NE+N-NNE)-px, Clip(NE+p1-NEp1)-px));
        cm.set(hash(i>>8, Clip(N+NE-NNE)-px, column[0]));
        cm.set(hash(++i, Clip(NW+N-NNW)-px, Clip(NW+p1-NWp1)-px));
        cm.set(hash(i>>8, Clip(N+NW-NNW)-px, column[0]));
        cm.set(hash(i>>8, Clip(NN+W-NNW)-px, LogMeanDiffQt(N, Clip(NNN+NW-NNNW))));
        cm.set(hash(i>>8, Clip(W+NEE-NE)-px, LogMeanDiffQt(W, Clip(WW+NE-N))));
        cm.set(hash(++i, Clip(N+NN-NNN+buffer(1+(!color))-Clip(buffer(w+1+(!color))+buffer(w*2+1+(!color))-buffer(w*3+1+(!color))))-px));
        cm.set(hash(i>>8, Clip(N+NN-NNN)-px, Clip(5*N-10*NN+10*NNN-5*NNNN+NNNNN)-px));
        cm.set(hash(++i, Clip(N*2-NN)-px, LogMeanDiffQt(N, Clip(NN*2-NNN))));
        cm.set(hash(++i, Clip(W*2-WW)-px, LogMeanDiffQt(W, Clip(WW*2-WWW))));
        cm.set(hash(i>>8, Clip(N*3-NN*3+NNN)-px));
        cm.set(hash(++i, Clip(N*3-NN*3+NNN)-px, LogMeanDiffQt(W, Clip(NW*2-NNW))));
        cm.set(hash(i>>8, Clip(W*3-WW*3+WWW)-px));
        cm.set(hash(++i, Clip(W*3-WW*3+WWW)-px, LogMeanDiffQt(N, Clip(NW*2-NWW))));
        cm.set(hash(i>>8, Clip((35*N-35*NNN+21*NNNNN-5*buffer(w*7))/16)-px));
        cm.set(hash(++i, (W+Clip(NE*3-NNE*3+NNNE))/2-px, R2));
        cm.set(hash(++i, (W+Clamp4(NE*3-NNE*3+NNNE, W, N, NE, NEE))/2-px, LogMeanDiffQt(N, (NW+NE)/2)));
        cm.set(hash(++i, (W+NEE)/2-px, R1/2));
        cm.set(hash(++i, Clamp4(Clip(W*2-WW)+Clip(N*2-NN)-Clip(NW*2-NNWW), W, NW, N, NE)-px));
        cm.set(hash(++i, buf(stride+(x<=stride)), buf(1+(x<2)), buf(2+(x<3))));
        cm.set(hash(++i, buf(1+(x<2)), px));
        cm.set(hash(i>>8, buf(w+1), buf((w+1)*2), buf((w+1)*3), px));
                                                   
        cm.set(U32(~0x5ca1ab1e));
        for (int j=0;j<9;j++)cm.set();

        ctx[0] = (min(color,stride-1)<<9)|((abs(W-N)>3)<<8)|((W>N)<<7)|((W>NW)<<6)|((abs(N-NW)>3)<<5)|((N>NW)<<4)|((N>NE)<<3)|min(5, filterOn?filter+1:0);
        ctx[1] = ((LogMeanDiffQt(p1,Clip(Np1+NEp1-buffer(w*2-stride+1)))>>1)<<5)|((LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))>>1)<<2)|min(color,stride-1);
      }
       i=0;
      Map[i++].set((W&0xC0)|((N&0xC0)>>2)|((WW&0xC0)>>4)|(NN>>6));
      Map[i++].set((N&0xC0)|((NN&0xC0)>>2)|((NE&0xC0)>>4)|(NEE>>6));
      Map[i++].set(buf(1+(isPNG && x<2)));
      Map[i++].set(min(color, stride-1));
      
        xx.Image.plane =  min(color, stride-1);
        xx.Image.pixels.W = W;
        xx.Image.pixels.N = N;
        xx.Image.pixels.NN = NN;
        xx.Image.pixels.WW = WW;
        xx.Image.pixels.Wp1 = Wp1;
        xx.Image.pixels.Np1 = Np1;
        xx.Image.ctx = ctx[0]>>3;
      
    }
  }
  if (x>0 || !isPNG) {
    U8 B=(c0<<(8-bpos));
    int i=5;

    Map[i++].set((((U8)(Clip(W+N-NW)-px-B))*8+bpos)|(LogMeanDiffQt(Clip(N+NE-NNE), Clip(N+NW-NNW))<<11));
    Map[i++].set((((U8)(Clip(N*2-NN)-px-B))*8+bpos)|(LogMeanDiffQt(W, Clip(NW*2-NNW))<<11));
    Map[i++].set((((U8)(Clip(W*2-WW)-px-B))*8+bpos)|(LogMeanDiffQt(N, Clip(NW*2-NWW))<<11));
    Map[i++].set((((U8)(Clip(W+N-NW)-px-B))*8+bpos)|(LogMeanDiffQt(p1, Clip(Wp1+Np1-NWp1))<<11));
    Map[i++].set((((U8)(Clip(W+N-NW)-px-B))*8+bpos)|(LogMeanDiffQt(p2, Clip(Wp2+Np2-NWp2))<<11));
    Map[i++].set(hash(W-px-B, N-px-B)*8+bpos);
    Map[i++].set(hash(W-px-B, WW-px-B)*8+bpos);
    Map[i++].set(hash(N-px-B, NN-px-B)*8+bpos);
    Map[i++].set(hash(Clip(N+NE-NNE)-px-B, Clip(N+NW-NNW)-px-B)*8+bpos);
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(N+p1-Np1)-px-B))*8+bpos));
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(N+p2-Np2)-px-B))*8+bpos));
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(W+p1-Wp1)-px-B))*8+bpos));
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(W+p2-Wp2)-px-B))*8+bpos));
    for (int j=0; j<n2Maps1; i++, j++)
      Map[i].set((MapCtxs[j]-px-B)*8+bpos);
    for (int j=0; i<n2Maps; i++, j++)
      Map[i].set((pOLS[j]-px-B)*8+bpos);
    for (int i=0; i<nSCMaps-1; i++)
      SCMap[i].set((SCMapCtxs[i]-px-B)*8+bpos);
  }

  // Predict next bit
  if (x || !isPNG){
  if (++col>=stride*8) col=0;
      if (val2==1) return 1; 
      int  cnx=m.nx;
      cm.mix(m);
      int count=(m.nx-cnx)/inpts;
      m.nx=cnx;
      for (int i=count*inpts;i!=0;--i) m.sp(3); // decrease all

    for (int i=0;i<n2Maps;i++)
      Map[i].mix1(m);
    for (int i=0;i<nSCMaps;i++)
      SCMap[i].mix(m,9,1,3);

    m.add(0);
    if (bpos==0){
        int i=0;
        mixCxt[i++]=(((line&7)<<5));
        mixCxt[i++]=(min(63,column[0])+((ctx[0]>>3)&0xC0));
        mixCxt[i++]=(min(127,column[1])+((ctx[0]>>2)&0x180));
        mixCxt[i++]=((ctx[0]&0x7FC));//
        mixCxt[i++]=0;//(col+(isPNG?(ctx[0]&7)+1:(c0==((0x100|((N+W)/2))>>(8-bpos))))*32);
        mixCxt[i++]=(((isPNG?p1:0)>>4)*stride+(x%stride) + min(5,filterOn?filter+1:0)*64);
        mixCxt[i++]=( 256*(isPNG && abs(R1-128)>8));//
        mixCxt[i++]=((ctx[1]<<2));//
        mixCxt[i++]=(hash(LogMeanDiffQt(W,WW,5), LogMeanDiffQt(N,NN,5), LogMeanDiffQt(W,N,5), ilog2(W), color)&0x1FFF);
        mixCxt[i++]=(hash(ctx[0], column[0]/8)&0x1FFF);
        mixCxt[i++]=0;//(hash(LogQt(N,5), LogMeanDiffQt(N,NN,3), c0)&0x1FFF);
        mixCxt[i++]=0;//(hash(LogQt(W,5), LogMeanDiffQt(W,WW,3), c0)&0x1FFF);
        mixCxt[i++]=(min(255,(x+line)/32));
    }
    int i=0;
    m.set(mixCxt[i++]|col, 256);
    m.set(mixCxt[i++], 256);
    m.set(mixCxt[i++], 512);
    m.set(mixCxt[i++]|(bpos>>1), 2048);
    m.set(col+(isPNG?(ctx[0]&7)+1:(c0==((0x100|((N+W)/2))>>(8-bpos))))*32, 8*32);i++;
    m.set(mixCxt[i++], 6*64);
    m.set(c0+mixCxt[i++], 256*2);
    m.set(mixCxt[i++]|(bpos>>1), 1024);
    m.set(mixCxt[i++], 8192);
    m.set(mixCxt[i++], 8192);
    m.set(hash(LogQt(N,5), LogMeanDiffQt(N,NN,3), c0)&0x1FFF, 8192);i++;
    m.set(hash(LogQt(W,5), LogMeanDiffQt(W,WW,3), c0)&0x1FFF, 8192);i++;
    m.set(mixCxt[i++], 256);
    
  }
  else{
    m.add( -2048+((filter>>(7-bpos))&1)*4096 );
    m.set(min(4,filter),6);
  }
  return 0;
}



