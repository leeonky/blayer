#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include <SDL2/SDL.h>
#include <libavutil/imgutils.h>
#include "bputil/bputil.h"
#include "mock_sdl/mock_sdl.h"
#include "wrpsdl/wrpsdl.h"
#include "iob/iob.h"
#include "iob/vfs.h"

SUITE_START("sdl_present_test");

static SDL_Renderer *arg_renderer;
static SDL_Texture *arg_texture;
static sdl_window arg_window;
static uint8_t *arg_datas[3];
static int arg_lines[3];
static io_stream arg_io_s;
static int arg_line_0, arg_line_1, arg_line_2;
static uint8_t *arg_data_0, *arg_data_1, *arg_data_2;

BEFORE_EACH() {
	arg_renderer = (SDL_Renderer *)&arg_renderer;
	arg_texture = (SDL_Texture *)&arg_texture;
	arg_window.renderer = arg_renderer;
	arg_window.texture = arg_texture;

	arg_lines[0] = arg_line_0 = 100;
	arg_lines[1] = arg_line_1 = 50;
	arg_lines[2] = arg_line_2 = 25;

	arg_datas[0] = arg_data_0 = (uint8_t *)10000;
	arg_datas[1] = arg_data_1 = (uint8_t *)100000;
	arg_datas[2] = arg_data_2 = (uint8_t *)1000000;

	init_subject("");

	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;
	init_mock_function(SDL_UpdateYUVTexture, NULL);
	init_mock_function(SDL_RenderCopy, NULL);
	init_mock_function(SDL_RenderPresent, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return sdl_present(&arg_window, arg_datas, arg_lines, &arg_io_s);
}

SUITE_CASE("present video frame with index") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(SDL_UpdateYUVTexture);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_UpdateYUVTexture, 1, arg_texture);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_UpdateYUVTexture, 2, NULL);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_UpdateYUVTexture, 3, arg_data_0);
	CUE_EXPECT_CALLED_WITH_INT(SDL_UpdateYUVTexture, 4, arg_line_0);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_UpdateYUVTexture, 5, arg_data_1);
	CUE_EXPECT_CALLED_WITH_INT(SDL_UpdateYUVTexture, 6, arg_line_1);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_UpdateYUVTexture, 7, arg_data_2);
	CUE_EXPECT_CALLED_WITH_INT(SDL_UpdateYUVTexture, 8, arg_line_2);

	CUE_EXPECT_CALLED_ONCE(SDL_RenderCopy);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_RenderCopy, 1, arg_renderer);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_RenderCopy, 2, arg_texture);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_RenderCopy, 3, NULL);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_RenderCopy, 4, NULL);

	CUE_EXPECT_CALLED_ONCE(SDL_RenderPresent);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_RenderPresent, 1, arg_renderer);
}

static int stub_SDL_UpdateYUVTexture_failed(SDL_Texture *t, const SDL_Rect *r, const Uint8 *d0, int l0, const Uint8 *d1, int l1, const Uint8 *d2, int l2) {
	return -1;
}

SUITE_CASE("failed to update texture") {
	init_mock_function(SDL_UpdateYUVTexture, stub_SDL_UpdateYUVTexture_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(SDL_RenderCopy);

	CUE_EXPECT_NEVER_CALLED(SDL_RenderPresent);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");
}

static int stub_SDL_RenderCopy_failed(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect) {
	return -1;
}

SUITE_CASE("failed to renderer copy") {
	init_mock_function(SDL_RenderCopy, stub_SDL_RenderCopy_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(SDL_RenderPresent);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");
}

SUITE_END(sdl_present_test);
