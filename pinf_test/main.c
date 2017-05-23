#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "pinf/pinf.h"

SUITE_START("pinf test");

BEFORE_ALL() {
	return init_subject("Hello world!\nEXIT\ndump");
}

AFTER_ALL() {
	return close_subject();
}

SUITE_CASE("should pass through line and exit when got EXIT") {
	CU_ASSERT_EQUAL(pinf_main(actxt.input_stream, actxt.output_stream, actxt.error_stream), 0);
	CU_ASSERT_STRING_EQUAL(std_out, "Hello world!\n")
}

SUITE_END(pinf_test);

int main() {
	init_test();

	ADD_SUITE(pinf_test);

	return run_test();
}
