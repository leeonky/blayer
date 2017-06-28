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

typedef struct shm_rbuf {
	int shm_id;
	char *buffer;
	size_t element_size;
	int index;
	int mask;
} shm_rbuf;

void *shrb_allocate(shm_rbuf *);

int shrb_new(size_t, size_t, void *, int(*)(shm_rbuf *, void *, io_stream *), io_stream *);

#endif
