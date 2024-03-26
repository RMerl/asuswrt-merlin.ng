/* Copyright (c) 2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compress_lzma.c
 * \brief Compression backend for LZMA.
 *
 * This module should never be invoked directly. Use the compress module
 * instead.
 **/

#include "orconfig.h"

#include "lib/compress/compress.h"
#include "lib/compress/compress_lzma.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/thread/threads.h"

#ifdef HAVE_LZMA
#include <lzma.h>
#endif

/** The maximum amount of memory we allow the LZMA decoder to use, in bytes. */
#define MEMORY_LIMIT (16 * 1024 * 1024)

/** Total number of bytes allocated for LZMA state. */
static atomic_counter_t total_lzma_allocation;

#ifdef HAVE_LZMA
/** Given <b>level</b> return the memory level. */
static int
memory_level(compression_level_t level)
{
  switch (level) {
    default:
    case BEST_COMPRESSION:
    case HIGH_COMPRESSION: return 6;
    case MEDIUM_COMPRESSION: return 4;
    case LOW_COMPRESSION: return 2;
  }
}

/** Convert a given <b>error</b> to a human readable error string. */
static const char *
lzma_error_str(lzma_ret error)
{
  switch (error) {
    case LZMA_OK:
      return "Operation completed successfully";
    case LZMA_STREAM_END:
      return "End of stream";
    case LZMA_NO_CHECK:
      return "Input stream lacks integrity check";
    case LZMA_UNSUPPORTED_CHECK:
      return "Unable to calculate integrity check";
    case LZMA_GET_CHECK:
      return "Integrity check available";
    case LZMA_MEM_ERROR:
      return "Unable to allocate memory";
    case LZMA_MEMLIMIT_ERROR:
      return "Memory limit reached";
    case LZMA_FORMAT_ERROR:
      return "Unknown file format";
    case LZMA_OPTIONS_ERROR:
      return "Unsupported options";
    case LZMA_DATA_ERROR:
      return "Corrupt input data";
    case LZMA_BUF_ERROR:
      return "Unable to progress";
    case LZMA_PROG_ERROR:
      return "Programming error";
#if LZMA_VERSION >= 50030010
    case LZMA_SEEK_NEEDED:
      // This can be returned by the .xz file_info decoder but with
      // lzma_alone_decoder/encoder as we use, it should never be seen.
      return "Seek needed";
#endif
#if LZMA_VERSION >= 50030020
    case LZMA_RET_INTERNAL1:
    case LZMA_RET_INTERNAL2:
    case LZMA_RET_INTERNAL3:
    case LZMA_RET_INTERNAL4:
    case LZMA_RET_INTERNAL5:
    case LZMA_RET_INTERNAL6:
    case LZMA_RET_INTERNAL7:
    case LZMA_RET_INTERNAL8:
      FALLTHROUGH;
#endif
    default:
      return "Unknown LZMA error";
  }
}
#endif /* defined(HAVE_LZMA) */

/** Return 1 if LZMA compression is supported; otherwise 0. */
int
tor_lzma_method_supported(void)
{
#ifdef HAVE_LZMA
  return 1;
#else
  return 0;
#endif
}

/** Return a string representation of the version of the currently running
 * version of liblzma. Returns NULL if LZMA is unsupported. */
const char *
tor_lzma_get_version_str(void)
{
#ifdef HAVE_LZMA
  return lzma_version_string();
#else
  return NULL;
#endif
}

/** Return a string representation of the version of liblzma used at
 * compilation time. Returns NULL if LZMA is unsupported. */
const char *
tor_lzma_get_header_version_str(void)
{
#ifdef HAVE_LZMA
  return LZMA_VERSION_STRING;
#else
  return NULL;
#endif
}

/** Internal LZMA state for incremental compression/decompression.
 * The body of this struct is not exposed. */
struct tor_lzma_compress_state_t {
#ifdef HAVE_LZMA
  lzma_stream stream; /**< The LZMA stream. */
#endif

  int compress; /**< True if we are compressing; false if we are inflating */

  /** Number of bytes read so far.  Used to detect compression bombs. */
  size_t input_so_far;
  /** Number of bytes written so far.  Used to detect compression bombs. */
  size_t output_so_far;

  /** Approximate number of bytes allocated for this object. */
  size_t allocation;
};

#ifdef HAVE_LZMA
/** Return an approximate number of bytes stored in memory to hold the LZMA
 * encoder/decoder state. */
static size_t
tor_lzma_state_size_precalc(int compress, compression_level_t level)
{
  uint64_t memory_usage;

  if (compress)
    memory_usage = lzma_easy_encoder_memusage(memory_level(level));
  else
    memory_usage = lzma_easy_decoder_memusage(memory_level(level));

  if (memory_usage == UINT64_MAX) {
    // LCOV_EXCL_START
    log_warn(LD_GENERAL, "Unsupported compression level passed to LZMA %s",
                         compress ? "encoder" : "decoder");
    goto err;
    // LCOV_EXCL_STOP
  }

  if (memory_usage + sizeof(tor_lzma_compress_state_t) > SIZE_MAX)
    memory_usage = SIZE_MAX;
  else
    memory_usage += sizeof(tor_lzma_compress_state_t);

  return (size_t)memory_usage;

 // LCOV_EXCL_START
 err:
  return 0;
 // LCOV_EXCL_STOP
}
#endif /* defined(HAVE_LZMA) */

/** Construct and return a tor_lzma_compress_state_t object using
 * <b>method</b>. If <b>compress</b>, it's for compression; otherwise it's for
 * decompression. */
tor_lzma_compress_state_t *
tor_lzma_compress_new(int compress,
                      compress_method_t method,
                      compression_level_t level)
{
  tor_assert(method == LZMA_METHOD);

#ifdef HAVE_LZMA
  tor_lzma_compress_state_t *result;
  lzma_ret retval;
  lzma_options_lzma stream_options;

  // Note that we do not explicitly initialize the lzma_stream object here,
  // since the LZMA_STREAM_INIT "just" initializes all members to 0, which is
  // also what `tor_malloc_zero()` does.
  result = tor_malloc_zero(sizeof(tor_lzma_compress_state_t));
  result->compress = compress;
  result->allocation = tor_lzma_state_size_precalc(compress, level);

  if (compress) {
    lzma_lzma_preset(&stream_options, memory_level(level));

    retval = lzma_alone_encoder(&result->stream, &stream_options);

    if (retval != LZMA_OK) {
      // LCOV_EXCL_START
      log_warn(LD_GENERAL, "Error from LZMA encoder: %s (%u).",
               lzma_error_str(retval), retval);
      goto err;
      // LCOV_EXCL_STOP
    }
  } else {
    retval = lzma_alone_decoder(&result->stream, MEMORY_LIMIT);

    if (retval != LZMA_OK) {
      // LCOV_EXCL_START
      log_warn(LD_GENERAL, "Error from LZMA decoder: %s (%u).",
               lzma_error_str(retval), retval);
      goto err;
      // LCOV_EXCL_STOP
    }
  }

  atomic_counter_add(&total_lzma_allocation, result->allocation);
  return result;

 /* LCOV_EXCL_START */
 err:
  tor_free(result);
  return NULL;
 /* LCOV_EXCL_STOP */
#else /* !defined(HAVE_LZMA) */
  (void)compress;
  (void)method;
  (void)level;

  return NULL;
#endif /* defined(HAVE_LZMA) */
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
tor_lzma_compress_process(tor_lzma_compress_state_t *state,
                          char **out, size_t *out_len,
                          const char **in, size_t *in_len,
                          int finish)
{
#ifdef HAVE_LZMA
  lzma_ret retval;
  lzma_action action;

  tor_assert(state != NULL);
  tor_assert(*in_len <= UINT_MAX);
  tor_assert(*out_len <= UINT_MAX);

  state->stream.next_in = (unsigned char *)*in;
  state->stream.avail_in = *in_len;
  state->stream.next_out = (unsigned char *)*out;
  state->stream.avail_out = *out_len;

  action = finish ? LZMA_FINISH : LZMA_RUN;

  retval = lzma_code(&state->stream, action);

  state->input_so_far += state->stream.next_in - ((unsigned char *)*in);
  state->output_so_far += state->stream.next_out - ((unsigned char *)*out);

  *out = (char *)state->stream.next_out;
  *out_len = state->stream.avail_out;
  *in = (const char *)state->stream.next_in;
  *in_len = state->stream.avail_in;

  if (! state->compress &&
      tor_compress_is_compression_bomb(state->input_so_far,
                                       state->output_so_far)) {
    log_warn(LD_DIR, "Possible compression bomb; abandoning stream.");
    return TOR_COMPRESS_ERROR;
  }

  switch (retval) {
    case LZMA_OK:
      if (state->stream.avail_out == 0 || finish)
        return TOR_COMPRESS_BUFFER_FULL;

      return TOR_COMPRESS_OK;

    case LZMA_BUF_ERROR:
      if (state->stream.avail_in == 0 && !finish)
        return TOR_COMPRESS_OK;

      return TOR_COMPRESS_BUFFER_FULL;

    case LZMA_STREAM_END:
      return TOR_COMPRESS_DONE;

    // We list all the possible values of `lzma_ret` here to silence the
    // `switch-enum` warning and to detect if a new member was added.
    case LZMA_NO_CHECK:
    case LZMA_UNSUPPORTED_CHECK:
    case LZMA_GET_CHECK:
    case LZMA_MEM_ERROR:
    case LZMA_MEMLIMIT_ERROR:
    case LZMA_FORMAT_ERROR:
    case LZMA_OPTIONS_ERROR:
    case LZMA_DATA_ERROR:
    case LZMA_PROG_ERROR:
#if LZMA_VERSION >= 50030010
    case LZMA_SEEK_NEEDED:
#endif
#if LZMA_VERSION >= 50030020
    case LZMA_RET_INTERNAL1:
    case LZMA_RET_INTERNAL2:
    case LZMA_RET_INTERNAL3:
    case LZMA_RET_INTERNAL4:
    case LZMA_RET_INTERNAL5:
    case LZMA_RET_INTERNAL6:
    case LZMA_RET_INTERNAL7:
    case LZMA_RET_INTERNAL8:
#endif
    default:
      log_warn(LD_GENERAL, "LZMA %s didn't finish: %s.",
               state->compress ? "compression" : "decompression",
               lzma_error_str(retval));
      return TOR_COMPRESS_ERROR;
  }
#else /* !defined(HAVE_LZMA) */
  (void)state;
  (void)out;
  (void)out_len;
  (void)in;
  (void)in_len;
  (void)finish;
  return TOR_COMPRESS_ERROR;
#endif /* defined(HAVE_LZMA) */
}

/** Deallocate <b>state</b>. */
void
tor_lzma_compress_free_(tor_lzma_compress_state_t *state)
{
  if (state == NULL)
    return;

  atomic_counter_sub(&total_lzma_allocation, state->allocation);

#ifdef HAVE_LZMA
  lzma_end(&state->stream);
#endif

  tor_free(state);
}

/** Return the approximate number of bytes allocated for <b>state</b>. */
size_t
tor_lzma_compress_state_size(const tor_lzma_compress_state_t *state)
{
  tor_assert(state != NULL);
  return state->allocation;
}

/** Return the approximate number of bytes allocated for all LZMA states. */
size_t
tor_lzma_get_total_allocation(void)
{
  return atomic_counter_get(&total_lzma_allocation);
}

/** Initialize the lzma module */
void
tor_lzma_init(void)
{
  atomic_counter_init(&total_lzma_allocation);
}
