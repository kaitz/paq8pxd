#include <assert.h>
#include "types.hpp"
#include "hash.hpp"
// Finalizers
// - Keep the necessary number of MSBs after a (combination of) 
//   multiplicative hash(es)

U32 finalize32(const U32 hash, const int hashbits) {
  assert(0<hashbits && hashbits<=32);
  return hash>>(32-hashbits);
}
U32 finalize64(const U64 hash, const int hashbits) {
  assert(0<hashbits && hashbits<=32);
  return U32(hash>>(64-hashbits));
}

// Get the next MSBs (8 or 6 bits) following "hasbits" for checksum
// Remark: the result must be cast/masked to the proper checksum size (U8, U16) by the caller
U64 checksum64(const U64 hash, const int hashbits, const int checksumbits) {
  return hash>>(64-hashbits-checksumbits); 
}


//////////////////////////// hash //////////////////////////////

// Hash 2-5 ints.
/*inline U32 hash(U32 a, U32 b, U32 c=0xffffffff, U32 d=0xffffffff,
    U32 e=0xffffffff) {
  U32 h=a*200002979u+b*30005491u+c*50004239u+d*70004807u+e*110002499u;
  return h^h>>9^a>>2^b>>3^c>>4^d>>5^e>>6;
}
*/


// A hash function to diffuse a 32-bit input
U32 hash(U32 x) {
  x++; // zeroes are common and mapped to zero
  x = ((x >> 16) ^ x) * 0x85ebca6b;
  x = ((x >> 13) ^ x) * 0xc2b2ae35;
  x = (x >> 16) ^ x;
  return x;
}

// Combine a hash value (seed) with another (non-hash) value.
// The result is a combined hash. 
//
// Use this function repeatedly to combine all input values 
// to be hashed to a final hash value.
U32 combine(U32 seed, const U32 x) {
  seed+=(x+1)*PHI;
  seed+=seed<<10;
  seed^=seed>>6;
  return seed;
}
