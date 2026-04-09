#include "im24bit.hpp"

// This model is from paq8px

//////////////////////////// im24bitModel /////////////////////////////////
// Model for 24-bit image data

im24bitModel1::im24bitModel1(BlockData& bd): nOLS(6),inpts(47),
cm(bd,inpts,CMlimit(bd.MEM()*4)),
col(0) ,color(-1),stride(3), padding(0), x(0),xx(bd),
buf(bd.buf), WWW(0), WW(0), W(0),NWW(0),NW(0) ,N(0), NE(0), NEE(0), NNWW(0), NNW(0),
NN(0), NNE(0), NNEE(0), NNN(0),  w(0), line(0), R1(0), R2(0),
c4(bd.c4),c0(bd.c0),bpos(bd.bpos), WWp1(0), Wp1(0), p1(0), NWp1(0),
Np1(0), NEp1(0), NNp1(0),p2(0),lastw(0),lastpos(0),curpos(0), MapCtxs(n2Maps1), SCMapCtxs(nSCMaps-1), pOLS(nOLS) {

    columns[0] = 1, columns[1]=1;
    column[0]=0,column[1]=0;
    ctx[0]=0,ctx[1]=0;
    
    // Set model mixer contexts and parameters
    mxp.push_back( {  256,64,0,28,&mxcxt[0],0,false} );
    mxp.push_back( {  256,64,0,28,&mxcxt[1],0,false} );
    mxp.push_back( {  512,64,0,28,&mxcxt[2],0,false} );
    mxp.push_back( { 2048,64,0,28,&mxcxt[3],0,false} );
    mxp.push_back( { 8*32,64,0,28,&mxcxt[4],0,false} );
    mxp.push_back( {    4,64,0,28,&mxcxt[5],0,false} );
    mxp.push_back( {256*4,64,0,28,&mxcxt[6],0,false} );
    mxp.push_back( { 1024,64,0,28,&mxcxt[7],0,false} );
    mxp.push_back( { 8192,64,0,28,&mxcxt[8],0,false} );
    mxp.push_back( { 8192,64,0,28,&mxcxt[9],0,false} );
    mxp.push_back( { 8192,64,0,28,&mxcxt[10],0,false} );
    mxp.push_back( { 8192,64,0,28,&mxcxt[11],0,false} );
    mxp.push_back( {  256,64,0,28,&mxcxt[12],0,false} );
}

int im24bitModel1::p(Mixers& m, int info, int val2) {
    if (!bpos) {
        if (xx.blpos==1) {
            const int alpha=xx.filetype==IMAGE32?1:0;
            stride = 3+alpha;
            lastpos=curpos;
            curpos=buf.pos;
            lastw=w;
            w = info&0xFFFFFF;
            
            padding = w%stride;
            
            x =1; color = line =0;
            columns[0] = max(1,w/max(1,ilog2(w)*3));
            columns[1] = max(1,columns[0]/max(1,ilog2(columns[0])));
            for (int i=0;i<n2Maps;i++)
            Map[i].Reset();
            xx.count=0;
        } else {
            x++;
            if(x>=w) {
                x=0;
                line++;
            }
        }

        
        if (x+padding<w) {
            color++;
            if (color>=stride) color=0;
        } else {
            if (padding>0) color=stride;
            else color=0;
        }
        
        int i=color<<5;
        column[0]=x/columns[0];
        column[1]=x/columns[1];
        WWWWWW=buf(6*stride), WWWWW=buf(5*stride), WWWW=buf(4*stride), WWW=buf(3*stride), WW=buf(2*stride), W=buf(stride);
        NWWWW=buf(w+4*stride), NWWW=buf(w+3*stride), NWW=buf(w+2*stride), NW=buf(w+stride), N=buf(w), NE=buf(w-stride), NEE=buf(w-2*stride), NEEE=buf(w-3*stride), NEEEE=buf(w-4*stride);
        NNWWW=buf(w*2+stride*3), NNWW=buf((w+stride)*2), NNW=buf(w*2+stride), NN=buf(w*2), NNE=buf(w*2-stride), NNEE=buf((w-stride)*2), NNEEE=buf(w*2-stride*3);
        NNNWW=buf(w*3+stride*2), NNNW=buf(w*3+stride), NNN=buf(w*3), NNNE=buf(w*3-stride), NNNEE=buf(w*3-stride*2);
        NNNNW=buf(w*4+stride), NNNN=buf(w*4), NNNNE=buf(w*4-stride);
        NNNNN=buf(w*5);
        NNNNNN=buf(w*6);
        WWp1=buf(stride*2+1), Wp1=buf(stride+1), p1=buf(1), NWp1=buf(w+stride+1), Np1=buf(w+1), NEp1=buf(w-stride+1), NNp1=buf(w*2+1);
        WWp2=buf(stride*2+2), Wp2=buf(stride+2), p2=buf(2), NWp2=buf(w+stride+2), Np2=buf(w+2), NEp2=buf(w-stride+2), NNp2=buf(w*2+2);
        
        int j = 0;
        MapCtxs[j++] = Clamp4(N+p1-Np1, W, NW, N, NE);
        MapCtxs[j++] = Clamp4(N+p2-Np2, W, NW, N, NE);
        MapCtxs[j++] = (W+Clamp4(NE*3-NNE*3+NNNE, W, N, NE, NEE))/2;
        MapCtxs[j++] = Clamp4((W+Clip(NE*2-NNE))/2, W, NW, N, NE);
        MapCtxs[j++] = (W+NEE)/2;
        MapCtxs[j++] = Clip((WWW-4*WW+6*W+Clip(NE*4-NNE*6+NNNE*4-NNNNE))/4);
        MapCtxs[j++] = Clip((-WWWW+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+NNNE*4-NNNNE, N, NE, NEE, NEEE))/5);
        MapCtxs[j++] = Clip((-4*WW+15*W+10*Clip(NE*3-NNE*3+NNNE)-Clip(NEEE*3-NNEEE*3+buf(w*3-3*stride)))/20);
        MapCtxs[j++] = Clip((-3*WW+8*W+Clamp4(NEE*3-NNEE*3+NNNEE, NE, NEE, NEEE, NEEEE))/6);
        MapCtxs[j++] = Clip((W+Clip(NE*2-NNE))/2+p1-(Wp1+Clip(NEp1*2-buf(w*2-stride+1)))/2);
        MapCtxs[j++] = Clip((W+Clip(NE*2-NNE))/2+p2-(Wp2+Clip(NEp2*2-buf(w*2-stride+2)))/2);
        MapCtxs[j++] = Clip((-3*WW+8*W+Clip(NEE*2-NNEE))/6+p1-(-3*WWp1+8*Wp1+Clip(buf(w-stride*2+1)*2-buf(w*2-stride*2+1)))/6);
        MapCtxs[j++] = Clip((-3*WW+8*W+Clip(NEE*2-NNEE))/6+p2-(-3*WWp2+8*Wp2+Clip(buf(w-stride*2+2)*2-buf(w*2-stride*2+2)))/6);
        MapCtxs[j++] = Clip((W+NEE)/2+p1-(Wp1+buf(w-stride*2+1))/2);
        MapCtxs[j++] = Clip((W+NEE)/2+p2-(Wp2+buf(w-stride*2+2))/2);
        MapCtxs[j++] = Clip((WW+Clip(NEE*2-NNEE))/2+p1-(WWp1+Clip(buf(w-stride*2+1)*2-buf(w*2-stride*2+1)))/2);
        MapCtxs[j++] = Clip((WW+Clip(NEE*2-NNEE))/2+p2-(WWp2+Clip(buf(w-stride*2+2)*2-buf(w*2-stride*2+2)))/2);
        MapCtxs[j++] = Clip(WW+NEE-N+p1-Clip(WWp1+buf(w-stride*2+1)-Np1));
        MapCtxs[j++] = Clip(WW+NEE-N+p2-Clip(WWp2+buf(w-stride*2+2)-Np2));
        MapCtxs[j++] = Clip(W+N-NW);
        MapCtxs[j++] = Clip(W+N-NW+p1-Clip(Wp1+Np1-NWp1));
        MapCtxs[j++] = Clip(W+N-NW+p2-Clip(Wp2+Np2-NWp2));
        MapCtxs[j++] = Clip(W+NE-N);
        MapCtxs[j++] = Clip(N+NW-NNW);
        MapCtxs[j++] = Clip(N+NW-NNW+p1-Clip(Np1+NWp1-buf(w*2+stride+1)));
        MapCtxs[j++] = Clip(N+NW-NNW+p2-Clip(Np2+NWp2-buf(w*2+stride+2)));
        MapCtxs[j++] = Clip(N+NE-NNE);
        MapCtxs[j++] = Clip(N+NE-NNE+p1-Clip(Np1+NEp1-buf(w*2-stride+1)));
        MapCtxs[j++] = Clip(N+NE-NNE+p2-Clip(Np2+NEp2-buf(w*2-stride+2)));
        MapCtxs[j++] = Clip(N+NN-NNN);
        MapCtxs[j++] = Clip(N+NN-NNN+p1-Clip(Np1+NNp1-buf(w*3+1)));
        MapCtxs[j++] = Clip(N+NN-NNN+p2-Clip(Np2+NNp2-buf(w*3+2)));
        MapCtxs[j++] = Clip(W+WW-WWW);
        MapCtxs[j++] = Clip(W+WW-WWW+p1-Clip(Wp1+WWp1-buf(stride*3+1)));
        MapCtxs[j++] = Clip(W+WW-WWW+p2-Clip(Wp2+WWp2-buf(stride*3+2)));
        MapCtxs[j++] = Clip(W+NEE-NE);
        MapCtxs[j++] = Clip(W+NEE-NE+p1-Clip(Wp1+buf(w-stride*2+1)-NEp1));
        MapCtxs[j++] = Clip(W+NEE-NE+p2-Clip(Wp2+buf(w-stride*2+2)-NEp2));
        MapCtxs[j++] = Clip(NN+p1-NNp1);
        MapCtxs[j++] = Clip(NN+p2-NNp2);
        MapCtxs[j++] = Clip(NN+W-NNW);
        MapCtxs[j++] = Clip(NN+W-NNW+p1-Clip(NNp1+Wp1-buf(w*2+stride+1)));
        MapCtxs[j++] = Clip(NN+W-NNW+p2-Clip(NNp2+Wp2-buf(w*2+stride+2)));
        MapCtxs[j++] = Clip(NN+NW-NNNW);
        MapCtxs[j++] = Clip(NN+NW-NNNW+p1-Clip(NNp1+NWp1-buf(w*3+stride+1)));
        MapCtxs[j++] = Clip(NN+NW-NNNW+p2-Clip(NNp2+NWp2-buf(w*3+stride+2)));
        MapCtxs[j++] = Clip(NN+NE-NNNE);
        MapCtxs[j++] = Clip(NN+NE-NNNE+p1-Clip(NNp1+NEp1-buf(w*3-stride+1)));
        MapCtxs[j++] = Clip(NN+NE-NNNE+p2-Clip(NNp2+NEp2-buf(w*3-stride+2)));
        MapCtxs[j++] = Clip(NN+NNNN-NNNNNN);
        MapCtxs[j++] = Clip(NN+NNNN-NNNNNN+p1-Clip(NNp1+buf(w*4+1)-buf(w*6+1)));
        MapCtxs[j++] = Clip(NN+NNNN-NNNNNN+p2-Clip(NNp2+buf(w*4+2)-buf(w*6+2)));
        MapCtxs[j++] = Clip(WW+p1-WWp1);
        MapCtxs[j++] = Clip(WW+p2-WWp2);
        MapCtxs[j++] = Clip(WW+WWWW-WWWWWW);
        MapCtxs[j++] = Clip(WW+WWWW-WWWWWW+p1-Clip(WWp1+buf(stride*4+1)-buf(stride*6+1)));
        MapCtxs[j++] = Clip(WW+WWWW-WWWWWW+p2-Clip(WWp2+buf(stride*4+2)-buf(stride*6+2)));
        MapCtxs[j++] = Clip(N*2-NN+p1-Clip(Np1*2-NNp1));
        MapCtxs[j++] = Clip(N*2-NN+p2-Clip(Np2*2-NNp2));
        MapCtxs[j++] = Clip(W*2-WW+p1-Clip(Wp1*2-WWp1));
        MapCtxs[j++] = Clip(W*2-WW+p2-Clip(Wp2*2-WWp2));
        MapCtxs[j++] = Clip(N*3-NN*3+NNN);
        MapCtxs[j++] = Clamp4(N*3-NN*3+NNN, W, NW, N, NE);
        MapCtxs[j++] = Clamp4(W*3-WW*3+WWW, W, NW, N, NE);
        MapCtxs[j++] = Clamp4(N*2-NN, W, NW, N, NE);
        MapCtxs[j++] = Clip((NNNNN-6*NNNN+15*NNN-20*NN+15*N+Clamp4(W*4-NWW*6+NNWWW*4-buf(w*3+4*stride), W, NW, N, NN))/6);
        MapCtxs[j++] = Clip((buf(w*3-3*stride)-4*NNEE+6*NE+Clip(W*4-NW*6+NNW*4-NNNW))/4);
        MapCtxs[j++] = Clip(((N+3*NW)/4)*3-((NNW+NNWW)/2)*3+(NNNWW*3+buf(w*3+3*stride))/4);
        MapCtxs[j++] = Clip((W*2+NW)-(WW+2*NWW)+NWWW);
        MapCtxs[j++] = (Clip(W*2-NW)+Clip(W*2-NWW)+N+NE)/4;
        MapCtxs[j++] = NNNNNN;
        MapCtxs[j++] = (NEEEE+buf(w-6*stride))/2;
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
        SCMapCtxs[j++] = W+NEE-NE+p1-Wp1-buf(w-stride*2+1)+NEp1;
        SCMapCtxs[j++] = W+NEE-NE+p2-Wp2-buf(w-stride*2+2)+NEp2;
        SCMapCtxs[j++] = N+NN-NNN;
        SCMapCtxs[j++] = N+NN-NNN+p1-Np1-NNp1+buf(w*3+1);
        SCMapCtxs[j++] = N+NN-NNN+p2-Np2-NNp2+buf(w*3+2);
        SCMapCtxs[j++] = N+NE-NNE;
        SCMapCtxs[j++] = N+NE-NNE+p1-Np1-NEp1+buf(w*2-stride+1);
        SCMapCtxs[j++] = N+NE-NNE+p2-Np2-NEp2+buf(w*2-stride+2);
        SCMapCtxs[j++] = N+NW-NNW;
        SCMapCtxs[j++] = N+NW-NNW+p1-Np1-NWp1+buf(w*2+stride+1);
        SCMapCtxs[j++] = N+NW-NNW+p2-Np2-NWp2+buf(w*2+stride+2);
        SCMapCtxs[j++] = NE+NW-NN;
        SCMapCtxs[j++] = NE+NW-NN+p1-NEp1-NWp1+NNp1;
        SCMapCtxs[j++] = NE+NW-NN+p2-NEp2-NWp2+NNp2;
        SCMapCtxs[j++] = NW+W-NWW;
        SCMapCtxs[j++] = NW+W-NWW+p1-NWp1-Wp1+buf(w+stride*2+1);
        SCMapCtxs[j++] = NW+W-NWW+p2-NWp2-Wp2+buf(w+stride*2+2);
        SCMapCtxs[j++] = W*2-WW;
        SCMapCtxs[j++] = W*2-WW+p1-Wp1*2+WWp1;
        SCMapCtxs[j++] = W*2-WW+p2-Wp2*2+WWp2;
        SCMapCtxs[j++] = N*2-NN;
        SCMapCtxs[j++] = N*2-NN+p1-Np1*2+NNp1;
        SCMapCtxs[j++] = N*2-NN+p2-Np2*2+NNp2;
        SCMapCtxs[j++] = NW*2-NNWW;
        SCMapCtxs[j++] = NW*2-NNWW+p1-NWp1*2+buf(w*2+stride*2+1);
        SCMapCtxs[j++] = NW*2-NNWW+p2-NWp2*2+buf(w*2+stride*2+2);
        SCMapCtxs[j++] = NE*2-NNEE;
        SCMapCtxs[j++] = NE*2-NNEE+p1-NEp1*2+buf(w*2-stride*2+1);
        SCMapCtxs[j++] = NE*2-NNEE+p2-NEp2*2+buf(w*2-stride*2+2);
        SCMapCtxs[j++] = N*3-NN*3+NNN+p1-Np1*3+NNp1*3-buf(w*3+1);
        SCMapCtxs[j++] = N*3-NN*3+NNN+p2-Np2*3+NNp2*3-buf(w*3+2);
        SCMapCtxs[j++] = N*3-NN*3+NNN;
        SCMapCtxs[j++] = (W+NE*2-NNE)/2;
        SCMapCtxs[j++] = (W+NE*3-NNE*3+NNNE)/2;
        SCMapCtxs[j++] = (W+NE*2-NNE)/2+p1-(Wp1+NEp1*2-buf(w*2-stride+1))/2;
        SCMapCtxs[j++] = (W+NE*2-NNE)/2+p2-(Wp2+NEp2*2-buf(w*2-stride+2))/2;
        SCMapCtxs[j++] = NNE+NE-NNNE;
        SCMapCtxs[j++] = NNE+W-NN;
        SCMapCtxs[j++] = NNW+W-NNWW;
        j = 0;
        
        for (int k=(color>0)?color-1:stride-1; j<nOLS; j++) {
            pOLS[j] = Clip(floor(ols[j][color].Predict(ols_ctxs[j])));
            ols[j][k].Update(p1);
        }

        int mean=W+NW+N+NE;
        mean>>=2;
        const int diff4=
            DiffQt(W, N, 4) << 12 | 
            DiffQt(NW, NE, 4) << 8 |
            DiffQt(NW, N, 4) << 4 |
            DiffQt(W, NE, 4);
        cm.set(hash(++i,(N+1)>>1, DiffQt(N,(NN*2-NNN))));
        cm.set(hash(++i,(W+1)>>1, DiffQt(W,(WW*2-WWW))));
        cm.set(hash(++i,Clamp4(W+N-NW,W,NW,N,NE), DiffQt((N+NE-NNE), (N+NW-NNW))));
        cm.set(hash(++i,(NNN+N+4)/8, Clip(N*3-NN*3+NNN)>>1 ));
        cm.set(hash(++i,(WWW+W+4)/8, Clip(W*3-WW*3+WWW)>>1 ));
        cm.set(hash(++i,color, (W+Clip(NE*3-NNE*3+NNNE))/4, DiffQt(N,(NW+NE)/2)));
        cm.set(hash(++i,color, Clip((-WWWW+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+NNNE*4-NNNNE,N,NE,NEE, NEEE))/5)/4));
        cm.set(hash(++i,Clip(NEE+N-NNEE), DiffQt(W,Clip(NW+NE-NNE))));
        cm.set(hash(++i,Clip(NN+W-NNW), DiffQt(W,Clip(NNW+WW-NNWW))));
        cm.set(hash(++i,color, p1));
        cm.set(hash(++i,color, p2));
        cm.set(hash(++i,color, Clip(W+N-NW)/2, Clip(W+p1-Wp1)/2));
        cm.set(hash(++i,Clip(N*2-NN)/2, DiffQt(N,(NN*2-NNN))));
        cm.set(hash(++i,Clip(W*2-WW)/2, DiffQt(W,(WW*2-WWW))));
        cm.set(hash(++i,        Clamp4(N*3-NN*3+NNN, W, NW, N, NE)/2));
        cm.set(hash(++i,        Clamp4(W*3-WW*3+WWW, W, N, NE, NEE)/2));
        cm.set(hash(++i, color, DiffQt(W,Wp1), Clamp4((p1*W)/(Wp1<1?1:Wp1),W,N,NE,NEE))); //using max(1,Wp1) results in division by zero in VC2015
        cm.set(hash(++i, color, Clamp4(N+p2-Np2,W,NW,N,NE)));
        cm.set(hash(++i, color, Clip(W+N-NW), column[0]));
        cm.set(hash(++i, color, (N*2-NN), DiffQt(W,(NW*2-NNW))));
        cm.set(hash(++i, color, (W*2-WW), DiffQt(N,(NW*2-NWW))));
        cm.set(hash(++i, (W+NEE)/2, DiffQt(W,(WW+NE)/2) ));
        cm.set(hash(++i, (Clamp4(Clip(W*2-WW)+Clip(N*2-NN)-Clip(NW*2-NNWW), W, NW, N, NE))));
        cm.set(hash(++i, color, W, p2 ));
        cm.set(hash(++i, N, NN&0x1F, NNN&0x1F ));
        cm.set(hash(++i, W, WW&0x1F, WWW&0x1F ));
        cm.set(hash(++i, color, N, column[0] ));
        cm.set(hash(++i, color, Clip(W+NEE-NE), DiffQt(W,(WW+NE-N))));
        cm.set(hash(++i,NN, NNNN&0x1F, NNNNNN&0x1F, column[1]));
        cm.set(hash(++i,WW, WWWW&0x1F, WWWWWW&0x1F, column[1]));
        cm.set(hash(++i,NNN, NNNNNN&0x1F, buf(w*9)&0x1F, column[1]));
        cm.set(hash(++i,  color,column[1]));
        
        cm.set(hash(++i, color, W, DiffQt(W,WW)));
        cm.set(hash(++i, color, W, p1));
        cm.set(hash(++i, color, W/4, DiffQt(W,p1), DiffQt(W,p2) ));
        cm.set(hash(++i, color, N, DiffQt(N,NN)));
        cm.set(hash(++i, color, N, p1));
        cm.set(hash(++i, color, N/4, DiffQt(N,p1), DiffQt(N,p2) ));
        cm.set(hash(++i, color, (W+N)>>3, p1>>4, p2>>4));
        cm.set(hash(++i, color, p1/2, p2/2));
        cm.set(hash(++i, color, W, p1-Wp1));
        cm.set(hash(++i, color, W+p1-Wp1));
        cm.set(hash(++i, color, N, p1-Np1));
        cm.set(hash(++i, color, N+p1-Np1));
        cm.set(hash(++i, color, NNNE, NNNEE));
        cm.set(hash(++i, color, NNNW, NNNWW));

        cm.set(hash(++i, mean, diff4));

        ctx[0] = (min(color,stride-1)<<9)|((abs(W-N)>3)<<8)|((W>N)<<7)|((W>NW)<<6)|((abs(N-NW)>3)<<5)|((N>NW)<<4)|((abs(N-NE)>3)<<3)|((N>NE)<<2)|((W>WW)<<1)|(N>NN);
        ctx[1] = ((DiffQt(p1,(Np1+NEp1-buf(w*2-stride+1)))>>1)<<5)|((DiffQt((N+NE-NNE),(N+NW-NNW))>>1)<<2)|min(color,stride-1);

        i=0;
        Map[i++].set((W&0xC0)|((N&0xC0)>>2)|((WW&0xC0)>>4)|(NN>>6));
        Map[i++].set((N&0xC0)|((NN&0xC0)>>2)|((NE&0xC0)>>4)|(NEE>>6));
        Map[i++].set(buf(1));
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

    U8 B=(c0<<(8-bpos));
    int i=5;

    Map[i++].set((((U8)(Clip(W+N-NW)-B))*8+bpos)|(DiffQt(Clip(N+NE-NNE), Clip(N+NW-NNW))<<11));
    Map[i++].set((((U8)(Clip(N*2-NN)-B))*8+bpos)|(DiffQt(W, Clip(NW*2-NNW))<<11));
    Map[i++].set((((U8)(Clip(W*2-WW)-B))*8+bpos)|(DiffQt(N, Clip(NW*2-NWW))<<11));
    Map[i++].set((((U8)(Clip(W+N-NW)-B))*8+bpos)|(DiffQt(p1, Clip(Wp1+Np1-NWp1))<<11));
    Map[i++].set((((U8)(Clip(W+N-NW)-B))*8+bpos)|(DiffQt(p2, Clip(Wp2+Np2-NWp2))<<11));
    Map[i++].set(hash(W-B, N-B)*8+bpos);
    Map[i++].set(hash(W-B, WW-B)*8+bpos);
    Map[i++].set(hash(N-B, NN-B)*8+bpos);
    Map[i++].set(hash(Clip(N+NE-NNE)-B, Clip(N+NW-NNW)-B)*8+bpos);
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(N+p1-Np1)-B))*8+bpos));
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(N+p2-Np2)-B))*8+bpos));
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(W+p1-Wp1)-B))*8+bpos));
    Map[i++].set((min(color, stride-1)<<11)|(((U8)(Clip(W+p2-Wp2)-B))*8+bpos));
    for (int j=0; j<n2Maps1; i++, j++)
        Map[i].set((MapCtxs[j]-B)*8+bpos);
    for (int j=0; i<n2Maps; i++, j++)
        Map[i].set((pOLS[j]-B)*8+bpos);
    for (int i=0; i<nSCMaps-1; i++)
        SCMap[i].set((SCMapCtxs[i]-B)*8+bpos);

    if (++col>=stride*8) col=0;

    cm.mix(m,1);
    for (int i=0;i<n2Maps;i++)
        Map[i].mix1(m,4,4,0-((val2-2)/3));
    for (int i=0;i<nSCMaps;i++)
        SCMap[i].mix(m,9,4,4);

    m.add(0);
    if (bpos==0) {
        mxcxt[1]=(min(63,column[0])+((ctx[0]>>3)&0xC0));
        mxcxt[2]=(min(127,column[1])+((ctx[0]>>2)&0x180));
        mxcxt[5]=min(color, stride - 1);
        int trendN = (N >= NN && NN >= NNN) || (N <= NN && NN <= NNN);
        int trendW = (W >= WW && WW >= WWW) || (W <= WW && WW <= WWW);
        int trend = trendN << 1 | trendW;
        mxcxt[8]=(hash(DiffQt(W,WW,5), DiffQt(N,NN,5), DiffQt(W,N,5), trend, color)&0x1FFF);
        mxcxt[9]=(hash(ctx[0], column[0]>>3)&0x1FFF);
        mxcxt[12]=(min(255,(x+line)>>5));
    }
   
    mxcxt[0]=(((line&7)<<5))|col;
    mxcxt[3]=((ctx[0]&0x7FC))|(bpos>>1);
    mxcxt[4]=col+((c0==((0x100|((N+W)/2))>>(8-bpos))))*32;
    mxcxt[6]=(c0 - 1) << 2 | min(color, stride - 1);
    mxcxt[7]=((ctx[1]<<2))|(bpos>>1);
    mxcxt[10]=hash(LogQt(N,5), LogMeanDiffQt(N,NN,3), c0)&0x1FFF;
    mxcxt[11]=hash(LogQt(W,5), LogMeanDiffQt(W,WW,3), c0)&0x1FFF;
    
    return 0;
}
