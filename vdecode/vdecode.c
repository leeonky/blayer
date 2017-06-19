#include <libavformat/avformat.h>
#include <getopt.h>
#include "vdecode.h"

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

int vdecode_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	int res = 0;
	vdecode_args args;

	AVFormatContext *context = NULL;
	AVCodecContext *codec_context = NULL;

	process_args(&args, argc, argv, stderr);

	av_register_all();

	avformat_open_input(&context, args.file_name, NULL, NULL);

	avformat_find_stream_info(context, NULL);

	AVStream *stream = context->streams[args.video_index];
	AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
	codec_context = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codec_context, stream->codecpar);
	avcodec_open2(codec_context, codec, NULL);

	AVFrame *frame = av_frame_alloc();
	AVPacket packet;

	while(av_read_frame(context, &packet)>=0) {
		if(packet.stream_index == args.video_index) {
			avcodec_send_packet(codec_context, &packet);
			if(!avcodec_receive_frame(codec_context, frame)) {
				int64_t pts = av_frame_get_best_effort_timestamp(frame)-stream->start_time;
				fprintf(stdout, "video_frame:: width:%d height:%d format:%d pts:%lld\n" , frame->width, frame->height, frame->format, pts*stream->time_base.num*1000/stream->time_base.den);
			}
			continue;
		}
	}

	av_packet_unref(&packet);
	av_frame_free(&frame);
	avcodec_close(codec_context);
	avcodec_free_context(&codec_context);
	avformat_close_input(&context);
	return res;
}
