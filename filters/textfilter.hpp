#pragma once
#include "filter.hpp"

class TextFilter: public Filter {

public:
  TextFilter(std::string n, Filetype f=DEFAULT);
  ~TextFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};
