#include "contextmap.hpp"
// Context map for large contexts.  Most modeling uses this type of context
// map.  It includes a built in RunContextMap to predict the last byte seen
// in the same context, and also bit-level contexts that map to a bit
// history state.
//
// Bit histories are stored in a hash table.  The table is organized into
// 64-byte buckets alinged on cache page boundaries.  Each bucket contains
// a hash chain of 7 elements, plus a 2 element queue (packed into 1 byte)
// of the last 2 elements accessed for LRU replacement.  Each element has
// a 2 byte checksum for detecting collisions, and an array of 7 bit history
// states indexed by the last 0 to 2 bits of context.  The buckets are indexed
// by a context ending after 0, 2, or 5 bits of the current byte.  Thus, each
// byte modeled results in 3 main memory accesses per context, with all other
// accesses to cache.
//
// On bits 0, 2 and 5, the context is updated and a new bucket is selected.
// The most recently accessed element is tried first, by comparing the
// 16 bit checksum, then the 7 elements are searched linearly.  If no match
// is found, then the element with the lowest priority among the 5 elements
// not in the LRU queue is replaced.  After a replacement, the queue is
// emptied (so that consecutive misses favor a LFU replacement policy).
// In all cases, the found/replaced element is put in the front of the queue.
//
// The priority is the state number of the first element (the one with 0
// additional bits of context).  The states are sorted by increasing n0+n1
// (number of bits seen), implementing a LFU replacement policy.
//
// When the context ends on a byte boundary (bit 0), only 3 of the 7 bit
// history states are used.  The remaining 4 bytes implement a run model
// as follows: <count:7,d:1> <b1> <unused> <unused> where <b1> is the last byte
// seen, possibly repeated.  <count:7,d:1> is a 7 bit count and a 1 bit
// flag (represented by count * 2 + d).  If d=0 then <count> = 1..127 is the
// number of repeats of <b1> and no other bytes have been seen.  If d is 1 then
// other byte values have been seen in this context prior to the last <count>
// copies of <b1>.
//
// As an optimization, the last two hash elements of each byte (representing
// contexts with 2-7 bits) are not updated until a context is seen for
// a second time.  This is indicated by <count,d> = <1,0> (2).  After update,
// <count,d> is updated to <2,0> or <1,1> (4 or 3).

// Construct using m bytes of memory for c contexts(c+7)&-8
extern int n0n1[256];

ContextMap::ContextMap(U64 m, int c,int mod): C(c),  t(m>>6), cp(C), cp0(C),
    cxt(C), runp(C), r0(C),r1(C),r0i(C),rmask(C),cn(0),model(mod) {
  assert(m>=64 && (m&m-1)==0);  // power of 2?
  assert(sizeof(E)==64);
  sm=new StateMap[C];
  for (int i=0; i<C; ++i) {
    cp0[i]=cp[i]=&t[0].bh[0][0];
    runp[i]=cp[i]+3;
  }
 // #ifndef NDEBUG 
 // printf("ContextMap t %0.2f mbytes\n",(((t.size()*sizeof(E)) +0.0)/1024)/1024);
 // #endif
}
#define CMBADLIMIT 8*32767
ContextMap::~ContextMap() {
  delete[] sm;
  
}
  void ContextMap::set() {
    set(U32(0));
  
}
// Set the i'th context to cx
  void ContextMap::set(U64 cx, int next) {
    set(finalize64(cx,32));
  
}
// Set the i'th context to cx
  void ContextMap::set(U32 cx, int next) {
    if (cn==C) cn=0; // model bypass FIXME
  int i=cn++;
  i&=next;
  assert(i>=0 && i<C);
  if (cx==0){ cxt[i]=0; rmask[i]=0;}
  else{
  cx=cx*987654323+i;  // permute (don't hash) cx to spread the distribution
  cx=cx<<16|cx>>16;
  cxt[i]=cx*123456791+i;
  rmask[i]=-1;
  }
}
// Predict to mixer m from bit history state s, using sm to map s to
// a probability.

  int ContextMap::mix2(Mixer& m, int s, StateMap& sm) {
  int p1=sm.p(s,m.x.y);
  if (s==0){
    m.add(0); m.add(0);m.add(0);m.add(0);m.add(48+16);
    return 0;
  }else{
  //int n0=-!nex(s,2);
  //int n1=-!nex(s,3);
    int st=stretch(p1)>>2;
    m.add(st);
    p1>>=4;
    int p0=255-p1;
    if (m.x.rm1)  m.add(p1-p0); else m.add(0); // ??
    //m.add(st*(n1-n0));
    //m.add((p1&n0)-(p0&n1));
    //m.add((p1&n1)-(p0&n0));
    int n01=n0n1[s];
    if (n01){
        p0=(n01<2)?255:0;
        m.add((p1)-(p0));
        if (n0n1[s]==2){ // -1
              m.add((-st));
        }else{
              m.add((st));
        }
     }else {
        m.add(0);
        m.add(0);
     }
     m.add(0);
  return s>0;
  }
}

// Update the model with bit y1, and predict next bit to mixer m.
// Context: cc=c0, bp=bpos, c1=buf(1), y1=y.
int ContextMap::mix1(Mixer& m, int cc, int bp, int c1, int y1) {
  // Update model with y
  int result=0;
  for (int i=0; i<cn; ++i) {
    if (cp[i]) {
      assert(cp[i]>=&t[0].bh[0][0] && cp[i]<=&t[t.size()-1].bh[6][6]);
      assert(((long long)(cp[i])&63)>=15);
      int ns=nex(*cp[i], y1);
      if (ns>=204 && rnd() << ((452-ns)>>3)) ns-=4;  // probabilistic increment
      *cp[i]=ns;
    }

    // Update context pointers
    if (m.x.bpos>1 && runp[i][0]==0)
    {
     cp[i]=0;
    }
    else
    {
     U16 chksum=cxt[i]>>16;
     U64 tmask=t.size()-1;
     switch(m.x.bpos)
     {
      case 1: case 3: case 6: cp[i]=cp0[i]+1+(cc&1); break;
      case 4: case 7: cp[i]=cp0[i]+3+(cc&3); break;
      case 2: case 5: cp0[i]=cp[i]=t[(cxt[i]+cc)&tmask].get(chksum); break;
      default:
      {
       cp0[i]=cp[i]=t[(cxt[i]+cc)&tmask].get(chksum);
       // Update pending bit histories for bits 2-7
       if (cp0[i][3]==2) {
         const int c=cp0[i][4]+256;
         U8 *p=t[(cxt[i]+(c>>6))&tmask].get(chksum);
         p[0]=1+((c>>5)&1);
         p[1+((c>>5)&1)]=1+((c>>4)&1);
         p[3+((c>>4)&3)]=1+((c>>3)&1);
         p=t[(cxt[i]+(c>>3))&tmask].get(chksum);
         p[0]=1+((c>>2)&1);
         p[1+((c>>2)&1)]=1+((c>>1)&1);
         p[3+((c>>1)&3)]=1+(c&1);
         cp0[i][6]=0;
       }
       // Update run count of previous context
       if (runp[i][0]==0)  // new context
         runp[i][0]=2, runp[i][1]=c1;
       else if (runp[i][1]!=c1)  // different byte in context
         runp[i][0]=1, runp[i][1]=c1;
       else if (runp[i][0]<254)  // same byte in context
         runp[i][0]+=2;
       else if (runp[i][0]==255)
         runp[i][0]=128;
       runp[i]=cp0[i]+3;
      } break;
     }
    }
   }
  
    
    for (int i=0; i<cn; ++i) {
       if(cxt[i]){
            // predict from bit context
            int s = 0;
            if (cp[i]) s = *cp[i];
            if (s>0) result++;
            mix2(m, s, sm[i]);
       }else{
            for (int i=0; i<(inputs()-1); i++)
                m.add(0);     
       }
   }
 

    // predict from last byte in context
     
     for (int i=0; i<cn; ++i) {
         U8 a=runp[i][0];
         U8 b=runp[i][1];
         r0[i]=a;
         r1[i]=b;
         r0i[i]=ilog(a+1);
     }

       
    for (int i=0; i<cn; ++i) {
        if (rmask[i] && ((r1[i  ]+256)>>(8-bp)==cc)) {
            m.add(((r1[i  ]>>(7-bp)&1)*2-1) *((r0i[i  ])<<(2+(~r0[i  ]&1)))); }
        else   m.add(0);
      }

   
  if (bp==7) cn=0;
  return result;
}

int ContextMap::inputs() {
    return 6;
}


