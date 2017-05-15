#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "snubber/snubber.h"

FILE *input_stdin, *input_stdout, *input_stderr;

int pinf_main(FILE *stdin, FILE *stdout, FILE *stderr) {
	input_stdin = stdin;
	input_stdout = stdout;
	input_stderr = stderr;
	return 100;
}


static void test_invoke_pinf_main() {
	char input_buffer[1024] = "Hello world!\nEXIT\ndump";
	char output_buffer[1024] = {};
	char error_buffer[1024] = {};
	FILE *input_stream = fmemopen(input_buffer, sizeof(input_buffer), "r");
	FILE *output_stream = fmemopen(output_buffer, sizeof(output_buffer), "w");
	FILE *error_stream = fmemopen(error_buffer, sizeof(error_buffer), "w");

	CU_ASSERT_EQUAL(snubber_main(0, NULL, input_stream, output_stream, error_stream), 100);
	CU_ASSERT_EQUAL(input_stdin, input_stream);
	CU_ASSERT_EQUAL(input_stdout, output_stream);
	CU_ASSERT_EQUAL(input_stderr, error_stream);
	fclose(input_stream);
	fclose(output_stream);
	fclose(error_stream);
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("snubber test", NULL, NULL);
	add_case(suite, test_invoke_pinf_main);

	return run_test();
}
