#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_frame_test");

static ffmpeg_frame frame;
static ffmpeg_decoder decoder;
static AVCodecContext codec_context;
static AVFrame avframe, tmp_avframe;

static char frame_buffer[100];

BEFORE_EACH() {
	frame.decoder = &decoder;
	decoder.codec_context = &codec_context;
	decoder.frame = &avframe;
	decoder.tmp_frame = &tmp_avframe;

	codec_context.codec_type = AVMEDIA_TYPE_VIDEO;
	codec_context.width = 1080;
	codec_context.height = 640;
	codec_context.pix_fmt = AV_PIX_FMT_NV12;

	init_subject("");

	init_mock_function(av_image_fill_arrays, NULL);
	init_mock_function(av_frame_copy, NULL);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return ffmpeg_frame_copy(&frame, frame_buffer, &io_s);
}

SUITE_CASE("ffmpeg_frame_copy") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_image_fill_arrays);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 1, tmp_avframe.data);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 2, tmp_avframe.linesize);
	CUE_EXPECT_CALLED_WITH_PTR(av_image_fill_arrays, 3, frame_buffer);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 4, codec_context.pix_fmt);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 5, codec_context.width);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 6, codec_context.height);
	CUE_EXPECT_CALLED_WITH_INT(av_image_fill_arrays, 7, 1);

	CUE_EXPECT_CALLED_ONCE(av_frame_copy);

	CUE_EXPECT_CALLED_WITH_PTR(av_frame_copy, 1, &tmp_avframe);
	CUE_EXPECT_CALLED_WITH_PTR(av_frame_copy, 2, &avframe);
}

static int stub_av_image_fill_arrays_error(uint8_t **d, int *i, const uint8_t *b, enum AVPixelFormat f, int w, int h, int a) {
	return -10;
}

SUITE_CASE("failed to fill buffer") {
	init_mock_function(av_image_fill_arrays, stub_av_image_fill_arrays_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(av_frame_copy);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -10\n");
}

static int stub_av_frame_copy_error(AVFrame *f, const AVFrame *sf) {
	return -2;
}

SUITE_CASE("failed to copy") {
	init_mock_function(av_frame_copy, stub_av_frame_copy_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -2\n");
}

SUITE_END(ffmpeg_frame_test);
