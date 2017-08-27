#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include <SDL2/SDL.h>
#include "bputil/bputil.h"
#include "mock_sdl/mock_sdl.h"
#include "wrpsdl/wrpsdl.h"

SUITE_START("sdl_init_audio_test");

static int arg_dev;
static void *arg_arg;
static io_stream arg_io_s;
static const char *arg_dev_name;
static SDL_AudioDeviceID arg_device_id;

mock_function_3(int, sdl_init_audio_action, sdl_audio *, void *, io_stream *);

static const char *stub_SDL_GetAudioDeviceName(int dev, int flag) {
	return arg_dev_name;
}

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_dev_name = "default";

	init_mock_function(sdl_init_audio_action, NULL);
	init_mock_function(SDL_InitSubSystem, NULL);
	init_mock_function(SDL_QuitSubSystem, NULL);
	init_mock_function(SDL_GetAudioDeviceName, stub_SDL_GetAudioDeviceName);
	init_mock_function(SDL_CloseAudioDevice, NULL);

	arg_arg = &arg_arg;
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return sdl_init_audio(arg_dev, arg_arg, sdl_init_audio_action, &arg_io_s);
}

static int sdl_init_audio_assert(sdl_audio *audio, void *arg, io_stream *io_s) {
	CUE_ASSERT_STRING_EQ(audio->device_name, arg_dev_name);
}

SUITE_CASE("init sdl audio with device") {
	arg_dev = 10;
	arg_dev_name = "hdmi";
	init_mock_function(SDL_GetAudioDeviceName, stub_SDL_GetAudioDeviceName);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(SDL_InitSubSystem);
	CUE_EXPECT_CALLED_WITH_INT(SDL_InitSubSystem, 1, SDL_INIT_AUDIO);

	CUE_EXPECT_CALLED_ONCE(SDL_GetAudioDeviceName);
	CUE_EXPECT_CALLED_WITH_INT(SDL_GetAudioDeviceName, 1, arg_dev);
	CUE_EXPECT_CALLED_WITH_INT(SDL_GetAudioDeviceName, 2, 0);

	CUE_EXPECT_CALLED_ONCE(sdl_init_audio_action);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_init_audio_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_init_audio_action, 3, &arg_io_s);

	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);
	CUE_EXPECT_CALLED_WITH_INT(SDL_QuitSubSystem, 1, SDL_INIT_AUDIO);
}

static int stub_SDL_InitSubSystem_failed(Uint32 f) {
	return -1;
}

SUITE_CASE("failed to init audio sub module") {
	init_mock_function(SDL_InitSubSystem, stub_SDL_InitSubSystem_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(SDL_GetAudioDeviceName);

	CUE_EXPECT_NEVER_CALLED(sdl_init_audio_action);

	CUE_EXPECT_NEVER_CALLED(SDL_QuitSubSystem);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");
}

static const char *stub_SDL_GetAudioDeviceName_failed(int dev, int flag) {
	return NULL;
}

SUITE_CASE("device not found") {
	arg_dev = 1;
	init_mock_function(SDL_GetAudioDeviceName, stub_SDL_GetAudioDeviceName_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(sdl_init_audio_action);

	CUE_EXPECT_CALLED_ONCE(SDL_QuitSubSystem);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: get device name at [1] failed\n");
}

static int sdl_init_audio_opened(sdl_audio *audio, void *arg, io_stream *io_s) {
	audio->device_id = arg_device_id;
	return 100;
}

SUITE_CASE("close if device opened and return action return code") {
	arg_device_id = 1;

	init_mock_function(sdl_init_audio_action, sdl_init_audio_opened);

	CUE_ASSERT_SUBJECT_FAILED_WITH(100);

	CUE_EXPECT_CALLED_ONCE(SDL_CloseAudioDevice);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CloseAudioDevice, 1, arg_device_id);
}

SUITE_END(sdl_init_audio_test);
