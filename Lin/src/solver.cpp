#include "solver.h"
#include <string.h>
#include <stdio.h>

static void copyCStrToCharArray (const char* source, char* dest, int ndest);
static bool strIsEqualToCStr (const char* str, const char* cstr); 

// initialize solver data structures
SolverBase::SolverBase (int n, double *x_init, double* lb, double* ub, int* btype,
     double *x_ret, double *f_ret, double *g,
	   int m, int maxiter,
	   double factr, double pgtol)
{
   
   this->n = n;

   this->x = x_ret;
   memcpy(this->x, x_init, n*sizeof(double));

   this->lb = new double[n];
   memcpy(this->lb, lb, n*sizeof(double));
  
   this->ub = new double[n];
   memcpy(this->ub, ub, n*sizeof(double));

   this->btype = new int[n];
   memcpy(this->btype, btype, n*sizeof(int));

   this->f = f_ret;
   *(this->f) = 0;

   if(g == NULL)
   {
      this->g = new double[n];
      g_owner = true;
   }
   else
   {
      this->g = g;
      g_owner = false;
   }

   for(int i = 0; i < n; i++)
   {
      this->g[i] = 0;
   }

   this->m = m;
   this->maxiter = maxiter;
   this->factr = factr;
   this->pgtol = pgtol;
   this->iprint = defaultprintlevel;

   iter = 0;
   wa  = new double[(2*m + 4)*n + 12*m*(m + 1)];
   iwa = new int[3*n];
}

// release solver resources
SolverBase::~SolverBase() { 

  delete[] lb;
  delete[] ub;
  delete[] btype;
  if(g_owner) delete[] g;
  delete[] wa;
  delete[] iwa;
}


// run one iteration of the L-BFGS-B algorithm 
void SolverBase::callLBFGS (const char* cmd) {
  if (cmd)
    copyCStrToCharArray(cmd,task,60);

// call appropiate fortran routine to the run the solver
#ifdef _WIN32
//Windows	
  SETULB(&n,&m,x,lb,ub,btype,f,g,&factr,&pgtol,wa,iwa,task,&iprint,
	  csave,lsave,isave,dsave);
#else
//Linux
  setulb_(&n,&m,x,lb,ub,btype,f,g,&factr,&pgtol,wa,iwa,task,&iprint,
	  csave,lsave,isave,dsave);

#endif

}

// Copy a C-style string (a null-terminated character array) to a
// non-C-style string (a simple character array). The length of the
// destination character array is given by "ndest". If the source is
// shorter than the destination, the destination is padded with blank
// characters.
static void copyCStrToCharArray (const char* source, char* dest, int ndest) {
  
  // Get the length of the source C string.
  int nsource = strlen(source);
  
  // Only perform the copy if the source can fit into the destination.
  if (nsource < ndest) {

    // Copy the string.
    strcpy(dest,source);

    // Fill in the rest of the string with blanks.
    for (int i = nsource; i < ndest; i++)
      dest[i] = ' ';
  }
}

// Return true if the two strings are the same. The second input
// argument must be a C-style string (a null-terminated character
// array), but the first input argument need not be one. We only
// compare the two strings up to the length of "cstr".
static bool strIsEqualToCStr (const char* str, const char* cstr) {

  // Get the length of the C string.
  int n = strlen(cstr);

  return !strncmp(str,cstr,n);
}

// Run L-BFGS-B Solver on f(x) until completion 
SolverExitStatus Solver::runSolver() {
  SolverExitStatus status = success;  // The return value.

  // This initial call sets up the structures for L-BFGS.
  callLBFGS("START");

  // Repeat until we've reached the maximum number of iterations.
  while (true) {

    // Do something according to the "task" from the previous call to
    // L-BFGS.
    if (strIsEqualToCStr(task,"FG")) {

      // Evaluate the objective function and the gradient of the
      // objective at the current point x.
      *f = computeObjective(x);
      computeGradient(x);
   //   printf("g = %f\n", g[0]);
    } 
    
    else if (strIsEqualToCStr(task,"NEW_X")) {

       // Go to the next iteration and call the iterative callback
       // routine.
       iter++;
       //printf("iter %d\n", iter);

       // If we've reached the maximum number of iterations, terminate
       // the optimization.
       if (iter == maxiter) {
          callLBFGS("STOP");
          break;
       }
    } 
    
    else if (strIsEqualToCStr(task,"CONV"))
       break;
    else if (strIsEqualToCStr(task,"ABNO")) {
       status = abnormalTermination;
       break;
    } 
    else if (strIsEqualToCStr(task,"ERROR")) {
       status = errorOnInput;
       break;
    }

    // Call L-BFGS again.
    callLBFGS();
  }

  return status;
}


// evaluate obj_func at point x
double Solver::computeObjective(double *x)
{
   return obj_func(x, aux_func_data);
}


// evaluate the gradient of f(x) 
void Solver::computeGradient(double *x)
{
  
   // central difference formula
   double h = 1e-8;

   memcpy(x_tmp1, x, sizeof(double)*n);
   memcpy(x_tmp2, x, sizeof(double)*n);

   int i;


   for(i = 0; i < n; i++)
   {
      x_tmp1[i] += h;
      x_tmp2[i] -= h;
      g[i] = (computeObjective(x_tmp1) - computeObjective(x_tmp2)) / (2*h);
      x_tmp1[i] = x[i];
      x_tmp2[i] = x[i];
   }
   

  /*

   // forward difference formula
   double h = 1e-8;

   memcpy(x_tmp1, x, sizeof(double)*n);
   unsigned int i;

   for(i = 0; i < n; i++)
   {
      x_tmp1[i] += h;
      g[i] = (computeObjective(x_tmp1) - *f) / (h);
      x_tmp1[i] = x[i];
   }
   */

}


// Run one iteration of the BFGS-B solver
// if exit status == evalWait then f(x) and its gradient 
// must be evaluated externally to this class
// before this funcion is called again
// this should be done by evaluating f(x_ret) and storing the result in f_ret
SolverExitStatus SolverExtEval::runSolver() {
  SolverExitStatus status;  // The return value.

  if(solverInit == false)
  {
     // This initial call sets up the structures for L-BFGS.
     callLBFGS("START");
     solverInit = true;
  }

  else callLBFGS();

  // Repeat until we've reached the maximum number of iterations.

  while (true) {

    // Do something according to the "task" from the previous call to
    // L-BFGS.
    if (strIsEqualToCStr(task,"FG")) {

      status =  evalWait;
      break;
    } 
    
    else if (strIsEqualToCStr(task,"NEW_X")) {

       // Go to the next iteration and call the iterative callback
       // routine.
       iter++;

       // printf("new x\n");

       // If we've reached the maximum number of iterations, terminate
       // the optimization.
       if (iter == maxiter) {
          callLBFGS("STOP");
          solverDone = true;
          status = done;
          break;
       }
    } 
    
    else if (strIsEqualToCStr(task,"CONV")) {
       status = done;
       solverDone = true;
       break;
    }
    else if (strIsEqualToCStr(task,"ABNO")) {
       status = abnormalTermination;
       solverDone = true;
       break;
    } else if (strIsEqualToCStr(task,"ERROR")) {
       status = errorOnInput;
       solverDone = true;
       break;
    }

    // Call L-BFGS again.
    callLBFGS();
  }

  return status;
}

