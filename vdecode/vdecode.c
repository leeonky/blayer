#include <libavformat/avformat.h>
#include <getopt.h>
#include "vdecode.h"
#include "wrpffp/wrpffp.h"
#include "bputil/bputil.h"

typedef struct app_context {
	vdecode_args *args;
	ffmpeg_stream *stream;
	ffmpeg_decoder *decoder;
	shm_cbuf *cbuf;
} app_context;

int process_args(vdecode_args *args, int argc, char **argv, FILE *stderr) {
	args->file_name = NULL;
	args->video_index = -1;
	args->buffer_bits = 4;
	int option_index = 0, c;
	struct option long_options[] = {
		{"video_index", required_argument, 0, 'v'},
		{"buffer_bits", required_argument, 0, 'b'},
		{0, 0, 0, 0}
	};
	optind = 1;
	while((c = getopt_long(argc, argv, "v:b:", long_options, &option_index)) != -1) {
		switch(c) {
			case 'v':
				sscanf(optarg, "%d", &args->video_index);
				break;
			case 'b':
				sscanf(optarg, "%d", &args->buffer_bits);
				break;
		}
	}
	if(optind < argc)
		args->file_name = argv[optind++];
	if(!args->file_name) {
		fprintf(stderr, "Error[vdecode]: require video file\n");
		return -1;
	}
	return 0;
}

static int process_decoded_frame(ffmpeg_decoder *decoder, ffmpeg_frame *frame, void *arg, io_stream *io_s) {
	shm_cbuf *cbuf = ((app_context *)arg)-> cbuf;
	if(!ffmpeg_frame_copy(frame, shrb_allocate(cbuf), cbuf->element_size, io_s)) {
		fprintf(io_s->stdout, "VFS %s align:%d %s frames:%d=>%lld\n", ffmpeg_video_info(decoder), 1, shrb_info(cbuf), shrb_index(cbuf), ffmpeg_frame_present_timestamp(frame));
		fflush(io_s->stdout);
	}
	return 0;
}

static int cbuf_allocated(shm_cbuf *cbuf, void *arg, io_stream *io_s) {
	app_context *context = (app_context *)arg;
	ffmpeg_stream *stream = context->stream;
	ffmpeg_decoder *decoder = context->decoder;
	context->cbuf = cbuf;
	while(!ffmpeg_read_and_feed(stream, decoder)) {
		ffmpeg_decode(decoder, 1, arg, process_decoded_frame, io_s);
	}
	while(!ffmpeg_decode(decoder, 1, arg, process_decoded_frame, io_s));
	return 0;
}

static int decoder_opened(ffmpeg_stream *stream, ffmpeg_decoder *decoder, void *arg, io_stream *io_s) {
	((app_context *)arg)->decoder = decoder;
	return shrb_new(((app_context *)arg)->args->buffer_bits, ffmpeg_decoded_size(decoder, 1), arg, cbuf_allocated, io_s);
}

static int process_video_stream(ffmpeg_stream *stream, void *arg, io_stream *io_s) {
	((app_context *)arg)->stream = stream;
	return ffmpeg_open_decoder(stream, arg, decoder_opened, io_s);
}

int vdecode_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	vdecode_args args;
	app_context context = {&args};

	if(!process_args(&args, argc, argv, stderr)) {
		io_stream io_s = {stdin, stdout, stderr};
		return ffmpeg_open_stream(args.file_name, AVMEDIA_TYPE_VIDEO, args.video_index, &context, process_video_stream, &io_s);
	}
	return -1;
}
