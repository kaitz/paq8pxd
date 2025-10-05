#pragma once
#include "filter.hpp"
#include "../prt/DECAlpha.hpp"

class DecAFilter: public Filter {

public:
  DecAFilter(std::string n, Filetype f=DEFAULT);
  ~DecAFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};
