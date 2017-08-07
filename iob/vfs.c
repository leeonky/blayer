#include <string.h>
#include "iob.h"
#include "vfs.h"

static void process_video_frames(io_bus *iob, const char *command, const char *event_args, void *arg, io_stream *io_s) {
	iob_video_frames_handler *handler = (iob_video_frames_handler *)arg; 
	if(handler->action) {
		video_frames frames;
		sscanf(event_args, "w:%d h:%d fmt:%d align:%d cbuf:%d size:%d", &frames.width, &frames.height, &frames.format, &frames.align, &frames.cbuf_id, &frames.element_size);

		char *frames_args = strstr(event_args, "frames:");
		if(frames_args) {
			FILE *frames_stream = fmemopen(frames_args, strlen(frames_args)+1, "r");
			if(frames_stream) {
				if(2==fscanf(frames_stream, "frames:%d=>%lld", &frames.frames[0].index, &frames.frames[0].pts)) {
					frames.count = 1;
					while(frames.count<MAX_VIDEO_FRAMES_SIZE && 2==fscanf(frames_stream, ",%d=>%lld", &frames.frames[frames.count].index, &frames.frames[frames.count].pts))
						frames.count++;
				}
				fclose(frames_stream);
			}
		}
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

