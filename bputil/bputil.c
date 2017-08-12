#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <execinfo.h>
#include "bputil.h"

static void output_errno(io_stream *io_s) {
	fprintf(io_s->stderr, "Error[shm_cbuf]: %s\n", strerror(errno));
	print_stack(io_s->stderr);
}

static void init_shm_cbuf(shm_cbuf *rb, size_t bits, size_t size) {
	size_t page_size = getpagesize();
	rb->element_size = (size+page_size-1)/page_size*page_size;
	rb->bits = bits;
	rb->mask = (1<<bits)-1;
	rb->index = 0;
}

static int map_and_process(shm_cbuf *rb, void *arg, int(*process)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	if ((rb->buffer = shmat(rb->shm_id, NULL, 0)) != (void *)-1) {
		if (process) {
			res = process(rb, arg, io_s);
		}
		shmdt(rb->buffer);
	} else {
		res = -1;
		output_errno(io_s);
	}
	return res;
}

int shrb_new(size_t bits, size_t size, void *arg, int(*process)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	shm_cbuf cbuf = {};
	init_shm_cbuf(&cbuf, bits, size);
	if ((cbuf.shm_id = shmget(IPC_PRIVATE, cbuf.element_size * (cbuf.mask+1), 0666 | IPC_CREAT)) != -1) {
		res = map_and_process(&cbuf, arg, process, io_s);
		shmctl(cbuf.shm_id, IPC_RMID, NULL);
	} else {
		res = -1;
		output_errno(io_s);
	}
	return res;
}

void *shrb_get(shm_cbuf *rb, int index) {
	return rb->buffer + index*rb->element_size;
}

void *shrb_allocate(shm_cbuf *rb) {
	rb->index = (rb->index+1) & rb->mask;
	return shrb_get(rb, rb->index);
}

int shrb_load(int id, size_t bits, size_t size, void *arg, int(*process)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	shm_cbuf cbuf = {};
	init_shm_cbuf(&cbuf, bits, size);
	cbuf.shm_id = id;
	return map_and_process(&cbuf, arg, process, io_s);
}

const char *shrb_info(shm_cbuf *cbuf) {
	static __thread char buffer[1024];
	sprintf(buffer, "cbuf:%d bits:%d size:%d", cbuf->shm_id, cbuf->bits, cbuf->element_size);
	return buffer;
}

#define MAX_STACK_DEPTH	100

void print_stack(FILE *f) {
	int fd = fileno(f);
	void *buffer[MAX_STACK_DEPTH];
	int depth;

	depth = backtrace(buffer, MAX_STACK_DEPTH);
	backtrace_symbols_fd(buffer, depth, fd);
}
