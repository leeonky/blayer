#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_frame_test");

static ffmpeg_frame frame;
static ffmpeg_decoder decoder;
static AVCodecContext codec_context;
static AVFrame avframe;

BEFORE_EACH() {
	frame.decoder = &decoder;
	decoder.codec_context = &codec_context;
	decoder.frame = &avframe;

	codec_context.width = 1080;
	codec_context.height = 640;
	codec_context.pix_fmt = AV_PIX_FMT_NV12;

	return 0;
}

SUITE_CASE("image size") {
	CUE_ASSERT_EQ(ffmpeg_image_width(&frame), 1080);
	CUE_ASSERT_EQ(ffmpeg_image_height(&frame), 640);
	CUE_ASSERT_EQ(ffmpeg_image_pixel_format(&frame), AV_PIX_FMT_NV12);
}

SUITE_END(ffmpeg_frame_test);
