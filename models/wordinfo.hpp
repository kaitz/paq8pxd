#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirect.hpp"
//#include "../prt/indirectcontext.hpp"
#include "../prt/contextmap.hpp"
//#include "../prt/sscm.hpp"
//#include "../prt/ols.hpp"
#include "../prt/wrt/wrton.hpp"
#include "../prt/stemmer/stemmer.hpp"
#include "../prt/tables.hpp"

class Info {
   BlockData& x;
   Buf& buf;
    ContextMap  &cm;
    U32 word0, word1, word2, word3, word4, word5,word6,word7;  // hashes
    U32 wrdhsh;
    U32 xword0,xword1,xword2,xword3,cword0,ccword,fword;
    U32 number0, number1,hq;  // hashes
    U32 text0,data0,type0;  // hash stream of letters
    int nl3,nl2,nl1, nl;  // previous, current newline position
    U32 mask,mask2;
    Array<int> wpos;  // last position of word
    Array<U16> wchk ;
    Array<U32> inkeyw;
    Array<U32> inkeyw1;
    int w;
    bool doXML;
    U32 lastLetter,firstLetter, lastUpper, lastDigit,wordGap;
    Array<Word> StemWords;
    Word *cWord, *pWord;
    EnglishStemmer StemmerEN;
    int StemIndex,same,linespace;
    int islink,istemplate;
    bool is_letter, is_letter_pC, is_letter_ppC;
    U32 numbers,numbercount;
    U64 g_ascii_lo=0, g_ascii_hi=0; // group identifiers of 8+8 last characters
    U8 c, pC, ppC;           // last char, previous char, char before the previous char (converted to lowearcase)
    U32 c4;
    U32 ccount,col1;
    bool firstwasspace;
    U8 opened;
    U32 line0;
    U32 iword0;
    U32 lastSpace;
    bool pdft;
    U16 chk;
    int above, above1, above2,fl,f;
    U32 expr0, expr1, expr2, expr3, expr4,exprlen0,expr0chars,wchar1,wchar2,wrth;  // wordtoken hashes for expressions
    int wcol,wnl2,wnl1,wnl,wabove,scount;
    bool scountset;
    U32 wpword1;
  public:
   
    Info(BlockData& bd,U32 val, ContextMap  &contextmap);
    void reset() ;
    // forget it
    void shrwords() ;
    void killwords();
    void process_char(const int is_extended_char,int val1=0,int val2=0);

    int predict(const U8 pdf_text_parser_state,int val1=0,int val2=0);
  };
