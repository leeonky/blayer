#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "vdecode/vdecode.h"

static vdecode_args args;

#define subject_i process_args(&args, actxt.argc, actxt.argv, actxt.error_stream)

SUITE_START("vdecode_args");

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
	CUE_ASSERT_STDERR_EQ("Error[vdecode]: require video file\n");
}

SUITE_CASE("should got file name when only set video file") {
	init_subject("", "test.avi");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.video_index, -1);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_CASE("can set opt of video track") {
	init_subject("", "-v", "1", "test.avi");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.video_index, 1);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_CASE("can set long opt") {
	init_subject("", "--video_index", "2", "test.avi");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.video_index, 2);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_CASE("can add '=' between long opt and value") {
	init_subject("", "--video_index=2", "test.avi");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_EQ(args.video_index, 2);
	CUE_ASSERT_STRING_EQ(args.file_name, "test.avi");
}

SUITE_END(test_vdecode_args);
