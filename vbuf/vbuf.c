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

/*static int get_frame_info(*/

int vbuf_main(int size, io_stream *io_s) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int i;

	for(i=0; i<size; i++) {
	}

	read = getline(&line, &len, io_s->stdin);
	sscanf(line, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld\n", &frame_width, &frame_height, &pixel_format, &frame_align, &cbuf_id, &cbuf_element_size, &cbuf_index, &pts);
	fprintf(io_s->stdout, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld", frame_width, frame_height, pixel_format, frame_align, cbuf_id, cbuf_element_size, cbuf_index, pts);

	read = getline(&line, &len, io_s->stdin);
	sscanf(line, "video_frames:: width:%d height:%d format:%d align:%d cbuf:%d size:%d frames:%d=>%lld\n", &frame_width, &frame_height, &pixel_format, &frame_align, &cbuf_id, &cbuf_element_size, &cbuf_index, &pts);
	fprintf(io_s->stdout, ",%d=>%lld", cbuf_index, pts);

	fprintf(io_s->stdout, "\n");
	fflush(io_s->stdout);

	return 0;
}
