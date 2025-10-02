#pragma once
#include "filter.hpp"

#define RGB565_MIN_RUN 63

class Img24Filter: public Filter {

public:
  Img24Filter(std::string n);
  ~Img24Filter();
  void encode(File *in, File *out, uint64_t size, int info);
  uint64_t decode(File *in, File *out,  uint64_t size, int info);
};
