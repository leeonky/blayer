#include <inttypes.h>
#include "wrpffp.h"

static inline void unsupported_operation(const char *fun, enum AVMediaType t) {
	fprintf(stderr, "%s not support [%s] yet\n", fun, av_get_media_type_string(t));
	abort();
}

#define not_support_media_type(t) unsupported_operation(__FUNCTION__, t)

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

static int64_t samples_to_duration(int size, const AVCodecContext *codec_context) {
	return (int64_t)size*1000000/codec_context->sample_rate;
}

static void guess_avg_duration(ffmpeg_decoder *decoder) {
	AVStream *stream = decoder->stream->stream;
	AVCodecContext *codec_context = decoder->codec_context;
	switch(codec_context->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			if(codec_context->framerate.num != 0 && codec_context->framerate.den != 0) {
				int ticks = stream->parser ? stream->parser->repeat_pict+1 : codec_context->ticks_per_frame;
				decoder->_avg_duration = ((int64_t)AV_TIME_BASE *
						codec_context->framerate.den * ticks) /
					codec_context->framerate.num / codec_context->ticks_per_frame;
			}
			break;
		case AVMEDIA_TYPE_AUDIO:
			break;
		default:
			not_support_media_type(codec_context->codec_type);
			break;
	}
}

static int open_for_media(ffmpeg_decoder *decoder, void *arg, int(*process)(ffmpeg_stream *, ffmpeg_decoder *, void *, io_stream *) , io_stream *io_s) {
	int res = 0, ret;
	if((decoder->wframe = av_frame_alloc())) {
		if((decoder->rframe = av_frame_alloc())) {
			AVCodecContext *codec_context = decoder->codec_context;
			AVFrame *rframe = decoder->rframe;
			if(AVMEDIA_TYPE_AUDIO == codec_context->codec_type) {
				rframe->channels = codec_context->channels;
				rframe->format =  codec_context->sample_fmt;
				rframe->sample_rate = codec_context->sample_rate;
				rframe->channel_layout = codec_context->channel_layout;
				rframe->pkt_duration = 0;
				decoder->samples_size = codec_context->sample_rate/10;
				decoder->align = 1;
				if(codec_context->frame_size > decoder->samples_size)
					decoder->samples_size = codec_context->frame_size;
				if((ret=av_samples_alloc(rframe->data, rframe->linesize, codec_context->channels, decoder->samples_size, codec_context->sample_fmt, decoder->align))>=0) {
					if (process) {
						res = process(decoder->stream, decoder, arg, io_s);
					}
				} else {
					res = print_error(ret, io_s->stderr);
				}
			} else {
				if (process) {
					decoder->align = 64;
					res = process(decoder->stream, decoder, arg, io_s);
				}
			}
			av_frame_free(&decoder->rframe);
		} else {
			res = -1;
			fprintf(io_s->stderr, "Error[libwrpffp]: failed to alloc AVFrame\n");
		}
		av_frame_free(&decoder->wframe);
	} else {
		res = -1;
		fprintf(io_s->stderr, "Error[libwrpffp]: failed to alloc AVFrame\n");
	}
	return res;
}

int ffmpeg_open_decoder(ffmpeg_stream *stream, void *arg, int(*process)(ffmpeg_stream *, ffmpeg_decoder *, void *, io_stream *) , io_stream *io_s) {
	int res = 0, ret;
	AVCodec *codec;
	ffmpeg_decoder decoder = {
		.stream = stream,
	};
	if((codec = avcodec_find_decoder(stream->stream->codecpar->codec_id))) {
		if ((decoder.codec_context = avcodec_alloc_context3(codec))) {
			if ((ret=avcodec_parameters_to_context(decoder.codec_context, stream->stream->codecpar)) >= 0
					&& (!(ret=avcodec_open2(decoder.codec_context, codec, NULL)))) {
				guess_avg_duration(&decoder);
				res = open_for_media(&decoder, arg, process, io_s);
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
	else {
		avcodec_send_packet(decoder->codec_context, NULL);
		decoder->stream_ended = 1;
	}
	return res;
}

int ffmpeg_decoded_size(ffmpeg_decoder *decoder) {
	AVCodecContext *codec_context = decoder->codec_context;
	switch(codec_context->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			return av_image_get_buffer_size(codec_context->pix_fmt, codec_context->width, codec_context->height, decoder->align);
		case AVMEDIA_TYPE_AUDIO:
			return av_samples_get_buffer_size(NULL, codec_context->channels,
					decoder->samples_size, codec_context->sample_fmt, decoder->align);
		default:
			not_support_media_type(codec_context->codec_type);
			break;
	}
	return -1;
}

static inline int output_frame(ffmpeg_decoder *decoder, ffmpeg_frame *frame, AVFrame *avframe, void *arg, int (*action)(ffmpeg_decoder *, ffmpeg_frame *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	frame->frame = avframe;
	if(action) {
		res = action(decoder, frame, arg, io_s);
	}
	avframe->nb_samples = 0;
	avframe->pkt_duration = 0;
	return res;
}

int ffmpeg_decode(ffmpeg_decoder *decoder, void *arg, int (*action)(ffmpeg_decoder *, ffmpeg_frame *, void *, io_stream *), io_stream *io_s) {
	int res = 0, ret;
	ffmpeg_frame fffrm = {
		.decoder = decoder,
		.codec_type = decoder->codec_context->codec_type,
		.align = decoder->align,
	};
	AVCodecContext *codec_context = decoder->codec_context;
	AVFrame *wframe = decoder->wframe;
	AVFrame *rframe = decoder->rframe;

	if(!(res=avcodec_receive_frame(codec_context, wframe))) {
		switch(codec_context->codec_type) {
			case AVMEDIA_TYPE_VIDEO:
				fffrm.frame = wframe;
				res = action(decoder, &fffrm, arg, io_s);
				break;
			case AVMEDIA_TYPE_AUDIO:
				if(rframe->nb_samples + wframe->nb_samples > decoder->samples_size)
					res = output_frame(decoder, &fffrm, rframe, arg, action, io_s);
				if(!rframe->nb_samples)
					av_frame_set_best_effort_timestamp(rframe, av_frame_get_best_effort_timestamp(wframe));
				av_samples_copy(rframe->data, wframe->data,
						rframe->nb_samples, 0,
						wframe->nb_samples,
						wframe->channels, wframe->format);
				rframe->nb_samples += wframe->nb_samples;
				break;
			default:
				not_support_media_type(codec_context->codec_type);
				break;
		}
	}
	if(decoder->stream_ended && decoder->rframe->nb_samples)
		output_frame(decoder, &fffrm, rframe, arg, action, io_s);
	return res;
}

static inline int64_t guess_duration(const AVFrame *frame, const AVCodecContext *codec_context) {
	switch(codec_context->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			return 33367;
		case AVMEDIA_TYPE_AUDIO:
			return samples_to_duration(frame->nb_samples, codec_context);
		default:
			not_support_media_type(codec_context->codec_type);
			break;
	}
	return -1;
}

int64_t ffmpeg_frame_present_timestamp(const ffmpeg_frame *frame) {
	ffmpeg_decoder *decoder = frame->decoder;
	AVFrame *avframe = frame->frame;
	AVStream *stream = decoder->stream->stream;

	int64_t pts = av_frame_get_best_effort_timestamp(avframe);
	if(AV_NOPTS_VALUE == pts)
		decoder->_prev_pts += decoder->_prev_duration;
	else
		decoder->_prev_pts = av_rescale_q(pts-stream->start_time, stream->time_base, AV_TIME_BASE_Q);

	if(avframe->pkt_duration)
		decoder->_prev_duration = av_rescale_q(avframe->pkt_duration, stream->time_base, AV_TIME_BASE_Q);
	else if(decoder->_avg_duration)
		decoder->_prev_duration = decoder->_avg_duration;
	else
		decoder->_prev_duration = guess_duration(avframe, decoder->codec_context);

	return decoder->_prev_pts;
}

const char *ffmpeg_media_info(const ffmpeg_decoder *decoder) {
	static __thread char buffer[1024];
	AVCodecContext *codec_context = decoder->codec_context;
	switch(codec_context->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			sprintf(buffer, "VFS w:%d h:%d fmt:%d",
					codec_context->width,
					codec_context->height,
					codec_context->pix_fmt);
			break;
		case AVMEDIA_TYPE_AUDIO:
			sprintf(buffer, "AFS rt:%d ch:%d fmt:%d buf:%d lay:%"PRIu64,
					codec_context->sample_rate,
					codec_context->channels,
					codec_context->sample_fmt,
					decoder->samples_size,
					codec_context->channel_layout);
			break;
		default:
			not_support_media_type(codec_context->codec_type);
			break;
	}

	return buffer;
}

const char *ffmpeg_frame_info(const ffmpeg_frame *frame) {
	static __thread char buffer[1024];
	switch(frame->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			sprintf(buffer, "%"PRId64, ffmpeg_frame_present_timestamp(frame));
			break;
		case AVMEDIA_TYPE_AUDIO:
			sprintf(buffer, "%"PRId64",%d", ffmpeg_frame_present_timestamp(frame), frame->frame->nb_samples);
			break;
		default:
			not_support_media_type(frame->codec_type);
			break;
	}
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
			memcpy(buf, avframe->data[0], av_samples_get_buffer_size(NULL, avframe->channels, frame->decoder->samples_size, avframe->format, frame->align));
			break;
		default:
			not_support_media_type(frame->codec_type);
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
