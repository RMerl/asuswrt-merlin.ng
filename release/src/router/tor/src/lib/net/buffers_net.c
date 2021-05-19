/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file buffers_net.c
 * \brief Read and write data on a buf_t object.
 **/

#define BUFFERS_PRIVATE
#include "lib/net/buffers_net.h"
#include "lib/buf/buffers.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/net/nettypes.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef PARANOIA
/** Helper: If PARANOIA is defined, assert that the buffer in local variable
 * <b>buf</b> is well-formed. */
#define check() STMT_BEGIN buf_assert_ok(buf); STMT_END
#else
#define check() STMT_NIL
#endif /* defined(PARANOIA) */

/** Read up to <b>at_most</b> bytes from the file descriptor <b>fd</b> into
 * <b>chunk</b> (which must be on <b>buf</b>). If we get an EOF, set
 * *<b>reached_eof</b> to 1. Uses <b>tor_socket_recv()</b> iff <b>is_socket</b>
 * is true, otherwise it uses <b>read()</b>.  Return -1 on error (and sets
 * *<b>error</b> to errno), 0 on eof or blocking, and the number of bytes read
 * otherwise. */
static inline int
read_to_chunk(buf_t *buf, chunk_t *chunk, tor_socket_t fd, size_t at_most,
              int *reached_eof, int *error, bool is_socket)
{
  ssize_t read_result;
  if (at_most > CHUNK_REMAINING_CAPACITY(chunk))
    at_most = CHUNK_REMAINING_CAPACITY(chunk);

  if (is_socket)
    read_result = tor_socket_recv(fd, CHUNK_WRITE_PTR(chunk), at_most, 0);
  else
    read_result = read(fd, CHUNK_WRITE_PTR(chunk), at_most);

  if (read_result < 0) {
    int e = is_socket ? tor_socket_errno(fd) : errno;

    if (!ERRNO_IS_EAGAIN(e)) { /* it's a real error */
#ifdef _WIN32
      if (e == WSAENOBUFS)
        log_warn(LD_NET, "%s() failed: WSAENOBUFS. Not enough ram?",
                 is_socket ? "recv" : "read");
#endif
      if (error)
        *error = e;
      return -1;
    }
    return 0; /* would block. */
  } else if (read_result == 0) {
    log_debug(LD_NET,"Encountered eof on fd %d", (int)fd);
    *reached_eof = 1;
    return 0;
  } else { /* actually got bytes. */
    buf->datalen += read_result;
    chunk->datalen += read_result;
    log_debug(LD_NET,"Read %ld bytes. %d on inbuf.", (long)read_result,
              (int)buf->datalen);
    tor_assert(read_result <= BUF_MAX_LEN);
    return (int)read_result;
  }
}

/** Read from file descriptor <b>fd</b>, writing onto end of <b>buf</b>.  Read
 * at most <b>at_most</b> bytes, growing the buffer as necessary.  If recv()
 * returns 0 (because of EOF), set *<b>reached_eof</b> to 1 and return 0.
 * Return -1 on error; else return the number of bytes read.
 */
/* XXXX indicate "read blocked" somehow? */
static int
buf_read_from_fd(buf_t *buf, int fd, size_t at_most,
                 int *reached_eof,
                 int *socket_error,
                 bool is_socket)
{
  /* XXXX It's stupid to overload the return values for these functions:
   * "error status" and "number of bytes read" are not mutually exclusive.
   */
  int r = 0;
  size_t total_read = 0;

  check();
  tor_assert(reached_eof);
  tor_assert(SOCKET_OK(fd));

  if (BUG(buf->datalen > BUF_MAX_LEN))
    return -1;
  if (BUG(buf->datalen > BUF_MAX_LEN - at_most))
    return -1;

  while (at_most > total_read) {
    size_t readlen = at_most - total_read;
    chunk_t *chunk;
    if (!buf->tail || CHUNK_REMAINING_CAPACITY(buf->tail) < MIN_READ_LEN) {
      chunk = buf_add_chunk_with_capacity(buf, at_most, 1);
      if (readlen > chunk->memlen)
        readlen = chunk->memlen;
    } else {
      size_t cap = CHUNK_REMAINING_CAPACITY(buf->tail);
      chunk = buf->tail;
      if (cap < readlen)
        readlen = cap;
    }

    r = read_to_chunk(buf, chunk, fd, readlen,
                      reached_eof, socket_error, is_socket);
    check();
    if (r < 0)
      return r; /* Error */
    tor_assert(total_read+r <= BUF_MAX_LEN);
    total_read += r;
    if ((size_t)r < readlen) { /* eof, block, or no more to read. */
      break;
    }
  }
  return (int)total_read;
}

/** Helper for buf_flush_to_socket(): try to write <b>sz</b> bytes from chunk
 * <b>chunk</b> of buffer <b>buf</b> onto file descriptor <b>fd</b>.  Return
 * the number of bytes written on success, 0 on blocking, -1 on failure.
 */
static inline int
flush_chunk(tor_socket_t fd, buf_t *buf, chunk_t *chunk, size_t sz,
            bool is_socket)
{
  ssize_t write_result;

  if (sz > chunk->datalen)
    sz = chunk->datalen;

  if (is_socket)
    write_result = tor_socket_send(fd, chunk->data, sz, 0);
  else
    write_result = write(fd, chunk->data, sz);

  if (write_result < 0) {
    int e = is_socket ? tor_socket_errno(fd) : errno;

    if (!ERRNO_IS_EAGAIN(e)) { /* it's a real error */
#ifdef _WIN32
      if (e == WSAENOBUFS)
        log_warn(LD_NET,"write() failed: WSAENOBUFS. Not enough ram?");
#endif
      return -1;
    }
    log_debug(LD_NET,"write() would block, returning.");
    return 0;
  } else {
    buf_drain(buf, write_result);
    tor_assert(write_result <= BUF_MAX_LEN);
    return (int)write_result;
  }
}

/** Write data from <b>buf</b> to the file descriptor <b>fd</b>.  Write at most
 * <b>sz</b> bytes, and remove the written bytes
 * from the buffer.  Return the number of bytes written on success,
 * -1 on failure.  Return 0 if write() would block.
 */
static int
buf_flush_to_fd(buf_t *buf, int fd, size_t sz,
                bool is_socket)
{
  /* XXXX It's stupid to overload the return values for these functions:
   * "error status" and "number of bytes flushed" are not mutually exclusive.
   */
  int r;
  size_t flushed = 0;
  tor_assert(SOCKET_OK(fd));
  if (BUG(sz > buf->datalen)) {
    sz = buf->datalen;
  }

  check();
  while (sz) {
    size_t flushlen0;
    tor_assert(buf->head);
    if (buf->head->datalen >= sz)
      flushlen0 = sz;
    else
      flushlen0 = buf->head->datalen;

    r = flush_chunk(fd, buf, buf->head, flushlen0, is_socket);
    check();
    if (r < 0)
      return r;
    flushed += r;
    sz -= r;
    if (r == 0 || (size_t)r < flushlen0) /* can't flush any more now. */
      break;
  }
  tor_assert(flushed <= BUF_MAX_LEN);
  return (int)flushed;
}

/** Write data from <b>buf</b> to the socket <b>s</b>.  Write at most
 * <b>sz</b> bytes, decrement *<b>buf_flushlen</b> by
 * the number of bytes actually written, and remove the written bytes
 * from the buffer.  Return the number of bytes written on success,
 * -1 on failure.  Return 0 if write() would block.
 */
int
buf_flush_to_socket(buf_t *buf, tor_socket_t s, size_t sz)
{
  return buf_flush_to_fd(buf, s, sz, true);
}

/** Read from socket <b>s</b>, writing onto end of <b>buf</b>.  Read at most
 * <b>at_most</b> bytes, growing the buffer as necessary.  If recv() returns 0
 * (because of EOF), set *<b>reached_eof</b> to 1 and return 0. Return -1 on
 * error; else return the number of bytes read.
 */
int
buf_read_from_socket(buf_t *buf, tor_socket_t s, size_t at_most,
                     int *reached_eof,
                     int *socket_error)
{
  return buf_read_from_fd(buf, s, at_most, reached_eof, socket_error, true);
}

/** Write data from <b>buf</b> to the pipe <b>fd</b>.  Write at most
 * <b>sz</b> bytes, decrement *<b>buf_flushlen</b> by
 * the number of bytes actually written, and remove the written bytes
 * from the buffer.  Return the number of bytes written on success,
 * -1 on failure.  Return 0 if write() would block.
 */
int
buf_flush_to_pipe(buf_t *buf, int fd, size_t sz)
{
  return buf_flush_to_fd(buf, fd, sz, false);
}

/** Read from pipe <b>fd</b>, writing onto end of <b>buf</b>.  Read at most
 * <b>at_most</b> bytes, growing the buffer as necessary.  If read() returns 0
 * (because of EOF), set *<b>reached_eof</b> to 1 and return 0. Return -1 on
 * error; else return the number of bytes read.
 */
int
buf_read_from_pipe(buf_t *buf, int fd, size_t at_most,
                   int *reached_eof,
                   int *socket_error)
{
  return buf_read_from_fd(buf, fd, at_most, reached_eof, socket_error, false);
}
