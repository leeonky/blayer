#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "decoder/decoder.h"

static decoder_args args;

SUITE_START("decoder_args");

BEFORE_EACH() {
	return init_subject("");
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return process_args(&args, actxt.argc, actxt.argv, actxt.error_stream);
}

SUITE_CASE("should got error message when invalid arguments") {
	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);
	CUE_ASSERT_STDERR_EQ("Error[decoder]: require media file\n");
}

SUITE_CASE("should got file name when only set video file") {
	init_subject("", "test.avi", "test");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.track_index, -1);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_CASE("can set opt of video track") {
	init_subject("", "-t", "1", "test.avi", "test");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.track_index, 1);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_CASE("can set long opt of video track") {
	init_subject("", "--track_index", "2", "test.avi", "test");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.track_index, 2);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_CASE("can add '=' between long opt and value") {
	init_subject("", "--track_index=2", "test.avi", "test");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.track_index, 2);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_CASE("default of cbuf bits") {
	init_subject("", "test.avi", "test");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.buffer_bits, 4);
}

SUITE_CASE("can set cbuf bits") {
	init_subject("", "-b 3", "test.avi", "test");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.buffer_bits, 3);
}

SUITE_END(test_decoder_args);
