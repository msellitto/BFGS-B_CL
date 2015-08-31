#ifndef PARALLEL_EVAL_H
#define PARALLEL_EVAL_H


#include <CL/cl.h>

#include "bfgsb_cl.h"


#define MAX_STR_SZ 512		// maximum string size

typedef struct s_pEval_user_buff {
   cl_mem data_dev;						// if user needs a buffer  
   bfgsb_cl_user_data_arg arg;
} pEval_user_buff;



class pEval {
  
      public:


    pEval(
      int num_vars,							// number of variables in each function
      int num_funcs,						// number of functions in evaluation
      const char *evalSrcFileNameFull,		// name of the source code that contains the evaluation kernel
      const char *OpenCL_incDir,			// directory to search for OpenCL "include" files. Use "" if none.
      int num_user_args,					// number of additional user arguments to evaluation kernel
      bfgsb_cl_user_data_arg *user_args,	// user arg structs
      bool use_coarse_grain_search,			// set to true if also using coarse-grain search
      unsigned int coarse_grain_n,			// number of points used in coarse-grain search
      double *coarse_grain_points			// array of size num_vars*coarse_grain_n points for coarse-grain search
      );

    ~pEval();

	// init OpenCL subsystem
    void OpenCL_setup();

	// execute parallel evaluation on OpenCL device
    void eval();
    
	void coarse_grain_search(double *init_ret);

	// functions to return F, x, gradient, and active mask
    double *getF() { return F_host; }
    double *getx() { return x_host; }
    double *getg() { return g_host; }
    int *getActive() {return active_mask_host; }

   private:

    int num_vars;
    int num_funcs;
    char evalSrcFileNameFull[MAX_STR_SZ];
    char OpenCL_incDir[MAX_STR_SZ];
    int num_user_args;
    pEval_user_buff *user_buffs;
    bool use_coarse_grain_search;
    unsigned int coarse_grain_n;
    double *coarse_grain_points;

	// OpenCL data structures
    cl_context context;
    cl_command_queue cmdQueue;
    cl_kernel evalKernel;
    cl_kernel coarseGrainedSearchKernel;

	// device memory handles
    cl_mem F_dev;
    cl_mem x_dev;
    cl_mem g_dev;
    cl_mem active_mask_dev;

    cl_mem coarse_grain_points_dev;
    cl_mem init_ret_dev;

	// OpenCL subsystem functions
    void OpenCL_mainSetup();
    void OpenCL_initInputs();
    void OpenCL_cleanup();

	// host memory pointers
    double *F_host;
    double *x_host;
    double *g_host;
    int *active_mask_host;
};

#endif
