#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <execinfo.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "bputil.h"
#include "sem.h"

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

int shrb_new(size_t bits, size_t size, void *arg, int(*action)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	shm_cbuf cbuf = {};
	init_shm_cbuf(&cbuf, bits, size);
	if ((cbuf.shm_id = shmget(IPC_PRIVATE, cbuf.element_size*(cbuf.element_count), 0666 | IPC_CREAT)) != -1) {
		if ((cbuf.buffer = shmat(cbuf.shm_id, NULL, 0)) != (void *)-1) {
			if(SEM_FAILED != (cbuf.semaphore = sem_new_with_ppid(cbuf.element_count))) {
				if (action) {
					res = action(&cbuf, arg, io_s);
				}
				sem_close(cbuf.semaphore);
				sem_unlink_with_ppid();
			} else {
				res = -1;
				output_errno(io_s);
			}
			shmdt(cbuf.buffer);
		} else {
			res = -1;
			output_errno(io_s);
		}
		shmctl(cbuf.shm_id, IPC_RMID, NULL);
	} else {
		res = -1;
		output_errno(io_s);
	}
	return res;
}

static inline void shrb_destroy(shm_cbuf *cb) {
	shmdt(cb->buffer);
	sem_close(cb->semaphore);
}

static inline int load_shm_cbuf(shm_cbuf *rb, int id, size_t bits, size_t size, io_stream *io_s) {
	int res = 0;
	init_shm_cbuf(rb, bits, size);
	if((void *)-1 != (rb->buffer = shmat(id, NULL, 0))) {
		if(SEM_FAILED!=(rb->semaphore = sem_load_with_ppid())) {
			rb->shm_id = id;
		} else {
			shmdt(rb->buffer);
			res = -1;
			output_errno(io_s);
		}
	} else {
		res = -1;
		output_errno(io_s);
	}
	return res;
}

int shrb_load(int id, size_t bits, size_t size, void *arg, int(*action)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	shm_cbuf cbuf = {};
	if(!(res = load_shm_cbuf(&cbuf, id, bits, size, io_s))) {
		if(action)
			res = action(&cbuf, arg, io_s);
		shrb_destroy(&cbuf);
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

const char *shrb_info(shm_cbuf *cbuf) {
	static __thread char buffer[1024];
	sprintf(buffer, "cbuf:%d bits:%d size:%d", cbuf->shm_id, cbuf->bits, cbuf->element_size);
	return buffer;
}

int shrb_init(void *arg, int(*action)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	shm_cbuf cbuf = {
		.shm_id = -1,
	};

	if(action)
		res = action(&cbuf, arg, io_s);

	if(-1 != cbuf.shm_id) {
		shrb_destroy(&cbuf);
	}
	return res;
}

static inline int is_same(shm_cbuf *rb ,int id, size_t bits, size_t size) {
	return rb->shm_id == id && rb->bits == bits && rb->element_size == size;
}

int shrb_reload(shm_cbuf *rb, int id, size_t bits, size_t size, void *arg, int(*action)(shm_cbuf *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	shm_cbuf cbuf = {};
	if(is_same(rb, id, bits, size)) {
		if(action)
			res = action(rb, arg, io_s);
	} else {
		if(-1 != rb->shm_id) {
			shrb_destroy(rb);
		}
		if(!(res = load_shm_cbuf(&cbuf, id, bits, size, io_s))) {
			*rb = cbuf;
			if(action)
				res = action(rb, arg, io_s);
		}
	}
	return res;
}

#define MAX_STACK_DEPTH	100

void print_stack(FILE *f) {
	int fd = fileno(f);
	void *buffer[MAX_STACK_DEPTH];
	int depth;

	depth = backtrace(buffer, MAX_STACK_DEPTH);
	backtrace_symbols_fd(buffer, depth, fd);
}
