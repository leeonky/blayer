#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "vdecode/vdecode.h"

static vdecode_args args;

#define subject process_args(&args, actxt.argc, actxt.argv, actxt.error_stream)

SUITE_START("vdecode_args");

BEFORE_EACH() {
	init_subject("");
}

AFTER_EACH() {
	close_subject();
}

SUITE_CASE("should got error message when invalid arguments") {
	CU_ASSERT_EQUAL(subject, -1);
	CU_ASSERT_STRING_EQUAL(std_err, "Error[vdecode]: require video file\n");
}

SUITE_CASE("should got file name when only set video file") {
	init_subject("", "test.avi");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, -1);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
}

SUITE_CASE("can set opt of video track") {
	init_subject("", "-v", "1", "test.avi");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, 1);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
}

SUITE_CASE("can set long opt") {
	init_subject("", "--video_index", "2", "test.avi");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, 2);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
}

SUITE_CASE("can add '=' between long opt and value") {
	init_subject("", "--video_index=2", "test.avi");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, 2);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
}

SUITE_END(test_vdecode_args);
