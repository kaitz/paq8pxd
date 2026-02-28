#include "normal.hpp"

normalModel1::normalModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf), N(10),
    cm(x,N,CMlimit(x.MEM()*32)),
    StateMaps{ 256, 256*256,256*256,256*256 },
    rcm9(CMlimit(x.MEM()/((x.settings.level>8?8:4))),bd),
    rcm10(CMlimit(x.MEM()/(x.settings.level>8?4:2)),bd) {
}

int normalModel1::p(Mixers& m,int val1,int val2){  
    if (x.bpos==0) {
        int i;
        if (val2==0) cm.set(x.cxt[15]); else cm.sets();
        for (i=1; i<=7; ++i)
        cm.set(x.cxt[i]);

        cm.set(x.cxt[9]);
        rcm9.set(x.cxt[10]);
        
        rcm10.set(x.cxt[12]);
        cm.set(x.cxt[14]);
    }

    rcm9.mix(m);
    rcm10.mix(m);
    m.add((stretch(StateMaps[0].p(x.c0-1,x.y)))>>2);
    m.add((stretch(StateMaps[1].p((x.c0-1)|(buf(1)<<8),x.y)))>>2);
    m.add((stretch(StateMaps[2].p((x.c0-1)|(buf(1)<<8),x.y,64)))>>2);
    m.add((stretch(StateMaps[3].p((x.c0-1)|(buf(2)<<8),x.y,64)))>>2);
    return max(0, cm.mix(m)-(N-7)); 
}
