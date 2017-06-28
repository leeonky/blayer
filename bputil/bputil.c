#include <sys/shm.h>
#include <unistd.h>
#include "bputil.h"

int shrb_new(size_t bits, size_t size, void *arg, int(*process)(shm_rbuf *, void *, io_stream *), io_stream *io_s) {
	shm_rbuf rbuf = {};
	int res = 0;
	size_t page_size = getpagesize();
	rbuf.element_size = (size+page_size-1)/page_size*page_size;
	rbuf.mask = (1<<bits)-1;
	rbuf.index = 0;
	if ((rbuf.shm_id = shmget(IPC_PRIVATE, rbuf.element_size * (rbuf.mask+1), 0666 | IPC_CREAT)) > 0) {
		if ((rbuf.buffer = shmat(rbuf.shm_id, NULL, 0)) != (void *)-1) {
			if (process) {
				res = process(&rbuf, arg, io_s);
			}
			shmdt(rbuf.buffer);
		} else {
			res = -1;
		}
		shmctl(rbuf.shm_id, IPC_RMID, NULL);
	} else {
		res = -1;
	}
	return res;
}

void *shrb_allocate(shm_rbuf *rb) {
	rb->index = (rb->index+1) & rb->mask;
	return rb->buffer + rb->index*rb->element_size;
}
