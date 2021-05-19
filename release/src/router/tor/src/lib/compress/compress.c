/* Copyright (c) 2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compress.c
 * \brief Common compression API implementation.
 *
 * This file provides a unified interface to all the compression libraries Tor
 * knows how to use.
 **/

#include "orconfig.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/cc/torint.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/arch/bytes.h"
#include "lib/ctime/di_ops.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_lzma.h"
#include "lib/compress/compress_none.h"
#include "lib/compress/compress_sys.h"
#include "lib/compress/compress_zlib.h"
#include "lib/compress/compress_zstd.h"
#include "lib/intmath/cmp.h"
#include "lib/malloc/malloc.h"
#include "lib/subsys/subsys.h"
#include "lib/thread/threads.h"

/** Total number of bytes allocated for compression state overhead. */
static atomic_counter_t total_compress_allocation;

/** @{ */
/* These macros define the maximum allowable compression factor.  Anything of
 * size greater than CHECK_FOR_COMPRESSION_BOMB_AFTER is not allowed to
 * have an uncompression factor (uncompressed size:compressed size ratio) of
 * any greater than MAX_UNCOMPRESSION_FACTOR.
 *
 * Picking a value for MAX_UNCOMPRESSION_FACTOR is a trade-off: we want it to
 * be small to limit the attack multiplier, but we also want it to be large
 * enough so that no legitimate document --even ones we might invent in the
 * future -- ever compresses by a factor of greater than
 * MAX_UNCOMPRESSION_FACTOR. Within those parameters, there's a reasonably
 * large range of possible values. IMO, anything over 8 is probably safe; IMO
 * anything under 50 is probably sufficient.
 */
#define MAX_UNCOMPRESSION_FACTOR 25
#define CHECK_FOR_COMPRESSION_BOMB_AFTER (1024*64)
/** @} */

/** Return true if uncompressing an input of size <b>in_size</b> to an input of
 * size at least <b>size_out</b> looks like a compression bomb. */
MOCK_IMPL(int,
tor_compress_is_compression_bomb,(size_t size_in, size_t size_out))
{
  if (size_in == 0 || size_out < CHECK_FOR_COMPRESSION_BOMB_AFTER)
    return 0;

  return (size_out / size_in > MAX_UNCOMPRESSION_FACTOR);
}

/** Guess the size that <b>in_len</b> will be after compression or
 * decompression. */
static size_t
guess_compress_size(int compress, compress_method_t method,
                    compression_level_t compression_level,
                    size_t in_len)
{
  // ignore these for now.
  (void)compression_level;
  if (method == NO_METHOD) {
    /* Guess that we'll need an extra byte, to avoid a needless realloc
     * for nul-termination */
    return (in_len < SIZE_MAX) ? in_len + 1 : in_len;
  }

  /* Always guess a factor of 2. */
  if (compress) {
    in_len /= 2;
  } else {
    if (in_len < SIZE_T_CEILING/2)
      in_len *= 2;
  }
  return MAX(in_len, 1024);
}

/** Internal function to implement tor_compress/tor_uncompress, depending on
 * whether <b>compress</b> is set.  All arguments are as for tor_compress or
 * tor_uncompress. */
static int
tor_compress_impl(int compress,
                  char **out, size_t *out_len,
                  const char *in, size_t in_len,
                  compress_method_t method,
                  compression_level_t compression_level,
                  int complete_only,
                  int protocol_warn_level)
{
  tor_compress_state_t *stream;
  int rv;

  stream = tor_compress_new(compress, method, compression_level);

  if (stream == NULL) {
    log_warn(LD_GENERAL, "NULL stream while %scompressing",
             compress?"":"de");
    log_debug(LD_GENERAL, "method: %d level: %d at len: %lu",
              method, compression_level, (unsigned long)in_len);
    return -1;
  }

  size_t in_len_orig = in_len;
  size_t out_remaining, out_alloc;
  char *outptr;

  out_remaining = out_alloc =
    guess_compress_size(compress, method, compression_level, in_len);
  *out = outptr = tor_malloc(out_remaining);

  const int finish = complete_only || compress;

  while (1) {
    switch (tor_compress_process(stream,
                                 &outptr, &out_remaining,
                                 &in, &in_len, finish)) {
      case TOR_COMPRESS_DONE:
        if (in_len == 0 || compress) {
          goto done;
        } else {
          // More data is present, and we're decompressing.  So we may need to
          // reinitialize the stream if we are handling multiple concatenated
          // inputs.
          tor_compress_free(stream);
          stream = tor_compress_new(compress, method, compression_level);
          if (stream == NULL) {
            log_warn(LD_GENERAL, "NULL stream while %scompressing",
                     compress?"":"de");
            goto err;
          }
        }
        break;
      case TOR_COMPRESS_OK:
        if (compress || complete_only) {
          log_fn(protocol_warn_level, LD_PROTOCOL,
                 "Unexpected %s while %scompressing",
                 complete_only?"end of input":"result",
                 compress?"":"de");
          log_debug(LD_GENERAL, "method: %d level: %d at len: %lu",
                    method, compression_level, (unsigned long)in_len);
          goto err;
        } else {
          if (in_len == 0) {
            goto done;
          }
        }
        break;
      case TOR_COMPRESS_BUFFER_FULL: {
        if (!compress && outptr < *out+out_alloc) {
          // A buffer error in this case means that we have a problem
          // with our input.
          log_fn(protocol_warn_level, LD_PROTOCOL,
                 "Possible truncated or corrupt compressed data");
          goto err;
        }
        if (out_alloc >= SIZE_T_CEILING / 2) {
          log_warn(LD_GENERAL, "While %scompressing data: ran out of space.",
                   compress?"":"un");
          goto err;
        }
        if (!compress &&
            tor_compress_is_compression_bomb(in_len_orig, out_alloc)) {
          // This should already have been caught down in the backend logic.
          // LCOV_EXCL_START
          tor_assert_nonfatal_unreached();
          goto err;
          // LCOV_EXCL_STOP
        }
        const size_t offset = outptr - *out;
        out_alloc *= 2;
        *out = tor_realloc(*out, out_alloc);
        outptr = *out + offset;
        out_remaining = out_alloc - offset;
        break;
      }
      case TOR_COMPRESS_ERROR:
        log_fn(protocol_warn_level, LD_GENERAL,
               "Error while %scompressing data: bad input?",
               compress?"":"un");
        goto err; // bad data.

        // LCOV_EXCL_START
      default:
        tor_assert_nonfatal_unreached();
        goto err;
        // LCOV_EXCL_STOP
    }
  }
 done:
  *out_len = outptr - *out;
  if (compress && tor_compress_is_compression_bomb(*out_len, in_len_orig)) {
    log_warn(LD_BUG, "We compressed something and got an insanely high "
             "compression factor; other Tors would think this was a "
             "compression bomb.");
    goto err;
  }
  if (!compress) {
    // NUL-terminate our output.
    if (out_alloc == *out_len)
      *out = tor_realloc(*out, out_alloc + 1);
    (*out)[*out_len] = '\0';
  }
  rv = 0;
  goto out;

 err:
  tor_free(*out);
  *out_len = 0;
  rv = -1;
  goto out;

 out:
  tor_compress_free(stream);
  return rv;
}

/** Given <b>in_len</b> bytes at <b>in</b>, compress them into a newly
 * allocated buffer, using the method described in <b>method</b>.  Store the
 * compressed string in *<b>out</b>, and its length in *<b>out_len</b>.
 * Return 0 on success, -1 on failure.
 */
int
tor_compress(char **out, size_t *out_len,
             const char *in, size_t in_len,
             compress_method_t method)
{
  return tor_compress_impl(1, out, out_len, in, in_len, method,
                           BEST_COMPRESSION,
                           1, LOG_WARN);
}

/** Given zero or more compressed strings of total length <b>in_len</b> bytes
 * at <b>in</b>, uncompress them into a newly allocated buffer, using the
 * method described in <b>method</b>.  Store the uncompressed string in
 * *<b>out</b>, and its length in *<b>out_len</b>.  Return 0 on success, -1 on
 * failure.
 *
 * If any bytes are written to <b>out</b>, an extra byte NUL is always
 * written at the end, but not counted in <b>out_len</b>.  This is a
 * safety feature to ensure that the output can be treated as a
 * NUL-terminated string -- though of course, callers should check
 * out_len anyway.
 *
 * If <b>complete_only</b> is true, we consider a truncated input as a
 * failure; otherwise we decompress as much as we can.  Warn about truncated
 * or corrupt inputs at <b>protocol_warn_level</b>.
 */
int
tor_uncompress(char **out, size_t *out_len,
               const char *in, size_t in_len,
               compress_method_t method,
               int complete_only,
               int protocol_warn_level)
{
  return tor_compress_impl(0, out, out_len, in, in_len, method,
                           BEST_COMPRESSION,
                           complete_only, protocol_warn_level);
}

/** Try to tell whether the <b>in_len</b>-byte string in <b>in</b> is likely
 * to be compressed or not.  If it is, return the likeliest compression method.
 * Otherwise, return UNKNOWN_METHOD.
 */
compress_method_t
detect_compression_method(const char *in, size_t in_len)
{
  if (in_len > 2 && fast_memeq(in, "\x1f\x8b", 2)) {
    return GZIP_METHOD;
  } else if (in_len > 2 && (in[0] & 0x0f) == 8 &&
             (tor_ntohs(get_uint16(in)) % 31) == 0) {
    return ZLIB_METHOD;
  } else if (in_len > 2 &&
             fast_memeq(in, "\x5d\x00\x00", 3)) {
    return LZMA_METHOD;
  } else if (in_len > 3 &&
             fast_memeq(in, "\x28\xb5\x2f\xfd", 4)) {
    return ZSTD_METHOD;
  } else {
    return UNKNOWN_METHOD;
  }
}

/** Return 1 if a given <b>method</b> is supported; otherwise 0. */
int
tor_compress_supports_method(compress_method_t method)
{
  switch (method) {
    case GZIP_METHOD:
    case ZLIB_METHOD:
      return tor_zlib_method_supported();
    case LZMA_METHOD:
      return tor_lzma_method_supported();
    case ZSTD_METHOD:
      return tor_zstd_method_supported();
    case NO_METHOD:
      return 1;
    case UNKNOWN_METHOD:
    default:
      return 0;
  }
}

/**
 * Return a bitmask of the supported compression types, where 1&lt;&lt;m is
 * set in the bitmask if and only if compression with method <b>m</b> is
 * supported.
 */
unsigned
tor_compress_get_supported_method_bitmask(void)
{
  static unsigned supported = 0;
  if (supported == 0) {
    compress_method_t m;
    for (m = NO_METHOD; m <= UNKNOWN_METHOD; ++m) {
      if (tor_compress_supports_method(m)) {
        supported |= (1u << m);
      }
    }
  }
  return supported;
}

/** Table of compression method names.  These should have an "x-" prefix,
 * if they are not listed in the IANA content coding registry. */
static const struct {
  const char *name;
  compress_method_t method;
} compression_method_names[] = {
  { "gzip", GZIP_METHOD },
  { "deflate", ZLIB_METHOD },
  // We call this "x-tor-lzma" rather than "x-lzma", because we impose a
  // lower maximum memory usage on the decoding side.
  { "x-tor-lzma", LZMA_METHOD },
  { "x-zstd" , ZSTD_METHOD },
  { "identity", NO_METHOD },

  /* Later entries in this table are not canonical; these are recognized but
   * not emitted. */
  { "x-gzip", GZIP_METHOD },
};

/** Return the canonical string representation of the compression method
 * <b>method</b>, or NULL if the method isn't recognized. */
const char *
compression_method_get_name(compress_method_t method)
{
  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(compression_method_names); ++i) {
    if (method == compression_method_names[i].method)
      return compression_method_names[i].name;
  }
  return NULL;
}

/** Table of compression human readable method names. */
static const struct {
  compress_method_t method;
  const char *name;
} compression_method_human_names[] = {
  { NO_METHOD, "uncompressed" },
  { GZIP_METHOD, "gzipped" },
  { ZLIB_METHOD, "deflated" },
  { LZMA_METHOD, "LZMA compressed" },
  { ZSTD_METHOD, "Zstandard compressed" },
  { UNKNOWN_METHOD, "unknown encoding" },
};

/** Return a human readable string representation of the compression method
 * <b>method</b>, or NULL if the method isn't recognized. */
const char *
compression_method_get_human_name(compress_method_t method)
{
  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(compression_method_human_names); ++i) {
    if (method == compression_method_human_names[i].method)
      return compression_method_human_names[i].name;
  }
  return NULL;
}

/** Return the compression method represented by the string <b>name</b>, or
 * UNKNOWN_METHOD if the string isn't recognized. */
compress_method_t
compression_method_get_by_name(const char *name)
{
  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(compression_method_names); ++i) {
    if (!strcmp(compression_method_names[i].name, name))
      return compression_method_names[i].method;
  }
  return UNKNOWN_METHOD;
}

/** Return a string representation of the version of the library providing the
 * compression method given in <b>method</b>. Returns NULL if <b>method</b> is
 * unknown or unsupported. */
const char *
tor_compress_version_str(compress_method_t method)
{
  switch (method) {
    case GZIP_METHOD:
    case ZLIB_METHOD:
      return tor_zlib_get_version_str();
    case LZMA_METHOD:
      return tor_lzma_get_version_str();
    case ZSTD_METHOD:
      return tor_zstd_get_version_str();
    case NO_METHOD:
    case UNKNOWN_METHOD:
    default:
      return NULL;
  }
}

/** Return a string representation of the version of the library, found at
 * compile time, providing the compression method given in <b>method</b>.
 * Returns NULL if <b>method</b> is unknown or unsupported. */
const char *
tor_compress_header_version_str(compress_method_t method)
{
  switch (method) {
    case GZIP_METHOD:
    case ZLIB_METHOD:
      return tor_zlib_get_header_version_str();
    case LZMA_METHOD:
      return tor_lzma_get_header_version_str();
    case ZSTD_METHOD:
      return tor_zstd_get_header_version_str();
    case NO_METHOD:
    case UNKNOWN_METHOD:
    default:
      return NULL;
  }
}

/** Return the approximate number of bytes allocated for all
 * supported compression schemas. */
size_t
tor_compress_get_total_allocation(void)
{
  return atomic_counter_get(&total_compress_allocation) +
         tor_zlib_get_total_allocation() +
         tor_lzma_get_total_allocation() +
         tor_zstd_get_total_allocation();
}

/** Internal state for an incremental compression/decompression.  The body of
 * this struct is not exposed. */
struct tor_compress_state_t {
  compress_method_t method; /**< The compression method. */

  union {
    tor_zlib_compress_state_t *zlib_state;
    tor_lzma_compress_state_t *lzma_state;
    tor_zstd_compress_state_t *zstd_state;
  } u; /**< Compression backend state. */
};

/** Construct and return a tor_compress_state_t object using <b>method</b>.  If
 * <b>compress</b>, it's for compression; otherwise it's for decompression. */
tor_compress_state_t *
tor_compress_new(int compress, compress_method_t method,
                 compression_level_t compression_level)
{
  tor_compress_state_t *state;

  state = tor_malloc_zero(sizeof(tor_compress_state_t));
  state->method = method;

  switch (method) {
    case GZIP_METHOD:
    case ZLIB_METHOD: {
      tor_zlib_compress_state_t *zlib_state =
        tor_zlib_compress_new(compress, method, compression_level);

      if (zlib_state == NULL)
        goto err;

      state->u.zlib_state = zlib_state;
      break;
    }
    case LZMA_METHOD: {
      tor_lzma_compress_state_t *lzma_state =
        tor_lzma_compress_new(compress, method, compression_level);

      if (lzma_state == NULL)
        goto err;

      state->u.lzma_state = lzma_state;
      break;
    }
    case ZSTD_METHOD: {
      tor_zstd_compress_state_t *zstd_state =
        tor_zstd_compress_new(compress, method, compression_level);

      if (zstd_state == NULL)
        goto err;

      state->u.zstd_state = zstd_state;
      break;
    }
    case NO_METHOD: {
      break;
    }
    case UNKNOWN_METHOD:
      goto err;
  }

  atomic_counter_add(&total_compress_allocation,
                     sizeof(tor_compress_state_t));
  return state;

 err:
  tor_free(state);
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
tor_compress_process(tor_compress_state_t *state,
                     char **out, size_t *out_len,
                     const char **in, size_t *in_len,
                     int finish)
{
  tor_assert(state != NULL);
  const size_t in_len_orig = *in_len;
  const size_t out_len_orig = *out_len;
  tor_compress_output_t rv;

  if (*out_len == 0 && (*in_len > 0 || finish)) {
    // If we still have input data, but no space for output data, we might as
    // well return early and let the caller do the reallocation of the out
    // variable.
    return TOR_COMPRESS_BUFFER_FULL;
  }

  switch (state->method) {
    case GZIP_METHOD:
    case ZLIB_METHOD:
      rv = tor_zlib_compress_process(state->u.zlib_state,
                                     out, out_len, in, in_len,
                                     finish);
      break;
    case LZMA_METHOD:
      rv = tor_lzma_compress_process(state->u.lzma_state,
                                     out, out_len, in, in_len,
                                     finish);
      break;
    case ZSTD_METHOD:
      rv = tor_zstd_compress_process(state->u.zstd_state,
                                     out, out_len, in, in_len,
                                     finish);
      break;
    case NO_METHOD:
      rv = tor_cnone_compress_process(out, out_len, in, in_len,
                                      finish);
      break;
    default:
    case UNKNOWN_METHOD:
      goto err;
  }
  if (BUG((rv == TOR_COMPRESS_OK) &&
          *in_len == in_len_orig &&
          *out_len == out_len_orig)) {
    log_warn(LD_GENERAL,
             "More info on the bug: method == %s, finish == %d, "
             " *in_len == in_len_orig == %lu, "
             "*out_len == out_len_orig == %lu",
             compression_method_get_human_name(state->method), finish,
             (unsigned long)in_len_orig, (unsigned long)out_len_orig);
    return TOR_COMPRESS_ERROR;
  }

  return rv;
 err:
  return TOR_COMPRESS_ERROR;
}

/** Deallocate <b>state</b>. */
void
tor_compress_free_(tor_compress_state_t *state)
{
  if (state == NULL)
    return;

  switch (state->method) {
    case GZIP_METHOD:
    case ZLIB_METHOD:
      tor_zlib_compress_free(state->u.zlib_state);
      break;
    case LZMA_METHOD:
      tor_lzma_compress_free(state->u.lzma_state);
      break;
    case ZSTD_METHOD:
      tor_zstd_compress_free(state->u.zstd_state);
      break;
    case NO_METHOD:
      break;
    case UNKNOWN_METHOD:
      break;
  }

  atomic_counter_sub(&total_compress_allocation,
                     sizeof(tor_compress_state_t));
  tor_free(state);
}

/** Return the approximate number of bytes allocated for <b>state</b>. */
size_t
tor_compress_state_size(const tor_compress_state_t *state)
{
  tor_assert(state != NULL);

  size_t size = sizeof(tor_compress_state_t);

  switch (state->method) {
    case GZIP_METHOD:
    case ZLIB_METHOD:
      size += tor_zlib_compress_state_size(state->u.zlib_state);
      break;
    case LZMA_METHOD:
      size += tor_lzma_compress_state_size(state->u.lzma_state);
      break;
    case ZSTD_METHOD:
      size += tor_zstd_compress_state_size(state->u.zstd_state);
      break;
    case NO_METHOD:
    case UNKNOWN_METHOD:
      break;
  }

  return size;
}

/** Initialize all compression modules. */
int
tor_compress_init(void)
{
  atomic_counter_init(&total_compress_allocation);

  tor_zlib_init();
  tor_lzma_init();
  tor_zstd_init();

  return 0;
}

/** Warn if we had any problems while setting up our compression libraries.
 *
 * (This isn't part of tor_compress_init, since the logs aren't set up yet.)
 */
void
tor_compress_log_init_warnings(void)
{
  // XXXX can we move this into tor_compress_init() after all?  log.c queues
  // XXXX log messages at startup.
  tor_zstd_warn_if_version_mismatched();
}

static int
subsys_compress_initialize(void)
{
  return tor_compress_init();
}

const subsys_fns_t sys_compress = {
  .name = "compress",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  .level = -55,
  .initialize = subsys_compress_initialize,
};
