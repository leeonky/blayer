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

/*static int frame_width, frame_height, pixel_format, frame_align, cbuf_id, cbuf_index, cbuf_element_size;*/
/*static int64_t pts;*/

/*#define MAX_VIDEO_FRAMES_SIZE	256*/

/*typedef struct video_frames {*/
	/*int width, height, format, align, cbuf_id, element_size;*/
	/*size_t count;*/
	/*struct frame {*/
		/*int index;*/
		/*int64_t pts;*/
	/*} frames[MAX_VIDEO_FRAMES_SIZE];*/
/*} video_frames;*/

/*static int get_frame_info(char **line, size_t *len, io_stream *io_s, void *arg, void(*process)(video_frames *, void *)) {*/
	/*ssize_t read;*/
	/*video_frames frames;*/
	/*read = getline(line, len, io_s->stdin);*/

	/*sscanf(*line, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld\n", &frames.width, &frames.height, &frames.format, &frames.align, &frames.cbuf_id, &frames.element_size, &frames.frames[0].index, &frames.frames[0].pts);*/

	/*if(process) {*/
		/*process(&frames, arg);*/
	/*}*/
/*}*/

/*static int read_frames(io_stream *io_s, void *arg, void(*process)(video_frames *, void *, io_stream *)) {*/
	/*char *line = NULL;*/
	/*size_t len = 0;*/
	/*ssize_t read;*/
	/*video_frames frames;*/
	/*if(getline(&line, &len, io_s->stdin) != -1) {*/
		/*if(8 == sscanf(line, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld\n", &frames.width, &frames.height, &frames.format, &frames.align, &frames.cbuf_id, &frames.element_size, &frames.frames[0].index, &frames.frames[0].pts)) {*/
			/*if(process) {*/
				/*process(&frames, arg, io_s);*/
			/*}*/
			/*return 0;*/
		/*}*/
	/*}*/
	/*return -1;*/
/*}*/

/*static void output_frames(video_frames *frames, void *arg, io_stream *io_s) {*/
	/*fprintf(io_s->stdout, ",%d=>%lld", frames->frames[0].index, frames->frames[0].pts);*/
/*}*/

/*static void output_video_frame(video_frames *frames, void *arg, io_stream *io_s) {*/
	/*fprintf(io_s->stdout, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld", frames->width, frames->height, frames->format, frames->align, frames->cbuf_id, frames->element_size, frames->frames[0].index, frames->frames[0].pts);*/
	/*int size = *(int *)arg;*/

	/*while((--size) && !read_frames(io_s, NULL, output_frames))*/
		/*;*/

	/*fprintf(io_s->stdout, "\n");*/
	/*fflush(io_s->stdout);*/
/*}*/

#define MAX_BUFFER_SIZE 64
typedef struct vf_buf {
	int size;
	int count;
	video_frames frameses[64];
} vf_buf;

static void output_video(video_frames *frame, io_stream *io_s) {
	fprintf(io_s->stdout, "VFS w:%d h:%d fmt:%d align:%d cbuf:%d size:%d frames:%d=>%lld", frame->width, frame->height, frame->format, frame->align, frame->cbuf_id, frame->element_size, frame->frames[0].index, frame->frames[0].pts);
}

static void output_append_frame(frame *frm, io_stream *io_s) {
	fprintf(io_s->stdout, ",%d=>%lld", frm->index, frm->pts);
}

static void output_vf_buf(vf_buf *vbuf, io_stream *io_s) {
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
		output_vf_buf(vbuf, io_s);
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
