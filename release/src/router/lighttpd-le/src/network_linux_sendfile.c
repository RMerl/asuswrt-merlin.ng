#include "first.h"

#include "network_backends.h"

#if defined(USE_LINUX_SENDFILE)

#include "network.h"
#include "log.h"

#include <sys/sendfile.h>

#include <errno.h>
#include <string.h>

int network_write_file_chunk_sendfile(server *srv, connection *con, int fd, chunkqueue *cq, off_t *p_max_bytes) {
	chunk* const c = cq->first;
	ssize_t r;
	off_t offset;
	off_t toSend;

	force_assert(NULL != c);
	force_assert(FILE_CHUNK == c->type);
	force_assert(c->offset >= 0 && c->offset <= c->file.length);

	offset = c->file.start + c->offset;
	toSend = c->file.length - c->offset;
	if (toSend > *p_max_bytes) toSend = *p_max_bytes;

	if (0 == toSend) {
		chunkqueue_remove_finished_chunks(cq);
		return 0;
	}

	if (0 != network_open_file_chunk(srv, con, cq)) return -1;

	if (-1 == (r = sendfile(fd, c->file.fd, &offset, toSend))) {
		switch (errno) {
		case EAGAIN:
		case EINTR:
			break;
		case EPIPE:
		case ECONNRESET:
			return -2;
		case EINVAL:
		case ENOSYS:
	      #if defined(ENOTSUP) \
		&& (!defined(EOPNOTSUPP) || EOPNOTSUPP != ENOTSUP)
		case ENOTSUP:
	      #endif
	      #ifdef EOPNOTSUPP
		case EOPNOTSUPP:
	      #endif
	      #ifdef ESOCKTNOSUPPORT
		case ESOCKTNOSUPPORT:
	      #endif
	      #ifdef EAFNOSUPPORT
		case EAFNOSUPPORT:
	      #endif
		      #ifdef USE_MMAP
			return network_write_file_chunk_mmap(srv, con, fd, cq, p_max_bytes);
		      #else
			return network_write_file_chunk_no_mmap(srv, con, fd, cq, p_max_bytes);
		      #endif
		default:
			log_error_write(srv, __FILE__, __LINE__, "ssd",
					"sendfile failed:", strerror(errno), fd);
			return -1;
		}
	}

	if (r >= 0) {
		chunkqueue_mark_written(cq, r);
		*p_max_bytes -= r;
	}

	return (r > 0 && r == toSend) ? 0 : -3;
}

#endif /* USE_LINUX_SENDFILE */
