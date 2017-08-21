#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_frame_copy_test");

static ffmpeg_frame frame;
static ffmpeg_decoder decoder;
static AVCodecContext codec_context;
static AVFrame avframe, tmp_avframe;

static char frame_buffer[100];

BEFORE_EACH() {
	frame.decoder = &decoder;
	decoder.codec_context = &codec_context;
	decoder.frame = &avframe;

	codec_context.codec_type = AVMEDIA_TYPE_VIDEO;
	avframe.width = 1080;
	avframe.height = 640;
	avframe.format = AV_PIX_FMT_NV12;

	init_subject("");

	init_mock_function(av_image_copy_to_buffer, NULL);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return ffmpeg_frame_copy(&frame, frame_buffer, sizeof(frame_buffer), 10, &io_s);
}

SUITE_CASE("ffmpeg_frame_copy") {
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
	init_mock_function(av_image_copy_to_buffer, stub_av_image_copy_to_buffer_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -10\n");
}

SUITE_END(ffmpeg_frame_copy_test);
