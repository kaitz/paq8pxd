#if !defined xwrt_h_included
#define xwrt_h_included

#define PRG_NAME "XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com"
#define XWRT_VERSION 320 // 150-405

//#define XWRT_HEADER "XWRC"
//#define ADD_FILENAME_EXT ".xwrt" 
//#define CUT_FILENAME_CHAR '.'

#define PRINT_CHARS(data) ;//printf data
#define PRINT_CODEWORDS(data) ;//printf data
#define PRINT_STACK(data) ;//printf data;
#define PRINT_DICT(data);// printf data;
#define PRINT_CONTAINERS(data) ;//printf data
#define PRINT_STATUS(data) printf data;

#pragma warning(disable:4244) //  '=' : conversion from ... to ..., possible loss of data
#pragma warning(disable:4786) // STL warnings
#pragma warning(disable:4996) // '_getch' was declared deprecated
#pragma warning(disable:4503) // STL
#pragma warning(disable:4390) // empty controlled statement found; is this the intent?
#pragma warning(disable:4018) // signed/unsigned mismatch
#define _CRT_SECURE_NO_DEPRECATE // VC++ 2005 deprecate warnings


#if defined WIN32 || defined WIN64


	#define getch _getch
#else

	
	#define getch getchar
#endif
#ifndef WINDOWS
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}
#endif



#define CHAR_FIRSTUPPER		1	// for encode lower word with first capital letter
#define CHAR_UPPERWORD		2	// for encode upper word
#define CHAR_NOSPACE		3
#define CHAR_END_TAG		4   
#define CHAR_END_TAG_EOL	5
#define CHAR_ESCAPE			6	// for encode reserved chars (CHAR_ESCAPE,CHAR_FIRSTUPPER,...)
#define CHAR_CRLF			7
#define CHAR_NEWWORD		8 
#define CHAR_IP				25
#define CHAR_HOURMIN		26
#define CHAR_HOURMINSEC		27
#define CHAR_PAGES			28
#define CHAR_REMAIN			29
#define CHAR_DATE_ENG		30
#define CHAR_TIME			31
#define BINARY_FIRST		128
#define BINARY_LAST			255
#define LETTER_LAST			'E'

#define OPTION_SPACELESS_WORDS				1
#define OPTION_SPACE_AFTER_CC_FLAG			2
#define OPTION_TRY_SHORTER_WORD				4
#define OPTION_ADD_SYMBOLS_0_5				8
#define OPTION_ADD_SYMBOLS_14_31			16
#define OPTION_ADD_SYMBOLS_MISC				32



#define OPTION_PAQ							512
//#define OPTION_USE_DICTIONARY				1024
#define OPTION_CRLF							2048
#define OPTION_SPACES_MODELING				4096
#define OPTION_QUOTES_MODELING				8192
//#define OPTION_USE_CONTAINERS				16384
//#define OPTION_LETTER_CONTAINER				32768
//#define OPTION_NUMBER_CONTAINER				65536
#define OPTION_BINARY_DATA					131072
#define OPTION_UNICODE_LE					262144
#define OPTION_UNICODE_BE					524288


#if !defined min
	#define min(a,b) (((a)>(b))?(b):(a))
#endif
#define IF_OPTION(option) (preprocFlag & option)
#define OPTION(option) (xml_wrt.preprocFlag & option)
#define TURN_OFF(option) {if (preprocFlag & option) preprocFlag-=option;}
#define TURN_ON(option) {if ((preprocFlag & option)==0) preprocFlag+=option;}
#define RESET_OPTIONS (preprocFlag=0)
//template <class T> inline T CLAMP(const T& X,const T& LoX,const T& HiX) { return (X >= LoX)?((X <= HiX)?(X):(HiX)):(LoX); }

#define WORD_MIN_SIZE		2
#define WORD_AVG_SIZE		8
#define WORD_MAX_SIZE		48
#define STRING_MAX_SIZE		255  // 1-byte for container.size()

//#define COMPRESS	1
//#define DECOMPRESS	2

#define MAX_DYNAMIC_DICT_COUNT	(65536*256)
#define HASH_TABLE_SIZE			(1<<20) //1MB*4

#define BYTES_TO_DETECT			(50*1024)

#define NUM_BASE			256
#define HASH_DOUBLE_MULT	37
#define HASH_MULT			23
#define CHARSET_COUNT		6


enum EWordType { LOWERWORD, FIRSTUPPER, UPPERWORD, VARWORD, NUMBER, NUMBER2, NUMBER3 };
//enum EXMLState { UNKNOWN, OPEN, CLOSE, CLOSE_EOL, CLOSED, ADDED, ADDED2, INSIDE };
enum EPreprocessType {  PAQ };
enum ELetterType { XMLCHAR, LOWERCHAR, UPPERCHAR, UNKNOWNCHAR, RESERVEDCHAR, NUMBERCHAR };


//int g_fileLenMB;

#define OUT_OF_MEMORY() \
{ \
	/*printf("Not enough memory!\n"); */\
	exit(0); \
}


#define PUTC(c) { putc(c,XWRT_fileout); }
#define GETC(c) { c=getc(XWRT_file); }


#include <stdio.h>

extern FILE* XWRT_file;
extern FILE* XWRT_fileout;

#endif
