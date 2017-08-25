#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_audio_decode_test");

static AVCodecContext arg_codec_context;
static ffmpeg_decoder arg_decoder;
static AVFrame arg_wframe, arg_rframe;
static int arg_align;
static void *arg_arg;
static io_stream io_s;

mock_function_4(int, ffmpeg_decode_action, ffmpeg_decoder *, ffmpeg_frame *, void *, io_stream *);

BEFORE_EACH() {
	init_mock_function(ffmpeg_decode_action, NULL);
	init_mock_function(avcodec_receive_frame, NULL);
	init_mock_function(av_samples_copy, NULL);
	arg_codec_context.codec_type = AVMEDIA_TYPE_AUDIO;
	arg_decoder.codec_context = &arg_codec_context;
	arg_decoder.rframe = &arg_rframe;
	arg_decoder.wframe = &arg_wframe;
	arg_wframe.channels = 8;
	arg_wframe.format = 2;
	return 0;
}

SUBJECT(int) {
	return ffmpeg_decode(&arg_decoder, arg_align, arg_arg, ffmpeg_decode_action, &io_s);
}

SUITE_CASE("cache audio data from wframe to rframe") {
	arg_decoder.samples_size = 100;
	arg_wframe.nb_samples = 50; 
	arg_rframe.nb_samples = 10;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(avcodec_receive_frame);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_receive_frame, 1, &arg_codec_context);
	CUE_EXPECT_CALLED_WITH_PTR(avcodec_receive_frame, 2, &arg_wframe);

	CUE_EXPECT_NEVER_CALLED(ffmpeg_decode_action);

	CUE_EXPECT_CALLED_ONCE(av_samples_copy);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_copy, 1, arg_rframe.data);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_copy, 2, arg_wframe.data);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 3, 10);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 4, 0);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 5, 50);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 6, 8);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 7, 2);

	CUE_ASSERT_EQ(arg_rframe.nb_samples, 60);
}

static int ffmpeg_decode_action_assert(ffmpeg_decoder *decoder, ffmpeg_frame *frame, void *arg, io_stream *io_s) {
	CUE_ASSERT_EQ(frame->frame->nb_samples, 60);
	return 0;
}

SUITE_CASE("drain rframe when rframe full") {
	arg_decoder.samples_size = 100;
	arg_wframe.nb_samples = 50; 
	arg_rframe.nb_samples = 60;
	init_mock_function(ffmpeg_decode_action, ffmpeg_decode_action_assert);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(ffmpeg_decode_action);
	CUE_EXPECT_CALLED_WITH_PTR(ffmpeg_decode_action, 1, &arg_decoder);
	CUE_EXPECT_CALLED_WITH_PTR(ffmpeg_decode_action, 3, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(ffmpeg_decode_action, 4, &io_s);

	CUE_EXPECT_CALLED_ONCE(av_samples_copy);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_copy, 1, arg_rframe.data);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_copy, 2, arg_wframe.data);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 3, 0);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 4, 0);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 5, 50);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 6, 8);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_copy, 7, 2);

	CUE_ASSERT_EQ(arg_rframe.nb_samples, 50);
}

SUITE_CASE("stream to the end") {
}

SUITE_CASE("audio data too large") {
}

SUITE_END(ffmpeg_audio_decode_test);
