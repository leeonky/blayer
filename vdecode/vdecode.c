#include <libavformat/avformat.h>
#include <getopt.h>
#include "vdecode.h"
#include "wrpffp/wrpffp.h"

int process_args(vdecode_args *args, int argc, char **argv, FILE *stderr) {
	args->file_name = NULL;
	args->video_index = -1;
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


static int decoding_video_stream(ffmpeg_stream *stream, ffmpeg_decoder *decoder, void *arg, io_stream *io_s) {
	AVFrame *frame = av_frame_alloc();
	while(ffmpeg_stream_read(stream, io_s)>=0) {
		avcodec_send_packet(decoder->codec_context, &stream->packet);
		if(!avcodec_receive_frame(decoder->codec_context, frame)) {
			int64_t pts = av_frame_get_best_effort_timestamp(frame)-stream->stream->start_time;
			fprintf(io_s->stdout, "video_frame:: width:%d height:%d format:%d pts:%lld\n" , frame->width, frame->height, frame->format, pts*stream->stream->time_base.num*1000/stream->stream->time_base.den);
		}
	}
	av_frame_free(&frame);
	return 0;
}

static int process_video_stream(ffmpeg_stream *stream, void *arg, io_stream *io_s) {
	return ffmpeg_decoding(stream, arg, decoding_video_stream, io_s);
}

int vdecode_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	vdecode_args args;
	io_stream io_s = {stdin, stdout, stderr};

	process_args(&args, argc, argv, stderr);

	return ffmpeg_main_stream(args.file_name, AVMEDIA_TYPE_VIDEO, args.video_index, NULL, process_video_stream, &io_s);
}
