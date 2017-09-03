#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "mock_sys/mock_sys.h"
#include "wrpffp/wrpffp.h"
#include "iob/iob.h"
#include "iob/vfs.h"
#include "iob/afs.h"

SUITE_START("ffmpeg_frame_copy_test");

static ffmpeg_frame arg_fframe;
static ffmpeg_decoder decoder;
static AVCodecContext codec_context;
static AVFrame avframe, tmp_avframe;

static char frame_buffer[100];

BEFORE_EACH() {
	arg_fframe.frame = &avframe;
	init_subject("");

	init_mock_function(av_image_copy_to_buffer, NULL);
	init_mock_function(av_samples_fill_arrays, NULL);
	init_mock_function(av_samples_copy, NULL);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return ffmpeg_frame_copy(&arg_fframe, frame_buffer, sizeof(frame_buffer), &io_s);
}

SUITE_CASE("ffmpeg_frame_copy for video") {
	arg_fframe.codec_type = AVMEDIA_TYPE_VIDEO;
	arg_fframe.align = 10;
	avframe.width = 1080;
	avframe.height = 640;
	avframe.format = AV_PIX_FMT_NV12;
	arg_fframe.decoder = &decoder;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_image_copy_to_buffer);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_copy_to_buffer, 1, frame_buffer);
	CUE_EXPECT_CALLED_WITH_INT(av_image_copy_to_buffer, 2, sizeof(frame_buffer));
	CUE_EXPECT_CALLED_WITH_PTR(av_image_copy_to_buffer, 3, avframe.data);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_copy_to_buffer, 4, avframe.linesize);
	CUE_EXPECT_CALLED_WITH_INT(av_image_copy_to_buffer, 5, AV_PIX_FMT_NV12);
	CUE_EXPECT_CALLED_WITH_INT(av_image_copy_to_buffer, 6, 1080);
	CUE_EXPECT_CALLED_WITH_INT(av_image_copy_to_buffer, 7, 640);
	CUE_EXPECT_CALLED_WITH_INT(av_image_copy_to_buffer, 8, 10);
}

static int stub_av_image_copy_to_buffer_error(uint8_t *b, int s, const uint8_t * const * d, const int *l, enum AVPixelFormat f, int w, int h, int a) {
	return -10;
}

SUITE_CASE("failed to copy") {
	arg_fframe.codec_type = AVMEDIA_TYPE_VIDEO;
	init_mock_function(av_image_copy_to_buffer, stub_av_image_copy_to_buffer_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -10\n");
}

static int stub_av_samples_copy_assert(uint8_t **dst, uint8_t * const *src, int dst_of, int src_of, int nb_samples, int nb_channels, enum AVSampleFormat format) {
	CUE_ASSERT_PTR_EQ(dst[0], frame_buffer);
	CUE_ASSERT_PTR_EQ(src[0], avframe.data[0]);
	return 0;
}

static int stub_av_samples_get_buffer_size(int *lines, int channels, int samples, enum AVSampleFormat format, int align) {
	return 1000;
}

SUITE_CASE("copy for audio, depents on decoder samples_size, channels, format") {
	arg_fframe.codec_type = AVMEDIA_TYPE_AUDIO;
	arg_fframe.align = 10;
	avframe.format = AV_SAMPLE_FMT_DBL;
	avframe.channels = 7;
	decoder.samples_size = 128;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_samples_fill_arrays);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_fill_arrays, 2, NULL);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_fill_arrays, 3, frame_buffer);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 4, avframe.channels);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 5, avframe.nb_samples);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 6, avframe.format);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 7, arg_fframe.align);

	CUE_EXPECT_CALLED_ONCE(av_samples_copy);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_copy, 2, avframe.data);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 3, 0);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 4, 0);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 5, avframe.nb_samples);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 6, avframe.channels);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 7, avframe.format);

	CUE_ASSERT_PTR_EQ(params_of(av_samples_fill_arrays, 1), params_of(av_samples_copy, 1));
}

static int stub_av_samples_fill_arrays_failed(uint8_t **b, int *s, const uint8_t *d, int channels, int samples, enum AVSampleFormat format, int align) {
	return -200;
}

SUITE_CASE("failed to fill array") {
	init_mock_function(av_samples_fill_arrays, stub_av_samples_fill_arrays_failed);

	CUE_EXPECT_NEVER_CALLED(av_samples_copy);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -200\n");
}

SUITE_END(ffmpeg_frame_copy_test);

SUITE_START("ffmpeg_create_test");

static AVFrame arg_frame;
static AVFrame *stub_av_frame_alloc() {
	return &arg_frame;
}

static void *arg_arg;
static io_stream arg_io_s;

mock_function_3(int, ffmpeg_create_frame_action, ffmpeg_frame *, void *, io_stream *);

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_arg = &arg_arg;

	init_mock_function(av_frame_alloc, stub_av_frame_alloc);
	init_mock_function(av_frame_free, NULL);
	init_mock_function(ffmpeg_create_frame_action, NULL);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUBJECT(int) {
	return ffmpeg_create_frame(arg_arg, ffmpeg_create_frame_action, &arg_io_s);
}

static int ffmpeg_create_frame_action_assert(ffmpeg_frame *frame, void *arg, io_stream *io_s) {
	CUE_ASSERT_PTR_EQ(frame->frame, &arg_frame);
	return 0;
}

SUITE_CASE("create AVFrame") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_frame_alloc);

	CUE_EXPECT_CALLED_ONCE(ffmpeg_create_frame_action);
	CUE_EXPECT_CALLED_WITH_PTR(ffmpeg_create_frame_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(ffmpeg_create_frame_action, 3, &arg_io_s);

	CUE_EXPECT_CALLED_ONCE(av_frame_free);
}

static AVFrame *stub_av_frame_alloc_error() {
	return NULL;
}

SUITE_CASE("failed to alloc AVFrame") {
	init_mock_function(av_frame_alloc, stub_av_frame_alloc_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(ffmpeg_create_frame_action);

	CUE_EXPECT_NEVER_CALLED(av_frame_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: failed to alloc AVFrame\n");
}

SUITE_END(ffmpeg_create_test);

SUITE_START("ffmpeg_load_image_test");

static uint8_t arg_buffer[128];
static video_frames arg_vfs;

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_fframe.frame = &arg_frame;

	arg_vfs.width = 1920;
	arg_vfs.height = 1080;
	arg_vfs.format = 128;
	arg_vfs.align = 4;

	init_mock_function(av_image_fill_arrays, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return ffmpeg_load_image(&arg_fframe, &arg_vfs, arg_buffer,  &arg_io_s);
}

SUITE_CASE("load image") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_image_fill_arrays);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 1, arg_frame.data);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 2, arg_frame.linesize);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 3, arg_buffer);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 4, arg_vfs.format);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 5, arg_vfs.width);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 6, arg_vfs.height);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 7, arg_vfs.align);

	CUE_ASSERT_EQ(arg_fframe.codec_type, AVMEDIA_TYPE_VIDEO);
	CUE_ASSERT_EQ(arg_fframe.align, 4);
}

static int stub_av_image_fill_arrays_error(uint8_t **b, int *s, const uint8_t *d, enum AVPixelFormat f, int w, int h, int a) {
	return -200;
}

SUITE_CASE("failed to fill") {
	init_mock_function(av_image_fill_arrays, stub_av_image_fill_arrays_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -200\n");
}

SUITE_END(ffmpeg_load_image_test);

SUITE_START("ffmpeg_load_audio_test");

static uint8_t arg_buffer[128];
static audio_frames arg_afs;
static int arg_samples;

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_fframe.frame = &arg_frame;

	arg_afs.sample_rate = 44100;
	arg_afs.channels = 3;
	arg_afs.format = 10;
	arg_afs.align = 1;
	arg_samples = 90;

	init_mock_function(av_samples_fill_arrays, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return ffmpeg_load_audio(&arg_fframe, &arg_afs, arg_samples, arg_buffer,  &arg_io_s);
}

SUITE_CASE("load audio") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_samples_fill_arrays);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_fill_arrays, 1, arg_frame.data);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_fill_arrays, 2, arg_frame.linesize);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_fill_arrays, 3, arg_buffer);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 4, arg_afs.channels);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 5, arg_samples);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 6, arg_afs.format);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_fill_arrays, 7, arg_afs.align);

	CUE_ASSERT_EQ(arg_fframe.codec_type, AVMEDIA_TYPE_AUDIO);
	CUE_ASSERT_EQ(arg_fframe.align, 1);
}

SUITE_CASE("failed to fill") {
	init_mock_function(av_samples_fill_arrays, stub_av_samples_fill_arrays_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -200\n");
}

SUITE_END(ffmpeg_load_audio_test);
