#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(sdl_open_window_test);
	ADD_SUITE(sdl_present_test);
	ADD_SUITE(sdl_init_audio_test);
	ADD_SUITE(sdl_reload_audio_test);
	ADD_SUITE(sdl_play_audio_test);
	ADD_SUITE(sdl_audio_time_test);

	return run_test();
}
