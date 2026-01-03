#pragma once
#include "../prt/file.hpp"
#include "../preflate/support/stream.h"
#include <vector>
#include <algorithm>

// Adapter: paq8pxd File* -> preflate InputStream
class FileInputStream : public InputStream {
    File* file;
    uint64_t remaining;
public:
    FileInputStream(File* f, uint64_t size) : file(f), remaining(size) {}
    
    bool eof() const override { 
        return remaining == 0 || file->eof(); 
    }
    
    size_t read(unsigned char* buffer, const size_t size) override {
        size_t toRead = std::min(size, static_cast<size_t>(remaining));
        size_t bytesRead = file->blockread(buffer, toRead);
        remaining -= bytesRead;
        return bytesRead;
    }
};

// Adapter: paq8pxd File* -> preflate OutputStream  
class FileOutputStream : public OutputStream {
    File* file;
public:
    FileOutputStream(File* f) : file(f) {}
    
    size_t write(const unsigned char* buffer, const size_t size) override {
        return file->blockwrite(const_cast<unsigned char*>(buffer), size);
    }
};
