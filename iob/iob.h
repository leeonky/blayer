#ifndef IOB_H
#define IOB_H
#include "bputil/bputil.h"

typedef struct io_bus {
}io_bus;

extern int iob_main(void *, int (*)(io_bus *, void *, io_stream *), io_stream *);

#endif
