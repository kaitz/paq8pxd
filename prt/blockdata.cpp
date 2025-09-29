#include "blockdata.hpp"
extern U8  level;

BlockData::BlockData(): wrtpos(0),wrtfile(0),wrtsize(0),wrtcount(0),wrtdata(0),wrtLoaded(false),wrtText(255),wrtTextSize(0),wrtstatus(0),wrtbytesize(0),
y(0), c0(1), c4(0),c8(0),bpos(0),blpos(0),rm1(1),filetype(DEFAULT),
    b2(0),b3(0),b4(0),w4(0), w5(0),f4(0),tt(0),col(0),x4(0),s4(0),finfo(0),fails(0),failz(0),
    failcount(0),x5(0), frstchar(0),spafdo(0),spaces(0),spacecount(0), words(0),wordcount(0),
    wordlen(0),wordlen1(0),grp(0),Misses(0),count(0),wwords(0),tmask(0),
    wrtc4(0),dictonline(false),inpdf(false),wcol(0),utf8l(0),wlen(0),wstat(false),wdecoded(false),
    pwords(0),pbc(0),bc4(0),istex(true),ishtml(false),cxt(16)
   {
       memset(&Image, 0, sizeof(Image));
       memset(&Match, 0, sizeof(Match));
       memset(&Text, 0, sizeof(Text));
       memset(&DEC, 0, sizeof(DEC));
       memset(&JPEG, 0, sizeof(JPEG));
       memset(&x86_64, 0, sizeof(x86_64));
        // Set globals according to option
        assert(level<=15);
        bufn.setsize(0x10000);
        if (level>=11) buf.setsize(0x20000000); //limit 512mb
        else buf.setsize(MEM()*8);
       /* #ifndef NDEBUG 
        printf("\n Buf size %d bytes\n", buf.poswr);
        #endif */
    }

