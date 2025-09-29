#pragma once
#include "../prt/types.hpp"
#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
//#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/indirectcontext.hpp"
#include "../prt/statemap.hpp"
#include "../prt/stationarymap.hpp"
//#include "../prt/sscm.hpp"
#include "../prt/contextmap2.hpp"
#include "../prt/largestationarymap.hpp"


class matchModel2: public Model {
private:
    BlockData& x;
    static constexpr int nCM = 2;
    static constexpr int nST = 3;
    static constexpr int nLSM = 1;
    static constexpr int nSM = 1;
    Array<HashElementForMatchPositions> hashtable;
    StateMap stateMaps[nST];
    ContextMap2 cm;
    LargeStationaryMap mapL[nLSM];
    StationaryMap map[nSM];
    static constexpr uint32_t iCtxBits = 7;
    IndirectContext1<uint8_t> iCtx;
    uint32_t ctx[nST] {0};
    const int hashBits;

    static constexpr int MINLEN_RM = 5; //minimum length in recovery mode before we "fully recover"
    static constexpr int LEN5 = 5;
    static constexpr int LEN7 = 7;
    static constexpr int LEN9 = 9;

    struct MatchInfo {

      uint32_t length = 0; /**< rebased length of match (length=1 represents the smallest accepted match length), or 0 if no match */
      uint32_t index = 0; /**< points to next byte of match in buf, 0 when there is no match */
      uint32_t lengthBak = 0; /**< allows match recovery after a 1-byte mismatch */
      uint32_t indexBak = 0;
      uint8_t expectedByte = 0; /**< prediction is based on this byte (buf[index]), valid only when length>0 */
      bool delta = false; /**< indicates that a match has just failed (delta mode) */

      bool isInNoMatchMode() {
        return length == 0 && !delta && lengthBak == 0;
      }

      bool isInPreRecoveryMode() {
        return length == 0 && !delta && lengthBak != 0;
      }

      bool isInRecoveryMode() {
        return length != 0 && lengthBak != 0;
      }

      uint32_t recoveryModePos() {
        assert(isInRecoveryMode()); //must be in recovery mode
        assert(length - lengthBak <= MINLEN_RM);
        return length - lengthBak;
      }

      uint64_t prio() {
        return
          static_cast<uint64_t>(length != 0) << 49 | //normal mode (match)
          static_cast<uint64_t>(delta) << 48 | //delta mode
          static_cast<uint64_t>(delta ? lengthBak : length) << 32 | //the lnger wins
          static_cast<uint64_t>(index); //the more recent wins
      }
      bool isBetterThan(MatchInfo* other) {
        return this->prio() > other->prio();
      }

      void update(BlockData& x) {
        if (false) {
          printf("- pos %d %d  index %d  length %d  lengthBak %d  delta %d\n", x.blpos, x.bpos, index, length, lengthBak, delta ? 1 : 0);
        }
        if (length != 0) {
          const int expectedBit = (expectedByte >> ((8 - x.bpos) & 7)) & 1;
          if (x.y != expectedBit) {
            if (isInRecoveryMode()) { // another mismatch in recovery mode -> give up
              lengthBak = 0;
              indexBak = 0;
            }
            else { //backup match information: maybe we can recover it just after this mismatch
              lengthBak = length;
              indexBak = index;
              delta = true; //enter into delta mode - for the remaining bits in this byte length will be 0; we will exit delta mode and enter into recovery mode on bpos==0
            }
            length = 0;
          }
        }

        if (x.bpos == 0) {

          // recover match after a 1-byte mismatch
          if (isInPreRecoveryMode()) { // just exited delta mode, so we have a backup
            //the match failed 2 bytes ago, we must increase indexBak by 2:
            indexBak++;
            if (lengthBak < 65535) {
              lengthBak++;
            }
            if (x.buf[indexBak] == (U8)x.c4) { // match continues -> recover
              length = lengthBak;
              index = indexBak;
            }
            else { // still mismatch
              lengthBak = indexBak = 0; // purge backup (give up)
            }
          }

          // extend current match
          if (length != 0) {
            index++;
            if (length < 65535) {
              length++;
            }
            if (isInRecoveryMode() && recoveryModePos() >= MINLEN_RM) { // strong recovery -> exit tecovery mode (finalize)
              lengthBak = indexBak = 0; // purge backup
            }
          }
          delta = false;
        }
       /* if constexpr(false) {
          printf("  pos %d %d  index %d  length %d  lengthBak %d  delta %d\n", blockPos, bpos, index, length, lengthBak, delta ? 1 : 0);
        }*/

      }

      void registerMatch(const uint32_t pos, const uint32_t LEN) {
        assert(pos != 0);
        length = LEN - LEN5 + 1; // rebase
        index = pos;
        lengthBak = indexBak = 0;
        expectedByte = 0;
        delta = false;
      }

    };

    bool isMatch(BlockData& x,const uint32_t pos, const uint32_t MINLEN);

    void AddCandidates(BlockData& x,HashElementForMatchPositions* matches, uint32_t LEN);

    static constexpr size_t N = 4;
    Array<MatchInfo> matchCandidates{ N };
    uint32_t numberOfActiveCandidates = 0;
void update();
public:
    virtual ~matchModel2(){ }
    matchModel2(BlockData& bd);
    //int p(Mixer &m,int val1,int val2);
int inputs()   {return 2+nCM*cm.inputs()+nST*2 +nLSM*3+nSM*2/*+3*2 */;}
int nets()     {return 12;}
int netcount() {return 1;}




int p(Mixer &m,int val1=0,int val2=0) ;
};





