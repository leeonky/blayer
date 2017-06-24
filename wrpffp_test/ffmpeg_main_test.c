#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_main_test");

static AVFormatContext format_context;
static int stub_avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	*ps = &format_context;
	return 0;
}

static int stub_av_strerror(int errnum, char *errbuf, size_t errbuf_size) {
	snprintf(errbuf, errbuf_size, "%d", errnum);
	return 0;
}

static int int_arg;

BEFORE_EACH() {
	int_arg = 0;
	init_subject("");
	init_mock_function(av_register_all, NULL);
	init_mock_function(avformat_open_input, stub_avformat_open_input);
	init_mock_function(avformat_find_stream_info, NULL);
	init_mock_function(avformat_close_input, NULL);
	init_mock_function(av_strerror, stub_av_strerror);
	return 0;
}

AFTER_EACH() {
	close_subject();
	return 0;
}

static int(*ffmpeg_main_process)(ffmpeg *, void *, io_stream *);

static int test_main(ffmpeg *ffp, void *arg, io_stream *io_s) {
	CUE_ASSERT_PTR_EQ(arg, &int_arg);
	CUE_ASSERT_PTR_EQ(ffp->format_context, &format_context);
	*(int*)arg = 100;
	return -3;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return ffmpeg_main("test.avi", &int_arg, ffmpeg_main_process, &io_s);
}

SUITE_CASE("should make sure open and close stream file") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(av_register_all);

	CUE_EXPECT_CALLED_ONCE(avformat_open_input);
	CUE_EXPECT_CALLED_WITH_STRING(avformat_open_input, 2, "test.avi");

	CUE_EXPECT_CALLED_ONCE(avformat_find_stream_info);
	CUE_EXPECT_CALLED_WITH_PTR(avformat_find_stream_info, 1, &format_context);

	CUE_EXPECT_CALLED_ONCE(avformat_close_input);
}

static int stub_avformat_open_input_error(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	return -2;
}

SUITE_CASE("should output avformat_open_input error message and exit") {
	ffmpeg_main_process = test_main;
	init_mock_function(avformat_open_input, stub_avformat_open_input_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_CALLED_ONCE(avformat_open_input);

	CUE_EXPECT_NEVER_CALLED(avformat_find_stream_info);

	CUE_EXPECT_NEVER_CALLED(avformat_close_input);

	CUE_ASSERT_STDERR_EQ("Error[wrpffp]: -2\n");

	CUE_ASSERT_EQ(int_arg, 0);
}

static int stub_avformat_find_stream_info_error(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options) {
	return -2;
}

SUITE_CASE("should output avformat_find_stream_info error message and exit") {
	ffmpeg_main_process = test_main;
	init_mock_function(avformat_find_stream_info, stub_avformat_find_stream_info_error);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_CALLED_ONCE(avformat_open_input);

	CUE_EXPECT_CALLED_ONCE(avformat_find_stream_info);

	CUE_EXPECT_CALLED_ONCE(avformat_close_input);

	CUE_ASSERT_STDERR_EQ("Error[wrpffp]: -2\n");

	CUE_ASSERT_EQ(int_arg, 0);
}

SUITE_CASE("call block and return the return of block") {
	ffmpeg_main_process = test_main;

	CUE_ASSERT_SUBJECT_FAILED_WITH(-3);

	CUE_EXPECT_CALLED_ONCE(avformat_find_stream_info);

	CUE_EXPECT_CALLED_ONCE(avformat_close_input);

	CUE_ASSERT_EQ(int_arg, 100);
}

SUITE_END(ffmpeg_main_test);

