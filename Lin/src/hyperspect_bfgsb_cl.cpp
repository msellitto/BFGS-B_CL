#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For cmd line handling
#ifdef _WIN32
//Windows
#include "getopt.h"

#else
//Linux
#include <unistd.h>  
#endif

#include <iostream>
#include <fstream>

using namespace std;

#include "bfgsb_cl.h"
#include "hyperspect_constants.h"
#include "hyperspect.h"
#include "solver.h"
#include "coarse_grain.h"
#include "yexp_calc_cl.h"

#define MAX_STR_SZ 512  // max string size


// Set directory information here

// point data and OpenCL src directories back to the Linux folders
#if 0
// Point back data and src directories back to Linux folder.
static const char* dataDir = "../../Lin/data";
static const char* OpenCL_sourceDir = "../../Lin/src";
static const char* OpenCL_incDir = "../../Lin/src";

#else

static const char* dataDir = "data";
static const char* OpenCL_sourceDir = "src";
static const char* OpenCL_incDir = "src";

#endif


static const char* outputDir = "output";
static const char* OpenCLEvalFileName = "eval_kernel.cl";
static const char* OpenCLYexpCalcFileName = "yexp_calc.cl";

// internal functions
static void setGlobalSettings(int argc, char *argv[]);
static void processCmdArgs(int argc, char *argv[]);
static void display_usage();
static void globalSettingsErrorChk();
static void display_prologue();

static void mySettings();
static void mySettings_synth();
static void mySettings_real();
static void hyperspect_bfgsb_cl_run_cpu(double *params_ret, double *err_ret);
static void hyperspect_bfgsb_cl_run_gpu(double *params_ret, double *err_ret);

static double image_f(double *x, void *aux);

// global settings for program
struct s_globalSettings {
   char imageFileNameFull[MAX_STR_SZ];           // image file to use
   int num_image_rows;                           // number of image rows
   int num_image_cols;                           // number of image columns
   int cols_rows;                                // columns x rows
   char spectInpFileNameFull[MAX_STR_SZ];        // spectral input file
   bool useSerialCPUVersion;                     // use serial CPU version instead of CPU-GPU OpenCL version
   int num_cpu_work_threads;                     // number of cpu working threads to use in OpenCL version
   int hessian_approx_factor;                    // hessian approximate factor (m) to use in solver
   int max_iterations;                           // maximum iterations to use in solver
   char paramOutFileNameFull[MAX_STR_SZ];        // binary output file of parameters
   bool createASCIIParamOutFile;                 // also create ascii output file
   bool useCoarseGrainedSearch;                  // use coarse-grained search before solver to find initial values
   unsigned int coarse_grain_n;                  // number of coarse grain points to use
   char coarseGrainInitFileNameFull[MAX_STR_SZ]; // file to read in  
   bool calcYexp;                                // calculate yexp
   bool verbosePrint;							 // prints out more information about program while its running
} globalSettings;


// main hyperspect program entry point
void hyperspect_bfgsb_cl(int argc, char *argv[])
{
   // set program global settings
   setGlobalSettings(argc, argv);

   // print prologue
   display_prologue();

   printf("Running...\n");

   // parameters and error vals to be filled in by the solver (these are the final outputs)
   double *params = (double *) malloc(globalSettings.cols_rows * 5 * sizeof(double));
   double *err = (double *) malloc(globalSettings.cols_rows * sizeof(double));

   // use cpu version if selected
   if(globalSettings.useSerialCPUVersion)
   {
      hyperspect_bfgsb_cl_run_cpu(params, err);
   }

   // else use multi-threaded CPU + OpenCL GPU version
   else
   {
      hyperspect_bfgsb_cl_run_gpu(params, err);
   }

   // write to output files if needed
   if(globalSettings.paramOutFileNameFull[0] != '\0')
   {
      // output image format is in floats (not double)
      float *params_f = (float *) malloc(globalSettings.cols_rows * 5 * sizeof(float));
      float *err_f = (float *) malloc(globalSettings.cols_rows * sizeof(float));

      for(int i = 0; i < globalSettings.cols_rows; i++)
      {
         err_f[i] = err[i];
      }

      for(int i = 0; i < 5*globalSettings.cols_rows; i++)
      {
         params_f[i] = params[i];
      }

      FILE *paramsOutF = fopen(globalSettings.paramOutFileNameFull, "wb");
      if(paramsOutF == NULL) {
         perror("paramsOutF");
         exit(1);
      }

      for(int i = 0; i < globalSettings.cols_rows; i++)
      {
         fwrite(&(params_f[i*5]), sizeof(float), 5, paramsOutF);
         fwrite(&(err_f[i]), sizeof(float), 1, paramsOutF);
      }

      free(params_f);
      free(err_f);

      // create output file in regular txt format if needed
      if(globalSettings.createASCIIParamOutFile)
      {
         char outputFilename[MAX_STR_SZ];
         sprintf(outputFilename, "%s.txt", globalSettings.paramOutFileNameFull);

         FILE *paramsOutTxtF = fopen(outputFilename, "w");
         if(paramsOutTxtF == NULL) {
            perror("paramsOutTxtF");
            exit(1);
         }

		 // output file column headers
         fprintf(paramsOutTxtF, "row col  P      G      BP     B      H       err \n");

         double *p = params;

         for(int i = 0; i < globalSettings.cols_rows; i++) 
         {
            int myrow = i/globalSettings.num_image_cols;
            int mycol = i % globalSettings.num_image_cols;


            fprintf(paramsOutTxtF, "%3d %3d %2.4f %2.4f %2.4f %2.4f %2.4f %2.4f\n", myrow, 
                  mycol,
                  p[0],
                  p[1],
                  p[2],
                  p[3],
                  p[4],
                  err[i]);

            p = p+5;
         }


         fclose(paramsOutTxtF);
      }

   }


   free(params);
   free(err);
}


// image structure to pass to CPU image function
typedef struct s_imageFStruct {
   int offset;						// 1-d element offset
   hyperspect *hyp_image_p;         // pointer to the hyperspectral image object
} imageFStruct;


// Serial CPU Version of the solver (does not use GPU at all).
// returns results in params_ret and err_ret
static void hyperspect_bfgsb_cl_run_cpu(double *params_ret, double *err_ret)
{

   double x_init[5] = {0.05, 0.2, 0.001, 0.1, 1};   // default solver start point
   double L[5] = {minP, minG, minBP, minB, minH};	// lower bounds
   double U[5] = {maxP, maxG, maxBP, maxB, maxH};   // upper bounds
   int b[5] = {2, 2, 2, 2, 2};                      // bound types (both upper and lower)
   double *coarse_grain_points = NULL;				// coarse grain search init points

   // if also using coarse grained search
   if(globalSettings.useCoarseGrainedSearch && (globalSettings.coarse_grain_n > 0))
   {
      if(globalSettings.coarseGrainInitFileNameFull[0] != '\0')
      {
         coarse_grain_points = (double *) malloc(globalSettings.coarse_grain_n * 5 * sizeof(double));

         ifstream coarse_grain_init_file;

         coarse_grain_init_file.open(globalSettings.coarseGrainInitFileNameFull, ifstream::in);

         for(unsigned int i = 0; i < 5*globalSettings.coarse_grain_n; i++)
         {
            coarse_grain_init_file >> coarse_grain_points[i];
         }

         coarse_grain_init_file.close();

      }
   }

   // set up hyperspectral image
   hyperspect hyp_image(
         globalSettings.imageFileNameFull, 
         globalSettings.num_image_rows, 
         globalSettings.num_image_cols,
         globalSettings.spectInpFileNameFull,
         globalSettings.calcYexp);

   // for each image element run the CPU solver
   for(int id = 0; id < globalSettings.cols_rows; id++)
   {
      if(globalSettings.verbosePrint) printf("id %d\n", id);
      imageFStruct aux_data;
      aux_data.offset = id * total_bands;
      aux_data.hyp_image_p = &hyp_image;

	  // run coarse grained search if needed
      if(globalSettings.useCoarseGrainedSearch && (globalSettings.coarse_grain_n > 0))
      {
         coarse_grain_search(5, globalSettings.coarse_grain_n, coarse_grain_points, image_f, &aux_data, x_init);
      }

	  // Setup BFGS-B CPU solver
      Solver solver(
            5, 
            image_f, 
            sizeof(imageFStruct), 
            &aux_data, 
            x_init, 
            L, 
            U,
            b, 
            &(params_ret[id*5]), 
            &(err_ret[id]), 
            globalSettings.hessian_approx_factor, 
            globalSettings.max_iterations); 
   
	  // Run BFGS-B CPU solver
      solver.runSolver();
   }

   if(globalSettings.useCoarseGrainedSearch && globalSettings.coarse_grain_n > 0) free(coarse_grain_points);
}


// image function for use with CPU version
static double image_f(double *x, void *aux)
{
   imageFStruct aux_data = *(imageFStruct *) aux;
   int rss_offset_index = aux_data.offset;
   hyperspect *hyp_image_p = aux_data.hyp_image_p;
   
   return hyp_image_p->obj_fun(x[0], x[1], x[2], x[3], x[4], rss_offset_index);
}





// do compile time settings here
static void mySettings()
{
  // mySettings_synth();
  //mySettings_real();

}



static void mySettings_synth()
{
   sprintf(globalSettings.imageFileNameFull, "%s/%s", dataDir, "test_file_reflectance");
   sprintf(globalSettings.spectInpFileNameFull, "%s/%s", dataDir, "spec_in_aviris_kbay_coral.txt");
   globalSettings.num_image_rows = 256;
   globalSettings.num_image_cols = 51;
   globalSettings.useSerialCPUVersion = false;
   globalSettings.num_cpu_work_threads = 8;
   globalSettings.hessian_approx_factor = 6;
   globalSettings.max_iterations = 2000;
   sprintf(globalSettings.paramOutFileNameFull, "%s/%s", outputDir, "paramOutput_gpu");
   globalSettings.createASCIIParamOutFile = true;
   globalSettings.useCoarseGrainedSearch = false;
   globalSettings.coarse_grain_n = 0;
   globalSettings.coarseGrainInitFileNameFull[0] = '\0';
   //sprintf(globalSettings.coarseGrainInitFileNameFull, "%s/%s", dataDir, "initial729.txt");
   globalSettings.calcYexp = false;
}

static void mySettings_real()
{
   sprintf(globalSettings.imageFileNameFull, "%s/%s", dataDir, "1212r14_Rrs_sub_geo3_fs");
   sprintf(globalSettings.spectInpFileNameFull, "%s/%s", dataDir, "spec_in_aviris_enrique.txt");
   globalSettings.num_image_rows = 300;
   globalSettings.num_image_cols = 350;
   globalSettings.useSerialCPUVersion = false;
   globalSettings.num_cpu_work_threads = 8;
   globalSettings.hessian_approx_factor = 6;
   globalSettings.max_iterations = 2000;
   sprintf(globalSettings.paramOutFileNameFull, "%s/%s", outputDir, "paramOutput_gpu_r1");
   globalSettings.createASCIIParamOutFile = true;
   globalSettings.useCoarseGrainedSearch = false;
   globalSettings.coarse_grain_n = 0;
   globalSettings.coarseGrainInitFileNameFull[0] = '\0';
   //sprintf(globalSettings.coarseGrainInitFileNameFull, "%s/%s", dataDir, "initial729.txt");
   globalSettings.calcYexp = true;
}



// run bfgsb_cl on hyperspect data using GPU and multithreaded CPU
// returns results in params_ret and err_ret
static void hyperspect_bfgsb_cl_run_gpu(double *params_ret, double *err_ret)
{
   double *coarse_grain_points = NULL;

   // set up hyperspectral image
   hyperspect hyp_image(
         globalSettings.imageFileNameFull, 
         globalSettings.num_image_rows, 
         globalSettings.num_image_cols, 
         globalSettings.spectInpFileNameFull,
         false);

   // calculate yexp if needed on the gpu
   if(globalSettings.calcYexp == true)
   {
      char yexpCalcSrcFileNameFull[MAX_STR_SZ];
      sprintf(yexpCalcSrcFileNameFull, "%s/%s", OpenCL_sourceDir, OpenCLYexpCalcFileName); 

      yexp_calc_cl(&hyp_image, yexpCalcSrcFileNameFull, OpenCL_incDir);
   }

   // if also using coarse grained search
   if(globalSettings.useCoarseGrainedSearch && (globalSettings.coarse_grain_n > 0))
   {
      if(globalSettings.coarseGrainInitFileNameFull[0] != '\0')
      {
         coarse_grain_points = (double *) malloc(globalSettings.coarse_grain_n * 5 * sizeof(double));

         ifstream coarse_grain_init_file;

         coarse_grain_init_file.open(globalSettings.coarseGrainInitFileNameFull, ifstream::in);

         for(unsigned int i = 0; i < 5*globalSettings.coarse_grain_n; i++)
         {
            coarse_grain_init_file >> coarse_grain_points[i];
         }

         coarse_grain_init_file.close();
      }
   }

   double *image; 
   double *spectral_input;
   double *powf_spectral43;
   double *yexp;

   int total_image_elements = globalSettings.cols_rows * total_bands;

   // get image data to pass to solver
   hyp_image.image_get_data(&image, &spectral_input, &powf_spectral43, &yexp);

   bfgsb_cl_user_data_arg user_args[5];  // user data arguments to pass to Open CL function

   // rss_calc
   user_args[0].buffer = true;
   user_args[0].size = total_image_elements * sizeof(double);
   user_args[0].init = false;
   user_args[0].data = NULL;
   user_args[0].small_const = false;

   // hyperspectral image
   user_args[1].buffer = true;
   user_args[1].size = total_image_elements * sizeof(double);
   user_args[1].init = true;
   user_args[1].data = image;
   user_args[1].small_const = false;

   // spectral_input
   user_args[2].buffer = true;
   user_args[2].size = total_bands * 6 * sizeof(double);
   user_args[2].init = true;
   user_args[2].data = spectral_input;
   user_args[2].small_const = true;

   // powf_spectral43
   user_args[3].buffer = true;
   user_args[3].size = total_bands * sizeof(double); 
   user_args[3].init = true;
   user_args[3].data = powf_spectral43;
   user_args[3].small_const = true;

   // yexp
   user_args[4].buffer = true;
   user_args[4].size = globalSettings.cols_rows * sizeof(double);
   user_args[4].init = true;
   user_args[4].data = yexp;
   user_args[4].small_const = false;

   //printf("yexp = %f\n", yexp[0]);

   // solver initializations
   double params_init[5] = {0.05, 0.2, 0.001, 0.1, 1};   // default solver start point
   double L[5] = {minP, minG, minBP, minB, minH};        // lower bounds
   double U[5] = {maxP, maxG, maxBP, maxB, maxH};        // upper bounds
   int b[5] = {2, 2, 2, 2, 2};                           // bound types (both upper and lower)

   char OpenCLEvalFileNameFull[MAX_STR_SZ];
   sprintf(OpenCLEvalFileNameFull, "%s/%s", OpenCL_incDir, "eval_kernel.cl");

   // call bfgsb_cl (multi-threaded CPU + GPU) solver to run on hyperspectral data
   bfgsb_cl(
      5,
      params_init,
      b,
      L,
      U,
      globalSettings.cols_rows,
      OpenCLEvalFileNameFull,
      OpenCL_incDir,
      5,
      user_args,
      globalSettings.max_iterations,
      globalSettings.hessian_approx_factor,
      globalSettings.num_cpu_work_threads,
      params_ret,
      err_ret,
      globalSettings.useCoarseGrainedSearch,
      globalSettings.coarse_grain_n,
      coarse_grain_points,
	  globalSettings.verbosePrint);


   if(globalSettings.useCoarseGrainedSearch && globalSettings.coarse_grain_n > 0) free(coarse_grain_points);
}



// Outputs run information to stdout prior to running solver
static void display_prologue()
{
   printf("\n");
   printf("Running hyperspect_bfgsb_cl\n");
   printf("On image file: %s\n", globalSettings.imageFileNameFull);
   printf("Containing %d x %d = %d image element(s)\n", globalSettings.num_image_rows, globalSettings.num_image_cols, globalSettings.num_image_rows * globalSettings.num_image_cols);
   printf("Using spectral input file: %s\n", globalSettings.spectInpFileNameFull);

   if(globalSettings.paramOutFileNameFull[0] != '\0')
   {
      printf("Outputting results to: %s\n", globalSettings.paramOutFileNameFull);

      if(globalSettings.createASCIIParamOutFile)
      {
         printf("And also in ASCII formatted file: %s.txt\n", globalSettings.paramOutFileNameFull);
      }

   } 

   if(globalSettings.calcYexp)
   {
      printf("Also calculating yexp\n");
   }

   if(globalSettings.useSerialCPUVersion)
   {
      printf("Computing on CPU\n");
   }

   else
   {
      printf("Computing on GPU using %d CPU working thread(s)\n", globalSettings.num_cpu_work_threads);
   }


   printf("\nOptions:\n");
   printf("Max iterations = %d\n", globalSettings.max_iterations);
   printf("Hessian approx factor = %d\n", globalSettings.hessian_approx_factor);
   
   if(globalSettings.coarse_grain_n > 0)
   {
      printf("Using %d-point coarse-grained search", globalSettings.coarse_grain_n);
  
      if(globalSettings.coarseGrainInitFileNameFull[0] != '\0')
      {
         printf(" using %s as coarse-grain init file", globalSettings.coarseGrainInitFileNameFull);
      }

      printf("\n");

   }

   if(globalSettings.verbosePrint) printf("Verbose print on\n");


   printf("\n");

}


// set program global settings
static void setGlobalSettings(int argc, char *argv[])
{
   // set default global settings
   globalSettings.imageFileNameFull[0] = '\0';
   globalSettings.spectInpFileNameFull[0] = '\0';
   globalSettings.num_image_rows = 0;
   globalSettings.num_image_cols = 0;
   globalSettings.useSerialCPUVersion = false;
   globalSettings.num_cpu_work_threads = 1;
   globalSettings.hessian_approx_factor = 6;
   globalSettings.max_iterations = 2000;
   globalSettings.paramOutFileNameFull[0] = '\0';
   globalSettings.createASCIIParamOutFile = false;
   globalSettings.useCoarseGrainedSearch = false;
   globalSettings.coarse_grain_n = 0;
   globalSettings.coarseGrainInitFileNameFull[0] = '\0';
   globalSettings.calcYexp = false;
   globalSettings.verbosePrint = false;

   // get settings from cmd line args
   processCmdArgs(argc, argv);

   // custom compile time settings
   mySettings();

   globalSettings.cols_rows = globalSettings.num_image_rows * globalSettings.num_image_cols;

   // TODO: Init file for settings

   // check configuration for errors
   globalSettingsErrorChk();
}



// set global settings according to cmd line arguments
// relies on getopt() to do the real work
static void processCmdArgs(int argc, char *argv[])
{
   const char *optString = "i:w:l:r:asp:m:t:o:ac:n:yhv?";

   int opt = getopt(argc, argv, optString);

   while(opt != -1)
   {
      switch(opt)
      {
      case 'i':
         {
            sprintf(globalSettings.imageFileNameFull, "%s/%s", dataDir, optarg); 
         }
         break;
      case 'w':
         {
            globalSettings.num_image_rows = atoi(optarg);
         }
         break;
      case 'l':
         {
            globalSettings.num_image_cols = atoi(optarg);
         }
         break;
       case 'r':
         {
            sprintf(globalSettings.spectInpFileNameFull, "%s/%s", dataDir, optarg); 
         }
         break; 
       case 'a':
         {
            globalSettings.createASCIIParamOutFile = true;
         }
         break;
       case 's':
         {
            globalSettings.useSerialCPUVersion = true;
         }
         break;
      case 'p':
         {
            globalSettings.num_cpu_work_threads = atoi(optarg);   
         }
         break;
     case 'c':
         {
            globalSettings.useCoarseGrainedSearch = true;
            globalSettings.coarse_grain_n = atoi(optarg);   
         }
         break;
     case 'n':
         {
            sprintf(globalSettings.coarseGrainInitFileNameFull, "%s/%s", dataDir, optarg);
         }
         break;
      case 'm':
         {
            globalSettings.hessian_approx_factor = atoi(optarg);
         }
         break;
      case 't':
         {
            globalSettings.max_iterations = atoi(optarg);
         }
         break;
      case 'o':
         {
            sprintf(globalSettings.paramOutFileNameFull, "%s/%s", outputDir, optarg);
         }
         break;
      case 'y':
         {
            globalSettings.calcYexp = true;
         }
         break;
      case 'h':
         {
            display_usage();
            exit(EXIT_SUCCESS);
         }
         break;
      case 'v':
         {
			 globalSettings.verbosePrint = true;
         }
         break;
      case '?':
         {
            display_usage();
            exit(EXIT_FAILURE);
         }
         break;
      default:
         break;

      }

      opt = getopt(argc, argv, optString);
   }

}

// TODO check configuratin for error, quit gracefully if so
static void globalSettingsErrorChk()
{



}

// display program usage
static void display_usage()
{
   printf("\n");
   printf("hyperspect_bfgsb_cl\n\n"); 
   printf("Usage: hyperspect_bfgsb_cl -i <image_file> -w <num_image_rows> -l <num_image_cols> [options]\n\n");
   printf("Runs bfgsb_cl algorithm on the supplied hyperspectral image file of <num_image_rows>x<num_image_cols> pixels.\n");
   printf("Image_file should be placed in the ./data directory)\n\n");
   printf("-------------\n");
   printf("Command line options:\n\n");
   printf("-v : Use verbose printing (prints out progress of the optimization solver).\n\n");
   printf("-h : Display this help message.\n\n");
   printf("-s : Use serial CPU only version of bfgsb instead of GPU (default is to use GPU).\n\n");
   printf("-p <num_cpu_work_threads> : Number of cpu work threads to use with gpu version (default is 1).\n\n");
   printf("-m <hessian_approx_factor> : Hessian approximation factor to use for bfgsb (default is 6).\n");
   printf("                            (higher is better but more compute intensive)\n\n");
   printf("-t <max_iterations>: Maximum number of iterations to use for bfgsb (default is 2000)\n");  
   printf("                     (higher is better but more compute intensive)\n\n");
   printf("-o <param_out_file> : Output hyperspectral parameters in binary format to this file (will be placed in the ./output directory).\n\n");
   printf("-a : Will also write an ASCII formatted params out file <param_out_file>.txt when used with -o (will be placed in the ./output directory).\n\n");
   printf("-y : Calculate yexp (Use this switch when using real-world images in order to calculate yexp).\n\n");
   printf("-c <n> : Use coarse grained search with <n> points to find initial starting positions.\n\n");
   printf("-n <coarse_grain_init_file> : Use <coarse_grain_init_file> contents as initial starting positions for coarse grained search.\n");
   printf("                              (must be used with switch -c) (should be placed in the ./data directory)\n\n");
   printf("\n");
}
