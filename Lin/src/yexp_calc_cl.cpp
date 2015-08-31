#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

#include "hyperspect.h"


#define MAX_STR_SZ 512			// maximum string size

// comment this line out to NOT use OpenCL code compiler optimizations
// the optimization slightly improves performance at a slight cost to mathematical
// accuracy
//#define USE_OPENCL_RELAXED_MATH_OPTS


#ifdef USE_OPENCL_RELAXED_MATH_OPTS
static const char* OpenCL_optSwitches = "-cl-mad-enable -cl-fast-relaxed-math";
#else
static const char* OpenCL_optSwitches = "";
#endif

static const char* yexpCalcKernel_name = "yexp_calc";

static char* readSource(const char *sourceFilename); 


void yexp_calc_cl(hyperspect *hyp_image, const char *yexpCalcSrcFileNameFull, const char *OpenCL_incDir)
{

    cl_context context;
    cl_command_queue cmdQueue;
    cl_kernel yexpCalcKernel;


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

   // This function reads in the source code of the program
   source = readSource(yexpCalcSrcFileNameFull);

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


   yexpCalcKernel = clCreateKernel(program, yexpCalcKernel_name, &status);
   if(status != CL_SUCCESS) {
      printf("clCreateKernel failed\n");
      exit(-1);
   }



   free(platforms);
   free(devices);
   free(source);

   int cols_rows;

   hyp_image->image_get_size(&cols_rows);
   
    double *band440_host; 
    double *band440p1_host;
    double *band490_host;
    double *band490p1_host;
    double *yexp_host;

    double wlb440, wlb490, wlb440p1, wlb490p1;

    cl_mem band440_dev; 
    cl_mem band440p1_dev;
    cl_mem band490_dev;
    cl_mem band490p1_dev;
    cl_mem yexp_dev;

    hyp_image->yexp_get_data(
          &band440_host, 
          &band440p1_host, 
          &band490_host, 
          &band490p1_host, 
          &yexp_host, 
          &wlb440, 
          &wlb440p1, 
          &wlb490, 
          &wlb490p1);


   band440_dev = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        cols_rows * sizeof(double), band440_host, &status);
   if(status != CL_SUCCESS || band440_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   band440p1_dev = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         cols_rows * sizeof(double), band440p1_host, &status);
   if(status != CL_SUCCESS || band440p1_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   band490_dev = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         cols_rows * sizeof(double), band490_host, &status);
   if(status != CL_SUCCESS ||  band490_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   band490p1_dev = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         cols_rows * sizeof(double), band490p1_host, &status);
   if(status != CL_SUCCESS || band490p1_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   yexp_dev = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         cols_rows * sizeof(double), yexp_host, &status);
   if(status != CL_SUCCESS || yexp_dev == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }


    status =  clSetKernelArg(yexpCalcKernel, 0, sizeof(cl_mem), &band440_dev);
    status |= clSetKernelArg(yexpCalcKernel, 1, sizeof(cl_mem), &band440p1_dev);
    status |= clSetKernelArg(yexpCalcKernel, 2, sizeof(cl_mem), &band490_dev);
    status |= clSetKernelArg(yexpCalcKernel, 3, sizeof(cl_mem), &band490p1_dev);
    status |= clSetKernelArg(yexpCalcKernel, 4, sizeof(cl_mem), &yexp_dev);
    status |= clSetKernelArg(yexpCalcKernel, 5, sizeof(cl_double), &wlb440);
    status |= clSetKernelArg(yexpCalcKernel, 6, sizeof(cl_double), &wlb490);
    status |= clSetKernelArg(yexpCalcKernel, 7, sizeof(cl_double), &wlb440p1);
    status |= clSetKernelArg(yexpCalcKernel, 8, sizeof(cl_double), &wlb490p1);


   if(status != CL_SUCCESS)
   {
      printf("clSetKernelArg error\n");
      exit(-1);
   }


   size_t globalWorkSize[1] = {cols_rows};
   size_t localWorkSize[1];

   // Execute the kernel.
   // 'globalWorkSize' is the 1D dimension of the work-items
   status = clEnqueueNDRangeKernel(cmdQueue, yexpCalcKernel, 1, NULL, globalWorkSize, 
                           NULL, 0, NULL, NULL);
   if(status != CL_SUCCESS) {
      printf("clEnqueueNDRangeKernel failed\n");
      exit(-1);
   }

   clFinish(cmdQueue);

   status = clEnqueueReadBuffer(cmdQueue, yexp_dev, CL_TRUE, 0,
         cols_rows * sizeof(double), yexp_host, 
         0, NULL, NULL);

   if(status != CL_SUCCESS) {
      printf("clEnqueueReadBuffer failed\n");
      exit(-1);
   }

   printf("yexp = %f\n", yexp_host[0]);

   clReleaseMemObject(band440_dev); 
   clReleaseMemObject(band440p1_dev);
   clReleaseMemObject(band490_dev);
   clReleaseMemObject(band490p1_dev);
   clReleaseMemObject(yexp_dev);

   clReleaseKernel(yexpCalcKernel);
   clReleaseProgram(program);
   clReleaseCommandQueue(cmdQueue);
   clReleaseContext(context);
}





// read OpenCL source from a file
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



