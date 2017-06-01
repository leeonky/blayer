#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "pinf/pinf.h"

SUITE_START("libpinf");

BEFORE_EACH() {
	return init_subject("Hello world!\nEXIT\ndump");
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return pinf_main(actxt.input_stream, actxt.output_stream, actxt.error_stream);
}

SUITE_CASE("should pass through line and exit when got EXIT") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_STDOUT_EQ("Hello world!\n");
}

SUITE_END(pinf_test);

int main() {
	init_test();

	ADD_SUITE(pinf_test);

	return run_test();
}
