#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include <sys/shm.h>
#include <unistd.h>
#include "bputil/bputil.h"

SUITE_START("shm_rbuf_test");

static int shmid = 100;

int stub_shmget(key_t k, size_t size, int flag) {
	return shmid;
}

static char buffer_data[100];
static char *buffer;

void *stub_shmat(int shmid, const void *addr, int flag) {
	return buffer;
}

mock_function_3(int, shmget, key_t, size_t, int);
mock_function_3(void *, shmat, int, const void *, int);
mock_function_3(int, shmctl, int, int, struct shmid_ds *);
mock_function_1(int, shmdt, const void *);

static int bits = 2;
static size_t e_size = 100;
static int int_arg;
static int (*rbuf_process) (shm_rbuf *, void *, io_stream *);

static int test_rbuf_process(shm_rbuf *rb, void *arg, io_stream *io_s) {
	*(int*)arg = 100;
	return 0;
}

BEFORE_EACH() {
	init_subject("", "-v", "0", "test.avi");
	buffer = buffer_data;
	rbuf_process = test_rbuf_process;

	init_mock_function(shmget, stub_shmget);
	init_mock_function(shmat, stub_shmat);
	init_mock_function(shmdt, NULL);
	init_mock_function(shmctl, NULL);

	return 0;
}

AFTER_EACH() {
	return close_subject();
}


SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return shrb_new(bits, e_size, &int_arg, rbuf_process, &io_s);
}

SUITE_CASE("create private shm buffer for rbuf, elements size is smaller than page size") {
	e_size = getpagesize() - 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(shmget);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 1, IPC_PRIVATE);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 2, getpagesize()*4);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 3, 0666 | IPC_CREAT);

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_INT(shmdt, 1, buffer);

	CUE_EXPECT_CALLED_ONCE(shmctl);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 2, IPC_RMID);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 3, NULL);

	CUE_ASSERT_EQ(int_arg, 100);
}

SUITE_CASE("create shm with elements size bigger than pagesize") {
	e_size = getpagesize() + 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(shmget);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 2, getpagesize()*2*4);
}

static int test_rbuf_process_failed(shm_rbuf *rb, void *arg, io_stream *io_s) {
	return 1000;
}

SUITE_CASE("return process return") {
	rbuf_process = test_rbuf_process_failed;

	CUE_ASSERT_SUBJECT_FAILED_WITH(1000);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_INT(shmdt, 1, buffer);

	CUE_EXPECT_CALLED_ONCE(shmctl);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 2, IPC_RMID);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 3, NULL);
}

int stub_shmget_failed(key_t k, size_t size, int flag) {
	return -1;
}

SUITE_CASE("failed to create shm") {
	init_mock_function(shmget, stub_shmget_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shmat);
	CUE_EXPECT_NEVER_CALLED(shmdt);
	CUE_EXPECT_NEVER_CALLED(shmctl);
}

void *stub_shmat_failed(int shm_id, void *addr, int flag) {
	return (void *)-1;
}

SUITE_CASE("failed to map shm") {
	init_mock_function(shmat, stub_shmat_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_CALLED_ONCE(shmctl);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 2, IPC_RMID);
}

static int process_assert_allocate(shm_rbuf *rb, void *arg, io_stream *io_s) {
	CUE_ASSERT_PTR_EQ(shrb_allocate(rb), buffer + getpagesize());
	return 0;
}

SUITE_CASE("allocate buffer") {
	rbuf_process = process_assert_allocate;
	e_size = getpagesize() - 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();
}

static int process_assert_allocate_in_range(shm_rbuf *rb, void *arg, io_stream *io_s) {
	shrb_allocate(rb);
	shrb_allocate(rb);
	shrb_allocate(rb);
	shrb_allocate(rb);
	CUE_ASSERT_PTR_EQ(shrb_allocate(rb), buffer + getpagesize());
	return 0;
}

SUITE_CASE("allocate buffer") {
	rbuf_process = process_assert_allocate_in_range;
	e_size = getpagesize() - 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();
}

SUITE_END(shm_rbuf_test);
