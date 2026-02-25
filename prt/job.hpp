#pragma once
#include "types.hpp"
#include "file.hpp"
#include "../stream/streams.hpp"
#include "segment.hpp"
#include "settings.hpp"
#include <vector>

#ifdef MT

#ifdef WINDOWS
#include <windows.h>
#endif
#ifdef PTHREAD
#include "pthread.h"
#endif

#ifdef PTHREAD
typedef pthread_t pthread_tx;
#else
typedef HANDLE pthread_tx;
#endif

typedef enum {READY, RUNNING, FINISHED_ERR, FINISHED, ERR, OK} State;
// Instructions to thread to compress or decompress one block.
struct Job {
  State state;        // job state, protected by mutex
  int id;             
  int streamid;
  U64 datasegmentsize;
  int command;
  File*in;
  File*out;
  pthread_tx tid;      // thread ID (for scheduler)
  Streams *st;
  Segment *sg;
  Settings *set;
  Job();
  void print(int i) const;
};

extern void decompress(const Job& job);
extern void compress(const Job& job);

#define check(f) { \
  int rc=f; \
  if (rc) fprintf(stderr, "Line %d: %s: error %d\n", __LINE__, #f, rc); \
}

#endif

void run_jobs(std::vector<Job>& jobs, int threads);
