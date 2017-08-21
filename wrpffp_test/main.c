#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>

int main() {
	init_test();

	ADD_SUITE(ffmpeg_open_test);
	ADD_SUITE(ffmpeg_stream_test);
	ADD_SUITE(ffmpeg_read_test);
	ADD_SUITE(ffmpeg_frame_size_test);
	ADD_SUITE(ffmpeg_read_and_feed_test);
	ADD_SUITE(ffmpeg_decoder_test);
	ADD_SUITE(ffmpeg_decode_test);
	ADD_SUITE(ffmpeg_frame_copy_test);
	/*ADD_SUITE(ffmpeg_frame_fill_test);*/

	return run_test();
}
