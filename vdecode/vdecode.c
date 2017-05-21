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
	AVFormatContext *avformat_context = NULL;
	vdecode_args args;

	if(process_args(&args, argc, argv, stderr))
		return -1;

	av_register_all();
	avformat_open_input(&avformat_context, args.file_name, NULL, NULL);
	avformat_find_stream_info(avformat_context, NULL);
	if (!avformat_context->nb_streams) {
		fprintf(stderr, "Warning[vdecode]: no streams in file\n");
	} else {
		if (avformat_context->streams[args.video_index]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
			fprintf(stderr, "Error[vdecode]: No video stream at 1\n");
			res = -1;
		}
	}
	avformat_close_input(&avformat_context);
	return res;
}
