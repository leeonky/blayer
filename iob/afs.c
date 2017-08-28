#include <string.h>
#include <inttypes.h>
#include "iob.h"
#include "afs.h"
#include "sys/sys.h"

static int parse_frames_in_stream(FILE *frames_stream, void *arg) {
	audio_frames *frames = (audio_frames *)arg;
	if(3==fscanf(frames_stream, "frames:%d=>%"PRId64",%d", &frames->frames[0].index, &frames->frames[0].pts, &frames->frames[0].samples_size)) {
		frames->count = 1;
		while(frames->count<MAX_AUDIO_FRAMES_SIZE && 3==fscanf(frames_stream, ",%d=>%"PRId64",%d", &frames->frames[frames->count].index, &frames->frames[frames->count].pts, &frames->frames[frames->count].samples_size))
			frames->count++;
		return 0;
	}
	return -1;
}

static int parse_audio_frames(audio_frames *frames, const char *event_args) {
	int res = 0;
	if(9==sscanf(event_args, "rt:%d ch:%d fmt:%d buf:%d lay:%d align:%d cbuf:%d bits:%d size:%d", &frames->sample_rate, &frames->channels, &frames->format, &frames->buffer_size, &frames->layout, &frames->align, &frames->cbuf_id, &frames->cbuf_bits, &frames->cbuf_size)) {
		char *frames_args = strstr(event_args, "frames:");
		if(frames_args)
			return fmemprocess(frames_args, strlen(frames_args), "r", frames, parse_frames_in_stream);
	}
	return -1;
}

static void process_audio_frames(io_bus *iob, const char *command, const char *event_args, void *arg, io_stream *io_s) {
	iob_audio_frames_handler *handler = (iob_audio_frames_handler *)arg;
	if(handler->action) {
		audio_frames frames;
		if(parse_audio_frames(&frames, event_args))
			fprintf(io_s->stderr, "Error[iob]: bad AFS: [%s]\n", event_args);
		else
			handler->action(&frames, handler->arg, io_s);
	}
}

int iob_add_audio_frames_handler(io_bus *iob, const iob_audio_frames_handler *handler) {
	static iob_audio_frames_handler ah;
	ah = *handler;
	iob_handler ioh = {
		.command = "AFS",
		.arg = &ah,
		.action = process_audio_frames,
		.close = NULL,
	};
	return iob_add_handler(iob, &ioh);
}
