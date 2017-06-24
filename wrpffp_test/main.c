#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"

int main() {
	init_test();

	ADD_SUITE(ffmpeg_main_test);
	ADD_SUITE(ffmpeg_stream_test);

	return run_test();
}
