#include <assert.h>
#include "vbuf.h"
#include "sys/sys.h"

int main(int argc, char **argv) {
	vbuf_args args;
	int res = process_args(&args, argc, argv, stderr);
	if(!res) {
		io_stream io_s = {stdin, stdout, stderr};
		res =  vbuf_main(args.size, &io_s);
	}
	return res;
}
