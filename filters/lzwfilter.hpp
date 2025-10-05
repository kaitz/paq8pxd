#pragma once
#include "filter.hpp"
#include "../prt/hash.hpp"

class lzwFilter: public Filter {

public:
    lzwFilter(std::string n, Filetype f=DEFAULT);
    ~lzwFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};


struct LZWentry{
    int16_t prefix;
    int16_t suffix;
};

#define LZW_RESET_CODE 256
#define LZW_EOF_CODE   257

class LZWDictionary{
private:
    const static int32_t HashSize = 9221;
    LZWentry dictionary[4096];
    int16_t table[HashSize];
    uint8_t buffer[4096];
public:
    int32_t index;
    LZWDictionary(): index(0) { reset(); }
    void reset() {
        memset(&dictionary, 0xFF, sizeof(dictionary));
        memset(&table, 0xFF, sizeof(table));
        for (int32_t i=0; i<256; i++) {
            table[-findEntry(-1, i)-1]=(int16_t)i;
            dictionary[i].suffix=i;
        }
        index = 258; //2 extra codes, one for resetting the dictionary and one for signaling EOF
    }
    int32_t findEntry(const int32_t prefix, const int32_t suffix) {
        int32_t i=finalize32(hash(prefix, suffix), 13);
        int32_t offset=(i>0)?HashSize-i:1;
        while (true) {
            if (table[i]<0) //free slot?
            return -i-1;
            else if (dictionary[table[i]].prefix==prefix && dictionary[table[i]].suffix==suffix) //is it the entry we want?
            return table[i];
            i-=offset;
            if (i<0)
            i+=HashSize;
        }
    }
    void addEntry(const int32_t prefix, const int32_t suffix, const int32_t offset=-1) {
        if (prefix==-1 || prefix>=index || index>4095 || offset>=0)
            return;
        dictionary[index].prefix=prefix;
        dictionary[index].suffix=suffix;
        table[-offset-1]=index;
        index+=(index<4096);
    }
    int32_t dumpEntry(File *f, int32_t code) {
        int32_t n=4095;
        while (code>256 && n>=0) {
            buffer[n]=uint8_t(dictionary[code].suffix);
            n--;
            code=dictionary[code].prefix;
        }
        buffer[n]=uint8_t(code);
        f->blockwrite(&buffer[n], 4096-n);
        return code;
    }
};
