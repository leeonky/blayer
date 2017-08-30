#ifndef BPUTIL_H
#define BPUTIL_H

#include <stdio.h>
#include <semaphore.h>

typedef struct io_stream {
	FILE *stdin, *stdout, *stderr;
} io_stream;

#define iost_init(io_s) do { \
	io_stream *p = io_s; \
	p->stdout = stdout; \
	p->stderr = stderr; \
	p->stdin = stdin; \
} while(0);

typedef struct {
	int sem_id;
} shm_share_args;

typedef struct shm_cbuf {
	int bits;
	int element_size;
	int shm_id;
	sem_t *semaphore;
	char *buffer;
	size_t buffer_len;
	int index;
	int mask;
	size_t element_count;
	shm_share_args *share_args;
} shm_cbuf;

void *shrb_get(shm_cbuf *, int index);

#define shrb_index(cbuf) ((cbuf)->index)

void *shrb_allocate(shm_cbuf *);

void shrb_free(shm_cbuf *);

int shrb_new(size_t, size_t, void *, int(*)(shm_cbuf *, void *, io_stream *), io_stream *);

int shrb_load(int, size_t, size_t, void *, int(*)(shm_cbuf *, void *, io_stream *), io_stream *);

int shrb_init(void *, int(*)(shm_cbuf *, void *, io_stream *), io_stream *);

int shrb_reload(shm_cbuf *, int, size_t, size_t, void *, int(*)(shm_cbuf *, void *, io_stream *), io_stream *);

const char *shrb_info(shm_cbuf *);

void print_stack(FILE *);

typedef struct mclock {
	int64_t base, offset;
} mclock;

void mclk_init(mclock *);

int mclk_waiting(const mclock *, int64_t, int64_t);

void mclk_adjust(mclock *, int64_t);

void mclk_sync(mclock *, int64_t, int64_t);

#endif
