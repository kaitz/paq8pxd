#pragma once
#include "wrt.hpp"
#include "../types.hpp"
#include "../helper.hpp" 
// based on  "XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com"
//
#include <stdlib.h> 
#include <memory.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <string.h>



class wrtDecoder
{
public:
    wrtDecoder(); 
    ~wrtDecoder();
    int WRT_start_decoding(int count,int size,U8* data);
    int WRT_decode(int WRTd_c,U8 *decodedText,int *txtlen);
    void defaultSettings(int n);
protected:
    
    inline void stringHash(const U8 *ptr, int len,int& hash);
    int addWord(U8* &mem,int &i);
    U8* loadDynamicDictionary(U8* mem,U8* mem_end);
    
    void initializeCodeWords(int word_count,bool initMem=true);
    bool initialize(bool encoding);
    void WRT_deinitialize();

    int* word_hash;
    bool fileCorrupted;


    std::vector<std::string> sortedDict;

    int sizeDict;
    U8* dictmem;
    U8* dictmem_end;
    U8* mem;
    
U8** dict=NULL;
U8* dictlen=NULL;
    int addSymbols[256]; // reserved symbols in output alphabet 
    int reservedSet[256]; // reserved symbols in input alphabet
    int outputSet[256];
    int codeword2sym[256]; 

    int dictionary,dict1size,dict2size,dict3size,dict4size,dict1plus2plus3,dict1plus2;
    int bound4,bound3,dict123size,dict12size,collision,quoteOpen,quoteClose,detectedSym;
    int maxMemSize;
    int sortedDictSize;
    //int wrtnum;
    int dindex;
    void read_dict(int count,int size,U8* data);
    inline int decodeCodeWord(U8* &s,int& c);
    int s_size;
int more;//=-1
public:
};
   



// decode word using dictionary
#define DECODE_WORD1(i)\
    {\
        i++;\
        if (i>0 && i<sizeDict)\
        {\
            PRINT_CODEWORDS(("i=%d ",i)); \
            s_size=dictlen[i];\
            memcpy(s,dict[i],s_size+1);\
            PRINT_CODEWORDS(("%s\n",dict[i])); \
        }\
        else\
        {\
            s_size=0; \
            /*printf("File is corrupted %d/%d!\n",i,sizeDict);*/\
            fileCorrupted=true;\
        }\
    }


 

