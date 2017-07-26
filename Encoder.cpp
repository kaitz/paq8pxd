#include "XWRT.h"
#include <time.h>
#if defined WIN32 || defined WIN64
	#include <windows.h>
	#include <conio.h>
#endif
 
#include "Common.h"
#include "MemBuffer.h"
#include "Encoder.h"


XWRT_Encoder::XWRT_Encoder() : utf8cached(0), utf8pos(0), last_c_bak(0)
{ 	
	getcBuffer=new unsigned char[mFileBufferSize+1];
	
	if (!getcBuffer )
		OUT_OF_MEMORY();
};
XWRT_Encoder::~XWRT_Encoder() 
{ 
	if (getcBuffer)
		delete(getcBuffer);
	
}
#define ENCODE_PUTC(c)\
{ \
	if (!detect) \
	{ \
		if (cont.memout->memsize>maxMemSize) \
		{ \
			PRINT_DICT(("%d maxMemSize=%d\n",cont.memout->memsize,maxMemSize)); \
			cont.writeMemBuffers(preprocFlag,additionalParam); \
			cont.memout->memsize=0; \
		} \
 \
		PRINT_CHARS(("output=%d (%c)\n",c,c)); \
		cont.memout->OutTgtByte(c); \
	} \
}
#define READ_CHAR(c)\
{\
	if (getcBufferSize) \
	{ \
		c=getcBufferData[0]; \
		getcBufferData++; \
		getcBufferSize--; \
	} \
	else \
		readGetcBuffer(XWRT_file,c); \
}
#define ENCODE_UNICODE_GETC(c) \
{ \
	last_last_c=last_c; \
	last_c=last_c_bak; \
 \
 	READ_CHAR(c); \
 \
	if (c>=0) \
	{ \
		int d; \
		if (IF_OPTION(OPTION_UNICODE_BE)) \
		{ \
		 	READ_CHAR(d); \
			if (d<0) \
				d=65536; \
			c=256*c+d; \
		} \
		else \
		if (IF_OPTION(OPTION_UNICODE_LE)) \
		{ \
		 	READ_CHAR(d); \
			if (d<0) \
				d=256; \
			c+=256*d; \
		} \
	} \
 \
	last_c_bak=c; \
}
#define ENCODE_GETC(c) \
{\
	if (utf8cached>0) \
	{ \
		c=utf8buff[utf8pos++]; \
		utf8cached--; \
	} \
	else \
	{ \
		ENCODE_UNICODE_GETC(c); \
	\
		if (c>=0x80 && (IF_OPTION(OPTION_UNICODE_BE) || IF_OPTION(OPTION_UNICODE_LE))) \
	 	{ \
			utf8cached=unicode2utf8(c,&utf8buff[0]); \
			utf8pos=0; \
		\
			c=utf8buff[utf8pos++]; \
			utf8cached--; \
		} \
	} \
} 
inline void XWRT_Encoder::readGetcBuffer(FILE* &file,int &c)
{
	if (getcBufferDataParts>0)
	{
		if (getcBufferDataParts==1)
		{
			c=EOF; 
			return;
		}
		getcBufferDataParts--;
	}
	
	getcBuffer[0]=getcBufferData[-1];
	getcBufferSize=fread_fast(getcBuffer+1,mFileBufferSize,file); 
	getcBufferData=&getcBuffer[1];
	getcBufferSizeBak=getcBufferSize;
	
	if (getcBufferSize==0) 
		c=EOF; 
	else 
	{ 
		//if (!detect)
			//printStatus((int)getcBufferSize,0,true);
		c=getcBufferData[0]; 
		getcBufferData++;
		getcBufferSize--; 
	} 
}
inline int XWRT_Encoder::unicode2utf8(unsigned int cp, unsigned char* result)
{
	int len=0;
	if (cp < 0x80) {                       // one octet
		*(result++) = static_cast<unsigned char>(cp);  
		len=1;
	}
	else if (cp < 0x800) {                // two octets
		*(result++) = static_cast<unsigned char>((cp >> 6)          | 0xc0);
		*(result++) = static_cast<unsigned char>((cp & 0x3f)        | 0x80);
		len=2;
	}
	else if (cp < 0x10000) {              // three octets
		*(result++) = static_cast<unsigned char>((cp >> 12)         | 0xe0);
		*(result++) = static_cast<unsigned char>((cp >> 6) & 0x3f   | 0x80);
		*(result++) = static_cast<unsigned char>((cp & 0x3f)        | 0x80);
		len=3;
	}
	else {                                // four octets
		*(result++) = static_cast<unsigned char>((cp >> 18)         | 0xf0);
		*(result++) = static_cast<unsigned char>((cp >> 12)& 0x3f   | 0x80);
		*(result++) = static_cast<unsigned char>((cp >> 6) & 0x3f   | 0x80);
		*(result++) = static_cast<unsigned char>((cp & 0x3f)        | 0x80);
		len=4;
	}
	return len;
}
// encode word (should be lower case) using n-gram array (when word doesn't exist in the dictionary)
inline void XWRT_Encoder::encodeAsText(unsigned char* &s,int &s_size,EWordType wordType)
{
	int i=0;
	//if (!IF_OPTION(OPTION_LETTER_CONTAINER))
	{
#ifdef DYNAMIC_DICTIONARY
		if (s_size>=WORD_MIN_SIZE)
		{
			memcpy(mem,s,s_size);
			
			if (mem<dictmem_end && addWord(mem,s_size)!=0)
			{
				mem+=(s_size/4+1)*4;
				
//				s[s_size]=0;
//				printf("NEWWORD=%s %d/%d t=%d\n",s,sizeDict,dictionary,wordType);
				
				ENCODE_PUTC(CHAR_NEWWORD);
				for (i=0; i<s_size; i++)
					ENCODE_PUTC(s[i]);
				ENCODE_PUTC(0);
				return; 
			}
		}
#endif
		for (i=0; i<s_size; i++)
		{
			if (addSymbols[s[i]])
				ENCODE_PUTC(CHAR_ESCAPE);
			ENCODE_PUTC(s[i]);
		}
		return;
	}
	
}
inline void XWRT_Encoder::encodeCodeWord_PPM(int &i)
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
inline void XWRT_Encoder::encodeCodeWord(int &i)
{
		encodeCodeWord_PPM(i);
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
	
	/*if (XMLState==OPEN)
	{
		if (s_size>1)
		{
			PRINT_STACK(("push s=%s c=%c (%d) s_size=%d\n",s,c,c,s_size));
			justAdded=true;
			XMLState=ADDED;
			s[s_size]=0;
			stack.push_back((char*)s);
		}
		else
			XMLState=INSIDE;
		if (c=='>')
		{
			s[s_size++]=c;
			XMLState=ADDED2; // no encoding '>'
		}
	}
	else
		if (XMLState==CLOSE || XMLState==CLOSE_EOL)
		{
			encodeSpaces();
			static std::string str;
			if (stack.size()>0)
			{
				str=stack.back();
				stack.pop_back();
			}
			else
				str.erase();
		
			PRINT_STACK(("pop str=%s s=%s \n",str.c_str(),s));
			if (s_size==str.size() && memcmp(s,str.c_str(),s_size)==0)
			{
				if (c=='>')
				{
					if (XMLState==CLOSE)
						ENCODE_PUTC(CHAR_END_TAG)
					else
						ENCODE_PUTC(CHAR_END_TAG_EOL);
					XMLState=CLOSED;
					cont.MemBufferPopBack();
					return; 
				}
			}
			memmove(s+1,s,s_size); 
			s[0]='<';
			s[1]='/';
			s_size++; // <tag -> </tag
			if (c=='>')
			{
				s[s_size]='>';
				s_size++; // </tag -> </tag>
			}
			if (XMLState==CLOSE_EOL)
			{
				if (IF_OPTION(OPTION_CRLF))
				{
					s[s_size]=13;
					s_size++;
				}
				s[s_size]=10;
				s_size++;
			}
			XMLState=CLOSED;
		}*/
	if (wordType==NUMBER)
	{
		if (subWordType==NUMBER2 || subWordType==NUMBER3)
			wordType=subWordType;
		encodeSpaces();
		int len;
		if (s_size==4 && ((s[0]=='1' && s[1]>=7 && s[1]<='9') || (s[0]=='2' && s[1]=='0')))
		{
			len=1000*(s[0]-'0')+100*(s[1]-'0')+10*(s[2]-'0')+(s[3]-'0');
			len-=1900;
			if (len>=0 && len<256)
			{
				ENCODE_PUTC('5');
				
					ENCODE_PUTC(len);
				return;
			}
		} 
		if (s_size>=10)
		{
			int tmp_num=s_size-9;
			encodeWord(s,tmp_num,wordType,c);
			s+=s_size-9;
			s_size=9;
		}
		while (s_size>0 && s[0]=='0')
		{
			ENCODE_PUTC(s[0]);
			s++;
			s_size--;
		}
		if (s_size==0)
			return;
		len=s[s_size];
		s[s_size]=0;
		size=atol((char*)s);
		s[s_size]=len;
		len=0;
		while (size>0)
		{
			num[len++]=size%NUM_BASE; //100;
			size=size/NUM_BASE;
		}
		ENCODE_PUTC('0'+len);
		
		{
			for (i=0; i<len; i++)
				ENCODE_PUTC(num[i]);
		}
		
		return;
	}
	if (s_size>=WORD_MIN_SIZE)
	{
		/*if (XMLState==ADDED || XMLState==ADDED2) 	
		{
			if (spaces+s_size<STRING_MAX_SIZE)
			{
				memmove(s+spaces,s,s_size);
				memset(s,' ',spaces);
				s_size+=spaces;
			}
			else
				encodeSpaces();
		}*/
		
		checkHashExactly(s,s_size,i);
		PRINT_CODEWORDS(("i=%d/%d %s(%d)\n",i,sizeDynDict,s,s_size));
			
		if (i>=0)// && codeWordSize(i)<=s_size)
			wordType=LOWERWORD;
		/*if (XMLState==ADDED || XMLState==ADDED2) 	
		{
			if (i>=0 && i<sizeDynDict)
				spaces=0;
			else
			{
				s+=spaces;
				s_size-=spaces;
				checkHashExactly(s,s_size,i);
				if (i>=0)
					wordType=LOWERWORD;
			}
		}*/
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
			
			
			if (i<0 && IF_OPTION(OPTION_TRY_SHORTER_WORD))
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
		if (IF_OPTION(OPTION_SPACELESS_WORDS))
		{
			if ((s[0]>='a' && s[0]<='z') || (s[0]>='A' && s[0]<='Z'))
			{ 
				if ((beforeWord=='/' || beforeWord=='-' || beforeWord=='\"' || beforeWord=='_' || beforeWord=='>'))
				{
					if (spaces>0)
						ENCODE_PUTC(CHAR_NOSPACE);
				}
				else
				{
					if (spaces>0)
						spaces--;
					else
						ENCODE_PUTC(CHAR_NOSPACE);
				}
			}
		}
		encodeSpaces();
		if (wordType==FIRSTUPPER || wordType==UPPERWORD)
		{
			ENCODE_PUTC(flagToEncode);
			if (IF_OPTION(OPTION_SPACE_AFTER_CC_FLAG))
				ENCODE_PUTC(' ');
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
void XWRT_Encoder::WRT_encode(size_t bufferSize)
{
	unsigned char s[STRING_MAX_SIZE];
	EWordType wordType;
	int c,unicode_le,unicode_be,fftell;
	
	unicode_be=unicode_le=fftell=0;
	spaces=0;
	s_size=0;
	last_c=0;
	binCount=0;
	wordType=LOWERWORD;
	//XMLState=UNKNOWN;
	getcBuffer[0]=0;
	getcBufferData=&getcBuffer[1];
	getcBufferSize=bufferSize;
	ENCODE_GETC(c);
	while (true)
	{
		if (c==EOF)
			break;
		PRINT_CHARS(("c=%c (%d) last=%c \n",c,c,last_c));
		/*if (last_c=='>')
			XMLState=UNKNOWN;*/
		if (detect)
		{
			fftell++;
			if (c<32 && c!=9 && c!=10 && c!=13 && c!=0)
				binCount++;
			else
			if (c==0)
			{
				if (fftell%2 != 0)
					unicode_be++;
				else
					unicode_le++;
			}
	
			if (fftell==BYTES_TO_DETECT)
			{
				TURN_OFF(OPTION_UNICODE_LE);
				TURN_OFF(OPTION_UNICODE_BE);
				if (unicode_le*4/3>fftell/2)
					TURN_ON(OPTION_UNICODE_LE)
				else
				if (unicode_be*4/3>fftell/2)
					TURN_ON(OPTION_UNICODE_BE)
			}
			value[c]++;
			if (last_c=='=' && c=='\"')
				quotes++;
			letterType=letterSet[c];
		}
		else
		{
			if (c==13)
			{
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				ENCODE_GETC(c);
				if (c==10 && IF_OPTION(OPTION_CRLF))
				{
					ENCODE_PUTC(CHAR_CRLF);
					ENCODE_GETC(c);
				}
				else
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
			
			
			if (IF_OPTION(OPTION_QUOTES_MODELING))
			{
				if (c=='=')
				{
					encodeWord(s,s_size,wordType,c);
					s_size=0;
	
					ENCODE_GETC(c);
					
					if (c=='\"')
					{
						encodeCodeWord(quoteOpen);
						
						ENCODE_GETC(c);
					}
					else
						ENCODE_PUTC('=');
					
					continue;
				}
				
				if (c=='\"')
				{
					encodeWord(s,s_size,wordType,c);
					s_size=0;
					
					ENCODE_GETC(c);
					
					/*if (c=='>' && XMLState==ADDED)
					{
						encodeCodeWord(quoteClose);
				
						ENCODE_GETC(c);
					}
					else*/
						ENCODE_PUTC('\"');
					
					continue;
				}
			}
			if (letterType==NUMBERCHAR) // && (XMLState!=OPEN && XMLState!=CLOSE))
			{				
				if (wordType!=NUMBER)
				{
					encodeWord(s,s_size,wordType,c);
					s_size=0;
				}
				
				wordType=NUMBER;
				
				s[s_size++]=c;
				if (s_size>=STRING_MAX_SIZE-2)
				{
					encodeWord(s,s_size,wordType,c);
					s_size=0;
				}
				ENCODE_GETC(c);
				continue;
			}
			
			
			if (wordType==NUMBER)
			{
				if (c==':' && s_size<=2)
				{
					int lsize=s_size;
					do
					{
						s[s_size++]=c; 
						ENCODE_GETC(c);
						if (c==EOF)
							break;
	
						letterType=letterSet[c];
					}
					while (letterType==NUMBERCHAR && s_size<STRING_MAX_SIZE-2);
					int hour,minute;
					if (lsize==2)
						hour=(s[0]-'0')*10+(s[1]-'0')-1;
					else
						hour=(s[0]-'0')-1;
					minute=(s[s_size-2]-'0')*10+(s[s_size-1]-'0');
					if (minute<0 || minute>59 || hour<0 || hour>11 || s_size-lsize!=3 || (c!='a' && c!='p'))
					{
						if (lsize==2)
							hour=(s[0]-'0')*10+(s[1]-'0');
						else
							hour=-1;
						minute=(s[s_size-2]-'0')*10+(s[s_size-1]-'0');
						if (s_size-lsize==3 && minute>=0 && minute<=99 && hour>=0 && hour<=99)
						{
							int sec;
							if (c==':')
							{
								s[s_size++]=c; 
								ENCODE_GETC(c);
								if (c!=EOF && letterSet[c]==NUMBERCHAR)
								{
									s[s_size++]=c;
									sec=(c-'0')*10;
									ENCODE_GETC(c);
									if (c!=EOF && letterSet[c]==NUMBERCHAR)
									{
										s[s_size++]=c;
										sec+=(c-'0');
										ENCODE_PUTC(CHAR_HOURMINSEC);
										
										
										{
											ENCODE_PUTC(hour);
											ENCODE_PUTC(minute);
											ENCODE_PUTC(sec);
										}
										ENCODE_GETC(c);
										wordType=LOWERWORD;
										s_size=0;
										continue;
									}
								}
								encodeMixed(s,s_size,c);
								wordType=LOWERWORD;
								s_size=0;
								continue;
							}
							ENCODE_PUTC(CHAR_HOURMIN);
	
							
							{
								ENCODE_PUTC(hour);
								ENCODE_PUTC(minute);
							}
							wordType=LOWERWORD;
							s_size=0;
							continue;
						}
						minute=-1;
					}
					else
					{
						if (c=='p')
							hour+=12; // 0-23
						if (last_c!=' ')
							beforeWord=last_c;
						else
							beforeWord=last_last_c;
						ENCODE_GETC(c);
					}
					if (minute<0 || c!='m')
					{
						encodeWord(s,lsize,wordType,c);
						ENCODE_PUTC(':');
						if (minute>=0)
						{
							s_size-=lsize+1;
							encodeWord(s+lsize+1,s_size,wordType,c);
							if (hour>=12)
								s[0]='p';
							else
								s[0]='a';
							s_size=1;
							wordType=LOWERWORD;
						}
						else
						{
							s_size-=lsize+1;
							memmove(s,s+lsize+1,s_size);
						}
					}
					else
					{
						if (lsize==2 && s[0]=='0')
							ENCODE_PUTC('0');
						ENCODE_PUTC(CHAR_TIME);
						
						{
							ENCODE_PUTC(hour);
							ENCODE_PUTC(minute);
						}
						ENCODE_GETC(c);
						wordType=LOWERWORD;
						s_size=0;
					}
					continue;
				}
				else
				if (c=='.')
				{
					int lsize=s_size;
					do
					{
						s[s_size++]=c; 
						ENCODE_GETC(c);
						if (c==EOF)
							break;
	
						letterType=letterSet[c];
					}
					while (letterType==NUMBERCHAR && s_size<STRING_MAX_SIZE-2);
					if (c=='.' && lsize<4 && s_size-lsize>=2 && s_size-lsize<=4)
					{
						int x1,x2,x3,x4,c2;
						if (lsize==2)
							x1=10*(s[0]-'0')+(s[1]-'0');
						else
						if (lsize==3)
							x1=100*(s[0]-'0')+10*(s[1]-'0')+(s[2]-'0');
						else
							x1=(s[0]-'0');
						if (s_size-lsize==3)
						{
							c2=s[s_size-2];
							x2=10*(s[s_size-2]-'0')+(s[s_size-1]-'0');
						}
						else
						if (s_size-lsize==4)
						{
							c2=s[s_size-3];
							x2=100*(s[s_size-3]-'0')+10*(s[s_size-2]-'0')+(s[s_size-1]-'0');
						}
						else
						{
							c2=s[s_size-1];
							x2=(s[s_size-1]-'0');
						}
						s[s_size++]=c; 
						ENCODE_GETC(c);
						if (c==EOF || letterSet[c]!=NUMBERCHAR || c=='0' || s[0]=='0' || c2=='0' || x1>255 || x2>255)
						{
							encodeMixed(s,s_size,c);
							s_size=0;
							wordType=LOWERWORD;
							continue;
						}	
						x3=0;
						x3=10*x3+(c-'0');
						s[s_size++]=c; 
						ENCODE_GETC(c);
						if (c!=EOF && letterSet[c]==NUMBERCHAR)
						{
							x3=10*x3+(c-'0');
							s[s_size++]=c; 
							ENCODE_GETC(c);
							if (c!=EOF && letterSet[c]==NUMBERCHAR)
							{
								x3=10*x3+(c-'0');
								s[s_size++]=c; 
								ENCODE_GETC(c);
							}
						}
						if (c!='.' || x3>255)
						{	
							encodeMixed(s,s_size,c);
							s_size=0;
							wordType=LOWERWORD;
							continue;
						}	
						s[s_size++]=c; 
						ENCODE_GETC(c);
						if (c==EOF || letterSet[c]!=NUMBERCHAR || c=='0')
						{
							encodeMixed(s,s_size,c);
							s_size=0;
							wordType=LOWERWORD;
							continue;
						}	
						x4=0;
						x4=10*x4+(c-'0');
						s[s_size++]=c; 
						ENCODE_GETC(c);
						if (c!=EOF && letterSet[c]==NUMBERCHAR)
						{
							x4=10*x4+(c-'0');
							s[s_size++]=c; 
							ENCODE_GETC(c);
							if (c!=EOF && letterSet[c]==NUMBERCHAR)
							{
								x4=10*x4+(c-'0');
								s[s_size++]=c; 
								ENCODE_GETC(c);
							}
						}
						if (x4>255)
						{	
							encodeMixed(s,s_size,c);
							s_size=0;
							wordType=LOWERWORD;
							continue;
						}	
						ENCODE_PUTC(CHAR_IP);
						
						{
							ENCODE_PUTC(x1);
							ENCODE_PUTC(x2);
							ENCODE_PUTC(x3);
							ENCODE_PUTC(x4);
						}
						s_size=0;
						wordType=LOWERWORD;
						continue;
					}
					if (s_size-lsize==3)
					{
						encodeWord(s,lsize,wordType,c);
						s_size-=lsize;
						memmove(s,s+lsize,s_size);
						ENCODE_PUTC('7');
						int remain=(s[1]-'0')*10+(s[2]-'0');
						
							ENCODE_PUTC(remain);
					}
					else
					if (lsize==1 && s_size==3)
					{
						ENCODE_PUTC(CHAR_REMAIN);
						int remain=(s[0]-'0')*10+(s[2]-'0');
						
							ENCODE_PUTC(remain);
					} 
					else
					if (lsize==2 && s_size==4 && (s[0]=='1' || (s[0]=='2' && s[1]<='4')))
					{
						ENCODE_PUTC(CHAR_REMAIN);
						int remain=(s[0]-'0')*100+(s[1]-'0')*10+(s[3]-'0');
						
							ENCODE_PUTC(remain);
					} 
					else
					{
						encodeWord(s,lsize,wordType,c);
						s_size-=lsize;
						memmove(s,s+lsize,s_size);
						ENCODE_PUTC('.');
						s_size--;
						encodeWord(s+1,s_size,wordType,c);
					}
	
					s_size=0;
					wordType=LOWERWORD;
					continue;
				} 
				else
				if (c!='-' || s_size==0)
				{
					encodeWord(s,s_size,wordType,c);
					s_size=0;
					wordType=LOWERWORD;
				}
				else
				if (s_size==4)
				{
					int year=(s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+(s[3]-'0')-1929;
					ENCODE_GETC(c);
					letterType=letterSet[c];
					if (letterType==NUMBERCHAR && year>=0)
					{
						s[s_size++]='-'; // s_size==5;
						do
						{
							s[s_size++]=c; // s_size==6;  // 1977-12-31
							ENCODE_GETC(c);
							if (c==EOF)
								break;
							letterType=letterSet[c];
						}
						while (s_size<STRING_MAX_SIZE-2 && (letterType==NUMBERCHAR || (c=='-' && s_size==7)));
						if (s_size==10 && s[7]=='-')
						{
							int month=(s[5]-'0')*10+(s[6]-'0')-1;
							int day=(s[8]-'0')*10+(s[9]-'0')-1;
							if (month>=0 && month<=11 && day>=0 && day<=30)
							{
								int all=day+31*month+31*12*year;
								if (all<65536/2)
								{
									int newAll=(256/2)+all-lastAll;
									if (newAll>=0 && newAll<256)
									{
										ENCODE_PUTC('9');
										
											ENCODE_PUTC(newAll);
									}
									else
									{
										newAll=(65536/2)+all-lastAll;
										ENCODE_PUTC('8');
										
										{
											ENCODE_PUTC(newAll%256);
											ENCODE_PUTC(newAll/256);
										}
									}
									lastAll=all;
									s_size=0;
									wordType=LOWERWORD;
									continue;
								}
							}
						}
						if (s_size>7 && s[7]=='-')
						{
							year=4;
							encodeWord(s,year,wordType,c);
							ENCODE_PUTC('-');
							year=2;
							encodeWord(s+5,year,wordType,c);
							ENCODE_PUTC('-');
							if (s_size>8)
							{
								s_size-=8;
								encodeWord(s+8,s_size,wordType,c);
							}
						}
						else
						{
							if (s_size==9)
							{
								int page2;
								year+=1929;
								page2=(s[5]-'0')*1000+(s[6]-'0')*100+(s[7]-'0')*10+(s[8]-'0');
								page2-=year;
								if (page2>=0 && page2<256)
								{
									ENCODE_PUTC(CHAR_PAGES);
									
									{
										ENCODE_PUTC(year/256);
										ENCODE_PUTC(year%256);
										ENCODE_PUTC(page2);
									}
									s_size=0;
									wordType=LOWERWORD;
									continue;
								}
							}
		 
							if (s_size<=14 && s_size>=6 && s[0]!='0' && s[5]!='0')
							{
								ENCODE_PUTC('6');
								subWordType=NUMBER2;
								year=4;
								encodeWord(s,year,wordType,c);
								subWordType=NUMBER3;
								s_size-=5;
								encodeWord(s+5,s_size,wordType,c);
								subWordType=LOWERWORD;
							}
							else 
							{
								year=4;
								encodeWord(s,year,wordType,c);
								ENCODE_PUTC('-');
	
								if (s_size>5)
								{
									s_size-=5;
									encodeWord(s+5,s_size,wordType,c);
								}
							}
						}
					}
					else
					{
						encodeWord(s,s_size,wordType,c);
						ENCODE_PUTC('-');
					}
					s_size=0;
					wordType=LOWERWORD;
					continue;
				}
				else
				if (s_size==3)
				{
					do
					{
						s[s_size++]=c; 
						ENCODE_GETC(c);
						if (c==EOF)
							break;
	
						letterType=letterSet[c];
					}
					while (s_size<STRING_MAX_SIZE-2 && letterType==NUMBERCHAR);
					int page,page2;
					if (s_size==7) // || (s_size==8 && s[4]=='1'))
					{
						page=(s[0]-'0')*100+(s[1]-'0')*10+(s[2]-'0');
						page2=(s[4]-'0')*100+(s[5]-'0')*10+(s[6]-'0');
						page2-=page;
						if (page2>=0 && page2<256)
						{
							ENCODE_PUTC(CHAR_PAGES);
					
							{
								ENCODE_PUTC(page/256);
								ENCODE_PUTC(page%256);
								ENCODE_PUTC(page2);
							}
							s_size=0;
							wordType=LOWERWORD;
							continue;
						}
					}
					if (s_size<=13 && s_size>=5 && s[0]!='0' && s[4]!='0')
					{
						ENCODE_PUTC('6');
						subWordType=NUMBER2;
						page=3;
						encodeWord(s,page,wordType,c);
						subWordType=NUMBER3;
						s_size-=4;
						encodeWord(s+4,s_size,wordType,c);
						subWordType=LOWERWORD;
					}
					else 
					{
						page=3;
						encodeWord(s,page,wordType,c);
						ENCODE_PUTC('-');
						if (s_size>4)
						{
							s_size-=4;
							encodeWord(s+4,s_size,wordType,c);
						}
					}
					s_size=0;
					wordType=LOWERWORD;
					continue;
				} 
				else
				{
					ENCODE_GETC(c);
					if (s_size==2 && c>='A' && c<='Z')
					{
						int day=(s[0]-'0')*10+(s[1]-'0')-1;
						letterType=letterSet[c];
						if (letterType==UPPERCHAR && day>=0 && day<=30)
						{
							s[s_size++]='-'; // s_size==3;
							do
							{
								s[s_size++]=c;
								ENCODE_GETC(c);
								letterType=letterSet[c];
							}
							while (s_size<STRING_MAX_SIZE-2 && letterType==UPPERCHAR);// || letterType==LOWERCHAR);
							
							int month=-1;
							if (s_size==6 && c=='-')
							{
								std::string mon;
								char* str=(char*)s+3;
								mon.append((char*)str,3);
								//	month=3;
								//	toLower((unsigned char*)mon.c_str(),month);
								month=-1;
								if (mon=="JAN")	month=0;
								else if (mon=="FEB") month=1;
								else if (mon=="MAR") month=2;
								else if (mon=="APR") month=3;
								else if (mon=="MAY") month=4;
								else if (mon=="JUN") month=5;
								else if (mon=="JUL") month=6;
								else if (mon=="AUG") month=7;
								else if (mon=="SEP") month=8;
								else if (mon=="OCT") month=9;
								else if (mon=="NOV") month=10;
								else if (mon=="DEC") month=11;
							}
							
							if (month==-1)
							{		
								month=2;
								encodeWord(s,month,wordType,c);
								ENCODE_PUTC('-');
								beforeWord='-';
								s_size-=3;
								memmove(s,s+3,s_size);
								if (s_size==1)
									wordType=FIRSTUPPER;
								else if (s_size>1)
									wordType=UPPERWORD;
								else
									wordType=LOWERWORD;
								continue;
							}
							
							do
							{
								s[s_size++]=c; 		// s_size==7; 12-aug-
								ENCODE_GETC(c);
								letterType=letterSet[c];
							}
							while (s_size<STRING_MAX_SIZE-2 && letterType==NUMBERCHAR);
							
							int year=-1;
							int newAll=0;
							
							if (s_size==11)
							{
								year=(s[7]-'0')*1000+(s[8]-'0')*100+(s[9]-'0')*10+(s[10]-'0')-1929;
								newAll=day+31*month+31*12*year;
							}
							
							if (year<0 || newAll<0 || newAll>65535)
							{							
								month=2;
								encodeWord(s,month,wordType,c);
								ENCODE_PUTC('-');
								beforeWord='-';
								month=3;
								wordType=UPPERWORD;
								encodeWord(s+3,month,wordType,c);
								ENCODE_PUTC('-');
								
								s_size-=7;
								memmove(s,s+7,s_size);
								
								wordType=NUMBER;
								continue;
							}
							
									
							ENCODE_PUTC(CHAR_DATE_ENG);
							
							{
								ENCODE_PUTC(newAll%256);
								ENCODE_PUTC(newAll/256);
							}
						}
						else
						{
							encodeWord(s,s_size,wordType,c);
							
							ENCODE_PUTC('-');
						}
						
						s_size=0;
						wordType=LOWERWORD;
						continue;
					}
					else
					{
						int lsize=s_size;
						s[s_size++]='-'; 
						
						letterType=letterSet[c];
						while (s_size<STRING_MAX_SIZE-2 && letterType==NUMBERCHAR)
						{
							s[s_size++]=c; 
							
							ENCODE_GETC(c);
							
							if (c==EOF)
								break;
							
							letterType=letterSet[c];
						}
						if (lsize<=9 && s_size<=lsize+10 && s_size>=lsize+2 && s[0]!='0' && s[lsize+1]!='0')
						{
							ENCODE_PUTC('6');
							
							subWordType=NUMBER2;
							encodeWord(s,lsize,wordType,c);
							
							subWordType=NUMBER3;
							s_size-=lsize+1;
							encodeWord(s+lsize+1,s_size,wordType,c);
							subWordType=LOWERWORD;
						}
						else
						{
							encodeWord(s,lsize,wordType,c);
							
							ENCODE_PUTC('-');
							
							if (s_size>lsize+1)
							{
								s_size-=lsize+1;
								encodeWord(s+lsize+1,s_size,wordType,c);
							}
						}
						
						s_size=0;
						wordType=LOWERWORD;
						continue;
					} 
				}
				
			} // if (c=='.')
			
		} // if (wordType==NUMBER)
		
		/*if (c=='<')
		{
			if (s_size>0)
			{
				encodeWord(s,s_size,wordType,XMLState,c);
				s_size=0;
			}
			s[s_size++]=c;
			ENCODE_GETC(c);
			if (c=='/')
			{
				XMLState=CLOSE;
				ENCODE_GETC(c);
			}
			else
				XMLState=OPEN;
			while (true) 
			{
				if (c==EOF)
					break;
				if (!startTagSet[c] || s_size>=STRING_MAX_SIZE-2) // || c==' ' || c=='>' || c=='/' || c==':')
					break;
				s[s_size++]=c;
				ENCODE_GETC(c);
			}
			wordType=VARWORD;
			continue;
		}
		if (c=='>')
		{
			int cd=0;
			if (XMLState==CLOSED)
				XMLState=UNKNOWN;
			if (XMLState==CLOSE)
			{
				ENCODE_GETC(c);
				if (IF_OPTION(OPTION_CRLF))
				{
					if (c==13)
					{
						ENCODE_GETC(c);
						if (c==10)
						{
							ENCODE_GETC(c);
							XMLState=CLOSE_EOL;
						}
						else
							cd=13;
					}
				}
				else
				{
					if (c==10)
					{
						ENCODE_GETC(c);
						XMLState=CLOSE_EOL;
					}
				}
				letterType=letterSet[c];
			} 
			int cc='>';
			encodeWord(s,s_size,wordType,XMLState,cc);
			s_size=0;
			if (cd)
			{
				if (addSymbols[13])
					ENCODE_PUTC(CHAR_ESCAPE);
				ENCODE_PUTC(13);
			}
			if (XMLState==CLOSED)
			{
				XMLState=UNKNOWN;	
				continue;
			}
			if (last_c=='/' && XMLState==ADDED) // <xml="xxx"/>
			{
				XMLState=UNKNOWN;
				if (stack.size()>0)
					stack.pop_back();
			}  
			if (c!='>')
				continue;
		}*/
		if (c=='&')
		{
			encodeWord(s,s_size,wordType,c);
			s_size=0;
			wordType=LOWERWORD;
			if (last_c!=' ')
				beforeWord=last_c;
			else
				beforeWord=last_last_c;
			s[s_size++]=c;
			while (true) 
			{
				ENCODE_GETC(c);
				if (c==EOF)
					break;
				letterType=letterSet[c];
				if (letterType==UPPERCHAR) // needed only for tryShorter
					wordType=VARWORD;
				if ((letterType!=LOWERCHAR && letterType!=UPPERCHAR) || s_size>=STRING_MAX_SIZE-2)
					break;
				s[s_size++]=c;
			}
			if (c==';' || c=='#')
			{
				s[s_size++]=c;
				ENCODE_GETC(c);
			}
			encodeWord(s,s_size,wordType,c);
			s_size=0;
			continue;
		}
		if (c==':' && s_size==4 && wordType==LOWERWORD)
		{
			if (s[0]=='h' && s[1]=='t' && s[2]=='t' && s[3]=='p')
			{
				s[s_size++]=c;
				ENCODE_GETC(c);
				if (c=='/')
				{
					s[s_size++]=c;
					ENCODE_GETC(c);
					if (c=='/')
					{
						while (true) 
						{
							s[s_size++]=c;
							ENCODE_GETC(c);
							if (c==EOF)
								break;
							if (urlSet[c] || s_size>=STRING_MAX_SIZE-2) 
								break;
						}
						if (c=='/')
						{
							s[s_size++]=c;
							ENCODE_GETC(c);
						} 
						encodeWord(s,s_size,wordType,c);
						s_size=0;
						wordType=LOWERWORD;
					}
				}
				continue;
			} 
		} 
		if (s[0]!='<')
		{
			if (c=='@' && s_size>0 && wordType==LOWERWORD)
			{
				s[s_size++]=c;
				
				while (true) 
				{
					ENCODE_GETC(c);
					
					if (c==EOF)
						break;
					
					letterType=letterSet[c];
					
					if (letterType==UPPERCHAR) // needed only for tryShorter
						wordType=VARWORD;
					
					if ((letterType!=LOWERCHAR && letterType!=UPPERCHAR && c!='.') || s_size>=STRING_MAX_SIZE-2)
						break;
					
					s[s_size++]=c;
				}
						
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				continue;
			}
#ifdef DYNAMIC_DICTIONARY	
			if (c=='\'' && s_size>=1) // it's
			{
				s[s_size++]=c;
				if (s_size>=STRING_MAX_SIZE-2)
				{
					encodeWord(s,s_size,wordType,c);
					s_size=0;
				}
				ENCODE_GETC(c);
				
				if (s_size==2 && wordType==FIRSTUPPER && letterSet[c]==UPPERCHAR)
					wordType=UPPERWORD;
				
				continue;
			}
#endif
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
		/*if (XMLState==ADDED2)
			XMLState=INSIDE;
		else*/
			ENCODE_PUTC(c);
 
		ENCODE_GETC(c);
	}
	encodeWord(s,s_size,wordType,c);
	s_size=0;
	if (detect && !IF_OPTION(OPTION_UNICODE_LE) && !IF_OPTION(OPTION_UNICODE_BE))
	{
		if (unicode_le*4/3>fftell/2)
			TURN_ON(OPTION_UNICODE_LE)
		else
		if (unicode_be*4/3>fftell/2)
			TURN_ON(OPTION_UNICODE_BE)
		PRINT_DICT(("unicode_le=%d unicode_be=%d uni=%d\n",unicode_le,unicode_be,IF_OPTION(OPTION_UNICODE_LE) || IF_OPTION(OPTION_UNICODE_BE)));
	}
	//printf(" + dynamic dictionary %d/%d words\n",sizeDict,dictionary);
}
inline int common(const char* offset1,const char* offset2, int bound)
{
	int lp=0;
	while (offset1[lp]==offset2[lp] && lp<bound)
		lp++;
	return lp;
}
void XWRT_Encoder::write_dict(int comprLevel)
{
	int i,count=0;
	unsigned char *bound=(unsigned char*)&word_hash[0]+HASH_TABLE_SIZE*sizeof(word_hash[0])-WORD_MAX_SIZE;
	unsigned char *writeBuffer=(unsigned char*)&word_hash[0]; //putcBuffer;
	unsigned char *bufferData=writeBuffer+3;
	if (IF_OPTION(OPTION_SPACES_MODELING))
	{
		for (i=0; i<256; i++)
			if (spacesCont[i]>=minSpacesFreq())
				count++;
	PRINT_DICT(("sp_count=%d\n",count));
		bufferData[0]=count;
		bufferData++;
		for (i=0; i<256; i++)
			if (spacesCont[i]>=minSpacesFreq())
			{		
				bufferData[0]=i;
				bufferData++;
			}
	}
	unsigned char *count_header=bufferData;
	bufferData+=3;
	PRINT_DICT(("sortedDict.size()=%d\n",sortedDict.size()));
	int cmn;
	count=(int)sortedDict.size();
	for (i=0; i<count; i++)
	{
		cmn=0;
		if (i>0)
			cmn=common(sortedDict[i-1].c_str(),sortedDict[i].c_str(),min(sortedDict[i].size(),sortedDict[i-1].size()));
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
		//printStatus(0,count,true);
	}
}
void XWRT_Encoder::WRT_get_options(int& c,int& c2)
{
	c=c2=0;
	/*if (IF_OPTION(OPTION_USE_CONTAINERS))
		c=c+128;*/
	if (IF_OPTION(OPTION_PAQ))
		c=c+64;
	
	
	
	if (IF_OPTION(OPTION_BINARY_DATA))
		c=c+4;
	c+=preprocType; // 0-3
	//if (IF_OPTION(OPTION_LETTER_CONTAINER))
	//	c2=c2+128;
	//if (IF_OPTION(OPTION_NUMBER_CONTAINER))
	//	c2=c2+64;
	if (IF_OPTION(OPTION_SPACES_MODELING))
		c2=c2+32;
	if (IF_OPTION(OPTION_CRLF))
		c2=c2+16;
	if (IF_OPTION(OPTION_QUOTES_MODELING))
		c2=c2+8;
	//if (IF_OPTION(OPTION_USE_DICTIONARY))
	//	c2=c2+4;
	if (IF_OPTION(OPTION_UNICODE_LE))
		c2=c2+2;
	if (IF_OPTION(OPTION_UNICODE_BE))
		c2=c2+1;
}
void XWRT_Encoder::WRT_start_encoding(FILE* in, FILE* out,unsigned int fileLen,bool type_detected)
{
	int c,c2,dictPathLen;
	//unsigned char s[STRING_MAX_SIZE];
	//unsigned char dictPath[STRING_MAX_SIZE];
	//s[0]=0;
	lastAll=0;
	getcBufferDataParts=0;
	collision=0;
XWRT_file=in;
XWRT_fileout=out;
//	PUTC(XWRT_HEADER[0]);
//	PUTC(XWRT_HEADER[1]);
//	PUTC(XWRT_HEADER[2]);
//	PUTC(XWRT_HEADER[3]);
//	PUTC(XWRT_VERSION-150);
	fileLenMB=fileLen/(1024*1024);
	if (fileLenMB>255*256)
		fileLenMB=255*256;
	//g_fileLenMB=fileLenMB;
	
	cont.prepareMemBuffers();
	cont.memout->memsize=0;
	
	
	if (fileLenMB<32)
		minWordFreq+=3;
	if ((preprocType==PAQ) && fileLenMB<6)
		minWordFreq=10;
	int pos=ftell(XWRT_file);
	if (!type_detected)
		WRT_detectFileType();
#ifdef DYNAMIC_DICTIONARY
	getcBufferSize=getcBufferSizeBak;
#else
	getcBufferSize=0;
	fseek(XWRT_file, pos, SEEK_SET );
#endif
	/*dictPathLen=getSourcePath((char*)dictPath,sizeof(dictPath));
	if (dictPathLen>0)
	{
		dictPath[dictPathLen]=0;
		strcat((char*)dictPath,(char*)s);
		strcat((char*)dictPath,(char*)"wrt-eng.dic");
		strcpy((char*)s,(char*)dictPath);
	}*/
	WRT_get_options(c,c2); // po make_dict()
	WRT_print_options();
	PUTC(c);
	PUTC(c2);
	PUTC(maxMemSize/(1024*1024));
	PUTC(fileLenMB/256);
	PUTC(fileLenMB%256);
	PUTC(additionalParam);
	if (IF_OPTION(OPTION_BINARY_DATA))
	{
		
		getcBuffer[0]=0;
		getcBufferData=&getcBuffer[1];
		while (true)
		{
			READ_CHAR(c);
			if (c<0)
				break;
			ENCODE_PUTC(c);
		}
		cont.writeMemBuffers(preprocFlag,additionalParam);
		cont.freeMemBuffers(true);
		return;
	}
	PRINT_DICT(("maxMemSize=%d fileLenMB=%d preprocType=%d\n",maxMemSize,fileLenMB,preprocType));
	write_dict(additionalParam); // przed initialize()
	memset(detectedSymbols,0,sizeof(detectedSymbols));
	decoding=false;
	WRT_deinitialize();
	if (!initialize(true))
		return;
	memset(value,0,sizeof(value));
	if (!IF_OPTION(OPTION_PAQ))
	{
		PUTC(1*detectedSymbols[0]+2*detectedSymbols[1]+4*detectedSymbols[2]+8*detectedSymbols[3]+16*detectedSymbols[4]+32*detectedSymbols[5]+64*detectedSymbols[6]+128*detectedSymbols[7]);
		PUTC(1*detectedSymbols[8]+2*detectedSymbols[9]+4*detectedSymbols[10]+8*detectedSymbols[11]+16*detectedSymbols[12]+32*detectedSymbols[13]+64*detectedSymbols[14]+128*detectedSymbols[15]);
		PUTC(1*detectedSymbols[16]+2*detectedSymbols[17]+4*detectedSymbols[18]+8*detectedSymbols[19]+16*detectedSymbols[20]+32*detectedSymbols[21]+64*detectedSymbols[22]+128*detectedSymbols[23]);
	}
	else
	{
	}
	
	WRT_encode(getcBufferSize);
	cont.writeMemBuffers(preprocFlag,additionalParam);
	cont.freeMemBuffers(true);
}
inline void XWRT_Encoder::setSpaces(int c)
{
	if (IF_OPTION(OPTION_SPACELESS_WORDS))
	if (spaces>0 && ((c>='a' && c<='z') || (c>='A' && c<='Z')))
		spaces--;
	if (IF_OPTION(OPTION_SPACES_MODELING) && spaces>1 && spaces<256)
		spacesCont[spaces]++;
	spaces=0;
}
inline void XWRT_Encoder::checkWord(unsigned char* &s,int &s_size,int& c)
{
	if (s_size<1)
	{
		setSpaces('-');
		return;
	}
	if (s_size>WORD_MAX_SIZE)
		s_size=WORD_MAX_SIZE; 
	/*if (XMLState==CLOSE || XMLState==CLOSE_EOL)
	{
		XMLState=CLOSED;
		setSpaces(s[0]);
		return;
	}*/
	/*if (XMLState==OPEN)
	{
		if (c!='!' && c!='?')
			XMLState=ADDED;
		else
			XMLState=INSIDE;
		if (c=='>')
		{
			s[s_size++]=c;
			XMLState=ADDED2;
		}
	}*/
	/*if (s[0]=='<' && (XMLState==ADDED || XMLState==ADDED2))
	{
		if (spaces+s_size<STRING_MAX_SIZE)
		{
			memmove(s+spaces,s,s_size);
			memset(s,' ',spaces);
			s_size+=spaces;
		}
		spaces=0;
	}
	else*/
		setSpaces(s[0]);
	if (s_size<WORD_MIN_SIZE)
	{
		setSpaces('-');
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
int XWRT_Encoder::WRT_detectFileType()
{
	detect=true;
	memset(value,0,sizeof(value));
	memset(addSymbols,0,sizeof(addSymbols));
	memset(reservedSet,0,sizeof(reservedSet));
	memset(spacesCont,0,sizeof(spacesCont));
	quotes=0;
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
	    clock_t start_time=clock();
		if (fileLenMB>0)
			getcBufferDataParts=1+((1024*1024/(mFileBufferSize))*(fileLenMB+1))/firstPassBlock;
	PRINT_DICT(("firstPassBlock=%d fileLenMB=%d getcBufferDataParts=%d\n",firstPassBlock,fileLenMB,getcBufferDataParts));
#ifdef DYNAMIC_DICTIONARY
		getcBufferDataParts=2;
#endif
		WRT_encode(0);
		PRINT_DICT(("bincount=%d/%d\n",binCount,ftell(XWRT_file)/100));
		if (binCount>ftell(XWRT_file)/100) // (for textual files in UTF-8)
			TURN_ON(OPTION_BINARY_DATA);
		if (value[13]>value[10]/2)
			TURN_ON(OPTION_CRLF);
	    PRINT_DICT(("+ WRT_detectFileType time %1.2f sec\n",double(clock()-start_time)/CLOCKS_PER_SEC));
		WRT_detectFinish();
	}
	WRT_deinitialize();
	if (collision>0)
		PRINT_DICT(("warning: hash collisions=%d\n",collision));
	detect=false;
	getcBufferDataParts=0;
	WRT_print_options();
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
	if (IF_OPTION(OPTION_QUOTES_MODELING))
		add+=2;
	if (IF_OPTION(OPTION_SPACES_MODELING))
	{
		for (i=0; i<256; i++)
		if (spacesCont[i]>=minSpacesFreq())
			add++;
	}
	dict1size-=add;
	bound3-=add;
	bound4-=add;
	int* inttable=new int[size];
	if (!inttable)
		OUT_OF_MEMORY();
	for (i=0; i<size; i++)
		inttable[i]=i+1;
	qsort(&inttable[0],size,sizeof(inttable[0]),compare_freq);
	
	{
		qsort(&inttable[0],min(size,dict1size),sizeof(inttable[0]),compare_str);
		
		if (size>dict1size)
			qsort(&inttable[dict1size],min(size,bound3)-dict1size,sizeof(inttable[0]),compare_str);
		
		if (size>bound3)
			qsort(&inttable[bound3],min(size,bound4)-bound3,sizeof(inttable[0]),compare_str);
		
		if (size>bound4)
			qsort(&inttable[bound4],size-bound4,sizeof(inttable[0]),compare_str);
	}
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
	TURN_OFF(OPTION_SPACES_MODELING);
	for (i=0; i<256; i++)
		if (spacesCont[i]>=minSpacesFreq())
			TURN_ON(OPTION_SPACES_MODELING);
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
	if (quotes>=minSpacesFreq())
		TURN_ON(OPTION_QUOTES_MODELING);
	sortDict(sizeDict);
	PRINT_DICT(("quotes=%d\n",quotes));
}
