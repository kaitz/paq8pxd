#pragma once
#include "filter.hpp"

class base85Filter: public Filter {

public:
  base85Filter(std::string n, Filetype f=DEFAULT);
  ~base85Filter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
  
};

