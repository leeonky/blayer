#ifndef SYS_H
#define SYS_H

#include <stdio.h>

int fmemprocess(void *, size_t, const char *, void *, int(*)(FILE *, void *));
int fileprocess(const char *, const char *, void *, int(*)(FILE *, void *));
int mtfileprocess(const char *, const char *, const char *, const char *, void *, int(*)(FILE *, FILE *, void *));

#endif
