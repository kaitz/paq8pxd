#pragma once
#include "filter.hpp"

class base64Filter: public Filter {

public:
  base64Filter(std::string n, Filetype f=DEFAULT);
  ~base64Filter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
  
};

bool is_base64(unsigned char c);
