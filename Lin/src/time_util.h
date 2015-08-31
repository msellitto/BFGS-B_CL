#ifndef TIME_UTIL_H
#define TIME_UTIl_H

#ifdef _WIN32
//Windows
#include <Winsock2.h>
#include "gettimeofday.h"

#else
//Linux
#include <sys/time.h>
#include <unistd.h>
#endif

// calulate the time in ms between times t1 and t2
double calc_time(struct timeval *t1, struct timeval *t2); 


#endif
