#include "first.h"

#include "base.h"
#include "connections.h"
#include "joblist.h"
#include "log.h"

#include <errno.h>

#ifdef USE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

const char *connection_get_state(connection_state_t state) {
	switch (state) {
	case CON_STATE_CONNECT: return "connect";
	case CON_STATE_READ: return "read";
	case CON_STATE_READ_POST: return "readpost";
	case CON_STATE_WRITE: return "write";
	case CON_STATE_CLOSE: return "close";
	case CON_STATE_ERROR: return "error";
	case CON_STATE_HANDLE_REQUEST: return "handle-req";
	case CON_STATE_REQUEST_START: return "req-start";
	case CON_STATE_REQUEST_END: return "req-end";
	case CON_STATE_RESPONSE_START: return "resp-start";
	case CON_STATE_RESPONSE_END: return "resp-end";
	default: return "(unknown)";
	}
}

const char *connection_get_short_state(connection_state_t state) {
	switch (state) {
	case CON_STATE_CONNECT: return ".";
	case CON_STATE_READ: return "r";
	case CON_STATE_READ_POST: return "R";
	case CON_STATE_WRITE: return "W";
	case CON_STATE_CLOSE: return "C";
	case CON_STATE_ERROR: return "E";
	case CON_STATE_HANDLE_REQUEST: return "h";
	case CON_STATE_REQUEST_START: return "q";
	case CON_STATE_REQUEST_END: return "Q";
	case CON_STATE_RESPONSE_START: return "s";
	case CON_STATE_RESPONSE_END: return "S";
	default: return "x";
	}
}

int connection_set_state(server *srv, connection *con, connection_state_t state) {
	UNUSED(srv);

	con->state = state;

	return 0;
}

#if 0
static void dump_packet(const unsigned char *data, size_t len) {
	size_t i, j;

	if (len == 0) return;

	for (i = 0; i < len; i++) {
		if (i % 16 == 0) fprintf(stderr, "  ");

		fprintf(stderr, "%02x ", data[i]);

		if ((i + 1) % 16 == 0) {
			fprintf(stderr, "  ");
			for (j = 0; j <= i % 16; j++) {
				unsigned char c;

				if (i-15+j >= len) break;

				c = data[i-15+j];

				fprintf(stderr, "%c", c > 32 && c < 128 ? c : '.');
			}

			fprintf(stderr, "\n");
		}
	}

	if (len % 16 != 0) {
		for (j = i % 16; j < 16; j++) {
			fprintf(stderr, "   ");
		}

		fprintf(stderr, "  ");
		for (j = i & ~0xf; j < len; j++) {
			unsigned char c;

			c = data[j];
			fprintf(stderr, "%c", c > 32 && c < 128 ? c : '.');
		}
		fprintf(stderr, "\n");
	}
}
#endif

static int connection_handle_read_ssl(server *srv, connection *con) {
#ifdef USE_OPENSSL
	int r, ssl_err, len;
	char *mem = NULL;
	size_t mem_len = 0;

	if (!con->srv_socket->is_ssl) return -1;

	ERR_clear_error();
	do {
		chunkqueue_get_memory(con->read_queue, &mem, &mem_len, 0, SSL_pending(con->ssl));
#if 0
		/* overwrite everything with 0 */
		memset(mem, 0, mem_len);
#endif

		len = SSL_read(con->ssl, mem, mem_len);
		if (len > 0) {
			chunkqueue_use_memory(con->read_queue, len);
			con->bytes_read += len;
		} else {
			chunkqueue_use_memory(con->read_queue, 0);
		}

		if (con->renegotiations > 1 && con->conf.ssl_disable_client_renegotiation) {
			log_error_write(srv, __FILE__, __LINE__, "s", "SSL: renegotiation initiated by client, killing connection");
			connection_set_state(srv, con, CON_STATE_ERROR);
			return -1;
		}
	} while (len > 0 && (con->conf.ssl_read_ahead || SSL_pending(con->ssl) > 0));

	if (len < 0) {
		int oerrno = errno;
		switch ((r = SSL_get_error(con->ssl, len))) {
		case SSL_ERROR_WANT_WRITE:
			con->is_writable = -1;
		case SSL_ERROR_WANT_READ:
			con->is_readable = 0;

			/* the manual says we have to call SSL_read with the same arguments next time.
			 * we ignore this restriction; no one has complained about it in 1.5 yet, so it probably works anyway.
			 */

			return 0;
		case SSL_ERROR_SYSCALL:
			/**
			 * man SSL_get_error()
			 *
			 * SSL_ERROR_SYSCALL
			 *   Some I/O error occurred.  The OpenSSL error queue may contain more
			 *   information on the error.  If the error queue is empty (i.e.
			 *   ERR_get_error() returns 0), ret can be used to find out more about
			 *   the error: If ret == 0, an EOF was observed that violates the
			 *   protocol.  If ret == -1, the underlying BIO reported an I/O error
			 *   (for socket I/O on Unix systems, consult errno for details).
			 *
			 */
			while((ssl_err = ERR_get_error())) {
				/* get all errors from the error-queue */
				log_error_write(srv, __FILE__, __LINE__, "sds", "SSL:",
						r, ERR_error_string(ssl_err, NULL));
			}

			switch(oerrno) {
			default:
				log_error_write(srv, __FILE__, __LINE__, "sddds", "SSL:",
						len, r, oerrno,
						strerror(oerrno));
				break;
			}

			break;
		case SSL_ERROR_ZERO_RETURN:
			/* clean shutdown on the remote side */

			if (r == 0) {
				/* FIXME: later */
			}

			/* fall thourgh */
		default:
			while((ssl_err = ERR_get_error())) {
				switch (ERR_GET_REASON(ssl_err)) {
				case SSL_R_SSL_HANDSHAKE_FAILURE:
			      #ifdef SSL_R_TLSV1_ALERT_UNKNOWN_CA
				case SSL_R_TLSV1_ALERT_UNKNOWN_CA:
			      #endif
			      #ifdef SSL_R_SSLV3_ALERT_CERTIFICATE_UNKNOWN
				case SSL_R_SSLV3_ALERT_CERTIFICATE_UNKNOWN:
			      #endif
			      #ifdef SSL_R_SSLV3_ALERT_BAD_CERTIFICATE
				case SSL_R_SSLV3_ALERT_BAD_CERTIFICATE:
			      #endif
					if (!con->conf.log_ssl_noise) continue;
					break;
				default:
					break;
				}
				/* get all errors from the error-queue */
				log_error_write(srv, __FILE__, __LINE__, "sds", "SSL:",
				                r, ERR_error_string(ssl_err, NULL));
			}
			break;
		}

		connection_set_state(srv, con, CON_STATE_ERROR);

		return -1;
	} else if (len == 0) {
		con->is_readable = 0;
		/* the other end close the connection -> KEEP-ALIVE */

		return -2;
	} else {
		return 0;
	}
#else
	UNUSED(srv);
	UNUSED(con);
	return -1;
#endif
}

/* 0: everything ok, -1: error, -2: con closed */
int connection_handle_read(server *srv, connection *con) {
	int len;
	char *mem = NULL;
	size_t mem_len = 0;
	int toread;

	if (con->srv_socket->is_ssl) {
		return connection_handle_read_ssl(srv, con);
	}

	/* default size for chunks is 4kb; only use bigger chunks if FIONREAD tells
	 *  us more than 4kb is available
	 * if FIONREAD doesn't signal a big chunk we fill the previous buffer
	 *  if it has >= 1kb free
	 */
#if defined(__WIN32)
	chunkqueue_get_memory(con->read_queue, &mem, &mem_len, 0, 4096);

	len = recv(con->fd, mem, mem_len, 0);
#else /* __WIN32 */
	if (ioctl(con->fd, FIONREAD, &toread) || toread <= 4*1024) {
		toread = 4096;
	}
	else if (toread > MAX_READ_LIMIT) {
		toread = MAX_READ_LIMIT;
	}
	chunkqueue_get_memory(con->read_queue, &mem, &mem_len, 0, toread);

	len = read(con->fd, mem, mem_len);
#endif /* __WIN32 */

	chunkqueue_use_memory(con->read_queue, len > 0 ? len : 0);

	if (len < 0) {
		con->is_readable = 0;

#if defined(__WIN32)
		{
			int lastError = WSAGetLastError();
			switch (lastError) {
			case EAGAIN:
				return 0;
			case EINTR:
				/* we have been interrupted before we could read */
				con->is_readable = 1;
				return 0;
			case ECONNRESET:
				/* suppress logging for this error, expected for keep-alive */
				break;
			default:
				log_error_write(srv, __FILE__, __LINE__, "sd", "connection closed - recv failed: ", lastError);
				break;
			}
		}
#else /* __WIN32 */
		switch (errno) {
		case EAGAIN:
			return 0;
		case EINTR:
			/* we have been interrupted before we could read */
			con->is_readable = 1;
			return 0;
		case ECONNRESET:
			/* suppress logging for this error, expected for keep-alive */
			break;
		default:
			log_error_write(srv, __FILE__, __LINE__, "ssd", "connection closed - read failed: ", strerror(errno), errno);
			break;
		}
#endif /* __WIN32 */

		connection_set_state(srv, con, CON_STATE_ERROR);

		return -1;
	} else if (len == 0) {
		con->is_readable = 0;
		/* the other end close the connection -> KEEP-ALIVE */

		/* pipelining */

		return -2;
	} else if (len != (ssize_t) mem_len) {
		/* we got less then expected, wait for the next fd-event */

		con->is_readable = 0;
	}

	con->bytes_read += len;
#if 0
	dump_packet(b->ptr, len);
#endif

	return 0;
}

static int connection_handle_read_post_cq_compact(chunkqueue *cq) {
    /* combine first mem chunk with next non-empty mem chunk
     * (loop if next chunk is empty) */
    chunk *c;
    while (NULL != (c = cq->first) && NULL != c->next) {
        buffer *mem = c->next->mem;
        off_t offset = c->next->offset;
        size_t blen = buffer_string_length(mem) - (size_t)offset;
        force_assert(c->type == MEM_CHUNK);
        force_assert(c->next->type == MEM_CHUNK);
        buffer_append_string_len(c->mem, mem->ptr+offset, blen);
        c->next->offset = c->offset;
        c->next->mem = c->mem;
        c->mem = mem;
        c->offset = offset + (off_t)blen;
        chunkqueue_remove_finished_chunks(cq);
        if (0 != blen) return 1;
    }
    return 0;
}

static int connection_handle_read_post_chunked_crlf(chunkqueue *cq) {
    /* caller might check chunkqueue_length(cq) >= 2 before calling here
     * to limit return value to either 1 for good or -1 for error */
    chunk *c;
    buffer *b;
    char *p;
    size_t len;

    /* caller must have called chunkqueue_remove_finished_chunks(cq), so if
     * chunkqueue is not empty, it contains chunk with at least one char */
    if (chunkqueue_is_empty(cq)) return 0;

    c = cq->first;
    b = c->mem;
    p = b->ptr+c->offset;
    if (p[0] != '\r') return -1; /* error */
    if (p[1] == '\n') return 1;
    len = buffer_string_length(b) - (size_t)c->offset;
    if (1 != len) return -1; /* error */

    while (NULL != (c = c->next)) {
        b = c->mem;
        len = buffer_string_length(b) - (size_t)c->offset;
        if (0 == len) continue;
        p = b->ptr+c->offset;
        return (p[0] == '\n') ? 1 : -1; /* error if not '\n' */
    }
    return 0;
}

handler_t connection_handle_read_post_error(server *srv, connection *con, int http_status) {
    UNUSED(srv);

    con->keep_alive = 0;

    /*(do not change status if response headers already set and possibly sent)*/
    if (0 != con->bytes_header) return HANDLER_ERROR;

    con->http_status = http_status;
    con->mode = DIRECT;
    chunkqueue_reset(con->write_queue);
    return HANDLER_FINISHED;
}

static handler_t connection_handle_read_post_chunked(server *srv, connection *con, chunkqueue *cq, chunkqueue *dst_cq) {

    /* con->conf.max_request_size is in kBytes */
    const off_t max_request_size = (off_t)con->conf.max_request_size << 10;
    off_t te_chunked = con->request.te_chunked;
    do {
        off_t len = cq->bytes_in - cq->bytes_out;

        while (0 == te_chunked) {
            char *p;
            chunk *c = cq->first;
            force_assert(c->type == MEM_CHUNK);
            p = strchr(c->mem->ptr+c->offset, '\n');
            if (NULL != p) { /* found HTTP chunked header line */
                off_t hsz = p + 1 - (c->mem->ptr+c->offset);
                unsigned char *s = (unsigned char *)c->mem->ptr+c->offset;
                for (unsigned char u;(u=(unsigned char)hex2int(*s))!=0xFF;++s) {
                    if (te_chunked > (~((off_t)-1) >> 4)) {
                        log_error_write(srv, __FILE__, __LINE__, "s",
                                        "chunked data size too large -> 400");
                        /* 400 Bad Request */
                        return connection_handle_read_post_error(srv, con, 400);
                    }
                    te_chunked <<= 4;
                    te_chunked |= u;
                }
                while (*s == ' ' || *s == '\t') ++s;
                if (*s != '\r' && *s != ';') {
                    log_error_write(srv, __FILE__, __LINE__, "s",
                                    "chunked header invalid chars -> 400");
                    /* 400 Bad Request */
                    return connection_handle_read_post_error(srv, con, 400);
                }

                if (hsz >= 1024) {
                    /* prevent theoretical integer overflow
                     * casting to (size_t) and adding 2 (for "\r\n") */
                    log_error_write(srv, __FILE__, __LINE__, "s",
                                    "chunked header line too long -> 400");
                    /* 400 Bad Request */
                    return connection_handle_read_post_error(srv, con, 400);
                }

                if (0 == te_chunked) {
                    /* do not consume final chunked header until
                     * (optional) trailers received along with
                     * request-ending blank line "\r\n" */
                    if (p[0] == '\r' && p[1] == '\n') {
                        /*(common case with no trailers; final \r\n received)*/
                        hsz += 2;
                    }
                    else {
                        /* trailers or final CRLF crosses into next cq chunk */
                        hsz -= 2;
                        do {
                            c = cq->first;
                            p = strstr(c->mem->ptr+c->offset+hsz, "\r\n\r\n");
                        } while (NULL == p
                                 && connection_handle_read_post_cq_compact(cq));
                        if (NULL == p) {
                            /*(effectively doubles max request field size
                             * potentially received by backend, if in the future
                             * these trailers are added to request headers)*/
                            if ((off_t)buffer_string_length(c->mem) - c->offset
                                < srv->srvconf.max_request_field_size) {
                                break;
                            }
                            else {
                                /* ignore excessively long trailers;
                                 * disable keep-alive on connection */
                                con->keep_alive = 0;
                            }
                        }
                        hsz = p + 4 - (c->mem->ptr+c->offset);
                        /* trailers currently ignored, but could be processed
                         * here if 0 == con->conf.stream_request_body, taking
                         * care to reject any fields forbidden in trailers,
                         * making trailers available to CGI and other backends*/
                    }
                    chunkqueue_mark_written(cq, (size_t)hsz);
                    con->request.content_length = dst_cq->bytes_in;
                    break; /* done reading HTTP chunked request body */
                }

                /* consume HTTP chunked header */
                chunkqueue_mark_written(cq, (size_t)hsz);
                len = cq->bytes_in - cq->bytes_out;

                if (0 !=max_request_size
                    && (max_request_size < te_chunked
                     || max_request_size - te_chunked < dst_cq->bytes_in)) {
                    log_error_write(srv, __FILE__, __LINE__, "sos",
                                    "request-size too long:",
                                    dst_cq->bytes_in + te_chunked, "-> 413");
                    /* 413 Payload Too Large */
                    return connection_handle_read_post_error(srv, con, 413);
                }

                te_chunked += 2; /*(for trailing "\r\n" after chunked data)*/

                break; /* read HTTP chunked header */
            }

            /*(likely better ways to handle chunked header crossing chunkqueue
             * chunks, but this situation is not expected to occur frequently)*/
            if ((off_t)buffer_string_length(c->mem) - c->offset >= 1024) {
                log_error_write(srv, __FILE__, __LINE__, "s",
                                "chunked header line too long -> 400");
                /* 400 Bad Request */
                return connection_handle_read_post_error(srv, con, 400);
            }
            else if (!connection_handle_read_post_cq_compact(cq)) {
                break;
            }
        }
        if (0 == te_chunked) break;

        if (te_chunked > 2) {
            if (len > te_chunked-2) len = te_chunked-2;
            if (dst_cq->bytes_in + te_chunked <= 64*1024) {
                /* avoid buffering request bodies <= 64k on disk */
                chunkqueue_steal(dst_cq, cq, len);
            }
            else if (0 != chunkqueue_steal_with_tempfiles(srv,dst_cq,cq,len)) {
                /* 500 Internal Server Error */
                return connection_handle_read_post_error(srv, con, 500);
            }
            te_chunked -= len;
            len = cq->bytes_in - cq->bytes_out;
        }

        if (len < te_chunked) break;

        if (2 == te_chunked) {
            if (-1 == connection_handle_read_post_chunked_crlf(cq)) {
                log_error_write(srv, __FILE__, __LINE__, "s",
                                "chunked data missing end CRLF -> 400");
                /* 400 Bad Request */
                return connection_handle_read_post_error(srv, con, 400);
            }
            chunkqueue_mark_written(cq, 2);/*consume \r\n at end of chunk data*/
            te_chunked -= 2;
        }

    } while (!chunkqueue_is_empty(cq));

    con->request.te_chunked = te_chunked;
    return HANDLER_GO_ON;
}

handler_t connection_handle_read_post_state(server *srv, connection *con) {
	chunkqueue *cq = con->read_queue;
	chunkqueue *dst_cq = con->request_content_queue;

	int is_closed = 0;

	if (con->is_readable) {
		con->read_idle_ts = srv->cur_ts;

		switch(connection_handle_read(srv, con)) {
		case -1:
			return HANDLER_ERROR;
		case -2:
			is_closed = 1;
			break;
		default:
			break;
		}
	}

	chunkqueue_remove_finished_chunks(cq);

	if (-1 == con->request.content_length) { /*(Transfer-Encoding: chunked)*/
		handler_t rc = connection_handle_read_post_chunked(srv, con, cq, dst_cq);
		if (HANDLER_GO_ON != rc) return rc;
	}
	else if (con->request.content_length <= 64*1024) {
		/* don't buffer request bodies <= 64k on disk */
		chunkqueue_steal(dst_cq, cq, (off_t)con->request.content_length - dst_cq->bytes_in);
	}
	else if (0 != chunkqueue_steal_with_tempfiles(srv, dst_cq, cq, (off_t)con->request.content_length - dst_cq->bytes_in)) {
		/* writing to temp file failed */
		return connection_handle_read_post_error(srv, con, 500); /* Internal Server Error */
	}

	chunkqueue_remove_finished_chunks(cq);

	if (dst_cq->bytes_in == (off_t)con->request.content_length) {
		/* Content is ready */
		con->conf.stream_request_body &= ~FDEVENT_STREAM_REQUEST_POLLIN;
		if (con->state == CON_STATE_READ_POST) {
			connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);
		}
		return HANDLER_GO_ON;
	} else if (is_closed) {
	      #if 0
		return connection_handle_read_post_error(srv, con, 400); /* Bad Request */
	      #endif
		return HANDLER_ERROR;
	} else {
		con->conf.stream_request_body |= FDEVENT_STREAM_REQUEST_POLLIN;
		return (con->conf.stream_request_body & FDEVENT_STREAM_REQUEST)
		  ? HANDLER_GO_ON
		  : HANDLER_WAIT_FOR_EVENT;
	}
}

void connection_response_reset(server *srv, connection *con) {
	UNUSED(srv);

	con->mode = DIRECT;
	con->http_status = 0;
	con->is_writable = 1;
	con->file_finished = 0;
	con->file_started = 0;
	con->got_response = 0;
	con->parsed_response = 0;
	con->response.keep_alive = 0;
	con->response.content_length = -1;
	con->response.transfer_encoding = 0;
	if (con->physical.path) { /*(skip for mod_fastcgi authorizer)*/
		buffer_reset(con->physical.doc_root);
		buffer_reset(con->physical.path);
		buffer_reset(con->physical.basedir);
		buffer_reset(con->physical.rel_path);
		buffer_reset(con->physical.etag);
	}
	array_reset(con->response.headers);
	chunkqueue_reset(con->write_queue);
}
