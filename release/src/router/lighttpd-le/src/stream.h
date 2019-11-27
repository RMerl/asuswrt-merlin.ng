#ifndef _STREAM_H_
#define _STREAM_H_
#include "first.h"

#include "buffer.h"

typedef struct {
	char *start;
	off_t size;
	int mapped;
} stream;

int stream_open(stream *f, const buffer *fn);
int stream_close(stream *f);

#endif
