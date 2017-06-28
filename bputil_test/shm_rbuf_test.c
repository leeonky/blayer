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

BEFORE_EACH() {
	buffer = buffer_data;

	init_mock_function(shmget, stub_shmget);
	init_mock_function(shmat, stub_shmat);

	return 0;
}

static shm_rbuf buf = {};

SUITE_CASE("create private shm buffer for rbuf, elements size is smaller than page size") {
	CUE_ASSERT_EQ(shrb_create(&buf, 4, getpagesize()-1), 0);

	CUE_EXPECT_CALLED_ONCE(shmget);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 1, IPC_PRIVATE);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 2, getpagesize()*16);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 3, 0666 | IPC_CREAT);

	CUE_EXPECT_CALLED_ONCE(shmat);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 2, 0);
	CUE_EXPECT_CALLED_WITH_INT(shmat, 3, 0);
}

SUITE_CASE("create shm with elements size bigger than pagesize") {
	CUE_ASSERT_EQ(shrb_create(&buf, 4, getpagesize()+1), 0);
	CUE_EXPECT_CALLED_ONCE(shmget);
	CUE_EXPECT_CALLED_WITH_INT(shmget, 2, getpagesize()*2*16);
}

int stub_shmget_failed(key_t k, size_t size, int flag) {
	return -1;
}

SUITE_CASE("failed to create shm") {
	init_mock_function(shmget, stub_shmget_failed);

	CUE_ASSERT_EQ(shrb_create(&buf, 4, 100), -1);

	CUE_EXPECT_NEVER_CALLED(shmat);
}

void *stub_shmat_failed(int shm_id, void *addr, int flag) {
	return (void *)-1;
}

SUITE_CASE("failed to map shm") {
	init_mock_function(shmat, stub_shmat_failed);

	CUE_ASSERT_EQ(shrb_create(&buf, 4, 100), -1);

	CUE_EXPECT_CALLED_ONCE(shmctl);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 1, shmid);
	CUE_EXPECT_CALLED_WITH_INT(shmctl, 2, IPC_RMID);
}

SUITE_CASE("allocate buffer") {
	init_mock_function(shmget, stub_shmget);
	init_mock_function(shmat, stub_shmat);

	shrb_create(&buf, 2, getpagesize()-1);

	CUE_ASSERT_PTR_EQ(shrb_allocate(&buf), buffer + getpagesize());
}

SUITE_CASE("allocate buffer") {
	shrb_create(&buf, 2, getpagesize()-1);
	shrb_allocate(&buf);
	shrb_allocate(&buf);
	shrb_allocate(&buf);
	shrb_allocate(&buf);

	CUE_ASSERT_PTR_EQ(shrb_allocate(&buf), buffer + getpagesize());
}

SUITE_END(shm_rbuf_test);
