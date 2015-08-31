#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

using namespace std;

#include "coarse_grain.h"


// carrys out a num_points coarse grain search on obj_func using coarse_grain_points
// returns value in x_ret

void coarse_grain_search(
      int num_vars,
      int num_points,
      double *coarse_grain_points,
      double (*obj_func)(double *, void *), 
      void *aux_func_data,
      double *x_ret
      ) 

{

   double min = 10000;
   int min_point_num;

   for(int i = 0; i < num_points; i++)
   {
      double f = obj_func(coarse_grain_points + (i*num_vars), aux_func_data);
      //printf("%d error = %f\n", i, f);
      if(f < min)
      {
         min = f;
         min_point_num = i;
      }
   }

   //printf("min init num = %d\n", min_point_num);
   memcpy(x_ret, coarse_grain_points+(min_point_num*num_vars), num_vars * sizeof(double));
   //printf("ret 0 = %f\n", ret[0]);


}



