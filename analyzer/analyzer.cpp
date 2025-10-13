//#pragma once
#include "analyzer.hpp"

Analyser::Analyser():info(0),remaining(0),typefound(false),lastType(0) {
    AddParser( new DefaultParser());
    AddParser( new BMPParser());
    AddParser( new TextParser());
    AddParser( new DECaParser());
    AddParser( new mrbParser());
    AddParser( new EXEParser());
    AddParser( new zlibParser());      // brute=true, low priority
    AddParser( new zlibParser(false)); // brute=false, high priority
    AddParser( new NesParser());
    AddParser( new MSZIPParser());
    AddParser( new JPEGParser());
    AddParser( new WAVParser());
    AddParser( new PNMParser());
    AddParser( new PDFLzwParser());
    AddParser( new GIFParser());
    AddParser( new dBaseParser());
    
    emptyType.start=0;
    emptyType.end=0;
    emptyType.info=0;
    emptyType.rpos=0;
    emptyType.type=DEFAULT;
    emptyType.recursive=false;
}

void Analyser::AddParser(Parser *p) {
    parsers.push_back(p);
}

Analyser::~Analyser() {
}

bool Analyser::Detect(File* in, U64 n, int it) {
    const int BLOCK=0x10000;  // block size 64k
    uint8_t blk[BLOCK];
    Filetype type=DEFAULT;
    bool pri[5]={false};
    typefound=false;
    // Reset all parsers exept recursive
    for (size_t j=0; j<parsers.size(); j++) {
        dType t=parsers[j]->getType(0);
        if (t.recursive==false) parsers[j]->Reset();
    }
    remaining=n;
    while (remaining) {
        size_t reads=min(BLOCK, remaining);
        int ReadIn=in->blockread(&blk[0], reads);
        // Pass the data block to the parsers one by one
        for (size_t j=0; j<parsers.size(); j++) {
            if (parsers[j]->state!=DISABLE) {
                //printf("T=%d parser %s PARSE\n",j,parsers[j]->name.c_str());
                //open type detection file and load into memory
                DetectState dstate=parsers[j]->Parse(&blk[0],BLOCK,n-remaining);
                if (dstate==INFO) {
                    parsers[j]->state=INFO;
                    dType t=parsers[j]->getType(0);
                    type=t.type;
                    //printf("T=%d INFO %d, %d-%d\n",j,t.info,t.start,t.end);
                    pri[parsers[j]->priority]|=true;   // we have valid header, set priority true
                } else if (dstate==END){
                    //printf("T=%d END\n",j);
                    // request current state data
                    parsers[j]->state=END;
                    typefound=true; // we should ignore pri=4 in some cases?
                    break;
                } else if (dstate==DISABLE) {
                    //printf("T=%d DISABLE\n",j);
                    parsers[j]->state=DISABLE;
                }
            }
        }
        // Test valid parsers and disable lower priority ones that are not detected a new type. 
        // Priority 4 not tested as it is default
        if (typefound==false){
            for (size_t i=0; i<4; i++) {
                bool p=pri[i];
                if (p) {
                    for (size_t j=0; j<parsers.size(); j++) {
                        if (parsers[j]->priority>=i && parsers[j]->state!=DISABLE && parsers[j]->state!=END&& parsers[j]->state!=INFO) {
                            parsers[j]->state=DISABLE;
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
            // TODO: look for largest detected type, not first
            // TODO: trim low priority types and add but do not report if higher priority type has returned info.
            for (size_t j=0; j<parsers.size(); j++) {
                if (parsers[j]->state==END) { // partial/damaged files?
                    dType d=parsers[0]->getType(0);
                    dType t=parsers[j]->getType(0);
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
                    for (size_t j=0; j<parsers.size(); j++) {
                        if (parsers[j]->state==INFO /*&& parsers[j]->priority<=p*/) {
                            dType det=parsers[j]->getType(0);
                            if (t.end>det.start && det.start) {
                                t.end=det.start;
                                //printf("T=%d parser %s LIMITED\n",j,parsers[j]->name.c_str());
                            }
                        }
                    }
                    //printf("T=%d parser %s\n Start %d,%d\n",j,parsers[j]->name.c_str(),t.start,t.end);
                    types.push_back(t);
                    return true;
                }
            }
        }
        //
        remaining-=ReadIn;
    }
    // Nothing found so add default type
    dType def=parsers[0]->getType(0);
    def.end=n;
    types.push_back(def);
    //printf("T=%d parser default\n Start %d,%d\n",0,def.start,def.end);
    return true;
}

dType Analyser::GetNext() {
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
