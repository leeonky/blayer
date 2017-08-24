#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_decoder_test");

static int int_arg = 0;
static int (*test_main)(ffmpeg_stream *, ffmpeg_decoder *decoder, void *, io_stream *);

static int default_process(ffmpeg_stream *stream, ffmpeg_decoder *decoder, void *arg, io_stream *io_s) {
	int_arg = 100;
	return 10;
}

static ffmpeg_stream stream;
static AVCodecParameters codec_parameters;
static AVStream av_stream;

static AVCodec codec;
static AVCodec *stub_avcodec_find_decoder(enum AVCodecID codecID) {
	return &codec;
}

static AVCodecContext codec_context;
static AVCodecContext *stub_avcodec_alloc_context3(const AVCodec *codec) {
	return &codec_context;
}

static AVFrame frame;
static AVFrame *stub_av_frame_alloc() {
	return &frame;
}

BEFORE_EACH() {
	int_arg = 0;
	test_main = default_process;
	init_subject("");
	init_mock_function(avcodec_find_decoder, stub_avcodec_find_decoder);
	init_mock_function(avcodec_alloc_context3, stub_avcodec_alloc_context3);
	init_mock_function(avcodec_parameters_to_context, NULL);
	init_mock_function(avcodec_open2, NULL);
	init_mock_function(avcodec_close, NULL);
	init_mock_function(avcodec_free_context, NULL);
	init_mock_function(av_frame_alloc, stub_av_frame_alloc);
	init_mock_function(av_frame_free, NULL);

	stream.stream = &av_stream;

	av_stream.codecpar = &codec_parameters;
	av_stream.time_base.num = 1;
	av_stream.time_base.den = 100;
	av_stream.start_time = 1000;

	codec_parameters.codec_type = AVMEDIA_TYPE_VIDEO;
	codec_parameters.codec_id = 100;
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return ffmpeg_open_decoder(&stream, &int_arg, test_main, &io_s);
}

SUITE_CASE("should open and close stream's decoder; return process value") {
	CUE_ASSERT_SUBJECT_FAILED_WITH(10);

	CUE_EXPECT_CALLED_ONCE(avcodec_find_decoder);
	CUE_EXPECT_CALLED_WITH_INT(avcodec_find_decoder, 1, 100);

	CUE_EXPECT_CALLED_ONCE(avcodec_alloc_context3);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_alloc_context3, 1, &codec);

	CUE_EXPECT_CALLED_ONCE(avcodec_parameters_to_context);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_parameters_to_context, 1, &codec_context);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_parameters_to_context, 2, &codec_parameters);

	CUE_EXPECT_CALLED_ONCE(avcodec_open2);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_open2, 1, &codec_context);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_open2, 2, &codec);

	CUE_EXPECT_CALLED_TIMES(av_frame_alloc, 1);

	CUE_EXPECT_CALLED_TIMES(av_frame_free, 1);

	CUE_EXPECT_CALLED_ONCE(avcodec_close);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_close, 1, &codec_context);

	CUE_EXPECT_CALLED_ONCE(avcodec_free_context);

	CUE_ASSERT_EQ(int_arg, 100);
}

static AVCodec *stub_avcodec_find_decoder_error(enum AVCodecID id) {
	return NULL;
}

SUITE_CASE("failed to find decoder") {
	init_mock_function(avcodec_find_decoder, stub_avcodec_find_decoder_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(avcodec_alloc_context3);
	CUE_EXPECT_NEVER_CALLED(avcodec_parameters_to_context);
	CUE_EXPECT_NEVER_CALLED(avcodec_open2);
	CUE_EXPECT_NEVER_CALLED(avcodec_close);
	CUE_EXPECT_NEVER_CALLED(avcodec_free_context);
	CUE_EXPECT_NEVER_CALLED(av_frame_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: failed to find decoder\n");
	CUE_ASSERT_EQ(int_arg, 0);
}

static AVCodecContext *stub_avcodec_alloc_context3_error(const AVCodec *codec) {
	return NULL;
}

SUITE_CASE("failed to alloc codec_context") {
	init_mock_function(avcodec_alloc_context3, stub_avcodec_alloc_context3_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(avcodec_parameters_to_context);
	CUE_EXPECT_NEVER_CALLED(avcodec_open2);
	CUE_EXPECT_NEVER_CALLED(avcodec_close);
	CUE_EXPECT_NEVER_CALLED(avcodec_free_context);
	CUE_EXPECT_NEVER_CALLED(av_frame_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: failed to alloc AVCodecContext\n");
	CUE_ASSERT_EQ(int_arg, 0);
}

static int stub_avcodec_parameters_to_context_error(AVCodecContext *codec_context, const AVCodecParameters *p) {
	return -100;
}

SUITE_CASE("failed to avcodec_parameters_to_context") {
	init_mock_function(avcodec_parameters_to_context, stub_avcodec_parameters_to_context_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(avcodec_open2);
	CUE_EXPECT_NEVER_CALLED(avcodec_close);
	CUE_EXPECT_NEVER_CALLED(av_frame_free);

	CUE_EXPECT_CALLED_ONCE(avcodec_free_context);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -100\n");
	CUE_ASSERT_EQ(int_arg, 0);
}

static int stub_avcodec_open2_error(AVCodecContext *codec_context, const AVCodec *codec, AVDictionary **opts) {
	return -200;
}

SUITE_CASE("open decoder failed") {
	init_mock_function(avcodec_open2, stub_avcodec_open2_error)

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(avcodec_close);

	CUE_EXPECT_CALLED_ONCE(avcodec_free_context);
	CUE_EXPECT_NEVER_CALLED(av_frame_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -200\n");
	CUE_ASSERT_EQ(int_arg, 0);
}

static AVFrame *stub_av_frame_alloc_error() {
	return NULL;
}

SUITE_CASE("failed to alloc frame") {
	init_mock_function(av_frame_alloc, stub_av_frame_alloc_error)

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(av_frame_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: failed to alloc AVFrame\n");
	CUE_ASSERT_EQ(int_arg, 0);
}

SUITE_END(ffmpeg_decoder_test);

SUITE_START("ffmpeg_decode_test");

static ffmpeg_decoder decoder;
static int arg_align;

static int stub_avcodec_receive_frame_got_frame(AVCodecContext *codec_context, AVFrame *frame) {
	return 0;
}

BEFORE_EACH() {
	arg_align = 32;
	decoder.codec_context = &codec_context;
	decoder.frame = &frame;

	codec_context.pix_fmt = AV_PIX_FMT_YUVA420P10BE;
	codec_context.width = 1920;
	codec_context.height = 1080;
	codec_context.codec_type = AVMEDIA_TYPE_VIDEO;

	init_mock_function(avcodec_receive_frame, stub_avcodec_receive_frame_got_frame);

	int_arg = 0;

	init_subject("");
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

static int process_frame(ffmpeg_decoder *d, ffmpeg_frame *f, void *arg, io_stream *io_s) {
	*(int *)arg = 100;
	CUE_ASSERT_PTR_EQ(f->frame, d->frame);
	CUE_ASSERT_EQ(f->codec_type, AVMEDIA_TYPE_VIDEO);
	CUE_ASSERT_EQ(f->align, arg_align);
	return 0;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return ffmpeg_decode(&decoder, arg_align, &int_arg, process_frame, &io_s);
}

SUITE_CASE("decode to frame and invoke process") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(avcodec_receive_frame);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_receive_frame, 1, decoder.codec_context);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_receive_frame, 2, decoder.frame);

	CUE_ASSERT_EQ(int_arg, 100);
}

static int stub_avcodec_receive_frame_error(AVCodecContext *codec_context, AVFrame *frame) {
	return -100;
}

SUITE_CASE("no frame to receive") {
	init_mock_function(avcodec_receive_frame, stub_avcodec_receive_frame_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-100);

	CUE_ASSERT_EQ(int_arg, 0);
}

SUITE_END(ffmpeg_decode_test)

SUITE_START("ffmpeg_decoded_size_test");

static int arg_align;

BEFORE_EACH() {
	decoder.codec_context = &codec_context;
	arg_align = 8;
	return 0;
}

static int stub_av_image_get_buffer_size(enum AVPixelFormat format, int width, int height, int align) {
	return 100;
}

SUITE_CASE("get frame buffer size for video") {
	codec_context.codec_type = AVMEDIA_TYPE_VIDEO;
	codec_context.width = 1920;
	codec_context.height = 1080;
	codec_context.pix_fmt = AV_PIX_FMT_YUVA420P10BE;
	init_mock_function(av_image_get_buffer_size, stub_av_image_get_buffer_size);

	CUE_ASSERT_EQ(ffmpeg_decoded_size(&decoder, arg_align), 100);

	CUE_EXPECT_CALLED_ONCE(av_image_get_buffer_size);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 1, AV_PIX_FMT_YUVA420P10BE);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 2, 1920);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 3, 1080);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 4, arg_align);
}

static int stub_av_samples_get_buffer_size(int *lines, int channels, int samples, enum AVSampleFormat format, int align) {
	return 1000;
}

SUITE_CASE("get frame buffer size for audio") {
	codec_context.codec_type = AVMEDIA_TYPE_AUDIO;
	codec_context.channels = 8;
	codec_context.sample_fmt = AV_SAMPLE_FMT_S64;
	codec_context.frame_size = 128;
	init_mock_function(av_samples_get_buffer_size, stub_av_samples_get_buffer_size);

	CUE_ASSERT_EQ(ffmpeg_decoded_size(&decoder, arg_align), 1000);

	CUE_EXPECT_CALLED_ONCE(av_samples_get_buffer_size);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_get_buffer_size, 1, NULL);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 2, 8);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 3, 128);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 4, AV_SAMPLE_FMT_S64);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 5, 1);
}

SUITE_CASE("audio codec_context frame_size not set") {
	codec_context.codec_type = AVMEDIA_TYPE_AUDIO;
	codec_context.channels = 8;
	codec_context.sample_rate = 96000;
	codec_context.sample_fmt = AV_SAMPLE_FMT_S64;
	codec_context.frame_size = 0;

	init_mock_function(av_samples_get_buffer_size, stub_av_samples_get_buffer_size);

	CUE_ASSERT_EQ(ffmpeg_decoded_size(&decoder, arg_align), 1000);

	CUE_EXPECT_CALLED_ONCE(av_samples_get_buffer_size);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_get_buffer_size, 1, NULL);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 2, 8);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 3, 48000);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 4, AV_SAMPLE_FMT_S64);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 5, 1);
}

SUITE_END(ffmpeg_decoded_size_test);
