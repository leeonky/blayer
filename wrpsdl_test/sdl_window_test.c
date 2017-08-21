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
static SDL_Texture *texture;

static SDL_Window *stub_SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flag) {
	return window;
}

static SDL_Renderer *stub_SDL_CreateRenderer(SDL_Window *w, int index, Uint32 flag) {
	return renderer;
}

static SDL_Texture *stub_SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h) {
	return texture;
}

static int arg_texture_w, arg_texture_h;
static void stub_SDL_GL_GetDrawableSize(SDL_Window *window, int *w, int *h) {
	*w = arg_texture_w;
	*h = arg_texture_h;
}

BEFORE_EACH() {
	int_arg = 0;
	window = (SDL_Window *)&window;
	renderer = (SDL_Renderer *)&renderer;
	texture = (SDL_Texture *)&texture;
	arg_texture_w = 400;
	arg_texture_h = 300;

	init_subject("");

	init_mock_function(SDL_InitSubSystem, NULL);
	init_mock_function(SDL_CreateWindow, stub_SDL_CreateWindow);
	init_mock_function(SDL_CreateRenderer, stub_SDL_CreateRenderer);
	init_mock_function(SDL_DestroyRenderer, NULL);
	init_mock_function(SDL_DestroyWindow, NULL);
	init_mock_function(SDL_QuitSubSystem, NULL);
	init_mock_function(SDL_ShowCursor, NULL);
	init_mock_function(SDL_CreateTexture, stub_SDL_CreateTexture);
	init_mock_function(SDL_DestroyTexture, NULL);
	init_mock_function(SDL_GL_GetDrawableSize, stub_SDL_GL_GetDrawableSize);
	return 0;
}

AFTER_EACH() {
	close_subject();
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

SUITE_CASE("open window with args and get window and renderer and texture") {
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

	CUE_EXPECT_CALLED_ONCE(SDL_ShowCursor);
	CUE_EXPECT_CALLED_WITH_INT(SDL_ShowCursor, 1, SDL_DISABLE);

	CUE_EXPECT_CALLED_ONCE(SDL_CreateRenderer);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_CreateRenderer, 1, window);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateRenderer, 2, -1);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateRenderer, 3, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_WINDOW_ALLOW_HIGHDPI);

	CUE_EXPECT_CALLED_ONCE(SDL_CreateTexture);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_CreateTexture, 1, renderer);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateTexture, 2, SDL_PIXELFORMAT_IYUV);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateTexture, 3, SDL_TEXTUREACCESS_STREAMING);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateTexture, 4, arg_texture_w);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CreateTexture, 5, arg_texture_h);

	CUE_ASSERT_EQ(int_arg, 100);

	CUE_EXPECT_CALLED_ONCE(SDL_DestroyTexture);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_DestroyTexture, 1, texture);

	CUE_EXPECT_CALLED_ONCE(SDL_DestroyRenderer);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_DestroyRenderer, 1, renderer);

	CUE_EXPECT_CALLED_ONCE(SDL_DestroyWindow);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_DestroyWindow, 1, window);

	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);
	CUE_EXPECT_CALLED_WITH_INT(SDL_QuitSubSystem, 1, SDL_INIT_VIDEO);
}

static int stub_SDL_InitSubSystem_error(Uint32 f) {
	return -1;
}

SUITE_CASE("failed to init video") {
	init_mock_function(SDL_InitSubSystem, stub_SDL_InitSubSystem_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");

	CUE_EXPECT_NEVER_CALLED(SDL_CreateWindow);
	CUE_EXPECT_NEVER_CALLED(SDL_CreateRenderer);
	CUE_EXPECT_NEVER_CALLED(SDL_CreateTexture);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyTexture);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyRenderer);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyWindow);
	CUE_EXPECT_NEVER_CALLED(SDL_QuitSubSystem);
}

static SDL_Window *stub_SDL_CreateWindow_error(const char *title, int x, int y, int w, int h, Uint32 flag) {
	return NULL;
}

SUITE_CASE("failed to create window") {
	init_mock_function(SDL_CreateWindow, stub_SDL_CreateWindow_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");

	CUE_EXPECT_NEVER_CALLED(SDL_CreateRenderer);
	CUE_EXPECT_NEVER_CALLED(SDL_CreateTexture);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyTexture);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyRenderer);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyWindow);
	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);
}

static SDL_Renderer *stub_SDL_CreateRenderer_error(SDL_Window *w, int index, Uint32 flag) {
	return NULL;
}

SUITE_CASE("failed to create renderer") {
	init_mock_function(SDL_CreateRenderer, stub_SDL_CreateRenderer_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");

	CUE_EXPECT_NEVER_CALLED(SDL_CreateTexture);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyTexture);
	CUE_EXPECT_NEVER_CALLED(SDL_DestroyRenderer);
	CUE_EXPECT_CALLED_ONCE(SDL_DestroyWindow);
	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);
}

static SDL_Texture *stub_SDL_CreateTexture_failed(SDL_Renderer *rd, Uint32 format, int mode, int w, int h) {
	return NULL;
}

SUITE_CASE("failed to create texture") {
	init_mock_function(SDL_CreateTexture, stub_SDL_CreateTexture_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");

	CUE_EXPECT_NEVER_CALLED(SDL_DestroyTexture);
	CUE_EXPECT_CALLED_ONCE(SDL_DestroyRenderer);
	CUE_EXPECT_CALLED_ONCE(SDL_DestroyWindow);
	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);
}

static SDL_Renderer *stub_SDL_CreateRenderer_allow_soft_mode(SDL_Window *w, int index, Uint32 flag) {
	if (SDL_RENDERER_SOFTWARE == flag)
		return renderer;
	return NULL;
}

SUITE_CASE("try to create renderer in soft mode") {
	init_mock_function(SDL_CreateRenderer, stub_SDL_CreateRenderer_allow_soft_mode);

	CUE_ASSERT_SUBJECT_FAILED_WITH(10);

	CUE_EXPECT_CALLED_ONCE(SDL_CreateTexture);
	CUE_EXPECT_CALLED_ONCE(SDL_DestroyTexture);
	CUE_EXPECT_CALLED_ONCE(SDL_DestroyRenderer);
	CUE_EXPECT_CALLED_ONCE(SDL_DestroyWindow);
	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);
}

SUITE_END(sdl_open_window_test);
