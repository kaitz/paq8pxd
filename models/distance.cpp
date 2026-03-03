#include "distance.hpp"
//////////////////////////// distanceModel ///////////////////////

// Model for modelling distances between symbols

distanceModel1::distanceModel1(BlockData& bd):pos00(0),pos20(0),posnl(0), x(bd),buf(bd.buf),Maps{ {8}, {8}, {8} } {
    // Set model mixer contexts and parameters
    mxp.push_back( {256,64,0,28,&mxcxt[0],0} );
    mxp.push_back( {256,64,0,28,&mxcxt[1],0} );
    mxp.push_back( {256,64,0,28,&mxcxt[2],0} );
}

int distanceModel1::p(Mixers& m, int val1, int val2) {
    if (x.bpos==0) {
        int c=x.c4&0xff;
        if (c==0x00) pos00=x.buf.pos;
        if (c==0x20) pos20=x.buf.pos;
        if (c==0xff||c=='\r'||c=='\n') posnl=x.buf.pos;
        Maps[0].set(min(llog(buf.pos-pos00),255) );
        Maps[1].set(min(llog(buf.pos-pos20),255) );
        Maps[2].set(min(llog(buf.pos-posnl),255) );
    }
    mxcxt[0]=min(llog(buf.pos-pos00),255);
    mxcxt[1]=min(llog(buf.pos-pos20),255);
    mxcxt[2]=min(llog(buf.pos-posnl),255);
    Maps[0].mix(m );
    Maps[1].mix(m);
    Maps[2].mix(m);
    return 0;
}
