#include <stdio.h>
#include <stdlib.h>

// For timing functions
#ifdef _WIN32
//Windows
#include <Winsock2.h>
#include "gettimeofday.h"

#else
//Linux
#include <sys/time.h>
#include <unistd.h>
#endif

#include "time_util.h"
#include "hyperspect_bfgsb_cl.h"


int main(int argc, char *argv[])
{
   struct timeval start, end;
   gettimeofday(&start, NULL); 

   hyperspect_bfgsb_cl(argc, argv); 

   gettimeofday(&end, NULL); 
   printf("TOTAL EXECUTION TIME: %f (ms)\n", calc_time(&start, &end));

   return EXIT_SUCCESS;
}



