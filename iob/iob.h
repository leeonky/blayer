#ifndef IOB_H
#define IOB_H
#include "bputil/bputil.h"

typedef struct io_bus io_bus;

typedef struct iob_handler {
	const char *command;
	void * arg;
	void (*action)(io_bus *, const char *, const char *, void *, io_stream *);
} iob_handler;

#define MAX_HANDLER 128

typedef struct io_bus {
	iob_handler handlers[MAX_HANDLER];
	int handler_count;
} io_bus;

extern int iob_add_handler(io_bus *, const iob_handler *);

extern int iob_main(void *, int (*)(io_bus *, void *, io_stream *), io_stream *);

#endif
