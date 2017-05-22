#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "snubber/snubber.h"

FILE *input_stdin, *input_stdout, *input_stderr;

int pinf_main(FILE *stdin, FILE *stdout, FILE *stderr) {
	input_stdin = stdin;
	input_stdout = stdout;
	input_stderr = stderr;
	return 100;
}

static void test_invoke_pinf_main() {
	init_subject("Hello world!\nEXIT\ndump");

	CU_ASSERT_EQUAL(invoke_subject(snubber_main), 100);
	CU_ASSERT_PTR_EQUAL(input_stdin, actxt.input_stream);
	CU_ASSERT_PTR_EQUAL(input_stdout, actxt.output_stream);
	CU_ASSERT_PTR_EQUAL(input_stderr, actxt.error_stream);

	close_subject();
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("snubber test", NULL, NULL);
	add_case(suite, test_invoke_pinf_main);

	return run_test();
}
