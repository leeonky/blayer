#ifndef BPUTIL_H
#define BPUTIL_H

#include <stdio.h>

typedef struct io_stream {
	FILE *stdin, *stdout, *stderr;
} io_stream;

#define iost_init(io_s) do { \
	io_stream *p = io_s; \
	p->stdout = stdout; \
	p->stderr = stderr; \
	p->stdin = stdin; \
} while(0);

typedef struct shm_cbuf {
	int shm_id;
	char *buffer;
	size_t element_size;
	int index;
	int mask;
} shm_cbuf;

void *shrb_get(shm_cbuf *, int index);

void *shrb_allocate(shm_cbuf *);

int shrb_new(size_t, size_t, void *, int(*)(shm_cbuf *, void *, io_stream *), io_stream *);

int shrb_load(int, size_t, size_t, void *, int(*)(shm_cbuf *, void *, io_stream *), io_stream *);

const char *shrb_info(shm_cbuf *);

void print_stack(FILE *);

#endif
