#pragma once
#include "filter.hpp"

class witFilter: public Filter {

public:
  witFilter(std::string n);
  ~witFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
  
};

