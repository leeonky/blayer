#include "wrpsdl.h"

int sdl_open_window(const char *title, int x, int y, int w, int h, Uint32 flag, void *arg, int(*process)(sdl_window *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	sdl_window window;
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	window.window = SDL_CreateWindow(title, x, y, w, h, flag);
	window.renderer = SDL_CreateRenderer(window.window, -1, SDL_RENDERER_ACCELERATED);
	if(process) {
		res = process(&window, arg, io_s);
	}
	SDL_DestroyRenderer(window.renderer);
	SDL_DestroyWindow(window.window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return res;
}
