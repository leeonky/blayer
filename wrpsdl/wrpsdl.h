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

extern int sdl_present(sdl_window *, const video_frames *, uint8_t **, int *, io_stream *);

typedef struct sdl_audio {
	const char *device_name;
	SDL_AudioDeviceID device_id;
	int freq, channels;
	SDL_AudioFormat format;
} sdl_audio;

extern int sdl_init_audio(int, void *, int(*)(sdl_audio *, void *, io_stream *), io_stream *);
extern int sdl_reload_audio(sdl_audio *, int, int, SDL_AudioFormat, void *, int(*)(sdl_audio *, void *, io_stream *), io_stream *);

#endif
