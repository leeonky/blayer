#include <getopt.h>
#include <stdint.h>
#include "iob/iob.h"
#include "iob/vfs.h"
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

#define MAX_BUFFER_SIZE 64
typedef struct vf_buf {
	int size;
	int count;
	video_frames frameses[MAX_BUFFER_SIZE];
} vf_buf;

static void output_video(video_frames *frame, io_stream *io_s) {
	fprintf(io_s->stdout, "VFS w:%d h:%d fmt:%d align:%d cbuf:%d size:%d frames:%d=>%lld", frame->width, frame->height, frame->format, frame->align, frame->cbuf_id, frame->element_size, frame->frames[0].index, frame->frames[0].pts);
}

static void output_append_frame(frame *frm, io_stream *io_s) {
	fprintf(io_s->stdout, ",%d=>%lld", frm->index, frm->pts);
}

static void output_and_clean_vf_buf(vf_buf *vbuf, io_stream *io_s) {
	video_frames *first = &vbuf->frameses[0];
	int i,j;
	output_video(first, io_s);
	for(i=1; i<vbuf->count; ++i) {
		output_append_frame(&vbuf->frameses[i].frames[0], io_s);
	}
	fprintf(io_s->stdout, "\n");
	fflush(io_s->stdout);
	vbuf->count = 0;
}

static void process_frames(const video_frames *vfs, void *arg, io_stream *io_s) {
	vf_buf *vbuf = (vf_buf*)arg;
	vbuf->frameses[vbuf->count++] = *vfs;
	if(vbuf->count == vbuf->size) {
		output_and_clean_vf_buf(vbuf, io_s);
	}
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_video_frames_handler handler = {
		.arg = arg,
		.action = process_frames,
	};

	iob_add_video_frames_handler(iob, &handler);
	return 0;
}

int vbuf_main(int size, io_stream *io_s) {
	vf_buf buf = {
		.size = size,
	};
	return iob_main(&buf, setup_frames_event, io_s);
}
