#include "CM128.hpp"

ContextMap3::ContextMap3(BlockData& bd, int c, uint64_t m):x(bd),C(c),cp(c),cp0(c),cxt(c),runp(c),t(m>>7),tsize(m),cxtn(c) {
    assert(C<64);
    Init();
}

inline void ContextMap3::updateStatePr(int i) {
    assert(i<C);
    assert(cxtn[i]<256);
    U32 *p=&ts[cxtn[i]], p0=p[0];
    int pr1=p0>>14;
    p0+=(x.y<<18)-pr1;
    p[0]=p0;
}
// Look for state. If found return prediction otherwise 
// add new state and return predicton
inline int ContextMap3::getStatePr(const int state, int i) {
    assert(sti<C);
    int pr=ts[state]>>20;
    for (int j=0; j<sti; j++) {
        if (cxtn[j]==state) return pr;
    }
    cxtn[sti++]=state;
    return pr;
} 
void ContextMap3::updateStates() {
    assert(sti<=C);
    for (int i=0; i<sti; ++i) {
        updateStatePr(i);
    }
    sti=0;
} 
// Construct using m bytes of memory for c contexts(c+7)&-8
void ContextMap3::Init(int k,int u){
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

void ContextMap3::reset(){
    /*memset((void*)t, 0, (tmask+1)*sizeof(E1<14,128>));
    for (int i=0; i<C; ++i) {
        cp0[i]=cp[i]=&t[0].bh[0][0];
        runp[i]=cp[i]+3;
    }*/
}

inline U32 ContextMap3::getStateByteLocation(const int bpos, const int c0) {
    U32 pis = 0; //state byte position in slot
    const U32 smask = (U32(0x31031010) >> (bpos << 2)) & 0x0F;
    pis = smask + (c0 & smask);
    return pis;
}
// Update the model with bit y, and predict next bit to mixer m.
int ContextMap3::mix(Mixers& m, int sh) {
    result=0;
    updateStates();
    for (int i=0; i<cn; ++i) {
        if ((cxtMask>>(cn-i))&1) {
            m.add(0);
            m.add(0);
            m.add(0);
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
            if (s==0) {
                m.add(0);
                m.add(0);
            } else {
                const int p1=getStatePr(s,i);
                m.add(st1[p1]>>sh);
                m.add(st32[s]);
                result++;
            }
            // predict from last byte in context
            int c0shift_bpos=(x.c0<<1)^(256>>x.bposshift);
            int b=c0shift_bpos^(runp[i][1]>>x.bposshift);
            
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

