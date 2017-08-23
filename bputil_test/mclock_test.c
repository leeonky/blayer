#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include "mock_sys/mock_sys.h"
#include "bputil/bputil.h"

SUITE_START("mclock_test")

static int64_t arg_usec;

static int64_t stub_usectime() {
	return arg_usec;
}

BEFORE_EACH() {
	arg_usec = 123456;
	init_mock_function(usectime, stub_usectime);
	init_mock_function(usleep, NULL);
}

SUITE_CASE("init clock with current clock") {
	mclock mclk;

	mclk_init(&mclk);

	CUE_EXPECT_CALLED_ONCE(usectime);

	CUE_ASSERT_EQ(mclk.base, arg_usec);
	CUE_ASSERT_EQ(mclk.base_offset, 0);
}

SUITE_CASE("wait for present in right second") {
	mclock mclk = {3000, 20};
	arg_usec = 3010;

	CUE_ASSERT_EQ(mclk_waiting(&mclk, 60, 100), 0);

	CUE_EXPECT_CALLED_ONCE(usleep);
	CUE_EXPECT_CALLED_WITH_INT(usleep, 1, 30);
}

SUITE_CASE("do not wait when time is synced") {
	mclock mclk = {3000, 20};
	arg_usec = 3040;

	CUE_ASSERT_EQ(mclk_waiting(&mclk, 60, 100), 0);

	CUE_EXPECT_NEVER_CALLED(usleep);
}

SUITE_CASE("failed if time passed") {
	mclock mclk = {3000, 20};
	arg_usec = 3041;

	CUE_ASSERT_EQ(mclk_waiting(&mclk, 60, 10), -1);

	CUE_EXPECT_NEVER_CALLED(usleep);
}

SUITE_CASE("failed if too long time to wait") {
	mclock mclk = {3000, 20};
	arg_usec = 3000;

	CUE_ASSERT_EQ(mclk_waiting(&mclk, 60, 39), -1);

	CUE_EXPECT_NEVER_CALLED(usleep);
}

SUITE_END(mclock_test)
