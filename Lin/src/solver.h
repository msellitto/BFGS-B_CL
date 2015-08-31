#ifndef SOLVER_H
#define SOLVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This defines the possible results of running the L-BFGS-B solver.
enum SolverExitStatus { 
  success,              // The algorithm has converged to a stationary
			            // point or has reached the maximum number of
			            // iterations.

  evalWait,				// wait for another evaluation of F and g
  done,					// succesfully converged or hit max iterations (same as success)

  
  
  abnormalTermination,  // The algorithm has terminated abnormally and
						// was unable to satisfy the convergence
						// criteria.

  errorOnInput          // The routine has detected an error in the
			            // input parameters.
      
    
};

// Default L-BFGS-B Solver Parameters
// -----------------------------------------------------------------
const int    defaultm          = 5;
const int    defaultmaxiter    = 100;
const double defaultfactr      = 1e7;
const double defaultpgtol      = 1e-5;
const int    defaultprintlevel = -1;


// Function declarations.
// -----------------------------------------------------------------


// gfortran and the Intel Visual Fortran compiler use different
// fortran function names when called from C
#ifdef _WIN32
//Windows

// This is the L-BFGS-B routine implemented in Fortran 77.
extern "C" void SETULB (int* n, int* m, double x[], double l[], 
			 double u[], int nbd[], double* f, double g[], 
			 double* factr, double* pgtol, double wa[], 
			 int iwa[], char task[], int* iprint, 
			 char csave[], bool lsave[], int isave[], 
			 double dsave[]);


#else
//Linux

// This is the L-BFGS-B routine implemented in Fortran 77.
extern "C" void setulb_ (int* n, int* m, double x[], double l[], 
			 double u[], int nbd[], double* f, double g[], 
			 double* factr, double* pgtol, double wa[], 
			 int iwa[], char task[], int* iprint, 
			 char csave[], bool lsave[], int isave[], 
			 double dsave[]);


#endif





// Class SolverBase.
// -----------------------------------------------------------------
// This class encapsulates execution of the L-BFGS-B routine for
// solving a nonlinear optimization problem with bound constraints
// using limited-memory approximations to the Hessian.
//
// This is an abstract class since some of the class methods have not
// been implemented (they are "pure virtual" functions). In order to
// use this class, one needs to define a child class and provide
// definitions for the pure virtual methods.
class SolverBase {
public:

  // The first input argument "n" is the number of variables of the function
  // x_init is a size n array of the initial x values
  // This the suggested starting point for the optimization
  // algorithm. See the L-BFGS-B documentation for more information on
  // the inputs to the constructor.
  // lb and ub are the upper and lower bound constraints
  // bytype is the bound types
  // The solver output is stored in x_ret and f_ret
  SolverBase (int n, double *x_init, double* lb, double* ub, int* btype,
     double *x_ret, double *f_ret, double *g = NULL,
	   int m = defaultm, int maxiter = defaultmaxiter,
	   double factr = defaultfactr, double pgtol = defaultpgtol);

  // The destructor.
  virtual ~SolverBase();

  // Run the solver.
  virtual SolverExitStatus runSolver() = 0;

protected:

  // The copy constructor and copy assignment operator are kept
  // private so that they are not used.
  SolverBase            (const SolverBase& source) { };
  SolverBase& operator= (const SolverBase& source) { return *this; };

  int     n;			// The number of variables.
  double *f;			// The current value of the objective.
  double *x;			// The current x value
  double *g;			// The current value of the gradient.
  double *lb;			// The lower bounds.
  double *ub;			// The upper bounds.
  int    *btype;		// The bound types.

  int     iter;			// current iteration number
  bool    g_owner;		// If true, then the block of memory pointed to by
						// g will be managed by the solver

  int     iprint;		// The print level of the solver.
  int     maxiter;		// The maximum number of iterations.
  double  pgtol;		// Convergence parameter passed to L-BFGS-B.
  double  factr;		// Convergence parameter passed to L-BFGS-B.
  int     m;			// The number of variable corrections to the 
						// limited-memory approximation to the Hessian.

  // Execute a single step the L-BFGS-B solver routine.
  void callLBFGS (const char* cmd = 0);

  // These are structures used by the L-BFGS-B solver routine.
  double* wa;
  int*    iwa;
  char    task[60];
  char    csave[60];
  bool    lsave[4];
  int     isave[44];
  double  dsave[29];
};


// Solver that runs completely on the CPU that solves 
// the n dimensional objective function obj_func
class Solver : public SolverBase {

   public:

  Solver (int n,										// number of variables of the objective function
        double (*obj_func)(double *, void *),			// function pointer to the objective function
        unsigned int aux_data_size,						// size in bytes of auxilary data to pass to function
        void *aux_func_data,							// pointer to a block of memory of size bytes to pass to obj_func
        double *x_init,									// array of size n of initial values for x
        double *lb,										// array of size n of lower bounds for x
        double *ub,										// array of size n of upper bounds for x
        int *btype,										// array of size n of bound types for x
        double *x_ret,									// output: final x value upon solver run completion
        double *f_ret,									// output final f(x) value upon solver run completion
        int m,											// hessian approx factor
        int maxiter)									// maximum iterations to run solver
     : SolverBase(n, x_init, lb, ub, btype, x_ret, f_ret, NULL, m, maxiter)
  {

      x_tmp1 =  (double *) malloc(n*sizeof(double));
      x_tmp2 =  (double *) malloc(n*sizeof(double));  

      this->obj_func = obj_func;
      this->aux_data_size = aux_data_size;

      if(aux_data_size > 0)
      {
         this->aux_func_data = (void *) malloc(aux_data_size);
         memcpy(this->aux_func_data, aux_func_data, aux_data_size);
      }

  }


   ~Solver()
   {
      if(aux_data_size > 0) free(aux_func_data);
      free(x_tmp1);
      free(x_tmp2);
   }

   // run solver to completion on the CPU
   SolverExitStatus runSolver();

   protected:

  // Returns the value of the objective function
  // at point x
  double computeObjective(double *x);

  // Computes the gradient of the objective function at point x
  void computeGradient(double *x);
 
  double (*obj_func)(double *, void *);
  unsigned int aux_data_size;
  void * aux_func_data;
  double *x_tmp1;
  double *x_tmp2;
}; 

// This solver class relies on a routine external to this class to
// evaluate f(x) and the gradient(f(x)) 
class SolverExtEval : public SolverBase {

   public:

  SolverExtEval (int n,							// number of variables of the objective function
        double *x_init,							// array of size n of initial values for x
        double *lb,								// array of size n of lower bounds for 
        double *ub,								// array of size n of upper bounds for x
        int *btype,								// array of size n of bound types for x
        double *x_ret,							// output: final x value upon solver run completion
        double *f_ret,							// output final f(x) value upon solver run completion
        double *g,								// array of size n to store the gradient of f(x)
        int m,									// hessian approx factor
        int maxiter,							// maximum iterations to run solver
        int id = 0)								// solver identifier
     : SolverBase(n, x_init, lb, ub, btype, x_ret, f_ret, g, m, maxiter)
  {

      solverInit = false;
      solverDone = false;
      this->id = id;
  }

   ~SolverExtEval()
   {
   }

  // executes a single step of the solver
  SolverExitStatus runSolver();

  bool finished() { return solverDone; }   // returns true if solver is finished, false otherwise
  int getId() { return id; }			   // returns solver identifier

   protected:
  bool solverInit;							// true if solver has been initialized
  bool solverDone;							// true if solver is done
  int id;									// solver identifier
};



#endif
