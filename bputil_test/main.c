#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(io_stream_test);
	ADD_SUITE(shm_rbuf_test);

	return run_test();
}
