#include "mtime.h"

double d2mce_mtime(){
	double gettime;
	struct timeval now;
	gettimeofday(&now, 0);
	gettime = 1000000*now.tv_sec + now.tv_usec;
	gettime /= 1000000;
	return gettime;
}
