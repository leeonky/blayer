#include "wrpsdl.h"

static int print_error(FILE *stderr) {
	fprintf(stderr, "Error[libwrpsdl]: %s\n", SDL_GetError());
	print_stack(stderr);
	return -1;
}

int sdl_open_window(const char *title, int x, int y, int w, int h, Uint32 flag, void *arg, int(*process)(sdl_window *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	sdl_window window;
	if(!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		if((window.window = SDL_CreateWindow(title, x, y, w, h, flag))) {
			SDL_ShowCursor(SDL_DISABLE);
			if((window.renderer = SDL_CreateRenderer(window.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))
				||(window.renderer = SDL_CreateRenderer(window.window, -1, SDL_RENDERER_SOFTWARE))) {
				int tw, th;
				SDL_GL_GetDrawableSize(window.window, &tw, &th);
				if((window.texture = SDL_CreateTexture(window.renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, tw, th))) {
					if(process) {
						res = process(&window, arg, io_s);
					}
					SDL_DestroyTexture(window.texture);
				}else{
					res = print_error(io_s->stderr);
				}
				SDL_DestroyRenderer(window.renderer);
			}else{
				res = print_error(io_s->stderr);
			}
			SDL_DestroyWindow(window.window);
		} else {
			res = print_error(io_s->stderr);
		}
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	} else {
		res = print_error(io_s->stderr);
	}
	return res;
}

int sdl_present(sdl_window *window, video_frames *vfs, uint8_t **datas, int *lines, io_stream *io_s) {
	int res = 0;
	if(!SDL_UpdateYUVTexture(window->texture, NULL, datas[0], lines[0], datas[1], lines[1], datas[2], lines[2]) && 
			!SDL_RenderCopy(window->renderer, window->texture,  NULL, NULL))
		SDL_RenderPresent(window->renderer);
	else
		res = print_error(io_s->stderr);
	return res;
}
