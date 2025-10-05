#pragma once
#include "stream.hpp"
#include <assert.h>

class Streams {
private:
    void Add(Stream *s);
public:
    std::vector<Stream*> streams;
    const std::string name;
    ~Streams();
    Streams();
    int Count();
    int GetStreamID(Filetype type)  ;
    bool isStreamType(Filetype type,int id);
    int GetTypeInfo(Filetype type) ;
    File& GetStreamFile(int id)  ;
};
