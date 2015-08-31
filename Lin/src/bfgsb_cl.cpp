#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// for timing functions
#ifdef _WIN32
//Windows
#include <Winsock2.h>
#include "gettimeofday.h"

#else
//Linux
#include <sys/time.h>
#include <unistd.h>
#endif

#include <pthread.h>

#include <iostream>
#include <fstream>
#include <list>
#include <queue>
using namespace std;

#include "bfgsb_cl.h"
#include "time_util.h"
#include "solver.h"
#include "parallel_eval.h"

// CPU work-thread status
typedef enum e_workerStatus
{
   busy,
   idle
} workerStatus;

// solver status
typedef enum e_bfgsbCLStatus
{
   running,
   finished
} bfgsbCLStatus;

// Thread structures

static void *WorkThread4(void *threadid);  // cpu work thread

static workerStatus *workThreadStatus;  
static bfgsbCLStatus *solverStatus; 
static pthread_mutex_t *workThreadStatusMutex;
static pthread_mutex_t *solverStatusMutex;

static pthread_mutex_t *queueMutex;
static queue<SolverExtEval *> *solverWorkQueue;


// This is the main BFGS-B CL Solver function.
// It solves a num_funcs sized array of non-linear bound constrained optimization problems of num_vars variables
// using multi-threaded CPU code + OpenCL on a GPU.
// It returns the result in x_ret and f_ret.
void bfgsb_cl(
      int num_vars,
      double *x_init,
      int *b,
      double *L,
      double *U,
      int num_funcs,
      const char *evalSrcFileNameFull, 
      const char *OpenCL_incDir,
      int num_user_args,
      bfgsb_cl_user_data_arg *user_args,
      int max_iterations,
      int hessian_approx_factor,
      int num_cpu_work_threads,
      double *x_ret,
      double *f_ret,
      bool use_coarse_grain_search,
      unsigned int coarse_grain_n,
      double *coarse_grain_points,
	  bool verbosePrint)
{

   struct timeval start, end;
   gettimeofday(&start, NULL); 

   // initialize parallel evaluation module
   pEval pe(
         num_vars,
         num_funcs,
         evalSrcFileNameFull,
         OpenCL_incDir,
         num_user_args,
         user_args,
         use_coarse_grain_search,
         coarse_grain_n,
         coarse_grain_points);
         
   // init thread structures
   long t;
   pthread_t *threads = (pthread_t *) malloc(num_cpu_work_threads * sizeof(pthread_t));
   workThreadStatus = (workerStatus *) malloc(num_cpu_work_threads * sizeof(workerStatus));
   solverStatus = (bfgsbCLStatus *) malloc(num_cpu_work_threads * sizeof(solverStatus));
   workThreadStatusMutex = (pthread_mutex_t *) malloc(num_cpu_work_threads * sizeof(pthread_mutex_t));
   solverStatusMutex = (pthread_mutex_t *) malloc(num_cpu_work_threads * sizeof(pthread_mutex_t));
   queueMutex = (pthread_mutex_t *) malloc(num_cpu_work_threads * sizeof(pthread_mutex_t));
   solverWorkQueue = new queue<SolverExtEval *>[num_cpu_work_threads];

   for(t = 0; t < num_cpu_work_threads; t++)
   {
      solverStatus[t] = running;
      workThreadStatus[t] = idle;
      pthread_mutex_init(&queueMutex[t], NULL);
      pthread_mutex_init(&workThreadStatusMutex[t], NULL);
      pthread_mutex_init(&solverStatusMutex[t], NULL);
   }


   // initialize OpenCL and required buffers
   pe.OpenCL_setup();

   // solver working list
   list<SolverExtEval *> solverWorkList;

   // master solver array
   SolverExtEval **masterSolverArray;
   masterSolverArray = (SolverExtEval **) malloc(num_funcs * sizeof(SolverExtEval *));   
  
   // get host memory pointers from PE module
   double *x = pe.getx();
   double *f = pe.getF();
   double *g = pe.getg();
   int *active = pe.getActive();

   // will serve as initializers for x to pass to solver drivers
   double *x_inits = (double *) malloc(num_vars * num_funcs * sizeof(double));

   // if not using coarse grain search simply use arg x_init for all initial
   // values
   if(!use_coarse_grain_search)
   {
      for(int i = 0; i < num_funcs; i++)
      {
         memcpy(x_inits+(i * num_vars), x_init, num_vars * sizeof(double));
      }
   }

   // use coarse grain search
   else
   {
      pe.coarse_grain_search(x_inits);
   }

   // initialize solver drivers
   for(int id = 0; id < num_funcs; id++)
   {
      masterSolverArray[id] = new SolverExtEval(num_vars, x_inits+(id * num_vars), L, U, b, x, f, g, hessian_approx_factor, max_iterations, id);
      f++;
      g = g+num_vars;
      x = x+num_vars;
   }

   // initialize work list
   solverWorkList.assign(masterSolverArray, masterSolverArray+num_funcs);

  // launch CPU work threads
   for(t = 0; t < num_cpu_work_threads; t++)
   {
      pthread_create(&threads[t], NULL, WorkThread4, (void *)t);
   }


   // start main bfgs_cl solver loop

   long wt = 0; // work thread
   int r = 0;
   while(1)
   {

      if(verbosePrint) printf("iter %d\n", r++);
      wt = 0;

      // iterate over solver list
      for(list<SolverExtEval *>::iterator it = solverWorkList.begin(); it != solverWorkList.end();)
      {
         SolverExtEval *s = *it;

         list<SolverExtEval *>::iterator tmp_it = it; 
         it++;

         // if solver is finished, remove from work list, set PE active flag to
         // inactive
         if(s->finished())
         {
            int id = s->getId();
            active[id] = 0;
            solverWorkList.erase(tmp_it);
         }

         // else push on to a work queue
         else
         {
            pthread_mutex_lock(&queueMutex[wt]);
            solverWorkQueue[wt].push(s);
            pthread_mutex_unlock(&queueMutex[wt]);

            wt++;
            if(wt == num_cpu_work_threads) wt = 0;
         }

      }

      bool wait = true;

      // wait for all work threads to be done with their work for this
      // iteration
      while(1)
      {
         // make sure all work queues are empty and all work threads are idle
         for(t = 0; t < num_cpu_work_threads; t++)
         {
            pthread_mutex_lock(&workThreadStatusMutex[t]);
            workerStatus status = workThreadStatus[t];
            pthread_mutex_unlock(&workThreadStatusMutex[t]);

            if(status == busy) break;

            pthread_mutex_lock(&queueMutex[t]);
            bool empty = solverWorkQueue[t].empty();
            pthread_mutex_unlock(&queueMutex[t]);

            if(!empty) break;

            if(t == (num_cpu_work_threads - 1)) wait = false; // all threads are finished with work

         }

         if(wait == false) break; // work is all done for this iteration, break out

      }

      // GPU parallel evaluation
      if(verbosePrint) printf("GPU calc\n");
      pe.eval(); 
      if(verbosePrint) printf("Done GPU calc\n");

      // if solver work list is empty we are done
      if(solverWorkList.empty()) 
      {
 
         // signal to all worker threads that we are done
         for(t = 0; t < num_cpu_work_threads; t++)
         {
            pthread_mutex_lock(&solverStatusMutex[t]);
            solverStatus[t] = finished;
            pthread_mutex_unlock(&solverStatusMutex[t]);
         }

         break; // exit bfgs_cl solver loop 
      }


   }

   // join CPU work threads
   for(t = 0; t < num_cpu_work_threads; t++)
   {
       pthread_join(threads[t], NULL);
   }

   // copy data back to output parameters (passed from calling function)
   x = pe.getx();
   f = pe.getF();

   memcpy(x_ret, x, num_vars * num_funcs * sizeof(double));
   memcpy(f_ret, f, num_funcs * sizeof(double));

   // cleanup

   free(x_inits);

   free(threads);
   free(workThreadStatus);
   free(solverStatus);
   free(workThreadStatusMutex);
   free(solverStatusMutex);
   free(queueMutex);
   delete[] solverWorkQueue;

   // free masterSolverArray
   for(int id = 0; id < num_funcs; id++)
   {
      delete masterSolverArray[id];
   }

   free(masterSolverArray);

	
   gettimeofday(&end, NULL); 
   printf("SOLVER EXECUTION TIME: %f (ms)\n", calc_time(&start, &end));
}


// worker thread for bfgs_cl solver
static void *WorkThread4(void *threadid)
{

   long tid = (long) threadid;

//   These are experimental pthread options to set thread affinity, they are not really needed. 
//   And they are not supported under pthread-w32. Turning off to for now.
//   cpu_set_t cpuset;
//   CPU_ZERO(&cpuset);
//   CPU_SET(tid, &cpuset);
//   pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

   printf("work thread %d started\n", tid);

   while(1)
   {
      bool qEmpty;

      // check if work queue is empty
      pthread_mutex_lock(&queueMutex[tid]);
      qEmpty = solverWorkQueue[tid].empty();

      // if queue is empty
      if(qEmpty) 
      {
         pthread_mutex_unlock(&queueMutex[tid]);

         // check signal to shut down worker
         pthread_mutex_lock(&solverStatusMutex[tid]);
         bfgsbCLStatus status = solverStatus[tid];
         pthread_mutex_unlock(&solverStatusMutex[tid]);

         // if we are done, shut down worker thread
         if(status == finished) break;
      }

      else 
      {
         // set worker state to being busy
         pthread_mutex_lock(&workThreadStatusMutex[tid]);
         workThreadStatus[tid] = busy;
         pthread_mutex_unlock(&workThreadStatusMutex[tid]);

         // get work item and do work
         SolverExtEval*&  s = solverWorkQueue[tid].front();
         solverWorkQueue[tid].pop();
         SolverExtEval *s_tmp = s;
         pthread_mutex_unlock(&queueMutex[tid]);

         s_tmp->runSolver(); // do work

         // set worker state to being idle
         pthread_mutex_lock(&workThreadStatusMutex[tid]);
         workThreadStatus[tid] = idle;
         pthread_mutex_unlock(&workThreadStatusMutex[tid]);
      }

   }


  printf("work thread %d ending\n", tid);
  pthread_exit(NULL);

  return NULL;
}

