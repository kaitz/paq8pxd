#pragma once
#include "../prt/types.hpp"
#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/blockdata.hpp"
//#include "../prt/buffers.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirectcontext.hpp"
//#include "../prt/mft.hpp"
//#include "../prt/helper.hpp"
//#include "../prt/contextmap2.hpp"
#include "../prt/indirect.hpp"
#include "../prt/DECAlpha.hpp"

template<class T>
class RingBuffer1 {
private:
    Array<T> b;
    uint32_t offset {0}; /**< Number of input bytes in buffer (not wrapped), will be masked when used for indexing */
    uint32_t mask;

public:
    /**
     * Creates an array of @ref size bytes (must be a power of 2).
     * @param size number of bytes in array
     */
    explicit RingBuffer1(const uint32_t size = 0) : b(size), mask(size - 1) {
#ifdef VERBOSE
      printf("Created RingBuffer with size = %d\n", size);
#endif
      assert(isPowerOf2(size));
    }

    void setSize(uint32_t newSize) {
      assert(newSize > 0 && isPowerOf2(newSize));
      b.resize(newSize);
      offset = 0;
      mask = newSize - 1;
    }

     auto getpos() const -> uint32_t {
      return offset;
    }

    void fill(const T B) {
      const auto n = (uint32_t) b.size();
      for( uint32_t i = 0; i < n; i++ ) {
        b[i] = B;
      }
    }

    void add(const T B) {
      b[offset & mask] = B;
      offset++;
    }

    /**
     * Returns a reference to the i'th byte with wrap (no out of bounds).
     * @param i
     * @return
     */
    auto operator[](const uint32_t i) const -> T {
      return b[i & mask];
    }

    void set(const uint32_t i, const T B) {
      b[i & mask] = B;
    }

    /**
     * Returns i'th byte back from pos (i>0) with wrap (no out of bounds)
     * @param i
     * @return
     */
    auto operator()(const uint32_t i) const -> T {
      //assert(i!=0);
      return b[(offset - i) & mask];
    }

    void reset() {
      fill(0);
      offset = 0;
    }

    /**
     * @return the size of the RingBuffer
     */
    auto size() -> uint32_t {
      return (uint32_t) b.size();
    }

    void copyTo(RingBuffer1 &dst) {
      dst.setSize(size());
      dst.offset = offset;
      auto n = (uint32_t) b.size();
      for( uint32_t i = 0; i < n; i++ ) {
        dst.b[i] = b[i];
      }
    }
};


class decModel1: public Model {
public:  
  BlockData& x;
  Buf& buf;
    enum State {
    OpCode,
    Bra_Ra,
    Bra_Displacement,
    F_P_Function,
    F_P_Ra,
    F_P_Rb,
    F_P_Rc,
    Mem_Ra,
    Mem_Rb,
    Mem_Displacement,
    Mfc_Function,
    Mfc_Ra,
    Mfc_Rb,
    Mbr_Ra,
    Mbr_Rb,
    Mbr_Displacement,
    Opr_Bit,
    Opr_Function,
    Opr_Ra,
    Opr_Rb,
    Opr_Unused,
    Opr_Literal,
    Opr_Rc,
    Pcd_Function,
    Nop_Skip,
    Count
  };
  struct Instruction {
    std::uint32_t Function, Displacement;
    std::uint8_t Opcode, Bit, Ra, Rb, Rc, Literal, relOpcode, Format;
  };
  
  static constexpr std::uint32_t nMaps = 11u;
   std::uint32_t maps_mask[nMaps-1] = { 0x6FC3FFu, 0x6F0387u, 0x4E0383u, 0x440183u, 0x181u, 0x181u, 0x81u, 0x1u, 0x1u, 0x1u };
 private:
  RingBuffer1<Instruction> cache;
  State state;
  Instruction op;
  std::uint32_t count;
  std::uint8_t lastRc;
  Instruction last[8];
  IndirectMap maps0[State::Count], maps1[18], maps2[12], maps3[9], maps4[6], maps5[3], maps6[3], maps7[2], maps8[1], maps9[1], maps10[1];
  IndirectMap* const maps[nMaps - 1] = { &maps1[0], &maps2[0], &maps3[0], &maps4[0], &maps5[0], &maps6[0], &maps7[0], &maps8[0], &maps9[0], &maps10[0] };


           std::int32_t map_state(std::uint32_t const map, State const state) {
    std::int32_t r = -1;
    if ((map < nMaps) && (((maps_mask[map] >> state) & 1u) != 0u))     
      for (std::int32_t i = state; i >= 0; r += (maps_mask[map] >> i) & 1, i--);
    return r;
  }
  public:
  decModel1(BlockData& bd,U32 val=0);
 int inputs() {return nMaps*2 ;}
 int nets() {return State::Count * 26 + State::Count * 64 + 2048 + 4096 + 4096 + 8192 + 8192 + 8192 + 4096 + 4096 + 4096;;}
  int netcount() {return 11;}
int p(Mixer& m,int val1=0,int val2=0);
 virtual ~decModel1(){}
};
