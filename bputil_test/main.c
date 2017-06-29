#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(io_stream_test);
	ADD_SUITE(shm_cbuf_test);

	return run_test();
}
