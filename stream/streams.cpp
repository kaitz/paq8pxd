#include "streams.hpp"

Streams::Streams():streams(0){
    //Create streams
    Add( new Stream({STR_NONE,"empty"}));
    Add( new Stream({STR_DEFAULT,"default"}));
    Add( new Stream({STR_JPEG,"jpeg"}));
    Add( new Stream({STR_IMAGE1,"image1"}));
    Add( new Stream({STR_IMAGE4,"image4"}));
    Add( new Stream({STR_IMAGE8,"image8"}));
    Add( new Stream({STR_IMAGE24,"image24"}));
    Add( new Stream({STR_AUDIO,"audio"}));
    Add( new Stream({STR_EXE,"exe"}));
    Add( new Stream({STR_TEXT0,"text0 wrt"}));
    Add( new Stream({STR_TEXT,"text wrt"}));
    Add( new Stream({STR_BIGTEXT,"bigtext wrt"}));
    Add( new Stream({STR_DECA,"dec"}));
    Add( new Stream({STR_CMP,"compressed"}));
    //assert(streams.size()==(STR_LAST-1));
    // Add data types accepted by a stream 
    //STR_NONE
    streams[0]->dataType.push_back({CD,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({BASE64,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({BASE85,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({UUENC,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({GIF,TR_NONE|TR_TRANSFORM});
    streams[0]->dataType.push_back({SZDD,TR_INFO|TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({MRBR,TR_NONE|TR_TRANSFORM});
    streams[0]->dataType.push_back({MRBR4,TR_NONE|TR_TRANSFORM});
    streams[0]->dataType.push_back({RLE,TR_NONE|TR_TRANSFORM});
    streams[0]->dataType.push_back({LZW,TR_NONE|TR_TRANSFORM});
    streams[0]->dataType.push_back({BZIP2,TR_INFO|TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({ZLIB,TR_INFO|TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({PREFLATE,TR_INFO|TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({ZIP,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({GZIP,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({MDF,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({EOLTEXT,TR_RECURSIVE|TR_TRANSFORM});
    streams[0]->dataType.push_back({TAR,TR_NONE|TR_DIRECT});
    streams[0]->dataType.push_back({WIT,TR_INFO|TR_RECURSIVE|TR_TRANSFORM});
    //STR_DEFAULT
    streams[1]->dataType.push_back({DEFAULT,TR_NONE});
    streams[1]->dataType.push_back({BINTEXT,TR_INFO});
    streams[1]->dataType.push_back({DBASE,TR_INFO});
    streams[1]->dataType.push_back({HDR,TR_NONE});
    streams[1]->dataType.push_back({IMGUNK,TR_INFO});
    streams[1]->dataType.push_back({ARM,TR_NONE|TR_TRANSFORM});
    streams[1]->dataType.push_back({NESROM,TR_NONE});
    //STR_JPEG
    streams[2]->dataType.push_back({JPEG,TR_NONE});
    //STR_IMAGE1
    streams[3]->dataType.push_back({IMAGE1,TR_INFO});
    //STR_IMAGE4
    streams[4]->dataType.push_back({IMAGE4,TR_INFO});
    //STR_IMAGE8
    streams[5]->dataType.push_back({IMAGE8,TR_INFO});
    streams[5]->dataType.push_back({IMAGE8GRAY,TR_INFO});
    streams[5]->dataType.push_back({PNG8,TR_INFO|TR_TRANSFORM});
    streams[5]->dataType.push_back({PNG8GRAY,TR_INFO|TR_TRANSFORM});
    //STR_IMAGE24
    streams[6]->dataType.push_back({IMAGE24,TR_INFO|TR_TRANSFORM});
    streams[6]->dataType.push_back({IMAGE32,TR_INFO|TR_TRANSFORM});
    streams[6]->dataType.push_back({PNG24,TR_INFO|TR_TRANSFORM});
    streams[6]->dataType.push_back({PNG32,TR_INFO|TR_TRANSFORM});
    //STR_AUDIO
    streams[7]->dataType.push_back({AUDIO,TR_INFO});
    //STR_EXE
    streams[8]->dataType.push_back({EXE,TR_NONE|TR_TRANSFORM});
    //STR_TEXT0
    streams[9]->dataType.push_back({TEXT0,TR_NONE|TR_TRANSFORM});
    //STR_TEXT
    streams[10]->dataType.push_back({ISOTEXT,TR_NONE|TR_TRANSFORM});
    streams[10]->dataType.push_back({TEXT,TR_NONE|TR_TRANSFORM});
    streams[10]->dataType.push_back({TXTUTF8,TR_NONE|TR_TRANSFORM});
    streams[10]->dataType.push_back({DICTTXT,TR_NONE});
    //STR_BIGTEXT
    streams[11]->dataType.push_back({BIGTEXT,TR_NONE|TR_TRANSFORM});
    streams[11]->dataType.push_back({NOWRT,TR_NONE});
    //STR_DECA
    streams[12]->dataType.push_back({DECA,TR_NONE|TR_TRANSFORM});
    //STR_CMP
    streams[13]->dataType.push_back({CMP,TR_NONE});
    streams[13]->dataType.push_back({MSZIP,TR_NONE});
} 

Streams::~Streams(){
}

void Streams::Add(Stream *s){
    streams.push_back(s);
}

int Streams::Count() {
    return streams.size();
}

Streamtype Streams::GetStreamID(Filetype type) {
    assert(type<TYPELAST);
    size_t s=Count();
    for(size_t i=0;i<s; i++) {
        size_t size=streams[i]->dataType.size();
        for(size_t j=0; j<size; j++) {
            if (streams[i]->dataType[j].type==type) return streams[i]->id;
       }
    }
    return STR_NONE;    
}

bool Streams::isStreamType(Filetype type,int id) {
    assert(id<Count());
    assert(type<TYPELAST);
    if (GetStreamID(type)==id) 
        return true;
    else
        return false;
}

int Streams::GetTypeInfo(Filetype type) {
    assert(type<TYPELAST);
    size_t s=Count();
    for(size_t i=0; i<s; i++) {
        size_t size=streams[i]->dataType.size();
        for(size_t j=0; j<size; j++) {
            if (streams[i]->dataType[j].type==type) return streams[i]->dataType[j].info;
        }
    }
    return TR_NONE;    
}

  File& Streams::GetStreamFile(Streamtype id) {
      return streams[int(id)]->file;
  }

