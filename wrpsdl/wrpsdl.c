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

int sdl_present(sdl_window *window, const video_frames *vfs, uint8_t **datas, int *lines, io_stream *io_s) {
	int res = 0;
	if(!SDL_UpdateYUVTexture(window->texture, NULL, datas[0], lines[0], datas[1], lines[1], datas[2], lines[2]) && 
			!SDL_RenderCopy(window->renderer, window->texture,  NULL, NULL))
		SDL_RenderPresent(window->renderer);
	else
		res = print_error(io_s->stderr);
	return res;
}

int sdl_init_audio(int dev, void *arg, int(*action)(sdl_audio *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	if(!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		sdl_audio audio = {
			.device_name = SDL_GetAudioDeviceName(dev, 0),
		};
		if(audio.device_name) {
			if(action)
				res = action(&audio, arg, io_s);
			if(audio.device_id)
				SDL_CloseAudioDevice(audio.device_id);
		} else {
			res = -1;
			fprintf(io_s->stderr, "Error[libwrpsdl]: get device name at [%d] failed\n", dev);
			print_stack(io_s->stderr);
		}
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	} else {
		res = print_error(io_s->stderr);
	}
	return res;
}

static int is_same_audio(sdl_audio *audio, const SDL_AudioSpec *desired) {
	return audio->freq==desired->freq && audio->channels==desired->channels && audio->format==desired->format;
}

static int not_meet_desired(const SDL_AudioSpec *desired, const SDL_AudioSpec *obtained) {
	return desired->freq != obtained->freq || desired->channels != obtained->channels || desired->format != obtained->format;
}

int sdl_reload_audio(sdl_audio *audio, int freq, int channels, SDL_AudioFormat format, void *arg, int(*action)(sdl_audio *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	SDL_AudioSpec obtained = {};
	SDL_AudioSpec desired = {
		.freq = freq,
		.channels = channels,
		.format = format,
	};

	if(audio->device_id) {
		if(is_same_audio(audio, &desired)) {
			if(action)
				res = action(audio, arg, io_s);
			return res;
		} else
			SDL_CloseAudioDevice(audio->device_id);
	}

	if(audio->device_id = SDL_OpenAudioDevice(audio->device_name, 0, &desired, &obtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) {
		audio->freq = freq;
		audio->channels = channels;
		audio->format = format;
		if(not_meet_desired(&desired, &obtained))
			fprintf(io_s->stderr, "Warning[libwrpsdl]: not support [%d %d %d] on device [%s]\n", freq, channels, format, audio->device_name);
		if(action)
			res = action(audio, arg, io_s);
	} else
		res = print_error(io_s->stderr);
	return res;
}

int sdl_play_audio(sdl_audio *audio, const void *buffer, size_t buffer_len) {
	if(!audio->started) {
		SDL_PauseAudioDevice(audio->device_id, 0);
		audio->started = 1;
	}
	return SDL_QueueAudio(audio->device_id, buffer, buffer_len);
}
