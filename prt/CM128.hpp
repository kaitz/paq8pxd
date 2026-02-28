#pragma once
#include "array.hpp"
#include "blockdata.hpp"
#include "statetable.hpp"
#include "hash.hpp"
//#include "mixer.hpp"
#include "mixers.hpp"

extern short rc1[512];
extern short st1[4096];
//extern short st2[4096];
extern short st32[256];

template <const int A, const int B> // Warning: values 3, 7 for A are the only valid parameters
union  E1 {  // hash element, 64 bytes
    struct{ // this is bad uc
        U16 chk[A];  // byte context checksums
        U8 last;     // last 2 accesses (0-6) in low, high nibble
        U8 bh[A][7]; // byte context, 3-bit context -> bit history state
        // bh[][0] = 1st bit, bh[][1,2] = 2nd bit, bh[][3..6] = 3rd bit
        // bh[][0] is also a replacement priority, 0 = empty
        //  U8* get(U16 chk);  // Find element (0-6) matching checksum.
        // If not found, insert or replace lowest priority (not last).
    };
    U8 pad[B] ;
    __attribute__ ((noinline)) U8* get(U16 ch,int keep) {

        if (chk[last&15]==ch) return &bh[last&15][0];
        int b=0xffff, bi=0;

        for (int i=0; i<A; ++i) {
            if (chk[i]==ch) return last=last<<4|i, (U8*)&bh[i][0];
            int pri=bh[i][0];
            if (pri<b && (last&15)!=i && last>>4!=i) b=pri, bi=i;
        }
        return last=last<<4|bi|keep, chk[bi]=ch, (U8*)memset(&bh[bi][0], 0, 7);
    }
};

struct ContextMap3 {
    BlockData& x;
    int C;           // max number of contexts
    Array<U8*> cp;   // C pointers to current bit history
    Array<U8*> cp0;  // First element of 7 element array containing cp[i]
    Array<U32> cxt;  // C whole byte contexts (hashes)
    Array<U8*> runp; // C [0..3] = count, value, unused, unused
    int cn;          // Next context to set by set()
    int result;
    int kep;
    Array<E1<14,128>, 64> t;
    U32 tmask,tsize;
    int skip2;
    uint64_t cxtMask;
    // State
    Array<int> cxtn;   // Unique states set
    U32 ts[256];       // Statemap
    int sti;           // Count of set states
    U32 m;

    ContextMap3(BlockData& bd, int c, uint64_t m):x(bd),C(c),cp(c),cp0(c),cxt(c),runp(c),t(m>>7),tsize(m),cxtn(c) {
        assert(C<64);
        Init();
    }
    /*inline int pre(const int state) {
        assert(state>=0 && state<256);
        U32 n0=nex(state, 2)*3+1;
        U32 n1=nex(state, 3)*3+1;
        return (n1<<12) / (n0+n1);
    }*/
    void updateStatePr(int i) {
        assert(i<C);
        assert(cxtn[i]<256);
        U32 *p=&ts[cxtn[i]], p0=p[0];
        int pr1=p0>>14;
        p0+=(x.y<<18)-pr1;
        p[0]=p0;
    }
    // Look for state. If found return prediction otherwise 
    // add new state and return predicton
    int getStatePr(const int state, int i) {
        assert(sti<C);
        int pr=ts[state]>>20;
        for (int j=0; j<sti; j++) {
            if (cxtn[j]==state) return pr;
        }
        cxtn[sti++]=state;
        return pr;
    } 
    void updateStates() {
        assert(sti<=C);
        for (int i=0; i<sti; ++i) {
            updateStatePr(i);
        }
        sti=0;
    } 
    // Construct using m bytes of memory for c contexts(c+7)&-8
    void __attribute__ ((noinline)) Init(int k=0,int u=1){
        tmask=((tsize>>7)-1);
        cn=0;
        cxtMask=((1<C)-1)*2; // Inital zero contexts
        result=0;
        kep=0xf0;
        assert(tsize>=64 && (tsize&tsize-1)==0);  // power of 2?
        assert(sizeof(E1<14,128>)==128);
        // Init state predictions
        for (int i=0; i<256; ++i){
            U32 n0=nex(i, 2)*3+1;
            U32 n1=nex(i, 3)*3+1;
            ts[i]=(((n1<<20) / (n0+n1)) << 12);
        }
        for (int i=0; i<C; ++i)cxtn[i]=0;
        sti=0;
        //cm
        for (int i=0; i<C; ++i) {
            cp0[i]=cp[i]=&t[0].bh[0][0];
            runp[i]=cp[i]+3;
        }
    }

    void reset(){
        /*memset((void*)t, 0, (tmask+1)*sizeof(E1<14,128>));
    for (int i=0; i<C; ++i) {
        cp0[i]=cp[i]=&t[0].bh[0][0];
        runp[i]=cp[i]+3;
    }*/
    }
    int inputs() {
        return 3;
    }
    // Set the i'th context to cx
    inline void set(U32 cx) {
        const int i=cn++;
       /* if ((i>=C )){
            printf("FG");
        }*/
        assert(i>=0 && i<C);
        cx=cx*987654323+i;  // permute (don't hash) cx to spread the distribution
        cx=cx<<16|cx>>16;
        cxt[i]=cx*123456791+i;
        cxtMask=cxtMask*2;
    }

    inline void sets() {
        const int i=cn++;
        assert(i>=0 && i<C);
        cxtMask++;
        cxtMask=cxtMask*2;
    }
    // Predict to mixer m from bit history state s, using sm to map s to
    // a probability.
   /* inline int mix3(Mixer& m, const int s, int i) {
        if (s==0) {
            m.add(0);
            m.add(0);
            return 0;
        } else {
            const int p1=getStatePr(s,i);
            m.add(st1[p1]);
            m.add(st32[s]);
            return 1;
        }
    }*/
    inline int mix3(Mixers& m, const int s, int i) {
        if (s==0) {
            m.add(0);
            m.add(0);
            return 0;
        } else {
            const int p1=getStatePr(s,i);
            m.add(st1[p1]);
            m.add(st32[s]);
            return 1;
        }
    }

    // Zero prediction
   /* inline void mix4(Mixer& m) {
        m.add(0);
        m.add(0);
        m.add(0);
    }*/
    inline void mix4(Mixers& m) {
        m.add(0);
        m.add(0);
        m.add(0);
    }

    inline U32 getStateByteLocation(const int bpos, const int c0) {
        U32 pis = 0; //state byte position in slot
        const U32 smask = (U32(0x31031010) >> (bpos << 2)) & 0x0F;
        pis = smask + (c0 & smask);
        return pis;
    }
    // Update the model with bit y1, and predict next bit to mixer m.
    // Context: cc=c0, bp=bpos, c1=buf(1), y1=y.
    /*int __attribute__ ((noinline)) mix(Mixer& m) {
        // Update model with y
        result=0;
        updateStates();
        for (int i=0; i<cn; ++i) {
            if ((cxtMask>>(cn-i))&1) {
                mix4(m);
            } else {
                if (cp[i]) {
                    assert(cp[i]>=&t[0].bh[0][0] && cp[i]<=&t[tmask].bh[14][6]);
                    //assert(((long long)(cp[i])&127)>=29);
                    *cp[i]=nex(*cp[i], x.y);
                }

                // Update context pointers
                int s=0;
                if (x.bpos>1 && runp[i][0]==0) {
                    cp[i]=0;
                } else {
                    const U16 chksum=(cxt[i]>>16)^i;
                    
                    if (x.bpos) {
                        if (x.bpos==2 || x.bpos==5)cp0[i]=cp[i]=t[(cxt[i]+x.c0)&tmask].get(chksum,kep);
                        else cp[i]=cp0[i]+getStateByteLocation(x.bpos,x.c0);
                    } else {// default
                        cp0[i]=cp[i]=t[(cxt[i]+x.c0)&tmask].get(chksum,kep);
                        // Update pending bit histories for bits 2-7
                        if (cp0[i][3]==2) {
                            const int c=cp0[i][4]+256;
                            U8 *p=t[(cxt[i]+(c>>6))&tmask].get(chksum,kep);
                            p[0]=1+((c>>5)&1);
                            p[1+((c>>5)&1)]=1+((c>>4)&1);
                            p[3+((c>>4)&3)]=1+((c>>3)&1);
                            p=t[(cxt[i]+(c>>3))&tmask].get(chksum,kep);
                            p[0]=1+((c>>2)&1);
                            p[1+((c>>2)&1)]=1+((c>>1)&1);
                            p[3+((c>>1)&3)]=1+(c&1);
                            cp0[i][6]=0;
                        }
                        const U8 c1=x.c4;
                        // Update run count of previous context
                        if (runp[i][0]==0)  // new context
                        runp[i][0]=2, runp[i][1]=c1;
                        else if (runp[i][1]!=c1)  // different byte in context
                        runp[i][0]=1, runp[i][1]=c1;
                        else if (runp[i][0]<254)  // same byte in context
                        runp[i][0]+=2;
                        runp[i]=cp0[i]+3;
                    }
                    s=*cp[i];
                }
                // predict from bit context

                result=result+mix3(m, s, i);
                // predict from last byte in context
                int bposshift=7-x.bpos;
                int c0shift_bpos=(x.c0<<1)^(256>>(bposshift));
                int b=c0shift_bpos^(runp[i][1]>>bposshift);
                
                if (b<=1) {
                    b=b*256;   // predicted bit + for 1, - for 0
                    // count*2, +1 if 2 different bytes seen
                    m.add(rc1[runp[i][0]+b]);
                }
                else
                m.add(0);
            }
        }
        if (x.bpos==7) {
            assert(cn==0 || cn==C);
            cn=cxtMask=0;  
        } 

        return result;
    }*/
    int __attribute__ ((noinline)) mix(Mixers& m) {
        // Update model with y
        result=0;
        updateStates();
        for (int i=0; i<cn; ++i) {
            if ((cxtMask>>(cn-i))&1) {
                mix4(m);
            } else {
                if (cp[i]) {
                    assert(cp[i]>=&t[0].bh[0][0] && cp[i]<=&t[tmask].bh[14][6]);
                    //assert(((long long)(cp[i])&127)>=29);
                    *cp[i]=nex(*cp[i], x.y);
                }

                // Update context pointers
                int s=0;
                if (x.bpos>1 && runp[i][0]==0) {
                    cp[i]=0;
                } else {
                    const U16 chksum=(cxt[i]>>16)^i;
                    
                    if (x.bpos) {
                        if (x.bpos==2 || x.bpos==5)cp0[i]=cp[i]=t[(cxt[i]+x.c0)&tmask].get(chksum,kep);
                        else cp[i]=cp0[i]+getStateByteLocation(x.bpos,x.c0);
                    } else {// default
                        cp0[i]=cp[i]=t[(cxt[i]+x.c0)&tmask].get(chksum,kep);
                        // Update pending bit histories for bits 2-7
                        if (cp0[i][3]==2) {
                            const int c=cp0[i][4]+256;
                            U8 *p=t[(cxt[i]+(c>>6))&tmask].get(chksum,kep);
                            p[0]=1+((c>>5)&1);
                            p[1+((c>>5)&1)]=1+((c>>4)&1);
                            p[3+((c>>4)&3)]=1+((c>>3)&1);
                            p=t[(cxt[i]+(c>>3))&tmask].get(chksum,kep);
                            p[0]=1+((c>>2)&1);
                            p[1+((c>>2)&1)]=1+((c>>1)&1);
                            p[3+((c>>1)&3)]=1+(c&1);
                            cp0[i][6]=0;
                        }
                        const U8 c1=x.c4;
                        // Update run count of previous context
                        if (runp[i][0]==0)  // new context
                        runp[i][0]=2, runp[i][1]=c1;
                        else if (runp[i][1]!=c1)  // different byte in context
                        runp[i][0]=1, runp[i][1]=c1;
                        else if (runp[i][0]<254)  // same byte in context
                        runp[i][0]+=2;
                        runp[i]=cp0[i]+3;
                    }
                    s=*cp[i];
                }
                // predict from bit context

                result=result+mix3(m, s, i);
                // predict from last byte in context
                int bposshift=7-x.bpos;
                int c0shift_bpos=(x.c0<<1)^(256>>(bposshift));
                int b=c0shift_bpos^(runp[i][1]>>bposshift);
                
                if (b<=1) {
                    b=b*256;   // predicted bit + for 1, - for 0
                    // count*2, +1 if 2 different bytes seen
                    m.add(rc1[runp[i][0]+b]);
                }
                else
                m.add(0);
            }
        }
        if (x.bpos==7) {
            assert(cn==0 || cn==C);
            cn=cxtMask=0;  
        } 

        return result;
    }
};
