#include <sys/time.h>
#include "usectime.h"

int64_t usectime() {
	int64_t us = 0;
	struct timeval tv;
	if(!gettimeofday(&tv, 0)) {
		us = tv.tv_sec;
		us = us*1000000 + tv.tv_usec;
	}
	return us;
}

