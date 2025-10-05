#pragma once
#include "filter.hpp"

// EXE transform: <encoded-size> <begin> <block>...
// Encoded-size is 4 bytes, MSB first.
// begin is the offset of the start of the input file, 4 bytes, MSB first.
// Each block applies the e8e9 transform to strings falling entirely
// within the block starting from the end and working backwards.
// The 5 byte pattern is E8/E9 xx xx xx 00/FF (x86 CALL/JMP xxxxxxxx)
// where xxxxxxxx is a relative address LSB first.  The address is
// converted to an absolute address by adding the offset mod 2^25
// (in range +-2^24).

class ExeFilter: public Filter {

public:
  ExeFilter(std::string n, Filetype f=DEFAULT);
  ~ExeFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};
