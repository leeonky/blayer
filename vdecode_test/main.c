#include <stdio.h>
#include <stdlib.h>
#include <libavformat/avformat.h>
#include "test.h"
#include "vdecode/vdecode.h"

mock_void_function_0(av_register_all);
mock_function_4(int, avformat_open_input, AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
mock_function_2(int, avformat_find_stream_info, AVFormatContext *, AVDictionary **);
mock_void_function_1(avformat_close_input, AVFormatContext **);

static int main_argc;
static char **main_args;
char input_buffer[1024];
char output_buffer[1024];
char error_buffer[1024];
FILE *input_stream;
FILE *output_stream;
FILE *error_stream;

static int init_suite() {
	static char *args[] = { "test.avi" };
	main_args = args;
	main_argc = sizeof(args)/sizeof(args[0]);

	input_stream = fmemopen(input_buffer, sizeof(input_buffer), "r");
	output_stream = fmemopen(output_buffer, sizeof(output_buffer), "w");
	error_stream = fmemopen(error_buffer, sizeof(error_buffer), "w");

	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, NULL);
	init_mock_function(avformat_close_input, NULL);
	return 0;
}

static int clean_suite() {
	fclose(input_stream);
	fclose(output_stream);
	fclose(error_stream);
	return 0;
}

int fake_avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	*ps = (AVFormatContext*)1;
	return 0;
}

static void open_stream_with_file_and_exit() {
	init_mock_function(avformat_open_input, fake_avformat_open_input);
	vdecode_main(main_argc, main_args);

	CU_EXPECT_CALLED_ONCE(av_register_all);

	CU_EXPECT_CALLED_ONCE(avformat_open_input);
	CU_EXPECT_CALLED_WITH_STRING(avformat_open_input, 2, "test.avi");

	CU_EXPECT_CALLED_ONCE(avformat_find_stream_info);
	CU_EXPECT_CALLED_WITH(avformat_find_stream_info, 1, (AVFormatContext*)1);

	CU_EXPECT_CALLED_ONCE(avformat_close_input);
	CU_EXPECT_CALLED_WITH(avformat_close_input, 1, params_of(avformat_close_input, 1));
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("vdecode test", init_suite, NULL);
	add_case(suite, open_stream_with_file_and_exit);

	return run_test();
}
