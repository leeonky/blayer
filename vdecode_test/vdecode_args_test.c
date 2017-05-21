#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "testutil/testutil.h"
#include "vdecode/vdecode.h"

static app_context ctxt;
static vdecode_args args;

#define subject process_args(&args, main_argc, main_argv, ctxt.error_stream)

static void invalid_arguments() {
	init_app_context(&ctxt, "");
	set_main_args("");

	CU_ASSERT_EQUAL(subject, -1);
	CU_ASSERT_STRING_EQUAL(error_buffer(&ctxt), "Error[vdecode]: require video file\n");
	close_app_context(&ctxt);
}

static void only_set_video_file() {
	init_app_context(&ctxt, "");
	set_main_args("test.avi", "");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, -1);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
	close_app_context(&ctxt);
}

static void set_video_track() {
	init_app_context(&ctxt, "");
	set_main_args("-v", "1", "test.avi", "");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, 1);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
	close_app_context(&ctxt);
}

static void set_video_track_with_long_opt() {
	init_app_context(&ctxt, "");
	set_main_args("--video_index", "2", "test.avi", "");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, 2);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
	close_app_context(&ctxt);
}

static void set_video_track_with_long_opt_style_2() {
	init_app_context(&ctxt, "");
	set_main_args("--video_index=2", "test.avi", "");

	CU_ASSERT_EQUAL(subject, 0);
	CU_ASSERT_EQUAL(args.video_index, 2);
	CU_ASSERT_STRING_EQUAL(args.file_name, "test.avi");
	close_app_context(&ctxt);
}

void test_vdecode_args() {
	CU_pSuite suite = create_suite("vdecode arguments test", NULL, NULL);
	add_case(suite, invalid_arguments);
	add_case(suite, only_set_video_file);
	add_case(suite, set_video_track);
	add_case(suite, set_video_track_with_long_opt);
	add_case(suite, set_video_track_with_long_opt_style_2);
}
