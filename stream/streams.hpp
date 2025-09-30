#pragma once
#include "stream.hpp"
#include <assert.h>

class Streams {
private:
    void Add(Stream *s);
public:
    std::vector<Stream*> streams;
    ~Streams();
    Streams();
    int Count();
    int GetStreamID(Filetype type);
    bool isStreamType(Filetype type,int id);
    int GetTypeInfo(Filetype type);
};
