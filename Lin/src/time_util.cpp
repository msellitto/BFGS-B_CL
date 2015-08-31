
// for timing functions
#ifdef _WIN32
//Windows
#include <Winsock2.h>
#include "gettimeofday.h"

#else
//Linux
#include <sys/time.h>
#include <unistd.h>
#endif


#include "time_util.h"


/* calc_time
 *
 * Takes two timeval structs as input and returns the time that has passed in 
 * milliseconds.
 */
double calc_time(struct timeval *t1, struct timeval *t2) {

    long int seconds, useconds;
    double mtime;
    seconds  = t2->tv_sec  - t1->tv_sec;
    useconds = t2->tv_usec - t1->tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    return mtime;
}



