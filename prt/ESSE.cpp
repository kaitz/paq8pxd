#include "ESSE.hpp"
// Extra SSE
//#include "mod_sse.cpp"

eSSE::eSSE(BlockData& bd):x(bd) {
    sse_=new M_T1();
    sse_->M_Init();
}

int eSSE::p(int input){
  int pr = (4096-input)*(32768/4096); //1 + (1 - input) * 32766; // 
  if( pr<1 ) pr=1;
  if( pr>32767 ) pr=32767;
  pr = sse_->M_Estimate(pr);
 /// pr = (32768-pr)/(32768/4096);
  if( pr<1 ) pr=1;
  if( pr>32767 ) pr=32767;
  return pr;
}
void eSSE::update(){
   sse_->M_Update( x.y);
}
