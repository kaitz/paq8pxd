#include "job.hpp"

#ifdef MT
//multithreading code from pzpaq.cpp v0.05
#ifdef PTHREAD
pthread_cond_t cv=PTHREAD_COND_INITIALIZER;  // to signal FINISHED
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; // protects cv
//typedef pthread_t pthread_tx;
#else
HANDLE mutex;  // protects Job::state
//typedef HANDLE pthread_tx;
#endif
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
      compress(*job);
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
      compress(*job);
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
void run_jobs(std::vector<Job>& jobs, int threads) {

    // Loop until all jobs return OK or ERR: start a job whenever one
    // is eligible. If none is eligible then wait for one to finish and
    // try again. If none are eligible and none are running then it is
    // an error.
    int thread_count=0;  // number RUNNING, not to exceed threads
    U32 job_count=0;     // number of jobs with state OK or ERR

    // Aquire lock on jobs[i].state.
    // Threads can access only while waiting on a FINISHED signal.
#ifdef PTHREAD
    pthread_attr_t attr; // thread joinable attribute
    check(pthread_attr_init(&attr));
    check(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
    check(pthread_mutex_lock(&mutex));  // locked
#else
    mutex=CreateMutex(NULL, FALSE, NULL);  // not locked
#endif

    while(job_count<jobs.size()) {

        // If there is more than 1 thread then run the biggest jobs first
        // that satisfies the memory bound. If 1 then take the next ready job
        // that satisfies the bound. If no threads are running, then ignore
        // the memory bound.
        int bi=-1;  // find a job to start
        if (thread_count<threads) {
            for (U32 i=0; i<jobs.size(); ++i) {
                if (jobs[i].state==READY  && bi<0 ) {
                    bi=i;
                    if (threads==1) break;
                }
            }
        }

        // If found then run it
        if (bi>=0) {
            jobs[bi].state=RUNNING;
            ++thread_count;
#ifdef PTHREAD
            check(pthread_create(&jobs[bi].tid, &attr, thread, &jobs[bi]));
#else
            jobs[bi].tid=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread,
            &jobs[bi], 0, NULL);
#endif
        }

        // If no jobs can start then wait for one to finish
        else {
#ifdef PTHREAD
            check(pthread_cond_wait(&cv, &mutex));  // wait on cv

            // Join any finished threads. Usually that is the one
            // that signaled it, but there may be others.
            for (U32 i=0; i<jobs.size(); ++i) {
                if (jobs[i].state==FINISHED || jobs[i].state==FINISHED_ERR) {
                    void* status=0;
                    check(pthread_join(jobs[i].tid, &status));
                    if (jobs[i].state==FINISHED) jobs[i].state=OK;
                    if (jobs[i].state==FINISHED_ERR) quit("thread"); //exit program on thread error 
                    ++job_count;
                    --thread_count;
                }
            }
#else
            // Make a list of running jobs and wait on one to finish
            HANDLE joblist[MAXIMUM_WAIT_OBJECTS];
            int jobptr[MAXIMUM_WAIT_OBJECTS];
            DWORD njobs=0;
            WaitForSingleObject(mutex, INFINITE);
            for (U32 i=0; i<jobs.size() && njobs<MAXIMUM_WAIT_OBJECTS; ++i) {
                if (jobs[i].state==RUNNING || jobs[i].state==FINISHED
                        || jobs[i].state==FINISHED_ERR) {
                    jobptr[njobs]=i;
                    joblist[njobs++]=jobs[i].tid;
                }
            }
            ReleaseMutex(mutex);
            DWORD id=WaitForMultipleObjects(njobs, joblist, FALSE, INFINITE);
            if (id>=WAIT_OBJECT_0 && id<WAIT_OBJECT_0+njobs) {
                id-=WAIT_OBJECT_0;
                id=jobptr[id];
                if (jobs[id].state==FINISHED) jobs[id].state=OK;
                if (jobs[id].state==FINISHED_ERR) quit("thread"); //exit program on thread error 
                ++job_count;
                --thread_count;
            }
#endif
        }
    }
#ifdef PTHREAD
    check(pthread_mutex_unlock(&mutex));
#endif

}
