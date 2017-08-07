#include <cunitexd.h>
#include "bputil/bputil.h"
#include "iob/iob.h"
#include "iob/vfs.h"

SUITE_START("video_frame_test");

static int int_arg;
static void (*process_frames)(const video_frames *vfs, void *arg, io_stream *io_s);

BEFORE_EACH() {
	int_arg = 0;
	process_frames = NULL;
	init_subject("");
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

static void process_one_frame(const video_frames *vfs, void *arg, io_stream *io_s) {
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
	process_frames = process_one_frame;
	init_subject("VFS w:1920 h:1080 fmt:0 align:1 cbuf:950284 size:3112960 frames:1=>0\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_EQ(int_arg, 100);
}

static void process_many_frames(const video_frames *vfs, void *arg, io_stream *io_s) {
	int i;
	CUE_ASSERT_EQ(vfs->count, 64);
	for(i=0; i<64; ++i) {
		CUE_ASSERT_EQ(vfs->frames[i].index, i+1);
		CUE_ASSERT_EQ((int)vfs->frames[i].pts, i+2);
	}
}

SUITE_CASE("invoke handler with many frames") {
	process_frames = process_many_frames;
	init_subject("VFS w:1 h:1 fmt:0 align:1 cbuf:1 size:1 frames:1=>2,2=>3,3=>4,4=>5,5=>6,6=>7,7=>8,8=>9,9=>10,10=>11,11=>12,12=>13,13=>14,14=>15,15=>16,16=>17,17=>18,18=>19,19=>20,20=>21,21=>22,22=>23,23=>24,24=>25,25=>26,26=>27,27=>28,28=>29,29=>30,30=>31,31=>32,32=>33,33=>34,34=>35,35=>36,36=>37,37=>38,38=>39,39=>40,40=>41,41=>42,42=>43,43=>44,44=>45,45=>46,46=>47,47=>48,48=>49,49=>50,50=>51,51=>52,52=>53,53=>54,54=>55,55=>56,56=>57,57=>58,58=>59,59=>60,60=>61,61=>62,62=>63,63=>64,64=>65\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();
}

SUITE_END(video_frame_test)
