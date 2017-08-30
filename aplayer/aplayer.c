#include <libavutil/imgutils.h>
#include <unistd.h>
#include <time.h>
#include "bputil/bputil.h"
#include "wrpsdl/wrpsdl.h"
#include "wrpffp/wrpffp.h"
#include "iob/iob.h"
#include "iob/afs.h"
#include "aplayer.h"
#include "SDL2/SDL.h"

typedef struct app_context {
	sdl_audio *audio;
	ffmpeg_frame *frame;
	shm_cbuf *cbuf;
	const audio_frames *frames;
} app_context;

/*static mclock mclk;*/

static SDL_AudioFormat parse_format(enum AVSampleFormat format) {
	switch(format) {
		case AV_SAMPLE_FMT_U8:
			return AUDIO_U8;
		case AV_SAMPLE_FMT_S16:
			return AUDIO_S16;
		case AV_SAMPLE_FMT_S32:
			return AUDIO_S32;
		case AV_SAMPLE_FMT_FLT:
			return AUDIO_F32;
		case AV_SAMPLE_FMT_DBL:
		case AV_SAMPLE_FMT_U8P:
		case AV_SAMPLE_FMT_S16P:
		case AV_SAMPLE_FMT_S32P:
		case AV_SAMPLE_FMT_FLTP:
		case AV_SAMPLE_FMT_DBLP:
		case AV_SAMPLE_FMT_S64:
		case AV_SAMPLE_FMT_S64P:
		default:
			return 0;
	}
}

static int process_frame(shm_cbuf *cb, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	const audio_frames *afs = context_arg->frames;
	ffmpeg_frame *frame = context_arg->frame;
	int i;

	for(i=0; i<afs->count; ++i) {
		int size = av_samples_get_buffer_size(NULL, afs->channels, afs->frames[i].samples_size, afs->format, afs->align);
		sdl_play_audio(context_arg->audio, shrb_get(cb, afs->frames[i].index), size);
		/*if(!ffmpeg_load_image(frame, afs, shrb_get(cb, afs->frames[i].index), io_s)) {*/
			/*if(!mclk_waiting(&mclk, afs->frames[i].pts, 300000))*/
				/*sdl_present(context_arg->window, afs, frame->frame->data, frame->frame->linesize, io_s);*/
			/*else*/
				/*fprintf(io_s->stderr, "Error[aplayer]: skip frame\n");*/
		/*}*/
		shrb_free(cb);
	}
	return 0;
}

static int audio_reloaded(sdl_audio *audio, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	const audio_frames *afs = context_arg->frames;
	return shrb_reload(context_arg->cbuf, afs->cbuf_id, afs->cbuf_bits, afs->cbuf_size, arg, process_frame, io_s);
}

static void audio_frames_action(const audio_frames *afs, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	context_arg->frames = afs;
	sdl_reload_audio(context_arg->audio, afs->sample_rate, afs->channels, parse_format(afs->format), arg, audio_reloaded, io_s);
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_audio_frames_handler handler = {
		.arg = arg,
		.action = audio_frames_action,
	};

	iob_add_audio_frames_handler(iob, &handler);
	return 0;
}

static int cbuf_inited(shm_cbuf *cb, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	context_arg->cbuf = cb;
	return iob_main(arg, setup_frames_event, io_s);
}

static int ffmpeg_frame_created(ffmpeg_frame *frame, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	context_arg->frame = frame;
	return shrb_init(arg, cbuf_inited, io_s);
}

static int audio_action(sdl_audio *audio, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	context_arg->audio = audio;
	return ffmpeg_create_frame(arg, ffmpeg_frame_created, io_s);
}

int aplayer_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	app_context context_arg = {};
	io_stream io_s = {stdin, stdout, stderr};

	return sdl_init_audio(0, &context_arg, audio_action, &io_s);
}

