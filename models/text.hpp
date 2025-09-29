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
#include "../prt/contextmap2.hpp"
//#include "../prt/sscm.hpp"
//#include "../prt/ols.hpp"
//#include "../prt/wrt/wrton.hpp"
#include "../prt/stemmer/stemmer.hpp"
#include "../prt/tables.hpp"

template <class T, const U32 Size> class Cache {
  static_assert(Size>1 && (Size&(Size-1))==0, "Cache size must be a power of 2 bigger than 1");
private:
  Array<T> Data;
  U32 Index;
public:
  explicit Cache() : Data(Size) { Index=0; }
  T& operator()(U32 i) {
    return Data[(Index-i)&(Size-1)];
  }
  void operator++(int) {
    Index++;
  }
  void operator--(int) {
    Index--;
  }
  T& Next() {
    return Index++, *((T*)memset(&Data[Index&(Size-1)], 0, sizeof(T)));
  }
};
  



class TextModel: public Model {
private:
  const U32 MIN_RECOGNIZED_WORDS = 4;
  U32 N;
   Buf& buffer;
  ContextMap2 Map;
  Array<Stemmer*> Stemmers;
  Array<Language*> Languages;
  Cache<Word, 8> Words[Language::Count];
  Cache<Segment1, 4> Segments;
  Cache<Sentence, 4> Sentences;
  Cache<Paragraph, 2> Paragraphs;
  Array<U32> WordPos;
  U32 BytePos[256];
  Word xWord;
  Word *cWord, *pWord; // current word, previous word
  Segment1 *cSegment; // current segment
  Sentence *cSentence; // current sentence
  Paragraph *cParagraph; // current paragraph  
  
  enum Parse {
    Unknown,
    ReadingWord,
    PossibleHyphenation,
    WasAbbreviation,
    AfterComma,
    AfterQuote,
    AfterAbbreviation,
    ExpectDigit
  } State, pState;
  struct {
    U32 Count[Language::Count-1]; // number of recognized words of each language in the last 64 words seen
    U64 Mask[Language::Count-1];  // binary mask with the recognition status of the last 64 words for each language
    int Id;  // current language detected
    int pId; // detected language of the previous word
  } Lang;
  struct {
    U64 numbers[6];   // last 2 numbers seen
    U32 numHashes[6]; // hashes of the last 2 numbers seen
    U8  numLength[6]; // digit length of last 2 numbers seen
    U32 numMask;      // binary mask of the results of the arithmetic comparisons between the numbers seen
    U32 numDiff;      // log2 of the consecutive differences between the last 16 numbers seen, clipped to 2 bits per difference
    U32 lastUpper;    // distance to last uppercase letter
    U32 maskUpper;    // binary mask of uppercase letters seen (in the last 32 bytes)
    U32 lastLetter;   // distance to last letter
    U32 lastDigit;    // distance to last digit
    U32 lastPunct;    // distance to last punctuation character
    U32 lastNewLine;  // distance to last new line character
    U32 prevNewLine;  // distance to penultimate new line character
    U32 wordGap;      // distance between the last words
    U32 spaces;       // binary mask of whitespace characters seen (in the last 32 bytes)
    U32 spaceCount;   // count of whitespace characters seen (in the last 32 bytes)
    U32 lastSpace;
    U32 commas;       // number of commas seen in this line (not this segment/sentence)
    U32 quoteLength;  // length (in words) of current quote
    U32 maskPunct;    // mask of relative position of last comma related to other punctuation
    U32 nestHash;     // hash representing current nesting state
    U32 lastNest;     // distance to last nesting character
    U32 linespace;
    U8  islink;
    U8 istemplate;
    U8 isqoute;
    U32 nl;
    U32 nl1;
    U32 nl2;
    U32 masks[5],
        wordLength[4];
    int UTF8Remaining;// remaining bytes for current UTF8-encoded Unicode code point (-1 if invalid byte found)
    U8 firstLetter;   // first letter of current word
    U8 firstChar;     // first character of current line
    U8 expectedDigit; // next expected digit of detected numerical sequence
        U8 prevPunct;     // most recent punctuation character seen
    Word TopicDescriptor; // last word before ':'
    Word WikiHead0;
    Word WikiHead1;
    Word WikiHead2;
    Word WikiHead3;
     Word WikiHead4;   
  } Info;
  U32 ParseCtx;
   bool doXML;
   bool firstwasspace;
  void Update(Buf& buffer,Mixer& mixer);
  void SetContexts(Buf& buffer,Mixer& mixer);
public:
  TextModel(BlockData& bd, U64 Size);
  virtual ~TextModel() {
    for (int i=0; i<Language::Count-1; i++) {
      delete Stemmers[i];
      delete Languages[i];
    }
  }
  void setword(U8 *w,int len);
  int inputs()   {return N*Map.inputs();}
  int nets()     {return  1024+2048+4096+ 4096+2048+ 2048+ 4096+ 8192;}
  int netcount() {return 8;}
  int p(Mixer& mixer,int val1=0, int val2=0);
  //void Update(Buf& buffer,Mixer& mixer);
  //void SetContexts(Buf& buffer,Mixer& mixer) ;
};



