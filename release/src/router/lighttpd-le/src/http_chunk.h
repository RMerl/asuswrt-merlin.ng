#ifndef _HTTP_CHUNK_H_
#define _HTTP_CHUNK_H_
#include "first.h"

#include "server.h"
#include <sys/types.h>

int http_chunk_append_mem(server *srv, connection *con, const char * mem, size_t len); /* copies memory */
int http_chunk_append_buffer(server *srv, connection *con, buffer *mem); /* may reset "mem" */
int http_chunk_append_file(server *srv, connection *con, buffer *fn); /* copies "fn" */
int http_chunk_append_file_range(server *srv, connection *con, buffer *fn, off_t offset, off_t len); /* copies "fn" */
void http_chunk_close(server *srv, connection *con);

#endif
