#include <cunitexd.h>
#include "bputil/bputil.h"
#include "iob/iob.h"
#include "iob/vfs.h"

SUITE_START("video_frame_test");

static int int_arg;

BEFORE_EACH() {
	int_arg = 0;
	init_subject("");
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

static void process_frames(const video_frames *vfs, void *arg, io_stream *io_s) {
	*(int *)arg = 100;

	CUE_ASSERT_EQ(vfs->width, 1920);
	CUE_ASSERT_EQ(vfs->height, 1080);
	CUE_ASSERT_EQ(vfs->format, 0);
	CUE_ASSERT_EQ(vfs->align, 1);
	CUE_ASSERT_EQ(vfs->cbuf_id, 950284);
	CUE_ASSERT_EQ(vfs->element_size, 3112960);
	CUE_ASSERT_EQ(vfs->count, 1);
	CUE_ASSERT_EQ(vfs->frames[0].index, 1);
	CUE_ASSERT_EQ(vfs->frames[0].pts, 0);
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_video_frames_handler handler = {
		.arg = &int_arg,
		.action = process_frames,
	};

	iob_add_video_frames_handler(iob, &handler);
	return 0;
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return iob_main(NULL, setup_frames_event, &io_s);
}

SUITE_CASE("invoke handler with right args") {
	init_subject("VFS w:1920 h:1080 fmt:0 align:1 cbuf:950284 size:3112960 frames:1=>0\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_EQ(int_arg, 100);
}

SUITE_END(video_frame_test)
