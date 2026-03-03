#pragma once
#include "types.hpp"
#include "array.hpp"
#include "blockdata.hpp"
#include "log.hpp"
#include "logistic.hpp"
typedef __m128i XMM;
typedef __m256i YMM;

// Mixer input parameters from models
struct mparm {
    int m;
    U32 dmul;              // range 1...254
    int elim;              // range 0...32767
    int lr;                // range 1...31
    int *cxt;              // range 0...X, where X is set by user. Or -1 to skip p() and update()
    int bias;              // range -+32767
};

class Mixer1 { 
    int N, M;              // max inputs, max context
    Array<short, 32> &tx;  // external inputs 
    Array<short, 32> wx ;  // N*M weights
    int *cxt;              // S contexts
    int pr;                // last result (scaled 12 bits)
    const int dotMul;      // dot product shift scale
    const int errLimit;    // error limit
    const int lrate;       // learning rate
    int bs;                // bias
    
public:
    U64 count;
    U64 tcount;
    Mixer1(Array<short, 32> &t, int m, int n, const int dmul, const int elim, const int lr, int *context, const int bias=0):N(n),M(m),tx(t),wx(N*M),
    cxt(context), pr(2048), dotMul(dmul), errLimit(elim), lrate(lr),count(0),tcount(0) {
        assert(dmul<255);
        assert(lrate>0 && lrate<32);
        assert(errLimit<32767);
        assert(M>0);
        assert(N == ((N + 15) & -16));
        // Set bias
        for (int j=0; j<M*N; ++j) wx[j]=0+bias;
        
    }
    ~Mixer1() {
        //printf("Mix %d\n",count);
    }
    // Adjust weights to minimize coding cost of last prediction
    void __attribute__ ((noinline)) update(int y, int n) {
        if (*cxt!=-1) {
            int err=((y<<12)-pr)*lrate/4;
            if (err<-errLimit || err>errLimit)
            train(&tx[0], &wx[*cxt*N], n, err);
            else  count++;
            tcount++;
        }
    }

    // predict next bit
    int __attribute__ ((noinline)) p(int n, int sh=0) {
        assert(*cxt>=0 && *cxt<M || *cxt==-1);
        int dp=0;
        if (*cxt!=-1) {
            dp=dot_product(&tx[0], &wx[*cxt*N], n)*(dotMul>>sh)>>11;
        }
        return pr=squash(dp);
    }

    int p1(int n, int sh=0) {
        assert(*cxt>=0 && *cxt<M || *cxt==-1);
        int dp=0;
        if (*cxt!=-1) {
            dp=dot_product(&tx[0], &wx[*cxt*N], n)*(dotMul>>sh)>>11;
            if (dp<-2047) {
                dp=-2047;
            } else if (dp>2047) {
                dp=2047;
            }
        }
        pr=squash(dp);
        return dp;
    }

    int dot_product (const short* const t, const short* const w, int n) {
        assert(n == ((n + 15) & -16));
        YMM sum = _mm256_setzero_si256 ();
        while ((n -= 16) >= 0) { // Each loop sums 16 products
            YMM tmp = _mm256_madd_epi16 (*(YMM *) &t[n], *(YMM *) &w[n]);   // t[n] * w[n] + t[n+1] * w[n+1]
            tmp = _mm256_srai_epi32 (tmp, 8);                               // (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
            sum = _mm256_add_epi32 (sum, tmp);                              // sum += (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
        } 
        sum =_mm256_hadd_epi32(sum,_mm256_setzero_si256 ());             //add [1]=[1]+[2], [2]=[3]+[4], [3]=0, [4]=0, [5]=[5]+[6], [6]=[7]+[8], [7]=0, [8]=0
        sum =_mm256_hadd_epi32(sum,_mm256_setzero_si256 ());             //add [1]=[1]+[2], [2]=0,       [3]=0, [4]=0, [5]=[5]+[6], [6]=0,       [7]=0, [8]=0
        XMM lo = _mm256_extractf128_si256(sum, 0);
        XMM hi = _mm256_extractf128_si256(sum, 1);
        XMM newsum = _mm_add_epi32(lo, hi);                              //sum last two
        return _mm_cvtsi128_si32(newsum);
    }

    void train (const short* const t, short* const w, int n, const int e) {
        assert(n == ((n + 15) & -16));
        //if (e) {
        const YMM one = _mm256_set1_epi16 (1);
        const YMM err = _mm256_set1_epi16 (short(e));
        // Each iteration adjusts 16 weights
        while ((n-=16) >= 0) {
            YMM tmp = _mm256_adds_epi16 (*(YMM *) &t[n], *(YMM *) &t[n]); // t[n] * 2
            tmp = _mm256_mulhi_epi16 (tmp, err);                          //  (t[n] * 2 * err) >> 16
            tmp = _mm256_adds_epi16 (tmp, one);                           //  ((t[n] * 2 * err) >> 16) + 1
            tmp = _mm256_srai_epi16 (tmp, 1);                             //  (((t[n] * 2 * err) >> 16) + 1) >> 1
            tmp = _mm256_adds_epi16 (tmp, *(YMM *) &w[n]);                //  ((((t[n] * 2 * err) >> 16) + 1) >> 1) + w[n]
            *(YMM *) &w[n] = tmp;                                         //  save the new eight weights, bounded to +- 32K
        }
        //}
    }

    /*
// dot_product returns dot product t*w of n elements.  n is rounded
// up to a multiple of 8.  Result is scaled down by 8 bits.
int dot_product(short *t, short *w, int n) {
int sum=0;
n=(n+15)&-16;
for (int i=0; i<n; i+=2)
    sum+=(t[i]*w[i]+t[i+1]*w[i+1]) >> 8;
return sum;
}

// Train neural network weights w[n] given inputs t[n] and err.
// w[i] += t[i]*err, i=0..n-1.  t, w, err are signed 16 bits (+- 32K).
// err is scaled 16 bits (representing +- 1/2).  w[i] is clamped to +- 32K
// and rounded.  n is rounded up to a multiple of 8.

void train(short *t, short *w, int n, int err) {
n=(n+15)&-16;
for (int i=0; i<n; ++i) {
    int wt=w[i]+(((t[i]*err*2>>16)+1)>>1);
    if (wt<-32768) wt=-32768;
    if (wt>32767) wt=32767;
    w[i]=wt;
}
}
*/

};
