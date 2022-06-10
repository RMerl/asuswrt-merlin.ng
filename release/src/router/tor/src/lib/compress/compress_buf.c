/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compress_buf.c
 *
 * \brief Working with compressed data in buffers.
 **/

#define BUFFERS_PRIVATE
#include "lib/cc/compat_compiler.h"
#include "lib/buf/buffers.h"
#include "lib/compress/compress.h"
#include "lib/log/util_bug.h"

#ifdef PARANOIA
/** Helper: If PARANOIA is defined, assert that the buffer in local variable
 * <b>buf</b> is well-formed. */
#define check() STMT_BEGIN buf_assert_ok(buf); STMT_END
#else
#define check() STMT_NIL
#endif /* defined(PARANOIA) */

/** Compress or uncompress the <b>data_len</b> bytes in <b>data</b> using the
 * compression state <b>state</b>, appending the result to <b>buf</b>.  If
 * <b>done</b> is true, flush the data in the state and finish the
 * compression/uncompression.  Return -1 on failure, 0 on success. */
int
buf_add_compress(buf_t *buf, tor_compress_state_t *state,
                 const char *data, size_t data_len,
                 const int done)
{
  char *next;
  size_t old_avail, avail;
  int over = 0;

  do {
    int need_new_chunk = 0;
    if (!buf->tail || ! CHUNK_REMAINING_CAPACITY(buf->tail)) {
      size_t cap = data_len / 4;
      buf_add_chunk_with_capacity(buf, cap, 1);
    }
    next = CHUNK_WRITE_PTR(buf->tail);
    avail = old_avail = CHUNK_REMAINING_CAPACITY(buf->tail);
    switch (tor_compress_process(state, &next, &avail,
                                 &data, &data_len, done)) {
      case TOR_COMPRESS_DONE:
        over = 1;
        break;
      case TOR_COMPRESS_ERROR:
        return -1;
      case TOR_COMPRESS_OK:
        if (data_len == 0) {
          tor_assert_nonfatal(!done);
          over = 1;
        }
        break;
      case TOR_COMPRESS_BUFFER_FULL:
        if (avail) {
          /* The compression module says we need more room
           * (TOR_COMPRESS_BUFFER_FULL).  Start a new chunk automatically,
           * whether were going to or not. */
          need_new_chunk = 1;
        }
        if (data_len == 0 && !done) {
          /* We've consumed all the input data, though, so there's no
           * point in forging ahead right now. */
          over = 1;
        }
        break;
    }
    buf->datalen += old_avail - avail;
    buf->tail->datalen += old_avail - avail;
    if (need_new_chunk) {
      buf_add_chunk_with_capacity(buf, data_len/4, 1);
    }

  } while (!over);
  check();
  return 0;
}
