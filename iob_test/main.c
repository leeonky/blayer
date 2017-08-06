#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "bputil/bputil.h"
#include "iob/iob.h"

SUITE_START("libiob");

BEFORE_EACH() {
	return init_subject("Hello world!\nEXIT\ndump");
}

static int (*processor) (io_bus *, void *, io_stream *);

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

SUITE_CASE("invoke handler with args") {
}

SUITE_END(iob_test);

int main() {
	init_test();

	ADD_SUITE(iob_test);

	return run_test();
}
