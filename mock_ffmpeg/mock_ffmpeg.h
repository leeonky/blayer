#ifndef MOCK_FFMPEG_
#define MOCK_FFMPEG_

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <cunitexd.h>

extern_mock_void_function_0(av_register_all);
extern_mock_function_4(int, avformat_open_input, AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
extern_mock_function_2(int, avformat_find_stream_info, AVFormatContext *, AVDictionary **);
extern_mock_function_1(AVCodec *, avcodec_find_decoder, enum AVCodecID);
extern_mock_function_3(int, avcodec_open2, AVCodecContext *, const AVCodec *, AVDictionary **);
extern_mock_function_1(int, avcodec_close, AVCodecContext *);
extern_mock_function_1(AVCodecContext *, avcodec_alloc_context3, const AVCodec *);
extern_mock_function_2(int, avcodec_parameters_to_context, AVCodecContext *, const AVCodecParameters *);
extern_mock_function_0(AVFrame *, av_frame_alloc);
extern_mock_function_2(int, av_read_frame, AVFormatContext *, AVPacket *);
extern_mock_function_2(int, avcodec_send_packet, AVCodecContext *, const AVPacket *);
extern_mock_function_2(int, avcodec_receive_frame, AVCodecContext *, AVFrame *);
extern_mock_function_1(int64_t, av_frame_get_best_effort_timestamp, const AVFrame *);
extern_mock_void_function_1(av_init_packet, AVPacket *);
extern_mock_void_function_1(av_packet_unref, AVPacket *);
extern_mock_void_function_1(avcodec_free_context, AVCodecContext **);
extern_mock_void_function_1(av_frame_free, AVFrame **);
extern_mock_void_function_1(avformat_close_input, AVFormatContext **);
extern_mock_function_1(const char *, av_get_media_type_string, enum AVMediaType);
extern_mock_function_4(int, av_image_get_buffer_size, enum AVPixelFormat, int, int, int);
extern_mock_function_7(int, av_image_fill_arrays, uint8_t **, int *, const uint8_t *, enum AVPixelFormat, int, int, int);
extern_mock_function_3(int64_t, av_rescale_q, int64_t, AVRational, AVRational);
extern_mock_function_2(int, av_frame_copy, AVFrame *, const AVFrame *);

#endif
