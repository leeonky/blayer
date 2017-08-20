#ifndef MOCK_SYS_H
#define MOCK_SYS_H

#include <cunitexd.h>
#include <sys/shm.h>
#include <semaphore.h>

extern_mock_function_3(int, shmget, key_t, size_t, int);
extern_mock_function_3(void *, shmat, int, const void *, int);
extern_mock_function_3(int, shmctl, int, int, struct shmid_ds *);
extern_mock_function_1(int, shmdt, const void *);
extern_mock_function_1(char *, strerror, int);

//extern_mock_function_3(int, sem_init, sem_t *, int, unsigned int);
//extern_mock_function_1(int, sem_destroy, sem_t *);
extern_mock_function_1(int, sem_wait, sem_t *);
extern_mock_function_1(int, sem_post, sem_t *);

extern_mock_function_2(sem_t *, sem_new_with_ppid, int, int);
extern_mock_function_1(sem_t *, sem_load_with_ppid, int);
extern_mock_function_1(int, sem_close, sem_t *);
extern_mock_function_1(int, sem_unlink_with_ppid, int);

#endif
