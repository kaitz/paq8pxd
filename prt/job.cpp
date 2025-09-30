#include "job.hpp"

#ifdef MT

// Initialize
Job::Job(): state(READY),id(0),streamid(-1),datasegmentsize(0),command(-1) {
  // tid is not initialized until state==RUNNING
}

// Print contents
void Job::print(int i=0) const {
  fprintf(stderr,
      "Job %d: state=%d stream=%d\n", i, state,streamid);
}

// Worker thread
#ifndef PTHREAD
DWORD thread(void *arg) {
  // Do the work and receive status in msg
  Job* job=(Job*)arg;
  const char* result=0;  // error message unless OK
  try {
    if (job->command==0) {
      compressStream(job->streamid,job->datasegmentsize,job->in,job->out);
      }
    else if (job->command==1)
      decompress(*job); 
  }
  catch (const char* msg) {
    result=msg;
  }
// Call f and check that the return code is 0

  // Let controlling thread know we're done and the result
  WaitForSingleObject(mutex, INFINITE);
  job->state=result?FINISHED_ERR:FINISHED;
  ReleaseMutex(mutex);
  return 0;
}
#endif

// Worker thread
#ifdef PTHREAD
void *thread(void *arg) {
  // Do the work and receive status in msg
  Job* job=(Job*)arg;
  const char* result=0;  // error message unless OK
  try {
    if (job->command==0) {
      compressStream(job->streamid,job->datasegmentsize,job->in,job->out);
      }
    else if (job->command==1)
      decompress(*job); 
  }
  catch (const char* msg) {
    result=msg;
  }
 // Call f and check that the return code is 0

  // Let controlling thread know we're done and the result
  check(pthread_mutex_lock(&mutex));
  job->state=result?FINISHED_ERR:FINISHED;
  check(pthread_cond_signal(&cv));
  check(pthread_mutex_unlock(&mutex));
  return 0;
}
#endif
#endif
