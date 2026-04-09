#include "im4bit.hpp"

//////////////////////////// im4bitModel /////////////////////////////////
// Model for 4-bit image data

im4bitModel1::im4bitModel1( BlockData& bd, U32 val): x(bd),buf(bd.buf),
    t(CMlimit((x.settings.level>14?x.MEM()/2:x.MEM())/4) ),
    S(14+1+1+1+1+1+1),cp(S),cxt(S),map(16), WWW(0), WW(0), W(0), NWW(0), NW(0), N(0), NE(0),
    NEE(0), NNWW(0), NNW(0), NN(0), NNE(0), NNEE(0),col(0), line(0), run(0), runN(0), prevColor(0), px(0),
    mapL({
      { 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},
      { 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},
      { 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2}
    }),nPrd(5) {

    sm=new StateMap[S];
    for (int i=0;i<S;i++)
        cp[i]=t[hash(i,0)];
    
    // Set model mixer contexts and parameters
    mxp.push_back( { 256,64,0,28,&mxcxt[0],0,false} );
    mxp.push_back( { 512,64,0,28,&mxcxt[1],0,false} );
    mxp.push_back( { 512,64,0,28,&mxcxt[2],0,false} );
    mxp.push_back( {1024,64,0,28,&mxcxt[3],0,false} );
    mxp.push_back( { 512,64,0,28,&mxcxt[4],0,false} );
    mxp.push_back( { 256,64,0,28,&mxcxt[5],0,false} );
    mxp.push_back( { 256,64,0,28,&mxcxt[6],0,false} );
}

int im4bitModel1::p(Mixers& m, int w, int val2)  {
    
    for (int i=0;i<S;i++)
        *cp[i]=nex(*cp[i],x.y);

    if (x.bpos==0 || x.bpos==4) {
        WWW=WW, WW=W, NWW=NW, NW=N, N=NE, NE=NEE, NNWW=NWW, NNW=NN, NN=NNE, NNE=NNEE;
        if (x.bpos==0) {
            W=x.c4&0xF,NNN=buf(w*2)>>4, NEE=buf(w-1)>>4, NNEE=buf(w*2-1)>>4;
        } else {
            W=x.c0&0xF,NNN=buf(w*2)&0xF, NEE=buf(w-1)&0xF, NNEE=buf(w*2-1)&0xF;
        }
        run=(W!=WW || col==0)?(prevColor=WW,0):min(0xFFF,run+1);
        runN=(N!=NW || col==0)?0:min(0xFFF,runN+1);

        x.Image.pixels.W = W;
        x.Image.pixels.N = N;
        x.Image.pixels.NN = NN;
        x.Image.pixels.WW = WW;
        x.Image.ctx=
         ((abs(W-N)>7)<<8)|
         ((W>N)<<7)|
         ((W>NW)<<6)|
         ((abs(N-NW)>7)<<5)|
         ((N>NW)<<4)|
         ((abs(N-NE)>7)<<3)|
         ((N>NE)<<2)|
         ((W>WW)<<1)|
         (N>NN);

        px=1;
        int i=0;
        cxt[i++]=hash(W,NW,N);
        cxt[i++]=hash(N, min(0xFFF, col/8));
        cxt[i++]=hash(W,NW,N,NN,NE);
        cxt[i++]=hash(W, N, NE+NNE*16, NEE+NNEE*16);
        cxt[i++]=hash(W, N, NW+NNW*16, NWW+NNWW*16);
        cxt[i++]=hash(W, ilog2(run+1), prevColor, col/max(1,w/2));
        cxt[i++]=hash(NE, min(0x3FF, (col+line)/max(1,w*8)));
        cxt[i++]=hash(NW, (col-line)/max(1,w*8));
        cxt[i++]=hash(WW*16+W,NN*16+N,NNWW*16+NW);
        cxt[i++]=hash(NN, NNEE, NNWW);
        cxt[i++]=hash(i,N,NN);
        cxt[i++]=hash(i,W,WW);
        cxt[i++]=hash(i,W,NE);
        cxt[i++]=hash(i,WW,NN,NEE);
        cxt[i++]=hash(i,N,NWW,NEE);
        cxt[i++]=hash(i,NE,NNE);
        cxt[i++]=hash(i,NE,NEE);
        cxt[i++]=hash(i,W*3-WW*3+WWW,NW,N);
        cxt[i++]=hash(i,x.Image.ctx,W);
        cxt[i++]=hash(i,0);

        for (int i=0; i<S; i++) {
            mapL[i].set(cxt[i]);
            cp[i]=t[cxt[i]];
        }
        col++;
        if (col==w*2) {
            col=0; line++;
        }

        i=0;
        prd[i++]=Clip4(W*2-NE);
        prd[i++]=Clip4(W*2-NW);
        prd[i++]=Clip4(W*2-N);
        prd[i++]=Clip4(W*2-WW);
        prd[i++]=Clip4(W*3-WW*3+WWW>>1);
        prd[i++]=Clip4(N*3-NN*3+NNN>>1);
    } else {
        px+=px+x.y;
        int j=(x.y+1)<<(x.bpos&3);
        for (int i=0; i<S; i++)
            cp[i]+=j;
    }
    x.Image.pixels.px=px;
    for (int i=0; i<S; i++) mapL[i].mix(m);
    // predict
    for (int i=0; i<S; i++) {
        const U8 s=*cp[i];
        const int n0=-!nex(s, 2), n1=-!nex(s, 3);
        const int p1=sm[i].p3(s,x.y,2);
        const int st=stretch(p1);
        m.add(st);
        m.add(clp(st*abs(n1-n0))); 
    }
    m.add(stretch(map.p3(px,x.y,1)));

    const U8 B=px<<(4-(x.bpos&3));
    for (int i=0; i<nPrd; i++) {
        sMap[i].set((prd[i]-B)*4+(x.bpos&3));
        sMap[i].mix(m, 8, 1, 1);
    }
    mxcxt[0]=W*16+px;
    mxcxt[1]=min(31,col/max(1,w/16))+N*32;
    mxcxt[2]=(x.bpos&3)+4*W+64*min(7,ilog2(run+1));  // current line
    mxcxt[3]=W+NE*16+(x.bpos&3)*256;
    mxcxt[4]=(x.bpos&3)+4*N+64*min(7,ilog2(runN+1)); // above line
    mxcxt[5]=x.Image.ctx;
    mxcxt[6]=NEE*16+N;
    return 0;
}

