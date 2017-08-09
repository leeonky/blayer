#ifndef MOCK_H
#define MOCK_H
#include <cunitexd.h>

extern_mock_function_3(FILE *, fmemopen, void *, size_t, const char *);
extern_mock_function_1(int, fclose, FILE *);

#endif
