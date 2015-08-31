#ifndef YEXP_CALC_CL_H
#define YEXP_CALC_CL_H

#include "hyperspect.h"

// calculate yexp on image hyp_image in OpenCL on the GPU
// the file yexpCalcSrcFileNameFull should conain the kernel to calculate yexp
// any additional OpenCL include files should be placed in the diretory OpenCL_incDir
void yexp_calc_cl(hyperspect *hyp_image, const char *yexpCalcSrcFileNameFull, const char *OpenCL_incDir);

#endif
