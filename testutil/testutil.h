#ifndef TESTUTIL_H
#define TESTUTIL_H

#include <CUnit/Basic.h>
#include <stdio.h>

typedef struct app_context {
	char input_buffer[1024];
	char output_buffer[1024];
	char error_buffer[1024];
	FILE *input_stream;
	FILE *output_stream;
	FILE *error_stream;
} app_context;

extern void init_app_context(app_context *, const char *);
extern char* output_buffer(app_context *);
extern char* error_buffer(app_context *);
extern void close_app_context(app_context *);

extern int main_argc;
extern char *main_argv[64];
extern void set_main_args(char *, ...);
extern int invoke_main(app_context *, int(*)(int, char**, FILE *, FILE *, FILE *));

extern void init_test();
extern int run_test();
extern CU_pSuite create_suite(const char *suit_name, int (*init)(), int (*clean)());
extern void add_case_with_name(CU_pSuite suite, const char *case_name, void (*test)());

extern void (*before_each)();
extern void (*after_each)();

#define CU_CASE(name) \
static void exec_ ## name();\
static void name() {\
	if(before_each){\
		before_each();\
	}\
	exec_ ## name();\
	if(after_each){\
		after_each();\
	}\
}\
static void exec_ ## name()
extern void before_after(void(*)(), void(*)());

#define add_case(suite, test_case) add_case_with_name(suite, #test_case, test_case)

#define extern_mock_void_function_0(func) \
	extern int func ## _times;\
	extern void (*mock_ ## func)();
#define mock_void_function_0(func) \
	int func ## _times;\
	void (*mock_ ## func)();\
	void func() {\
		++func ## _times;\
		if(mock_ ## func)\
			mock_ ## func();\
	}

#define extern_mock_void_function_1(func, t1) \
	extern int func ## _times;\
	extern void (*mock_ ## func)(t1);\
	extern t1 func ## _p1;
#define mock_void_function_1(func, t1) \
	int func ## _times;\
	t1 func ## _p1;\
	void (*mock_ ## func)(t1);\
	void func(t1 p1) {\
		++func ## _times;\
		func ## _p1 = p1;\
		if(mock_ ## func)\
			mock_ ## func(p1);\
	}

#define extern_mock_function_2(rtype, func, t1, t2) \
	extern int func ## _times;\
	extern rtype (*mock_ ## func)(t1, t2);\
	extern t1 func ## _p1;\
	extern t2 func ## _p2;
#define mock_function_2(rtype, func, t1, t2) \
	int func ## _times;\
	t1 func ## _p1;\
	t2 func ## _p2;\
	rtype (*mock_ ## func)(t1, t2);\
	rtype func(t1 p1, t2 p2) {\
		static rtype default_result;\
		++func ## _times;\
		func ## _p1 = p1;\
		func ## _p2 = p2;\
		if(mock_ ## func)\
			return mock_ ## func(p1, p2);\
		return default_result;\
	}

#define extern_mock_function_4(rtype, func, t1, t2, t3, t4) \
	extern int func ## _times;\
	extern rtype (*mock_ ## func)(t1, t2, t3, t4);\
	extern t1 func ## _p1;\
	extern t2 func ## _p2;\
	extern t3 func ## _p3;\
	extern t4 func ## _p4;
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

#define CU_EXPECT_CALLED_ONCE(func) CU_ASSERT_EQUAL(called_times_of(func), 1)
#define CU_EXPECT_CALLED_WITH_STRING(func, at, arg) CU_ASSERT_STRING_EQUAL(params_of(func, at), arg)
#define CU_EXPECT_CALLED_WITH(func, at, arg) CU_ASSERT_EQUAL(params_of(func, at), arg)
#endif
