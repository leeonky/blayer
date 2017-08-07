#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include "bputil/bputil.h"

#define MAX_VIDEO_FRAMES_SIZE	256

typedef struct video_frames {
	int width, height, format, align, cbuf_id, element_size;
	size_t count;
	struct frame {
		int index;
		int64_t pts;
	} frames[MAX_VIDEO_FRAMES_SIZE];
} video_frames;

typedef struct iob_video_frames_handler {
	void * arg;
	void (*action)(const video_frames *, void *, io_stream *);
	void (*close)(void *, io_stream *);
} iob_video_frames_handler;

int iob_add_video_frames_handler(io_bus *, const iob_video_frames_handler *);

#endif
