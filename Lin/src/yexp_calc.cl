

#pragma OPENCL EXTENSION cl_khr_fp64: enable


__kernel void 
yexp_calc(__global double *band440,
        __global double *band440p1, 
        __global double *band490,
        __global double *band490p1,
        __global double *yexp, 
        double wlb440,
        double wlb490, 
        double wlb440p1,
        double wlb490p1)
{
 
   int gid = get_global_id(0);

   //-------------------- STEP 3 --------------------------------

   // Calculate yexp;

   // x440=rrs[b440]+(440-wl[b440])*(rrs[b440+1]-rrs[b440])/(wl[b440+1]-wl[b440])
   
   double x440 = band440[gid] + (440-wlb440) * 
      (band440p1[gid] - band440[gid]) / 
      (wlb440p1 - wlb440);

   // x490=rrs[b490]+(490-wl[b490])*(rrs[b490+1]-rrs[b490])/(wl[b490+1]-wl[b490])
   
   double x490 = band490[gid] + (490-wlb490) * 
      (band490p1[gid] - band490[gid]) / 
      (wlb490p1 - wlb490);

   double yexp_val =  3.44 * (1 - 3.17 * exp((-2.01) * (x440/x490)));
   
   if (yexp_val < 0)  {
      yexp_val = 0.0;
   }
   else if (yexp_val > 2.5) {
      yexp_val = 2.5; 
   }
 
   yexp[gid] = yexp_val;
   //yexp[gid] = 1.0f;
}


