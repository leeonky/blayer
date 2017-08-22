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
	ffmpeg_stream *stream;
	int64_t _pts;
	int64_t _duration;
} ffmpeg_decoder;

typedef struct ffmpeg_frame {
	ffmpeg_decoder *decoder;
	AVFrame *frame;
	enum AVMediaType codec_type;
} ffmpeg_frame;

extern int ffmpeg_open(const char *, void *, int(*)(ffmpeg *, void *, io_stream *), io_stream *);

extern int ffmpeg_find_stream(ffmpeg *, enum AVMediaType, int, void *, int(*)(ffmpeg_stream *, void *, io_stream *), io_stream *);

extern int ffmpeg_open_stream(const char *, enum AVMediaType, int, void *, int(*)(ffmpeg_stream *, void *, io_stream *), io_stream *);

extern int ffmpeg_open_decoder(ffmpeg_stream *, void *, int(*)(ffmpeg_stream *, ffmpeg_decoder *, void *, io_stream *) , io_stream *);

extern int ffmpeg_read(ffmpeg_stream *);

extern int ffmpeg_frame_size(ffmpeg_stream *);

extern int ffmpeg_read_and_feed(ffmpeg_stream *, ffmpeg_decoder *);

extern int ffmpeg_decode(ffmpeg_decoder *, void *, int (*)(ffmpeg_decoder *, ffmpeg_frame *, void *, io_stream *), io_stream *);

int64_t ffmpeg_frame_present_timestamp(ffmpeg_frame *);

extern const char *ffmpeg_video_info(ffmpeg_decoder *);

extern int ffmpeg_frame_copy(ffmpeg_frame *frame, void *, size_t, int, io_stream *);

#endif

