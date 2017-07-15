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
	return ffmpeg_decoding(&stream, &int_arg, test_main, &io_s);
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

	CUE_EXPECT_CALLED_ONCE(av_frame_alloc);

	CUE_EXPECT_CALLED_ONCE(avcodec_close);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_close, 1, &codec_context);

	CUE_EXPECT_CALLED_ONCE(avcodec_free_context);

	CUE_EXPECT_CALLED_ONCE(av_frame_free);

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

SUITE_START("ffmpeg_decoder_methods_test");

static ffmpeg_decoder decoder;

static int stub_av_image_get_buffer_size(enum AVPixelFormat format, int width, int height, int align) {
	return 100;
}

static AVFrame frame;

BEFORE_EACH() {
	init_subject("");
	decoder.codec_context = &codec_context;
	decoder.frame = &frame;
	codec_context.pix_fmt = AV_PIX_FMT_YUVA420P10BE;
	codec_context.width = 1920;
	codec_context.height = 1080;
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUITE_CASE("get frame buffer size") {
	init_mock_function(av_image_get_buffer_size, stub_av_image_get_buffer_size);

	CUE_ASSERT_EQ(ffmpeg_decoder_frame_size(&decoder), 100);

	CUE_EXPECT_CALLED_ONCE(av_image_get_buffer_size);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 1, AV_PIX_FMT_YUVA420P10BE);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 2, 1920);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 3, 1080);
	CUE_EXPECT_CALLED_WITH_INT(av_image_get_buffer_size, 4, 1);
}

/*static ffmpeg_stream stream;*/

/*static int stub_av_image_fill_arrays() {*/
	/*return 100;*/
/*}*/

/*SUITE_CASE("decode to") {*/
	/*void *p = &p;*/
	/*io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };*/
	/*init_mock_function(av_image_fill_arrays, stub_av_image_fill_arrays);*/
	/*init_mock_function(avcodec_send_packet, NULL);*/
	/*init_mock_function(avcodec_receive_frame, NULL);*/

	/*CUE_ASSERT_EQ(ffmpeg_decoder_decode_to(&decoder, &stream, p, &io_s), 0); */

	/*CUE_EXPECT_CALLED_ONCE(av_image_fill_arrays);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 1, frame.data);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 2, frame.linesize);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 3, p);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 4, AV_PIX_FMT_YUVA420P10BE);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 5, 1920);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 6, 1080);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 7, 1);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_send_packet);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_receive_frame);*/
/*}*/

/*static int stub_av_image_fill_arrays_error() {*/
	/*return -100;*/
/*}*/

/*SUITE_CASE("failed to fill buffer to frame") {*/
	/*io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };*/
	/*init_mock_function(av_image_fill_arrays, stub_av_image_fill_arrays_error);*/
	/*init_mock_function(avcodec_send_packet, NULL);*/
	/*init_mock_function(avcodec_receive_frame, NULL);*/

	/*CUE_ASSERT_EQ(ffmpeg_decoder_decode_to(&decoder, &stream, NULL, &io_s), -1); */

	/*CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -100\n");*/

	/*CUE_EXPECT_NEVER_CALLED(avcodec_send_packet);*/

	/*CUE_EXPECT_NEVER_CALLED(avcodec_receive_frame);*/
/*}*/

SUITE_END(ffmpeg_decoder_methods_test);
