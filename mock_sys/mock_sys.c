#include <cunitexd.h>
#include <sys/shm.h>

mock_function_3(int, shmget, key_t, size_t, int);
mock_function_3(void *, shmat, int, const void *, int);
mock_function_3(int, shmctl, int, int, struct shmid_ds *);
mock_function_1(int, shmdt, const void *);
mock_function_1(char *, strerror, int);
