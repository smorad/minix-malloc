#include <stdio.h>
#include <stdlib.h>

#ifndef MEMALLOC_H
#define MEMALLOC_H
int meminit(long n_bytes, unsigned int flags, int parm1, int *parm2);

void *memalloc (int handle, long n_bytes);

void memfree (void *region);
#endif
