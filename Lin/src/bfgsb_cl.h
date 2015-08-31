#ifndef BFGSB_CL_H
#define BFGSB_CL_H

// user argument info struct to give to the bfgsb CL solver (one for each user argument)
typedef struct s_bfgsb_cl_user_data_arg {
   bool buffer;                 // should be set true if data should be stored in a buffer on the GPU (and not simply passed by a kernel arg)
   unsigned int size;			// size of data to be passed into kernel (in bytes)
   bool init;                   // should be set true if data should be initialized before being used
   void *data;                  // should be pointer to initial buffer data (if init is set to true)
   bool small_const;			// should be set true if data is small constant data (usually < 64k on most GPUs)
} bfgsb_cl_user_data_arg;


// This is the main BFGS-B CL Solver function.
// It solves a num_funcs sized array of non-linear bound constrained optimization problems of num_vars variables
// using multi-threaded CPU code + OpenCL on a GPU.
// It returns the result in x_ret and f_ret.
void bfgsb_cl(
      int num_vars,							// number of variables in the objective functions 
      double *x_init,						// array of initial values of the objective variables of size num_vars
      int *b,								// array of bound types of the objective variables of size num_vars
      double *L,							// array of lower bounds of the objective variables of size num_vars
      double *U,							// array of upper bounds of the objective variables of size num_vars
      int num_funcs,						// the number of objective functions for the solver to solve
      const char *evalSrcfilenamefull,		// name of the OpenCL file that contains the objective function to be evaluated (and optionally coarse-grained search)
      const char *opencl_incDir,			// directory to find any additional files that are #included in the OpenCL file (use "" string if none)
      int num_user_args,					// number of additional user arguments to pass to evaluation kernel
      bfgsb_cl_user_data_arg *user_args,	// additional user arguments array of size num_user_args
      int max_iterations,					// maximum number of iterations to run the solver on each function
      int hessian_approx_factor,			// hessian approximation factor to use for the solver (
      int num_cpu_work_threads,				// number of CPU work-threads to use (must be at >= 1)
      double *x_ret,						// output: array of size num_vars * num_funcs that returns the solver's solution x  
      double *f_ret,						// output: array of size num_funcs that returns the solver's solution for f(x)
      bool use_coarse_grain_search,			// set to true if using coarse-grained search to determine initial values
      unsigned int coarse_grain_n,			// number of start points to search during coarse-grained search
      double *coarse_grain_points,			// array of starting points to search during coarse grained search, size of coarse_grain_n * num_vars
	  bool verbosePrint);					// turn printing of solver progress on

#endif
