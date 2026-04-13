#pragma once
#include "array.hpp"
#include "blockdata.hpp"
#include "statetable.hpp"
#include "hash.hpp"
#include "mixers.hpp"

extern short rc1[512];
extern short st1[4096];
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

class ContextMap3 {
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
    
    void updateStatePr(int i);
    int getStatePr(const int state, int i);
    void updateStates();
    void Init(int k=0,int u=1);
    U32 getStateByteLocation(const int bpos, const int c0);
    
public:
    ContextMap3(BlockData& bd, int c, uint64_t m);
    void reset();
    inline const int inputs() {
        return 3;
    }
    inline void set(U32 cx) {
        const int i=cn++;
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
    int mix(Mixers& m, int sh=0);
};
