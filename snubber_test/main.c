#include <stdio.h>
#include <stdlib.h>
#include "testutil/testutil.h"
#include "snubber/snubber.h"

FILE *input_stdin, *input_stdout, *input_stderr;

int pinf_main(FILE *stdin, FILE *stdout, FILE *stderr) {
	input_stdin = stdin;
	input_stdout = stdout;
	input_stderr = stderr;
	return 100;
}

static void test_invoke_pinf_main() {
	app_context ctxt;
	init_app_context(&ctxt, "Hello world!\nEXIT\ndump");
	set_main_args("");

	CU_ASSERT_EQUAL(invoke_main(&ctxt, snubber_main), 100);
	CU_ASSERT_EQUAL(input_stdin, ctxt.input_stream);
	CU_ASSERT_EQUAL(input_stdout, ctxt.output_stream);
	CU_ASSERT_EQUAL(input_stderr, ctxt.error_stream);

	close_app_context(&ctxt);
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("snubber test", NULL, NULL);
	add_case(suite, test_invoke_pinf_main);

	return run_test();
}
