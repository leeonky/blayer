#include <libavformat/avformat.h>
#include <getopt.h>
#include "decoder.h"
#include "wrpffp/wrpffp.h"
#include "bputil/bputil.h"

typedef struct app_context {
	decoder_args *args;
	ffmpeg_stream *stream;
	ffmpeg_decoder *decoder;
	shm_cbuf *cbuf;
} app_context;

int process_args(decoder_args *args, int argc, char **argv, FILE *stderr) {
	args->file_name = NULL;
	args->track_index = -1;
	args->buffer_bits = 4;
	args->track_type = AVMEDIA_TYPE_VIDEO;
	int option_index = 0, c;
	struct option long_options[] = {
		{"audio", required_argument, 0, 'a'},
		{"video", required_argument, 0, 'v'},
		{"bits", required_argument, 0, 'b'},
		{0, 0, 0, 0}
	};
	optind = 1;
	while((c = getopt_long(argc, argv, "v:b:", long_options, &option_index)) != -1) {
		switch(c) {
			case 'a':
				sscanf(optarg, "%d", &args->track_index);
				args->track_type = AVMEDIA_TYPE_AUDIO;
				break;
			case 'v':
				sscanf(optarg, "%d", &args->track_index);
				args->track_type = AVMEDIA_TYPE_VIDEO;
				break;
			case 'b':
				sscanf(optarg, "%d", &args->buffer_bits);
				break;
		}
	}
	if(optind < argc)
		args->file_name = argv[optind++];
	if(!args->file_name) {
		fprintf(stderr, "Error[decoder]: require media file\n");
		return -1;
	}
	return 0;
}

static int process_decoded_frame(ffmpeg_decoder *decoder, ffmpeg_frame *frame, void *arg, io_stream *io_s) {
	shm_cbuf *cbuf = ((app_context *)arg)-> cbuf;
	if(!ffmpeg_frame_copy(frame, shrb_allocate(cbuf), cbuf->element_size, io_s)) {
		fprintf(io_s->stdout, "%s align:%d %s frames:%d=>%s\n", ffmpeg_media_info(decoder), frame->align, shrb_info(cbuf), shrb_index(cbuf), ffmpeg_frame_info(frame));
		fflush(io_s->stdout);
	}
	return 0;
}

static int cbuf_allocated(shm_cbuf *cbuf, void *arg, io_stream *io_s) {
	app_context *context = (app_context *)arg;
	context->cbuf = cbuf;
	ffmpeg_decoder *decoder = context->decoder;

	while(!ffmpeg_read_and_feed(context->stream, decoder)) {
		ffmpeg_decode(decoder, arg, process_decoded_frame, io_s);
	}
	while(!ffmpeg_decode(decoder, arg, process_decoded_frame, io_s));
	return 0;
}

static int decoder_opened(ffmpeg_stream *stream, ffmpeg_decoder *decoder, void *arg, io_stream *io_s) {
	((app_context *)arg)->decoder = decoder;
	return shrb_new(((app_context *)arg)->args->buffer_bits, ffmpeg_decoded_size(decoder), arg, cbuf_allocated, io_s);
}

static int stream_opened(ffmpeg_stream *stream, void *arg, io_stream *io_s) {
	((app_context *)arg)->stream = stream;
	return ffmpeg_open_decoder(stream, arg, decoder_opened, io_s);
}

int decoder_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	decoder_args args;
	app_context context = {&args};

	if(!process_args(&args, argc, argv, stderr)) {
		io_stream io_s = {stdin, stdout, stderr};
		return ffmpeg_open_stream(args.file_name, args.track_type, args.track_index, &context, stream_opened, &io_s);
	}
	return -1;
}
