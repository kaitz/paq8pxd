#include "XWRT.h"
#include "Decoder.h"


XWRT_Decoder::XWRT_Decoder() : WRTd_s(&WRTd_data[0]) 
{ 	

};

XWRT_Decoder::~XWRT_Decoder() 
{ 
if (cont.bigBuffer)
	{
		free(cont.bigBuffer);
		cont.bigBuffer=NULL;
		cont.freeMemBuffers(false);
	}
	else
		cont.freeMemBuffers(true);
}

#define DECODE_GETC(c)\
{\
	if (cont.memout->memsize>maxMemSize) \
	{ \
		PRINT_DICT(("%d maxMemSize=%d\n",cont.memout->memsize,maxMemSize)); \
		cont.readMemBuffers(preprocFlag,maxMemSize); \
		cont.memout->memsize=0; \
	} \
 \
	c=cont.memout->InpSrcByte(); \
}

// decode word using dictionary
#define DECODE_WORD(dictNo,i)\
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


// convert lower string to upper
inline void XWRT_Decoder::toUpper(unsigned char* s,int &s_size)
{
	for (int i=0; i<s_size; i++)
		s[i]=toupper(s[i]); 
}

inline int XWRT_Decoder::decodeCodeWord(unsigned char* &s,int& c)
{
	int i,s_size;

	if (codeword2sym[c]<dict1size)
	{
		i=codeword2sym[c];
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
		i=dict1size*(codeword2sym[c]-dict1size);
	else
	if (codeword2sym[c]<dict1plus2plus3)
	{
		PRINT_CODEWORDS(("DC1b c=%d\n",codeword2sym[c]-dict1plus2));
		i=dict12size*(codeword2sym[c]-dict1plus2);
	}
	else
		i=dict123size*(codeword2sym[c]-dict1plus2plus3);

	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC1 c=%d i=%d\n",c,i));

	if (codeword2sym[c]<dict1size)
	{
		i+=codeword2sym[c];
		i+=dict1size; //dictNo=2;
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
	{
		PRINT_CODEWORDS(("DC2b c=%d\n",codeword2sym[c]-dict1size));
		i+=dict1size*(codeword2sym[c]-dict1size);
	}
	else
		i+=dict12size*(codeword2sym[c]-dict1plus2);

	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC2 c=%d i=%d\n",c,i));

	if (codeword2sym[c]<dict1size)
	{
		PRINT_CODEWORDS(("DC3b c=%d\n",codeword2sym[c]));
		i+=codeword2sym[c];
		i+=bound3; //dictNo=3;
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
		i+=dict1size*(codeword2sym[c]-dict1size);


	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC3 c=%d i=%d\n",c,i));


	if (codeword2sym[c]<dict1size)
		i+=codeword2sym[c];
	//else 
		//printf("File is corrupted (codeword2sym[c]<dict1size)!\n");

	i+=bound4; //dictNo=4;
	DECODE_WORD(dictNo, i);

	return s_size;
}

int XWRT_Decoder::WRT_decode()
{
int rchar=0;
int c;
static int s_sizep=0;
			if (s_sizep<s_size && s_sizep!=0 )
			{
				rchar=WRTd_s[s_sizep];
				last_c=rchar;
				++s_sizep;
				return rchar;
			}
			if(s_sizep==s_size && s_sizep!=0 ){
			DECODE_GETC(WRTd_c);
			s_sizep=0;
			s_size=0;
			}
	while (1)
	{
		if (fileCorrupted)
			return -1;

		PRINT_CHARS(("c=%d (%c)\n",WRTd_c,WRTd_c));

		if (outputSet[WRTd_c])
		{
			PRINT_CHARS(("addSymbols[%d] upperWord=%d\n",WRTd_c,upperWord));

			switch (WRTd_c)
			{
				case CHAR_ESCAPE:
					WRTd_upper=false;
					upperWord=UFALSE;
					DECODE_GETC(WRTd_c);
					PRINT_CHARS(("c==CHAR_ESCAPE, next=%x\n",WRTd_c));
					rchar=WRTd_c;
					last_c=rchar;
					DECODE_GETC(WRTd_c);
					return rchar;

				case CHAR_FIRSTUPPER:
					PRINT_CHARS(("c==CHAR_FIRSTUPPER\n"));

					WRTd_upper=true;
					upperWord=UFALSE;
					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_UPPERWORD:
					PRINT_CHARS(("c==CHAR_UPPERWORD\n"));

					upperWord=FORCE;
					DECODE_GETC(WRTd_c);
					continue;

			}

			if (upperWord==FORCE)
				upperWord=UTRUE;
			else
				upperWord=UFALSE;

				s_size=decodeCodeWord(WRTd_s,WRTd_c);

			if (WRTd_upper)
			{
				WRTd_upper=false;
				WRTd_s[0]=toupper(WRTd_s[0]);
			}
			
			if (upperWord!=UFALSE)
				toUpper(&WRTd_s[0],s_size);
			
			upperWord=UFALSE;
			
			if (s_size>0){
			s_sizep=1;
				rchar=WRTd_s[0];;
				last_c=rchar;
				return rchar;
			}
		}

		if (WRTd_c>='0' && WRTd_c<='9') 
		{
			unsigned int no,mult;
			int c,i;
			no=0;
			mult=1;
			static int wType=0;
			rchar=WRTd_c;
			DECODE_GETC(WRTd_c);
			last_c=rchar;
			return rchar;
		}

 		PRINT_CHARS(("other c=%d (%d) upperWord=%d\n",fileLenMB,upperWord));

		if (upperWord!=UFALSE)
		{
			if (upperWord==FORCE)
				upperWord=UTRUE;

			if (WRTd_c>='a' && WRTd_c<='z')
				WRTd_c=toupper(WRTd_c);
			else
				upperWord=UFALSE;
		}
		else
		if (WRTd_upper)
		{
			WRTd_upper=false;
			WRTd_c=toupper(WRTd_c);
		}
		rchar=WRTd_c;
		last_c=rchar;
		DECODE_GETC(WRTd_c);
		return rchar;
	}
}

void XWRT_Decoder::read_dict()
{
	int i,c,count;
	unsigned char* bound=(unsigned char*)&word_hash[0] + HASH_TABLE_SIZE*sizeof(word_hash[0]) - 6;

	unsigned char* bufferData=(unsigned char*)&word_hash[0] + 3;

	{
		for (i=0, count=0; i<3; i++)
		{
			GETC(c);
		    count=count*256+c;
		}

		fread_fast(bufferData,count,XWRT_file);
	}

	count=bufferData[0]; bufferData++;
	count+=256*bufferData[0]; bufferData++;
	count+=65536*bufferData[0]; bufferData++;
	
	sortedDict.clear();
	
	PRINT_DICT(("count=%d\n",count));
	
	std::string s;
	std::string last_s;
	for (i=0; i<count; i++)
	{
		if ( bufferData[0]>=128)
		{
			s.append(last_s.c_str(),bufferData[0]-128);
			bufferData++;
		}

		while (bufferData[0]!=10)
		{
			s.append(1,bufferData[0]);
			bufferData++;

			if (s.size()>WORD_MAX_SIZE || bufferData>bound)
			{
				//printf("File corrupted (s.size()>WORD_MAX_SIZE)!\n");
				OUT_OF_MEMORY();
			}
		}
		bufferData++;

		sortedDict.push_back(s);
		last_s=s;
		s.erase();
	}

	sortedDictSize=(int)sortedDict.size();
	PRINT_DICT(("read_dict count2=%d\n",count));

}


 int XWRT_Decoder::WRT_start_decoding(FILE* in)
{
	int i,j,k,c;
XWRT_file=in;
	last_c=0;
	WRTd_upper=false;
	upperWord=UFALSE;
	s_size=0;
	collision=0;

	defaultSettings(); 
	GETC(maxMemSize); 
	maxMemSize*=1024*1024;
	 int fileLen;

	GETC(c);
	fileLen=c;
	GETC(c);
	fileLen=fileLen|(c<<8);
	GETC(c);
	fileLen=fileLen|(c<<16);
	GETC(c);
	fileLen=fileLen|(c<<24);
	fileLenMB=fileLen/(1024*1024);
	if (fileLenMB>255*256)
		fileLenMB=255*256;

	PRINT_DICT(("maxMemSize=%d fileLenMB=%d\n",maxMemSize,fileLenMB));
	read_dict();

	cont.readMemBuffers(preprocFlag,maxMemSize);
	cont.memout->memsize=0;

	WRT_deinitialize();

	decoding=true;
	if (!initialize(false))
		return 0;

	DECODE_GETC(WRTd_c);
	PRINT_CHARS(("WRT_start_decoding WRTd_c=%d ftell=%d\n",WRTd_c,ftell(XWRT_file)));

	return fileLen;
}


