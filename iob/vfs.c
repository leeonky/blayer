#include <string.h>
#include "iob.h"
#include "vfs.h"

static void process_video_frames(io_bus *iob, const char *command, const char *event_args, void *arg, io_stream *io_s) {
	iob_video_frames_handler *handler = (iob_video_frames_handler *)arg; 
	if(handler->action) {
		video_frames frames;
		const char *frames_args = strstr(event_args, "frames:");
		sscanf(event_args, "w:%d h:%d fmt:%d align:%d cbuf:%d size:%d", &frames.width, &frames.height, &frames.format, &frames.align, &frames.cbuf_id, &frames.element_size);

		frames.count = sscanf(frames_args, "frames:%d=>%lld\n"
				, &frames.frames[0].index, &frames.frames[0].pts
				)/2;
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

