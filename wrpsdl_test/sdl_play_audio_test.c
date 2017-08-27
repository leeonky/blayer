#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include <SDL2/SDL.h>
#include "bputil/bputil.h"
#include "mock_sdl/mock_sdl.h"
#include "wrpsdl/wrpsdl.h"

SUITE_START("sdl_play_audio_test");

static sdl_audio arg_audio;
static io_stream arg_io_s;
static void *arg_arg;
static void *arg_buffer;
static int arg_buffer_len;
static SDL_AudioDeviceID arg_devide_id;

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_arg = &arg_arg;
	arg_buffer = &arg_buffer;
	arg_buffer_len = 100;

	init_mock_function(SDL_PauseAudioDevice, NULL);
	init_mock_function(SDL_QueueAudio, NULL);

	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return sdl_play_audio(&arg_audio, arg_buffer, arg_buffer_len);
}

SUITE_CASE("sdl_play_audio resume first") {
	arg_audio.started = 0;
	arg_audio.device_id = arg_devide_id = 10;

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_EQ(arg_audio.started, 1);

	CUE_EXPECT_CALLED_ONCE(SDL_PauseAudioDevice);

	CUE_EXPECT_CALLED_WITH_INT(SDL_PauseAudioDevice, 1, arg_devide_id);
	CUE_EXPECT_CALLED_WITH_INT(SDL_PauseAudioDevice, 2, 0);
}

SUITE_CASE("feed data to hardware") {
	arg_audio.device_id = arg_devide_id = 10;
	arg_buffer_len = 100;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(SDL_QueueAudio);
	CUE_EXPECT_CALLED_WITH_INT(SDL_QueueAudio, 1, arg_devide_id);
	CUE_EXPECT_CALLED_WITH_PTR(SDL_QueueAudio, 2, arg_buffer);
	CUE_EXPECT_CALLED_WITH_INT(SDL_QueueAudio, 3, arg_buffer_len);
}

SUITE_END(sdl_play_audio_test);
