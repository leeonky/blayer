#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_stream_test");

static ffmpeg ffp; 
static AVFormatContext format_context;
static AVStream streams[2];
static AVStream* stream_refs[2] = {&streams[0], &streams[1]};
static AVCodecParameters codec_parameters[2];

static int int_arg = 0;
static int track = -1;
static int (*test_main)(ffmpeg_stream *, void *, io_stream *);

static int default_process(ffmpeg_stream *stream, void *arg, io_stream *io_s) {
	int_arg = 100;
	return 0;
}

static const char *stub_av_get_media_type_string(enum AVMediaType type) {
	switch(type) {
		case AVMEDIA_TYPE_VIDEO:
			return "video";
		case AVMEDIA_TYPE_AUDIO:
			return "audio";
	}
	return "unkown";
}

BEFORE_EACH() {
	int_arg = 0;
	track = -1;
	test_main = default_process;

	ffp.format_context = &format_context;
	ffp.format_context->nb_streams = 2;
	ffp.format_context->streams = stream_refs;

	streams[0].codecpar = &codec_parameters[0];
	streams[1].codecpar = &codec_parameters[1];

	codec_parameters[0].codec_type = AVMEDIA_TYPE_VIDEO;
	codec_parameters[1].codec_type = AVMEDIA_TYPE_VIDEO;

	init_subject("");
	init_mock_function(av_get_media_type_string, stub_av_get_media_type_string);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return ffmpeg_find_stream(&ffp, AVMEDIA_TYPE_VIDEO, track, &int_arg, test_main, &io_s);
}

static int assert_process(ffmpeg_stream *stream, void *arg, io_stream *io_s) {
	int_arg = 200;
	CUE_ASSERT_PTR_EQ(stream->stream, &streams[0]);
	return 0;
}

SUITE_CASE("should get stream info by first track of type") {
	test_main = assert_process;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_EQ(int_arg, 200);
}

static int assert_process2(ffmpeg_stream *stream, void *arg, io_stream *io_s) {
	int_arg = 200;
	CUE_ASSERT_PTR_EQ(stream->stream, &streams[1]);
	return 0;
}

SUITE_CASE("should get stream by specific track") {
	track = 1;
	test_main = assert_process2;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_EQ(int_arg, 200);
}

SUITE_CASE("no matched stream") {
	track = 2;
	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: video stream 2 doesn't exist");
}

SUITE_END(ffmpeg_stream_test);