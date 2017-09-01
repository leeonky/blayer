#ifndef AFS_H
#define AFS_H

#include <stdint.h>
#include "bputil/bputil.h"

#define MAX_AUDIO_FRAMES_SIZE	128

typedef struct audio_frame_index {
	int index;
	int64_t pts;
	int samples_size;
} audio_frame_index;

typedef struct audio_frames {
	int sample_rate, channels, format, buffer_size, align, cbuf_id, cbuf_bits, cbuf_size;
	uint64_t layout;
	size_t count;
	audio_frame_index frames[MAX_AUDIO_FRAMES_SIZE];
} audio_frames;

typedef struct iob_audio_frames_handler {
	void * arg;
	void (*action)(const audio_frames *, void *, io_stream *);
	void (*close)(void *, io_stream *);
} iob_audio_frames_handler;

int iob_add_audio_frames_handler(io_bus *, const iob_audio_frames_handler *);

//void output_append_frame(const audio_frame_index *, FILE *);

//void output_audio_frames(const audio_frames *, FILE *);

#endif
