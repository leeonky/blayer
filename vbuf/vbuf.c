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

static void output_and_clean_vf_buf(vf_buf *vbuf, io_stream *io_s) {
	int i,j;
	output_video_frames(&vbuf->frameses[0], io_s);
	for(i=1; i<vbuf->count; ++i) {
		for(j=0; j<vbuf->frameses[i].count; ++j) {
			output_append_frame(&vbuf->frameses[i].frames[j], io_s);
		}
	}
	fprintf(io_s->stdout, "\n");
	fflush(io_s->stdout);
	vbuf->count = 0;
}

static inline void append_video_frames(vf_buf *vbuf, const video_frames *vfs) {
	vbuf->frameses[vbuf->count++] = *vfs;
}

static inline int is_vf_buf_full(vf_buf *vbuf) {
	return vbuf->count == vbuf->size;
}

static inline int is_video_format_different(const video_frames *vfs1, const video_frames *vfs2) {
	return vfs1->width != vfs2->width || vfs1->height != vfs2->height || vfs1->format != vfs2->format || vfs1->align != vfs2->align || vfs1->cbuf_id != vfs2->cbuf_id || vfs1->cbuf_size != vfs2->cbuf_size || vfs1->cbuf_bits != vfs2->cbuf_bits;
}

static inline int should_flush_output(vf_buf *vbuf, const video_frames *vfs) {
	return vbuf->count && is_video_format_different(&vbuf->frameses[vbuf->count-1], vfs);
}

static void process_frames(const video_frames *vfs, void *arg, io_stream *io_s) {
	vf_buf *vbuf = (vf_buf*)arg;
	if(should_flush_output(vbuf, vfs))
		output_and_clean_vf_buf(vbuf, io_s);
	append_video_frames(vbuf, vfs);
	if(is_vf_buf_full(vbuf)) {
		output_and_clean_vf_buf(vbuf, io_s);
	}
}

static void iob_close(void *arg, io_stream *io_s) {
	vf_buf *vbuf = (vf_buf*)arg;
	if(vbuf->count)
		output_and_clean_vf_buf(vbuf, io_s);
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_video_frames_handler handler = {
		.arg = arg,
		.action = process_frames,
		.close = iob_close,
	};

	return iob_add_video_frames_handler(iob, &handler);
}

int vbuf_main(int size, io_stream *io_s) {
	if(size > MAX_BUFFER_SIZE) {
		fprintf(io_s->stderr, "Error[vbuf]: required too much buffer\n");
		return -1;
	}
	vf_buf buf = {
		.size = size,
	};
	return iob_main(&buf, setup_frames_event, io_s);
}
