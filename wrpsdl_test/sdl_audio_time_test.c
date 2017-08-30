#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_sdl/mock_sdl.h"
#include "mock_sys/mock_sys.h"
#include "wrpsdl/wrpsdl.h"

SUITE_START("sdl_audio_time_test");

static sdl_audio arg_audio;
static int64_t arg_usec;
static int arg_bytes;
static io_stream arg_io_s;

static int64_t stub_usectime() {
	return arg_usec;
}

static Uint32 stub_SDL_GetQueuedAudioSize(SDL_AudioDeviceID id) {
	return arg_bytes;
}

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_usec = 123456;
	init_mock_function(usectime, stub_usectime);
	init_mock_function(usleep, NULL);
	init_mock_function(SDL_GetQueuedAudioSize, stub_SDL_GetQueuedAudioSize);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUITE_CASE("output time") {
	arg_usec = 500;
	arg_bytes = 1000;
	arg_audio.device_id = 128;
	arg_audio.freq = 100000;
	arg_audio.channels = 2;
	arg_audio.format = AUDIO_S16;

	sdl_audio_clock(&arg_audio, 9000, &arg_io_s);

	CUE_EXPECT_CALLED_ONCE(usectime);

	CUE_EXPECT_CALLED_ONCE(SDL_GetQueuedAudioSize);
	CUE_EXPECT_CALLED_WITH_INT(SDL_GetQueuedAudioSize, 1, 128);

	CUE_ASSERT_STDOUT_EQ("CLK base:500 offset:6500\n");
}

SUITE_CASE("audio delay until time left") {
	arg_audio.freq = 100000;
	arg_audio.channels = 2;
	arg_audio.format = AUDIO_S16;

	arg_bytes = 1000;

	CUE_ASSERT_EQ(sdl_audio_waiting(&arg_audio, 1500), 0);

	CUE_EXPECT_CALLED_ONCE(usleep);

	CUE_EXPECT_CALLED_WITH_INT(usleep, 1, 1000);
}

SUITE_CASE("skip audio") {
	arg_audio.freq = 100000;
	arg_audio.channels = 2;
	arg_audio.format = AUDIO_S16;

	arg_bytes = 1000;

	CUE_ASSERT_EQ(sdl_audio_waiting(&arg_audio, 3500), -1);

	CUE_EXPECT_NEVER_CALLED(usleep);
}

SUITE_END(sdl_audio_time_test);
