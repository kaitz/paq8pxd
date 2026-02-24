#pragma once
#include "../prt/enums.hpp"
#include "../prt/file.hpp"
#include <vector>

struct datainfo{
    Filetype type;
    int info; //Streamtypeinfo
};
//base class
class Stream {
public:
    Streamtype id;
    std::vector<datainfo> dataType;
    FileTmp file;
    FileTmp out;
    uint64_t streamsize;
    uint64_t cstreamsize;
    const std::string name;
    virtual ~Stream();
    Stream(Streamtype s,const std::string name);
};
