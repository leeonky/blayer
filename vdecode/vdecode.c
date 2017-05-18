#include <libavformat/avformat.h>
#include "vdecode.h"

int vdecode_main(int argc, char **argv) {
	AVFormatContext *avformat_context = NULL;
	av_register_all();
	avformat_open_input(&avformat_context, argv[0], NULL, NULL);
	avformat_find_stream_info(avformat_context, NULL);
	avformat_close_input(&avformat_context);
}
