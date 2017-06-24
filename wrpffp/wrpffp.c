#include <libavformat/avformat.h>
#include "wrpffp.h"

static int print_error(int no, FILE *stderr) {
	char buffer[1024];
	av_strerror(no, buffer, sizeof(buffer));
	fprintf(stderr, "Error[wrpffp]: %s\n", buffer);
	return -1;
}

int ffmpeg_main(const char *file, void *arg, int(*process)(ffmpeg *, void *, io_stream *), io_stream *io_s) {
	int res = 0, ret;
	ffmpeg ffp = {};

	av_register_all();
	if ((ret = avformat_open_input(&ffp.format_context, file, NULL, NULL))) {
		res = print_error(ret, io_s->stderr);
	} else {
		if((ret = avformat_find_stream_info(ffp.format_context, NULL)) < 0) {
			res = print_error(ret, io_s->stderr);
		} else if(process) {
			res = process(&ffp, arg, io_s);
		}
		avformat_close_input(&ffp.format_context);
	}
	return res;
}

static AVStream *find_stream(AVFormatContext *format_context, enum AVMediaType type, int track) {
	int i, matched = 0;
	for (i=0; i<format_context->nb_streams; ++i) {
		if(format_context->streams[i]->codecpar->codec_type == type && matched++ == track) {
			return format_context->streams[i];
		}
	}
	return NULL;
}

int ffmpeg_find_stream(ffmpeg *ffp, enum AVMediaType type, int track, void *arg, int(*process)(ffmpeg_stream *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	ffmpeg_stream stream = {};
	if (!(stream.stream = find_stream(ffp->format_context, type, (track<0 ? 0 : track)))) {
		fprintf(io_s->stderr, "Error[libwrpffp]: %s stream %d doesn't exist", av_get_media_type_string(type), track);
		return -1;
	}
	if (process) {
		res = process(&stream, arg, io_s);
	}
	return res;
}
