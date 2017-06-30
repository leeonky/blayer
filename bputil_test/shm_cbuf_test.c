#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include "mock_sys.h"
#include "bputil/bputil.h"

SUITE_START("shm_cbuf_new_test");

static int shmid = 100;

int stub_shmget(key_t k, size_t size, int flag) {
	return shmid;
}

static char buffer_data[100];
static char *buffer;

void *stub_shmat(int shmid, const void *addr, int flag) {
	return buffer;
}

static int bits = 2;
static size_t e_size = 100;
static int int_arg;
static int (*cbuf_process) (shm_cbuf *, void *, io_stream *);

static int test_cbuf_process(shm_cbuf *rb, void *arg, io_stream *io_s) {
	*(int*)arg = 100;
	return 0;
}

static const char *stub_strerror(int e) {
	static char buffer[256];
	sprintf(buffer, "%d", e);
	return buffer;
}

BEFORE_EACH() {
	init_subject("", "-v", "0", "test.avi");
	buffer = buffer_data;
	cbuf_process = test_cbuf_process;

	init_mock_function(shmget, stub_shmget);
	init_mock_function(shmat, stub_shmat);
	init_mock_function(shmdt, NULL);
	init_mock_function(shmctl, NULL);
	init_mock_function(strerror, stub_strerror);

	return 0;
}

AFTER_EACH() {
	return close_subject();
}


SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return shrb_new(bits, e_size, &int_arg, cbuf_process, &io_s);
}

SUITE_CASE("create private shm buffer for cbuf, elements size is smaller than page size") {
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

static int test_cbuf_process_failed(shm_cbuf *rb, void *arg, io_stream *io_s) {
	return 1000;
}

SUITE_CASE("return process return") {
	cbuf_process = test_cbuf_process_failed;

	CUE_ASSERT_SUBJECT_FAILED_WITH(1000);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_INT(shmdt, 1, buffer);

	CUE_EXPECT_CALLED_ONCE(shmctl);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 2, IPC_RMID);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 3, NULL);
}

int stub_shmget_failed(key_t k, size_t size, int flag) {
	errno = 100;
	return -1;
}

SUITE_CASE("failed to create shm") {
	init_mock_function(shmget, stub_shmget_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shmat);
	CUE_EXPECT_NEVER_CALLED(shmdt);
	CUE_EXPECT_NEVER_CALLED(shmctl);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 100\n");
}

void *stub_shmat_failed(int shm_id, void *addr, int flag) {
	errno = 10;
	return (void *)-1;
}

SUITE_CASE("failed to map shm") {
	init_mock_function(shmat, stub_shmat_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(shmdt);

	CUE_EXPECT_CALLED_ONCE(shmctl);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 2, IPC_RMID);

	CUE_ASSERT_STDERR_EQ("Error[shm_cbuf]: 10\n");
}

static int process_assert_allocate(shm_cbuf *rb, void *arg, io_stream *io_s) {
	CUE_ASSERT_PTR_EQ(shrb_allocate(rb), buffer + getpagesize());
	return 0;
}

SUITE_CASE("allocate buffer") {
	cbuf_process = process_assert_allocate;
	e_size = getpagesize() - 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();
}

static int process_assert_allocate_in_range(shm_cbuf *rb, void *arg, io_stream *io_s) {
	shrb_allocate(rb);
	shrb_allocate(rb);
	shrb_allocate(rb);
	shrb_allocate(rb);
	CUE_ASSERT_PTR_EQ(shrb_allocate(rb), buffer + getpagesize());
	return 0;
}

SUITE_CASE("allocate circle buffer") {
	cbuf_process = process_assert_allocate_in_range;
	e_size = getpagesize() - 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();
}

SUITE_END(shm_cbuf_new_test);

SUITE_START("shm_cbuf_load_test");

BEFORE_EACH() {
	init_subject("", "-v", "0", "test.avi");
	buffer = buffer_data;
	cbuf_process = test_cbuf_process;

	init_mock_function(shmget, stub_shmget);
	init_mock_function(shmat, stub_shmat);
	init_mock_function(shmdt, NULL);
	init_mock_function(shmctl, NULL);
	init_mock_function(strerror, stub_strerror);

	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	io_stream io_s = { actxt.input_stream, actxt.output_stream, actxt.error_stream };
	return shrb_load(shmid, bits, e_size, &int_arg, cbuf_process, &io_s);
}

SUITE_CASE("load with shmid") {
	e_size = getpagesize() - 1;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);

	CUE_EXPECT_CALLED_ONCE(shmdt);
	CUE_EXPECT_CALLED_WITH_INT(shmdt, 1, buffer);

	CUE_ASSERT_EQ(int_arg, 100);
}

SUITE_END(shm_cbuf_load_test);
