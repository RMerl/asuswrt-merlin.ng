#include "first.h"

#include "network_backends.h"

#include "network.h"
#include "fdevent.h"
#include "log.h"
#include "stat_cache.h"

#include "sys-socket.h"

#include <sys/time.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

int network_open_file_chunk(server *srv, connection *con, chunkqueue *cq) {
	chunk* const c = cq->first;
	off_t offset, toSend;
	struct stat st;
	UNUSED(con);

	force_assert(NULL != c);
	force_assert(FILE_CHUNK == c->type);
	force_assert(c->offset >= 0 && c->offset <= c->file.length);

	offset = c->file.start + c->offset;
	toSend = c->file.length - c->offset;

	if (-1 == c->file.fd) {
		if (-1 == (c->file.fd = fdevent_open_cloexec(c->file.name->ptr, O_RDONLY, 0))) {
			log_error_write(srv, __FILE__, __LINE__, "ssb", "open failed:", strerror(errno), c->file.name);
			return -1;
		}
	}

	/*(skip file size checks if file is temp file created by lighttpd)*/
	if (c->file.is_temp) return 0;

	if (-1 == fstat(c->file.fd, &st)) {
		log_error_write(srv, __FILE__, __LINE__, "ss", "fstat failed:", strerror(errno));
		return -1;
	}

	if (offset > st.st_size || toSend > st.st_size || offset > st.st_size - toSend) {
		log_error_write(srv, __FILE__, __LINE__, "sb", "file shrunk:", c->file.name);
		return -1;
	}

	return 0;
}

int network_write_file_chunk_no_mmap(server *srv, connection *con, int fd, chunkqueue *cq, off_t *p_max_bytes) {
	chunk* const c = cq->first;
	off_t offset, toSend;
	ssize_t r;

	force_assert(NULL != c);
	force_assert(FILE_CHUNK == c->type);
	force_assert(c->offset >= 0 && c->offset <= c->file.length);

	offset = c->file.start + c->offset;
	toSend = c->file.length - c->offset;
	if (toSend > 64*1024) toSend = 64*1024; /* max read 64kb in one step */
	if (toSend > *p_max_bytes) toSend = *p_max_bytes;

	if (0 == toSend) {
		chunkqueue_remove_finished_chunks(cq);
		return 0;
	}

	if (0 != network_open_file_chunk(srv, con, cq)) return -1;

	buffer_string_prepare_copy(srv->tmp_buf, toSend);

	if (-1 == lseek(c->file.fd, offset, SEEK_SET)) {
		log_error_write(srv, __FILE__, __LINE__, "ss", "lseek: ", strerror(errno));
		return -1;
	}
	if (-1 == (toSend = read(c->file.fd, srv->tmp_buf->ptr, toSend))) {
		log_error_write(srv, __FILE__, __LINE__, "ss", "read: ", strerror(errno));
		return -1;
	}

#if defined(__WIN32)
	if ((r = send(fd, srv->tmp_buf->ptr, toSend, 0)) < 0) {
		int lastError = WSAGetLastError();
		switch (lastError) {
		case WSAEINTR:
		case WSAEWOULDBLOCK:
			break;
		case WSAECONNRESET:
		case WSAETIMEDOUT:
		case WSAECONNABORTED:
			return -2;
		default:
			log_error_write(srv, __FILE__, __LINE__, "sdd",
				"send failed: ", lastError, fd);
			return -1;
		}
	}
#else /* __WIN32 */
	if ((r = write(fd, srv->tmp_buf->ptr, toSend)) < 0) {
		switch (errno) {
		case EAGAIN:
		case EINTR:
			break;
		case EPIPE:
		case ECONNRESET:
			return -2;
		default:
			log_error_write(srv, __FILE__, __LINE__, "ssd",
				"write failed:", strerror(errno), fd);
			return -1;
		}
	}
#endif /* __WIN32 */

	if (r >= 0) {
		*p_max_bytes -= r;
		chunkqueue_mark_written(cq, r);
	}

	return (r > 0 && r == toSend) ? 0 : -3;
}
