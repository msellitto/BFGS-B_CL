#ifndef COARSE_GRAIN_H
#define COARSE_GRAIN_H

// runs a num_points point coarse-grained search on the CPU
// on num_vars-dimenionsal objective function obj_func
// using the points specified in coarse_grain_points 
// returns result in x_ret
void coarse_grain_search(
      int num_vars,
      int num_points,
      double *coarse_grain_points,
      double (*obj_func)(double *, void *), 
      void *aux_func_data,
      double *x_ret
      );



#endif
