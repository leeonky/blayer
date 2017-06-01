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

SUITE_START("snubber");

BEFORE_EACH() {
	return init_subject("Hello world!\nEXIT\ndump");
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return invoke_subject(snubber_main);
}

SUITE_CASE("test invake pinf main") {
	CUE_ASSERT_SUBJECT_FAILED_WITH(100);
	CUE_ASSERT_PTR_EQUAL(input_stdin, actxt.input_stream);
	CUE_ASSERT_PTR_EQUAL(input_stdout, actxt.output_stream);
	CUE_ASSERT_PTR_EQUAL(input_stderr, actxt.error_stream);
}

SUITE_END(snubber);

int main() {
	init_test();

	ADD_SUITE(snubber);

	return run_test();
}
