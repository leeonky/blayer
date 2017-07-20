#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include <SDL2/SDL.h>
#include "bputil/bputil.h"
#include "mock_sdl/mock_sdl.h"
#include "wrpsdl/wrpsdl.h"

SUITE_START("sdl_open_window_test");

static int int_arg;
static SDL_Window *window;
static SDL_Renderer *renderer;

static SDL_Window *stub_SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flag) {
	return window;
}

static SDL_Renderer *stub_SDL_CreateRenderer(SDL_Window *w, int index, Uint32 flag) {
	return renderer;
}

BEFORE_EACH() {
	int_arg = 0;
	window = (SDL_Window *)&window;
	renderer = (SDL_Renderer *)&renderer;

	init_mock_function(SDL_InitSubSystem, NULL);
	init_mock_function(SDL_CreateWindow, stub_SDL_CreateWindow);
	init_mock_function(SDL_CreateRenderer, stub_SDL_CreateRenderer);
	init_mock_function(SDL_DestroyRenderer, NULL);
	init_mock_function(SDL_DestroyWindow, NULL);
	init_mock_function(SDL_QuitSubSystem, NULL);
	return 0;
}

static int process_window(sdl_window *window, void *arg, io_stream *io_s) {
	*(int *)arg = 100;
	return 10;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return sdl_open_window("test", 0, 0, 800, 400, SDL_WINDOW_RESIZABLE, &int_arg, process_window, &io_s);
}

SUITE_CASE("open window with args and get window and renderer") {
	CUE_ASSERT_SUBJECT_FAILED_WITH(10);

	CUE_EXPECT_CALLED_ONCE(SDL_InitSubSystem);
	CUE_EXPECT_CALLED_WITH_INT(SDL_InitSubSystem, 1, SDL_INIT_VIDEO);

	CUE_EXPECT_CALLED_ONCE(SDL_CreateWindow);
	CUE_EXPECT_CALLED_WITH_STRING(SDL_CreateWindow, 1, "test");
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateWindow, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateWindow, 3, 0);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateWindow, 4, 800);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateWindow, 5, 400);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateWindow, 6, SDL_WINDOW_RESIZABLE);

	CUE_EXPECT_CALLED_ONCE(SDL_CreateRenderer);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_CreateRenderer, 1, window);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateRenderer, 2, -1);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateRenderer, 3, SDL_RENDERER_ACCELERATED);

	CUE_ASSERT_EQ(int_arg, 100);

	CUE_EXPECT_CALLED_ONCE(SDL_DestroyRenderer);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_DestroyRenderer, 1, renderer);

	CUE_EXPECT_CALLED_ONCE(SDL_DestroyWindow);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_DestroyWindow, 1, window);

	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);
	CUE_EXPECT_CALLED_WITH_INT(SDL_QuitSubSystem, 1, SDL_INIT_VIDEO);
}

SUITE_END(sdl_open_window_test);
