#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "vdecode/vdecode.h"

static AVFormatContext format_context;

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

BEFORE_EACH() {
	init_subject("", "-v", "0", "test.avi");
	format_context.nb_streams = 0;

	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, stub_avformat_open_input);
	init_mock_function(avformat_find_stream_info, stub_stream_avformat_find_stream_info);
	init_mock_function(avformat_close_input, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return invoke_subject(vdecode_main);
}

SUITE_CASE("should called av* method when open video file and set video track") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_register_all);

	CUE_EXPECT_CALLED_ONCE(avformat_open_input);
	CUE_EXPECT_CALLED_WITH_STRING(avformat_open_input, 2, "test.avi");

	CUE_EXPECT_CALLED_ONCE(avformat_find_stream_info);
	CUE_EXPECT_CALLED_WITH_PTR(avformat_find_stream_info, 1, &format_context);

	CUE_EXPECT_CALLED_ONCE(avformat_close_input);
}

SUITE_CASE("integration test for missing video file argument") {
	init_subject("");

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);
	CUE_ASSERT_STDERR_EQ("Error[vdecode]: require video file\n");
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
	init_mock_function(avformat_find_stream_info, audio_stream_avformat_find_stream_info);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);
	CUE_ASSERT_STDERR_EQ( "Error[vdecode]: No video stream at 1\n");
	CUE_EXPECT_CALLED_ONCE(avformat_close_input);
}

SUITE_END(test_vdecode_main)
