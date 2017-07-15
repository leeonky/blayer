#ifndef WRPFFP_H
#define WRPFFP_H

#include <stdio.h>
#include "bputil/bputil.h"

typedef struct ffmpeg {
	AVFormatContext *format_context;
} ffmpeg;

typedef struct ffmpeg_stream {
	AVFormatContext *format_context;
	AVStream *stream;
	AVPacket packet;
} ffmpeg_stream;

typedef struct ffmpeg_decoder {
	AVCodecContext *codec_context;
	AVFrame *frame;
} ffmpeg_decoder;

extern int ffmpeg_main(const char *, void *, int(*)(ffmpeg *, void *, io_stream *), io_stream *);

extern int ffmpeg_find_stream(ffmpeg *, enum AVMediaType, int, void *, int(*)(ffmpeg_stream *, void *, io_stream *), io_stream *);

extern int ffmpeg_main_stream(const char *, enum AVMediaType, int, void *, int(*)(ffmpeg_stream *, void *, io_stream *), io_stream *);

extern int ffmpeg_decoding(ffmpeg_stream *, void *, int(*)(ffmpeg_stream *, ffmpeg_decoder *, void *, io_stream *) , io_stream *);

extern int ffmpeg_stream_read(ffmpeg_stream *, io_stream *);

extern int ffmpeg_decoder_frame_size(ffmpeg_decoder *);

extern int ffmpeg_stream_read_and_feed(ffmpeg_stream *, ffmpeg_decoder *, io_stream *);

//extern int ffmpeg_decoder_decode_to(ffmpeg_decoder *, ffmpeg_stream *, void *, io_stream *);

#endif

