#include "im8bit.hpp"
//////////////////////////// im8bitModel /////////////////////////////////
// Model for 8-bit image data

  im8bitModel1::im8bitModel1( BlockData& bd):  inpts(49+nPltMaps),cm(CMlimit(MEM()*4), inpts,M_IM8,
  CM_RUN1+
  CM_RUN0+
  CM_MAIN1+
  CM_MAIN2+
  CM_MAIN3+
  CM_MAIN4+
  CM_M12
  ),col(0),xx(bd),buf(bd.buf), ctx(0),lastPos(0), lastWasPNG(0),line(0), x(0),
   filter(0),gray(0),isPNG(0),jump(0), framePos(0), prevFramePos(0), frameWidth(0), prevFrameWidth(0), c4(bd.c4),c0(bd.c0),bpos(bd.bpos), px(0),prvFrmPx(0), prvFrmPred(0),
   res (0),buffer(0x100000),filterOn(false),jumps(0x8000),WWWWWW(0), WWWWW(0), WWWW(0), WWW(0), WW(0), W(0),
      NWWWW(0), NWWW(0), NWW(0), NW(0), N(0), NE(0), NEE(0), NEEE(0), NEEEE(0),
      NNWWW(0), NNWW(0), NNW(0), NN(0), NNE(0), NNEE(0), NNEEE(0),
      NNNWW(0), NNNW(0), NNN(0), NNNE(0), NNNEE(0),
      NNNNW(0), NNNN(0), NNNNE(0), NNNNN(0), NNNNNN(0),MapCtxs(nMaps1),   pOLS(nOLS),sceneOls(13, 1, 0.994){

  }

  
int im8bitModel1::p(Mixer& m,int w,int val2){
  assert(w>3); 
  if (!bpos) {
    if (xx.blpos==1){
      isPNG=  (xx.filetype==PNG8?1:xx.filetype==PNG8GRAY?1:0);
      gray=xx.filetype==PNG8GRAY?1:xx.filetype==IMAGE8GRAY?1:0;
      x =0; line = jump =  px= 0;
      filterOn = false;
      columns[0] = max(1,w/max(1,ilog2(w)*2));
      columns[1] = max(1,columns[0]/max(1,ilog2(columns[0])));
      if (gray){
        if (lastPos && lastWasPNG!=isPNG){
          for (int i=0;i<nMaps;i++)
            Map[i].Reset();
            xx.count=0;
        }
        lastWasPNG = isPNG;
      }
      buffer.Fill(0x7F);
      prevFramePos = framePos;
      framePos = xx.blpos;
      prevFrameWidth = frameWidth;
      frameWidth = w;
    }
    else{
      x++;
      if(x>=w+isPNG){x=0;line++;}
    }
//lastPos = xx.blpos;
    if (isPNG){
      if (x==1)
        filter = (U8)c4;
      else{
        U8 B = (U8)c4;

        switch (filter){
          case 1: {
            buffer.Add((U8)( B + buffer(1)*(x>2 || !x) ) );
            filterOn = x>1;
            px = buffer(1);
            break;
          }
          case 2: {
            buffer.Add((U8)( B + buffer(w)*(filterOn=(line>0)) ) );
            px = buffer(w);
            break;
          }
          case 3: {
            buffer.Add((U8)( B + (buffer(w)*(line>0) + buffer(1)*(x>2 || !x))/2 ) );
            filterOn = (x>1 || line>0);
            px = (buffer(1)*(x>1)+buffer(w)*(line>0))/2;
            break;
          }
          case 4: {
            buffer.Add((U8)( B + Paeth(buffer(1)*(x>2 || !x), buffer(w)*(line>0), buffer(w+1)*(line>0 && (x>2 || !x))) ) );
            filterOn = (x>1 || line>0);
            px = Paeth(buffer(1)*(x>1),buffer(w)*(line>0),buffer(w+1)*(x>1 && line>0));
            break;
          }
          default: buffer.Add(B);
            filterOn = false;
            px = 0;
        }
        if(!filterOn)px=0;
      }
    }  
    else {
      buffer.Add((U8)c4);
      if (x==0) {
        memset(&jumps[0], 0, sizeof(short)*jumps.size());
        if (line>0 && w>8) {
          U8 bMask = 0xFF-((1<<gray)-1);
          U32 pMask = bMask*0x01010101u;
          U32 left=0, right=0;
          int l=min(w, (int)jumps.size()), end=l-4;
          do {
            left = ((buffer(l-x)<<24)|(buffer(l-x-1)<<16)|(buffer(l-x-2)<<8)|buffer(l-x-3))&pMask;
            int i = end;
            while (i>=x+4) {
              right = ((buffer(l-i-3)<<24)|(buffer(l-i-2)<<16)|(buffer(l-i-1)<<8)|buffer(l-i))&pMask;
              if (left==right) {
                int j=(i+3-x-1)/2, k=0;
                for (; k<=j; k++) {
                  if (k<4 || (buffer(l-x-k)&bMask)==(buffer(l-i-3+k)&bMask)) {
                    jumps[x+k] = -(x+(l-i-3)+2*k);
                    jumps[i+3-k] = i+3-x-2*k;
                  }
                  else
                    break;
                }
                x+=k;
                end-=k;
                break;
              }
              i--;
            }
            x++;
            if (x>end)
              break;
          } while (x+4<l);
          x = 0;
        }
      }
    }

    if (x || !isPNG){
      column[0]=(x-isPNG)/columns[0];
      column[1]=(x-isPNG)/columns[1];
      
      WWWWW=buffer(5), WWWW=buffer(4), WWW=buffer(3), WW=buffer(2), W=buffer(1);
      NWWWW=buffer(w+4), NWWW=buffer(w+3), NWW=buffer(w+2), NW=buffer(w+1), N=buffer(w), NE=buffer(w-1), NEE=buffer(w-2), NEEE=buffer(w-3), NEEEE=buffer(w-4);
      NNWWW=buffer(w*2+3), NNWW=buffer(w*2+2), NNW=buffer(w*2+1), NN=buffer(w*2), NNE=buffer(w*2-1), NNEE=buffer(w*2-2), NNEEE=buffer(w*2-3);
      NNNWW=buffer(w*3+2), NNNW=buffer(w*3+1), NNN=buffer(w*3), NNNE=buffer(w*3-1), NNNEE=buffer(w*3-2);
      NNNNW=buffer(w*4+1), NNNN=buffer(w*4), NNNNE=buffer(w*4-1);
      NNNNN=buffer(w*5);
      NNNNNN=buffer(w*6);
      if (prevFramePos>0 && prevFrameWidth==w){
        int offset = prevFramePos+line*w+x;
        prvFrmPx = buf[offset];
        if (gray) {
          sceneOls.Update(W);
          sceneOls.Add(W); sceneOls.Add(NW); sceneOls.Add(N); sceneOls.Add(NE);
          for (int i=-1; i<2; i++) {
            for (int j=-1; j<2; j++)
              sceneOls.Add(buf[offset+i*w+j]);
          }
          prvFrmPred = Clip(int(floor(sceneOls.Predict())));
        }
        else
          prvFrmPred = W;
      }
      else
        prvFrmPx = prvFrmPred = W;
      sceneMap[0].set_direct(prvFrmPx);
      sceneMap[1].set_direct(prvFrmPred);

      int j = 0;
      jump = jumps[min(x,(int)jumps.size()-1)];

      U64 i= (filterOn ? (filter+1)*64 : 0) + (gray*1024);
      cm.set(hash(++i, (jump!=0)?(0x100|buffer(abs(jump)))*(1-2*(jump<0)):N, line&3));
      
      
      if (!gray){
       for (j=0; j<nPltMaps; j++)
        iCtx[j]+=W;
      iCtx[0]=W|(NE<<8);
      iCtx[1]=W|(N<<8);
      iCtx[2]=W|(WW<<8);
      iCtx[3]=N|(NN<<8);
      
        cm.set(hash(++i, W, px));
        cm.set(hash(++i, W, px, column[0]));
        cm.set(hash(++i, N, px));
        cm.set(hash(++i, N, px, column[0]));
        cm.set(hash(++i, NW, px));
        cm.set(hash(++i, NW, px, column[0]));
        cm.set(hash(++i, NE, px));
        cm.set(hash(++i, NE, px, column[0]));
        cm.set(hash(++i, NWW, px));
        cm.set(hash(++i, NEE, px));
        cm.set(hash(++i, WW, px));
        cm.set(hash(++i, NN, px));
        cm.set(hash(++i, W, N, px));
        cm.set(hash(++i, W, NW, px));
        cm.set(hash(++i, W, NE, px));
        cm.set(hash(++i, W, NEE, px));
        cm.set(hash(++i, W, NWW, px));
        cm.set(hash(++i, N, NW, px));
        cm.set(hash(++i, N, NE, px));
        cm.set(hash(++i, NW, NE, px));
        cm.set(hash(++i, W, WW, px));
        cm.set(hash(++i, N, NN, px));
        cm.set(hash(++i, NW, NNWW, px));
        cm.set(hash(++i, NE, NNEE, px));
        cm.set(hash(++i, NW, NWW, px));
        cm.set(hash(++i, NW, NNW, px));
        cm.set(hash(++i, NE, NEE, px));
        cm.set(hash(++i, NE, NNE, px));
        cm.set(hash(++i, N, NNW, px));
        cm.set(hash(++i, N, NNE, px));
        cm.set(hash(++i, N, NNN, px));
        cm.set(hash(++i, W, WWW, px));
        cm.set(hash(++i, WW, NEE, px));
        cm.set(hash(++i, WW, NN, px));
        cm.set(hash(++i, W, buffer(w-3), px));
        cm.set(hash(++i, W, buffer(w-4), px));
        cm.set(hash(++i, W, N,NW, px));
        cm.set(hash(++i, N, NN,NNN, px));
        cm.set(hash(++i, W, NE,NEE, px));
        cm.set(hash(hash( W,NW,N,NE), px));
        cm.set(hash( hash(N,NE,NN,NNE), px));
        cm.set(hash( hash(N,NW,NNW,NN), px));
        cm.set(hash( hash(W,WW,NWW,NW), px));
        cm.set(hash(++i, W, NW<<8 | N, WW<<8 | NWW, px));
        cm.set(hash(++i, px, column[0]));
        cm.set(hash(++i, px));
        cm.set(hash(++i, N, px, column[1] ));
        cm.set(hash(++i, W, px, column[1] ));
        for (int j=0; j<nPltMaps; j++)
          cm.set(hash(++i, iCtx[j](), px));
        ctx = min(0x1F,(x-isPNG)/min(0x20,columns[0]));
        res = W;
      }
      else{
      MapCtxs[j++] = Clamp4(W+N-NW,W,NW,N,NE);
      MapCtxs[j++] = Clip(W+N-NW);
      MapCtxs[j++] = Clamp4(W+NE-N,W,NW,N,NE);
      MapCtxs[j++] = Clip(W+NE-N);
      MapCtxs[j++] = Clamp4(N+NW-NNW,W,NW,N,NE);
      MapCtxs[j++] = Clip(N+NW-NNW);
      MapCtxs[j++] = Clamp4(N+NE-NNE,W,N,NE,NEE);
      MapCtxs[j++] = Clip(N+NE-NNE);
      MapCtxs[j++] = (W+NEE)/2;
      MapCtxs[j++] = Clip(N*3-NN*3+NNN);
      MapCtxs[j++] = Clip(W*3-WW*3+WWW);
      MapCtxs[j++] = (W+Clip(NE*3-NNE*3+buffer(w*3-1)))/2;
      MapCtxs[j++] = (W+Clip(NEE*3-buffer(w*2-3)*3+buffer(w*3-4)))/2;
      MapCtxs[j++] = Clip(NN+buffer(w*4)-buffer(w*6));
      MapCtxs[j++] = Clip(WW+buffer(4)-buffer(6));
      MapCtxs[j++] = Clip((buffer(w*5)-6*buffer(w*4)+15*NNN-20*NN+15*N+Clamp4(W*2-NWW,W,NW,N,NN))/6);
      MapCtxs[j++] = Clip((-3*WW+8*W+Clamp4(NEE*3-NNEE*3+buffer(w*3-2),NE,NEE,buffer(w-3),buffer(w-4)))/6);
      MapCtxs[j++] = Clip(NN+NW-buffer(w*3+1));
      MapCtxs[j++] = Clip(NN+NE-buffer(w*3-1));
      MapCtxs[j++] = Clip((W*2+NW)-(WW+2*NWW)+buffer(w+3));
      MapCtxs[j++] = Clip(((NW+NWW)/2)*3-buffer(w*2+3)*3+(buffer(w*3+4)+buffer(w*3+5))/2);
      MapCtxs[j++] = Clip(NEE+NE-buffer(w*2-3));
      MapCtxs[j++] = Clip(NWW+WW-buffer(w+4));
      MapCtxs[j++] = Clip(((W+NW)*3-NWW*6+buffer(w+3)+buffer(w*2+3))/2);
      MapCtxs[j++] = Clip((NE*2+NNE)-(NNEE+buffer(w*3-2)*2)+buffer(w*4-3));
      MapCtxs[j++] = buffer(w*6);
      MapCtxs[j++] = (buffer(w-4)+buffer(w-6))/2;
      MapCtxs[j++] = (buffer(4)+buffer(6))/2;
      MapCtxs[j++] = (W+N+buffer(w-5)+buffer(w-7))/4;
      MapCtxs[j++] = Clip(buffer(w-3)+W-NEE);
      MapCtxs[j++] = Clip(4*NNN-3*buffer(w*4));
      MapCtxs[j++] = Clip(N+NN-NNN);
      MapCtxs[j++] = Clip(W+WW-WWW);
      MapCtxs[j++] = Clip(W+NEE-NE);
      MapCtxs[j++] = Clip(WW+NEE-N);
      MapCtxs[j++] = (Clip(W*2-NW)+Clip(W*2-NWW)+N+NE)/4;
      MapCtxs[j++] = Clamp4(N*2-NN,W,N,NE,NEE);
      MapCtxs[j++] = (N+NNN)/2;
      MapCtxs[j++] = Clip(NN+W-NNW);
      MapCtxs[j++] = Clip(NWW+N-NNWW);
      MapCtxs[j++] = Clip((4*WWW-15*WW+20*W+Clip(NEE*2-NNEE))/10);
      MapCtxs[j++] = Clip((buffer(w*3-3)-4*NNEE+6*NE+Clip(W*3-NW*3+NNW))/4);
      MapCtxs[j++] = Clip((N*2+NE)-(NN+2*NNE)+buffer(w*3-1));
      MapCtxs[j++] = Clip((NW*2+NNW)-(NNWW+buffer(w*3+2)*2)+buffer(w*4+3));
      MapCtxs[j++] = Clip(NNWW+W-buffer(w*2+3));
      MapCtxs[j++] = Clip((-buffer(w*4)+5*NNN-10*NN+10*N+Clip(W*4-NWW*6+buffer(w*2+3)*4-buffer(w*3+4)))/5);
      MapCtxs[j++] = Clip(NEE+Clip(buffer(w-3)*2-buffer(w*2-4))-buffer(w-4));
      MapCtxs[j++] = Clip(NW+W-NWW);
      MapCtxs[j++] = Clip((N*2+NW)-(NN+2*NNW)+buffer(w*3+1));
      MapCtxs[j++] = Clip(NN+Clip(NEE*2-buffer(w*2-3))-NNE);
      MapCtxs[j++] = Clip((-buffer(4)+5*WWW-10*WW+10*W+Clip(NE*2-NNE))/5);
      MapCtxs[j++] = Clip((-buffer(5)+4*buffer(4)-5*WWW+5*W+Clip(NE*2-NNE))/4);
      MapCtxs[j++] = Clip((WWW-4*WW+6*W+Clip(NE*3-NNE*3+buffer(w*3-1)))/4);
      MapCtxs[j++] = Clip((-NNEE+3*NE+Clip(W*4-NW*6+NNW*4-buffer(w*3+1)))/3);
      MapCtxs[j++] = ((W+N)*3-NW*2)/4;
      for (j=0; j<nOLS; j++) {
        ols[j].Update(W);
        pOLS[j] = Clip(int(floor(ols[j].Predict(ols_ctxs[j]))));
      }
      
     
        cm.set();
        cm.set(hash(++i, N, px));
        cm.set(hash(++i, N-px));
        cm.set(hash(++i, W, px));
        cm.set(hash(++i, NW, px));
        cm.set(hash(++i, NE, px));
        cm.set(hash(++i, N, NN, px));
        cm.set(hash(++i, W, WW, px));
        cm.set(hash(++i, NE, NNEE, px ));
        cm.set(hash(++i, NW, NNWW, px ));
        cm.set(hash(++i, W, NEE, px));
        cm.set(hash(++i, (Clamp4(W+N-NW,W,NW,N,NE)-px)/2, LogMeanDiffQt(Clip(N+NE-NNE), Clip(N+NW-NNW))));
        cm.set(hash(++i, (W-px)/4, (NE-px)/4, column[0]));
        cm.set(hash(++i, (Clip(W*2-WW)-px)/4, (Clip(N*2-NN)-px)/4));
        cm.set(hash(++i, (Clamp4(N+NE-NNE,W,N,NE,NEE)-px)/4, column[0]));
        cm.set(hash(++i, (Clamp4(N+NW-NNW,W,NW,N,NE)-px)/4, column[0]));
        cm.set(hash(++i, (W+NEE)/4, px, column[0]));
        cm.set(hash(++i, Clip(W+N-NW)-px, column[0]));
        cm.set(hash(++i, Clamp4(N*3-NN*3+NNN,W,N,NN,NE), px, LogMeanDiffQt(W,Clip(NW*2-NNW))));
        cm.set(hash(++i, Clamp4(W*3-WW*3+WWW,W,N,NE,NEE), px, LogMeanDiffQt(N,Clip(NW*2-NWW))));
        cm.set(hash(++i, (W+Clamp4(NE*3-NNE*3+NNNE,W,N,NE,NEE))/2, px, LogMeanDiffQt(N,(NW+NE)/2)));
        cm.set(hash(++i, (N+NNN)/8, Clip(N*3-NN*3+NNN)/4, px));
        cm.set(hash(++i, (W+WWW)/8, Clip(W*3-WW*3+WWW)/4, px));
        cm.set(hash(++i, Clip((-buffer(4)+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+buffer(w*3-1)*4-buffer(w*4-1),N,NE,buffer(w-2),buffer(w-3)))/5)-px));
        cm.set(hash(++i, Clip(N*2-NN)-px, LogMeanDiffQt(N,Clip(NN*2-NNN))));
        cm.set(hash(++i, Clip(W*2-WW)-px, LogMeanDiffQt(NE,Clip(N*2-NW))));

      
        if (isPNG)
          ctx = ((abs(W-N)>8)<<10)|((W>N)<<9)|((abs(N-NW)>8)<<8)|((N>NW)<<7)|((abs(N-NE)>8)<<6)|((N>NE)<<5)|((W>WW)<<4)|((N>NN)<<3)|min(5,filterOn?filter+1:0);
        else
          ctx = min(0x1F,x/max(1,w/min(32,columns[0])))|( ( ((abs(W-N)*16>W+N)<<1)|(abs(N-NW)>8) )<<5 )|((W+N)&0x180);

        res = Clamp4(W+N-NW,W,NW,N,NE)-px;
      }
        xx.Image.pixels.W = W;
        xx.Image.pixels.N = N;
        xx.Image.pixels.NN = NN;
        xx.Image.pixels.WW = WW;
        xx.Image.ctx = ctx>>gray;
    }
  }
  U8 B=(c0<<(8-bpos));
  if (x || !isPNG){
      if (gray) {
    int i=1;
    Map[i++].set((((U8)(Clip(W+N-NW)-px-B))*8+bpos)|(LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))<<11));
    
    for (int j=0; j<nMaps1; i++, j++)
      Map[i].set((MapCtxs[j]-px-B)*8+bpos);

    for (int j=0; i<nMaps; i++, j++)
      Map[i].set((pOLS[j]-px-B)*8+bpos);
  }
    sceneMap[2].set_direct(finalize64(hash(x, line), 19)*8+bpos);
    sceneMap[3].set_direct((prvFrmPx-B)*8+bpos);
    sceneMap[4].set_direct((prvFrmPred-B)*8+bpos);
}
  // Predict next bit
  if (x || !isPNG){
  col=(col+1)&7;
      if (val2==1)  return 1;   
    cm.mix(m);
    if (gray){
      for (int i=0;i<nMaps;i++)
        Map[i].mix1(m);
    }else {
      for (int i=0; i<nPltMaps; i++) {
        pltMap[i].set((bpos<<8)|iCtx[i]());
        pltMap[i].mix(m);
      }
    }
    for (int i=0; i<5; i++)
      sceneMap[i].mix(m, (prevFramePos>0 && prevFrameWidth==w), 4, 255);

    m.set(5+ctx, 2048+5);
    m.set(col*2+(isPNG && c0==((0x100|res)>>(8-bpos))) + min(5,filterOn?filter+1:0)*16, 6*16);
    m.set(((isPNG?px:N+W)>>4) + min(5,filterOn?filter+1:0)*32, 6*32);
    m.set(c0, 256);
    m.set( ((abs((int)(W-N))>4)<<9)|((abs((int)(N-NE))>4)<<8)|((abs((int)(W-NW))>4)<<7)|((W>N)<<6)|((N>NE)<<5)|((W>NW)<<4)|((W>WW)<<3)|((N>NN)<<2)|((NW>NNWW)<<1)|(NE>NNEE), 1024 );
    m.set(min(63,column[0]), 64);
    m.set(min(127,column[1]), 128);
    m.set( min(255,(x+line)/32), 256);
  }
  else{
    m.add( -2048+((filter>>(7-bpos))&1)*4096 );
    m.set(min(4,filter),5);
  }
  return 0; //8 8 32 256 512 1792
  }

