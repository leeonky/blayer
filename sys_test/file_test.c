#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "sys/sys.h"
#include "mock.h"

SUITE_START("fmemopen_test");

static int int_arg;

mock_function_2(int, processer, FILE *, void *);

static FILE file;

static FILE * stub_fmemopen(void *buf, size_t size, const char *mode) {
	return &file;
}

BEFORE_EACH() {
	init_mock_function(fmemopen, stub_fmemopen);
	init_mock_function(fclose, NULL);
	init_mock_function(processer, NULL);
	int_arg = 0;
	return 0;
}

static int processer_return_10(FILE *f, void *arg) {
	return 10;
}

SUITE_CASE("open read and close") {
	char buf[100];
	init_mock_function(processer, processer_return_10);

	CUE_ASSERT_EQ(fmemprocess(buf, 100, "r", &int_arg, processer), 10);

	CUE_EXPECT_CALLED_ONCE(fmemopen);
	CUE_EXPECT_CALLED_WITH_PTR(fmemopen, 1, buf);
	CUE_EXPECT_CALLED_WITH_INT(fmemopen, 2, 100);
	CUE_EXPECT_CALLED_WITH_STRING(fmemopen, 3, "r");

	CUE_EXPECT_CALLED_ONCE(processer);
	CUE_EXPECT_CALLED_WITH_PTR(processer, 1, &file);
	CUE_EXPECT_CALLED_WITH_PTR(processer, 2, &int_arg);


	CUE_EXPECT_CALLED_ONCE(fclose);
	CUE_EXPECT_CALLED_WITH_PTR(fclose, 1, &file);
}

static FILE *stub_fmemopen_error(void *buf, size_t size, const char *mode) {
	return NULL;
}

SUITE_CASE("filed to open") {
	char buf[100];
	init_mock_function(fmemopen, stub_fmemopen_error);

	CUE_ASSERT_EQ(fmemprocess(buf, 100, "r", &int_arg, processer), -1);

	CUE_EXPECT_NEVER_CALLED(processer);

	CUE_EXPECT_NEVER_CALLED(fclose);
}

SUITE_END(fmemopen_test);

