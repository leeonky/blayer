#include <unistd.h>
#include "mock_sys.h"

mock_function_3(int, shmget, key_t, size_t, int);
mock_function_3(void *, shmat, int, const void *, int);
mock_function_3(int, shmctl, int, int, struct shmid_ds *);
mock_function_1(int, shmdt, const void *);
mock_function_1(char *, strerror, int);

/*mock_function_3(int, sem_init, sem_t *, int, unsigned int);*/
/*mock_function_1(int, sem_destroy, sem_t *);*/
mock_function_1(int, sem_wait, sem_t *);
mock_function_1(int, sem_post, sem_t *);

mock_function_2(sem_t *, sem_new_with_ppid, int, int);
mock_function_1(sem_t *, sem_load_with_ppid, int);
mock_function_1(int, sem_close, sem_t *);
mock_function_1(int, sem_unlink_with_ppid, int);

mock_function_0(int64_t, usectime);
mock_function_1(int, usleep, useconds_t);

mock_function_3(void *, memcpy, void *, const void *, size_t);
