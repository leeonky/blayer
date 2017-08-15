#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cunitexd.h>
#include "mock_sys/mock_sys.h"
#include "bputil/bputil.h"

SUITE_START("shm_cbuf_init_test");

static void *action_arg;
static io_stream io_s;

mock_function_3(int, shrb_init_action, shm_cbuf *, void *, io_stream *);

BEFORE_EACH() {
	static int int_arg;

	int_arg = 0;
	action_arg = &int_arg;

	init_subject("");
	init_mock_function(shrb_init_action, NULL);

	init_mock_function(shmdt, NULL);
	init_mock_function(shmctl, NULL);

	io_s.stdin = actxt.input_stream;
	io_s.stdout = actxt.output_stream;
	io_s.stderr = actxt.error_stream;
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return shrb_init(action_arg, shrb_init_action, &io_s);
}

static int shrb_init_assert_cbuf_data(shm_cbuf *cb, void *arg, io_stream *io_s) {
	CUE_ASSERT_EQ(cb->shm_id, -1);
	CUE_ASSERT_EQ(cb->bits, 0);
	CUE_ASSERT_EQ(cb->element_size, 0);
	CUE_ASSERT_PTR_EQ(cb->semaphore, NULL);

	return 100;
}

SUITE_CASE("init cbuf and no any actions") {
	init_mock_function(shrb_init_action, shrb_init_assert_cbuf_data);

	CUE_ASSERT_SUBJECT_FAILED_WITH(100);

	CUE_EXPECT_CALLED_ONCE(shrb_init_action);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_init_action, 2, action_arg);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_init_action, 3, &io_s);

	CUE_EXPECT_NEVER_CALLED(shmdt);
	CUE_EXPECT_NEVER_CALLED(shmctl);
}

SUITE_END(shm_cbuf_init_test);

