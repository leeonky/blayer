#ifndef MOCK_SYS_H
#define MOCK_SYS_H

#include <cunitexd.h>

extern_mock_function_3(int, shmget, key_t, size_t, int);
extern_mock_function_3(void *, shmat, int, const void *, int);
extern_mock_function_3(int, shmctl, int, int, struct shmid_ds *);
extern_mock_function_1(int, shmdt, const void *);
extern_mock_function_1(char *, strerror, int);
extern_mock_function_3(FILE *, fmemopen, void *, size_t, const char *);
extern_mock_function_1(int, fclose, FILE *);

#endif
