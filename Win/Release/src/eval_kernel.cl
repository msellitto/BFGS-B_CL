#include "hyperspect_constants.h"

#pragma OPENCL EXTENSION cl_khr_fp64: enable


#define inv_cosz 1.01373094981255
#define inv_cosv 1.0


// hyperspectal objective function
double
obj_fun(double P, double G, double BP, double B, double H, int rss_offset_index,
     __global double *rss_calc_device,
     __global double *image_device,
     __constant double *spectral_input,
     __constant double *powf_spectral_43,
     __global double *yexp_device
     ) 
{
   double sum1 = 0; double sum2 = 0; 
 //  double sum3 = 0; double sum4 = 0;
   double lp = log(P);
 //  double inv_cosz = 1.0 / cos(zenith*PI/180.0);
 //  double inv_cosv = 1.0 / cos(view * PI/180.0);
 // double inv_cosz = 1.01373094981255;
 //  double inv_cosv = 1.0;

   double yexp = yexp_device[rss_offset_index / total_bands];

   for(int j = 0; j < total_bands; j++) {
   double at, bb, u, karpa, duc, dub, rss_c, rss_b;
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

      rss_calc_device[rss_offset_index + j] =
                     (0.5*(rss_c+rss_b))/(1.0-1.5*(rss_c+rss_b)); //correct
   }

   //sum3 = 0;
   //sum4 = 0;
   for (int i= b400_id; i <= b675_id; ++i) {         // (Meas-est)^2
		sum1 += (rss_calc_device[rss_offset_index + i] -
			     image_device[rss_offset_index + i]) *
                (rss_calc_device[rss_offset_index + i] -
			     image_device[rss_offset_index + i]);


        sum2 += image_device[rss_offset_index + i] * // Meas^2
                image_device[rss_offset_index + i];
   }

  // 3 4
   for (int i= b720_id; i <= b800_id; ++i) {         // (Meas-est)^2
        sum1 += (image_device[rss_offset_index + i] -
                rss_calc_device[rss_offset_index + i]) *
                (image_device[rss_offset_index + i] -
                rss_calc_device[rss_offset_index + i]);

        sum2 += image_device[rss_offset_index + i] *  // Meas^2
                image_device[rss_offset_index + i];
   }


   return sqrt((sum1)/(sum2));                 //correct
  //return sqrt((sum1+sum3)/(sum2+sum4));                 //correct
	//err = sqrt(sum1+sum3)/sqrt(sum2+sum4);                 //correct

}


#define h 1e-8


__kernel void
coarse_grained_search(int num_inits,
                    __constant double *inits, 
                    __global double *ret,
                    __global double *rss_calc_device,
                    __global double *image_device,
                    __constant double *spectral_input,
                    __constant double *powf_spectral_43,
                    __global double *yexp_device
                    )
{
   int thread_id = get_global_id(0);
   int rss_offset_idx = thread_id * total_bands;

   double min_err = 10000;
   int min_init_num;

   for(int i = 0; i < num_inits; i++)
   {
     double P, G, BP, B, H;
     int idx = i * 5;

     P = inits[idx];
     G = inits[idx+1];
     BP = inits[idx+2];
     B = inits[idx+3];
     H = inits[idx+4];

     double err = obj_fun(P, G, BP, B, H, rss_offset_idx, rss_calc_device, image_device, spectral_input, powf_spectral_43, yexp_device);

      if(err < min_err)
      {
         min_err = err;
         min_init_num = i;
      }
   }

   int init_idx = thread_id * 5;
   for(int i = 0; i < 5; i++)
   { 
     ret[init_idx+i] = inits[min_init_num*5+i];
   }



}



__kernel void
eval_kernel(
    __global int *active_mask,
    __global double *F,
    __global double *x,
    __global double *g,
    __global double *rss_calc_device,
    __global double *image_device,
    __constant double *spectral_input,
    __constant double *powf_spectral_43,
    __global double *yexp_device
    )

{
  int thread_id = get_global_id(0);
  if(active_mask[thread_id] == 0) return;
  int xidx = thread_id * 5;  
  double P, G, BP, B, H;
  double f;
  
  P = x[xidx];
  G = x[xidx+1];
  BP = x[xidx+2];
  B = x[xidx+3];
  H = x[xidx+4];

  f = obj_fun(P, G, BP, B, H, thread_id * total_bands, rss_calc_device, image_device, spectral_input, powf_spectral_43, yexp_device);
  F[thread_id] = f; 
 
  // calculate gradient with forward method
  g[xidx] =   (obj_fun(P+h, G, BP, B, H, thread_id * total_bands, rss_calc_device, image_device, spectral_input, powf_spectral_43, yexp_device) - f ) / h;
  g[xidx+1] = (obj_fun(P, G+h, BP, B, H, thread_id * total_bands, rss_calc_device, image_device, spectral_input, powf_spectral_43, yexp_device) - f ) / h;
  g[xidx+2] = (obj_fun(P, G, BP+h, B, H, thread_id * total_bands, rss_calc_device, image_device, spectral_input, powf_spectral_43, yexp_device) - f ) / h;
  g[xidx+3] = (obj_fun(P, G, BP, B+h, H, thread_id * total_bands, rss_calc_device, image_device, spectral_input, powf_spectral_43, yexp_device) - f ) / h;
  g[xidx+4] = (obj_fun(P, G, BP, B, H+h, thread_id * total_bands, rss_calc_device, image_device, spectral_input, powf_spectral_43, yexp_device) - f ) / h;
}

