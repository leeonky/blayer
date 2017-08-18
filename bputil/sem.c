#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

sem_t *sem_new_with_ppid(int value) {
	/*return sem_open(name, flag, mode, value);*/
}

sem_t *sem_load_with_ppid() {
	/*return sem_open(name, flag);*/
}

int sem_unlink_with_ppid() {
	return 0;
}
