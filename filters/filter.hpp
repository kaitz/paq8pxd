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
  uint64_t fsize;
  Filter();
  virtual void encode(File *in, File *out, uint64_t size, uint64_t info)=0 ;
  virtual uint64_t decode(File *in, File *out, uint64_t size, uint64_t info)=0;
  virtual uint64_t CompareFiles(File *in, File *out, uint64_t size,uint64_t info,FMode m); 
  virtual ~Filter();
protected:
  virtual FMode compare(File *in, File *out, uint64_t size);
};
