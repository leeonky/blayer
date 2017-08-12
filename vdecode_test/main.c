#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(test_vdecode_args);
	ADD_SUITE(test_vdecode_main);

	return run_test();
}
