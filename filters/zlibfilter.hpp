#pragma once
#include "filter.hpp"
#include "../zlib/zlib.h"
#include "../prt/mft.hpp"

#define ZLIB_NUM_COMBINATIONS 81

class zlibFilter: public Filter {

public:
  zlibFilter(std::string n, Filetype f=DEFAULT);
  ~zlibFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};

int parse_zlib_header(int header);
int zlib_inflateInit(z_streamp strm, int zh);
