#include <libavformat/avformat.h>
#include "vdecode.h"

int vdecode_main(int argc, char **argv) {
	AVFormatContext *avformat_context = NULL;
	av_register_all();
	avformat_open_input(&avformat_context, argv[0], NULL, NULL);
	return 0;
}
