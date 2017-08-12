#ifndef VBUF_H
#define VBUF_H
#include <stdio.h>
#include "bputil/bputil.h"

typedef struct vbuf_args {
	int size;
} vbuf_args;

int process_args(vbuf_args *, int, char **, FILE *);

int vbuf_main(int, io_stream *);

#endif
