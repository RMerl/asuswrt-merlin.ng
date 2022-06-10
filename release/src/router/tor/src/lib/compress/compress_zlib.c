/* Copyright (c) 2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compress_zlib.c
 * \brief Compression backend for gzip and zlib.
 *
 * This module should never be invoked directly. Use the compress module
 * instead.
 **/

#include "orconfig.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_zlib.h"
#include "lib/thread/threads.h"

/* zlib 1.2.4 and 1.2.5 do some "clever" things with macros.  Instead of
   saying "(defined(FOO) ? FOO : 0)" they like to say "FOO-0", on the theory
   that nobody will care if the compile outputs a no-such-identifier warning.

   Sorry, but we like -Werror over here, so I guess we need to define these.
   I hope that zlib 1.2.6 doesn't break these too.
*/
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 0
#endif
#ifndef _LFS64_LARGEFILE
#define _LFS64_LARGEFILE 0
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 0
#endif
#ifndef off64_t
#define off64_t int64_t
#endif

#include <zlib.h>

#if defined ZLIB_VERNUM && ZLIB_VERNUM < 0x1200
#error "We require zlib version 1.2 or later."
#endif

static size_t tor_zlib_state_size_precalc(int inflate,
                                          int windowbits, int memlevel);

/** Total number of bytes allocated for zlib state */
static atomic_counter_t total_zlib_allocation;

/** Given <b>level</b> return the memory level. */
static int
memory_level(compression_level_t level)
{
  switch (level) {
    default:
    case BEST_COMPRESSION: return 9;
    case HIGH_COMPRESSION: return 8;
    case MEDIUM_COMPRESSION: return 7;
    case LOW_COMPRESSION: return 6;
  }
}

/** Return the 'bits' value to tell zlib to use <b>method</b>.*/
static inline int
method_bits(compress_method_t method, compression_level_t level)
{
  /* Bits+16 means "use gzip" in zlib >= 1.2 */
  const int flag = method == GZIP_METHOD ? 16 : 0;
  switch (level) {
    default:
    case BEST_COMPRESSION:
    case HIGH_COMPRESSION: return flag + 15;
    case MEDIUM_COMPRESSION: return flag + 13;
    case LOW_COMPRESSION: return flag + 11;
  }
}

/** Return 1 if zlib/gzip compression is supported; otherwise 0. */
int
tor_zlib_method_supported(void)
{
  /* We currently always support zlib/gzip, but we keep this function around in
   * case we some day decide to deprecate zlib/gzip support.
   */
  return 1;
}

/** Return a string representation of the version of the currently running
 * version of zlib. */
const char *
tor_zlib_get_version_str(void)
{
  return zlibVersion();
}

/** Return a string representation of the version of the version of zlib
* used at compilation. */
const char *
tor_zlib_get_header_version_str(void)
{
  return ZLIB_VERSION;
}

/** Internal zlib state for an incremental compression/decompression.
 * The body of this struct is not exposed. */
struct tor_zlib_compress_state_t {
  struct z_stream_s stream; /**< The zlib stream */
  int compress; /**< True if we are compressing; false if we are inflating */

  /** Number of bytes read so far.  Used to detect zlib bombs. */
  size_t input_so_far;
  /** Number of bytes written so far.  Used to detect zlib bombs. */
  size_t output_so_far;

  /** Approximate number of bytes allocated for this object. */
  size_t allocation;
};

/** Return an approximate number of bytes used in RAM to hold a state with
 * window bits <b>windowBits</b> and compression level 'memlevel' */
static size_t
tor_zlib_state_size_precalc(int inflate_, int windowbits, int memlevel)
{
  windowbits &= 15;

#define A_FEW_KILOBYTES 2048

  if (inflate_) {
    /* From zconf.h:

       "The memory requirements for inflate are (in bytes) 1 << windowBits
       that is, 32K for windowBits=15 (default value) plus a few kilobytes
       for small objects."
    */
    return sizeof(tor_zlib_compress_state_t) + sizeof(struct z_stream_s) +
      (1 << 15) + A_FEW_KILOBYTES;
  } else {
    /* Also from zconf.h:

       "The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
        ... plus a few kilobytes for small objects."
    */
    return sizeof(tor_zlib_compress_state_t) + sizeof(struct z_stream_s) +
      (1 << (windowbits + 2)) + (1 << (memlevel + 9)) + A_FEW_KILOBYTES;
  }
#undef A_FEW_KILOBYTES
}

/** Construct and return a tor_zlib_compress_state_t object using
 * <b>method</b>. If <b>compress</b>, it's for compression; otherwise it's for
 * decompression. */
tor_zlib_compress_state_t *
tor_zlib_compress_new(int compress_,
                      compress_method_t method,
                      compression_level_t compression_level)
{
  tor_zlib_compress_state_t *out;
  int bits, memlevel;

  if (! compress_) {
    /* use this setting for decompression, since we might have the
     * max number of window bits */
    compression_level = BEST_COMPRESSION;
  }

  out = tor_malloc_zero(sizeof(tor_zlib_compress_state_t));
  out->stream.zalloc = Z_NULL;
  out->stream.zfree = Z_NULL;
  out->stream.opaque = NULL;
  out->compress = compress_;
  bits = method_bits(method, compression_level);
  memlevel = memory_level(compression_level);
  if (compress_) {
    if (deflateInit2(&out->stream, Z_BEST_COMPRESSION, Z_DEFLATED,
                     bits, memlevel,
                     Z_DEFAULT_STRATEGY) != Z_OK)
    goto err; // LCOV_EXCL_LINE
  } else {
    if (inflateInit2(&out->stream, bits) != Z_OK)
      goto err; // LCOV_EXCL_LINE
  }
  out->allocation = tor_zlib_state_size_precalc(!compress_, bits, memlevel);

  atomic_counter_add(&total_zlib_allocation, out->allocation);

  return out;

 err:
  tor_free(out);
  return NULL;
}

/** Compress/decompress some bytes using <b>state</b>.  Read up to
 * *<b>in_len</b> bytes from *<b>in</b>, and write up to *<b>out_len</b> bytes
 * to *<b>out</b>, adjusting the values as we go.  If <b>finish</b> is true,
 * we've reached the end of the input.
 *
 * Return TOR_COMPRESS_DONE if we've finished the entire
 * compression/decompression.
 * Return TOR_COMPRESS_OK if we're processed everything from the input.
 * Return TOR_COMPRESS_BUFFER_FULL if we're out of space on <b>out</b>.
 * Return TOR_COMPRESS_ERROR if the stream is corrupt.
 */
tor_compress_output_t
tor_zlib_compress_process(tor_zlib_compress_state_t *state,
                          char **out, size_t *out_len,
                          const char **in, size_t *in_len,
                          int finish)
{
  int err;
  tor_assert(state != NULL);
  if (*in_len > UINT_MAX ||
      *out_len > UINT_MAX) {
    return TOR_COMPRESS_ERROR;
  }

  state->stream.next_in = (unsigned char*) *in;
  state->stream.avail_in = (unsigned int)*in_len;
  state->stream.next_out = (unsigned char*) *out;
  state->stream.avail_out = (unsigned int)*out_len;

  if (state->compress) {
    err = deflate(&state->stream, finish ? Z_FINISH : Z_NO_FLUSH);
  } else {
    err = inflate(&state->stream, finish ? Z_FINISH : Z_SYNC_FLUSH);
  }

  state->input_so_far += state->stream.next_in - ((unsigned char*)*in);
  state->output_so_far += state->stream.next_out - ((unsigned char*)*out);

  *out = (char*) state->stream.next_out;
  *out_len = state->stream.avail_out;
  *in = (const char *) state->stream.next_in;
  *in_len = state->stream.avail_in;

  if (! state->compress &&
      tor_compress_is_compression_bomb(state->input_so_far,
                                       state->output_so_far)) {
    log_warn(LD_DIR, "Possible zlib bomb; abandoning stream.");
    return TOR_COMPRESS_ERROR;
  }

  switch (err)
    {
    case Z_STREAM_END:
      return TOR_COMPRESS_DONE;
    case Z_BUF_ERROR:
      if (state->stream.avail_in == 0 && !finish)
        return TOR_COMPRESS_OK;
      return TOR_COMPRESS_BUFFER_FULL;
    case Z_OK:
      if (state->stream.avail_out == 0 || finish)
        return TOR_COMPRESS_BUFFER_FULL;
      return TOR_COMPRESS_OK;
    default:
      log_warn(LD_GENERAL, "Gzip returned an error: %s",
               state->stream.msg ? state->stream.msg : "<no message>");
      return TOR_COMPRESS_ERROR;
    }
}

/** Deallocate <b>state</b>. */
void
tor_zlib_compress_free_(tor_zlib_compress_state_t *state)
{
  if (state == NULL)
    return;

  atomic_counter_sub(&total_zlib_allocation, state->allocation);

  if (state->compress)
    deflateEnd(&state->stream);
  else
    inflateEnd(&state->stream);

  tor_free(state);
}

/** Return the approximate number of bytes allocated for <b>state</b>. */
size_t
tor_zlib_compress_state_size(const tor_zlib_compress_state_t *state)
{
  tor_assert(state != NULL);
  return state->allocation;
}

/** Return the approximate number of bytes allocated for all zlib states. */
size_t
tor_zlib_get_total_allocation(void)
{
  return atomic_counter_get(&total_zlib_allocation);
}

/** Set up global state for the zlib module */
void
tor_zlib_init(void)
{
  atomic_counter_init(&total_zlib_allocation);
}
