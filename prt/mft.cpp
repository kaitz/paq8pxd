
#include "mft.hpp"


MTFList::MTFList(const U16 n): Root(0), Index(0), Previous(n), Next(n) {
    assert(n>0);
    for (int i=0;i<n;i++) {
      Previous[i] = i-1;
      Next[i] = i+1;
    }
    Next[n-1] = -1;
  }
  int MTFList::GetFirst(){
    return Index=Root;
  }
  int MTFList::GetNext(){
    if(Index>=0){Index=Next[Index];return Index;}
    return Index; //-1
  }
  void MTFList::MoveToFront(int i){
    assert(i>=0 && i<Previous.size());
    if ((Index=i)==Root) return;
    int p=Previous[Index];
    int n=Next[Index];
    if(p>=0)Next[p] = Next[Index];
    if(n>=0)Previous[n] = Previous[Index];
    Previous[Root] = Index;
    Next[Index] = Root;
    Root=Index;
    Previous[Root]=-1;
  }

