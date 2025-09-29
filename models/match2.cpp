#include "match2.hpp"

//extern U8 level;


 bool matchModel2::isMatch(BlockData& x,const uint32_t pos, const uint32_t MINLEN) {
      for (uint32_t length = 1; length <= MINLEN; length++) {
        if (x.buf(length) != x.buf[pos - length])
          return false;
      }
      return true;
    }

    void matchModel2::AddCandidates(BlockData& x,HashElementForMatchPositions* matches, uint32_t LEN) {
      uint32_t i = 0;
      while (numberOfActiveCandidates < N && i < HashElementForMatchPositions::N) {
        uint32_t matchpos = matches->matchPositions[i];
        if (matchpos == 0)
          break;
        if (isMatch(x,matchpos, LEN)) {
          bool isSame = false;
          //is this position already registered?
          for (uint32_t j = 0; j < numberOfActiveCandidates; j++) {
            MatchInfo* oldcandidate = &matchCandidates[j];
            if (isSame = oldcandidate->index == matchpos)
              break;
          }
          if (!isSame) { //don't register an already registered sequence
            matchCandidates[numberOfActiveCandidates].registerMatch(matchpos, LEN);
            numberOfActiveCandidates++;
          }
        }
        i++;
      }
    }

matchModel2::matchModel2(BlockData& bd) : 
  x(bd),
  hashtable(CMlimit(MEM()/8)),
  stateMaps {{  28 * 512},
             {  8 * 256 * 256 + 1},
             {  256 * 256}},
  cm( CMlimit(MEM()/2), nCM, 0,
  CM_RUN1+
  CM_RUN0+
  CM_MAIN1+
  CM_MAIN2+
  CM_MAIN3+
  CM_MAIN4+
  CM_M12+
  CM_M6
  ),
  mapL{ /* LargeStationaryMap : HashBits, Scale=64, Rate=16  */
        { 20}, // effective bits: ~22
  },
  map { /* StationaryMap : BitsOfContext, InputBits, Scale=64, Rate=16  */
        { 1 /*< leading bit */ + iCtxBits + 3 /*< length3Rm */, 1}
  }, 
  iCtx{15, 1, iCtxBits},
  hashBits(ilog2(uint32_t(hashtable.size())))
{
  assert(isPowerOf2(hashtable.size()));
  assert(isPowerOf2(CMlimit(MEM())));
  iCtx.reset();
  x.Match.bypass=false;
  x.Match.bypassprediction=2048;
}

void matchModel2::update() {

  size_t n = max(numberOfActiveCandidates, 1);
  for (size_t i = 0; i < n; i++) {
    MatchInfo* matchInfo = &matchCandidates[i];
    matchInfo->update(x);
    if (numberOfActiveCandidates != 0 && matchInfo->length == 0 && !matchInfo->delta && matchInfo->lengthBak==0) {
      numberOfActiveCandidates--;
      if (numberOfActiveCandidates == i)
        break;
      memmove(&matchCandidates[i], &matchCandidates[i + 1], (numberOfActiveCandidates - i) * sizeof(MatchInfo));
      i--;
    }
  }

  if( x.bpos == 0 ) {
    uint64_t hash;
    HashElementForMatchPositions* matches;

    hash = x.cxt[LEN9];
    matches = &hashtable[finalize64(hash, hashBits)];
    if (numberOfActiveCandidates < N)
      AddCandidates(x,matches, LEN9); //note: this length is *not* modelled by NormalModel
    matches->Add(x.buf.pos);

    hash = x.cxt[LEN7];
    matches = &hashtable[finalize64(hash, hashBits)];
    if (numberOfActiveCandidates < N)
      AddCandidates(x,matches, LEN7); //note: this length is *not* modelled by NormalModel
    matches->Add(x.buf.pos);

    hash = x.cxt[LEN5];
    matches = &hashtable[finalize64(hash, hashBits)];
    if (numberOfActiveCandidates < N)
      AddCandidates(x,matches, LEN5); //note: this length is modelled by NormalModel
    matches->Add(x.buf.pos);

    for (size_t i = 0; i < numberOfActiveCandidates; i++) {
      matchCandidates[i].expectedByte = x.buf[matchCandidates[i].index];
    }
  }
}

int matchModel2::p(Mixer &m,int val1,int val2) {
  
  update();

  for( uint32_t i = 0; i < nST; i++ ) { // reset contexts
    ctx[i] = 0;
  }
  
  size_t bestCandidateIdx = 0; //default item is the first candidate, let's see if any other candidate is better
  for (int i = 1; i < numberOfActiveCandidates; i++) {
    if (matchCandidates[i].isBetterThan(&matchCandidates[bestCandidateIdx]))
      bestCandidateIdx = i;
  }

  const uint32_t length = matchCandidates[bestCandidateIdx].length;
  const uint8_t expectedByte = matchCandidates[bestCandidateIdx].expectedByte;
  const bool isInNoMatchMode= matchCandidates[bestCandidateIdx].isInNoMatchMode();
  const bool isInDeltaMode = matchCandidates[bestCandidateIdx].delta;
  const bool isInPreRecoveryMode = matchCandidates[bestCandidateIdx].isInPreRecoveryMode();
  const bool isInRecoveryMode = matchCandidates[bestCandidateIdx].isInRecoveryMode();
  if (x.bpos == 0 && length==0 && x.Match.bypass)
      x.Match.bypass = false; // can quit bypass mode on byte boundary only
  const int expectedBit = length != 0 ? (expectedByte >> (7 - x.bpos)) & 1 : 0;
  uint32_t denselength = 0; // 0..27
  if (length != 0 && x.Match.bypass==false) {
    if (length <= 16) {
      denselength = length - 1; // 0..15
    } else {
      denselength = 12 + (min(length - 1, 63) >> 2); // 16..27
    }
    ctx[0] = (denselength << 9) | (expectedBit << 8) | x.c0; // 1..28*512
    ctx[1] = ((expectedByte << 11) | (x.bpos << 8) | (U8)x.c4) + 1;
    
    const int sign = 2 * expectedBit - 1;
    m.add(sign * (min(length, 32) << 5)); // +/- 32..1024
    m.add(sign * (ilog(min(length, 65535)) << 2)); // +/-  0..1024
  } else { // no match at all or delta mode
    m.add(0);
    m.add(0);
  }

  if( isInDeltaMode ) { // delta mode: helps predicting the remaining bits of a character when a mismatch occurs
    ctx[2] = (expectedByte << 8) | x.c0;
  }
 if ( x.Match.bypass==false){

  for( uint32_t i = 0; i < nST; i++ ) {
    const uint32_t c = ctx[i]; 
    
    if( c != 0 ) {
     const int p1 = stateMaps[i].p(c,x.y);
      const int st = stretch(p1);
      m.add(st >> 2);
     m.add((p1 - 2048) >> 3);
    } else {
      m.add(0);
      m.add(0);
    }
  }
}
  const uint32_t lengthIlog2 = ilog2(length + 1);
  //length=0:      lengthIlog2=0
  //length=1..2:   lengthIlog2=1
  //length=3..6:   lengthIlog2=2
  //length=7..14:  lengthIlog2=3
  //length=15..30: lengthIlog2=4

  const uint8_t mode = 
    isInNoMatchMode ? 0 :
    isInDeltaMode ? 1 :
    isInPreRecoveryMode ? 2 :
    isInRecoveryMode ? 3 :
    3 + lengthIlog2;
  uint8_t mode3 = min(mode, 7); // 3 bits
  
  //bytewise contexts
  if( x.bpos == 0 && x.Match.bypass==false) {
    cm.set(hash(U32(length != 0 ? expectedByte : (U8)x.c4)<<3 | mode3)); //max context bits: 8+8+3 = 19
    cm.set( hash(length != 0 ? expectedByte : ((x.c4 >> 8) & 0xff), ((U8)x.c4<<3)| mode3)); //max context bits: 8+8+8+3=27
    
  }
  cm.mix(m);

  //bitwise contexts
  mapL[0].set(hash(expectedByte, x.c0, x.c4 & 0x00ffffff, mode3)); // max context bits: 8+8+24+3 = 43 bits -> hashed into ~22 bits
  mapL[0].mix(m);

  const uint32_t mCtx =
    isInNoMatchMode ? 0 :
    isInDeltaMode ? 1 :
    isInPreRecoveryMode ? 2 :
    isInRecoveryMode ? 3 :
    4 + ((lengthIlog2 - 1) << 1 | expectedBit);

  iCtx += x.y;
  iCtx = (x.bpos << 12) | ((U8)x.c4 << 4) | min(mCtx,15); // 15 bits
  map[0].set(iCtx() << 3 | mode3); // (max 7 bits + 1 leading bit) + 3 bits
  map[0].mix(m);
  
  m.set(min(mCtx, 11),12);
  x.Match.length3 =
    length == 0 ? 0 :
    isInDeltaMode ? 1 :
    length <= 7 ? 2 : 3;
  //x.Match.length3=(min(ilog2(length),3));
 
    //x.Match.byte = (length)?expectedByte:0;
    //x.Match.bypassprediction = length==0 ? 2048 : (expectedBit==0 ? 1 : 4095);
    //x.Match.length = length;
  return ilog(length);
}  


