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

static sem_t sem;
static sem_t *stub_sem_load_with_ppid() {
	return &sem;
}

BEFORE_EACH() {
	static int int_arg;

	int_arg = 0;
	action_arg = &int_arg;

	init_subject("");
	init_mock_function(shrb_init_action, NULL);

	init_mock_function(shmget, NULL);
	init_mock_function(shmat, NULL);
	init_mock_function(shmdt, NULL);

	init_mock_function(sem_load_with_ppid, stub_sem_load_with_ppid);
	init_mock_function(sem_close, NULL);

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
}

static char buffer[100];

static int shrb_init_first_load_cbuf(shm_cbuf *cb, void *arg, io_stream *io_s) {
	cb->shm_id = 10;
	cb->buffer = buffer;
	cb->semaphore = &sem;
	return 0;
}

SUITE_CASE("should delete buffer and sem if has buffer") {
	init_mock_function(shrb_init_action, shrb_init_first_load_cbuf);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(sem_close);
	CUE_EXPECT_CALLED_WITH_PTR(sem_close, 1, &sem);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_PTR(shmdt, 1, buffer);
}

mock_function_3(int, shrb_reload_action, shm_cbuf *, void *, io_stream *);

static void *stub_shmat(int shmid, const void *addr, int flag) {
	return buffer;
}

static int shrb_init_load_cbuf(shm_cbuf *cb, void *arg, io_stream *io_s) {
	return shrb_reload(cb, 100, 2, getpagesize(), arg, shrb_reload_action, io_s);
}

static size_t align_sem_t_size() {
	return getpagesize();
}

SUITE_CASE("load cbuf in action") {
	init_mock_function(shrb_init_action, shrb_init_load_cbuf);
	init_mock_function(shmat, stub_shmat);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, 100);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);

	CUE_EXPECT_CALLED_ONCE(sem_load_with_ppid);

	CUE_EXPECT_CALLED_ONCE(sem_close);
	CUE_EXPECT_CALLED_WITH_PTR(sem_close, 1, &sem);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_PTR(shmdt, 1, buffer);
}

SUITE_END(shm_cbuf_init_test);

