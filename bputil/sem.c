#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>

static const char *sem_name() {
	static __thread char buffer[128];
	sprintf(buffer, "/%d", getppid());
	return buffer;
}

sem_t *sem_new_with_ppid(int value) {
	return sem_open(sem_name(), O_CREAT|O_EXCL, 0644, value);
}

sem_t *sem_load_with_ppid() {
	return sem_open(sem_name(), 0);
}

int sem_unlink_with_ppid() {
	return sem_unlink(sem_name());
}
