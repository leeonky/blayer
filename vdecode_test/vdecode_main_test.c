#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "vdecode/vdecode.h"


SUITE_START("vdecode main process");


SUBJECT(int) {
	return invoke_subject(vdecode_main);
}

static AVFormatContext format_context;
static int stub_avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	*ps = &format_context;
	return 0;
}

static enum AVCodecID codec_id = 100;
static AVCodecParameters codec_parameters;
static int stub_avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options) {
	static AVStream streams[1];
	static AVStream* stream_refs[1] = {streams};
	ic->nb_streams = 1;
	ic->streams = stream_refs;

	streams[0].codecpar = &codec_parameters;
	streams[0].time_base.num = 1;
	streams[0].time_base.den = 100;
	streams[0].start_time = 1000;

	codec_parameters.codec_type = AVMEDIA_TYPE_VIDEO;
	codec_parameters.codec_id = codec_id;
	return 0;
}

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

static void stub_av_frame_free(AVFrame **frame_ref) {
	CUE_ASSERT_PTR_EQ(*frame_ref, &frame);
}
static void stub_avformat_close_input(AVFormatContext **format_context_ref) {
	CUE_ASSERT_PTR_EQ(*format_context_ref, &format_context);
}

static int stub_av_read_frame_at_the_end(AVFormatContext *format_context, AVPacket * packet) {
	return -1;
}

static int read_frame_times = 0;

BEFORE_EACH() {
	init_subject("", "-v", "0", "test.avi");
	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, stub_avformat_open_input);
	init_mock_function(avformat_find_stream_info, stub_avformat_find_stream_info);
	init_mock_function(avcodec_find_decoder, stub_avcodec_find_decoder);
	init_mock_function(avcodec_alloc_context3, stub_avcodec_alloc_context3);
	init_mock_function(avcodec_parameters_to_context, NULL);
	init_mock_function(avcodec_open2, NULL);
	init_mock_function(av_frame_alloc, stub_av_frame_alloc);
	init_mock_function(av_frame_free, NULL);
	init_mock_function(av_read_frame, NULL);
	init_mock_function(avcodec_close, NULL);
	init_mock_function(avcodec_free_context, NULL);
	init_mock_function(avformat_close_input, NULL);
	read_frame_times = 0;
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

/*SUITE_CASE("should called av* method when open video file and set video track") {*/
	/*init_mock_function(av_read_frame, stub_av_read_frame_at_the_end);*/

	/*CUE_ASSERT_SUBJECT_SUCCEEDED();*/

	/*CUE_EXPECT_CALLED_ONCE(av_register_all);*/

	/*CUE_EXPECT_CALLED_ONCE(avformat_open_input);*/
	/*CUE_EXPECT_CALLED_WITH_STRING(avformat_open_input, 2, "test.avi");*/

	/*CUE_EXPECT_CALLED_ONCE(avformat_find_stream_info);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avformat_find_stream_info, 1, &format_context);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_find_decoder);*/
	/*CUE_EXPECT_CALLED_WITH_INT(avcodec_find_decoder, 1, codec_id);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_alloc_context3);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_alloc_context3, 1, &codec);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_parameters_to_context);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_parameters_to_context, 1, &codec_context);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_parameters_to_context, 2, &codec_parameters);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_open2);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_open2, 1, &codec_context);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_open2, 2, &codec);*/

	/*CUE_EXPECT_CALLED_ONCE(av_frame_alloc);*/

	/*CUE_EXPECT_CALLED_ONCE(av_read_frame);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(av_read_frame, 1, &format_context);*/

	/*CUE_EXPECT_CALLED_ONCE(av_frame_free);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_close);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_close, 1, &codec_context);*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_free_context);*/

	/*CUE_EXPECT_CALLED_ONCE(avformat_close_input);*/
/*}*/

static int stub_av_read_frame_return_an_unexpect_packet(AVFormatContext *format_context, AVPacket * packet) {
	packet->stream_index = 1;
	return -(read_frame_times++);
}

/*SUITE_CASE("skip unexpected stream packet data") {*/
	/*init_mock_function(av_read_frame, stub_av_read_frame_return_an_unexpect_packet);*/
	/*init_mock_function(avcodec_send_packet, NULL);*/
	/*init_mock_function(av_packet_unref, NULL);*/

	/*CUE_ASSERT_SUBJECT_SUCCEEDED();*/

	/*CUE_EXPECT_NEVER_CALLED(avcodec_send_packet);*/

	/*CUE_EXPECT_CALLED_ONCE(av_packet_unref);*/
	/*CUE_ASSERT_PTR_EQ(params_of(av_read_frame, 2), params_of(av_packet_unref, 1));*/

	/*CUE_EXPECT_CALLED_ONCE(av_packet_unref);*/
	/*CUE_ASSERT_PTR_EQ(params_of(av_read_frame, 2), params_of(av_packet_unref, 1));*/
/*}*/

/*static stub_av_read_frame_return_an_expect_packet(AVFormatContext *format_context, AVPacket * packet) {*/
	/*packet->stream_index = 0;*/
	/*return -(read_frame_times++);*/
/*}*/

/*static int stub_avcodec_receive_frame_no_video_frame_return(AVCodecContext *codec_context, AVFrame *frame) {*/
	/*return -1;*/
/*}*/

/*SUITE_CASE("decode expected stream packet data but did not get a frame") {*/
	/*init_mock_function(av_read_frame, stub_av_read_frame_return_an_expect_packet);*/
	/*init_mock_function(avcodec_send_packet, NULL);*/
	/*init_mock_function(avcodec_receive_frame, stub_avcodec_receive_frame_no_video_frame_return);*/
	/*init_mock_function(av_packet_unref, NULL);*/

	/*CUE_ASSERT_SUBJECT_SUCCEEDED();*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_send_packet);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_send_packet, 1, &codec_context);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_send_packet, 2, params_of(av_read_frame, 2));*/

	/*CUE_EXPECT_CALLED_ONCE(avcodec_receive_frame);*/
	/*CUE_EXPECT_CALLED_WITH_PTR(avcodec_receive_frame, 1, &codec_context);*/

	/*CUE_ASSERT_STDOUT_EQ("");*/
/*}*/

/*static int stub_avcodec_receive_frame_got_a_video_frame(AVCodecContext *codec_context, AVFrame *frame) {*/
	/*frame->width = 300;*/
	/*frame->height = 200;*/
	/*frame->format = AV_PIX_FMT_YUV422P;*/
	/*return 0;*/
/*}*/

/*static int stub_av_frame_get_best_effort_timestamp(AVFrame *frame) {*/
	/*return 10000;*/
/*}*/

/*SUITE_CASE("should out put frame info if got a frame in decode") {*/
	/*init_mock_function(av_read_frame, stub_av_read_frame_return_an_expect_packet);*/
	/*init_mock_function(avcodec_send_packet, NULL);*/
	/*init_mock_function(avcodec_receive_frame, stub_avcodec_receive_frame_got_a_video_frame);*/
	/*init_mock_function(av_frame_get_best_effort_timestamp, stub_av_frame_get_best_effort_timestamp);*/
	/*init_mock_function(av_packet_unref, NULL);*/

	/*CUE_ASSERT_SUBJECT_SUCCEEDED();*/

	/*CUE_ASSERT_STDOUT_EQ("video_frame:: width:300 height:200 format:%d pts:90000\n", AV_PIX_FMT_YUV422P);*/
/*}*/

SUITE_END(test_vdecode_main)
