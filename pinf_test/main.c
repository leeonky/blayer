#include <stdio.h>
#include <stdlib.h>
#include "testutil/testutil.h"
#include "pinf/pinf.h"

static void test_pass_through_and_exit() {
	app_context ctxt;
	init_app_context(&ctxt, "Hello world!\nEXIT\ndump");

	CU_ASSERT_EQUAL(pinf_main(ctxt.input_stream, ctxt.output_stream, ctxt.error_stream), 0);

	CU_ASSERT_STRING_EQUAL(output_buffer(&ctxt), "Hello world!\n")

	close_app_context(&ctxt);
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("pinf test", NULL, NULL);
	add_case(suite, test_pass_through_and_exit);

	return run_test();
}
