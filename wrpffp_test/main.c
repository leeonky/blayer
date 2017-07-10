#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"

int main() {
	init_test();

	ADD_SUITE(ffmpeg_main_test);
	ADD_SUITE(ffmpeg_stream_test);
	ADD_SUITE(ffmpeg_stream_read_test);
	ADD_SUITE(ffmpeg_decoder_test);
	ADD_SUITE(ffmpeg_decoder_frame_size_test);

	return run_test();
}
