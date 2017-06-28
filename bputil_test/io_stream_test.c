#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "bputil/bputil.h"

SUITE_START("io_stream_test");

SUITE_CASE("init with std") {
	io_stream io_s;

	iost_init(&io_s);

	CUE_ASSERT_PTR_EQ(io_s.stdout, stdout);
	CUE_ASSERT_PTR_EQ(io_s.stdin, stdin);
	CUE_ASSERT_PTR_EQ(io_s.stderr, stderr);
}

SUITE_END(io_stream_test);
