#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(sdl_open_window_test);
	ADD_SUITE(sdl_present_test);

	return run_test();
}
