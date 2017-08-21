#ifndef WRPSDL_
#define WRPSDL_

#include <SDL2/SDL.h>
#include "iob/iob.h"
#include "iob/vfs.h"
#include "bputil/bputil.h"

typedef struct sdl_window {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
} sdl_window;

extern int sdl_open_window(const char *, int, int, int, int, Uint32, void *, int(*)(sdl_window *, void *, io_stream *), io_stream *);

extern int sdl_present(sdl_window *, video_frames *, uint8_t **, int *, io_stream *);

#endif
