#include <libavformat/avformat.h>
#include "vdecode.h"

int vdecode_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	int res = 0;
	AVFormatContext *avformat_context = NULL;
	av_register_all();
	avformat_open_input(&avformat_context, argv[0], NULL, NULL);
	avformat_find_stream_info(avformat_context, NULL);
	if (!avformat_context->nb_streams) {
		fprintf(stderr, "Warning[vdecode]: no streams in file\n");
	} else {
		if (avformat_context->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
			fprintf(stderr, "Error[vdecode]: No video stream at 1\n");
			res = -1;
		}
	}
	avformat_close_input(&avformat_context);
	return res;
}
