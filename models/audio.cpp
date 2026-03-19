#include "audio.hpp"

// This model is from paq8px

//////////////////////////// wavModel /////////////////////////////////

wavModel1::wavModel1(BlockData& bd): x(bd),buf(bd.buf){
    a8mode=new audio8(bd);
    a16mode=new audio16(bd);
    // Set  larger mixer contex set                  8b   16b
    mxp.push_back( {8192,64,0,28,&mxcxt[0],0,false} ); // 4096 8192
    mxp.push_back( {4096,64,0,28,&mxcxt[1],0,false} ); // 2048 4096
    mxp.push_back( {2560,64,0,28,&mxcxt[2],0,false} ); // 1280 2560
    mxp.push_back( { 256,64,0,28,&mxcxt[3],0,false} ); //  256  256
    mxp.push_back( {  20,64,0,28,&mxcxt[4],0,false} ); //   10   20
}

int wavModel1::p(Mixers& m,int info,int val2) {
    if ((info&2)==0) {
        a8mode->audio8bModel(m, info,&mxcxt[0]);
    } else {
        a16mode->audio16bModel(m, info,&mxcxt[0]);
    }
    return 0;
} 



