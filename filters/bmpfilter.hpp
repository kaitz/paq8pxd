#pragma once
#include "filter.hpp"

#define RGB565_MIN_RUN 63

class BmpFilter: public Filter {

public:
  BmpFilter(std::string n);
  ~BmpFilter();
  void encode(File *in, File *out, uint64_t size, int info);
  uint64_t decode(File *in, File *out,  uint64_t size, int info);
  void detect(File* in, uint64_t blockStart, uint64_t blockSize);
  uint64_t CompareFiles(File *in, File *out, uint64_t size, int info, FMode m);
};
