#include "im8bit.hpp"
//////////////////////////// im8bitModel /////////////////////////////////
// Model for 8-bit image data

im8bitModel1::im8bitModel1( BlockData& bd):inpts(48+1+1+1+1+1+1-3),
  cm(bd,inpts,CMlimit(bd.MEM()*4)),
  cmp(bd,nPltMaps,CMlimit(bd.MEM()/4)),
  col(0),xx(bd),buf(bd.buf), ctx(0),lastPos(0), line(0), x(0),
  filter(0),gray(0),jump(0), framePos(0), prevFramePos(0), frameWidth(0), prevFrameWidth(0), c4(bd.c4),c0(bd.c0),bpos(bd.bpos),prvFrmPx(0), prvFrmPred(0),
  res(0),jumps(0x8000),WWWWWW(0), WWWWW(0), WWWW(0), WWW(0), WW(0), W(0),
  NWWWW(0), NWWW(0), NWW(0), NW(0), N(0), NE(0), NEE(0), NEEE(0), NEEEE(0),
  NNWWW(0), NNWW(0), NNW(0), NN(0), NNE(0), NNEE(0), NNEEE(0),
  NNNWW(0), NNNW(0), NNN(0), NNNE(0), NNNEE(0),
  NNNNW(0), NNNN(0), NNNNE(0), NNNNN(0), NNNNNN(0),MapCtxs(nMaps1), pOLS(nOLS), sceneOls(13, 1, 0.994) {

    // Set  model mixer contexts and parameters
    mxp.push_back( {   512,64/2,0,28,&mxcxt[0],0,false} );
    mxp.push_back( {    16,64/2,0,28,&mxcxt[1],0,false} );
    mxp.push_back( {    32,64/2,0,28,&mxcxt[2],0,false} );
    mxp.push_back( {256*16,64/2,0,28,&mxcxt[3],0,false} );
    mxp.push_back( {  1024,64/2,0,28,&mxcxt[4],0,false} );
    mxp.push_back( { 64*16,64/2,0,28,&mxcxt[5],0,false} );
    mxp.push_back( {   128,64/2,0,28,&mxcxt[6],0,false} );
    mxp.push_back( {   256,64/2,0,28,&mxcxt[7],0,false} );
    mxp.push_back( {  1024,64/2,0,28,&mxcxt[8],0,false} );
}

int im8bitModel1::p(Mixers& m ,int w, int val2) {
    assert(w>3); 
    if (!bpos) {
        if (xx.blpos==1) {
            gray=xx.filetype==IMAGE8GRAY?1:0;
            x =0; line = jump =  0;
            columns[0] = max(1,w/max(1,ilog2(w)*2));
            columns[1] = max(1,columns[0]/max(1,ilog2(columns[0])));
            if (gray) {
                if (lastPos && false){
                    for (int i=0;i<nMaps;i++)
                    Map[i].Reset();
                    xx.count=0;
                }
            }
            prevFramePos = framePos;
            framePos = xx.buf.pos ;
            prevFrameWidth = frameWidth;
            frameWidth = w;
        } else {
            x++;
            if(x>=w) {
                x=0;
                line++;
            }
        }
        
        if (x==0) {
            memset(&jumps[0], 0, sizeof(short)*jumps.size());
            if (line>0 && w>8) {
                U8 bMask = 0xFF-((1<<gray)-1);
                U32 pMask = bMask*0x01010101u;
                U32 left=0, right=0;
                int l=min(w, (int)jumps.size()), end=l-4;
                do {
                    left = ((buf(l-x)<<24)|(buf(l-x-1)<<16)|(buf(l-x-2)<<8)|buf(l-x-3))&pMask;
                    int i = end;
                    while (i>=x+4) {
                        right = ((buf(l-i-3)<<24)|(buf(l-i-2)<<16)|(buf(l-i-1)<<8)|buf(l-i))&pMask;
                        if (left==right) {
                            int j=(i+3-x-1)/2, k=0;
                            for (; k<=j; k++) {
                                if (k<4 || (buf(l-x-k)&bMask)==(buf(l-i-3+k)&bMask)) {
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
        
        column[0]=x/columns[0];
        column[1]=x/columns[1];
        
        WWWWW=buf(5), WWWW=buf(4), WWW=buf(3), WW=buf(2), W=buf(1);
        NWWWW=buf(w+4), NWWW=buf(w+3), NWW=buf(w+2), NW=buf(w+1), N=buf(w), NE=buf(w-1), NEE=buf(w-2), NEEE=buf(w-3), NEEEE=buf(w-4);
        NNWWW=buf(w*2+3), NNWW=buf(w*2+2), NNW=buf(w*2+1), NN=buf(w*2), NNE=buf(w*2-1), NNEE=buf(w*2-2), NNEEE=buf(w*2-3);
        NNNWW=buf(w*3+2), NNNW=buf(w*3+1), NNN=buf(w*3), NNNE=buf(w*3-1), NNNEE=buf(w*3-2);
        NNNNW=buf(w*4+1), NNNN=buf(w*4), NNNNE=buf(w*4-1);
        NNNNN=buf(w*5);
        NNNNNN=buf(w*6);
        if (prevFramePos>0 && prevFrameWidth==w) {
            int offset = prevFramePos+line*w+x;
            prvFrmPx = buf[offset];
            if (gray) {
                sceneOls.Update(W);
                sceneOls.Add(W);
                sceneOls.Add(NW);
                sceneOls.Add(N);
                sceneOls.Add(NE);
                for (int i=-1; i<2; i++) {
                    for (int j=-1; j<2; j++)
                    sceneOls.Add(buf[offset+i*w+j]);
                }
                prvFrmPred = Clip(int(roundf(sceneOls.Predict())));
            }
            else
                prvFrmPred = W;
        } else
            prvFrmPx = prvFrmPred = W;
        
        sceneMap[0].set_direct(prvFrmPx);
        sceneMap[1].set_direct(prvFrmPred);

        int j = 0;
        jump = jumps[min(x,(int)jumps.size()-1)];

        U64 i= gray*1024;
        cm.set(hash(++i, (jump!=0)?(0x100|buf(abs(jump)))*(1-2*(jump<0)):N, line&3));

        if (!gray) {
            for (j=0; j<nPltMaps; j++)
            iCtx[j]+=W;
            iCtx[0]=W|(NE<<8);
            iCtx[1]=W|(N<<8);
            iCtx[2]=W|(WW<<8);
            iCtx[3]=N|(NN<<8);
            U32 bcxt[53];
            i=0;
            bcxt[i++]=( W);
            bcxt[i++]=(( W+ column[0]*256));
            bcxt[i++]=( N);
            bcxt[i++]=(( N+ column[0]*256));
            bcxt[i++]=( NW);
            bcxt[i++]=(( NW+ column[0]*256));
            bcxt[i++]=( NE);
            bcxt[i++]=(( NE+ column[0]*256));
            bcxt[i++]=( NWW);
            bcxt[i++]=( NEE);
            bcxt[i++]=( WW);
            bcxt[i++]=( NN);
            bcxt[i++]=(hash( W, N));
            bcxt[i++]=(( W*N/2));
            bcxt[i++]=(hash( W, NW));
            bcxt[i++]=(hash( W, NE));
            bcxt[i++]=(hash( W, NEE));
            bcxt[i++]=(hash( W, NWW));
            bcxt[i++]=(hash( N, NW));
            bcxt[i++]=(hash( N, NE));
            bcxt[i++]=(hash( NW, NE));
            bcxt[i++]=(hash( W, WW));
            bcxt[i++]=(hash( N, NN));
            bcxt[i++]=(( N* NN/2));
            bcxt[i++]=(hash( NW, NNWW));
            bcxt[i++]=(hash( NE, NNEE));
            bcxt[i++]=(hash( NW, NWW));
            bcxt[i++]=(hash( NW, NNW));
            bcxt[i++]=(hash( NE, NEE));
            bcxt[i++]=(hash( NE, NNE));
            bcxt[i++]=(hash( N, NNW));
            bcxt[i++]=(hash( N, NNE));
            bcxt[i++]=(hash( N, NNN));
            bcxt[i++]=(hash( W, WWW));
            bcxt[i++]=(hash( WW, NEE));
            bcxt[i++]=(hash( WW, NN));
            bcxt[i++]=(hash( W, buf(w-3)));
            bcxt[i++]=(hash( W, buf(w-4)));
            bcxt[i++]=(hash( W, N,NW));
            bcxt[i++]=(hash( N, NN,NNN));
            bcxt[i++]=(hash( W, NE,NEE));
            bcxt[i++]=(hash( W,NW,N,NE));
            bcxt[i++]=(hash( N,NE,NN,NNE));
            bcxt[i++]=(hash( N,NW,NNW,NN));
            bcxt[i++]=(hash( W,WW,NWW,NW));
            bcxt[i++]=(hash( W, NW<<8 | N, WW<<8 | NWW));
            bcxt[i++]=((  column[0]));
            bcxt[i++]=(( Clip((buf(w*5)-6*buf(w*4)+15*NNN-20*NN+15*N+Clamp4(W*2-NWW,W,NW,N,NN))/6)));
            bcxt[i++]=(( N+  column[1]*256 ));
            bcxt[i++]=(( W+  column[1]*256 )); //48
            /*bcxt[i++]=hash(++i, std::sqrt(W*WW+NW*NWW),std::sqrt(N*NN+ NE*NNE), px);
                bcxt[i++]=hash(++i, std::sqrt(W*WW+NW*NWW),std::sqrt(N*NN+ NE*NNE),std::sqrt(N*NW+ NN*NNW) ,px);
                bcxt[i++]=hash(++i, std::sqrt(W*WW+NW*NWW)-std::sqrt(W*NW+ N*128) ,px);*/
            for (int j=0; j<53-3; j++) cm.set(bcxt[j]);
            for (int j=0; j<nPltMaps; j++) //4
            cmp.set(( iCtx[j]()*191+W));
            ctx = min(0xfF,(x/*-isPNG*/)/min(0x20,columns[0]));
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
            MapCtxs[j++] = (W+Clip(NE*3-NNE*3+buf(w*3-1)))/2;
            MapCtxs[j++] = (W+Clip(NEE*3-buf(w*2-3)*3+buf(w*3-4)))/2;
            MapCtxs[j++] = Clip(NN+buf(w*4)-buf(w*6));
            MapCtxs[j++] = Clip(WW+buf(4)-buf(6));
            MapCtxs[j++] = Clip((buf(w*5)-6*buf(w*4)+15*NNN-20*NN+15*N+Clamp4(W*2-NWW,W,NW,N,NN))/6);
            MapCtxs[j++] = Clip((-3*WW+8*W+Clamp4(NEE*3-NNEE*3+buf(w*3-2),NE,NEE,buf(w-3),buf(w-4)))/6);
            MapCtxs[j++] = Clip(NN+NW-buf(w*3+1));
            MapCtxs[j++] = Clip(NN+NE-buf(w*3-1));
            MapCtxs[j++] = Clip((W*2+NW)-(WW+2*NWW)+buf(w+3));
            MapCtxs[j++] = Clip(((NW+NWW)/2)*3-buf(w*2+3)*3+(buf(w*3+4)+buf(w*3+5))/2);
            MapCtxs[j++] = Clip(NEE+NE-buf(w*2-3));
            MapCtxs[j++] = Clip(NWW+WW-buf(w+4));
            MapCtxs[j++] = Clip(((W+NW)*3-NWW*6+buf(w+3)+buf(w*2+3))/2);
            MapCtxs[j++] = Clip((NE*2+NNE)-(NNEE+buf(w*3-2)*2)+buf(w*4-3));
            MapCtxs[j++] = buf(w*6);
            MapCtxs[j++] = (buf(w-4)+buf(w-6))/2;
            MapCtxs[j++] = (buf(4)+buf(6))/2;
            MapCtxs[j++] = (W+N+buf(w-5)+buf(w-7))/4;
            MapCtxs[j++] = Clip(buf(w-3)+W-NEE);
            MapCtxs[j++] = Clip(4*NNN-3*buf(w*4));
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
            MapCtxs[j++] = Clip((buf(w*3-3)-4*NNEE+6*NE+Clip(W*3-NW*3+NNW))/4);
            MapCtxs[j++] = Clip((N*2+NE)-(NN+2*NNE)+buf(w*3-1));
            MapCtxs[j++] = Clip((NW*2+NNW)-(NNWW+buf(w*3+2)*2)+buf(w*4+3));
            MapCtxs[j++] = Clip(NNWW+W-buf(w*2+3));
            MapCtxs[j++] = Clip((-buf(w*4)+5*NNN-10*NN+10*N+Clip(W*4-NWW*6+buf(w*2+3)*4-buf(w*3+4)))/5);
            MapCtxs[j++] = Clip(NEE+Clip(buf(w-3)*2-buf(w*2-4))-buf(w-4));
            MapCtxs[j++] = Clip(NW+W-NWW);
            MapCtxs[j++] = Clip((N*2+NW)-(NN+2*NNW)+buf(w*3+1));
            MapCtxs[j++] = Clip(NN+Clip(NEE*2-buf(w*2-3))-NNE);
            MapCtxs[j++] = Clip((-buf(4)+5*WWW-10*WW+10*W+Clip(NE*2-NNE))/5);
            MapCtxs[j++] = Clip((-buf(5)+4*buf(4)-5*WWW+5*W+Clip(NE*2-NNE))/4);
            MapCtxs[j++] = Clip((WWW-4*WW+6*W+Clip(NE*3-NNE*3+buf(w*3-1)))/4);
            MapCtxs[j++] = Clip((-NNEE+3*NE+Clip(W*4-NW*6+NNW*4-buf(w*3+1)))/3);
            MapCtxs[j++] = ((W+N)*3-NW*2)/4;
            for (j=0; j<nOLS; j++) {
                ols[j].Update(W);
                pOLS[j] = Clip(int(roundf(ols[j].Predict(ols_ctxs[j]))));
            }

            cm.set(hash(++i, N));
            cm.set(hash(++i, N));//
            cm.set(hash(++i, W));
            cm.set(hash(++i, NW));
            cm.set(hash(++i, NE));
            cm.set(hash(++i, N, NN));
            cm.set(hash(++i, W, WW));
            cm.set(hash(++i, NE, NNEE ));
            cm.set(hash(++i, NW, NNWW ));
            cm.set(hash(++i, W, NEE));
            cm.set(hash(++i, (Clamp4(W+N-NW,W,NW,N,NE))/2, LogMeanDiffQt(Clip(N+NE-NNE), Clip(N+NW-NNW))));
            cm.set(hash(++i, (W)/4, (NE)/4, column[0]));
            cm.set(hash(++i, (Clip(W*2-WW))/4, (Clip(N*2-NN))/4));
            cm.set(hash(++i, (Clamp4(N+NE-NNE,W,N,NE,NEE))/4, column[0]));
            cm.set(hash(++i, (Clamp4(N+NW-NNW,W,NW,N,NE))/4, column[0]));
            cm.set(hash(++i, (W+NEE)/4, column[0]));
            cm.set(hash(++i, Clip(W+N-NW), column[0]));
            cm.set(hash(++i, Clamp4(N*3-NN*3+NNN,W,N,NN,NE), LogMeanDiffQt(W,Clip(NW*2-NNW))));
            cm.set(hash(++i, Clamp4(W*3-WW*3+WWW,W,N,NE,NEE), LogMeanDiffQt(N,Clip(NW*2-NWW))));
            cm.set(hash(++i, (W+Clamp4(NE*3-NNE*3+NNNE,W,N,NE,NEE))/2, LogMeanDiffQt(N,(NW+NE)/2)));
            cm.set(hash(++i, (N+NNN)/8, Clip(N*3-NN*3+NNN)/4));
            cm.set(hash(++i, (W+WWW)/8, Clip(W*3-WW*3+WWW)/4));
            cm.set(hash(++i, Clip((-buf(4)+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+buf(w*3-1)*4-buf(w*4-1),N,NE,buf(w-2),buf(w-3)))/5)));
            cm.set(hash(++i, Clip(N*2-NN), LogMeanDiffQt(N,Clip(NN*2-NNN))));
            cm.set(hash(++i, Clip(W*2-WW), LogMeanDiffQt(NE,Clip(N*2-NW)))); //25
            for (j=26; j<inpts; j++) cm.sets();
            for (int j=0; j<nPltMaps; j++) cmp.sets();
            
            ctx = min(0x1F,x/max(1,w/min(32,columns[0])))|( ( ((abs(W-N)*16>W+N)<<1)|(abs(N-NW)>8) )<<5 )|((W+N)&0x180);

            res = Clamp4(W+N-NW,W,NW,N,NE);
        }
        xx.Image.pixels.W = W;
        xx.Image.pixels.N = N;
        xx.Image.pixels.NN = NN;
        xx.Image.pixels.WW = WW;
        xx.Image.pixels.NE = NE;
        xx.Image.ctx = ctx>>gray;
        
    }
    U8 B=(c0<<(8-bpos));

    if (gray) {
        int i=1;
        Map[i++].set((((U8)(Clip(W+N-NW)-B))*8+bpos)|(LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))<<11));
        
        for (int j=0; j<nMaps1; i++, j++)
        Map[i].set((MapCtxs[j]-B)*8+bpos);

        for (int j=0; i<nMaps; i++, j++)
        Map[i].set((pOLS[j]-B)*8+bpos);
    }
    sceneMap[2].set_direct(finalize64(hash(x, line), 19)*8+bpos);
    sceneMap[3].set_direct((prvFrmPx-B)*8+bpos);
    sceneMap[4].set_direct((prvFrmPred-B)*8+bpos);
    // }
    // Predict next bit

    col=(col+1)&7;
    if (val2==1)  return 1;
    cm.mix(m);
    cmp.mix(m);
    if (gray) {
        for (int i=0; i<nMaps; i++)
        Map[i].mix1(m,4);
    } else {
        for (int i=0; i<nPltMaps; i++) {
            pltMap[i].set((bpos<<8)|iCtx[i]());
            pltMap[i].mix(m,7,4);
        }
    }
    for (int i=0; i<5; i++)
    sceneMap[i].mix(m, (prevFramePos>0 && prevFrameWidth==w), 4, 255);

    mxcxt[0]=ctx;
    mxcxt[1]=col*2+(c0==((0x100|res)>>(8-bpos)));
    mxcxt[2]=(N+W)>>4;
    mxcxt[3]=(W>>4)*256+c0-1;
    mxcxt[4]=((abs((int)(W-N))>4)<<9)|((abs((int)(N-NE))>4)<<8)|((abs((int)(W-NW))>4)<<7)|((W>N)<<6)|((N>NE)<<5)|((W>NW)<<4)|((W>WW)<<3)|((N>NN)<<2)|((NW>NNWW)<<1)|(NE>NNEE);
    mxcxt[5]=min(63,column[0])*16+(W>>4);
    mxcxt[6]=min(127,column[1]);
    mxcxt[7]=min(255,(x+line)/32);
    mxcxt[8]=gray?((N>>5)*8+(W>>5)*64+(NW>>5))*2+(bpos==0):-1;//((W==WW)*2+(N=NN));
    return 0;
}
