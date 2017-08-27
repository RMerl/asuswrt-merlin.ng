#include "first.h"

/**
 * the HTTP chunk-API
 *
 *
 */

#include "server.h"
#include "chunk.h"
#include "http_chunk.h"
#include "stat_cache.h"
#include "log.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

static void http_chunk_append_len(server *srv, connection *con, uintmax_t len) {
	buffer *b;

	force_assert(NULL != srv);

	b = srv->tmp_chunk_len;

	buffer_string_set_length(b, 0);
	buffer_append_uint_hex(b, len);
	buffer_append_string_len(b, CONST_STR_LEN("\r\n"));

	chunkqueue_append_buffer(con->write_queue, b);
}

static int http_chunk_append_file_open_fstat(server *srv, connection *con, buffer *fn, struct stat *st) {
	if (!con->conf.follow_symlink) {
		/*(preserve existing stat_cache symlink checks)*/
		stat_cache_entry *sce;
		if (HANDLER_ERROR == stat_cache_get_entry(srv, con, fn, &sce)) return -1;
	}

	return stat_cache_open_rdonly_fstat(srv, con, fn, st);
}

static void http_chunk_append_file_fd_range(server *srv, connection *con, buffer *fn, int fd, off_t offset, off_t len) {
	chunkqueue *cq = con->write_queue;

	if (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) {
		http_chunk_append_len(srv, con, (uintmax_t)len);
	}

	chunkqueue_append_file_fd(cq, fn, fd, offset, len);

	if (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) {
		chunkqueue_append_mem(cq, CONST_STR_LEN("\r\n"));
	}
}

int http_chunk_append_file_range(server *srv, connection *con, buffer *fn, off_t offset, off_t len) {
	struct stat st;
	const int fd = http_chunk_append_file_open_fstat(srv, con, fn, &st);
	if (fd < 0) return -1;

	if (-1 == len) {
		if (offset >= st.st_size) {
			close(fd);
			return (offset == st.st_size) ? 0 : -1;
		}
		len = st.st_size - offset;
	} else if (st.st_size - offset < len) {
		close(fd);
		return -1;
	}

	http_chunk_append_file_fd_range(srv, con, fn, fd, offset, len);
	return 0;
}

int http_chunk_append_file(server *srv, connection *con, buffer *fn) {
	struct stat st;
	const int fd = http_chunk_append_file_open_fstat(srv, con, fn, &st);
	if (fd < 0) return -1;

	if (0 != st.st_size) {
		http_chunk_append_file_fd_range(srv, con, fn, fd, 0, st.st_size);
	} else {
		close(fd);
	}
	return 0;
}

static int http_chunk_append_to_tempfile(server *srv, connection *con, const char * mem, size_t len) {
	chunkqueue * const cq = con->write_queue;

	if (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) {
		/*http_chunk_append_len(srv, con, len);*/
		buffer *b = srv->tmp_chunk_len;

		buffer_string_set_length(b, 0);
		buffer_append_uint_hex(b, len);
		buffer_append_string_len(b, CONST_STR_LEN("\r\n"));

		if (0 != chunkqueue_append_mem_to_tempfile(srv, cq, CONST_BUF_LEN(b))) {
			return -1;
		}
	}

	if (0 != chunkqueue_append_mem_to_tempfile(srv, cq, mem, len)) {
		return -1;
	}

	if (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) {
		if (0 != chunkqueue_append_mem_to_tempfile(srv, cq, CONST_STR_LEN("\r\n"))) {
			return -1;
		}
	}

	return 0;
}

static int http_chunk_append_data(server *srv, connection *con, buffer *b, const char * mem, size_t len) {

	chunkqueue * const cq = con->write_queue;
	chunk *c = cq->last;
	if (0 == len) return 0;

	/* current usage does not append_mem or append_buffer after appending
	 * file, so not checking if users of this interface have appended large
	 * (references to) files to chunkqueue, which would not be in memory */

	/*(allow slightly larger mem use if FDEVENT_STREAM_RESPONSE_BUFMIN
	 * to reduce creation of temp files when backend producer will be
	 * blocked until more data is sent to network to client)*/

	if ((c && c->type == FILE_CHUNK && c->file.is_temp)
	    || cq->bytes_in - cq->bytes_out + len
		> 1024 * ((con->conf.stream_response_body & FDEVENT_STREAM_RESPONSE_BUFMIN) ? 128 : 64)) {
		return http_chunk_append_to_tempfile(srv, con, b ? b->ptr : mem, len);
	}

	/* not appending to prior mem chunk just in case using openssl
	 * and need to resubmit same args as prior call to openssl (required?)*/

	if (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) {
		http_chunk_append_len(srv, con, len);
	}

	/*(chunkqueue_append_buffer() might steal buffer contents)*/
	b ? chunkqueue_append_buffer(cq, b) : chunkqueue_append_mem(cq, mem, len);

	if (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) {
		chunkqueue_append_mem(cq, CONST_STR_LEN("\r\n"));
	}

	return 0;
}

int http_chunk_append_buffer(server *srv, connection *con, buffer *mem) {
	force_assert(NULL != con);

	return http_chunk_append_data(srv, con, mem, NULL, buffer_string_length(mem));
}

int http_chunk_append_mem(server *srv, connection *con, const char * mem, size_t len) {
	force_assert(NULL != con);
	force_assert(NULL != mem || 0 == len);

	return http_chunk_append_data(srv, con, NULL, mem, len);
}

void http_chunk_close(server *srv, connection *con) {
	UNUSED(srv);
	force_assert(NULL != con);

	if (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) {
		chunkqueue_append_mem(con->write_queue, CONST_STR_LEN("0\r\n\r\n"));
	}
}
