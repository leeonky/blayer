#ifndef WRPFFP_H
#define WRPFFP_H

#include <stdio.h>

typedef struct io_stream {
	FILE *stdin, *stdout, *stderr;
} io_stream;

typedef struct ffmpeg {
	AVFormatContext *format_context;
} ffmpeg;

typedef struct ffmpeg_stream {
	AVStream *stream;
} ffmpeg_stream;

extern int ffmpeg_main(const char *, void *, int(*)(ffmpeg *, void *, io_stream *), io_stream *);

extern int ffmpeg_find_stream(ffmpeg *, enum AVMediaType, int, void *, int(*)(ffmpeg_stream *, void *, io_stream *), io_stream *);

#endif

