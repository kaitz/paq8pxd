#include "lstm.hpp"
// LSTM
//#include "lstm1.inc"

  lstmModel1::lstmModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf),
  apm1{0x10000u, 24}, apm2{0x800u, 24}, apm3{ 1024, 24 },
  horizon(20),
  iCtx{ 11, 1, 9 }  { 
  srand(0xDEADBEEF);
  lstm=new LSTM::ByteModel(25, 3, horizon, 0.05);// num_cells, num_layers, horizon, learning_rate)
 }

 int lstmModel1::p(Mixer& m,int val1,int val2){
    lstm->Perceive(x.y);
    int p=lstm->Predict();
     iCtx += x.y;
     iCtx = (x.bpos << 8) | lstm->expected();
    std::uint32_t ctx =  iCtx();
    m.add(stretch(p));
    m.add((p-2048)>>2);
    const int pr1 = apm1.p(p, (x.c0<<8) | (x.Misses & 0xFF), x.y,0xFF);
    const int pr2 = apm2.p(p, (x.bpos<<8) |lstm->expected(), x.y,0xFF);
    const int pr3 = apm3.p(pr2, ctx, x.y, 0xFF);
    m.add(stretch(pr1)>>1);
    m.add(stretch(pr2)>>1);
    m.add(stretch(pr3)>>1);
    m.set((x.bpos<<8) | lstm->expected(), 8 * 256);
    m.set(lstm->epoch() << 3 | x.bpos, (horizon<<3)+7+1);
  return 0;
}
  
