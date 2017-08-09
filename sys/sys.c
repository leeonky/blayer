#include "sys.h"

int fmemprocess(void *buf, size_t size, const char *mode, void *arg, int(*processer)(FILE *, void *)) {
	int res = 0;
	FILE *f = fmemopen(buf, size, mode);
	if(f) {
		res = processer(f, arg);
		fclose(f);
	} else {
		res = -1;
	}
	return res;
}

