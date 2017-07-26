#if !defined common_h_included
#define common_h_included

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "MemBuffer.h"


class XWRT_Common
{
public:
	XWRT_Common(int fileBufferSize=17); // 128 kb
	~XWRT_Common();

	void defaultSettings();
	unsigned int flen( FILE* &f );
	
	CContainers cont;
	int preprocFlag;

protected:
	
	inline void stringHash(const unsigned char *ptr, int len,int& hash);
	int addWord(unsigned char* &mem,int &i);
	unsigned char* loadDynamicDictionary(unsigned char* mem,unsigned char* mem_end);
	void initializeLetterSet();
	void initializeCodeWords(int word_count,bool initMem=true);
	bool initialize(bool encoding);
	void WRT_deinitialize();

	void WRT_print_options();
	int minSpacesFreq();

	int* word_hash;
	bool decoding,fileCorrupted,detect,firstWarn;
	int maxDynDictBuf,minWordFreq,maxDictSize;
	int tryShorterBound,spaces,fileLenMB,beforeWord;
	int spacesCodeword[256];
	int spacesCont[256];
	std::vector<std::string> sortedDict;

	ELetterType letterType;
	ELetterType letterSet[256];

	int sizeDict,sizeDynDict;
	unsigned char* dictmem;
	unsigned char* dictmem_end;
	unsigned char* mem;
	
	int addSymbols[256]; // reserved symbols in output alphabet 
	int reservedSet[256]; // reserved symbols in input alphabet
	int outputSet[256];
	int wordSet[256]; 
	int sym2codeword[256]; 
	int codeword2sym[256]; 

	int dictionary,dict1size,dict2size,dict3size,dict4size,dict1plus2plus3,dict1plus2;
	int bound4,bound3,dict123size,dict12size,collision,quoteOpen,quoteClose,detectedSym;
	int maxMemSize;
	int sortedDictSize;
	

public:
	
};

size_t fread_fast(unsigned char* dst, int len, FILE* file);
size_t fwrite_fast(unsigned char* dst, int len, FILE* file);

extern unsigned char** dict;
extern int* dictfreq;
extern unsigned char* dictlen;

#endif
