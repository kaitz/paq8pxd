#include "array.hpp"
 
//#ifndef NDEBUG
 void chkindex(U64 index, U64 upper_bound) {
  if (index>=upper_bound) {
    fprintf(stderr, "out of upper bound\n");
    quit("");
  }
}
//#endif

