#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include "bputil/bputil.h"

#define MAX_VIDEO_FRAMES_SIZE	64

typedef struct frame {
	int index;
	int64_t pts;
} frame;

typedef struct video_frames {
	int width, height, format, align, cbuf_id, cbuf_bits, cbuf_size;
	size_t count;
	frame frames[MAX_VIDEO_FRAMES_SIZE];
} video_frames;

typedef struct iob_video_frames_handler {
	void * arg;
	void (*action)(const video_frames *, void *, io_stream *);
	void (*close)(void *, io_stream *);
} iob_video_frames_handler;

int iob_add_video_frames_handler(io_bus *, const iob_video_frames_handler *);

void output_append_frame(const frame *, io_stream *);

void output_video_frames(const video_frames *, io_stream *);

#endif
