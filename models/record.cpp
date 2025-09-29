#include "record.hpp"

extern U8 level;
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}
//////////////////////////// recordModel ///////////////////////

// Model 2-D data with fixed record length.  Also order 1-2 models
// that include the distance to the last match.

  recordModel1::recordModel1( BlockData& bd,U64 msize ): x(bd),buf(bd.buf),  cpos1(256) , cpos2(256),
    cpos3(256), cpos4(256),wpos1(0x10000), rlen(3), rcount(2),padding(0),prevTransition(0),nTransition(0), col(0),mxCtx(0),
    x1(0),MayBeImg24b(false),cm(32768, 3,M_RECORD), cn(32768/2, 3,M_RECORD),cq(32768*4, 3,M_RECORD), co(32768*2, 3,M_RECORD), cp(level>9?0x10000000 :CMlimit(msize*2), 16,M_RECORD), nMaps ( 6),
    N(0), NN(0), NNN(0), NNNN(0),WxNW(0), nIndCtxs(5){
        // run length and 2 candidates
        rlen[0] = 2; 
        rlen[1] = 0; 
        rlen[2] = 0; 
        // candidate counts
        rcount[0] = 0;
        rcount[1] = 0; 
   }

  int recordModel1::p(Mixer& m,int rrlen,int val2) {
   // Find record length
  if (!x.bpos) {
   int w=x.c4&0xffff, c=w&255, d=w>>8,c1=x.b2,w1=(x.b3<<8)+x.b2,  e=x.c4&0xffffff;
   bool dict=x.dictonline;
    if (rrlen==0){
        int r=0;
        r=buf.pos-cpos1[c];
      if ( r>1) {
        if ( r==cpos1[c]-cpos2[c]  && r==cpos2[c]-cpos3[c] && (r>32 || r==cpos3[c]-cpos4[c])
          && (r>10 || ((c==buf(r*5+1)) && c==buf(r*6+1)))) {      
          if (r==rlen[1]) ++rcount[0];
          else if (r==rlen[2]) ++rcount[1];
          else if (rcount[0]>rcount[1]) rlen[2]=r, rcount[1]=1;
          else rlen[1]=r, rcount[0]=1;
        }
      }    

    // check candidate lengths
    for (int i=0; i < 2; i++) {
      if (rcount[i] > max(0,12-(int)ilog2(rlen[i+1]))){
        if (rlen[0] != rlen[i+1]){
            if (MayBeImg24b && rlen[i+1]==3){
              rcount[0]>>=1;
              rcount[1]>>=1;
              continue;
            }
            else if ( (rlen[i+1] > rlen[0]) && (rlen[i+1] % rlen[0] == 0) ){
            // maybe we found a multiple of the real record size..?
            // in that case, it is probably an immediate multiple (2x).
            // that is probably more likely the bigger the length, so
            // check for small lengths too
            if ((rlen[0] > 32) && (rlen[i+1] == rlen[0]*2)){
              rcount[0]>>=1;
              rcount[1]>>=1;
              continue;
            }
          }
          rlen[0] = rlen[i+1];
          rcount[i] = 0;
          MayBeImg24b = (rlen[0]>30 && (rlen[0]%3)==0);
          nTransition = 0;
        }
        else
          // we found the same length again, that's positive reinforcement that
          // this really is the correct record size, so give it a little boost
          rcount[i]>>=2;

        // if the other candidate record length is orders of
        // magnitude larger, it will probably never have enough time
        // to increase its counter before it's reset again. and if
        // this length is not a multiple of the other, than it might
        // really be worthwhile to investigate it, so we won't set its
        // counter to 0
        if (rlen[i+1]<<4 > rlen[1+(i^1)])
          rcount[i^1] = 0;
      }
    }
    }
    else rlen[0]=rrlen,rcount[0]=rcount[1]=0;
    
    assert(rlen[0]>0);
    //if (dict==true)
    //col=x.bufn.pos%x.wcol;
    //else 
    col=buf.pos%rlen[0];
    x1 = min(0x1F,col/max(1,rlen[0]/32));
    if (dict==true){
        N = x.bufn(x.wcol&0xffff), NN = x.bufn((x.wcol*2)&0xffff), NNN = x.bufn((x.wcol*3)&0xffff), NNNN = x.bufn((x.wcol*4)&0xffff);
    for (int i=0; i<nIndCtxs-1; iCtx[i]+=c, i++);
    iCtx[0]=(c<<8)|N;
    iCtx[1]=((x.wcol>1?x.bufn((x.wcol-1)&0xffff):0)<<8)|N;
    iCtx[2]=(c<<8)|(x.wcol>1?x.bufn((x.wcol-1)&0xffff):0);
    iCtx[3]=(hash(c, N, x.bufn((x.wcol+1)&0xffff)));
    }else{
    
    N = buf(rlen[0]), NN = buf(rlen[0]*2), NNN = buf(rlen[0]*3), NNNN = buf(rlen[0]*4);
    for (int i=0; i<nIndCtxs-1; iCtx[i]+=c, i++);
    iCtx[0]=(c<<8)|N;
    iCtx[1]=(buf(rlen[0]-1)<<8)|N;
    iCtx[2]=(c<<8)|buf(rlen[0]-1);
    iCtx[3]=(hash(c, N, buf(rlen[0]+1)));
    }
    /*
    Consider record structures that include fixed-length strings.
    These usually contain the text followed by either spaces or 0's,
    depending on whether they're to be trimmed or they're null-terminated.
    That means we can guess the length of the string field by looking
    for small repetitions of one of these padding bytes followed by a
    different byte. By storing the last position where this transition
    ocurred, and what was the padding byte, we are able to model these
    runs of padding bytes.
    Special care is taken to skip record structures of less than 9 bytes,
    since those may be little-endian 64 bit integers. If they contain
    relatively low values (<2^40), we may consistently get runs of 3 or
    even more 0's at the end of each record, and so we could assume that
    to be the general case. But with integers, we can't be reasonably sure
    that a number won't have 3 or more 0's just before a final non-zero MSB.
    And with such simple structures, there's probably no need to be fancy
    anyway
    */
    if (!col)
      nTransition = 0;
    if ((((U16)(x.c4>>8) == (U16)(SPACE*0x010101)) && (c != SPACE)) || (!(x.c4>>8) && c && ((padding != SPACE) || ((dict==true)?x.bufn.pos:buf.pos-prevTransition > rlen[0])))){
      prevTransition = (dict==true)?x.bufn.pos:buf.pos;
      nTransition+=(nTransition<31);
      padding = (U8)d;
    }

    U64 i=0;
    if (val2!=-1) {
   
    // Set 2 dimensional contexts
    cm.set(hash(++i,c<<8| (min(255, (dict==true)?x.bufn.pos:buf.pos-cpos1[c])/4)));
    if (!( x.filetype==DECA))
    cm.set(hash(++i,w<<9| llog((dict==true)?x.bufn.pos:buf.pos-wpos1[w])>>2));
    cm.set(hash(++i,rlen[0]|N<<10|NN<<18));

    cn.set(hash(++i,w|rlen[0]<<8));
    cn.set(hash(++i,d|rlen[0]<<16));
    cn.set(hash(++i,c|rlen[0]<<8));

    cq.set(hash(++i,w1<<3));
    cq.set(hash(++i,c1<<19));
    cq.set(hash(++i,e ));

    co.set(hash(++i,buf(1)<<8|min(255, (dict==true)?x.bufn.pos:buf.pos-cpos1[buf(1)])));
    co.set(hash(++i,buf(1)<<17|buf(2)<<9));
    co.set(hash(++i,buf(1)<<8|N));
    
    cp.set(hash(++i,rlen[0]|N<<10|col<<18,x.wcol));
    cp.set(hash(++i,rlen[0]|buf(1)<<10|col<<18,x.wcol));
    cp.set(hash(++i,col|rlen[0]<<12,x.wcol));
    if (rlen[0]>8){
      cp.set( hash(++i, min(min(0xFF,rlen[0]),(dict==true)?x.bufn.pos:buf.pos-prevTransition), min(0x3FF,col), (w&0xF0F0)|(w==((padding<<8)|padding)), nTransition ) );
      cp.set( hash(++i, w, ((dict==true)?x.bufn((x.wcol+1)&0xffff):buf(rlen[0]+1)==padding && N==padding), col/max(1,rlen[0]/32) ) );
    }
    else
      cp.set(), cp.set();

    cp.set(hash(++i, N|((NN&0xF0)<<4)|((NNN&0xE0)<<7)|((NNNN&0xE0)<<10)|((col/max(1,rlen[0]/16))<<18) ));
    cp.set(hash(++i, (N&0xF8)|((NN&0xF8)<<8)|(col<<16),x.wcol ));
    cp.set(hash(++i, N, NN));

    cp.set(hash(++i, col, x.wcol,iCtx[0]()));
    cp.set(hash(++i, col, x.wcol,iCtx[1]()));
    cp.set(hash(++i, col, x.wcol,iCtx[0]()&0xFF, iCtx[1]()&0xFF));

    cp.set(hash(++i, iCtx[2]()));  
    cp.set(hash(++i, iCtx[3]()));
    cp.set(hash(++i, iCtx[1]()&0xFF, iCtx[3]()&0xFF));

    cp.set(hash(++i, N, (WxNW=c^(dict==true)?x.bufn((x.wcol+1)&0xffff):buf(rlen[0]+1))));
    cp.set(hash(++i,(x.Match.length3!= 0) << 8U | x.Match.byte,U8(iCtx[1]()), N, WxNW));//cp.set(hash(++i, (min(ilog2(x.Match.length),3)!=0)<<8 |x.Match.byte,U8(iCtx[1]()), N, WxNW));
    int k=0x300;
    if (MayBeImg24b)
      i = (col%3)<<8, Maps[0].set(Clip(((U8)(x.c4>>16))+c-(x.c4>>24))|k);
    else
      Maps[0].set(Clip(c*2-d)|k);
    Maps[1].set(Clip(c+N-(dict==true)?x.bufn((x.wcol+1)&0xffff):buf(rlen[0]+1))|k);
    Maps[2].set(Clip(N+NN-NNN));
    Maps[3].set(Clip(N*2-NN));
    Maps[4].set(Clip(N*3-NN*3+NNN));
    iMap[0].set(N+NN-NNN);
    iMap[1].set(N*2-NN);
    iMap[2].set(N*3-NN*3+NNN);
     }
    // update last context positions
    cpos4[c]=cpos3[c];
    cpos3[c]=cpos2[c];
    cpos2[c]=cpos1[c];
    /*if (dict==true){
    cpos1[c]=x.bufn.pos;
    wpos1[w]=x.bufn.pos;
    }else*/
    {cpos1[c]=buf.pos;
    wpos1[w]=buf.pos;
    }
    mxCtx = (rlen[0]>128)?(min(0x7F,col/max(1,rlen[0]/128))):col;
  }
  U8 B = x.c0<<(8-x.bpos);
  U32 ctx = (N^B)|(x.bpos<<8);
  iCtx[nIndCtxs-1]+=x.y, iCtx[nIndCtxs-1]=ctx;
  Maps[nMaps-1].set(ctx);
  sMap[0].set(ctx);
  sMap[1].set(iCtx[nIndCtxs-1]());
  sMap[2].set((ctx<<8)|WxNW);
if (val2==-1) return rlen[0];
  cm.mix(m);
  cn.mix(m);
  cq.mix(m);
  co.mix(m);
  cp.mix(m);
  if (x.filetype==DICTTXT || x.filetype==BIGTEXT  || x.filetype==TEXT || x.filetype==TEXT0  ||x.filetype==EXE|| x.filetype==DECA || x.filetype==DBASE) {
  }else{
  
  for (int i=0;i<nMaps;i++)
    Maps[i].mix1(m, 1, 3);
   for (int i=0;i<3;i++)
    iMap[i].mix(m, 1, 3, 255); 
    }
  sMap[0].mix(m, 6, 1, 3);
  sMap[1].mix(m, 6, 1, 3);
  sMap[2].mix(m, 5, 1, 2);
  m.set( (rlen[0] > 2) * ((x.bpos << 7U) | mxCtx), 1024);
  m.set( ((N^B)>>4)|(x1<<4), 512 );
  m.set( (x.grp<<5)|x1, 11*32);
  return (rlen[0]>2)*( (x.bpos<<7)|mxCtx );//1024 
  }
 
