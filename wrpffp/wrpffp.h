#ifndef WRPFFP_H
#define WRPFFP_H

#include <stdio.h>

typedef struct io_stream {
	FILE *stdin, *stdout, *stderr;
} io_stream;

typedef struct ffmpeg {
	AVFormatContext *format_context;
} ffmpeg;

extern int ffmpeg_main(const char *, void *, int(*)(ffmpeg *, void *, io_stream *), io_stream *);

#endif

