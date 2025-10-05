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
    Streamtype GetStreamID(Filetype type);
    bool isStreamType(Filetype type,int id);
    int GetTypeInfo(Filetype type);
    File& GetStreamFile(Streamtype id);
};
