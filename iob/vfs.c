#include <string.h>
#include "iob.h"
#include "vfs.h"

static int parse_frames(video_frames *frames, char *frames_args) {
	int res = 0;
	FILE *frames_stream = fmemopen(frames_args, strlen(frames_args)+1, "r");
	if(frames_stream) {
		if(2==fscanf(frames_stream, "frames:%d=>%lld", &frames->frames[0].index, &frames->frames[0].pts)) {
			frames->count = 1;
			while(frames->count<MAX_VIDEO_FRAMES_SIZE && 2==fscanf(frames_stream, ",%d=>%lld", &frames->frames[frames->count].index, &frames->frames[frames->count].pts))
				frames->count++;
		} else	{
			res = -1;
		}
		fclose(frames_stream);
	} else	{
		res = -1;
	}
	return res;
}

static int parse_video_frames(video_frames *frames, const char *event_args) {
	int res = 0;
	if(6==sscanf(event_args, "w:%d h:%d fmt:%d align:%d cbuf:%d size:%d", &frames->width, &frames->height, &frames->format, &frames->align, &frames->cbuf_id, &frames->element_size)) {
		char *frames_args = strstr(event_args, "frames:");
		if(frames_args) {
			return parse_frames(frames, frames_args);
		}
	}
	return -1;
}

static void process_video_frames(io_bus *iob, const char *command, const char *event_args, void *arg, io_stream *io_s) {
	iob_video_frames_handler *handler = (iob_video_frames_handler *)arg; 
	if(handler->action) {
		video_frames frames;
		if(parse_video_frames(&frames, event_args))
			fprintf(io_s->stderr, "Error[iob]: bad VFS: [%s]\n", event_args);
		else
			handler->action(&frames, handler->arg, io_s);
	}
}

int iob_add_video_frames_handler(io_bus *iob, const iob_video_frames_handler *handler) {
	static iob_video_frames_handler vh;
	vh = *handler;
	iob_handler ioh = {
		.command = "VFS",
		.arg = &vh,
		.action = process_video_frames,
	};
	return iob_add_handler(iob, &ioh);
}

void output_append_frame(const frame *frm, io_stream *io_s) {
	fprintf(io_s->stdout, ",%d=>%lld", frm->index, frm->pts);
}

void output_video_frames(const video_frames *frames, io_stream *io_s) {
	int count;
	fprintf(io_s->stdout, "VFS w:%d h:%d fmt:%d align:%d cbuf:%d size:%d frames:%d=>%lld", frames->width, frames->height, frames->format, frames->align, frames->cbuf_id, frames->element_size, frames->frames[0].index, frames->frames[0].pts);

	for(count=1; count<frames->count; ++count) {
		output_append_frame(&frames->frames[count], io_s);
	}
}

