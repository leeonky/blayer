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
				if((window.texture = SDL_CreateTexture(window.renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w, h))) {
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
