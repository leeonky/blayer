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

SUITE_END(iob_test);

//failed event failed
//to many event

int main() {
	init_test();

	ADD_SUITE(iob_test);

	return run_test();
}
