#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <fstream>

using namespace std;

#include "hyperspect_constants.h"
#include "hyperspect.h"


// create an image from a file of size num_image_rows * num_image_cols
// also takes spectral input file
// will also optionally calculate yexp if calc_yexp is set to true
hyperspect::hyperspect(char *imageFullFilename, int num_image_rows, int num_image_cols, char *spectInpFullFilename, bool calc_yexp)
{

   this->num_image_rows = num_image_rows;
   this->num_image_cols = num_image_cols;
   this->cols_rows = num_image_rows * num_image_cols;

   float *image_temp;
   size_t result;
   FILE* pFile;
   pFile = fopen (imageFullFilename, "rb" );

   int num_image_elements = num_image_rows * num_image_cols * total_bands;

	if(pFile == NULL) 
	{
		printf("error opening image file\n");
	}

	long lSize;

	fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);			//size of file in bytes
    rewind (pFile);

	// read image from file
	image_temp = (float*) malloc (lSize);
	result = fread(image_temp,sizeof(float),num_image_elements,pFile);

	if (result != (size_t)num_image_elements) 
	{
		printf("error\n");
		return;
	}

	image_device = (double*) malloc (lSize*2);
    rss_calc_device = (double*) malloc(lSize*2);

	//FILE* imgOutFile; 
	//imgOutFile = fopen ("reals_file_reflectance.txt", "w");
  
	// convert float formatted image to doubles
	for(unsigned int i = 0; i < num_image_elements; i++)
	{
		image_device[i] = image_temp[i];
		//fprintf(imgOutFile, "%.30f\n", image_device[i]);
	}

	spectral_input = (double *) malloc(sizeof(double)*total_bands*6);

	FILE* spectInpFile; 
	spectInpFile = fopen (spectInpFullFilename, "r");

	if(spectInpFile == NULL) 
	{
		printf("error opening specInpFile\n");
	}

	//Read Spectral Input here
    //printf("reading spectral input.. \n");
    for (int i = 0; i < total_bands; ++i) {
        for (int j = 0; j < 6; ++j) {
            if (!feof(spectInpFile)) {
                fscanf(spectInpFile, "%lf", &spectral_input[ i * 6 + j]);
				//printf("spec input = %f\n", spectral_input[i*6+j]);
            }
        }
    }


	fclose(spectInpFile);

	// do some calculations now to save compute time later
	powf_spectral_43 = (double*)malloc(sizeof(double)*total_bands);

	for(int i = 0; i < total_bands; i++) {
      powf_spectral_43[i] = pow((400.0/spectral_input[i*6]),4.3);
   }


  free(image_temp);

  // yexp stuff ----------------------

  yexp_device = (double *) malloc(cols_rows * sizeof(double));

  wlb440 = spectral_input[b440_id * 6];
  wlb490 = spectral_input[b490_id * 6];
  wlb440p1 = spectral_input[b440_id * 6 + 6];
  wlb490p1 = spectral_input[b490_id * 6 + 6];

  band440 = (double *) malloc(cols_rows * sizeof(double));
  band440p1 = (double *) malloc(cols_rows * sizeof(double));
  band490 = (double *) malloc(cols_rows * sizeof(double));
  band490p1 = (double *) malloc(cols_rows * sizeof(double));

  for (int i=0; i < num_image_rows; i++) {
     for (int j=0; j < num_image_cols; j++){
       
        band440[i * num_image_cols + j] = image_device[b440_id + 
           (num_image_cols * total_bands * i) + (total_bands * j)];
    
        band440p1[i * num_image_cols + j] = image_device[(b440_id + 1) +
           (num_image_cols * total_bands * i) + (total_bands * j)]; 
    
        band490[i * num_image_cols + j] = image_device[b490_id + 
           (num_image_cols * total_bands * i) + (total_bands * j)];
    
        band490p1[i * num_image_cols + j] = image_device[(b490_id + 1) +
           (num_image_cols * total_bands * i) + (total_bands * j)];
     }
  }


  // calc yexp here on CPU, if not, it will be set elsewhere externally to the
  // hyperspect class or simply left to a constant value 
  if(calc_yexp)
  {
     yexp_calc();
  }

  else // set but don't calculate yexp, simply set it to a constant value
  {
     for(int i = 0; i < cols_rows; i++)
     {
        yexp_device[i] = yexp_const_val;
     }
  }


  //printf("yexp = %f\n", yexp_device[0]);

}

hyperspect::~hyperspect()
{
   image_cleanup();
}


// cleanup image resources
void hyperspect::image_cleanup()
{
  free(image_device);
  free(spectral_input);
  free(powf_spectral_43);
  free(rss_calc_device);
  free(yexp_device);
  free(band440);
  free(band440p1);
  free(band490);
  free(band490p1);
}

// returns image size (rows * columns)
void hyperspect::image_get_size(int *cols_rows_ret)
{
   *cols_rows_ret = cols_rows;
}


// returns data for use with calculating yexp externally to the image object
void hyperspect::yexp_get_data(
      double **band440_ret, 
      double **band440p1_ret, 
      double **band490_ret, 
      double **band490p1_ret, 
      double **yexp_ret, 
      double *wlb440_ret, 
      double *wlb440p1_ret, 
      double *wlb490_ret, 
      double *wlb490p1_ret)
{
   *band440_ret = band440;
   *band440p1_ret = band440p1;
   *band490_ret = band490;
   *band490p1_ret = band490p1;
   *yexp_ret = yexp_device;
   *wlb440_ret = wlb440;
   *wlb440p1_ret = wlb440p1;
   *wlb490_ret = wlb490;
   *wlb490p1_ret = wlb490p1;
}

// calculate yexp on the CPU for all image elements
void hyperspect::yexp_calc()
{

   // for all image elements
   for(int gid = 0; gid < cols_rows; gid++)
   {

      //-------------------- STEP 3 --------------------------------

      // Calculate yexp;
      //x440=rrs[b440]+(440-wl[b440])*(rrs[b440+1]-rrs[b440])/(wl[b440+1]-wl[b440])
      double x440 = band440[gid] + (440-wlb440) * 
         (band440p1[gid] - band440[gid]) / 
         (wlb440p1 - wlb440);

      //x490=rrs[b490]+(490-wl[b490])*(rrs[b490+1]-rrs[b490])/(wl[b490+1]-wl[b490])
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
 
      yexp_device[gid] = yexp_val;

   }


}


// returns private image data
void hyperspect::image_get_data(double **image_ret, double **spectral_input_ret, double **powf_spectral_43_ret, double **yexp_ret)
{
  if(image_ret != NULL) *image_ret = image_device;
  if(spectral_input_ret != NULL) *spectral_input_ret = spectral_input;
  if(powf_spectral_43 != NULL) *powf_spectral_43_ret = powf_spectral_43;
  if(yexp_ret != NULL) *yexp_ret = yexp_device;
}



// hyperspectal objective function
double 
hyperspect::obj_fun(double P, double G, double BP, double B, double H, int rss_offset_index) 
{
   double err;
   int spect_offset_index;
   double sum1; double sum2;
   double sum3; double sum4;

   double at, bb, u, karpa, duc, dub, rss_c, rss_b;
   double lp = log(P);
   double inv_cosz = 1.0 / cos(zenith*PI/180.0);
   double inv_cosv = 1.0 / cos(view * PI/180.0);
   double yexp = yexp_device[rss_offset_index / total_bands];

   // TODO check to see if cosf is less costly than cos.  If precision is still
   // good, use it instead.

   for(int j = 0; j < total_bands; j++) {
      spect_offset_index = j*6;

      at = spectral_input[spect_offset_index + 3] + (P *            
          (spectral_input[spect_offset_index + 1] + lp *
           spectral_input[spect_offset_index + 2])) + (G * exp((-0.015) *
          (spectral_input[spect_offset_index] - 440.0)));

      bb = 0.0038 * powf_spectral_43[j] + BP *
           pow((400.0f /spectral_input[spect_offset_index]), yexp); 

      u =  bb / (at + bb);            
      karpa = at + bb;                 
      duc = 1.03 * sqrt(1 + 2.4 * u);  
      dub = 1.04 * sqrt(1 + 5.4 * u);  

      rss_c = (0.084+(0.17*u))*u*(1.0-exp(-karpa*H*((inv_cosz)+  
                 duc*inv_cosv)));

      rss_b = (1.0/PI)*B*spectral_input[spect_offset_index+4] * exp((-karpa) * 
            H * ((inv_cosz) + dub*inv_cosv));

      rss_calc_device[rss_offset_index + j] =
                     (0.5*(rss_c+rss_b))/(1.0-1.5*(rss_c+rss_b)); 

   }

   // FIXME reduce these to just two variables
   sum1 = 0;
   sum2 = 0;
   sum3 = 0;
   sum4 = 0;

   for (int i= b400_id; i <= b675_id; ++i) {         // (Meas-est)^2
        /*sum1 += (image_device[rss_offset_index + i] -
                rss_calc_device[rss_offset_index + i]) *
                (image_device[rss_offset_index + i] -
                rss_calc_device[rss_offset_index + i]);*/

		//double temp = (rss_calc_device[i] - image_device[i]) * (rss_calc_device[i] - image_device[i]);


		sum1 += (rss_calc_device[rss_offset_index + i] -
			     image_device[rss_offset_index + i]) *
                (rss_calc_device[rss_offset_index + i] -
			     image_device[rss_offset_index + i]);


        sum2 += image_device[rss_offset_index + i] * // Meas^2
                image_device[rss_offset_index + i];
   }

   for (int i= b720_id; i <= b800_id; ++i) {         // (Meas-est)^2
        sum1 += (image_device[rss_offset_index + i] -
                rss_calc_device[rss_offset_index + i]) *
                (image_device[rss_offset_index + i] -
                rss_calc_device[rss_offset_index + i]);

        sum2 += image_device[rss_offset_index + i] *  // Meas^2
                image_device[rss_offset_index + i];
   }


   err = sqrt((sum1)/(sum2));                 
   // err = sqrt((sum1+sum3)/(sum2+sum4));                 
   //err = sqrt(sum1+sum3)/sqrt(sum2+sum4);                

   return err;
}


