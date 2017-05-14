#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "snubber/snubber.h"

static void test_pass_through_and_exit() {
	char input_buffer[1024] = "Hello world!\nEXIT";
	char output_buffer[1024] = {};
	char error_buffer[1024] = {};
	FILE *input_stream = fmemopen(input_buffer, sizeof(input_buffer), "r");
	FILE *output_stream = fmemopen(output_stream, sizeof(output_buffer), "w");
	FILE *error_stream = fmemopen(error_buffer, sizeof(error_buffer), "w");

	CU_ASSERT(snubber_main(0, NULL, input_stream, output_stream, error_stream) == 0 );
	CU_ASSERT(strcmp(output_buffer, "Hello world!") == 0 );

	fclose(input_stream);
	fclose(output_stream);
	fclose(error_stream);
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("snubber test", NULL, NULL);
	add_case(suite, test_pass_through_and_exit);

	return run_test();
}
