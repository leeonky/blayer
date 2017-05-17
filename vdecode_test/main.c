#include <stdio.h>
#include <stdlib.h>
#include <libavformat/avformat.h>
#include "test.h"
#include "vdecode/vdecode.h"

mock_void_function_0(av_register_all);
mock_function_4(int, avformat_open_input, AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);

static int main_argc;
static char **main_args;

static int init_suite() {
	static char *args[] = { "test.avi" };
	main_args = args;
	main_argc = sizeof(args)/sizeof(args[0]);
	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, NULL);
	return 0;
}

static void open_stream_with_file() {
	vdecode_main(main_argc, main_args);

	CU_ASSERT_EQUAL(called_times_of(av_register_all), 1);
	CU_ASSERT_EQUAL(called_times_of(avformat_open_input), 1);
	CU_ASSERT_STRING_EQUAL(avformat_open_input_p2, "test.avi");
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("vdecode test", init_suite, NULL);
	add_case(suite, open_stream_with_file);

	return run_test();
}
