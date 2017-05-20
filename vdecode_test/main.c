#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "mock.h"
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "vdecode/vdecode.h"

static int main_argc;
static char **main_args;
char input_buffer[1024];
char output_buffer[1024];
char error_buffer[1024];
FILE *input_stream;
FILE *output_stream;
FILE *error_stream;
AVFormatContext format_context;

int stub_avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	*ps = &format_context;
	return 0;
}

static void before_case() {
	static char *args[] = { "test.avi" };
	main_args = args;
	main_argc = sizeof(args)/sizeof(args[0]);
	format_context.nb_streams = 0;

	input_stream = fmemopen(input_buffer, sizeof(input_buffer), "r");
	output_stream = fmemopen(output_buffer, sizeof(output_buffer), "w");
	error_stream = fmemopen(error_buffer, sizeof(error_buffer), "w");

	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, stub_avformat_open_input);
	init_mock_function(avformat_close_input, NULL);
}

static void after_case() {
	fclose(input_stream);
	fclose(output_stream);
	fclose(error_stream);
}

static void open_stream_with_file_and_exit() {
	before_case();

	CU_ASSERT_EQUAL(vdecode_main(main_argc, main_args, input_stream, output_stream, error_stream), 0);

	CU_EXPECT_CALLED_ONCE(av_register_all);

	CU_EXPECT_CALLED_ONCE(avformat_open_input);
	CU_EXPECT_CALLED_WITH_STRING(avformat_open_input, 2, "test.avi");

	CU_EXPECT_CALLED_ONCE(avformat_find_stream_info);
	CU_EXPECT_CALLED_WITH(avformat_find_stream_info, 1, &format_context);

	CU_EXPECT_CALLED_ONCE(avformat_close_input);
	CU_EXPECT_CALLED_WITH(avformat_close_input, 1, params_of(avformat_close_input, 1));

	fflush(error_stream);
	CU_ASSERT_STRING_EQUAL(error_buffer, "Warning[vdecode]: no streams in file\n");
	after_case();
}

int audio_stream_avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options) {
	static AVCodecParameters codec_parameters;
	static AVStream streams[1];
	static AVStream* stream_refs[1] = {streams};
	ic->nb_streams = 1;
	ic->streams = stream_refs;

	streams[0].codecpar = &codec_parameters;
	codec_parameters.codec_type = AVMEDIA_TYPE_AUDIO;
	return 0;
}

static void set_video_track() {
	static char *args[] = { "--video", "1", "test.avi" };
	before_case();

	main_args = args;
	main_argc = sizeof(args)/sizeof(args[0]);

	init_mock_function(avformat_find_stream_info, audio_stream_avformat_find_stream_info);

	CU_ASSERT_EQUAL(vdecode_main(main_argc, main_args, input_stream, output_stream, error_stream), -1);
	fflush(error_stream);
	CU_ASSERT_STRING_EQUAL(error_buffer, "Error[vdecode]: No video stream at 1\n");
	CU_EXPECT_CALLED_ONCE(avformat_close_input);

	after_case();
}

/*static void check_opitons() {*/
	/*vdecode_args args = {};*/
/*}*/

/*set video track*/

/*error checking*/
/*default video track*/
/*docode*/
/*out put*/

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("vdecode test", NULL, NULL);
	add_case(suite, open_stream_with_file_and_exit);
	/*add_case(suite, check_opitons);*/
	add_case(suite, set_video_track);

	return run_test();
}
