#include "wrpffp.h"

static int print_error(int no, FILE *stderr) {
	char buffer[1024];
	av_strerror(no, buffer, sizeof(buffer));
	fprintf(stderr, "Error[libwrpffp]: %s\n", buffer);
	print_stack(stderr);
	return -1;
}

int ffmpeg_open(const char *file, void *arg, int(*process)(ffmpeg *, void *, io_stream *), io_stream *io_s) {
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
	stream.format_context = ffp->format_context;
	if (!(stream.stream = find_stream(ffp->format_context, type, (track<0 ? 0 : track)))) {
		fprintf(io_s->stderr, "Error[libwrpffp]: %s stream %d doesn't exist\n", av_get_media_type_string(type), track);
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

int ffmpeg_open_stream(const char *file, enum AVMediaType type, int track, void *arg, int(*process)(ffmpeg_stream *, void *, io_stream *), io_stream *io_s) {
	find_stream_args stream_arg = {type, track, arg, process};
	return ffmpeg_open(file, &stream_arg, find_stream_process, io_s);
}

int ffmpeg_open_decoder(ffmpeg_stream *stream, void *arg, int(*process)(ffmpeg_stream *, ffmpeg_decoder *, void *, io_stream *) , io_stream *io_s) {
	int res = 0, ret;
	ffmpeg_decoder decoder;
	AVCodec *codec;
	decoder.stream = stream;
	if((codec = avcodec_find_decoder(stream->stream->codecpar->codec_id))) {
		if ((decoder.codec_context = avcodec_alloc_context3(codec))) {
			if ((ret=avcodec_parameters_to_context(decoder.codec_context, stream->stream->codecpar)) >= 0
					&& (!(ret=avcodec_open2(decoder.codec_context, codec, NULL)))) {
				if((decoder.frame = av_frame_alloc())) {
					if (process) {
						res = process(stream, &decoder, arg, io_s);
					}
					av_frame_free(&decoder.frame);
				} else {
					res = -1;
					fprintf(io_s->stderr, "Error[libwrpffp]: failed to alloc AVFrame\n");
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

int ffmpeg_read(ffmpeg_stream *stream) {
	int res = 0;
	while((!(res = av_read_frame(stream->format_context, &stream->packet)))
			&& stream->stream->index != stream->packet.stream_index)
		;
	return res;
}

int ffmpeg_read_and_feed(ffmpeg_stream *stream, ffmpeg_decoder *decoder) {
	int res = 0;
	if((res=ffmpeg_read(stream)) >= 0)
		res = avcodec_send_packet(decoder->codec_context, &stream->packet);
	else
		avcodec_send_packet(decoder->codec_context, NULL);
	return res;
}

int ffmpeg_decoded_size(ffmpeg_decoder *decoder, int align) {
	AVCodecContext *codec_context = decoder->codec_context;
	switch(codec_context->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			return av_image_get_buffer_size(codec_context->pix_fmt, codec_context->width, codec_context->height, align);
		case AVMEDIA_TYPE_AUDIO:
			return av_samples_get_buffer_size(NULL, codec_context->channels, codec_context->frame_size, codec_context->sample_fmt, align!=0);
		default:
			fprintf(stderr, "ffmpeg_frame_size not support [%s] yet\n", av_get_media_type_string(codec_context->codec_type));
			abort();
	}
}

int ffmpeg_decode(ffmpeg_decoder *decoder, int align, void *arg, int (*process)(ffmpeg_decoder *, ffmpeg_frame *, void *, io_stream *), io_stream *io_s) {
	int res = 0, ret;
	ffmpeg_frame fffrm = {
		.decoder = decoder,
		.frame = decoder->frame,
		.codec_type = decoder->codec_context->codec_type,
		.align = align,
	};
	AVCodecContext *codec_context = decoder->codec_context;
	if(!(res=avcodec_receive_frame(codec_context, fffrm.frame))) {
		process(decoder, &fffrm, arg, io_s);
	}
	return res;
}

int64_t ffmpeg_frame_present_timestamp(ffmpeg_frame *frame) {
	AVStream *stream = frame->decoder->stream->stream;
	AVCodecContext *codec_context = frame->decoder->codec_context;
	int64_t pts = av_frame_get_best_effort_timestamp(frame->decoder->frame);
	if(AV_NOPTS_VALUE == pts) {
		return frame->decoder->_pts += frame->decoder->_duration;
	} else {
		if(frame->decoder->frame->pkt_duration) {
			frame->decoder->_duration = av_rescale_q(frame->decoder->frame->pkt_duration, stream->time_base, AV_TIME_BASE_Q);
		} else if(codec_context->framerate.num != 0 && codec_context->framerate.den != 0) {
			int ticks = stream->parser ? stream->parser->repeat_pict+1 : codec_context->ticks_per_frame;
			frame->decoder->_duration = ((int64_t)AV_TIME_BASE *
					codec_context->framerate.den * ticks) /
				codec_context->framerate.num / codec_context->ticks_per_frame;
		} else {
			frame->decoder->_duration = 3000;
		}
		return frame->decoder->_pts = av_rescale_q(pts-stream->start_time, stream->time_base, AV_TIME_BASE_Q);
	}
}

const char *ffmpeg_video_info(ffmpeg_decoder *decoder) {
	static __thread char buffer[1024];
	AVCodecContext *codec_context = decoder->codec_context;
	sprintf(buffer, "w:%d h:%d fmt:%d",
			codec_context->width,
			codec_context->height,
			codec_context->pix_fmt);

	return buffer;
}

int ffmpeg_frame_copy(ffmpeg_frame *frame, void *buf, size_t size, io_stream *io_s) {
	int res = 0, ret;
	AVFrame *avframe = frame->frame;
	switch(frame->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			if((ret=av_image_copy_to_buffer(buf, size, (const uint8_t * const *)avframe->data, avframe->linesize, avframe->format, avframe->width, avframe->height, frame->align)) < 0 )
				res = print_error(ret, io_s->stderr);
			break;
		case AVMEDIA_TYPE_AUDIO:
			{
				uint8_t *dst_bufs[] = {buf};
				if((ret=av_samples_copy(dst_bufs, avframe->data, 0, 0, avframe->nb_samples, avframe->channels, avframe->format))<0)
					res = print_error(ret, io_s->stderr);
			}
			break;
		default:
			fputs ("ffmpeg_frame_copy not support audio yet\n", stderr);
			abort();
			break;
	}
	return res;
}

int ffmpeg_create_frame(void *arg, int (*action)(ffmpeg_frame *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	ffmpeg_frame frame = {};
	if((frame.frame = av_frame_alloc())) {
		if (action) {
			res = action(&frame, arg, io_s);
		}
		av_frame_free(&frame.frame);
	} else {
		res = -1;
		fprintf(io_s->stderr, "Error[libwrpffp]: failed to alloc AVFrame\n");
		print_stack(io_s->stderr);
	}
	return res;
}

int ffmpeg_load_image(ffmpeg_frame *frame, const video_frames *vfs, void *data, io_stream *io_s) {
	int res = 0, ret;
	AVFrame *f = frame->frame;
	frame->codec_type = AVMEDIA_TYPE_VIDEO;
	frame->align = vfs->align;
	if((ret=av_image_fill_arrays(f->data, f->linesize, (const uint8_t *)data, vfs->format, vfs->width, vfs->height, vfs->align))<0) {
		res = print_error(ret, io_s->stderr);
	}
	return res;
}
