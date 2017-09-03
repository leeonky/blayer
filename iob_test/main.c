#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "bputil/bputil.h"
#include "iob/iob.h"

SUITE_START("libiob");

static int (*processor) (io_bus *, void *, io_stream *);
static io_bus *bus;

BEFORE_EACH() {
	bus = NULL;
	return init_subject("Hello world!\nEXIT\ndump");
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return iob_main(NULL, processor, &io_s);
}

SUITE_CASE("should pass through line and exit when got EXIT") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();
	CUE_ASSERT_STDOUT_EQ("Hello world!\n");
}

mock_void_function_5(event_handler, io_bus *, const char *, const char *, void *, io_stream *);

static int setup_event(io_bus *iob, void *arg, io_stream *io_s) {
	iob_handler handler = {
		.command = "CMD",
		.arg = "ARG",
		.action = event_handler,
	};
	iob_add_handler(iob, &handler);
	bus = iob;
	return 0;
}

SUITE_CASE("invoke handler with args") {
	processor = setup_event;
	init_subject("CMD hello");
	init_mock_function(event_handler, NULL);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(event_handler);
	CUE_EXPECT_CALLED_WITH_PTR(event_handler, 1, bus);
	CUE_EXPECT_CALLED_WITH_STRING(event_handler, 2, "CMD");
	CUE_EXPECT_CALLED_WITH_STRING(event_handler, 3, "hello");
	CUE_EXPECT_CALLED_WITH_PTR(event_handler, 4, "ARG");
}

static int setup_event_error(io_bus *iob, void *arg, io_stream *io_s) {
	return -10;
}

SUITE_CASE("setup event failed") {
	processor = setup_event_error;
	init_subject("Hello world\n");

	CUE_ASSERT_SUBJECT_FAILED_WITH(-10);

	CUE_ASSERT_STDOUT_EQ("");
	CUE_ASSERT_STDERR_EQ("Error[libiob]: failed to setup iob handler\n");
}

SUITE_CASE("too many event") {
	io_bus iob = {};
	iob_handler handler = {};
	int count = sizeof(iob.handlers)/sizeof(iob_handler);

	while(count--)
		iob_add_handler(&iob, &handler);

	CUE_ASSERT_EQ(-1, iob_add_handler(&iob, &handler));
}

mock_void_function_3(close_handler, io_bus *, void *, io_stream *);

static int setup_event_close(io_bus *iob, void *arg, io_stream *io_s) {
	iob_handler handler = {
		.command = "CMD",
		.arg = "ARG",
		.close = close_handler,
	};
	iob_add_handler(iob, &handler);
	bus = iob;
	return 0;
}

SUITE_CASE("invoke close method when bus close") {
	processor = setup_event_close;
	init_subject("");
	init_mock_function(close_handler, NULL);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(close_handler);
	CUE_EXPECT_CALLED_WITH_PTR(close_handler, 1, bus);
	CUE_EXPECT_CALLED_WITH_STRING(close_handler, 2, "ARG");
}

SUITE_END(iob_test);

int main() {
	init_test();

	ADD_SUITE(iob_test);
	ADD_SUITE(video_frame_handler_test);
	ADD_SUITE(video_frame_test);
	ADD_SUITE(audio_frame_handler_test);
	ADD_SUITE(audio_frame_test);

	return run_test();
}
