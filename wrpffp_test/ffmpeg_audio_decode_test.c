#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_audio_decode_test");

BEFORE_EACH() {
	init_subject("");
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUITE_END(ffmpeg_audio_decode_test);
