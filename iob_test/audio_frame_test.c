#include <cunitexd.h>
#include "bputil/bputil.h"
#include "iob/iob.h"
#include "iob/afs.h"

SUITE_START("audio_frame_test")

static io_stream arg_io_s;
static void *arg_arg;

mock_void_function_3(iob_audio_action, const audio_frames *, void *, io_stream *);
mock_void_function_2(iob_close_audio_handler, void *, io_stream *);

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_arg = &arg_arg;

	init_mock_function(iob_audio_action, NULL);
	init_mock_function(iob_close_audio_handler, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

static int setup_frames_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_audio_frames_handler handler = {
		.arg = arg,
		.action = iob_audio_action,
		.close = iob_close_audio_handler,
	};

	iob_add_audio_frames_handler(iob, &handler);
	return 0;
}

SUBJECT(int) {
	return iob_main(arg_arg, setup_frames_event, &arg_io_s);
}

static void assert_one_frame(const audio_frames *afs, void *arg, io_stream *io_s) {
	CUE_ASSERT_EQ(afs->sample_rate, 48000);
	CUE_ASSERT_EQ(afs->channels, 2);
	CUE_ASSERT_EQ(afs->format, 6);
	CUE_ASSERT_EQ(afs->buffer_samples, 4800);
	CUE_ASSERT_EQ(afs->layout, 3);
	CUE_ASSERT_EQ(afs->align, 1);

	CUE_ASSERT_EQ(afs->cbuf_id, 4456477);
	CUE_ASSERT_EQ(afs->cbuf_bits, 4);
	CUE_ASSERT_EQ(afs->cbuf_size, 20480);
	CUE_ASSERT_EQ(afs->count, 1);

	CUE_ASSERT_EQ(afs->frames[0].index, 2);
	CUE_ASSERT_EQ(afs->frames[0].pts, 96000);
	CUE_ASSERT_EQ(afs->frames[0].samples_size, 4608);
}

SUITE_CASE("invoke handler with right args") {
	init_mock_function(iob_audio_action, assert_one_frame);
	init_subject("AFS rt:48000 ch:2 fmt:6 buf:4800 lay:3 align:1 cbuf:4456477 bits:4 size:20480 frames:2=>96000,4608\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(iob_audio_action);
}

static void assert_many_frames(const audio_frames *afs, void *arg, io_stream *io_s) {
	int i;
	CUE_ASSERT_EQ(afs->count, 2);
	for(i=0; i<2; ++i) {
		CUE_ASSERT_EQ(afs->frames[i].index, i+1);
		CUE_ASSERT_EQ((int)afs->frames[i].pts, i+2);
		CUE_ASSERT_EQ(afs->frames[i].samples_size, i+3);
	}
}

SUITE_CASE("invoke handler with many frames") {
	init_mock_function(iob_audio_action, assert_many_frames);
	init_subject("AFS rt:48000 ch:2 fmt:6 buf:4800 lay:3 align:1 cbuf:4456477 bits:4 size:20480 frames:1=>2,3,2=>3,4\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(iob_audio_action);
}

SUITE_CASE("record log when bad audio format") {
	init_subject("AFS bad\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDERR_EQ("Error[iob]: bad AFS: [bad\n]\n");

	CUE_EXPECT_NEVER_CALLED(iob_audio_action);
}

SUITE_CASE("record log when bad frames format") {
	init_subject("AFS rt:48000 ch:2 fmt:6 buf:4800 lay:3 align:1 cbuf:4456477 bits:4 size:20480\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDERR_EQ("Error[iob]: bad AFS: [rt:48000 ch:2 fmt:6 buf:4800 lay:3 align:1 cbuf:4456477 bits:4 size:20480\n]\n");

	CUE_EXPECT_NEVER_CALLED(iob_audio_action);
}

SUITE_CASE("record log when bad frames format 2") {
	init_subject("AFS rt:48000 ch:2 fmt:6 buf:4800 lay:3 align:1 cbuf:4456477 bits:4 size:20480 frames:1=>2\n");

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDERR_EQ("Error[iob]: bad AFS: [rt:48000 ch:2 fmt:6 buf:4800 lay:3 align:1 cbuf:4456477 bits:4 size:20480 frames:1=>2\n]\n");

	CUE_EXPECT_NEVER_CALLED(iob_audio_action);
}

SUITE_END(audio_frame_handler_test)

static audio_frames aframes;

SUITE_START("audio_frame_test")

BEFORE_EACH() {
	return init_subject("");
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	output_audio_frames(&aframes, actxt.output_stream);
	return 0;
}

SUITE_CASE("output one frame") {
	aframes.sample_rate = 48000;
	aframes.channels = 8;
	aframes.layout = 18;
	aframes.format = 1;
	aframes.cbuf_id = 10;
	aframes.cbuf_bits = 4;
	aframes.cbuf_size = 1024;
	aframes.buffer_samples = 100;
	aframes.align = 1;
	aframes.count = 1;
	aframes.frames[0].index = 100;
	aframes.frames[0].pts = (int64_t)123456789012345;
	aframes.frames[0].samples_size = 100;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDOUT_EQ("AFS rt:48000 ch:8 fmt:1 buf:100 lay:18 align:1 cbuf:10 bits:4 size:1024 frames:100=>123456789012345,100");
}

SUITE_CASE("output more than one frames") {
	aframes.sample_rate = 48000;
	aframes.channels = 8;
	aframes.layout = 18;
	aframes.format = 1;
	aframes.cbuf_id = 10;
	aframes.cbuf_bits = 4;
	aframes.cbuf_size = 1024;
	aframes.buffer_samples = 100;
	aframes.align = 1;
	aframes.count = 2;
	aframes.frames[0].index = 100;
	aframes.frames[0].pts = (int64_t)123456789012345;
	aframes.frames[0].samples_size = 100;
	aframes.frames[1].index = 101;
	aframes.frames[1].pts = (int64_t)543210987654321;
	aframes.frames[1].samples_size = 50;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_ASSERT_STDOUT_EQ("AFS rt:48000 ch:8 fmt:1 buf:100 lay:18 align:1 cbuf:10 bits:4 size:1024 frames:100=>123456789012345,100,101=>543210987654321,50");
}

SUITE_END(audio_frame_test)
