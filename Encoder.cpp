#include "XWRT.h"
#if defined WIN32 || defined WIN64
	#include <windows.h>
	#include <conio.h>
#endif
 
#include "Common.h"
#include "MemBuffer.h"
#include "Encoder.h"


XWRT_Encoder::XWRT_Encoder() :  last_c_bak(0),filelento(0)
{ 	
};
XWRT_Encoder::~XWRT_Encoder() 
{ 
	
}
#define ENCODE_PUTC(c)\
{ \
	if (!detect) \
	{ \
		if (cont.memout->memsize>maxMemSize) \
		{ \
			PRINT_DICT(("%d maxMemSize=%d\n",cont.memout->memsize,maxMemSize)); \
			cont.writeMemBuffers(preprocFlag); \
			cont.memout->memsize=0; \
		} \
 \
		PRINT_CHARS(("output=%d (%c)\n",c,c)); \
		cont.memout->OutTgtByte(c); \
	} \
}

#define ENCODE_GETC(c) \
{ \
	last_last_c=last_c; \
	last_c=last_c_bak; \
 \
 	c=getc(XWRT_file); \
 filelento++;\
	last_c_bak=c; \
}


// encode word (should be lower case) using n-gram array (when word doesn't exist in the dictionary)
inline void XWRT_Encoder::encodeAsText(unsigned char* &s,int &s_size,EWordType wordType)
{
	int i=0;
		for (i=0; i<s_size; i++)
		{
			if (addSymbols[s[i]])
				ENCODE_PUTC(CHAR_ESCAPE);
			ENCODE_PUTC(s[i]);
		}
		return;
}
inline void XWRT_Encoder::encodeCodeWord(int &i)
{
	int first,second,third,fourth;
	first=i-1;
	if (first>=bound4)
	{
		first-=bound4;
		fourth=first/dict123size;
		first=first%dict123size;
		third=first/dict12size;		
		first=first%dict12size;
		second=first/dict1size;		
		first=first%dict1size;
		ENCODE_PUTC(sym2codeword[dict1plus2plus3+fourth]);
		PRINT_CODEWORDS(("1st=%d ",sym2codeword[dict1plus2plus3+fourth]));
		ENCODE_PUTC(sym2codeword[dict1plus2+third]);
		PRINT_CODEWORDS(("2nd=%d ",sym2codeword[dict1plus2+third]));
		ENCODE_PUTC(sym2codeword[dict1size+second]);
		PRINT_CODEWORDS(("3rd=%d ",sym2codeword[dict1size+second]));
		ENCODE_PUTC(sym2codeword[first]);
		PRINT_CODEWORDS(("4th=%d ",sym2codeword[first]));
	}
	else
	if (first>=bound3)
	{
		first-=bound3;
		third=first/dict12size;		
		first=first%dict12size;
		second=first/dict1size;		
		first=first%dict1size;
		ENCODE_PUTC(sym2codeword[dict1plus2+third]);
		PRINT_CODEWORDS(("1st=%d(%d) ",sym2codeword[dict1plus2+third],third));
		ENCODE_PUTC(sym2codeword[dict1size+second]);
		PRINT_CODEWORDS(("2nd=%d(%d) ",sym2codeword[dict1size+second],second));
		ENCODE_PUTC(sym2codeword[first]);
		PRINT_CODEWORDS(("3rd=%d(%d) ",sym2codeword[first],first));
	}
	else
		if (first>=dict1size)
		{
			first-=dict1size;
			second=first/dict1size;		
			first=first%dict1size;
			ENCODE_PUTC(sym2codeword[dict1size+second]);
			PRINT_CODEWORDS(("1st=%d ",sym2codeword[dict1size+second]));
	
			ENCODE_PUTC(sym2codeword[first]);
			PRINT_CODEWORDS(("2nd=%d ",sym2codeword[first]));
		}
		else
		{
			ENCODE_PUTC(sym2codeword[first]);
			PRINT_CODEWORDS(("1st=%d ",sym2codeword[first]));
		}
		PRINT_CODEWORDS((" no=%d %s\n", no-1,dict[no]));
}

inline void XWRT_Encoder::encodeSpaces()
{
	if (spaces==1)
	{
		ENCODE_PUTC(' ');
	}
	else
		if (spaces>0)
		{
			while (spaces>0)
			{
				int sp=spaces;
				if (spaces>=256)
					sp=255;
				
				while (sp>0 && spacesCodeword[sp]==0) sp--;
				if (spacesCodeword[sp])
				{		
					encodeCodeWord(spacesCodeword[sp]);
					spaces-=sp;
				}
				else
				{
					{
						ENCODE_PUTC(' ');
						spaces--;
					}
				}
			}
		}
	spaces=0;
}
// make hash from string
inline void XWRT_Encoder::stringHash(const unsigned char *ptr, int len,int& hash)
{
	for (hash = 0; len>0; len--, ptr++)
	{
		hash *= HASH_MULT;
		hash += *ptr;
	}
	hash=hash&(HASH_TABLE_SIZE-1);
}
// check if word "s" does exist in the dictionary 
inline void XWRT_Encoder::checkHashExactly(unsigned char* &s,int &s_size,int& i)
{
	int h;
	stringHash(s,s_size,h);
	i=word_hash[h];
	if (i>0)
	{
		if (dictlen[i]!=s_size || memcmp(dict[i],s,s_size)!=0)
		{
			i=word_hash[(h+s_size*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
			if (i>0)
			{
				if (dictlen[i]!=s_size || memcmp(dict[i],s,s_size)!=0)
				{
					i=word_hash[(h+s_size*HASH_DOUBLE_MULT*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
					if (i>0)
					{
						if (dictlen[i]!=s_size || memcmp(dict[i],s,s_size)!=0)
							i=-1;
					}
					else
						i=-1;
				}
			}
			else
				i=-1;
		}
	}
	else
		i=-1;
	if (i>=dictionary)
		i=-1;
}
// check if word "s" (prefix of original word) does exist in the dictionary using hash "h" 
inline int XWRT_Encoder::checkHash(unsigned char* &s,int &s_size,int h)
{
	int i=word_hash[h];
	if (i>0)
	{
		if (dictlen[i]>s_size || memcmp(dict[i],s,s_size)!=0)
		{
			i=word_hash[(h+s_size*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
			if (i>0)
			{
				if (dictlen[i]>s_size || memcmp(dict[i],s,s_size)!=0)
				{
					i=word_hash[(h+s_size*HASH_DOUBLE_MULT*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
					if (i>0)
					{
						if (dictlen[i]>s_size || memcmp(dict[i],s,s_size)!=0)
							i=-1;
					}
					else
						i=-1;
				}
			}
			else
				i=-1;
		}
	}
	else
		i=-1;
	if (i>=dictionary)
		i=-1;
	return i;
}
// check if word "s" or prefix of word "s" does exist in the dictionary using hash "h" 
inline int XWRT_Encoder::findShorterWord(unsigned char* &s,int &s_size)
{
	int ret;
	int i;
	int best;
	unsigned int hash;
	hash = 0;
	for (i=0; i<WORD_MIN_SIZE+tryShorterBound; i++)
		hash = HASH_MULT * hash + s[i];
 
	best=-1;
	for (i=WORD_MIN_SIZE+tryShorterBound; i<s_size; i++)
	{
		ret=checkHash(s,i,hash&(HASH_TABLE_SIZE-1));	
		if (ret>=0)
			best=ret;
		hash = HASH_MULT*hash + s[i];
	}
	return best;
}
// convert lower string to upper
inline void XWRT_Encoder::toUpper(unsigned char* s,int &s_size)
{
	for (int i=0; i<s_size; i++)
		s[i]=toupper(s[i]); 
}
// convert upper string to lower
inline void XWRT_Encoder::toLower(unsigned char* s,int &s_size)
{
	for (int i=0; i<s_size; i++)
		s[i]=tolower(s[i]);
}
void XWRT_Encoder::encodeMixed(unsigned char* s,int s_size,int& old_c)
{
	int c,size,start,ptr=0;
	EWordType wordType;
	unsigned char* s2;
	do
	{
		start=ptr;
		do
		{
			c=s[ptr++];
			letterType=letterSet[c];
		}
		while (ptr<s_size && letterType==NUMBERCHAR);
		
		if (letterType!=NUMBERCHAR)
			ptr--;
		wordType=NUMBER;
		encodeWord(s+start,ptr-start,wordType,old_c);
		
		if (ptr>=s_size)
			break;
		
		start=ptr;
		do
		{
			c=s[ptr++];
			letterType=letterSet[c];
		}
		while (ptr<s_size && letterType!=NUMBERCHAR);
		
		if (letterType==NUMBERCHAR)
			ptr--;
		wordType=VARWORD;
		s2=s+start;
		size=ptr-start;
		encodeAsText(s2,size,wordType);
	}
	while (ptr<s_size);
}
// encode word "s" using dictionary
void XWRT_Encoder::encodeWord(unsigned char* s,int s_size,EWordType wordType,int& c)
{
	if (detect)
	{
	toLower(s,s_size);
		checkWord(s,s_size,c);
		return;
	}
	if (s_size<1)
	{
		encodeSpaces();
		return;
	}
	int i=-1;
	int size=0;
	int flagToEncode=-1;
	bool justAdded=false;
	
	if (s_size>=WORD_MIN_SIZE)
	{
		
		checkHashExactly(s,s_size,i);
		PRINT_CODEWORDS(("i=%d/%d %s(%d)\n",i,sizeDynDict,s,s_size));
			
		if (i>=0)// && codeWordSize(i)<=s_size)
			wordType=LOWERWORD;
		
		if (i<0)
		{
			if (wordType==FIRSTUPPER || wordType==UPPERWORD)
			{
				if (wordType==FIRSTUPPER)
				{
					flagToEncode=CHAR_FIRSTUPPER;
					s[0]=tolower(s[0]);
				}
				else // wordType==UPPERWORD
				{
					flagToEncode=CHAR_UPPERWORD;
					toLower(s,s_size);
				}
				checkHashExactly(s,s_size,i);
				PRINT_CODEWORDS(("checkHashExactly i=%d %d=%s\n",i,s_size,s));
			}
			
			
			if (i<0 ) //&& IF_OPTION(OPTION_TRY_SHORTER_WORD))
			{
				// try to find shorter version of word in dictionary
				i=findShorterWord(s,s_size);
				PRINT_CODEWORDS(("findShorterWord i=%d\n",i));
				
				if (i>=0)
				{
					size=dictlen[i];
					
					if (wordType==UPPERWORD)
					{
						int ss=s_size-size;
						toUpper(s+size,ss);
					}
				}
			}
		}
	}
	if (i>=0)
	{
		
		encodeSpaces();
		if (wordType==FIRSTUPPER || wordType==UPPERWORD)
		{
			ENCODE_PUTC(flagToEncode);
		}
		encodeCodeWord(i);
		if (size>0)
		{
			if (wordType==FIRSTUPPER)
				wordType=LOWERWORD;
			unsigned char* s2=s+size;
			int s_size2=s_size-size;
			encodeAsText(s2,s_size2,wordType);
		}
	}
	else
	{
				if (wordType==FIRSTUPPER)
					s[0]=toupper(s[0]);
				else if (wordType==UPPERWORD)
					toUpper(s,s_size);
		encodeSpaces();
		encodeAsText(s,s_size,wordType);
	}
	
	return;
}
// process the file
void XWRT_Encoder::WRT_encode(int filelen)
{

	unsigned char s[STRING_MAX_SIZE];
	EWordType wordType;
	int c;
	spaces=0;
	s_size=0;
	last_c=0;
	filelento=-1;
	wordType=LOWERWORD;
	ENCODE_GETC(c);
	while (true) 
	{
		if (filelento==filelen)
			break;
		PRINT_CHARS(("c=%c (%d) last=%c \n",c,c,last_c));
		
		if (detect)
		{
			letterType=letterSet[c];
		}
		else
		{
			if (c==13)
			{
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				ENCODE_GETC(c);
				
				{
					if (addSymbols[13])
						ENCODE_PUTC(CHAR_ESCAPE);
					ENCODE_PUTC(13);
				}
				continue;
			}
			
			letterType=letterSet[c];
			
			if (letterType==RESERVEDCHAR)
			{
				PRINT_CHARS(("reservedSet[c] c=%d (%c)\n",c,c));
				
				encodeWord(s,s_size,wordType,c);
				s_size=0;
		
				PRINT_CHARS(("out CHAR_ESCAPE=%d\n",CHAR_ESCAPE));
				ENCODE_PUTC(CHAR_ESCAPE);	
				ENCODE_PUTC(c);
				
				ENCODE_GETC(c);
				continue;
			}
			
			
			if (letterType==NUMBERCHAR) 
			{	
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				ENCODE_PUTC(c);
				ENCODE_GETC(c);
				//	wordType=LOWERWORD;
				continue;
			}
			
			
			
		}
		if (wordSet[c])
		{
			if (c!=' ')
			{
				if (s_size==0)
				{
					if (last_c!=' ')
						beforeWord=last_c;
					else
						beforeWord=last_last_c;
					if (letterType==LOWERCHAR)
						wordType=LOWERWORD;
					else
					if (letterType==UPPERCHAR)
						wordType=FIRSTUPPER;
					else
						wordType=VARWORD;
				}
				else
				{
					switch (wordType)
					{
						case LOWERWORD:
							if (letterType!=LOWERCHAR)
								wordType=VARWORD;
							break;
						case UPPERWORD:
							if (letterType!=UPPERCHAR)
								wordType=VARWORD;
							break;
						case FIRSTUPPER:
							if (letterType!=LOWERCHAR)
							{
								if (s_size==1 && letterType==UPPERCHAR)
									wordType=UPPERWORD;
								else
									wordType=VARWORD;
							}
							break;
					}
				}
			}
			else
			{
				encodeWord(s,s_size,wordType,c);
				s_size=0;
					
				spaces++;
				while (true) 
				{
					ENCODE_GETC(c);
					if (c!=' ')
						break;
					spaces++;
				}
				continue;
			}
			s[s_size++]=c;
			if (s_size>=STRING_MAX_SIZE-2)
			{
				encodeWord(s,s_size,wordType,c);
				s_size=0;
			}
			ENCODE_GETC(c);
			continue;
		}
	
		encodeWord(s,s_size,wordType,c);
		s_size=0;
		
			ENCODE_PUTC(c);
 
		ENCODE_GETC(c);
	}
	encodeWord(s,s_size,wordType,c);
	s_size=0;
}
inline int common(const char* offset1,const char* offset2, int bound)
{
	int lp=0;
	while (offset1[lp]==offset2[lp] && lp<bound)
		lp++;
	return lp;
}
void XWRT_Encoder::write_dict()
{
	int i,count=0;
	unsigned char *bound=(unsigned char*)&word_hash[0]+HASH_TABLE_SIZE*sizeof(word_hash[0])-WORD_MAX_SIZE;
	unsigned char *writeBuffer=(unsigned char*)&word_hash[0]; //putcBuffer;
	unsigned char *bufferData=writeBuffer+3;
	
	unsigned char *count_header=bufferData;
	bufferData+=3;
	PRINT_DICT(("sortedDict.size()=%d\n",sortedDict.size()));
	int cmn;
	count=(int)sortedDict.size();
	for (i=0; i<count; i++)
	{
		cmn=0;
		//if (i>0)
		//	cmn=common(sortedDict[i-1].c_str(),sortedDict[i].c_str(),min(sortedDict[i].size(),sortedDict[i-1].size()));
		if ((cmn>0 || (unsigned char)(sortedDict[i][0])>=128))
			bufferData+=sprintf((char*)bufferData,"%c%s\n",128+cmn,sortedDict[i].c_str()+cmn);
		else
			bufferData+=sprintf((char*)bufferData,"%s\n",sortedDict[i].c_str());
		if (bufferData>bound)
			break;
	}
	sortedDictSize=(int)i; // i<=count
	PRINT_DICT(("sortedDictCount=%d\n",sortedDictSize));
	count_header[0]=sortedDictSize%256;
	count_header[1]=(sortedDictSize/256)%256;
	count_header[2]=sortedDictSize/65536;
	count=(int)(bufferData-(writeBuffer+3));
	PRINT_DICT(("write_dict count=%d\n",count));
	{
		PUTC(count>>16);
		PUTC(count>>8);
		PUTC(count);
		fwrite_fast((unsigned char*)writeBuffer+3,count,XWRT_fileout);
	}
}

void XWRT_Encoder::WRT_start_encoding(FILE* in, FILE* out,unsigned int fileLen,bool type_detected)
{
	collision=0;
XWRT_file=in;
XWRT_fileout=out;

	fileLenMB=fileLen/(1024*1024);
	if (fileLenMB>255*256)
		fileLenMB=255*256;
	
	cont.prepareMemBuffers();
	cont.memout->memsize=0;
/*	if (fileLenMB>64) minWordFreq-=16;
	if (fileLenMB<32)
		minWordFreq+=3;
	if (fileLenMB<6)
		minWordFreq=minWordFreq+15;
if (fileLenMB<1)
		minWordFreq=minWordFreq*2;*/
		int pos=ftell(XWRT_file);
	if (!type_detected)
		WRT_detectFileType(fileLen);

	fseek(XWRT_file, pos, SEEK_SET );

	PUTC(maxMemSize/(1024*1024));
	PUTC(fileLen&0xFF);
	PUTC((fileLen>>8)&0xFF);
	PUTC((fileLen>>16)&0xFF);
	PUTC((fileLen>>24)&0xFF);

	PRINT_DICT(("maxMemSize=%d fileLenMB=%d\n",maxMemSize,fileLenMB));
	write_dict(); // przed initialize()
	decoding=false;
	WRT_deinitialize();
	if (!initialize(true))
		return;
	WRT_encode(fileLen);
	cont.writeMemBuffers(preprocFlag);
	cont.freeMemBuffers(true);
}
 
inline void XWRT_Encoder::checkWord(unsigned char* &s,int &s_size,int& c)
{
	if (s_size<1)
	{
		spaces=0;
		return;
	}
	if (s_size>WORD_MAX_SIZE)
		s_size=WORD_MAX_SIZE; 
	
		spaces=0;
	if (s_size<WORD_MIN_SIZE)
	{
		spaces=0;
		return;
	} 
	int i;
	checkHashExactly(s,s_size,i);
	if (i<0)
	{
		if (dynmem>dictbound)
		{
			if (firstWarn)
			{
				//printf("warning: dictionary too big\n"); //-b option
				firstWarn=false;
			}
			return;
		}
		memcpy(dynmem,s,s_size);
		if (addWord(dynmem,s_size)==1)
		{
			dynmem+=(s_size/4+1)*4;
			dictfreq[sizeDict-1]=1;
		}
	}
	else
	{
		dictfreq[i]++;
	}
}
int XWRT_Encoder::WRT_detectFileType(int filelen)
{
	detect=true;
	//memset(value,0,sizeof(value));
	memset(addSymbols,0,sizeof(addSymbols));
	memset(reservedSet,0,sizeof(reservedSet));
	memset(spacesCont,0,sizeof(spacesCont));
	spaces=0;
	firstWarn=true;
	sizeDict=1;
	PRINT_DICT(("maxDynDictBuf=%d maxMemSize=%d\n",maxDynDictBuf,maxMemSize));
	dictionary=maxDynDictBuf*(MAX_DYNAMIC_DICT_COUNT/256);  // 512k, dblp=372k
	dictmem=(unsigned char*)calloc(dictionary*WORD_AVG_SIZE,1);
	dictbound=dictmem+dictionary*WORD_AVG_SIZE-WORD_MAX_SIZE;
	dict=(unsigned char**)calloc(sizeof(unsigned char*)*(dictionary+1),1);
	dictlen=(unsigned char*)calloc(sizeof(unsigned char)*(dictionary+1),1);
	dictfreq=(int*)calloc(sizeof(int)*(dictionary+1),1);
	memset(&word_hash[0],0,HASH_TABLE_SIZE*sizeof(word_hash[0]));
	dynmem=dictmem;
	PRINT_DICT(("maxDict=%d allocatedMemory=%d hashTable=%d\n",dictionary,dictionary*WORD_AVG_SIZE+sizeof(unsigned char*)*(dictionary+1)+sizeof(unsigned char)*(dictionary+1)+sizeof(int)*(dictionary+1),HASH_TABLE_SIZE*sizeof(word_hash[0])));
	if (dictmem && dict && dictlen && dictfreq)
	{
		initializeLetterSet();
		WRT_encode(filelen);
		WRT_detectFinish();
	}
	WRT_deinitialize();
	if (collision>0)
		PRINT_DICT(("warning: hash collisions=%d\n",collision));
	detect=false;
	return preprocFlag;
}
int compare_str( const void *arg1, const void *arg2 )
{
	int a=*(int*)arg1;
	int b=*(int*)arg2;
	return strcmp((char*)dict[a],(char*)dict[b]);
}
int compare_str_rev( const void *arg1, const void *arg2 )
{
	int a=*(int*)arg1;
	int b=*(int*)arg2;
	int minv=min(dictlen[a],dictlen[b]);
	for (int i=1; i<=minv; i++)
	{
		if (dict[a][dictlen[a]-i]!=dict[b][dictlen[b]-i])
			return dict[a][dictlen[a]-i] - dict[b][dictlen[b]-i];
	}
	return dictlen[a] - dictlen[b];
}
int compare_freq( const void *arg1, const void *arg2 )
{
	int a=*(int*)arg1;
	int b=*(int*)arg2;
	return dictfreq[b]-dictfreq[a];
}
void XWRT_Encoder::sortDict(int size)
{
	int i,add;
	size--;
	if (size<20)
		return;
	initializeCodeWords(size,false);
	add=0;
	dict1size-=add;
	bound3-=add;
	bound4-=add;
	int* inttable=new int[size];
	if (!inttable)
		OUT_OF_MEMORY();
	for (i=0; i<size; i++)
		inttable[i]=i+1;
	qsort(&inttable[0],size,sizeof(inttable[0]),compare_freq);
	
	
		qsort(&inttable[0],min(size,dict1size),sizeof(inttable[0]),compare_str);
		
		if (size>dict1size)
			qsort(&inttable[dict1size],min(size,bound3)-dict1size,sizeof(inttable[0]),compare_str);
		
		if (size>bound3)
			qsort(&inttable[bound3],min(size,bound4)-bound3,sizeof(inttable[0]),compare_str);
		
		if (size>bound4)
			qsort(&inttable[bound4],size-bound4,sizeof(inttable[0]),compare_str);
	
	for (i=0; i<size; i++)
	{
		std::string str=(char*)dict[inttable[i]];
		sortedDict.push_back(str);
	}
	delete(inttable);
}
void XWRT_Encoder::WRT_detectFinish()
{	
	int i,j;
	PRINT_DICT(("%d words ",sizeDict-1));
	sortedDict.clear();
	int num;
	int minWordFreq2;
	if (minWordFreq<6)
		minWordFreq2=minWordFreq;
	else
		minWordFreq2=minWordFreq-2;
	for (i=1; i<sizeDict-1; i++)
	{
		num=dictfreq[i];
		if (num>=minWordFreq || (num>=minWordFreq2 && (dictlen[i]>=7))) 
			;
		else
			dictfreq[i]=0;
	}
	for (i=1, j=sizeDict-2; i<j; i++)
	{
		if (dictfreq[i]>0)
			continue;
		while (j>0 && dictfreq[j]==0) j--;
		if (i>j)
			break;
		dict[i]=dict[j];
		dictfreq[i]=dictfreq[j];
		dictfreq[j--]=0;
	}
	sizeDict=i;
	if (sizeDict>maxDictSize)
		sizeDict=maxDictSize;
	PRINT_DICT(("reduced to %d words (freq>=%d)\n",sizeDict,minWordFreq));
	sortDict(sizeDict);
}
