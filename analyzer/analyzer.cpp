//#pragma once
#include "analyzer.hpp"

Analyzer::Analyzer(int it,Filetype p):info(0),remaining(0),typefound(false),lastType(0),iter(it),ptype(p) {
    AddParser( new DefaultParser());
    AddParser( new BMPParser());
    AddParser( new TextParser());
    AddParser( new DECaParser());
    AddParser( new mrbParser());
    AddParser( new EXEParser());
    if (ptype!=ZLIB) {
        AddParser( new zlibParser());      // brute=true, low priority
        AddParser( new zlibParser(false)); // brute=false, high priority
    }
    AddParser( new NesParser());
    AddParser( new MSZIPParser());
    AddParser( new JPEGParser());
    AddParser( new WAVParser());
    AddParser( new PNMParser());
    AddParser( new PDFLzwParser());
    AddParser( new GIFParser());
    AddParser( new dBaseParser());
    if (ptype==ZLIB) {
       AddParser( new pdfBiParser()); // Enable only inside ZLIB block
    }
    AddParser( new AIFFParser());
    AddParser( new ascii85Parser());
    AddParser( new base64_1Parser());  // stream
    AddParser( new base64_2Parser());  // multiline
    AddParser( new MODParser());
    AddParser( new sgiParser());
    AddParser( new TGAParser());
    if (ptype!=CD) {
       AddParser( new cdParser());
    }
    if (ptype!=MDF && ptype!=CD) {
       AddParser( new mdfParser());
    }
    AddParser( new uueParser());
    AddParser( new TIFFParser());
    if (ptype!=TAR) {
        AddParser( new TARParser());
    }
    AddParser( new PNGParser());
    AddParser( new ZIPParser());
    AddParser( new GZIPParser());
    if (ptype!=BZIP2) AddParser( new bzip2Parser());
    AddParser( new SZDDParser());
    AddParser( new MSCFParser());

    emptyType.start=0;
    emptyType.end=0;
    emptyType.info=0;
    emptyType.rpos=0;
    emptyType.type=DEFAULT;
    emptyType.recursive=false;
}

void Analyzer::AddParser(Parser *p) {
    parsers.push_back(p);
}

Analyzer::~Analyzer() {
}
void Analyzer::Status(uint64_t n, uint64_t size) {
    fprintf(stderr,"P%6.2f%%\b\b\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}
bool Analyzer::Detect(File* in, U64 n, int it) {
    const uint64_t BLOCK=0x10000;  // block size 64k
    uint8_t blk[BLOCK];
    Filetype type=DEFAULT;
    bool pri[MAX_PRI]={false};
    typefound=false;
    // Reset all parsers exept recursive
    int recP=0;
    for (size_t j=0; j<parsers.size(); j++) {
        dType t=parsers[j]->getType(0);
        if (t.recursive==false) parsers[j]->Reset(),parsers[j]->state=NONE;
        else recP=j;
        
    }
    if (recP>0)
    for (size_t j=1; j<parsers.size(); j++) {
        if (j!=recP) parsers[j]->state=DISABLE;//,printf("T%d disabled\n",j);
    }
    remaining=n;
    while (remaining) {
        size_t reads=min64(BLOCK, remaining);
        uint64_t ReadIn=in->blockread(&blk[0], reads);
        // Pass the data block to the parsers one by one
        for (size_t j=0; j<parsers.size(); j++) {
            if (parsers[j]->state!=DISABLE) {
                parsers[j]->file_handle = in;  // Give parser access to file for probing
                //printf("T=%d parser %s PARSE\n",j,parsers[j]->name.c_str());
                //open type detection file and load into memory
                DetectState dstate=parsers[j]->Parse(&blk[0],ReadIn,n-remaining,(remaining-ReadIn)==0?true:false);
                if (dstate==INFO) {
                    parsers[j]->state=INFO;
                    dType t=parsers[j]->getType(0);
                    type=t.type;
                    //printf("T=%d INFO %d, %d-%d %s\n",j,t.info,t.start,t.end,parsers[j]->name.c_str());
                    assert(parsers[j]->priority<MAX_PRI);
                    pri[parsers[j]->priority]|=true;   // we have valid header, set priority true
                    // INFO means "might be something" - don't break, let other parsers check too
                } else if (dstate==END){
                    //printf("T=%zu END %s\n",j,parsers[j]->name.c_str());
                    // request current state data
                    parsers[j]->state=END;
                    typefound=true; // we should ignore pri=4 in some cases?
                    //break;
                } else if (dstate==DISABLE) {
                    //printf("T=%d DISABLE\n",j);
                    parsers[j]->state=DISABLE;
                }
            }
        }
        // Test valid parsers and disable lower priority ones that are not detected a new type. 
        // Priority MAX_PRI-1 not tested as it is default
        if (typefound==false){
            for (size_t i=0; i<(MAX_PRI-1); i++) {
                bool p=pri[i];
                if (p) {
                    for (size_t j=0; j<parsers.size(); j++) {
                        if (parsers[j]->priority>=i && parsers[j]->state!=DISABLE && parsers[j]->state!=END&& parsers[j]->state!=INFO) {
                            //if (parsers[j]->state!=NONE) printf("T=%d parser %s DISABLED\n",j,parsers[j]->name.c_str());
                            dType t=parsers[j]->getType(0);
                            if (t.type!=DEFAULT || parsers[j]->priority!=(MAX_PRI-1))parsers[j]->state=DISABLE; // ignore default type or lowest priority
                            //printf("T=%d parser %s DISABLED\n",j,parsers[j]->name.c_str());
                        }
                    }
                    break;
                }
            }
        }
        // We found new type. Is some parser still detecting?
        if (typefound==true) {
            // Scan for overlaping types and report first detected type
            // TODO: multiplie type detection (default-text-xType)
            int largeP=0;int P=MAX_PRI-1;
            uint64_t minP=-1,maxP=0;
            for (size_t j=0; j<parsers.size(); j++) {
                if (parsers[j]->state==INFO || parsers[j]->state==END) { // partial/damaged files?
                    dType t=parsers[j]->getType(0);
                    // Clip jend to file size if parser set it larger
                    if (t.end > n) t.end = n;
                    //printf("T=%zu parser %s start=%lu end=%lu pri=%d\n",j,parsers[j]->name.c_str(),t.start,t.end,parsers[j]->priority);
                    if (t.end && (t.start<=minP || P>parsers[j]->priority)) {
                        if (largeP>0) parsers[largeP]->state=DISABLE;
                        P=parsers[j]->priority;
                        largeP=j;
                        maxP=t.end;
                        minP=t.start;
                        //printf("largeP=%zu set to %s\n",largeP,parsers[j]->name.c_str());
                    }
                }
            }
            for (size_t j=0; j<parsers.size(); j++) {
                if (parsers[j]->state==END && j==largeP) { // partial/damaged files?
                    dType d=parsers[0]->getType(0);
                    dType t=parsers[j]->getType(0);
                    t.pinfo=parsers[j]->pinfo;
                    d.pinfo=parsers[0]->pinfo;
                    int p=parsers[j]->priority;
                    // Do we have default type? If so add.
                    if (d.end>0 && t.start!=0) {
                        d.end=t.start;
                        types.push_back(d);
                        //printf("T=%d parser default\n Start %d,%d\n",0,d.start,d.end);
                    } else {
                        d.end=0;
                    }
                    // Look for all parsers still detecting
                    // and limit type end to start of any good priority parser still detecting
                    /*for (size_t j=0; j<parsers.size(); j++) {
                        if (parsers[j]->state==INFO && parsers[j]->priority<=p) {
                            dType det=parsers[j]->getType(0);
                            if (t.end>det.start && det.start) {
                                t.end=det.start;
                                printf("T=%d parser %s LIMITED\n",j,parsers[j]->name.c_str());
                            }
                        }
                    }*/
                    //printf("T=%d parser %s\n Start %d,%d\n",j,parsers[j]->name.c_str(),t.start,t.end);
                    // Clip jend to file size before adding
                    // For GZIP, subtract 8 for the CRC32+ISIZE trailer
                    if (t.type == GZIP && t.end > n - 8) t.end = n - 8;
                    else if (t.end > n) t.end = n;
                    types.push_back(t);
                    return true;
                }
            }
        }
        //
        remaining-=ReadIn;
        Status(n-remaining,n);
    }
    // Nothing found so add default type
    dType def=parsers[0]->getType(0);
    def.pinfo=parsers[0]->pinfo;
    def.end=n;
    types.push_back(def);
    //printf("T=%d parser default\n Start %d,%d\n",0,def.start,def.end);
    return true;
}

dType Analyzer::GetNext() {
    if (types.size()>lastType) {
        currentType=types[lastType];
        lastType++;
        return currentType;
    } else {
        lastType=0;
        types.clear();
        return emptyType;
    }
}

std::string &Analyzer::GetInfo() {
    return *pinfo;
}
