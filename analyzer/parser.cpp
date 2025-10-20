#include "parser.hpp"

Parser::Parser():state(NONE),priority(0),name("default"),pinfo("") {
}

Parser::~Parser() {
}

std::string Parser::itos(int64_t x, int n) {
  assert(x>=0);
  assert(n>=0);
  std::string r;
  for (; x || n>0; x/=10, --n) r=std::string(1, '0'+x%10)+r;
  return r;
}
