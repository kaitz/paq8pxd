//encoder/decoder for streams
#pragma once
#include "../prt/file.hpp"
#include "../prt/enums.hpp"
#include "../prt/coder.hpp"
#include "../prt/segment.hpp"
#include "../prt/textinfo.hpp"

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
#include "../filters/zlibfilter.hpp"
#include "../filters/bzip2filter.hpp"
#include "../filters/decafilter.hpp"
#include "../filters/rlefilter.hpp"
#include "../filters/base85filter.hpp"
#include "../filters/armfilter.hpp"
#include "../filters/lzwfilter.hpp"
#include "../filters/defaultfilter.hpp"

class Codec {
    FMode mode;
    Streams *streams;
    Segment *segment;
    std::vector<Filter*> filters;
    
    void AddFilter(Filter *f);
    void direct_encode_blockstream(Filetype type, File*in, U64 len, int info=0);
    void transform_encode_block(Filetype type, File*in, U64 len, Encoder &en, int info, int info2, char *blstr, int it, U64 begin);
	public:
		Codec(FMode m, Streams *s, Segment *g);
		virtual ~Codec();
		virtual void DecodeFile(const char* filename, uint64_t filesize);
		virtual void EncodeFile(const char* filename, uint64_t filesize);
		virtual Filter& GetFilter(Filetype f);
	protected:
	    virtual uint64_t DecodeFromStream(File *out, uint64_t size, FMode mode, int it=0);
        virtual void EncodeFileRecursive(File*in, uint64_t n, Encoder &en, char *blstr, int it=0);
};

struct TARheader{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char linkflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char major[8];
    char minor[8];
    char pad[167];
};
