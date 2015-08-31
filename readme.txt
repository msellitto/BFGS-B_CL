O==================|||---------------------------------|||====================O
                   |||    Hyperspect BFGS-B CL Readme  |||       
O==================|||---------------------------------|||====================O

Title: Readme for Hyperspect BFGS-B CL
Written by: Matt Sellitto
Date: 3/10/2012
Contact: matthew.sellitto@gmail.com

==============================================================================
                              - TABLE OF CONTENTS -
==============================================================================

1] Introduction........................................................[vb100]

    1.1] Purpose of this Document  . . . . . . . . . . .  . . . .[vb101]
    1.2] The Hyperspect BFGS-B CL Application . . . . . . . . . .[vb102]


2] Code Overview.......................................................[vb200]

    2.1] Codebase and Compiling Hyperspect BFGS-B CL. . . . . . .[vb201]
    2.2] Linux Version . . . . . . . . . . . . . . . . . . . . . [vb202]
    2.3] Windows Version . . . . . . . . . . . . . . . . . . . . [vb203]
    2.4] A Note on OpenCL Compilation . . . . . . . . . . . . . .[vb204]

3] Program Usage.......................................................[vb300]

    3.1] Introduction . . . . . . . . . . . . . . . . . . . . . .[vb301]
    3.2] Detailed Usage . . . . . . . . . . . . . . . . . . . . .[vb302]

	
4] Program Usage.......................................................[vb300]

    3.1] Introduction . . . . . . . . . . . . . . . . . . . . . .[vb301]
    3.2] Files . . . . . . . . . . . . . . . . . . . . . . . . . [vb302]



==============================================================================
                                - 1. Introduction -
==============================================================================

1.1 Purpose of this Document

This document provides an overview for a developer working on the 
Hyperspect BFGS-B CL code. It gives a brief overview of the purpose of the 
program, how to use it, and how it works (including how some of the main 
pieces of code fit together). 

1.2 The Hyperspect BFGS-B CL Application

The purpose of the Hyperspect BFGS-B CL Application is to take in as an input 
a hyperspectral image file, input parameters (input on the command line) and
to create as an output file that contains the water paremeters 
(P, B, BP, G, and H) for each water column (or pixel) in the source image.

The application uses the BFGS-B CL Solver to solve a 5-dimensional non-linear 
bound constrained optimization problems. The Solver uses multiple CPU threads
and a GPU (through the OpenCL framework) to solve the optimization problems
in parallel to achieve significant speedup over ordinary serial methods.

For more in-depth coverage of hyperspectral imaging, mathematical optimization 
theory pertaining to this application, the OpenCL framework, and the BFGS-B
CL Solver please see "Accelerating an Imging Specstroscopy Algorithm for 
Submerged Marine Environments Using Heterogenous Computing" by Sellitto.


==============================================================================
                                - 2. Code Overview
==============================================================================


2.1 Codebase and Compiling Hyperspect BFGS-B CL

The application is coded in C++ and Fortran along with some OpenCL files. 
The application runs on both Linux and Windows operating systems. Both
versions share the same basic codebase though the Windows version includes 
some additional pieces of code to replace some Linux libraries that are not 
provided by Windows standard libraries. 

2.1.1 Linux Version

Code: All the code for the Linux version is locaed in Lin/src

Compiling: The Linux version is compiled with Make by using the g++ and  
gfortran compilers. The executable is created in the Lin directory. To
compile, enter the command "make" in the Lin directory or type "make clean" 
to clean out all intermediate compilation files. For the "make" command there 
are two variables that can be set  on the command line to change how the 
program is compiled.

debug=1: This turns off compile optimizations and enables debug
info to use with a debugger such as gdb.

profile=1: Turns on profiling information to be used with gprof. 

Dependent Libraries: The Linux version depends on the standard C++ library, the
standrd Fortran libraries (lgfortran), the POSIX Thread Library (lpthread),
and the OpenCL library (lOpenCL). The Linux version also uses the sys/time.h 
header and unistd.h header for functions to handle program timing and the 
handling of command line options.

2.1.2 Windows Version

Code: The Windows version uses the same files as the basic codebase as the
Linux version. Therefore most of the code for the Windows version is
located in Lin/src. This was done to keep the codebases the same as the vast
majority of the code is not dependent on the operating system the program is
being executed on. Additional code that replaces some Linux specific libraries
is located in: Win/bfgsb_CL/Source and Win/bfgsb_CL/Include. The additional
code is for timing functions (gettimeofday) and handling command line
arguments (getopt). The additional code also includes the pthread.h header file
for use with windows.

Compiling: The Windows version is made to used with Micorosft Visual Studio 
2008 or above. The solution file that should be opened for development under 
Windows is located at Win/hyperspect_bfgsb_CL.sln. The C++ parts of the code
are compiled with the Microsoft C++ compiler and the Fortran code is compiled
with the Intel Fortran Compiler. The Intel Fortran compiler must be installed
as an extension in Visual Studio by installing Intel Visual Fortran. To
compile the program simply build the solution in Visual Studio. There are two 
different executables that can be built by selecting from the listbox in 
Visual Studio. The Debug build that contains debugging information and turns
off compile-time optimizations and the Release build that turns off debugging
information and turns on compile-time optimizations. Those executables are
created in Win/x64/Debug and Win/x64/Release respectively. There are also 
32-bit variant builds in case they are needed that are built in Win/Debug and
Win/Release.

Dependent Libraries: In addition to the standard runtime libraries for C++ and
Fortran and the OpenCL library the Windows version also relies on the Pthreads-w32
library since Windows does not have built-in support for the POSIX Thread library.
This library provides the POSIX Threads interface by wrapping it around the
standard Win32 Threads library. The library is located in Win/bfgsb_CL/Lib
For more information on Pthreads-w32 see http://sourceware.org/pthreads-win32/

2.2 A Note on OpenCL Compilation

The OpenCL source files *.cl are compiled on-the-fly during runtime. 
The program looks for these files in the ./src directory. 
(Where the "." directory is the programs working directory)


==============================================================================
                                - 3. Program Usage
==============================================================================


3.1 Introduction

Hyperspect BFGS-B CL is run from the command line. Currently the program is
set up to look for hyperspectral image file and spectral parameter files i
n the ./data folder in the application's working directory. It places the 
resulting output files in the ./output folder in the application's working
directory.

3.2 Detailed Usage

The detailed command line usage are as follows:
(note that this can also be viewed by using the -h command line switch)


Usage: 
hyperspect_bfgsb_cl -i <img_file> -w <num_img_rows> -l <num_image_cols> [opts]

Runs bfgsb_cl algorithm on the supplied hyperspectral image file 
of <num_img_rows>x<num_img_cols> pixels.
Image_file should be placed in the ./data directory)

-------------
Command line options:

-h : 
Display this help message.

v : 
Use verbose printing (prints out progress of the optimization solver).

-s : 
Use serial CPU only version of bfgsb instead of GPU (default is to use GPU).

-p <num_cpu_work_threads> : 
Number of cpu work threads to use with gpu version (default is 1).

-m <hessian_approx_factor> : 
Hessian approximation factor to use for bfgsb (default is 6).
(higher is better but more compute intensive)

-t <max_iterations>: 
Maximum number of iterations to use for bfgsb (default is 2000)
(higher is better but more compute intensive)

-o <param_out_file> : 
Output hyperspectral parameters in binary format to this file. (will be
placed in the ./output directory).

-a : 
Will also write an ASCII formatted params out 
file <param_out_file>.txt when used with -o  (will be placed in 
the ./output directory).

-c <n> : 
Use coarse grained search with <n> points to find initial starting positions.

-n <coarse_grain_init_file> :
Use <coarse_grain_init_file> contents as initial starting 
positions for coarse grained search. (must be used with switch -c) 
(should be placed in the ./data directory)

-y : Calculate yexp (Use this switch when using real-world images in order 
to calculate yexp).

Note: This detailed description is available by giving the program the -h
option on the command line.

==============================================================================
                                - 4. Code Structure
==============================================================================

4.1 Introduction

This section lists the various source code files in the Hyperspectral BFGS-B
program and their function. Please see the comments in the code for more
details.

4.2 Files

main.cpp:

This is the programs entry point, it passes the command line arguments to the
main hyperspect BFGS-B CL function and it calls timing functions to time the
total runtime of the program.

time_util.h,
time_util.cpp:

This section provides functions for timing the runtime of the program.

gettimeofday.h,
gettimeofday.cpp:

This section is used in the Windows version only and provides functions to 
get the current time.

hyperspect_bfgsb_cl.cpp,
hyperspect_bfgsb_cl.h:

This section is the main control part of the program and is responsible
for setting program globaling configuration, handling the command line
arguments, setting up global memory structures, and calling the other program
modules. 

getopt.h,
getopt.cpp:

This section is used in the Windows version only and provides functions to
handle command line arguments.

hyperspect_constants.h

This header sets up some compile time variables that are needed by other parts
of the program that deal with the hyperspectral image.

hyperspect.cpp,
hyperspect.h:

This section of the code provides an abstract data type for encapsulating a
hyperspectral image. It functions to provide methods to read in an image and
provide properties that are associated with that image.

solver.h,
solver.cpp:

This section encapsulates a single optimization solver. It provides two
different types of solvers, one where the class evaluates the functions
and gradients and one where the evaluations are done by code external
to the class. The first is used when in the CPU-only mode, the second
is used when using the GPU to perform the evaluations in parallel.
This code calls the Fortran solver code in lbfgsb.f

lbfgsb.f

This code is the base serial BFGS-B optimization solver that serves as the
basic building block for the parallel optimization solver.

bfgsb_cl.h,
bfgsb_cl.cpp:

This module is the general purpose non-linear parallel optimization
solver. It uses many solver instances with pthreads and the parallel
evaluation modules to solve the optimization problems in paralle using
the GPU and multiple CPU threads.

parallel_eval.h,
parallel_eval.cpp:

This module executes function evaluations in parallel by using OpenCL.
It calls the kernel functions in eval_kernel.cl.

coarse_grain.h,
coarse_grain.cpp:

This modules provides code to perform a coarse-grain search on a function
when in the CPU-only mode.

yexp_calc_cl.h,
yexp_calc_cl.cpp:

This module is used for parallel evaluations of yexp of the hyperspectral 
image. It invokes OpenCL to perform the kernel in yexp_calc.cl in parallel.

------------------------------------------------------------------------------










