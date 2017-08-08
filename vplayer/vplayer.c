#include "bputil/bputil.h"
#include "wrpsdl/wrpsdl.h"
#include "iob/iob.h"
#include "iob/vfs.h"
#include "vplayer.h"
#include "SDL2/SDL.h"

sdl_window *wnd;
SDL_Texture * sdlTexture;

static int process_frame(shm_cbuf *cb, void *arg, io_stream *io_s) {
	video_frames *vfs = (video_frames *)arg;
	uint8_t *dst_data[4];
	int dst_linesize[4];
	int i;

	for(i=0; i<vfs->count; ++i) {
		av_image_fill_arrays(dst_data, dst_linesize, shrb_get(cb, vfs->frames[i].index), vfs->format, vfs->width, vfs->height, vfs->align);

		SDL_Rect rect = {0, 0, vfs->width, vfs->height};

		SDL_UpdateYUVTexture(sdlTexture, &rect,  
				dst_data[0], dst_linesize[0],  
				dst_data[1], dst_linesize[1],  
				dst_data[2], dst_linesize[2]); 
		SDL_RenderCopy(wnd->renderer, sdlTexture,  NULL, &rect);    
		SDL_RenderPresent(wnd->renderer);
	}
}

static void process_frames(const video_frames *vfs, void *arg, io_stream *io_s) {
	shrb_load(vfs->cbuf_id, 4, vfs->element_size, (void *)vfs, process_frame, io_s);
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_video_frames_handler handler = {
		.action = process_frames,
	};

	iob_add_video_frames_handler(iob, &handler);
	return 0;
}

static int process_video(sdl_window *window, void *arg, io_stream *io_s) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	wnd = window;

	sdlTexture = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, 1920, 1080);
	iob_main(NULL, setup_frames_event, io_s);
	return 0;
}

int vplayer_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	const char *input = argv[1];
	FILE *input_file = fopen(input, "rt");
	io_stream io_s = {input_file, stdout, stderr};
	sdl_open_window("test", 0, 0, 1920, 1080, 0, NULL, process_video, &io_s);
	fclose(input_file);
	return 0;
}

