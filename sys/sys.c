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

int fileprocess(const char *name, const char *mode, void *arg, int(*processer)(FILE *, void *)) {
	int res = 0;
	FILE *f = fopen(name, mode);
	if(f) {
		res = processer(f, arg);
		fclose(f);
	} else {
		res = -1;
	}
	return res;
}

typedef struct file_args{
	FILE *f1;
	const char *name2, *mode2;
	void *arg;
	int(*action)(FILE *, FILE *, void *);
} file_args;

static int two_file_opened(FILE *f2, void *arg) {
	file_args *args = (file_args *)arg;
	if(args->action)
		return args->action(args->f1, f2, args->arg);
	return 0;
}

static int one_file_opened(FILE *f1, void *arg) {
	file_args *args = (file_args *)arg;
	args->f1 = f1;
	return fileprocess(args->name2, args->mode2, args, two_file_opened);
}

int mtfileprocess(const char *name1, const char *mode1, const char *name2, const char *mode2, void *arg, int(*processer)(FILE *, FILE *, void *)) {
	file_args args = {
		.name2 = name2,
		.mode2 = mode2,
		.action = processer,
		.arg = arg,
	};
	return fileprocess(name1, mode1, &args, one_file_opened);
}
