#include "codec.hpp"

extern int verbose;
extern bool witmode; //-w
extern TextParserStateInfo textparser;
extern int itcount;
extern U64 typenamess[datatypecount][5];
extern U32 typenamesc[datatypecount][5];
extern Filetype detect(File* in, U64 n, Filetype type, int &info, int &info2, int it=0);
extern void SetConColor(int color);

extern int getoct(const char *p, int n);
extern int tarchecksum(char *p);
extern bool tarend(const char *p);

Codec::Codec(FMode m, Streams *s, Segment *g):mode(m),streams(s),segment(g) {
    AddFilter( new Img24Filter( std::string("image 24bit"),IMAGE24));
    AddFilter( new Img32Filter( std::string("image 32bit"),IMAGE32));
    AddFilter( new ImgMRBFilter(std::string("image mrb"),MRBR));
    AddFilter( new ImgMRBFilter(std::string("image mrb4"),MRBR4));
    AddFilter( new ExeFilter(std::string("exe"),EXE));
    AddFilter( new TextFilter(std::string("text"),TEXT));
    AddFilter( new EOLFilter(std::string("eol"),EOLTEXT));
    AddFilter( new MDFFilter(std::string("mdf"),MDF));
    AddFilter( new CDFilter(std::string("cd"),CD));
    AddFilter( new gifFilter(std::string("gif"),GIF));
    AddFilter( new szddFilter(std::string("szdd"),SZDD));
    AddFilter( new base64Filter(std::string("base64"),BASE64));
    AddFilter( new witFilter(std::string("wit"),WIT));
    AddFilter( new uudFilter(std::string("uuencode"),UUENC));
    AddFilter( new zlibFilter(std::string("zlib"),ZLIB));
    AddFilter( new bzip2Filter(std::string("bzip2"),BZIP2));
    AddFilter( new DecAFilter(std::string("dec alpha"),DECA));
    AddFilter( new rleFilter(std::string("rle tga"),RLE));
    AddFilter( new base85Filter(std::string("base85"),BASE85));
    AddFilter( new armFilter(std::string("arm"),ARM));
    AddFilter( new lzwFilter(std::string("lzw"),LZW));
    AddFilter( new TextFilter(std::string("TXTUTF8"),TXTUTF8));
    AddFilter( new TextFilter(std::string("BIGTEXT"),BIGTEXT));
    AddFilter( new DefaultFilter(std::string("default"),DEFAULT)); // must be last
}

Codec::~Codec() {
}

void Codec::AddFilter(Filter *f) {
    filters.push_back(f);
}

Filter& Codec::GetFilter(Filetype f) {
    for (uint64_t j=0; j<filters.size(); j++) {
        if (filters[j]->Type==f) return *filters[j];
    }
    // or else default filter
    return *filters[filters.size()-1];
}

void Codec::DecodeFile(const char* filename, uint64_t filesize) {
    FMode mode=FDECOMPRESS;
    assert(filename && filename[0]);
    // Test if output file exists.  If so, then compare.
    FileDisk f;
    bool success=f.open(filename,false);
    if (success) {
        mode=FCOMPARE,printf("Comparing");
    } else {
        // Create file
        f.create(filename);
        mode=FDECOMPRESS, printf("Extracting");
    }
    printf(" %s %0.0f -> \n", filename, filesize+0.0);

    // Decompress/Compare
    U64 r=DecodeFromStream(&f, filesize, mode);
    if (mode==FCOMPARE && !r && f.getc()!=EOF) printf("file is longer\n");
    else if (mode==FCOMPARE && r) printf("differ at %0lu\n",r-1);
    else if (mode==FCOMPARE) printf("identical\n");
    else printf("done   \n");
    f.close();
}

void Codec::EncodeFile(const char* filename, uint64_t filesize) {
    assert(filename && filename[0]);
    FileDisk in;
    in.open(filename,true);
    if (verbose>2) printf("Block segmentation:\n");
    char blstr[32]="";
    EncodeFileRecursive(&in, filesize,  blstr);
    in.close();
}

//#define PNGFlag (1<<31)
//#define GrayFlag (1<<30)
uint64_t Codec::DecodeFromStream(File *out, uint64_t size, FMode mode, int it) {
    Filetype type;
    U64 len,i,diffFound;
    len=i=diffFound=0L;
    int info=-1;
    File *in;
    while (i<size) {
        type=(Filetype)segment->Get();
        for (int k=0; k<8; k++) len=len<<8,len+=segment->Get();
        for (int k=info=0; k<4; ++k) info=(info<<8)+segment->Get();
        Streamtype srid=streams->GetStreamID(type);
        int ti=streams->GetTypeInfo(type);
        if (srid>STR_NONE) in=&streams->GetStreamFile(srid);
        #ifdef VERBOSE  
         printf(" %d  %-9s |%0lu [%0lu]\n",it, typenames[type],len,i );
        #endif
        Filter& dataf=GetFilter(type); 
        if (srid>STR_NONE && srid!=STR_TEXT0 && srid!=STR_TEXT && (ti&TR_TRANSFORM)==TR_TRANSFORM) {
            printf("Filter: %s\n",dataf.name.c_str());
            diffFound=dataf.CompareFiles(in,out,len,uint64_t(info),mode);
            len=dataf.fsize;
        }  
        // LZW ?
        else if (srid==STR_NONE) {
            FileTmp tmp;
            DecodeFromStream(&tmp, len, FDECOMPRESS, it+1);
            if (mode!=FDISCARD) {
                tmp.setpos(0);
                diffFound=dataf.CompareFiles(&tmp,out,len,uint64_t(info),mode);
                len=dataf.fsize; // get decoded size
            }
            tmp.close();
        }
        else {
            for (U64 j=i; j<i+len; ++j) {
                //if (!(j&0x1ffff)) printStatus(j, s2);
                if (mode==FDECOMPRESS) out->putc(in->getc());
                else if (mode==FCOMPARE) {
                    int a=out->getc();
                    int b=in->getc();
                    if (a!=b && !diffFound) {
                        mode=FDISCARD;
                        diffFound=j+1;
                    }
                } else in->getc();
            }
        }
        i+=len;
    }
    return diffFound;
}

void Codec::EncodeFileRecursive(File*in, uint64_t n,  char *blstr, int it) {
    static const char* audiotypes[6]={"8b mono","8b stereo","16b mono","16b stereo","32b mono","32b stereo"};
  Filetype type=DEFAULT;
  int blnum=0, info,info2;  // image width or audio type
  U64 begin= in->curpos(), end0=begin+n;
  U64 textstart;
  U64 textend=0;
  U64 end=0;U64 len;
  Filetype nextType;
  //Filetype nextblock_type;
  Filetype nextblock_type_bak=DEFAULT; //initialized only to suppress a compiler warning, will be overwritten
  char b2[32];
  strcpy(b2, blstr);
  if (b2[0]) strcat(b2, "-");
  if (it==5) {
    direct_encode_blockstream(DEFAULT, in, n);
    return;
  }
  // Transform and test in blocks
  while (n>0) {
    if (it==0 && witmode==true) {
      len=end=end0,info=0,type=WIT;
    }
    else if (it==1 && witmode==true){    
      len=end=end0,info=0,type=TXTUTF8;
    } else {   
    if(type==TEXT || type==EOLTEXT || type==TXTUTF8) { // it was a split block in the previous iteration: TEXT -> DEFAULT -> ...
      nextType=nextblock_type_bak;
      end=textend+1;
    }
    else {
      nextType=detect(in, n, type, info,info2,it);
      end=in->curpos();
      in->setpos(begin);
    }
   // Filetype nextType=detect(in, n, type, info,info2,it,s1);
   // U64 end= in->curpos();
   //  in->setpos( begin);
     // override (any) next block detection by a preceding text block
    textstart=begin+textparser._start[0];
    textend=begin+textparser._end[0];
    if(textend>end-1)textend=end-1;
    if(type==DEFAULT && textstart<textend) { // only DEFAULT blocks may be overridden
     U64 nsize=0;
      if(textstart==begin && textend == end-1) { // whole first block is text
        type=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; // DEFAULT -> TEXT
        U64 nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))type=TEXT0;
        //if (type==TEXT && textparser.countUTF8>0xffff) type=TXTUTF8;
      }
      else if (textend - textstart + 1 >= TEXT_MIN_SIZE) { // we have one (or more) large enough text portion that splits DEFAULT
        if (textstart != begin) { // text is not the first block 
          end=textstart; // first block is still DEFAULT
          nextblock_type_bak=nextType;
          nextType=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; //next block is text
          //if (textparser.number()>((end-begin)>>1))nextblock_type=TEXT0;
           nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))nextType=TEXT0; 
          textparser.removefirst();
        } else {
          type=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; // first block is text
          nextType=DEFAULT;     // next block is DEFAULT
          end=textend+1; 
          //if (textparser.number()>((end-begin)>>1))type=TEXT0;
           nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))type=TEXT0;
        }
      }
      if (type==TEXT && textparser.countUTF8>0xffff) type=TXTUTF8,info=0;
      // no text block is found, still DEFAULT
      
    }
    if (end>end0) {  // if some detection reports longer then actual size file is
      end=begin+1;
      type=nextType=DEFAULT;
    }
      len=U64(end-begin);
    if (begin>end) len=0;
    if (len>=2147483646) {  
      if (!(type==BZIP2||type==WIT ||type==TEXT || type==TXTUTF8 ||type==TEXT0 ||type==EOLTEXT))len=2147483646,type=DEFAULT; // force to int
    }
   }
    if (len>0) {
    if ((type==EOLTEXT) && (len<1024*64 || len>0x1FFFFFFF)) type=TEXT;
    if (it>itcount)    itcount=it;
    if((len>>1)<(info) && type==DEFAULT && info<len) type=BINTEXT;
    if(len==info && type==DEFAULT ) type=ISOTEXT;
    if(len<=TEXT_MIN_SIZE && type==TEXT0 ) type=TEXT;
    typenamess[type][it]+=len,  typenamesc[type][it]++; 
      //s2-=len;
      sprintf(blstr,"%s%d",b2,blnum++);
      // printf(" %-11s | %-9s |%10.0" PRIi64 " [%0lu - %0lu]",blstr,typenames[type],len,begin,end-1);
      if (verbose>2) printf(" %-16s |",blstr);
      int streamcolor=streams->GetStreamID(type)+1+1;
      if (streamcolor<1) streamcolor=7;
      SetConColor(streamcolor);
      if (verbose>2) printf(" %-9s ",typenames[type]);
      SetConColor(7);
      if (verbose>2) {
        printf("|%12.0f [%0.0f - %0.0f]",len+0.0,begin+0.0,(end-1)+0.0);
        if (type==AUDIO) printf(" (%s)", audiotypes[(info&31)%4+(info>>7)*2]);
        else if (type==IMAGE1 || type==IMAGE4 || type==IMAGE8 || type==IMAGE24 || type==MRBR|| type==MRBR4|| type==IMAGE8GRAY || type==IMAGE32 ||type==GIF) printf(" (width: %d)", info&0xFFFFFF);
        else if (type==CD) printf(" (m%d/f%d)", info==1?1:2, info!=3?1:2);
        else if (type==ZLIB && (info>>24) > 0) printf(" (%s)",typenames[info>>24]);
        printf("\n");
      }
      transform_encode_block(type, in, len,  info,info2, blstr, it, begin);
      n-=len;
    }
    
    type=nextType;
    begin=end;
  }
}

void Codec::direct_encode_blockstream(Filetype type, File*in, U64 len, int info) {
    segment->putdata(type,len,info);
    int srid=streams->GetStreamID(type);
    Stream *sout=streams->streams[srid];
    for (U64 j=0; j<len; ++j) sout->file.putc(in->getc());
}

void Codec::transform_encode_block(Filetype type, File*in, U64 len, int info, int info2, char *blstr, int it, U64 begin) {
    if (streams->GetTypeInfo(type)&TR_TRANSFORM) {
        U64 diffFound=0;
        U32 winfo=0;
        FileTmp* tmp;
        tmp=new FileTmp;
        Filter& dataf=GetFilter(type);
        if (type==IMAGE24) dataf.encode(in, tmp, len, uint64_t(info));
        else if (type==IMAGE32) dataf.encode(in, tmp, len, uint64_t(info));
        else if (type==MRBR) dataf.encode(in, tmp, len, uint64_t(info)+(uint64_t(info2)<<32));
        else if (type==MRBR4) dataf.encode(in, tmp, len, uint64_t(((info*4+15)/16)*2)+(uint64_t(info2)<<32));
        else if (type==RLE) dataf.encode(in, tmp, len, info);
        else if (type==LZW) dataf.encode(in, tmp, len, info);
        else if (type==EXE) dataf.encode(in, tmp, len, begin);
        else if (type==DECA) dataf.encode(in, tmp, len, 0);
        else if (type==ARM) dataf.encode(in, tmp, len, 0);
        else if (type==TEXT || type==TXTUTF8 || type==TEXT0 || type==ISOTEXT) {
            if (type!=TXTUTF8) {
                dataf.encode(in, tmp, len,1);
                U64 txt0Size= tmp->curpos();
                //reset to text mode
                in->setpos(begin);
                tmp->close();
                tmp=new FileTmp;
                dataf.encode(in, tmp, (len),0);
                U64 txtSize= tmp->curpos();
                tmp->close();
                in->setpos( begin);
                tmp=new FileTmp;
                if (txt0Size<txtSize && (((txt0Size*100)/txtSize)<95)) {
                    in->setpos( begin);
                    dataf.encode(in, tmp, (len),1);
                    type=TEXT0,info=1;
                }else{
                    dataf.encode(in, tmp, (len),0);
                    type=TEXT,info=0;
                }
            }
            else dataf.encode(in, tmp, (len),info&1); 
        } else if (type==EOLTEXT) {
            dataf.encode(in, tmp, len,info&1);
            if (dataf.diffFound) {
                // if EOL size is below 25 then drop EOL transform and try TEXT type
                in->setpos(begin);
                type=TEXT;
                dataf=GetFilter(type);
                tmp->close();
                tmp=new FileTmp();
                dataf.encode(in, tmp, len,info&1); 
            }
        } else if (type==BASE64) dataf.encode(in, tmp, len,0);
        else if (type==UUENC) dataf.encode(in, tmp, len,info);
        else if (type==BASE85) dataf.encode(in, tmp, len,info);
        else if (type==SZDD) {
            dataf.encode(in, tmp,0, info);
        } else if (type==ZLIB) {
            dataf.encode(in, tmp, len,0);   
            diffFound=dataf.diffFound;
        } else if (type==BZIP2){
            dataf.encode(in, tmp, len,info);
        } else if (type==CD) dataf.encode(in, tmp, (len), info);
        else if (type==MDF) {
            dataf.encode(in, tmp, len,0);
        } else if (type==GIF)  {
            dataf.encode(in, tmp, len,0);
            diffFound=dataf.diffFound;
        } else if (type==WIT) {
            dataf.encode(in, tmp, len,0);
            winfo=dataf.fsize;
        }
        
        const U64 tmpsize= tmp->curpos();
        int tfail=0;
        tmp->setpos(0);
        
        if (type==BZIP2 || type==ZLIB || type==GIF || type==MRBR|| type==MRBR4|| type==RLE|| type==LZW||type==BASE85 ||
                type==BASE64 || type==UUENC|| type==DECA|| type==ARM || (type==WIT||type==TEXT || type==TXTUTF8 ||type==TEXT0)||type==EOLTEXT ){
            
            in->setpos(begin);
            if (type==BASE64 ) {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==UUENC ) {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==BASE85 ) {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==ZLIB && !diffFound) {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==BZIP2  )     {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info=info+256*17), FCOMPARE);
            } else if (type==GIF && !diffFound) {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==MRBR || type==MRBR4) {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==RLE)           {
                diffFound=dataf.CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if (type==LZW) {
                diffFound=dataf.CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if (type==DECA) {
                diffFound=dataf.CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if (type==ARM) {
                diffFound=dataf.CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if ((type==TEXT || (type==TXTUTF8 &&witmode==false) ||type==TEXT0) ) {
                diffFound=dataf.CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if ((type==WIT) ) {
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(winfo), FCOMPARE);
            }
            else if ((type==TXTUTF8 &&witmode==true) ) tmp->setend(); //skips 2* input size reading from a file
            else if (type==EOLTEXT ){
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            }
            if (type==EOLTEXT && (diffFound )) {
                // if fail fall back to text
                diffFound=0,info=-1, in->setpos(begin),type=TEXT,dataf=GetFilter(type),tmp->close(),tmp=new FileTmp(),dataf.encode(in, tmp, len,0); 
            }  else if (type==BZIP2 && (diffFound) ) {
                // maxLen was changed from 20 to 17 in bzip2-1.0.3 so try 20
                in->setpos(begin);
                tmp->setpos(0);
                diffFound=dataf.CompareFiles(tmp,in, tmpsize, uint64_t(info=(info&255)+256*20), FCOMPARE);
            }            
            tfail=(diffFound || tmp->getc()!=EOF); 
        }
        // Test fails, compress without transform
        if (tfail) {
            if (verbose>2) printf(" Transform fails at %0lu, skipping...\n", diffFound-1);
            in->setpos(begin);
            Filetype type2;
            if (type==ZLIB || (type==BZIP2))  type2=CMP; else type2=DEFAULT;
            
            direct_encode_blockstream(type2, in, len);
            typenamess[type][it]-=len,  typenamesc[type][it]--;       // if type fails set
            typenamess[type2][it]+=len,  typenamesc[type2][it]++; // default info
        } else {
            tmp->setpos(0);
            if (type==EXE) {
                direct_encode_blockstream(type, tmp, tmpsize);
            } else if (type==DECA || type==ARM) {
                direct_encode_blockstream(type, tmp, tmpsize);
            } else if (type==IMAGE24 || type==IMAGE32) {
                direct_encode_blockstream(type, tmp, tmpsize, info);
            } else if (type==MRBR || type==MRBR4) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=( tmp->getc()<<8)+(tmp->getc());
                Filetype type2 =type==MRBR?IMAGE8:IMAGE4;
                hdrsize=4+hdrsize*4+4;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info);
            } else if (type==RLE) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=( 4);
                Filetype type2 =(Filetype)(info>>24);
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info);
            } else if (type==LZW) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=( 0);
                Filetype type2 =(Filetype)(info>>24);
                tmp->setpos(0);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info&0xffffff);
            } else if (type==GIF) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=(tmp->getc()<<8)+tmp->getc();
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                typenamess[info>>24][it+1]+=tmpsize-hdrsize,  typenamesc[IMAGE8][it+1]++;
                direct_encode_blockstream((Filetype)(info>>24), tmp, tmpsize-hdrsize, info&0xffffff);
            } else if (type==AUDIO) {
                segment->putdata(type,len,info2); //original lenght
                direct_encode_blockstream(type, tmp, tmpsize, info);
            } else if (type==TEXT || type==TXTUTF8 ||type==TEXT0) {
                if ( len>0xA00000) { //if WRT is smaller then original block 
                    if (tmpsize>(len-256) ) {
                        in->setpos( begin);
                        direct_encode_blockstream(NOWRT, in, len); }
                    else
                    direct_encode_blockstream(BIGTEXT, tmp, tmpsize);
                } else if (tmpsize< (len*2-len/2)||len) {
                    // encode as text without wrt transoform, 
                    // this will be done when stream is compressed
                    in->setpos( begin);
                    direct_encode_blockstream(type, in, len);
                } else {
                    // wrt size was bigger, encode as NOWRT and put in bigtext stream.
                    in->setpos(begin);
                    direct_encode_blockstream(NOWRT, in, len);
                }
            }else if (type==EOLTEXT) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=tmp->get32();
                hdrsize=tmp->get32();
                hdrsize=hdrsize+12;
                tmp->setpos(0);
                typenamess[CMP][it+1]+=hdrsize,  typenamesc[CMP][it+1]++; 
                direct_encode_blockstream(CMP, tmp, hdrsize);
                typenamess[TEXT][it+1]+=tmpsize-hdrsize,  typenamesc[TEXT][it+1]++;
                transform_encode_block(TEXT,  tmp, tmpsize-hdrsize,  -1,-1, blstr, it, hdrsize); 
            } else if (streams->GetTypeInfo(type)&TR_RECURSIVE) {
                int isinfo=0;
                if (type==SZDD ||  type==ZLIB  || type==BZIP2) isinfo=info;
                else if (type==WIT) isinfo=winfo;
                segment->putdata(type,tmpsize,isinfo);
                if (type==ZLIB) {// PDF or PNG image && info
                    Filetype type2 =(Filetype)(info>>24);
                    if (it==itcount)    itcount=it+1;
                    int hdrsize=7+5*tmp->getc();
                    tmp->setpos(0);
                    typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                    direct_encode_blockstream(HDR,  tmp, hdrsize);
                    if (info){
                        typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                        transform_encode_block(type2,  tmp, tmpsize-hdrsize,  info&0xffffff,-1, blstr, it, hdrsize); }
                    else{
                        EncodeFileRecursive( tmp, tmpsize-hdrsize,  blstr,it+1);//it+1
                    }
                } else {     
                    EncodeFileRecursive( tmp, tmpsize,  blstr,it+1);//it+1
                    tmp->close();
                    return;
                }    
            }
        }
        tmp->close();  // deletes
    } else {
        
#define tarpad  //remove for filesize padding \0 and add to default stream as hdr        
        //do tar recursion, no transform
        if (type==TAR) {
            int tarl=int(len),tarn=0,blnum=0,pad=0;;
            TARheader tarh;
            char b2[32];
            strcpy(b2, blstr);
            if (b2[0]) strcat(b2, "-");
            while (tarl>0) {
                tarl=tarl-pad;
                U64 savedpos= in->curpos(); 
                in->setpos(savedpos+pad);
                in->blockread( (U8*)&tarh,  sizeof(tarh)  );
                in->setpos(savedpos);
                if (tarend((char*)&tarh)) {
                    tarn=512+pad;
                    if (verbose>2) printf(" %-16s | %-9s |%12.0" PRIi64 " [%0lu - %0lu]",blstr,typenames[BINTEXT],tarn,savedpos,savedpos+tarn-1);
                    typenamess[BINTEXT][it+1]+=tarn,  typenamesc[BINTEXT][it+1]++; 
                    direct_encode_blockstream(BINTEXT, in, tarn);
                }
                else if (!tarchecksum((char*)&tarh))  {
                    quit("tar checksum error\n");
                } else {
                    int a=getoct(tarh.size,12);
                    int b=a-(a/512)*512;
                    if (b) tarn=512+(a/512)*512;
                    else if (a==0) tarn=512;
                    else tarn= a;
                    sprintf(blstr,"%s%d",b2,blnum++);
                    int tarover=512+pad;
                    //if (a && a<=512) tarover=tarover+tarn,a=0,tarn+=512;
                    if (verbose>2) printf(" %-16s | %-9s |%12.0" PRIi64 " [%0lu - %0lu] %s\n",blstr,typenames[BINTEXT],tarover,savedpos,savedpos+tarover-1,tarh.name);
                    typenamess[BINTEXT][it+1]+=tarover,  typenamesc[BINTEXT][it+1]++; 
                    if (it==itcount)    itcount=it+1;
                    direct_encode_blockstream(BINTEXT, in, tarover);
                    pad=0;
                    if (a!=0) {
                        #ifdef tarpad
                        int filenamesize=strlen(tarh.name);
                        U64 ext=0;
                        if( filenamesize>4) for (int i=5;i>0;i--) {
                            U8 ch=tarh.name[filenamesize-i];
                            if (ch>='A' && ch<='Z') ch+='a'-'A';
                            ext=(ext<<8)+ch;
                        }
                        
                        if (filenamesize>3 && (
                                    (ext&0xffff)==0x2E63 ||  // .c
                                    (ext&0xffff)==0x2E68||   //.h
                                    (ext&0xffffffff)==0x2E747874 ||   //.txt
                                    (ext&0xffffffffff)==0x2E68746D6C ||  //.html
                                    (ext&0xffffffff)==0x2E637070 ||   //.cpp
                                    (ext&0xffffff)==0x2E706F // .po
                                    // ((tarh.name[filenamesize-1]=='c' || tarh.name[filenamesize-1]=='h') && tarh.name[filenamesize-2]=='.') ||
                                    //  (tarh.name[filenamesize-4]=='.' && tarh.name[filenamesize-3]=='t' && tarh.name[filenamesize-2]=='x' &&  tarh.name[filenamesize-1]=='t')
                                    )){
                            if (verbose>2) printf(" %-16s | %-9s |%12.0" PRIi64 " [%0lu - %0lu] %s\n",blstr,typenames[TEXT],a,0,a,"direct");
                            direct_encode_blockstream(TEXT, in, a);
                        } else {
                            EncodeFileRecursive(in, a, blstr, 0);
                        }
                        pad=tarn-a; 
                        tarn=a+512;
                        #else
                        EncodeFileRecursive(in, tarn, blstr, 0);
                        pad=0;
                        tarn+=512;
                        #endif
                    }
                }
                tarl-=tarn;
            }
            if (verbose>2) printf("\n");
        } else {
            const int i1=(streams->GetTypeInfo(type)&TR_INFO)?info:-1;
            direct_encode_blockstream(type, in, len, i1);
        }
    }
    
}
