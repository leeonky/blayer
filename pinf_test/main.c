#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "pinf/pinf.h"

static void test_pass_through_and_exit() {
	char input_buffer[1024] = "Hello world!\nEXIT\ndump";
	char output_buffer[1024] = {};
	char error_buffer[1024] = {};
	FILE *input_stream = fmemopen(input_buffer, sizeof(input_buffer), "r");
	FILE *output_stream = fmemopen(output_buffer, sizeof(output_buffer), "w");
	FILE *error_stream = fmemopen(error_buffer, sizeof(error_buffer), "w");

	CU_ASSERT_EQUAL(pinf_main(input_stream, output_stream, error_stream), 0);
	fclose(input_stream);
	fclose(output_stream);
	fclose(error_stream);
	CU_ASSERT_STRING_EQUAL(output_buffer, "Hello world!\n")
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("pinf test", NULL, NULL);
	add_case(suite, test_pass_through_and_exit);

	return run_test();
}
