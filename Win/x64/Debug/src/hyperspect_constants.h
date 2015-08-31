#ifndef HYPERSPECT_CONSTANTS_H
#define HYPERSPECT_CONSTANTS_H

// Don't change these.  They are the minimum values for each parameter
#define minP  0.005
#define minG  0.002
#define minBP 0.0001
#define minB  0.01
#define minH  0.2

// Don't change these.  They are the maximum values for each parameter
#define maxP  0.5
#define maxG  3.5
#define maxBP 0.5
#define maxB  1.0
//#define maxH  10.0
#define maxH 20.0

// total hyperspectral bands
#define total_bands 42

//#define zenith 9.441
#define zenith 32.0
#define view 0.0
#define PI 3.141592653589793

// band ids 
#define b400_id 0
#define b675_id 28
#define b720_id 33
#define b800_id 41

#define b440_id 4 
#define b490_id 9


#define yexp_const_val 1.0

//#define total_cols 51
//#define total_rows 256
//#define columns_rows total_cols * total_rows
//#define total_elements columns_rows * total_bands


#endif



