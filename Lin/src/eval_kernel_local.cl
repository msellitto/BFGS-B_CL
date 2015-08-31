#include "hyperspect_constants.h"

#pragma OPENCL EXTENSION cl_khr_fp64: enable


#define inv_cosz 1.01373094981255
#define inv_cosv 1.0


// hyperspectal objective function
double
obj_fun(double P, double G, double BP, double B, double H, int rss_offset_index,
     __local double *rss_calc_device,
     __local double *image_device,
     __constant double *spectral_input,
     __constant double *powf_spectral_43) 
{
   double err;
   double sum1; double sum2;
   double at, bb, u, karpa, duc, dub, rss_c, rss_b;
   double lp = log(P);
   int rss_off = get_local_id(0) * total_bands;
 //  double inv_cosz = 1.0 / cos(zenith*PI/180.0);
 //  double inv_cosv = 1.0 / cos(view * PI/180.0);
 //  double inv_cosz = 1.01373094981255;
 //  double inv_cosv = 1.0;

   // TODO check to see if cosf is less costly than cos.  If precision is still
   // good, use it instead.

   for(int j = 0; j < total_bands; j++) {
      int spect_offset_index = j*6;

      at = spectral_input[spect_offset_index + 3] + (P *            //correct
          (spectral_input[spect_offset_index + 1] + lp *
           spectral_input[spect_offset_index + 2])) + (G * exp((-0.015) *
          (spectral_input[spect_offset_index] - 440.0)));


      bb = 0.0038 * powf_spectral_43[j] + BP *
           pow((400.0f /spectral_input[spect_offset_index]), yexp); //correct

      u =  bb / (at + bb);             //correct
      karpa = at + bb;                 //correct
      duc = 1.03 * sqrt(1 + 2.4 * u);  //correct 
      dub = 1.04 * sqrt(1 + 5.4 * u);  //correct

      rss_c = (0.084+(0.17*u))*u*(1.0-exp(-karpa*H*((inv_cosz)+  //correct
                 duc*inv_cosv)));

      rss_b = (1.0/PI)*B*spectral_input[spect_offset_index+4] * exp((-karpa) * //correct
            H * ((inv_cosz) + dub*inv_cosv));

      rss_calc_device[rss_off + j] =
                     (0.5*(rss_c+rss_b))/(1.0-1.5*(rss_c+rss_b)); //correct
   }

   // FIXME reduce these to just two variables
   sum1 = 0;
   sum2 = 0;
   //sum3 = 0;
   //sum4 = 0;
   for (int i= b400_id; i <= b675_id; ++i) {         // (Meas-est)^2
		sum1 += (rss_calc_device[rss_off + i] -
			     image_device[rss_off + i]) *
                (rss_calc_device[rss_off + i] -
			     image_device[rss_off + i]);


        sum2 += image_device[rss_off + i] * // Meas^2
                image_device[rss_off + i];
   }

  // 3 4
   for (int i= b720_id; i <= b800_id; ++i) {         // (Meas-est)^2
        sum1 += (image_device[rss_off + i] -
                rss_calc_device[rss_off + i]) *
                (image_device[rss_off + i] -
                rss_calc_device[rss_off + i]);

        sum2 += image_device[rss_off + i] *  // Meas^2
                image_device[rss_off + i];
   }


   err = sqrt((sum1)/(sum2));                 //correct
  // err = sqrt((sum1+sum3)/(sum2+sum4));                 //correct
	//err = sqrt(sum1+sum3)/sqrt(sum2+sum4);                 //correct

   return err;
}




__kernel void
eval_kernel(
    __global double *F,
    __global double *x,
    __global double *rss_calc_device,
    __global double *image_device,
    __constant double *spectral_input,
    __constant double *powf_spectral_43)

{
  int thread_id = get_global_id(0);
  int xidx = thread_id * 5;  
  int local_offset = get_local_id(0) * total_bands;
  int rss_offset_idx = thread_id * total_bands;
  double P, G, BP, B, H;
  __local double rss_calc_local[42*64];
  __local double image_local[42*64];

  for(int i = 0; i < total_bands; i++)
        image_local[local_offset + i] = image_device[rss_offset_idx + i];


  P = x[xidx];
  G = x[xidx+1];
  BP = x[xidx+2];
  B = x[xidx+3];
  H = x[xidx+4];

  for(int i = 0; i < 5; i++)
  F[thread_id] = obj_fun(P, G, BP, B, H, thread_id * total_bands, rss_calc_local, image_local, spectral_input, powf_spectral_43);
  
  

}

