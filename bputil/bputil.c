#include <sys/shm.h>
#include <unistd.h>
#include "bputil.h"

int shrb_create(shm_rbuf *rb, size_t bits, size_t size) {
	int res = 0;
	size_t page_size = getpagesize();
	rb->element_size = (size+page_size-1)/page_size*page_size;
	rb->mask = (1<<bits)-1;
	rb->index = 0;
	if((rb->shm_id = shmget(IPC_PRIVATE, rb->element_size * (rb->mask+1), 0666 | IPC_CREAT)) > 0) {
		rb->buffer = shmat(rb->shm_id, NULL, 0);
		if (rb->buffer == (void *)-1) {
			shmctl(rb->shm_id, IPC_RMID, NULL);
			res = -1;
		}
	} else {
		res = -1;
	}
	return res;
}

void *shrb_allocate(shm_rbuf *rb) {
	rb->index = (rb->index+1) & rb->mask;
	return rb->buffer + rb->index*rb->element_size;
}
