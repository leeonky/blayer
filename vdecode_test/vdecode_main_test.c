#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "vdecode/vdecode.h"

static AVFormatContext format_context;

#define subject invoke_subject(vdecode_main)

SUITE_START("vdecode main process");

static int stub_avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	*ps = &format_context;
	return 0;
}

static int stub_stream_avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options) {
	static AVCodecParameters codec_parameters;
	static AVStream streams[1];
	static AVStream* stream_refs[1] = {streams};
	ic->nb_streams = 1;
	ic->streams = stream_refs;

	streams[0].codecpar = &codec_parameters;
	codec_parameters.codec_type = AVMEDIA_TYPE_VIDEO;
	return 0;
}

static void before_case() {
	init_subject("", "-v", "0", "test.avi");
	format_context.nb_streams = 0;

	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, stub_avformat_open_input);
	init_mock_function(avformat_find_stream_info, stub_stream_avformat_find_stream_info);
	init_mock_function(avformat_close_input, NULL);
}

static void after_case() {
	close_subject();
}

SUITE_CASE("should called av* method when open video file and set video track") {
	before_case();

	subject;

	CU_EXPECT_CALLED_ONCE(av_register_all);

	CU_EXPECT_CALLED_ONCE(avformat_open_input);
	CU_EXPECT_CALLED_WITH_STRING(avformat_open_input, 2, "test.avi");

	CU_EXPECT_CALLED_ONCE(avformat_find_stream_info);
	CU_EXPECT_CALLED_WITH(avformat_find_stream_info, 1, &format_context);

	CU_EXPECT_CALLED_ONCE(avformat_close_input);
	CU_EXPECT_CALLED_WITH(avformat_close_input, 1, params_of(avformat_close_input, 1));

	after_case();
}

SUITE_CASE("integration test for missing video file argument") {
	before_case();
	init_subject("");

	CU_ASSERT_EQUAL(subject, -1);
	CU_ASSERT_STRING_EQUAL(std_err, "Error[vdecode]: require video file\n");

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

SUITE_CASE("specific stream should be vedio stream") {
	before_case();
	init_mock_function(avformat_find_stream_info, audio_stream_avformat_find_stream_info);

	CU_ASSERT_EQUAL(subject, -1);
	CU_ASSERT_STRING_EQUAL(std_err, "Error[vdecode]: No video stream at 1\n");
	CU_EXPECT_CALLED_ONCE(avformat_close_input);

	after_case();
}

SUITE_END(test_vdecode_main)
