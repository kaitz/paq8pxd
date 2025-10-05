#pragma once
#include "../prt/enums.hpp"
#include "../prt/file.hpp"
#include <vector>

struct datainfo{
    Filetype type;
    int info;
};
//base class
class Stream {
public:
    Streamtype id;
    std::vector<datainfo> dataType;
    FileTmp file;
    FileTmp out;
    U64 streamsize;
    const std::string name;
    virtual ~Stream();
    Stream(Streamtype s,const std::string name);
};
