#ifndef TEST_H
#define TEST_H

#include <CUnit/Basic.h>

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

#define add_case(suite, test_case) add_case_with_name(suite, #test_case, test_case)

#define extern_mock_void_function_0(func) extern int func ## _times;\
	extern void (*mock_ ## func)();
#define mock_void_function_0(func) \
	int func ## _times;\
	void (*mock_ ## func)();\
	void func() {\
		++func ## _times;\
		if(mock_ ## func)\
			mock_ ## func();\
	}

#define extern_mock_function_4(rtype, func, t1, t2, t3, t4) extern int func ## _times;\
	extern void (*mock_ ## func)(t1, t2, t3, t4);
#define mock_function_4(rtype, func, t1, t2, t3, t4) \
	int func ## _times;\
	t1 func ## _p1;\
	t2 func ## _p2;\
	t3 func ## _p3;\
	t4 func ## _p4;\
	rtype (*mock_ ## func)(t1, t2, t3, t4);\
	rtype func(t1 p1, t2 p2, t3 p3, t4 p4) {\
		static rtype default_result;\
		++func ## _times;\
		func ## _p1 = p1;\
		func ## _p2 = p2;\
		func ## _p3 = p3;\
		func ## _p4 = p4;\
		if(mock_ ## func)\
			return mock_ ## func(p1, p2, p3, p4);\
		return default_result;\
	}

#define called_times_of(func) (func ## _times)
#define params_of(func, at) (func ## _p ## at)

#define init_mock_function(func, stub) \
	func ## _times = 0;\
	mock_ ## func = stub;

#define init_mock_function_4(func, stub) \
	func ## _times = 0;\
	mock_ ## func = stub;\

#endif
