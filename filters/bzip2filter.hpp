#pragma once
#include "filter.hpp"
#include "../bzip2/bzlib.h"
#define BZ2BLOCK 100*1024*100

class bzip2Filter: public Filter {

public:
  bzip2Filter(std::string n);
  ~bzip2Filter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};

U64 bzip2decompress(File* in, File* out, int compression_level, U64& csize, bool save=true);
U64 bzip2compress(File* im, File* out,int level, U64 size);
