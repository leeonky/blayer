#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "pinf/pinf.h"

static void test_pass_through_and_exit() {
	init_subject("Hello world!\nEXIT\ndump");

	CU_ASSERT_EQUAL(pinf_main(actxt.input_stream, actxt.output_stream, actxt.error_stream), 0);
	CU_ASSERT_STRING_EQUAL(std_out, "Hello world!\n")

	close_subject();
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("pinf test", NULL, NULL);
	add_case(suite, test_pass_through_and_exit);

	return run_test();
}
