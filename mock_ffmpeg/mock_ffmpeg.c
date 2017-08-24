#include "mock_ffmpeg.h"

mock_void_function_0(av_register_all);
mock_function_4(int, avformat_open_input, AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
mock_function_2(int, avformat_find_stream_info, AVFormatContext *, AVDictionary **);
mock_function_1(AVCodec *, avcodec_find_decoder, enum AVCodecID);
mock_function_3(int, avcodec_open2, AVCodecContext *, const AVCodec *, AVDictionary **);
mock_function_1(int, avcodec_close, AVCodecContext *);
mock_function_1(AVCodecContext *, avcodec_alloc_context3, const AVCodec *);
mock_function_2(int, avcodec_parameters_to_context, AVCodecContext *, const AVCodecParameters *);
mock_function_0(AVFrame *, av_frame_alloc);
mock_function_2(int, av_read_frame, AVFormatContext *, AVPacket *);
mock_function_2(int, avcodec_send_packet, AVCodecContext *, const AVPacket *);
mock_function_2(int, avcodec_receive_frame, AVCodecContext *, AVFrame *);
mock_function_1(int64_t, av_frame_get_best_effort_timestamp, const AVFrame *);
mock_void_function_1(av_init_packet, AVPacket *);
mock_void_function_1(av_packet_unref, AVPacket *);
mock_void_function_1(avcodec_free_context, AVCodecContext **);
mock_void_function_1(av_frame_free, AVFrame **);
mock_void_function_1(avformat_close_input, AVFormatContext **);
mock_function_1(const char *, av_get_media_type_string, enum AVMediaType);
mock_function_4(int, av_image_get_buffer_size, enum AVPixelFormat, int, int, int);
mock_function_7(int, av_image_fill_arrays, uint8_t **, int *, const uint8_t *, enum AVPixelFormat, int, int, int);
mock_function_3(int64_t, av_rescale_q, int64_t, AVRational, AVRational);
mock_function_8(int, av_image_copy_to_buffer, uint8_t *, int, const uint8_t * const *, const int *, enum AVPixelFormat, int, int, int)

mock_function_5(int, av_samples_get_buffer_size, int *, int, int, enum AVSampleFormat, int);

int av_strerror(int errnum, char *errbuf, size_t errbuf_size) {
	snprintf(errbuf, errbuf_size, "%d", errnum);
	return 0;
}
