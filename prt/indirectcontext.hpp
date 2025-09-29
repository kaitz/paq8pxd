#pragma once
#include "array.hpp"
#include "types.hpp"
#include "bucket.hpp"
#include "hash.hpp"
////////////////////////////// Indirect Context //////////////////////////////
template <typename T>
class IndirectContext {
private:
  Array<T> data;
  T* ctx;
  U32 ctxMask, inputMask, inputBits;
public:
  IndirectContext(const int BitsPerContext, const int InputBits = 8) :
    data(1ull<<BitsPerContext),
    ctx(&data[0]),
    ctxMask((1ul<<BitsPerContext)-1),
    inputMask((1ul<<InputBits)-1),
    inputBits(InputBits)
  {
    assert(BitsPerContext>0 && BitsPerContext<=20);
    assert(InputBits>0 && InputBits<=8);
  }
  void operator+=(const U32 i) {
    assert(i<=inputMask);
    (*ctx)<<=inputBits;
    (*ctx)|=i;
  }
  void operator=(const U32 i) {
    ctx = &data[i&ctxMask];
  }
  T& operator()(void) {
    return *ctx;
  }
};

template<typename T>
class IndirectContext1 {
private:
    Array<T> data;
    T *ctx;
    const uint32_t ctxMask, inputMask, inputBits, contextBits;

public:
    IndirectContext1(const int bitsPerContext, const int inputBits, const int contextBits = sizeof(T)*8) :
      data(UINT64_C(1) << bitsPerContext), ctx(&data[0]),
      ctxMask((UINT32_C(1) << bitsPerContext) - 1), 
      inputMask((UINT32_C(1) << inputBits) - 1), 
      inputBits(inputBits),
      contextBits(contextBits) {
#ifdef VERBOSE
      printf("Created IndirectContext with bitsPerContext = %d, inputBits = %d\n", bitsPerContext, inputBits);
#endif
      assert(bitsPerContext > 0 && bitsPerContext <= 20);
      assert(inputBits > 0 && inputBits <= 8);
      assert(contextBits <= sizeof(T)*8);
      if (contextBits < sizeof(T) * 8) // need for leading bit -> include it
        reset(); 
    }

    void reset() {
      for (uint64_t i = 0; i < data.size(); ++i) {
        data[i] = contextBits < sizeof(T) * 8 ? 1 : 0; // 1: leading bit to indicate the number of used bits
      }
    }

    void operator+=(const uint32_t i) {
      assert(i <= inputMask);
      // note: when the context is fully mature, we need to keep the leading bit in front of the contextbits as the MSB
      T leadingBit = (*ctx) & (1 << contextBits); 
      (*ctx) <<= inputBits;
      (*ctx) |= i | leadingBit;
      (*ctx) &= (1 << (contextBits + 1)) - 1;
    };

    void operator=(const uint32_t i) {
      ctx = &data[i & ctxMask];
    }

    auto operator()() -> T & {
      return *ctx;
    };
};

template<typename T>
class LargeIndirectContext {
private:
    Array<B16<T, 7>> data;
    const uint32_t hashBits, inputBits;

public:
  LargeIndirectContext(const int hashBits, const int inputBits) :
      data(UINT64_C(1) << hashBits),
      hashBits(hashBits),
      inputBits(inputBits)
    {
      assert(hashBits > 0 && hashBits <= 24);
      assert(inputBits > 0 && inputBits <= 8);
    }

    void reset() {
      for (uint64_t i = 0; i < data.size(); ++i) {
        data[i].reset();
      }
    }

    void set( uint64_t contextHash, const uint8_t c) {
      contextHash=contextHash*987654323;
      contextHash=contextHash<<16|contextHash>>16;
      contextHash=contextHash*123456791;
      assert(c < (1 << inputBits));
      uint32_t* ptr = data[finalize64(contextHash, hashBits)].find(checksum16(contextHash, hashBits), nullptr);
      *ptr = (*ptr) << inputBits | c;
    };

    uint32_t get(  uint64_t contextHash) {
      contextHash=contextHash*987654323;
      contextHash=contextHash<<16|contextHash>>16;
      contextHash=contextHash*123456791;
      return *data[finalize64(contextHash, hashBits)].find(checksum16(contextHash, hashBits), nullptr);
    };

};
