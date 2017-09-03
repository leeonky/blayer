#include "rsp.h"
#include "iob/iob.h"
#include "iob/afs.h"
#include "bputil/bputil.h"
#include "wrpffp/wrpffp.h"

typedef struct app_context {
	shm_cbuf *cbuf;
	ffmpeg_resampler *resampler;
	const audio_frames *afs;
	audio_frames out_afs;
} app_context;

static int audio_convertion(shm_cbuf *cb, void *arg, io_stream *io_s) {
	app_context *args = (app_context *)arg;
	audio_frames *afs = &args->out_afs;
	int i;
	for(i=0; i<afs->count; ++i) {
		audio_frame_index *afi = &afs->frames[i];
		void *data = shrb_get(cb, afi->index);
		if(!ffmpeg_resample(args->resampler, afi->samples_size, data, data, io_s)) {
			output_audio_frames(afs, io_s->stdout);
			fprintf(io_s->stdout, "\n");
			fflush(io_s->stdout);
		}
	}
	return 0;
}

static int reload_shrb(ffmpeg_resampler *rep, void *arg, io_stream *io_s) {
	app_context *args = (app_context *)arg;
	const audio_frames *afs = args->afs;

	return shrb_reload(args->cbuf, afs->cbuf_id, afs->cbuf_bits, afs->cbuf_size, arg, audio_convertion, io_s);
}

static void process_frames(const audio_frames *afs, void *arg, io_stream *io_s) {
	app_context *args = (app_context *)arg;
	args->afs = afs;
	ffmpeg_reload_resampler(((app_context *)arg)->resampler, afs, afs->layout, av_get_packed_sample_fmt(afs->format), &args->out_afs, arg, reload_shrb, io_s);
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_audio_frames_handler handler = {
		.arg = arg,
		.action = process_frames,
	};

	return iob_add_audio_frames_handler(iob, &handler);
}

static int shrb_inited(shm_cbuf *cb, void *arg, io_stream *io_s) {
	((app_context *)arg)->cbuf = cb;
	return iob_main(arg, setup_frames_event, io_s);
}

static int resampler_inited(ffmpeg_resampler *rsp, void *arg, io_stream *io_s) {
	((app_context *)arg)->resampler = rsp;
	return shrb_init(arg, shrb_inited, io_s);
}

int rsp_main(int argc, char **argv) {
	io_stream io_s = {stdin, stdout, stderr};
	app_context context;
	return ffmpeg_init_resampler(&context, resampler_inited, &io_s);
}
