#include "mock_sys.h"

mock_function_3(int, shmget, key_t, size_t, int);
mock_function_3(void *, shmat, int, const void *, int);
mock_function_3(int, shmctl, int, int, struct shmid_ds *);
mock_function_1(int, shmdt, const void *);
mock_function_1(char *, strerror, int);

mock_function_3(int, sem_init, sem_t *, int, unsigned int);
mock_function_1(int, sem_destroy, sem_t *);
mock_function_1(int, sem_wait, sem_t *);
mock_function_1(int, sem_post, sem_t *);

#ifdef __APPLE__

sem_t *sem_open(const char *name, int oflag, ...) {
	return (sem_t *)name;
}
mock_function_1(int, sem_close, sem_t *);
mock_function_1(int, sem_unlink, const char *);

#endif
