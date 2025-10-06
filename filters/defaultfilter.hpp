#pragma once
#include "filter.hpp"

class DefaultFilter: public Filter {

public:
  DefaultFilter(std::string n, Filetype f=DEFAULT);
  ~DefaultFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};
