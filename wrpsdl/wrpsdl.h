#ifndef WRPSDL_
#define WRPSDL_

#include <SDL2/SDL.h>
#include "bputil/bputil.h"

typedef struct sdl_window {
	SDL_Window *window;
	SDL_Renderer *renderer;
} sdl_window;

extern int sdl_open_window(const char *, int, int, int, int, Uint32, void *, int(*)(sdl_window *, void *, io_stream *), io_stream *);

#endif
