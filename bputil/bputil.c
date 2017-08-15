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

static size_t align_to_pagesize(size_t size) {
	size_t page_size = getpagesize();
	return (size+page_size-1)/page_size*page_size;
}

static void init_shm_cbuf(shm_cbuf *rb, size_t bits, size_t size) {
	rb->element_size = align_to_pagesize(size);
	rb->bits = bits;
	rb->element_count = 1<<bits;
	rb->mask = rb->element_count-1;
	rb->index = 0;
}

#define SEM_NAME "/blayer-video"

static int map_and_process(int isnew, shm_cbuf *rb, void *arg, int(*process)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	if ((rb->buffer = shmat(rb->shm_id, NULL, 0)) != (void *)-1) {
		if (process) {
#ifdef __APPLE__
			if(isnew) {
				sem_unlink(SEM_NAME);
				if(SEM_FAILED != (rb->semaphore = sem_open(SEM_NAME, O_CREAT|O_EXCL, 0644, rb->element_count))) {
					res = process(rb, arg, io_s);
					sem_close(rb->semaphore);
					sem_unlink(SEM_NAME);
				} else {
					res = -1;
					output_errno(io_s);
				}
			} else {
				if(SEM_FAILED != (rb->semaphore = sem_open(SEM_NAME, 0))) {
					res = process(rb, arg, io_s);
					sem_close(rb->semaphore);
				} else {
					res = -1;
					output_errno(io_s);
				}
			}
#else
			rb->semaphore = (sem_t *)(rb->buffer + rb->element_size*rb->element_count);
			if(isnew) {
				if(-1 != sem_init(rb->semaphore, 1, rb->element_count)) {
					res = process(rb, arg, io_s);
					sem_destroy(rb->semaphore);
				} else {
					res = -1;
					output_errno(io_s);
				}
			} else {
				res = process(rb, arg, io_s);
			}
#endif
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
	if ((cbuf.shm_id = shmget(IPC_PRIVATE, cbuf.element_size*(cbuf.element_count) + align_to_pagesize(sizeof(sem_t)), 0666 | IPC_CREAT)) != -1) {
		res = map_and_process(1, &cbuf, arg, process, io_s);
		shmctl(cbuf.shm_id, IPC_RMID, NULL);
	} else {
		res = -1;
		output_errno(io_s);
	}
	return res;
}

static inline void * get_inner(shm_cbuf *rb, int index) {
	return rb->buffer + index*rb->element_size;
}

void *shrb_get(shm_cbuf *rb, int index) {
	return get_inner(rb, index);
}

void shrb_free(shm_cbuf *rb) {
	if(-1 == sem_post(rb->semaphore))
		perror("semaphore post error");
}

void *shrb_allocate(shm_cbuf *rb) {
	if(-1 == sem_wait(rb->semaphore))
		perror("semaphore wait error");
	rb->index = (rb->index+1) & rb->mask;
	return get_inner(rb, rb->index);
}

int shrb_load(int id, size_t bits, size_t size, void *arg, int(*process)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	shm_cbuf cbuf = {};
	init_shm_cbuf(&cbuf, bits, size);
	cbuf.shm_id = id;
	return map_and_process(0, &cbuf, arg, process, io_s);
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

int shrb_init(void *arg, int(*action)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	shm_cbuf cbuf = {
		.shm_id = -1,
	};
	if(action)
		res = action(&cbuf, arg, io_s);
	return res;
}
