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
    const int K;          // Final mixer input count
    Array<short, 32> pr;  // Predictions from mixers
    const std::vector<mparm> &mp; // All mixer parameters
    int zpr;

    Mixers(BlockData& bd, const int c, const int t, const std::vector<mparm> &p):x(bd),C(c-1),m(C),mf(nullptr),
        N((t+15)&-16),tx(N+16),nx(0),K((C+15)&-16),pr(K+16),mp(p) {

        assert(C>=0);
        for (int i=0; i<C; ++i) {
            m[i]=new Mixer1(tx, p[i].m, N, p[i].dmul, p[i].elim, p[i].lr, p[i].cxt, p[i].bias, p[i].adaptive);
        }
        // Mixer to combine all mixers
        mf=new Mixer1(pr, p[C].m, K, p[C].dmul, p[C].elim, p[C].lr,p[C].cxt,p[C].bias, p[C].adaptive);
        // Disable all mixers
        reset();
    }

    ~Mixers() {
        if (mf!=nullptr) {
            //U64 tskip=0;
            //U64 tcount=0;
            for (int i=0; i<C; ++i) {
                //tskip+=m[i]->count;
                //tcount+=m[i]->tcount;
                delete m[i];
            }
            //tskip+=mf->count;
            //tcount+=mf->tcount;
            delete mf;
            mf=nullptr;
            //printf("Total mix skip %f%\n",tskip*100.0f/tcount);
        }
    }

    void add(int x) {
        assert(nx<N);
        tx[nx++]=x;
        zpr+=x==0;
    }
    
    void setErrLimit(int e, int r=28) {
        for (int i=0; i<C; ++i) {
            m[i]->ErrLimit(e,r);
        }
    }
    void reset() {
        for (size_t i=0; i<mp.size(); i++)  *mp[i].cxt=-1;
        nx=zpr=0;
    }

    void update() {
        while (nx&15) tx[nx++]=0; 
        for (int i=0; i<C; ++i) {
            m[i]->update(x.y,nx);
        }
        nx=zpr=0;
        mf->update(x.y,K);
    }

    short p(int sh0=0, int sh1=0) {
        while (nx&15) tx[nx++]=0; 
        for (int i=0; i<C; ++i) {
            pr[i]= m[i]->p1(nx,sh0);
        }
        return mf->p(K,sh1);
    }
};
