#include <stdlib.h>
#include <stdarg.h>
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

void init_app_context(app_context *context, const char *input) {
	strncpy(context->input_buffer, input, sizeof(context->input_buffer));
	context->input_stream = fmemopen(context->input_buffer, sizeof(context->input_buffer), "r");
	context->output_stream = fmemopen(context->output_buffer, sizeof(context->output_buffer), "w");
	context->error_stream = fmemopen(context->error_buffer, sizeof(context->error_buffer), "w");
}

char* output_buffer(app_context *context) {
	fflush(context->output_stream);
	return context->output_buffer;
}

char* error_buffer(app_context *context) {
	fflush(context->error_stream);
	return context->error_buffer;
}

void close_app_context(app_context *context) {
	fclose(context->input_stream);
	fclose(context->output_stream);
	fclose(context->error_stream);
}

int main_argc;
char *main_argv[64];
void set_main_args(char *arg1, ...) {
	va_list list;
	main_argc = 0;
	va_start(list, arg1);
	while(strlen(arg1)>0) {
		main_argv[main_argc++] = arg1;
		arg1 = va_arg(list, char *);
	}
	va_end(list);
}

int invoke_main(app_context *ctxt, int(*sub_main)(int, char**, FILE *, FILE *, FILE *)){
	return sub_main(main_argc, main_argv, ctxt->input_stream, ctxt->output_stream, ctxt->error_stream);
}
