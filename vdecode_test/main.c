#include <stdio.h>
#include <stdlib.h>
#include <libavformat/avformat.h>
#include "test.h"
#include "vdecode/vdecode.h"

void (*mock_av_register_all)();

void av_register_all() {
	if (mock_av_register_all)
		mock_av_register_all();
}

int (*mock_avformat_open_input)(AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);

int avformat_open_input (AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	if (mock_avformat_open_input)
		return mock_avformat_open_input(ps, url, fmt, options);
	return 0;
}


static int av_register_all_called;

static void fake_av_register_all() {
	av_register_all_called = 1;
}


static AVFormatContext **avformat_open_input_ps;
static const char *avformat_open_input_url;
static AVInputFormat *avformat_open_input_fmt;
static AVDictionary **avformat_open_input_options;

static int fake_avformat_open_input (AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	avformat_open_input_ps = ps;
	avformat_open_input_url = url;
	avformat_open_input_fmt = fmt;
	avformat_open_input_options = options;
}

static void open_stream_with_file() {
	mock_av_register_all = fake_av_register_all;
	mock_avformat_open_input = fake_avformat_open_input;
	char *args[] = { "test.avi" };
	avformat_open_input_url = "";

	vdecode_main(1, args);

	CU_ASSERT_TRUE(av_register_all_called);
	CU_ASSERT_STRING_EQUAL(avformat_open_input_url, "test.avi");
}

int main() {
	CU_pSuite suite = NULL;
	init_test();

	suite = create_suite("vdecode test", NULL, NULL);
	add_case(suite, open_stream_with_file);

	return run_test();
}
