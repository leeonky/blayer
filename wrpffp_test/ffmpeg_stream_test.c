#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

static ffmpeg ffp; 
static AVFormatContext format_context;
static AVStream streams[2];
static AVStream* stream_refs[2] = {&streams[0], &streams[1]};
static AVCodecParameters codec_parameters[2];

static int int_arg = 0;
static int track = -1;
static int (*test_main)(ffmpeg_stream *, void *, io_stream *);

SUITE_START("ffmpeg_stream_test");

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
	init_mock_function(av_init_packet, NULL);
	init_mock_function(av_packet_unref, NULL);
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

	CUE_EXPECT_NEVER_CALLED(av_packet_unref);

	CUE_ASSERT_PTR_EQ(stream->format_context, &format_context);
	return 0;
}

SUITE_CASE("should get stream info by first track of type") {
	test_main = assert_process;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_init_packet);

	CUE_EXPECT_CALLED_ONCE(av_packet_unref);

	CUE_ASSERT_PTR_EQ(params_of(av_init_packet, 1), params_of(av_packet_unref, 1));

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

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: video stream 2 doesn't exist\n");
}

SUITE_END(ffmpeg_stream_test);

SUITE_START("ffmpeg_read_test");

static ffmpeg_stream ffst = {};
static int read_times = 0;

BEFORE_EACH() {
	ffst.format_context = &format_context;
	ffst.stream = &streams[0];

	streams[0].index = 1;

	read_times = 0;

	init_subject("");
	init_mock_function(av_read_frame, NULL);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

static int stub_av_read_frame_index_in_sequence(AVFormatContext *format_context, AVPacket *packet) {
	packet->stream_index = read_times++;
	return 0;
}

SUBJECT(int) {
	return ffmpeg_read(&ffst);
}

SUITE_CASE("get a packet from stream") {
	init_mock_function(av_read_frame, stub_av_read_frame_index_in_sequence);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_EQ(ffst.packet.stream_index, 1);
}

SUITE_END(ffmpeg_read_test);

SUITE_START("ffmpeg_read_and_feed_test");

static ffmpeg_decoder decoder;
static AVCodecContext codec_context;
static AVFrame frame;

static int stub_av_read_frame_index_0(AVFormatContext *format_context, AVPacket *packet) {
	packet->stream_index = 0;
	return 0;
}

BEFORE_EACH() {
	ffst.format_context = &format_context;
	ffst.stream = &streams[0];

	streams[0].index = 0;

	decoder.codec_context = &codec_context;
	decoder.frame = &frame;

	init_subject("");
	init_mock_function(av_read_frame, stub_av_read_frame_index_0);
	init_mock_function(avcodec_send_packet, NULL);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUBJECT(int) {
	return ffmpeg_read_and_feed(&ffst, &decoder);
}

SUITE_CASE("read and send data to decoder") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_read_frame);

	CUE_EXPECT_CALLED_ONCE(avcodec_send_packet);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_send_packet, 1, &codec_context);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_send_packet, 2, &ffst.packet);
}

static int stub_av_read_frame_eof(AVFormatContext *format_context, AVPacket *packet) {
	return -1;
}

SUITE_CASE("enter last mode when get to the end of file") {
	init_mock_function(av_read_frame, stub_av_read_frame_eof);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_CALLED_ONCE(av_read_frame);

	CUE_EXPECT_CALLED_ONCE(avcodec_send_packet);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_send_packet, 1, &codec_context);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_send_packet, 2, NULL);
}

SUITE_END(ffmpeg_read_and_feed_test);

SUITE_START("ffmpeg_frame_size_test");

BEFORE_EACH() {
	init_subject("");

	ffst.stream = &streams[0];
	streams[0].codecpar = &codec_parameters[0];
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

static int stub_av_image_get_buffer_size(enum AVPixelFormat format, int width, int height, int align) {
	return 100;
}

SUITE_CASE("get frame buffer size for video") {
	codec_parameters[0].codec_type = AVMEDIA_TYPE_VIDEO;
	codec_parameters[0].format = AV_PIX_FMT_YUVA420P10BE;
	codec_parameters[0].width = 1920;
	codec_parameters[0].height = 1080;

	init_mock_function(av_image_get_buffer_size, stub_av_image_get_buffer_size);

	CUE_ASSERT_EQ(ffmpeg_frame_size(&ffst), 100);

	CUE_EXPECT_CALLED_ONCE(av_image_get_buffer_size);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 1, AV_PIX_FMT_YUVA420P10BE);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 2, 1920);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 3, 1080);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 4, 1);
}

SUITE_END(ffmpeg_frame_size_test);
