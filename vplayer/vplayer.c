#include "bputil/bputil.h"
#include "vplayer.h"
#include "wrpsdl/wrpsdl.h"
#include "SDL2/SDL.h"

static int w, h, fmt;
static int lines[4];
static int64_t pts;
static int cid, cindex, align, size;
static const char *input;

sdl_window *wnd;
SDL_Texture * sdlTexture;

static int process_frame(shm_cbuf *cb, void *arg, io_stream *io_s) {
	uint8_t *dst_data[4];
	int dst_linesize[4];

	av_image_fill_arrays(dst_data, dst_linesize, shrb_get(cb, cindex), fmt, w, h, align);

	SDL_Rect rect = {0, 0, w, h};

	SDL_UpdateYUVTexture(sdlTexture, &rect,  
			dst_data[0], dst_linesize[0],  
			dst_data[1], dst_linesize[1],  
			dst_data[2], dst_linesize[2]); 
	SDL_RenderCopy(wnd->renderer, sdlTexture,  NULL, &rect);    
	SDL_RenderPresent(wnd->renderer);
}

static int process_video(sdl_window *window, void *arg, io_stream *io_s) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *input_file = fopen(input, "rt");
	wnd = window;

	sdlTexture = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, 1920, 1080);

	while ((read = getline(&line, &len, input_file))!=-1 && strcmp(line, "EXIT\n")!=0) {
	/*while ((read = getline(&line, &len, stdin))!=-1 && strcmp(line, "EXIT\n")!=0) {*/
		sscanf(line, "video_frame:: width:%d height:%d format:%d pts:%lld align:%d cbuf:%d index:%d size:%d\n", &w, &h, &fmt, &pts, &align, &cid, &cindex, &size);
		shrb_load(cid, 4, size, NULL, process_frame, io_s);
	}

	fclose(input_file);
}

int vplayer_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	io_stream io_s = {stdin, stdout, stderr};
	input = argv[1];
	return sdl_open_window("test", 0, 0, 1920, 1080, 0, NULL, process_video, &io_s);
}

