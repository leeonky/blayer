#include <libavformat/avformat.h>
#include "wrpffp.h"

static int print_error(int no, FILE *stderr) {
	char buffer[1024];
	av_strerror(no, buffer, sizeof(buffer));
	fprintf(stderr, "Error[libwrpffp]: %s\n", buffer);
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
	av_init_packet(&stream.packet);
	if (process) {
		res = process(&stream, arg, io_s);
	}
	av_packet_unref(&stream.packet);
	return res;
}

typedef struct find_stream_args {
	enum AVMediaType type;
	int track;
	void *arg;
	int(*process)(ffmpeg_stream *, void *, io_stream *);
} find_stream_args;

static int find_stream_process(ffmpeg *ffp, void *arg, io_stream *io_s) {
	find_stream_args *stream_arg = (find_stream_args *)arg;
	return ffmpeg_find_stream(ffp, stream_arg->type, stream_arg->track, stream_arg->arg, stream_arg->process, io_s);
}

int ffmpeg_main_stream(const char *file, enum AVMediaType type, int track, void *arg, int(*process)(ffmpeg_stream *, void *, io_stream *), io_stream *io_s) {
	find_stream_args stream_arg = {type, track, arg, process};
	return ffmpeg_main(file, &stream_arg, find_stream_process, io_s);
}

int ffmpeg_decoding(ffmpeg_stream *stream, void *arg, int(*process)(ffmpeg_stream *, ffmpeg_decoder *, void *, io_stream *) , io_stream *io_s) {
	int res = 0, ret;
	ffmpeg_decoder decoder;
	AVCodec *codec;
	if(codec = avcodec_find_decoder(stream->stream->codecpar->codec_id)) {
		if (decoder.codec_context = avcodec_alloc_context3(codec)) {
			if ((ret=avcodec_parameters_to_context(decoder.codec_context, stream->stream->codecpar)) >= 0
					&& (!(ret=avcodec_open2(decoder.codec_context, codec, NULL)))) {
				if (process) {
					res = process(stream, &decoder, arg, io_s);
				}
				avcodec_close(decoder.codec_context);
			} else {
				res = print_error(ret, io_s->stderr);
			}
			avcodec_free_context(&decoder.codec_context);
		} else {
			res = -1;
			fprintf(io_s->stderr, "Error[libwrpffp]: failed to alloc AVCodecContext\n");
		}
	} else {
		res = -1;
		fprintf(io_s->stderr, "Error[libwrpffp]: failed to find decoder\n");
	}
	return res;
}

int ffmpeg_stream_read(ffmpeg_stream *stream, io_stream *io_s) {
	int res = 0;
	while((!(res = av_read_frame(stream->format_context, &stream->packet)))
			&& stream->stream->index != stream->packet.stream_index)
		;
	return res;
}
