#pragma once
#include "../prt/file.hpp"
#include "../prt/enums.hpp"
#include <cstdint>
#include <string>

class Filter {

public:
  uint64_t HeaderStart;//??
  uint64_t HeaderLength;//??
  uint64_t DataStart;
  uint64_t DataLength;
  Filetype Type;
  int Info;
  DetectState state;
  std::string name;
  uint64_t diffFound;
  Filter();
  virtual void encode(File *in, File *out, uint64_t size, int info)=0 ;
  virtual uint64_t decode(File *in, File *out, uint64_t size, int info)=0;
  virtual FMode compare(File *in, File *out, uint64_t size);
  virtual void detect(File* in, uint64_t blockStart, uint64_t blockSize)=0;
  virtual ~Filter();
};
