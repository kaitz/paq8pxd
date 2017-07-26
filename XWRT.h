#if !defined xwrt_h_included
#define xwrt_h_included

// based on  "XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com"
//

#define PRINT_CHARS(data) ;//printf data
#define PRINT_CODEWORDS(data); // printf data
#define PRINT_STACK(data) ;//printf data;
#define PRINT_DICT(data) ;//printf data;
#define PRINT_CONTAINERS(data) ;//printf data
//#define PRINT_STATUS(data) printf data;

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
#define CHAR_ESCAPE			3	// for encode reserved chars (CHAR_ESCAPE,CHAR_FIRSTUPPER,...)
#define BINARY_FIRST		128
#define BINARY_LAST			255

#define OPTION_TRY_SHORTER_WORD				4


#if !defined min
	#define min(a,b) (((a)>(b))?(b):(a))
#endif
#define IF_OPTION(option) (preprocFlag & option) //, printf("%d",option)
#define OPTION(option) (xml_wrt.preprocFlag & option)
#define TURN_OFF(option) {if (preprocFlag & option) preprocFlag-=option;}
#define TURN_ON(option) {if ((preprocFlag & option)==0) preprocFlag+=option;}
#define RESET_OPTIONS (preprocFlag=0)

#define WORD_MIN_SIZE		2
#define WORD_AVG_SIZE		8
#define WORD_MAX_SIZE		48
#define STRING_MAX_SIZE		255  // 1-byte for container.size()

#define MAX_DYNAMIC_DICT_COUNT	(65536*256)
#define HASH_TABLE_SIZE			(1<<20) //1MB*4

//#define BYTES_TO_DETECT			(50*1024)

//#define NUM_BASE			256
#define HASH_DOUBLE_MULT	37
#define HASH_MULT			23
//#define CHARSET_COUNT		6


enum EWordType { LOWERWORD, FIRSTUPPER, UPPERWORD, VARWORD, NUMBER};
enum ELetterType { LOWERCHAR, UPPERCHAR, UNKNOWNCHAR, RESERVEDCHAR, NUMBERCHAR };


#define OUT_OF_MEMORY() \
{ \
	printf("Not enough memory!\n");\
	exit(0); \
}


#define PUTC(c) { putc(c,XWRT_fileout); }
#define GETC(c) { c=getc(XWRT_file); }


#include <stdio.h>

extern FILE* XWRT_file;
extern FILE* XWRT_fileout;

#endif
