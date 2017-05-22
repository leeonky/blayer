#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "snubber/snubber.h"

SUITE_START("snubber test");

FILE *input_stdin, *input_stdout, *input_stderr;

int pinf_main(FILE *stdin, FILE *stdout, FILE *stderr) {
	input_stdin = stdin;
	input_stdout = stdout;
	input_stderr = stderr;
	return 100;
}

SUITE_CASE("test invake pinf main") {
	init_subject("Hello world!\nEXIT\ndump");

	CU_ASSERT_EQUAL(invoke_subject(snubber_main), 100);
	CU_ASSERT_PTR_EQUAL(input_stdin, actxt.input_stream);
	CU_ASSERT_PTR_EQUAL(input_stdout, actxt.output_stream);
	CU_ASSERT_PTR_EQUAL(input_stderr, actxt.error_stream);

	close_subject();
}

SUITE_END(snubber);

int main() {
	init_test();

	ADD_SUITE(snubber);

	return run_test();
}
