#include "XWRT.h"
#include "Decoder.h"
#include <time.h>

//extern int g_fileLenMB;

XWRT_Decoder::XWRT_Decoder() : WRTd_s(&WRTd_data[0]), utf8pos(0)  
{ 	
	putcBuffer=new unsigned char[mFileBufferSize];
	

	if (!putcBuffer )
		OUT_OF_MEMORY();


};

XWRT_Decoder::~XWRT_Decoder() 
{ 
	if (putcBuffer)
		delete(putcBuffer);

	
}

#define WRITE_CHAR(c)\
{ \
	putcBufferData[0]=c; \
	putcBufferData++; \
	putcBufferSize++; \
	if (putcBufferSize>=mFileBufferSize) \
		writePutcBuffer(XWRT_fileout); \
}

#define DECODE_PUTC(c)\
{\
	if (IF_OPTION(OPTION_UNICODE_BE) || IF_OPTION(OPTION_UNICODE_LE)) \
	{ \
		if (c<0x80) \
		{ \
			if (IF_OPTION(OPTION_UNICODE_LE)) \
			{ \
				WRITE_CHAR(c); \
				WRITE_CHAR(0); \
			} \
			else \
			{ \
				WRITE_CHAR(0); \
				WRITE_CHAR(c); \
			} \
		} \
		else \
		{ \
			utf8buff[utf8pos++]=c; \
			if (sequence_length(utf8buff[0])==utf8pos) \
			{ \
				int d=utf8_to_unicode(&utf8buff[0]); \
				utf8pos=0; \
			\
				if (IF_OPTION(OPTION_UNICODE_LE)) \
				{ \
					if (d<65536) \
					{ \
						WRITE_CHAR(d%256); \
						WRITE_CHAR(d/256); \
					} \
					else \
					{ \
						d-=65536; \
						WRITE_CHAR(d%256); \
					} \
				} \
				else \
				{ \
					if (d<65536) \
					{ \
						WRITE_CHAR(d/256); \
						WRITE_CHAR(d%256); \
					} \
					else \
					{ \
						d-=65536; \
						WRITE_CHAR(d/256); \
					} \
				} \
			} \
		} \
	} \
	else \
		WRITE_CHAR(c); \
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

void XWRT_Decoder::writePutcBuffer(FILE* &fileout)
{
	//printStatus(0,(int)putcBufferSize,false);
	size_t res=fwrite_fast(putcBuffer,putcBufferSize,fileout);
	putcBufferData-=res;
	putcBufferSize-=res;
}

inline unsigned char XWRT_Decoder::mask8(unsigned char oc)
{
	return static_cast<unsigned char>(0xff & oc);
}

inline int XWRT_Decoder::sequence_length(unsigned char lead_it)
{
        unsigned char lead = mask8(lead_it);
        if (lead < 0x80) 
            return 1;
        else if ((lead >> 5) == 0x6)
            return 2;
        else if ((lead >> 4) == 0xe)
            return 3;
        else if ((lead >> 3) == 0x1e)
            return 4;
        else 
            return 0;
}

inline unsigned int XWRT_Decoder::utf8_to_unicode(unsigned char* it)
{
	unsigned int cp = mask8(*it);
	int length = sequence_length(*it);
	switch (length) {
		case 1:
			break;
		case 2:
			it++;
			cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
			break;
		case 3:
			++it; 
			cp = ((cp << 12) & 0xffff) + ((mask8(*it) << 6) & 0xfff);
			++it;
			cp += (*it) & 0x3f;
			break;
		case 4:
			++it;
			cp = ((cp << 18) & 0x1fffff) + ((mask8(*it) << 12) & 0x3ffff);                
			++it;
			cp += (mask8(*it) << 6) & 0xfff;
			++it;
			cp += (*it) & 0x3f; 
			break;
	}
	++it;
	return cp;        
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





inline unsigned char* XWRT_Decoder::uint2string(unsigned int x,unsigned char* buf,int size)
{
	int i=size-1;

	buf[i]=0;
	i--;
	while (x>=10 && i>=0)
	{
		buf[i]='0'+x%10;
		x=x/10;
		i--;
	}
	buf[i]='0'+x%10;

	return &buf[i];
}


inline void XWRT_Decoder::hook_putc(int& c)
{
	if (c==EOF)
		return;


	last_c=c;

	DECODE_PUTC(c);
	return;
}

void XWRT_Decoder::WRT_decode()
{
	while (WRTd_c!=EOF)
	{
		if (fileCorrupted)
			return;

		PRINT_CHARS(("c=%d (%c)\n",WRTd_c,WRTd_c));

		if (outputSet[WRTd_c])
		{
			PRINT_CHARS(("addSymbols[%d] upperWord=%d\n",WRTd_c,upperWord));
			static std::string str;

			switch (WRTd_c)
			{
				case CHAR_ESCAPE:
					WRTd_upper=false;
					upperWord=UFALSE;

					DECODE_GETC(WRTd_c);
					PRINT_CHARS(("c==CHAR_ESCAPE, next=%x\n",WRTd_c));
					hook_putc(WRTd_c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_CRLF:
					if (!IF_OPTION(OPTION_CRLF))
						break;
					PRINT_CHARS(("c==CHAR_CRLF\n"));

					WRTd_c=13; hook_putc(WRTd_c);
					WRTd_c=10; hook_putc(WRTd_c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_END_TAG:
				case CHAR_END_TAG_EOL:
					PRINT_CHARS(("c==CHAR_END_TAG\n"));
					int c,i;
					c='<'; DECODE_PUTC(c);
					c='/'; DECODE_PUTC(c);

					if (stack.size()>0)
					{
						str=stack.back();
						stack.pop_back();
						PRINT_STACK(("pop2 %s\n",str.c_str()));
					}
					else
						str.erase();

					for (i=0; i<(int)str.size(); i++)
					{
						c=str[i];
						DECODE_PUTC(c);
					}

					c='>'; DECODE_PUTC(c);
					last_c=c;

					if (WRTd_c==CHAR_END_TAG_EOL)
					{
						if (IF_OPTION(OPTION_CRLF))
						{
							c=13;
							DECODE_PUTC(c);
						}
						c=10; 
						DECODE_PUTC(c);
						last_c=c;
					}

					cont.MemBufferPopBack();

					DECODE_GETC(WRTd_c);
					continue;


				case CHAR_FIRSTUPPER:
					PRINT_CHARS(("c==CHAR_FIRSTUPPER\n"));

					if (IF_OPTION(OPTION_SPACE_AFTER_CC_FLAG))
						DECODE_GETC(WRTd_c); // skip space

					WRTd_upper=true;
					upperWord=UFALSE;
					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_UPPERWORD:
					PRINT_CHARS(("c==CHAR_UPPERWORD\n"));

					if (IF_OPTION(OPTION_SPACE_AFTER_CC_FLAG))
						DECODE_GETC(WRTd_c); // skip space

					upperWord=FORCE;
					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_NOSPACE:
					if (!IF_OPTION(OPTION_SPACELESS_WORDS))
						break;

					PRINT_CHARS(("c==CHAR_NOSPACE\n"));

					if (upperWord==FORCE)
						upperWord=UTRUE;

					DECODE_GETC(WRTd_c);
					forceSpace=true;
					continue;

#ifdef DYNAMIC_DICTIONARY
				case CHAR_NEWWORD:

					//if (IF_OPTION(OPTION_LETTER_CONTAINER))
					//	break;

					s_size=0;
					while (true)
					{
						DECODE_GETC(WRTd_c);
						if (WRTd_c!=0)
							WRTd_s[s_size++]=WRTd_c;
						else
							break;
					}
							
					if (s_size>0)
					{
						memcpy(mem,WRTd_s,s_size);
						
						if (mem<dictmem_end && addWord(mem,s_size)!=0)
						{	
							mem+=(s_size/4+1)*4;
						}
					}
					
					for (i=0; i<s_size; i++)
					{
						c=WRTd_s[i];
						hook_putc(c);
					}
					
					DECODE_GETC(WRTd_c);
					continue;
#endif

			}


			if (upperWord==FORCE)
				upperWord=UTRUE;
			else
				upperWord=UFALSE;
			
			
					s_size=decodeCodeWord(WRTd_s,WRTd_c);
				
			int i;
				
			if (IF_OPTION(OPTION_SPACELESS_WORDS))
			{
				letterType=letterSet[WRTd_s[0]];

				if ((letterType==LOWERCHAR || letterType==UPPERCHAR))// && last_c!=' ')
				{
					beforeWord=last_c;
					if (forceSpace || (beforeWord=='/' || beforeWord=='-' || beforeWord=='\"' || beforeWord=='_' || beforeWord=='>'))
						forceSpace=false;
					else
					{
						int c=' ';
						hook_putc(c);
					}
				}
			}
			
			if (WRTd_upper)
			{
				WRTd_upper=false;
				WRTd_s[0]=toupper(WRTd_s[0]);
			}
			
			if (upperWord!=UFALSE)
				toUpper(&WRTd_s[0],s_size);
			
			upperWord=UFALSE;
			
			
			
			int c;
			for (i=0; i<s_size; i++)
			{
				c=WRTd_s[i];
				hook_putc(c);
			}
			
			DECODE_GETC(WRTd_c);
			continue;
		}

		

		if ((/*XMLState!=OPEN && XMLState!=CLOSE &&*/ (WRTd_c>='0' && WRTd_c<='9')) || (WRTd_c>=CHAR_IP && WRTd_c<=CHAR_TIME))
		{
			unsigned int no,mult;
			int c,i;
			no=0;
			mult=1;
			static int wType=0;
			static std::string mon;
			int day,month,year,newAll;

			switch (WRTd_c)
			{
				case CHAR_REMAIN:
					
						DECODE_GETC(WRTd_c);

					if (WRTd_c>=100)
					{
						c='0'+(WRTd_c/100); hook_putc(c);
					}
					c='0'+(WRTd_c/10)%10; hook_putc(c);
					c='.'; hook_putc(c);
					c='0'+WRTd_c%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_IP:
					int x1;

					for (i=0; i<4; i++)
					{
						
							DECODE_GETC(x1);
				
						if (x1>=100) { c='0'+x1/100; hook_putc(c); }
						if (x1>=10) { c='0'+(x1/10)%10; hook_putc(c); }
						c='0'+x1%10; hook_putc(c);
						if (i<3) { c='.'; hook_putc(c); }
					}

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_HOURMINSEC:
					int h,m,s;

					{
						DECODE_GETC(h);
						DECODE_GETC(m);
						DECODE_GETC(s);
					}

					c='0'+(h/10)%10; hook_putc(c);
					c='0'+h%10; hook_putc(c);
					c=':'; hook_putc(c);
					c='0'+(m/10)%10; hook_putc(c);
					c='0'+m%10; hook_putc(c);
					c=':'; hook_putc(c);
					c='0'+(s/10)%10; hook_putc(c);
					c='0'+s%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_HOURMIN:

					{
						DECODE_GETC(h);
						DECODE_GETC(m);
					}

					c='0'+(h/10)%10; hook_putc(c);
					c='0'+h%10; hook_putc(c);
					c=':'; hook_putc(c);
					c='0'+(m/10)%10; hook_putc(c);
					c='0'+m%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_PAGES:
					
					{
						DECODE_GETC(WRTd_c);
						no=256*WRTd_c;
						DECODE_GETC(WRTd_c);
						no+=WRTd_c;
						DECODE_GETC(WRTd_c);
					}

					WRTd_c+=no;

					if (WRTd_c>=1000)
					{
						c='0'+no/1000; hook_putc(c);
					}
					c='0'+(no/100)%10; hook_putc(c);
					c='0'+(no/10)%10; hook_putc(c);
					c='0'+no%10; hook_putc(c);
					c='-'; hook_putc(c);
					if (WRTd_c>=1000)
					{
						c='0'+WRTd_c/1000; hook_putc(c);
					}
					c='0'+(WRTd_c/100)%10; hook_putc(c);
					c='0'+(WRTd_c/10)%10; hook_putc(c);
					c='0'+WRTd_c%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_TIME:
					
					{
						DECODE_GETC(day);
						DECODE_GETC(month);
					}

					day++;
					if (day>12)
					{
						day-=12;
						year=1;
					}
					else
						year=0;

					if (day>=10)
					{
						c='0'+day/10; hook_putc(c);
					}
					c='0'+day%10; hook_putc(c);

					c=':'; hook_putc(c);

					c='0'+month/10; hook_putc(c);
					c='0'+month%10; hook_putc(c);

					if (year)
						c='p';
					else
						c='a';

					hook_putc(c);
					c='m'; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;
				case CHAR_DATE_ENG:

					
					{
						DECODE_GETC(no);
						DECODE_GETC(WRTd_c);
					}

					no+=256*WRTd_c;

					day=no%31+1;
					no=no/31;
					month=no%12+1;
					year=no/12+1929;

					c='0'+day/10; hook_putc(c);
					c='0'+day%10; hook_putc(c);
					c='-'; hook_putc(c);
					switch (month)
					{
						case 1: mon="JAN"; break;
						case 2: mon="FEB"; break;
						case 3: mon="MAR"; break;
						case 4: mon="APR"; break;
						case 5: mon="MAY"; break;
						case 6: mon="JUN"; break;
						case 7: mon="JUL"; break;
						case 8: mon="AUG"; break;
						case 9: mon="SEP"; break;
						case 10: mon="OCT"; break;
						case 11: mon="NOV"; break;
						default: mon="DEC"; break;
					}

					c=mon[0]; hook_putc(c);
					c=mon[1]; hook_putc(c);
					c=mon[2]; hook_putc(c);
					c='-'; hook_putc(c);
					c='0'+year/1000; hook_putc(c);
					c='0'+(year/100)%10; hook_putc(c);
					c='0'+(year/10)%10; hook_putc(c);
					c='0'+year%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;
				case '6':
					wType=2;
					DECODE_GETC(WRTd_c);
					continue;

				case '7':
					
						DECODE_GETC(WRTd_c);

					c='.'; hook_putc(c);
					c='0'+(WRTd_c/10)%10; hook_putc(c);
					c='0'+WRTd_c%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;
				case '8':
				case '9':
					if (WRTd_c=='8')
					{
						
						{
							DECODE_GETC(no);
							DECODE_GETC(WRTd_c);
						}
						no+=256*WRTd_c;
						newAll=no+lastAll-(65536/2);
					}
					else
					{
						
							DECODE_GETC(no);
						newAll=no+lastAll-(256/2);
					}

					no=newAll;

					day=no%31+1;
					no=no/31;
					month=no%12+1;
					year=no/12+1929;

					c='0'+year/1000; hook_putc(c);
					c='0'+(year/100)%10; hook_putc(c);
					c='0'+(year/10)%10; hook_putc(c);
					c='0'+year%10; hook_putc(c);
					c='-'; hook_putc(c);
					c='0'+month/10; hook_putc(c);
					c='0'+month%10; hook_putc(c);
					c='-'; hook_putc(c);
					c='0'+day/10; hook_putc(c);
					c='0'+day%10; hook_putc(c);

					lastAll=newAll;

					DECODE_GETC(WRTd_c);
					continue;
				case '5':
					
						DECODE_GETC(WRTd_c);

					no=1900+(WRTd_c);

					if (wType==2)
						wType=3;
					else
					if (wType==3)
					{
						WRTd_c='-'; hook_putc(WRTd_c);
						wType=0;
					}
					break;
				default:
					c=WRTd_c-'0';

					
					{
						for (i=0; i<c; i++)
						{
							DECODE_GETC(WRTd_c);	

							no+=mult*WRTd_c;
							mult*=NUM_BASE;
						}
					}
					

					if (wType==2)
						wType=3;
					else
					if (wType==3)
					{
						WRTd_c='-'; hook_putc(WRTd_c);
						wType=0;
					}
			}

			unsigned char* numdata;
			numdata=uint2string(no,num,sizeof(num));

			while (numdata[0])
			{
				c=numdata[0];
				hook_putc(c);
				numdata++;
			}

			DECODE_GETC(WRTd_c);
			continue;
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

		hook_putc(WRTd_c);

		DECODE_GETC(WRTd_c);
	}

	WRTd_c=EOF;
	hook_putc(WRTd_c);

	writePutcBuffer(XWRT_fileout);
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
		//printStatus(count,0,false);
	}


	if (IF_OPTION(OPTION_SPACES_MODELING))
	{
		for (i=0; i<256; i++)
			spacesCont[i]=0;

		count=bufferData[0]; bufferData++;

		PRINT_DICT(("sp_count=%d\n",count));

		for (i=0; i<count; i++)
		{
			c=bufferData[0]; bufferData++;
			spacesCont[c]=minSpacesFreq();
		}
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

void XWRT_Decoder::WRT_set_options(char c,char c2)
{
	/*if ((c&128)==0)
		TURN_OFF(OPTION_USE_CONTAINERS)
	else
		TURN_ON(OPTION_USE_CONTAINERS);
*/
	if ((c&64)==0)
		TURN_OFF(OPTION_PAQ)
	else
		TURN_ON(OPTION_PAQ);

	

	

	if ((c&4)==0)
		TURN_OFF(OPTION_BINARY_DATA)
	else
		TURN_ON(OPTION_BINARY_DATA);


	//if ((c2&128)==0)
	//	TURN_OFF(OPTION_LETTER_CONTAINER)
	//else
	//	TURN_ON(OPTION_LETTER_CONTAINER);

	/*if ((c2&64)==0)
		TURN_OFF(OPTION_NUMBER_CONTAINER)
	else
		TURN_ON(OPTION_NUMBER_CONTAINER);
*/
	if ((c2&32)==0)
		TURN_OFF(OPTION_SPACES_MODELING)
	else
		TURN_ON(OPTION_SPACES_MODELING);

	if ((c2&16)==0)
		TURN_OFF(OPTION_CRLF)
	else
		TURN_ON(OPTION_CRLF);

	if ((c2&8)==0)
		TURN_OFF(OPTION_QUOTES_MODELING)
	else
		TURN_ON(OPTION_QUOTES_MODELING);

	//if ((c2&4)==0)
	//	TURN_OFF(OPTION_USE_DICTIONARY)
	//else
	//	TURN_ON(OPTION_USE_DICTIONARY);

	if ((c2&2)==0)
		TURN_OFF(OPTION_UNICODE_LE)
	else
		TURN_ON(OPTION_UNICODE_LE);

	if ((c2&1)==0)
		TURN_OFF(OPTION_UNICODE_BE)
	else
		TURN_ON(OPTION_UNICODE_BE);
}

void XWRT_Decoder::WRT_start_decoding(FILE* in, FILE* out)
{
	int i,j,k,c, c2,dictPathLen;
	//unsigned char s[STRING_MAX_SIZE];
	//unsigned char dictPath[STRING_MAX_SIZE];
	//s[0]=0;
//	fseek(in,0, SEEK_SET);
XWRT_file=in;
XWRT_fileout=out;
	lastAll=0;
	//XMLState=UNKNOWN;	
	last_c=0;
	forceSpace=false;
	WRTd_upper=false;
	upperWord=UFALSE;
	s_size=0;
	WRTd_xs_size=0;
	collision=0;
//	int bbb=getc(in);
//	if (bbb==-1) 	printf("Decode eof");
 //   bbb=fgetc(in);
   // printf("1 %c",bbb);
 //   bbb=fgetc(in);
  //  printf("2 %c",bbb);
  //  bbb=fgetc(in);
   // printf("3 %c",bbb);
  //  bbb=fgetc(XWRT_file);
   // printf("4 %c\n",bbb);
//if (getc(XWRT_file)==XWRT_HEADER[0] || getc(XWRT_file)==XWRT_HEADER[1] || getc(XWRT_file)==XWRT_HEADER[2] || getc(XWRT_file)!=XWRT_HEADER[3] )
	{
	//	printf("Bad XWRT header in decoder! %c%c%c%c\n",getc(XWRT_file),getc(XWRT_file),getc(XWRT_file),getc(XWRT_file));
		//return;
	}
	//if ( getc(XWRT_file)!=XWRT_VERSION-150)
	{
	//	printf("Bad  version XWRT version in decoder!\n");
	//	return;
	}
	
GETC(c);
	preprocType=(EPreprocessType)(c%4); // { LZ77, LZMA/BWT, PPM, PAQ }
	
	GETC(c2);

	defaultSettings(); // after setting preprocType 
	WRT_set_options(c,c2);

	GETC(maxMemSize); // after defaultSettings()
	maxMemSize*=1024*1024;

	GETC(additionalParam); // fileLenMB/256
	GETC(fileLenMB); // fileLenMB%256
	fileLenMB+=256*additionalParam;
	//g_fileLenMB=fileLenMB;

	GETC(additionalParam);



	WRT_print_options();

	PRINT_DICT(("maxMemSize=%d fileLenMB=%d preprocType=%d\n",maxMemSize,fileLenMB,preprocType));


	if (IF_OPTION(OPTION_BINARY_DATA))
	{
		cont.readMemBuffers(preprocFlag,maxMemSize);
		cont.memout->memsize=0;

		putcBufferData=&putcBuffer[0];
		putcBufferSize=0;

		while (true)
		{
			DECODE_GETC(c);
			if (c<0)
				break;
			WRITE_CHAR(c);
		}

		writePutcBuffer(XWRT_fileout);

		if (cont.bigBuffer)
		{
			free(cont.bigBuffer);
			cont.bigBuffer=NULL;
			cont.freeMemBuffers(false);
		}
		else
			cont.freeMemBuffers(true);


		return;
	}


	read_dict();


	memset(detectedSymbols,0,sizeof(detectedSymbols));

	if (!IF_OPTION(OPTION_PAQ))
		GETC(i)


	k=1;
	for (j=0; j<8; j++)
	{
		if (i & k) detectedSymbols[j]=1;
		k*=2;
	}

	if (!IF_OPTION(OPTION_PAQ))
		GETC(i)


	k=1;
	for (j=8; j<16; j++)
	{
		if (i & k) detectedSymbols[j]=1;
		k*=2;
	}

	if (!IF_OPTION(OPTION_PAQ))
		GETC(i)


	k=1;
	for (j=16; j<24; j++)
	{
		if (i & k) detectedSymbols[j]=1;
		k*=2;
	}
	
	
	//dictPathLen=getSourcePath((char*)dictPath,sizeof(dictPath));


	/*if (dictPathLen>0)
	{
		dictPath[dictPathLen]=0;
		strcat((char*)dictPath,(char*)s);
		strcat((char*)dictPath,(char*)"wrt-eng.dic");
		strcpy((char*)s,(char*)dictPath);
	}*/

	cont.readMemBuffers(preprocFlag,maxMemSize);
	cont.memout->memsize=0;

	WRT_deinitialize();

	decoding=true;
	if (!initialize(false))
		return;

	putcBufferData=&putcBuffer[0];
	putcBufferSize=0;

	DECODE_GETC(WRTd_c);
	PRINT_CHARS(("WRT_start_decoding WRTd_c=%d ftell=%d\n",WRTd_c,ftell(XWRT_file)));

	WRT_decode(); 

	if (cont.bigBuffer)
	{
		free(cont.bigBuffer);
		cont.bigBuffer=NULL;
		cont.freeMemBuffers(false);
	}
	else
		cont.freeMemBuffers(true);


}


