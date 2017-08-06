#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "vbuf/vbuf.h"

SUITE_START("vbuf_arg_test");

static vbuf_args args;

BEFORE_EACH() {
	return init_subject("", "input");
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return process_args(&args, actxt.argc, actxt.argv, actxt.error_stream);
}

SUITE_CASE("default buffer count is 8") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STRING_EQ(args.input, "input");

	CUE_ASSERT_EQ(args.size, 8);
}

SUITE_CASE("should not miss the input") {
	init_subject("");

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[vbuf]: require input file\n");
}

SUITE_END(vbuf_arg_test)

SUITE_START("vbuf_test") 

static int buf_size;

BEFORE_EACH() {
	buf_size = 2;
	return init_subject("");
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return vbuf_main(buf_size, &io_s);
}

SUITE_CASE("no buffer data") {
	buf_size = 1;
	init_subject("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0\nvideo_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:2=>41711\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDOUT_EQ("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0\nvideo_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:2=>41711\n");
}

SUITE_CASE("full buffer data") {
	init_subject("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0\nvideo_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:2=>41711\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDOUT_EQ("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0,2=>41711\n");
}

SUITE_CASE("double full buffer data") {
	init_subject("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0\nvideo_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:2=>41711\nvideo_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:3=>0\nvideo_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:4=>41711\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDOUT_EQ("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0,2=>41711\nvideo_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:3=>0,4=>41711\n");
}

SUITE_CASE("to the end of file") {
	init_subject("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDOUT_EQ("video_frames:: width:1920 height:1080 format:0 align:1 cbuf:950284 size:3112960 frames:1=>0\n");
}

// different video format
// to file end
// got EXIT
// bad format
// buf and buf

SUITE_END(vbuf_test)
