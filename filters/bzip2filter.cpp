#include "bzip2filter.hpp"

bzip2Filter::bzip2Filter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void bzip2Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    bzip2decompress(in,out, int(info), size);
} 

uint64_t bzip2Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    fsize=bzip2compress(in, out, int(info), size);
    return fsize;
}

bzip2Filter::~bzip2Filter() {
}

U64 bzip2compress(File* im, File* out,int level, U64 size) {
    bz_stream stream;
    Array<char> bzin(BZ2BLOCK);
    Array<char> bzout(BZ2BLOCK);
    stream.bzalloc=NULL;
    stream.bzfree=NULL;
    stream.opaque=NULL;
    stream.next_in=NULL;
    stream.avail_in=0U;
    stream.avail_out=0U;
    U64 p=0,usize=size;
    int part,ret,status;
    ret=BZ2_bzCompressInit(&stream, level&255, 0, 0,level/256);
    if (ret!=BZ_OK) return ret;  
    do {
        stream.avail_in=im->blockread((U8*) &bzin[0], min(BZ2BLOCK,usize));
        status=usize<BZ2BLOCK?BZ_FINISH:BZ_RUN;
        usize=usize-stream.avail_in;
        stream.next_in=(char*) &bzin[0] ;
        do {
            stream.avail_out=BZ2BLOCK;
            stream.next_out=(char*)&bzout[0] ;
            ret=BZ2_bzCompress(&stream, status);
            part=BZ2BLOCK-stream.avail_out;
            if (part>0) p+=part,out->blockwrite((U8*) &bzout[0],part);
        } while (stream.avail_in != 0);

    } while (status!=BZ_FINISH);
    (void)BZ2_bzCompressEnd(&stream);
    return p;
}
U64 bzip2decompress(File* in, File* out, int compression_level, U64& csize, bool save) {
    bz_stream stream;
    Array<char> bzin(BZ2BLOCK);
    Array<char> bzout(BZ2BLOCK);
    stream.bzalloc=NULL;
    stream.bzfree=NULL;
    stream.opaque=NULL;
    stream.avail_in=0;
    stream.next_in=NULL;
    U64 dsize=0;
    int inbytes,part,ret;
    int blockz=csize?csize:BZ2BLOCK;
    csize=0;
    ret=BZ2_bzDecompressInit(&stream, 0, 0);
    if (ret!=BZ_OK) return ret;
    do {
        stream.avail_in=in->blockread((U8*) &bzin[0], min(BZ2BLOCK,blockz));
        inbytes=stream.avail_in;
        if (stream.avail_in==0) break;
        stream.next_in=(char*)&bzin[0];
        do {
            stream.avail_out=BZ2BLOCK;
            stream.next_out=(char*)&bzout[0];
            ret=BZ2_bzDecompress(&stream);
            if ((ret!=BZ_OK) && (ret!=BZ_STREAM_END)) {
                (void)BZ2_bzDecompressEnd(&stream);
                return ret;
            }
            csize+=(inbytes-stream.avail_in);
            inbytes=stream.avail_in;
            part=BZ2BLOCK-stream.avail_out;
            if (save==true)out->blockwrite((U8*) &bzout[0], part);
            dsize+=part;

        } while (stream.avail_out == 0);
    } while (ret != BZ_STREAM_END);
    (void)BZ2_bzDecompressEnd(&stream);
    if (ret == BZ_STREAM_END) return dsize; else return 0;
}
