//encoder/decoder for streams
#pragma once
#include "../prt/array.hpp"
#include "../prt/file.hpp"
#include "../prt/enums.hpp"
#include "../prt/segment.hpp"
#include "../prt/textinfo.hpp"
#include "../prt/helper.hpp"

#include "../stream/streams.hpp"
#include <cstdint>

#include "../filters/img24filter.hpp"
#include "../filters/img32filter.hpp"
#include "../filters/mrbfilter.hpp"
#include "../filters/exefilter.hpp"
#include "../filters/textfilter.hpp"
#include "../filters/eolfilter.hpp"
#include "../filters/mdffilter.hpp"
#include "../filters/cdfilter.hpp"
#include "../filters/giffilter.hpp"
#include "../filters/szddfilter.hpp"
#include "../filters/base64filter.hpp"
#include "../filters/witfilter.hpp"
#include "../filters/uudfilter.hpp"
#include "../filters/preflatefilter.hpp"
#include "../filters/bzip2filter.hpp"
#include "../filters/decafilter.hpp"
#include "../filters/rlefilter.hpp"
#include "../filters/base85filter.hpp"
#include "../filters/armfilter.hpp"
#include "../filters/lzwfilter.hpp"
#include "../filters/defaultfilter.hpp"
#include "../filters/zlibfilter.hpp"
#include "../filters/shrinkfilter.hpp"
#include "../filters/reducefilter.hpp"
#include "../filters/implodefilter.hpp"
#include "../filters/pngfilter.hpp"
#include "analyzer.hpp"

struct Stat {
    Array<uint64_t> size;
    Array<uint32_t> count;
    Stat():size(0),count(0){
    }
};

class Codec {
    FMode mode;
    Streams *streams;
    Segment *segment;
    std::vector<Filter*> filters;
    uint64_t fsame;
    uint64_t fdiff;
    int recDepth;
    Array<Stat> stat;          // block statistics
    Stat statFail;
    int itcount;
    void AddFilter(Filter *f);
    void direct_encode_blockstream(Filetype type, File*in, U64 len, int info=0);
    void transform_encode_block(Filetype type, File*in, U64 len, int info, int info2, char *blstr, int it, U64 begin,File*tmp);
    void Status(uint64_t n, uint64_t size);
    public:
        Codec(FMode m, Streams *s, Segment *g, int depth=6);
        virtual ~Codec();
        virtual void DecodeFile(const char* filename, uint64_t filesize);
        virtual void EncodeFile(const char* filename, uint64_t filesize);
        virtual Filter* GetFilter(Filetype f);
        void PrintResult();
        void PrintStat(int limit=5);
    protected:
        virtual uint64_t DecodeFromStream(File *out, uint64_t size, FMode mode, int it=0);
        virtual void EncodeFileRecursive(File*in, uint64_t n, char *blstr, int it=0, Filetype p=DEFAULT, ParserType etype=P_DEF);
};

