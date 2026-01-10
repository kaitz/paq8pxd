#pragma once
#include "helper.hpp"
#include "types.hpp"
#include <assert.h>
#include <stdio.h>
#include <cinttypes>
#include <cstring>
#include <cstdlib>
#ifndef NDEBUG
 void chkindex(U64 index, U64 upper_bound);
#endif
//////////////////////////// Array ////////////////////////////

// Array<T,Align> a(n); allocates memory for n elements of T.
// The base address is aligned if the "alignment" parameter is given.
// Constructors for T are not called, the allocated memory is initialized to 0s.
// It's the caller's responsibility to populate the array with elements.
// Parameters are checked and indexing is bounds checked if assertions are on.
// Use of copy and assignment constructors are not supported.
//
// a.size(): returns the number of T elements currently in the array.
// a.resize(newsize): grows or shrinks the array.
// a.clear(): release all the memory but preserve the memory pointer.
// a.append(x): appends x to the end of the array and reserving space for more elements if needed.
// a.pop_back(): removes the last element by reducing the size by one (but does not free memory).

template <class T, const int Align=16> class Array {
private:
  U64 used_size;
  U64 reserved_size;
  char *ptr; // Address of allocated memory (may not be aligned)
  T* data;   // Aligned base address of the elements, (ptr <= T)
  void create(U64 requested_size);
  inline U64 padding() const {return Align-1;}
  inline U64 allocated_bytes() const {return (reserved_size==0)?0:reserved_size*sizeof(T)+padding();}
public:
  explicit Array(U64 requested_size) {create(requested_size);}
  ~Array();
  T& operator[](U64 i) {
    #ifndef NDEBUG
    chkindex(i,used_size);
    #endif
    return data[i];
  }
  const T& operator[](U64 i) const {
    #ifndef NDEBUG
    chkindex(i,used_size);
    #endif
    return data[i];
  }
  U64 size() const {return used_size;}
  void resize(U64 new_size);
  void clear();                                        // dealloc memory, preserve pointer
  void pop_back() {assert(used_size>0); --used_size; }  // decrement size
  void push_back(const T& x);  // increment size, append x
  Array(const Array&) { assert(false); } //prevent copying - this method must be public (gcc must see it but actually won't use it)
private:
  Array& operator=(const Array&); //prevent assignment
};

template<class T, const int Align> void Array<T,Align>::create(U64 requested_size) {
  assert((Align&(Align-1))==0);
  used_size=reserved_size=requested_size;
  if (requested_size==0) {
    data=0;ptr=0;
    return;
  }
  U64 bytes_to_allocate=allocated_bytes();
  ptr=(char*)calloc(bytes_to_allocate,1);
  if(!ptr){
      printf("Requested size %0" PRIi64 " MB\n",((bytes_to_allocate)/1024)/1024);
      throw "Out of memory."; 
  }
  U64 pad=padding();
  data=(T*)(((uintptr_t)ptr+pad) & ~(uintptr_t)pad);
  assert(ptr<=(char*)data && (char*)data<=ptr+Align);
  assert(((uintptr_t)data & (Align-1))==0); //aligned as expected?
}

template<class T, const int Align> void Array<T,Align>::resize(U64 new_size) {
  if (new_size<=reserved_size) {
    used_size=new_size;
    return;
  }
  char *old_ptr=ptr;
  T *old_data=data;
  U64 old_size=used_size;
  create(new_size);
  if(old_size>0) {
    assert(old_ptr && old_data);
    memcpy(data, old_data, sizeof(T)*old_size);
  }
  if(old_ptr){free(old_ptr);old_ptr=0;}
}

template<class T, const int Align> void Array<T,Align>::clear() {
  if (used_size==0) return;
  char *old_ptr=ptr;
  T *old_data=data;
  U64 old_size=used_size;
  create(0);
  if(old_ptr){free(old_ptr);old_ptr=0;}
}

template<class T, const int Align> void Array<T,Align>::push_back(const T& x) {
  if(used_size==reserved_size) {
    U64 old_size=used_size;
    U64 new_size=used_size*2+16;
    resize(new_size);
    used_size=old_size;
  }
  data[used_size++]=x;
}

template<class T, const int Align> Array<T, Align>::~Array() {
  //programChecker.free(allocated_bytes());
  free(ptr);
  used_size=reserved_size=0;
  data=0;ptr=0;
}
