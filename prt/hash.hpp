#pragma once
#include "types.hpp"

U32 finalize32(const U32 hash, const int hashbits);
U32 finalize64(const U64 hash, const int hashbits);
U64 checksum64(const U64 hash, const int hashbits, const int checksumbits);

U32 hash(U32 x) ;
U32 combine(U32 seed, const U32 x);

// Magic number 2654435761 is the prime number closest to the 
// golden ratio of 2^32 (2654435769)
#define PHI 0x9E3779B1 //2654435761
