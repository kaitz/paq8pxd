#pragma once
#include "filter.hpp"

class armFilter: public Filter {

public:
  armFilter(std::string n, Settings &s, Filetype f=DEFAULT);
  ~armFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};

