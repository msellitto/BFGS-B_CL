#include <stdio.h>
#include <string.h>

#include <CL/cl.h>

#include <iostream>
#include <fstream>

using namespace std;

#include "parallel_eval.h"
#include "bfgsb_cl.h"

// comment out to NOT use OpenCL compiler optimizations
// the optimization slightly improves performance at a slight cost to mathematical
// accuracy
//#define USE_OPENCL_RELAXED_MATH_OPTS


static char* readSource(const char *sourceFilename); 

// kernel names to look for in user provided OpenCL file
static const char* evalKernel_name = "eval_kernel";
static const char* coarseGrainKernel_name = "coarse_grained_search";

#ifdef USE_OPENCL_RELAXED_MATH_OPTS
static const char* OpenCL_optSwitches = "-cl-mad-enable -cl-fast-relaxed-math";
#else
static const char* OpenCL_optSwitches = "";
#endif



pEval::pEval(
      int num_vars, 
      int num_funcs, 
      const char *evalSrcFileNameFull, 
      const char *OpenCL_incDir,
      int num_user_args,
      bfgsb_cl_user_data_arg *user_args,
      bool use_coarse_grain_search,
      unsigned int coarse_grain_n,
      double *coarse_grain_points
      )
{
   this->num_vars = num_vars;
   this->num_funcs = num_funcs;
   sprintf(this->evalSrcFileNameFull, "%s", evalSrcFileNameFull);
   sprintf(this->OpenCL_incDir, "%s", OpenCL_incDir);
   this->num_user_args = num_user_args;

   this->use_coarse_grain_search = use_coarse_grain_search;
   this->coarse_grain_n = coarse_grain_n;

   if((use_coarse_grain_search) && (coarse_grain_n > 0))
   {
      this->coarse_grain_points = (double *) malloc(coarse_grain_n * num_vars * sizeof(double));
      memcpy(this->coarse_grain_points, coarse_grain_points, coarse_grain_n * num_vars * sizeof(double));
   }

   if(num_user_args > 0)
   {
     user_buffs = (pEval_user_buff *) malloc(num_user_args * sizeof(pEval_user_buff));

     for(int i = 0; i < num_user_args; i++)
      {
         user_buffs[i].data_dev = NULL;
         memcpy(&user_buffs[i].arg, &user_args[i], sizeof(bfgsb_cl_user_data_arg));
      }
   }

    context = NULL;
    cmdQueue = NULL;
    evalKernel = NULL;
    coarseGrainedSearchKernel = NULL;

    active_mask_dev = NULL;
    F_dev = NULL;
    x_dev = NULL;
    g_dev = NULL;
    coarse_grain_points_dev = NULL;
    init_ret_dev = NULL;

    F_host = NULL;
    x_host = NULL;
    g_host = NULL;
    active_mask_host = NULL;
}


// free all resources
pEval::~pEval()
{
   OpenCL_cleanup();
   
   if(num_user_args > 0)
   {
      free(user_buffs);
   }

   if((use_coarse_grain_search) && (coarse_grain_n > 0))
   {
      free(coarse_grain_points);
   }

   free(F_host);
   free(x_host);
   free(g_host);
   free(active_mask_host);
}


// cleanup and release OpenCL resources
void pEval::OpenCL_cleanup()
{
   clReleaseMemObject(active_mask_dev);
   clReleaseMemObject(F_dev);
   clReleaseMemObject(x_dev);
   clReleaseMemObject(g_dev);

   for(int i = 0; i < num_user_args; i++)
   {
      clReleaseMemObject(user_buffs[i].data_dev);
   }

   if((use_coarse_grain_search) && (coarse_grain_n > 0))
   {
      clReleaseMemObject(init_ret_dev);
      clReleaseMemObject(coarse_grain_points_dev);
   }


   clReleaseKernel(evalKernel);

   if((use_coarse_grain_search) && (coarse_grain_n > 0))
   {
      
      clReleaseKernel(coarseGrainedSearchKernel);
   }

   clReleaseCommandQueue(cmdQueue);
   clReleaseContext(context);
}

// setup OpenCL subsystem and initalize OpenCL kernel inputs
void pEval::OpenCL_setup()
{
   OpenCL_mainSetup();
   OpenCL_initInputs();
}


// setup OpenCL subsystem
void pEval::OpenCL_mainSetup()
{
   //****************************************************************
   //                 OpenCL SETUP
   //****************************************************************

   printf("Initializing OpenCL\n");

   cl_int status;  // use as return value for most OpenCL functions

   cl_uint numPlatforms = 0;
   cl_platform_id *platforms;
                
   // Query for the number of recongnized platforms
   status = clGetPlatformIDs(0, NULL, &numPlatforms);
   if(status != CL_SUCCESS) {
      printf("clGetPlatformIDs failed\n");
      exit(-1);
   }

   // Make sure some platforms were found 
   if(numPlatforms == 0) {
      printf("No platforms detected.\n");
      exit(-1);
   }

   // Allocate enough space for each platform
   platforms = (cl_platform_id*)malloc(numPlatforms*sizeof(cl_platform_id));
   if(platforms == NULL) {
      perror("malloc");
      exit(-1);
   }

   // Fill in platforms
   clGetPlatformIDs(numPlatforms, platforms, NULL);
   if(status != CL_SUCCESS) {
      printf("clGetPlatformIDs failed\n");
      exit(-1);
   }

   // Print out some basic information about each platform
   printf("%u platforms detected\n", numPlatforms);
   for(unsigned int i = 0; i < numPlatforms; i++) {
      char buf[100];
      printf("Platform %u: \n", i);
      status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR,
                       sizeof(buf), buf, NULL);
      printf("\tVendor: %s\n", buf);
      status |= clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                       sizeof(buf), buf, NULL);
      printf("\tName: %s\n", buf);

      if(status != CL_SUCCESS) {
         printf("clGetPlatformInfo failed\n");
         exit(-1);
      }
   }
   printf("\n");

   cl_uint numDevices = 0;
   cl_device_id *devices;

   // Retrive the number of devices present
   status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, 
                           &numDevices);
   if(status != CL_SUCCESS) {
      printf("clGetDeviceIDs failed\n");
      exit(-1);
   }

   // Make sure some devices were found
   if(numDevices == 0) {
      printf("No devices detected.\n");
      exit(-1);
   }

   // Allocate enough space for each device
   devices = (cl_device_id*)malloc(numDevices*sizeof(cl_device_id));
   if(devices == NULL) {
      perror("malloc");
      exit(-1);
   }

   // Fill in devices
   status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices,
                     devices, NULL);
   if(status != CL_SUCCESS) {
      printf("clGetDeviceIDs failed\n");
      exit(-1);
   }   

   // Print out some basic information about each device
   printf("%u devices detected\n", numDevices);
   for(unsigned int i = 0; i < numDevices; i++) {
      char buf[100];
      printf("Device %u: \n", i);
      status = clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR,
                       sizeof(buf), buf, NULL);
      printf("\tDevice: %s\n", buf);
      status |= clGetDeviceInfo(devices[i], CL_DEVICE_NAME,
                       sizeof(buf), buf, NULL);
      printf("\tName: %s\n", buf);

      if(status != CL_SUCCESS) {
         printf("clGetDeviceInfo failed\n");
         exit(-1);
      }
   }
   printf("\n");


   // Create a context and associate it with the devices
   context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
   if(status != CL_SUCCESS || context == NULL) {
      printf("clCreateContext failed\n");
      exit(-1);
   }

   // Create a command queue and associate it with the device you 
   // want to execute on
   cmdQueue = clCreateCommandQueue(context, devices[0], 0, &status);
   if(status != CL_SUCCESS || cmdQueue == NULL) {
      printf("clCreateCommandQueue failed\n");
      exit(-1);
   }

   cl_program program;
   
   char *source;

   //const char *sourceFile = "eval_kernel.cl";
   // This function reads in the source code of the program
   source = readSource(evalSrcFileNameFull);

   //printf("Program source is:\n%s\n", source);

   // Create a program. The 'source' string is the code from the 
   program = clCreateProgramWithSource(context, 1, (const char**)&source, 
                              NULL, &status);
   if(status != CL_SUCCESS) {
      printf("clCreateProgramWithSource failed\n");
      exit(-1);
   }

   cl_int buildErr;
   // Build (compile & link) the program for the devices.
   // Save the return value in 'buildErr' (the following 
   // code will print any compilation errors to the screen)a
  
   char OpenCL_buildLine[MAX_STR_SZ];
   sprintf(OpenCL_buildLine, "-I %s %s", OpenCL_incDir, OpenCL_optSwitches);
   buildErr = clBuildProgram(program, numDevices, devices, OpenCL_buildLine, NULL, NULL);
   //buildErr = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);

   // If there are build errors, print them to the screen
   if(1) {
      //printf("Program failed to build.\n");
      cl_build_status buildStatus;
      for(unsigned int i = 0; i < numDevices; i++) {
         clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS,
                          sizeof(cl_build_status), &buildStatus, NULL);
        
        /* 
         if(buildStatus == CL_SUCCESS) {
            continue;
         }
         */

         char *buildLog;
         size_t buildLogSize;
         clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG,
                          0, NULL, &buildLogSize);
         buildLog = (char*)malloc(buildLogSize);
         if(buildLog == NULL) {
            perror("malloc");
            exit(-1);
         }
         clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG,
                          buildLogSize, buildLog, NULL);
         buildLog[buildLogSize-1] = '\0';
         printf("Device %u Build Log:\n%s\n", i, buildLog);   
         free(buildLog);
      }
      //exit(0);
   }
   else {
      printf("No build errors\n");
   }


   evalKernel = clCreateKernel(program, evalKernel_name, &status);
   if(status != CL_SUCCESS) {
      printf("clCreateKernel failed\n");
      exit(-1);
   }

   if((use_coarse_grain_search) && (coarse_grain_n > 0))
   {

      coarseGrainedSearchKernel = clCreateKernel(program, coarseGrainKernel_name, &status);
      if(status != CL_SUCCESS) {
         printf("clCreateKernel failed\n");
         exit(-1);
      }
   }


   free(platforms);
   free(devices);
   free(source);

}


// initialize inputs to OpenCL kernel
void pEval::OpenCL_initInputs()
{
   cl_int status;

   active_mask_host = (int *) malloc(num_funcs*sizeof(int));
   F_host = (double *) malloc(num_funcs * sizeof(double));
   x_host = (double *) malloc(num_vars * num_funcs * sizeof(double));
   g_host = (double *) malloc(num_vars * num_funcs * sizeof(double));
  
   for(int i = 0; i < num_funcs; i++)
   {
      active_mask_host[i] = 1;
   }

   active_mask_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
         num_funcs * sizeof(int), NULL, &status);
   if(status != CL_SUCCESS || active_mask_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   F_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
        num_funcs * sizeof(double), NULL, &status);
   if(status != CL_SUCCESS || F_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   x_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
        num_vars * num_funcs * sizeof(double), NULL, &status);
   if(status != CL_SUCCESS || x_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   g_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
        num_vars * num_funcs * sizeof(double), NULL, &status);
   if(status != CL_SUCCESS || g_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }


   // set up problem specific inputs
   for(int i = 0; i < num_user_args; i++)
   {

      if(user_buffs[i].arg.buffer == true)
      {
         cl_int status;
         void *host_ptr;
         cl_mem_flags mem_flags;

         if(user_buffs[i].arg.small_const == true)
         {
            mem_flags = CL_MEM_READ_ONLY;
         }

         else mem_flags = CL_MEM_READ_WRITE;

         if(user_buffs[i].arg.init == true)
         {
            mem_flags |= CL_MEM_COPY_HOST_PTR;
            host_ptr = user_buffs[i].arg.data;
         }

         else host_ptr = NULL;

         size_t size = user_buffs[i].arg.size;

         user_buffs[i].data_dev = clCreateBuffer(context, mem_flags,
             size, host_ptr, &status);

         if(status != CL_SUCCESS || user_buffs[i].data_dev == NULL) {
            printf("clCreateBuffer failed\n");
            exit(-1);
         }

      }



   }


   // set up eval kernel parameters
   status  = clSetKernelArg(evalKernel, 0, sizeof(cl_mem), &active_mask_dev);
   status |= clSetKernelArg(evalKernel, 1, sizeof(cl_mem), &F_dev);
   status |= clSetKernelArg(evalKernel, 2, sizeof(cl_mem), &x_dev);
   status |= clSetKernelArg(evalKernel, 3, sizeof(cl_mem), &g_dev);

   if(status != CL_SUCCESS)
   {
      printf("clSetKernelArg error 1\n");
      exit(-1);
   }


   for(int i = 0; i < num_user_args; i++)
   {
      if(user_buffs[i].arg.buffer == true)
      {
         status |= clSetKernelArg(evalKernel, 4+i, sizeof(cl_mem), &user_buffs[i].data_dev);
      }

      else
      {
         status |= clSetKernelArg(evalKernel, 4+i, user_buffs[i].arg.size, user_buffs[i].arg.data);
      }

      if(status != CL_SUCCESS)
      {
         printf("%d\n", status);
         printf("clSetKernelArg error\n");
         exit(-1);
      }


   }

   //********************************************************************
   //  init inputs for coarse grained search
   //********************************************************************

   if((use_coarse_grain_search) && (coarse_grain_n > 0))
   {

      coarse_grain_points_dev = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
            coarse_grain_n * num_vars * sizeof(double) , coarse_grain_points, &status);
      if(status != CL_SUCCESS || coarse_grain_points_dev == NULL) {
         printf("clCreateBuffer failed\n");
         exit(-1);
      }

      init_ret_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
            num_vars * num_funcs * sizeof(double), NULL, &status);
      if(status != CL_SUCCESS || init_ret_dev == NULL) {
         printf("clCreateBuffer failed\n");
         exit(-1);
      }

      printf("coarse grain n = %d\n", coarse_grain_n);

      status  = clSetKernelArg(coarseGrainedSearchKernel, 0, sizeof(cl_int), &coarse_grain_n);
      status |= clSetKernelArg(coarseGrainedSearchKernel, 1, sizeof(cl_mem), &coarse_grain_points_dev);
      status |= clSetKernelArg(coarseGrainedSearchKernel, 2, sizeof(cl_mem), &init_ret_dev);


      for(int i = 0; i < num_user_args; i++)
      {
         if(user_buffs[i].arg.buffer == true)
         {
            status |= clSetKernelArg(coarseGrainedSearchKernel, 3+i, sizeof(cl_mem), &user_buffs[i].data_dev);
         }

         else
         {
            status |= clSetKernelArg(coarseGrainedSearchKernel, 3+i, user_buffs[i].arg.size, user_buffs[i].arg.data);
         }

         if(status != CL_SUCCESS)
         {
            printf("%d\n", status);
            printf("clSetKernelArg error\n");
            exit(-1);
         }


      }

   }


}



// do parallel evaluation on the GPU
void pEval::eval()
{

   cl_int status;

   // transfer x and active mask data to the GPU

   status = clEnqueueWriteBuffer(cmdQueue, active_mask_dev, CL_TRUE, 0,
         num_funcs * sizeof(int), active_mask_host, 
         0, NULL, NULL);         
   if(status != CL_SUCCESS) {
      printf("clEnqueueWriteBuffer failed\n");
      exit(-1);
   }

   status = clEnqueueWriteBuffer(cmdQueue, x_dev, CL_TRUE, 0,
         num_vars * num_funcs * sizeof(double), x_host, 
         0, NULL, NULL);         
   if(status != CL_SUCCESS) {
      printf("clEnqueueWriteBuffer failed\n");
      exit(-1);
   }

   size_t globalWorkSize[1] = {num_funcs};
   size_t localWorkSize[1] = {64};

   // Execute the kernel.
   // 'globalWorkSize' is the 1D dimension of the work-items
   status = clEnqueueNDRangeKernel(cmdQueue, evalKernel, 1, NULL, globalWorkSize, 
                           NULL, 0, NULL, NULL);
   if(status != CL_SUCCESS) {
      printf("clEnqueueNDRangeKernel failed\n");
      exit(-1);
   }

   clFinish(cmdQueue);

	// copy F(x) and gradient back to the host

   status = clEnqueueReadBuffer(cmdQueue, F_dev, CL_TRUE, 0,
         num_funcs * sizeof(double), F_host, 
         0, NULL, NULL);

   if(status != CL_SUCCESS) {
      printf("clEnqueueReadBuffer failed\n");
      exit(-1);
   }


   status = clEnqueueReadBuffer(cmdQueue, g_dev, CL_TRUE, 0,
         num_vars * num_funcs * sizeof(double), g_host, 
         0, NULL, NULL);

   if(status != CL_SUCCESS) {
      printf("clEnqueueReadBuffer failed\n");
      exit(-1);
   }

}

// perform coarse grain search in parallel on the GPU
void pEval::coarse_grain_search(double *init_ret)
{
   cl_int status;

   size_t globalWorkSize[1] = {num_funcs};
   size_t localWorkSize[1] = {64};

   // Execute the kernel.
   // 'globalWorkSize' is the 1D dimension of the work-items
   status = clEnqueueNDRangeKernel(cmdQueue, coarseGrainedSearchKernel, 1, NULL, globalWorkSize, 
                           NULL, 0, NULL, NULL);
   if(status != CL_SUCCESS) {
      printf("clEnqueueNDRangeKernel failed\n");
      exit(-1);
   }

   clFinish(cmdQueue);

   status = clEnqueueReadBuffer(cmdQueue, init_ret_dev, CL_TRUE, 0,
         num_vars * num_funcs * sizeof(double), init_ret, 
         0, NULL, NULL);

   if(status != CL_SUCCESS) {
      printf("clEnqueueReadBuffer failed\n");
      exit(-1);
   }


}


// read OpenCL source from file
char* readSource(const char *sourceFilename) {

   FILE *fp;
   int err;
   int size;

   char *source;

   fp = fopen(sourceFilename, "rb");
   if(fp == NULL) {
      printf("Could not open kernel file: %s\n", sourceFilename);
      exit(-1);
   }
   
   err = fseek(fp, 0, SEEK_END);
   if(err != 0) {
      printf("Error seeking to end of file\n");
      exit(-1);
   }

   size = ftell(fp);
   if(size < 0) {
      printf("Error getting file position\n");
      exit(-1);
   }

   err = fseek(fp, 0, SEEK_SET);
   if(err != 0) {
      printf("Error seeking to start of file\n");
      exit(-1);
   }

   source = (char*)malloc(size+1);
   if(source == NULL) {
      printf("Error allocating %d bytes for the program source\n", size+1);
      exit(-1);
   }

   err = fread(source, 1, size, fp);
   if(err != size) {
      printf("only read %d bytes\n", err);
      exit(0);
   }

   source[size] = '\0';

   return source;
}



