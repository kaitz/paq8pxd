#pragma once
#include "filter.hpp"


class MDFFilter: public Filter {

public:
  MDFFilter(std::string n, Filetype f=DEFAULT);
  ~MDFFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
  
};
