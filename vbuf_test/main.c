#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(vbuf_arg_test);

	ADD_SUITE(vbuf_test);

	return run_test();
}

