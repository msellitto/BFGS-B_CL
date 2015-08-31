#ifndef HYPERSPECT2_H
#define HYPERSPECT2_H


// this object encapsulates a hyperspectral image for use with hyperspect bfgsb CL
class hyperspect {

public:

  // create an image from a file of size num_image_rows * num_image_cols
  hyperspect(char *imageFullFilename, int num_image_rows, int num_image_cols, char *spectInpFullFilename, bool calc_yexp);

  ~hyperspect();
  
  // evaluate objective function f(P, G, BP, B, H) on image element rss_offset_index
  double obj_fun(double P, double G, double BP, double B, double H, int rss_offset_index);  
  
  // returns internal data of the image
  void image_get_data(double **image_ret, double **spectral_input_ret, double **powf_spectral_43_ret, double **yexp_ret);
  
  // returns size of image (rows * cols)
  void image_get_size(int *cols_rows_ret);	

  // return data for using with calculating yexp
  void yexp_get_data(
      double **band440_ret, 
      double **band440p1_ret, 
      double **band490_ret, 
      double **band490p1_ret, 
      double **yexp_ret, 
      double *wlb440_ret, 
      double *wlb440p1_ret, 
      double *wlb490_ret, 
      double *wlb490p1_ret);

  // cleanup image
  void image_cleanup();	


private:

  // internal image data

  double *image_device;
  double *spectral_input;
  double *powf_spectral_43;
  double *rss_calc_device;
  double *yexp_device;

  double *band440;
  double *band440p1;
  double *band490;
  double *band490p1;

  double wlb440;
  double wlb490;
  double wlb440p1;
  double wlb490p1;

  int num_image_cols;
  int num_image_rows;
  int cols_rows;

  // calculate yexp of the image on the CPU
  void yexp_calc();

};


#endif

