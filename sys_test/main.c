#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(fmemopen_test);
	ADD_SUITE(fopen_test);

	return run_test();
}
