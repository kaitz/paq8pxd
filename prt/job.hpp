#pragma once
#include "types.hpp"
#include "file.hpp"
#include <vector>

#ifdef MT

#ifdef WINDOWS
#include <windows.h>
#endif
#ifdef PTHREAD
#include "pthread.h"
#endif

//multithreading code from pzpaq.cpp v0.05
#ifdef PTHREAD
static pthread_cond_t cv=PTHREAD_COND_INITIALIZER;  // to signal FINISHED
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; // protects cv
typedef pthread_t pthread_tx;
#else
static HANDLE mutex;  // protects Job::state
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
  Job();
  void print(int i) const;
};

extern void decompress(const Job& job);
extern void compressStream(int streamid,U64 size, File* in, File* out);

#define check(f) { \
  int rc=f; \
  if (rc) fprintf(stderr, "Line %d: %s: error %d\n", __LINE__, #f, rc); \
}
// Worker thread
#ifndef PTHREAD
DWORD thread(void *arg);
#endif

// Worker thread
#ifdef PTHREAD
void *thread(void *arg);
#endif

static std::vector<Job> jobs;
static int topt=1;
#endif

