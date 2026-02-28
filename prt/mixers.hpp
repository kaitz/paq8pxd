#pragma once
#include "mixer1.hpp"
#include <vector>

class Mixers {
public:
    BlockData& x;
    int C;                // Count of mixers + final
    Array<Mixer1*> m;     // Array of C mixers  
    Mixer1 *mf;           // Final mixer
    int N;                // Max input prediction count
    Array<short, 32> tx;  // Predictions from models
    int nx;               // Input count
    int K;                // Final mixer input count
    Array<short, 32> pr;  // Predictions from mixers
    int cxt;              // Local dummy context=0

    Mixers(BlockData& bd, int c, int t,std::vector<mparm> &p):x(bd),C(c-1),m(C),N((t+15)&-16),tx(N),nx(0),K((C+15)&-16),pr(K),cxt(0) {
        for (int i=0; i<C; ++i) {
            m[i]=new Mixer1(tx, p[i].m, N, p[i].dmul, p[i].elim, p[i].lr, p[i].cxt, p[i].bias);
        }
        // Mixer to combine all mixers
        mf=new Mixer1(pr, p[C].m, K, p[C].dmul, p[C].elim, p[C].lr,&cxt,p[C].bias);
    }

    ~Mixers() {
        for (int i=0; i<C; ++i) {
            delete m[i];
        }
        delete mf;
    }


    void add(int x) {
        assert(nx<N);
        tx[nx++]=x;
    }

    void update() {
        for (int i=0; i<C; ++i) {
            m[i]->update(x.y);
        }
        nx=0;
        mf->update(x.y);
    }

    short p(){
        for (int i=0; i<C; ++i) {
            pr[i]= m[i]->p1();
        }
        return mf->p();
    }
};
