#pragma once
#include "types.hpp"
#include "array.hpp"

#define TEXT_MIN_SIZE 1024*64   // size of minimum allowed text block (in bytes)
#define TEXT_MAX_MISSES 6    // threshold: max allowed number of invalid UTF8 sequences seen recently before reporting "fail"
#define TEXT_ADAPT_RATE 256  // smaller (like 32) = illegal sequences are allowed to come more often, larger (like 1024) = more rigorous detection

struct TextParserStateInfo {
  Array<U64> _start;
  Array<U64> _end;      // position of last char with a valid UTF8 state: marks the end of the detected TEXT block
  Array<U32> _EOLType;  // 0: none or CR-only;   1: CRLF-only (applicable to EOL transform);   2: mixed or LF-only
  Array<U32> _number;  // 0: none or CR-only;   1: CRLF-only (applicable to EOL transform);   2: mixed or LF-only
  U32 invalidCount;     // adaptive count of invalid UTF8 sequences seen recently
  U32 UTF8State;        // state of utf8 parser; 0: valid;  12: invalid;  any other value: yet uncertain (more bytes must be read)
  U32 countUTF8;
  TextParserStateInfo(): _start(0), _end(0), _EOLType(0),_number(0) {}
  void reset(U64 startpos) {
    _start.resize(1);
    _end.resize(1);
    _start[0]=startpos;
    _end[0]=startpos-1;
    _EOLType.resize(1);
    _number.resize(1);
    _EOLType[0]=0;
    _number[0]=0;
    invalidCount=0;
    UTF8State=0;
    countUTF8=0;
  }
  U64 start(){return _start[_start.size()-1];}
  U64 end(){return _end[_end.size()-1];}
  void setend(U64 end){_end[_end.size()-1]=end;}
  U32 EOLType(){return _EOLType[_EOLType.size()-1];}
  void setEOLType(U32 EOLType){_EOLType[_EOLType.size()-1]=EOLType;}
  U32 number(){return _number[_number.size()-1];}
  void set_number(U32 number){_number[_number.size()-1]=number;}
  U64 validlength(){return end() - start() + 1;}
  void next(U64 startpos) {
    _start.push_back(startpos);
    _end.push_back(startpos-1);
    _EOLType.push_back(0);
    _number.push_back(0);
    invalidCount=0;
    UTF8State=0;
    countUTF8=0;
  }
  void removefirst() {
    if(_start.size()==1)
      reset(0);
    else {
      for(int i=0;i<(int)_start.size()-1;i++){
        _start[i]=_start[i+1];
        _end[i]=_end[i+1];
        _EOLType[i]=_EOLType[i+1];
        _number[i]=_number[i+1];
      }
      _start.pop_back();
      _end.pop_back();
      _EOLType.pop_back();
      _number.pop_back();
    }
  }
} ;
