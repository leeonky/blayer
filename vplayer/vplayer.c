#include <libavutil/imgutils.h>
#include <unistd.h>
#include <time.h>
#include "bputil/bputil.h"
#include "wrpsdl/wrpsdl.h"
#include "wrpffp/wrpffp.h"
#include "iob/iob.h"
#include "iob/vfs.h"
#include "vplayer.h"
#include "SDL2/SDL.h"

typedef struct app_context {
	sdl_window *window;
	ffmpeg_frame *frame;
	shm_cbuf *cbuf;
	const video_frames *frames;
} app_context;

static mclock mclk;

static int process_frame(shm_cbuf *cb, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	const video_frames *vfs = context_arg->frames;
	ffmpeg_frame *frame = context_arg->frame;
	int i;

	for(i=0; i<vfs->count; ++i) {
		if(!ffmpeg_load_image(frame, vfs, shrb_get(cb, vfs->frames[i].index), io_s)) {
			if(!mclk_waiting(&mclk, vfs->frames[i].pts, 300000))
				sdl_present(context_arg->window, ffmpeg_frame_data(frame), ffmpeg_frame_linesize(frame), io_s);
			else
				fprintf(io_s->stderr, "Error[vplayer]: skip frame\n");
		}
		shrb_free(cb);
	}
	return 0;
}

static void video_frames_action(const video_frames *vfs, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	context_arg->frames = vfs;
	shrb_reload(context_arg->cbuf, vfs->cbuf_id, vfs->cbuf_bits, vfs->cbuf_size, arg, process_frame, io_s);
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_video_frames_handler handler = {
		.arg = arg,
		.action = video_frames_action,
	};

	iob_add_video_frames_handler(iob, &handler);

	mclk_init(&mclk);
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

static int video_action(sdl_window *window, void *arg, io_stream *io_s) {
	app_context *context_arg = (app_context *)arg;
	context_arg->window = window;
	return ffmpeg_create_frame(arg, ffmpeg_frame_created, io_s);
}

int vplayer_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	app_context context_arg = {};
	io_stream io_s = {stdin, stdout, stderr};

	return sdl_open_window("test", 0, 0, 1920, 1080, 0, &context_arg, video_action, &io_s);
}

