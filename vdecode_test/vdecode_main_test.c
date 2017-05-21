#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "testutil/testutil.h"
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "vdecode/vdecode.h"

static AVFormatContext format_context;
static app_context ctxt;

#define subject invoke_main(&ctxt, vdecode_main)

static int stub_avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	*ps = &format_context;
	return 0;
}

static void before_case() {
	set_main_args("-v", "0", "test.avi", "");
	init_app_context(&ctxt, "");
	format_context.nb_streams = 0;

	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, stub_avformat_open_input);
	init_mock_function(avformat_find_stream_info, NULL);
	init_mock_function(avformat_close_input, NULL);
}

static void after_case() {
	close_app_context(&ctxt);
}

static void miss_video_file() {
	before_case();
	set_main_args("--video", "1", "");

	CU_ASSERT_EQUAL(subject, -1);
	CU_ASSERT_STRING_EQUAL(error_buffer(&ctxt), "Error[vdecode]: require video file\n");

	after_case();
}

static void open_stream_with_file_and_exit() {
	before_case();

	CU_ASSERT_EQUAL(subject, 0);

	CU_EXPECT_CALLED_ONCE(av_register_all);

	CU_EXPECT_CALLED_ONCE(avformat_open_input);
	CU_EXPECT_CALLED_WITH_STRING(avformat_open_input, 2, "test.avi");

	CU_EXPECT_CALLED_ONCE(avformat_find_stream_info);
	CU_EXPECT_CALLED_WITH(avformat_find_stream_info, 1, &format_context);

	CU_EXPECT_CALLED_ONCE(avformat_close_input);
	CU_EXPECT_CALLED_WITH(avformat_close_input, 1, params_of(avformat_close_input, 1));

	CU_ASSERT_STRING_EQUAL(error_buffer(&ctxt), "Warning[vdecode]: no streams in file\n");

	after_case();
}

static int audio_stream_avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options) {
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
	before_case();
	init_mock_function(avformat_find_stream_info, audio_stream_avformat_find_stream_info);

	CU_ASSERT_EQUAL(subject, -1);
	CU_ASSERT_STRING_EQUAL(error_buffer(&ctxt), "Error[vdecode]: No video stream at 1\n");
	CU_EXPECT_CALLED_ONCE(avformat_close_input);

	after_case();
}

void test_vdecode_main() {
	CU_pSuite suite = create_suite("vdecode test", NULL, NULL);
	add_case(suite, miss_video_file);
	add_case(suite, open_stream_with_file_and_exit);
	add_case(suite, set_video_track);
}
