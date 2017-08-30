#ifndef WRPFFP_H
#define WRPFFP_H

#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include "iob/iob.h"
#include "iob/vfs.h"
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
	AVFrame *wframe;
	AVFrame *rframe;
	ffmpeg_stream *stream;
	int64_t _prev_pts;
	int64_t _prev_duration;
	int64_t _avg_duration;
	int samples_size;
	int stream_ended;
	int align;
} ffmpeg_decoder;

typedef struct ffmpeg_frame {
	ffmpeg_decoder *decoder;
	AVFrame *frame;
	enum AVMediaType codec_type;
	int align;
} ffmpeg_frame;

extern int ffmpeg_open(const char *, void *, int(*)(ffmpeg *, void *, io_stream *), io_stream *);

extern int ffmpeg_find_stream(ffmpeg *, enum AVMediaType, int, void *, int(*)(ffmpeg_stream *, void *, io_stream *), io_stream *);

extern int ffmpeg_open_stream(const char *, enum AVMediaType, int, void *, int(*)(ffmpeg_stream *, void *, io_stream *), io_stream *);

extern int ffmpeg_open_decoder(ffmpeg_stream *, void *, int(*)(ffmpeg_stream *, ffmpeg_decoder *, void *, io_stream *) , io_stream *);

extern int ffmpeg_read(ffmpeg_stream *);

extern int ffmpeg_read_and_feed(ffmpeg_stream *, ffmpeg_decoder *);

extern int ffmpeg_decode(ffmpeg_decoder *, void *, int (*)(ffmpeg_decoder *, ffmpeg_frame *, void *, io_stream *), io_stream *);

extern int ffmpeg_decoded_size(ffmpeg_decoder *);

extern int64_t ffmpeg_frame_present_timestamp(const ffmpeg_frame *);

extern const char *ffmpeg_media_info(const ffmpeg_decoder *);

extern const char *ffmpeg_frame_info(const ffmpeg_frame *);

extern int ffmpeg_frame_copy(ffmpeg_frame *frame, void *, size_t, io_stream *);

extern int ffmpeg_create_frame(void *, int (*)(ffmpeg_frame *, void *, io_stream *), io_stream *);

extern int ffmpeg_load_image(ffmpeg_frame *, const video_frames *, void *, io_stream *);

#define ffmpeg_frame_data(frame) (frame)->frame->data
#define ffmpeg_frame_linesize(frame) (frame)->frame->linesize

#endif

