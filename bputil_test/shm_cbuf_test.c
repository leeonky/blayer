#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include "mock_sys/mock_sys.h"
#include "bputil/bputil.h"

static int ret_shmid = 100;
int stub_shmget(key_t k, size_t size, int flag) {
	return ret_shmid;
}

static char ret_buffer[100];
void *stub_shmat(int shmid, const void *addr, int flag) {
	return ret_buffer;
}

static char *stub_strerror(int e) {
	static char buffer[256];
	sprintf(buffer, "%d", e);
	return buffer;
}

static sem_t ret_sem;
static sem_t *sem_new_with_ppid(int count) {
	return &ret_sem;
}

static int arg_buffer_bits;
static size_t arg_element_size;
static int arg_arg;
static io_stream arg_io_s;

SUITE_START("shm_cbuf_new_test");

mock_function_3(int, shrb_new_action, shm_cbuf *, void *, io_stream *);

BEFORE_EACH() {
	arg_buffer_bits = 2;
	arg_element_size = 100;
	arg_arg = 0;

	init_subject("");

	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	init_mock_function(shrb_new_action, NULL);

	init_mock_function(shmget, stub_shmget);
	init_mock_function(shmat, stub_shmat);
	init_mock_function(shmdt, NULL);
	init_mock_function(shmctl, NULL);
	init_mock_function(strerror, stub_strerror);

	init_mock_function(sem_new_with_ppid, sem_new_with_ppid);
	init_mock_function(sem_close, NULL);
	init_mock_function(sem_unlink_with_ppid, NULL);
	init_mock_function(sem_wait, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}


SUBJECT(int) {
	return shrb_new(arg_buffer_bits, arg_element_size, &arg_arg, shrb_new_action, &arg_io_s);
}

SUITE_CASE("create private shm buffer for cbuf, elements size is smaller than page size") {
	arg_buffer_bits = 2;
	arg_element_size = getpagesize();

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(shmget);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 1, IPC_PRIVATE);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 2, getpagesize()*4);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 3, 0666 | IPC_CREAT);

	CUE_EXPECT_CALLED_ONCE(sem_new_with_ppid);
	CUE_EXPECT_CALLED_WITH_INT(sem_new_with_ppid, 1, 4);

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, ret_shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);

	CUE_EXPECT_CALLED_ONCE(shrb_new_action);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_new_action, 2, &arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_new_action, 3, &arg_io_s);

	CUE_EXPECT_CALLED_ONCE(sem_close);
	CUE_EXPECT_CALLED_WITH_PTR(sem_close, 1, &ret_sem);

	CUE_EXPECT_CALLED_ONCE(sem_unlink_with_ppid);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_PTR(shmdt, 1, ret_buffer);

	CUE_EXPECT_CALLED_ONCE(shmctl);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 1, ret_shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 2, IPC_RMID);
	CUE_EXPECT_CALLED_WITH_PTR(shmctl, 3, NULL);
}

SUITE_CASE("create shm with elements size bigger than pagesize") {
	arg_element_size = getpagesize() + 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(shmget);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 2, getpagesize()*2*4);
}

static int shrb_new_action_failed(shm_cbuf *rb, void *arg, io_stream *io_s) {
	return 1000;
}

SUITE_CASE("return process return") {
	init_mock_function(shrb_new_action, shrb_new_action_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(1000);
}

int stub_shmget_failed(key_t k, size_t size, int flag) {
	errno = 100;
	return -1;
}

SUITE_CASE("failed to create shm") {
	init_mock_function(shmget, stub_shmget_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(sem_new_with_ppid);

	CUE_EXPECT_NEVER_CALLED(shmat);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_NEVER_CALLED(sem_close);

	CUE_EXPECT_NEVER_CALLED(sem_unlink_with_ppid);

	CUE_EXPECT_NEVER_CALLED(shmctl);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 100\n");
}

static sem_t *stub_sem_new_with_ppid_failed(unsigned int value) {
	errno = 100;
	return SEM_FAILED;
}

SUITE_CASE("failed to init semaphore") {
	init_mock_function(sem_new_with_ppid, stub_sem_new_with_ppid_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shmat);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_NEVER_CALLED(sem_close);

	CUE_EXPECT_NEVER_CALLED(sem_unlink_with_ppid);

	CUE_EXPECT_CALLED_ONCE(shmctl);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 100\n");
}

void *stub_shmat_failed(int shm_id, const void *addr, int flag) {
	errno = 10;
	return (void *)-1;
}

SUITE_CASE("failed to map shm") {
	init_mock_function(shmat, stub_shmat_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_CALLED_ONCE(sem_close);

	CUE_EXPECT_CALLED_ONCE(sem_unlink_with_ppid);

	CUE_EXPECT_CALLED_ONCE(shmctl);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 10\n");
}

static int shrb_new_action_allocate(shm_cbuf *rb, void *arg, io_stream *io_s) {
	CUE_ASSERT_PTR_EQ(shrb_allocate(rb), ret_buffer + getpagesize());
	return 0;
}

SUITE_CASE("allocate buffer") {
	init_mock_function(shrb_new_action, shrb_new_action_allocate);
	arg_element_size = getpagesize();

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(sem_wait);
	CUE_EXPECT_CALLED_WITH_PTR(sem_wait, 1, &ret_sem);
}

static int shrb_new_action_allocate_in_circle(shm_cbuf *rb, void *arg, io_stream *io_s) {
	shrb_allocate(rb);
	shrb_allocate(rb);
	shrb_allocate(rb);
	shrb_allocate(rb);
	CUE_ASSERT_PTR_EQ(shrb_allocate(rb), ret_buffer + getpagesize());
	return 0;
}

SUITE_CASE("allocate circle buffer") {
	init_mock_function(shrb_new_action, shrb_new_action_allocate_in_circle);
	arg_element_size = getpagesize();

	CUE_ASSERT_SUBJECT_SUCCEEDED();
}

SUITE_END(shm_cbuf_new_test);

SUITE_START("shm_cbuf_load_test");

static sem_t *stub_sem_load_with_ppid() {
	return &ret_sem;
}

static int arg_shmid;

mock_function_3(int, shrb_load_action, shm_cbuf *, void *, io_stream *);

BEFORE_EACH() {
	arg_shmid = 255;
	arg_buffer_bits = 2;
	arg_element_size = 100;
	arg_arg = 0;

	init_subject("");

	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	init_mock_function(shrb_load_action, NULL);

	init_mock_function(shmat, stub_shmat);
	init_mock_function(shmdt, NULL);
	init_mock_function(shmctl, NULL);
	init_mock_function(strerror, stub_strerror);

	init_mock_function(sem_load_with_ppid, stub_sem_load_with_ppid);
	init_mock_function(sem_close, NULL);
	init_mock_function(sem_unlink_with_ppid, NULL);
	init_mock_function(sem_wait, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return shrb_load(arg_shmid, arg_buffer_bits, arg_element_size, &arg_arg, shrb_load_action, &arg_io_s);
}

SUITE_CASE("load with shmid") {
	arg_element_size = getpagesize();

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(sem_load_with_ppid);

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, arg_shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);

	CUE_EXPECT_CALLED_ONCE(shrb_load_action);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_load_action, 2, &arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(shrb_load_action, 3, &arg_io_s);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_PTR(shmdt, 1, ret_buffer);

	CUE_EXPECT_CALLED_ONCE(sem_close);
	CUE_EXPECT_CALLED_WITH_PTR(sem_close, 1, &ret_sem);
}

static sem_t *stub_sem_load_with_ppid_failed() {
	errno = 100;
	return SEM_FAILED;
}

SUITE_CASE("failed to load sem with ppid") {
	init_mock_function(sem_load_with_ppid, stub_sem_load_with_ppid_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shmat);

	CUE_EXPECT_NEVER_CALLED(shrb_load_action);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_NEVER_CALLED(sem_close);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 100\n");
}

SUITE_CASE("failed to map shm") {
	init_mock_function(shmat, stub_shmat_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shrb_load_action);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_CALLED_ONCE(sem_close);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 10\n");
}

int shrb_load_action_failed(shm_cbuf *cb, void *arg, io_stream *io_s) {
	return 100;
}

SUITE_CASE("return action return") {
	init_mock_function(shrb_load_action, shrb_load_action_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(100);

	CUE_EXPECT_CALLED_ONCE(shmdt);

	CUE_EXPECT_CALLED_ONCE(sem_close);
}

SUITE_CASE("free one element") {
	shm_cbuf buf;

	shrb_free(&buf);

	CUE_EXPECT_CALLED_ONCE(sem_post);
	CUE_EXPECT_CALLED_WITH_PTR(sem_post, 1, buf.semaphore);
}

SUITE_END(shm_cbuf_load_test);
