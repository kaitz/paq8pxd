#pragma once
#include "types.hpp"
#include "array.hpp"

class MTFList{
private:
  int Root, Index;
  Array<int, 16> Previous;
  Array<int, 16> Next;
public:
  MTFList(const U16 n);
  int GetFirst();
  int GetNext();
  void MoveToFront(int i);
};
