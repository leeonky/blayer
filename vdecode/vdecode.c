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
	fprintf(io_s->stdout, "video_frame:: width:%d height:%d format:%d pts:%lld\n",
			ffmpeg_image_width(frame),
			ffmpeg_image_height(frame),
			ffmpeg_image_pixel_format(frame),
			ffmpeg_frame_present_timestamp(frame));
}

static int decoding_video_stream(ffmpeg_stream *stream, ffmpeg_decoder *decoder, void *arg, io_stream *io_s) {
	shm_cbuf *cbuf = ((app_context *)arg)-> cbuf;
	while(!ffmpeg_read_and_feed(stream, decoder)) {
		ffmpeg_decode(decoder, shrb_get(cbuf), arg, process_decoded_frame, io_s);
	}
	while(!ffmpeg_decode(decoder, shrb_get(cbuf), arg, process_decoded_frame, io_s));
	/*while(ffmpeg_read(stream, io_s)>=0) {*/
		/*avcodec_send_packet(decoder->codec_context, &stream->packet);*/
		/*if(!avcodec_receive_frame(decoder->codec_context, decoder->frame)) {*/
			/*int64_t pts = av_frame_get_best_effort_timestamp(decoder->frame)-stream->stream->start_time;*/
			/*fprintf(io_s->stdout, "video_frame:: width:%d height:%d format:%d pts:%lld\n" , decoder->frame->width, decoder->frame->height, decoder->frame->format, pts*stream->stream->time_base.num*1000/stream->stream->time_base.den);*/
		/*}*/
	/*}*/
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
