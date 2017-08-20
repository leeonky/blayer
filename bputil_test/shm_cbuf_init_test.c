#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cunitexd.h>
#include "mock_sys/mock_sys.h"
#include "bputil/bputil.h"

SUITE_START("shm_cbuf_init_test");

static int arg_arg;
static int arg_shm_id, arg_bits, arg_element_size;
static io_stream arg_io_s;

mock_function_3(int, shrb_init_action, shm_cbuf *, void *, io_stream *);

static sem_t ret_sem;
static sem_t *stub_sem_load_with_ppid() {
	return &ret_sem;
}

static char buffer[100];
static void *stub_shmat(int shmid, const void *addr, int flag) {
	return buffer;
}

mock_function_3(int, shrb_reload_action, shm_cbuf *, void *, io_stream *);
static int shrb_init_load_cbuf(shm_cbuf *cb, void *arg, io_stream *io_s) {
	return shrb_reload(cb, arg_shm_id, arg_bits, getpagesize(), arg, shrb_reload_action, io_s);
}

BEFORE_EACH() {
	arg_arg = 10;;
	arg_shm_id = 255;
	arg_bits = 2;

	init_subject("");
	init_mock_function(shrb_init_action, shrb_init_load_cbuf);

	init_mock_function(shmat, stub_shmat);
	init_mock_function(shmdt, NULL);
	init_mock_function(sem_close, NULL);

	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return shrb_init(&arg_arg, shrb_init_action, &arg_io_s);
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
	CUE_EXPECT_CALLED_WITH_PTR(shrb_init_action, 2, &arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_init_action, 3, &arg_io_s);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_NEVER_CALLED(sem_close);
}

static int shrb_init_first_load_cbuf(shm_cbuf *cb, void *arg, io_stream *io_s) {
	cb->shm_id = 10;
	cb->buffer = buffer;
	cb->semaphore = &ret_sem;
	return 0;
}

SUITE_CASE("should delete buffer and sem if has buffer") {
	init_mock_function(shrb_init_action, shrb_init_first_load_cbuf);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(sem_close);
	CUE_EXPECT_CALLED_WITH_PTR(sem_close, 1, &ret_sem);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_PTR(shmdt, 1, buffer);
}

SUITE_END(shm_cbuf_init_test);

SUITE_START("shm_cbuf_reload_test");

static shm_cbuf arg_cbuf;

BEFORE_EACH() {
	arg_arg = 10;
	arg_shm_id = 255;
	arg_bits = 2;
	arg_element_size = getpagesize();

	arg_cbuf.shm_id = -1;
	arg_cbuf.bits = 0;
	arg_cbuf.element_size = 0;
	arg_cbuf.semaphore = NULL;

	init_subject("");

	init_mock_function(shmat, NULL);
	init_mock_function(shmdt, NULL);

	init_mock_function(shrb_reload_action, NULL);

	init_mock_function(sem_load_with_ppid, stub_sem_load_with_ppid);
	init_mock_function(sem_close, NULL);

	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;
	return 0;
}

AFTER_EACH() {
	return close_subject();
}
SUBJECT(int) {
	return shrb_reload(&arg_cbuf, arg_shm_id, arg_bits, arg_element_size, &arg_arg, shrb_reload_action, &arg_io_s);
}

static int stub_shrb_reload_action_assert(shm_cbuf *cb, void *arg, io_stream *io_s) {
	return 0;
}

SUITE_CASE("first load shm") {
	init_mock_function(shrb_reload_action, stub_shrb_reload_action_assert);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(sem_load_with_ppid);

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, arg_shm_id);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);

	CUE_EXPECT_CALLED_ONCE(shrb_reload_action);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_reload_action, 1, &arg_cbuf);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_reload_action, 2, &arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_reload_action, 3, &arg_io_s);

	CUE_ASSERT_EQ(arg_cbuf.shm_id, arg_shm_id);
	CUE_ASSERT_EQ(arg_cbuf.bits, arg_bits);
	CUE_ASSERT_EQ(arg_cbuf.element_size, arg_element_size);
	CUE_ASSERT_EQ(arg_cbuf.semaphore, &ret_sem);
}

static sem_t *stub_sem_load_with_ppid_failed() {
	errno = 100;
	return SEM_FAILED;
}

SUITE_CASE("failed to load sem with ppid") {
	init_mock_function(sem_load_with_ppid, stub_sem_load_with_ppid_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 100\n");

	CUE_EXPECT_NEVER_CALLED(shmat);

	CUE_EXPECT_NEVER_CALLED(shrb_reload_action);

	CUE_ASSERT_EQ(arg_cbuf.shm_id, -1);
	CUE_ASSERT_EQ(arg_cbuf.bits, 0);
	CUE_ASSERT_EQ(arg_cbuf.element_size, 0);
	CUE_ASSERT_EQ(arg_cbuf.semaphore, NULL);
}

static void *stub_shmat_failed(int shm_id, const void *addr, int flag) {
	errno = 10;
	return (void *)-1;
}

SUITE_CASE("failed to shmat") {
	init_mock_function(shmat, stub_shmat_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 10\n");

	CUE_EXPECT_NEVER_CALLED(shrb_reload_action);

	CUE_EXPECT_CALLED_ONCE(sem_close);

	CUE_ASSERT_EQ(arg_cbuf.shm_id, -1);
	CUE_ASSERT_EQ(arg_cbuf.bits, 0);
	CUE_ASSERT_EQ(arg_cbuf.element_size, 0);
	CUE_ASSERT_EQ(arg_cbuf.semaphore, NULL);
}

SUITE_CASE("do not close if cbuf is the same") {
	arg_cbuf.shm_id = arg_shm_id;
	arg_cbuf.bits = arg_bits;
	arg_cbuf.element_size = arg_element_size;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_NEVER_CALLED(sem_load_with_ppid);

	CUE_EXPECT_CALLED_ONCE(shrb_reload_action);

	CUE_EXPECT_NEVER_CALLED(shmat);
}

static int last_shm_id, last_bits, last_size;

SUITE_CASE("close last cbuf if diff cbuf") {
	arg_cbuf.shm_id = last_shm_id = arg_shm_id + 1;
	arg_cbuf.bits = last_bits = arg_bits + 1;
	arg_cbuf.element_size = last_size = arg_element_size + 1;
	arg_cbuf.buffer = buffer;
	arg_cbuf.semaphore = &ret_sem;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_PTR(shmdt, 1, buffer);

	CUE_EXPECT_CALLED_ONCE(sem_close);
	CUE_EXPECT_CALLED_WITH_PTR(sem_close, 1, &ret_sem);

	CUE_EXPECT_CALLED_ONCE(sem_load_with_ppid);

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, arg_shm_id);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);

	CUE_EXPECT_CALLED_ONCE(shrb_reload_action);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_reload_action, 1, &arg_cbuf);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_reload_action, 2, &arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_reload_action, 3, &arg_io_s);

	CUE_ASSERT_EQ(arg_cbuf.shm_id, arg_shm_id);
	CUE_ASSERT_EQ(arg_cbuf.bits, arg_bits);
	CUE_ASSERT_EQ(arg_cbuf.element_size, arg_element_size);
	CUE_ASSERT_EQ(arg_cbuf.semaphore, &ret_sem);
}

SUITE_END(shm_cbuf_reload_test);
