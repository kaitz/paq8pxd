#pragma once
#include "types.hpp"
#include <cstring>

U32 finalize32(const U32 hash, const int hashbits);
U32 finalize64(const U64 hash, const int hashbits);
U64 checksum64(const U64 hash, const int hashbits, const int checksumbits);

U32 hash(U32 x) ;
U32 combine(U32 seed, const U32 x);

// Magic number 2654435761 is the prime number closest to the 
// golden ratio of 2^32 (2654435769)
#define PHI 0x9E3779B1 //2654435761


static uint64_t hashes[14] = {UINT64_C(0x9E3779B97F4A7C15), UINT64_C(0x993DDEFFB1462949), UINT64_C(0xE9C91DC159AB0D2D),
                              UINT64_C(0x83D6A14F1B0CED73), UINT64_C(0xA14F1B0CED5A841F), UINT64_C(0xC0E51314A614F4EF),
                              UINT64_C(0xDA9CC2600AE45A27), UINT64_C(0x826797AA04A65737), UINT64_C(0x2375BE54C41A08ED),
                              UINT64_C(0xD39104E950564B37), UINT64_C(0x3091697D5E685623), UINT64_C(0x20EB84EE04A3C7E1),
                              UINT64_C(0xF501F1D0944B2383), UINT64_C(0xE3E4E8AA829AB9B5)};

// Golden ratio of 2^64 (not a prime)
#define PHI64 hashes[0] // 11400714819323198485

// Some more arbitrary magic (prime) numbers
#define MUL64_1 hashes[1]
#define MUL64_2 hashes[2]
#define MUL64_3 hashes[3]
#define MUL64_4 hashes[4]
#define MUL64_5 hashes[5]
#define MUL64_6 hashes[6]
#define MUL64_7 hashes[7]
#define MUL64_8 hashes[8]
#define MUL64_9 hashes[9]
#define MUL64_10 hashes[10]
#define MUL64_11 hashes[11]
#define MUL64_12 hashes[12]
#define MUL64_13 hashes[13]

//
// value hashing
//
// - Hash 1-13 64-bit (usually small) integers

  uint64_t hash(const uint64_t x0);
  uint64_t hash(const uint64_t x0, const uint64_t x1);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2) ;
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5, const uint64_t x6);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5, const uint64_t x6,
     const uint64_t x7) ;
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5, const uint64_t x6,
     const uint64_t x7, const uint64_t x8) ;
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5, const uint64_t x6,
     const uint64_t x7, const uint64_t x8, const uint64_t x9);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5, const uint64_t x6,
     const uint64_t x7, const uint64_t x8, const uint64_t x9, const uint64_t x10);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5, const uint64_t x6,
     const uint64_t x7, const uint64_t x8, const uint64_t x9, const uint64_t x10, const uint64_t x11);
  uint64_t hash(const uint64_t x0, const uint64_t x1, const uint64_t x2, const uint64_t x3, const uint64_t x4, const uint64_t x5, const uint64_t x6,
     const uint64_t x7, const uint64_t x8, const uint64_t x9, const uint64_t x10, const uint64_t x11, const uint64_t x12);

/**
 * 32-bit pseudo random number generator
 */
class Random1 {
    uint64_t _state;

public:
    Random1();
    auto operator()(int numberOfBits) -> uint32_t;
};


uint16_t checksum16(const uint64_t hash, const int hashBits);

struct HashElementForMatchPositions { // sizeof(HashElementForMatchPositions) = 3*4 = 12
  static constexpr size_t N = 3;
  uint32_t matchPositions[N];
  void Add(uint32_t pos) {
    if (N > 1) {
      memmove(&matchPositions[1], &matchPositions[0], (N - 1) * sizeof(matchPositions[0]));
    }
    matchPositions[0] = pos;
  }
};
