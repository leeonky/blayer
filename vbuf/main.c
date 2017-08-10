#include "vbuf.h"
#include "sys/sys.h"

static int process_input(FILE *stdin, void *arg) {
	io_stream io_s = {stdin, stdout, stderr};
	vbuf_args *args = (vbuf_args *)arg;
	return vbuf_main(args->size, &io_s);
}

int main(int argc, char **argv) {
	vbuf_args args;
	int res = process_args(&args, argc, argv, stderr);
	if(!res) {
		res = fileprocess(args.input, "r", &args, process_input);
	}
	return res;
}
