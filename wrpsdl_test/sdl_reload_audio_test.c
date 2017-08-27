#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include <SDL2/SDL.h>
#include "bputil/bputil.h"
#include "mock_sdl/mock_sdl.h"
#include "wrpsdl/wrpsdl.h"

SUITE_START("sdl_reload_audio_test");

static sdl_audio arg_audio;
static int arg_freq;
static SDL_AudioFormat arg_format;
static Uint8 arg_channels;
static io_stream arg_io_s;
static const char *arg_dev_name;
static void *arg_arg;
static arg_device_id;

mock_function_3(int, sdl_reload_audio_action, sdl_audio *, void *, io_stream *);

static int stub_SDL_OpenAudioDevice(const char *device, int iscapture, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained, int allowed_changes) {
	return arg_device_id;
}

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_dev_name = "default";
	arg_audio.device_id = 0;
	arg_audio.device_name = arg_dev_name;
	arg_audio.format = 0;
	arg_device_id = 100;

	init_mock_function(sdl_reload_audio_action, NULL);
	init_mock_function(SDL_OpenAudioDevice, stub_SDL_OpenAudioDevice);
	init_mock_function(SDL_CloseAudioDevice, NULL);

	arg_arg = &arg_arg;
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return sdl_reload_audio(&arg_audio, arg_freq, arg_channels, arg_format, arg_arg, sdl_reload_audio_action, &arg_io_s);
}

static int stub_SDL_OpenAudioDevice_assert(const char *device, int iscapture, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained, int allowed_changes) {
	CUE_ASSERT_EQ(desired->freq, arg_freq);
	CUE_ASSERT_EQ(desired->format, arg_format);
	CUE_ASSERT_EQ(desired->channels, arg_channels);
	CUE_ASSERT_PTR_EQ(desired->callback, NULL);
	CUE_ASSERT_PTR_EQ(desired->userdata, NULL);
	*obtained = *desired;
	return arg_device_id;
}

SUITE_CASE("open device first") {
	arg_audio.device_id = 0;
	arg_audio.device_name = arg_dev_name = "hdmi";
	arg_freq = 44100;
	arg_format = 100;
	arg_channels = 8;
	arg_device_id = 1024;
	init_mock_function(SDL_OpenAudioDevice, stub_SDL_OpenAudioDevice_assert);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(SDL_OpenAudioDevice);

	CUE_EXPECT_CALLED_WITH_STRING(SDL_OpenAudioDevice, 1, arg_dev_name);
	CUE_EXPECT_CALLED_WITH_INT(SDL_OpenAudioDevice, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(SDL_OpenAudioDevice, 5, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

	CUE_EXPECT_CALLED_ONCE(sdl_reload_audio_action);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_reload_audio_action, 1, &arg_audio);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_reload_audio_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_reload_audio_action, 3, &arg_io_s);

	CUE_ASSERT_EQ(arg_audio.device_id, arg_device_id);
	CUE_ASSERT_EQ(arg_audio.freq, arg_freq);
	CUE_ASSERT_EQ(arg_audio.format, arg_format);
	CUE_ASSERT_EQ(arg_audio.channels, arg_channels);
}

SUITE_CASE("sound card does not meet specifation") {
	arg_audio.device_id = 0;
	arg_audio.device_name = "hdmi";
	arg_freq = 44100;
	arg_format = 100;
	arg_channels = 8;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(sdl_reload_audio_action);

	CUE_ASSERT_STDERR_EQ("Warning[libwrpsdl]: not support [44100 8 100] on device [hdmi]\n");
}

static int stub_SDL_OpenAudioDevice_failed(const char *device, int iscapture, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained, int allowed_changes) {
	return 0;
}

SUITE_CASE("failed to open device") {
	init_mock_function(SDL_OpenAudioDevice, stub_SDL_OpenAudioDevice_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(sdl_reload_audio_action);

	CUE_ASSERT_STDERR_EQ("Error[libwrpsdl]: sdl error\n");
}

SUITE_CASE("reload with same format") {
	arg_audio.device_id = arg_device_id = 1024;
	arg_audio.freq = arg_freq = 44100;
	arg_audio.format = arg_format = 100;
	arg_audio.channels = arg_channels = 8;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_NEVER_CALLED(SDL_OpenAudioDevice);

	CUE_EXPECT_CALLED_ONCE(sdl_reload_audio_action);
}

SUITE_CASE("different audio format") {
	arg_audio.device_id = arg_device_id = 1024;
	arg_audio.freq = arg_freq = 44100;
	arg_audio.format = arg_format = 100;
	arg_audio.channels = 6;
	arg_audio.started = 1;
	arg_channels = 8;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(SDL_CloseAudioDevice);
	CUE_EXPECT_CALLED_WITH_INT(SDL_CloseAudioDevice, 1, arg_device_id);

	CUE_EXPECT_CALLED_WITH_STRING(SDL_OpenAudioDevice, 1, arg_dev_name);
	CUE_EXPECT_CALLED_WITH_INT(SDL_OpenAudioDevice, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(SDL_OpenAudioDevice, 5, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

	CUE_EXPECT_CALLED_ONCE(sdl_reload_audio_action);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_reload_audio_action, 1, &arg_audio);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_reload_audio_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(sdl_reload_audio_action, 3, &arg_io_s);

	CUE_ASSERT_EQ(arg_audio.started, 0);
}

SUITE_END(sdl_reload_audio_test);
