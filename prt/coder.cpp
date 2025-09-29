#include "coder.hpp"
extern U8 level;

  // Compress bit y or return decompressed bit
  void Encoder::code(int i) {
    int p=predictor.p();
    rc.rc_BProcess( p, i );
    predictor.x.y=i;
    predictor.update0();
    predictor.update();
  }
  int Encoder::decode() {
    int p=predictor.p();
    predictor.x.y = rc.rc_BProcess( p, 0 );
    predictor.update0();
    predictor.update();
    return predictor.x.y;
  }
 
  // Compress one byte
  void Encoder::compress(int c) {
    assert(mode==COMPRESS);
    if (level==0)
      archive->putc(c);
    else {
      for (int i=7; i>=0; --i)
        code((c>>i)&1);
    }
  }

  // Decompress and return one byte
  int Encoder::decompress() {
    if (mode==COMPRESS) {
      assert(alt);
      return alt->getc();
    }
    else if (level==0){
     int a;
     a=archive->getc();
      return a ;}
    else {
      int c=0;
      for (int i=0; i<8; ++i)
        c+=c+decode();
      
      return c;
    }
  }
  


Encoder::Encoder(Mode m, File* f,Predictors& predict):
    mode(m), archive(f), alt(0),predictor(predict) {
    if(level>0) if(mode==DECOMPRESS) rc.StartDecode(f); else rc.StartEncode(f);

}

void Encoder::flush() {
  if (mode==COMPRESS && level>0)rc.FinishEncode();
}
