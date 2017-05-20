#include <stdlib.h>
#include "testutil.h"

void init_test() {
	if (CUE_SUCCESS != CU_initialize_registry())
		exit(CU_get_error());
}

int run_test() {
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}

CU_pSuite create_suite(const char *suit_name, int (*init)(), int (*clean)()) {
	CU_pSuite suite = CU_add_suite(suit_name, init, clean);
	if (NULL == suite) {
		CU_cleanup_registry();
		exit(CU_get_error());
	}
	return suite;
}

void add_case_with_name(CU_pSuite suite, const char *case_name, void (*test)()) {
	if (NULL == CU_add_test(suite, case_name, test)) {
		CU_cleanup_registry();
		exit(CU_get_error());
	}
}

