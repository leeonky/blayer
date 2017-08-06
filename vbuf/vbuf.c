#include <getopt.h>
#include <stdint.h>
#include "vbuf.h"

int process_args(vbuf_args *args, int argc, char **argv, FILE *stderr) {
	args->input = NULL;
	args->size = 8;

	int option_index = 0, c;
	struct option long_options[] = {
		{"size", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};
	optind = 1;
	while((c = getopt_long(argc, argv, "s:", long_options, &option_index)) != -1) {
		switch(c) {
			case 's':
				sscanf(optarg, "%d", &args->size);
				break;
		}
	}
	if(optind < argc)
		args->input = argv[optind];
	if(!args->input) {
		fprintf(stderr, "Error[vbuf]: require input file\n");
		return -1;
	}
	return 0;
}

static int frame_width, frame_height, pixel_format, frame_align, cbuf_id, cbuf_index, cbuf_element_size;
static int64_t pts;

#define MAX_VIDEO_FRAMES_SIZE	256

typedef struct video_frames {
	int width, height, format, align, cbuf_id, element_size;
	size_t count;
	struct frame {
		int index;
		int64_t pts;
	} frames[MAX_VIDEO_FRAMES_SIZE];
} video_frames;

static int get_frame_info(char **line, size_t *len, io_stream *io_s, void *arg, void(*process)(video_frames *, void *)) {
	ssize_t read;
	video_frames frames;
	read = getline(line, len, io_s->stdin);

	sscanf(*line, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld\n", &frames.width, &frames.height, &frames.format, &frames.align, &frames.cbuf_id, &frames.element_size, &frames.frames[0].index, &frames.frames[0].pts);

	if(process) {
		process(&frames, arg);
	}
}

static int read_frames(io_stream *io_s, void *arg, void(*process)(video_frames *, void *, io_stream *)) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	video_frames frames;
	if(getline(&line, &len, io_s->stdin) != -1) {
		if(8 == sscanf(line, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld\n", &frames.width, &frames.height, &frames.format, &frames.align, &frames.cbuf_id, &frames.element_size, &frames.frames[0].index, &frames.frames[0].pts)) {
			if(process) {
				process(&frames, arg, io_s);
			}
			return 0;
		}
	}
	return -1;
}

static void output_frames(video_frames *frames, void *arg, io_stream *io_s) {
	fprintf(io_s->stdout, ",%d=>%lld", frames->frames[0].index, frames->frames[0].pts);
}

static void output_video_frame(video_frames *frames, void *arg, io_stream *io_s) {
	fprintf(io_s->stdout, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld", frames->width, frames->height, frames->format, frames->align, frames->cbuf_id, frames->element_size, frames->frames[0].index, frames->frames[0].pts);
	int size = *(int *)arg;

	while((--size) && !read_frames(io_s, NULL, output_frames))
		;

	fprintf(io_s->stdout, "\n");
	fflush(io_s->stdout);
}

int vbuf_main(int size, io_stream *io_s) {
	while(!read_frames(io_s, &size, output_video_frame))
		;
	return 0;
}
