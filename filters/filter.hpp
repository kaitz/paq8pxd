#pragma once
#include "../prt/file.hpp"
#include "../prt/enums.hpp"
#include <cstdint>
#include <string>

class Filter {

public:
  Filetype Type;
  int Info;
  std::string name;
  uint64_t diffFound;
  Filter();
  virtual void encode(File *in, File *out, uint64_t size, int info)=0 ;
  virtual uint64_t decode(File *in, File *out, uint64_t size, int info)=0;
  virtual FMode compare(File *in, File *out, uint64_t size);
  virtual uint64_t CompareFiles(File *in, File *out, uint64_t size,int info,FMode m); 
  virtual ~Filter();
};
