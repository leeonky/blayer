#include <libavformat/avformat.h>
#include <getopt.h>
#include "vdecode.h"
#include "wrpffp/wrpffp.h"
#include "bputil/bputil.h"

typedef struct app_context {
	vdecode_args *args;
	shm_cbuf *cbuf;
	ffmpeg_stream *stream;
} app_context;

int process_args(vdecode_args *args, int argc, char **argv, FILE *stderr) {
	args->file_name = NULL;
	args->video_index = -1;
	args->video_buf_bits = 4;
	int option_index = 0, c;
	struct option long_options[] = {
		{"video_index", required_argument, 0, 'v'},
		{0, 0, 0, 0}
	};
	optind = 1;
	while((c = getopt_long(argc, argv, "v:", long_options, &option_index)) != -1) {
		switch(c)
		case 'v':
			sscanf(optarg, "%d", &args->video_index);
			break;
	}
	if(optind < argc)
		args->file_name = argv[optind];
	if(!args->file_name) {
		fprintf(stderr, "Error[vdecode]: require video file\n");
		return -1;
	}
	return 0;
}

static int process_decoded_frame(ffmpeg_frame *frame, void *arg, io_stream *io_s) {
	shm_cbuf *cbuf = ((app_context *)arg)-> cbuf;
	if(!ffmpeg_frame_copy(frame, shrb_allocate(cbuf), cbuf->element_size, 1, io_s))
		fprintf(io_s->stdout, "video_frame:: %s %s\n", ffmpeg_video_frame_info(frame), shrb_info(cbuf));
}

static int decoding_video_stream(ffmpeg_stream *stream, ffmpeg_decoder *decoder, void *arg, io_stream *io_s) {
	shm_cbuf *cbuf = ((app_context *)arg)-> cbuf;
	while(!ffmpeg_read_and_feed(stream, decoder)) {
		ffmpeg_decode(decoder, arg, process_decoded_frame, io_s);
	}
	while(!ffmpeg_decode(decoder, arg, process_decoded_frame, io_s));
	return 0;
}

static int alloc_cbuf(shm_cbuf *cbuf, void *arg, io_stream *io_s) {
	ffmpeg_stream *stream = ((app_context *)arg)->stream;
	((app_context *)arg)->cbuf = cbuf;
	return ffmpeg_open_decoder(stream, arg, decoding_video_stream, io_s);
}

static int process_video_stream(ffmpeg_stream *stream, void *arg, io_stream *io_s) {
	((app_context *)arg)->stream = stream;
	return shrb_new(4, ffmpeg_frame_size(stream), arg, alloc_cbuf, io_s);
}

int vdecode_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	vdecode_args args;
	io_stream io_s = {stdin, stdout, stderr};
	app_context context = {&args};

	if(process_args(&args, argc, argv, stderr))
		return -1;
	return ffmpeg_open_stream(args.file_name, AVMEDIA_TYPE_VIDEO, args.video_index, &context, process_video_stream, &io_s);
}
