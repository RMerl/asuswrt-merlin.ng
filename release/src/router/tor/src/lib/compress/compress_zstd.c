/* Copyright (c) 2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compress_zstd.c
 * \brief Compression backend for Zstandard.
 *
 * This module should never be invoked directly. Use the compress module
 * instead.
 **/

#include "orconfig.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_zstd.h"
#include "lib/string/printf.h"
#include "lib/thread/threads.h"

#ifdef ENABLE_ZSTD_ADVANCED_APIS
/* This is a lie, but we make sure it doesn't get us in trouble by wrapping
 * all invocations of zstd's static-only functions in a check to make sure
 * that the compile-time version matches the run-time version. */
#define ZSTD_STATIC_LINKING_ONLY
#endif /* defined(ENABLE_ZSTD_ADVANCED_APIS) */

#ifdef HAVE_ZSTD
#ifdef HAVE_CFLAG_WUNUSED_CONST_VARIABLE
DISABLE_GCC_WARNING("-Wunused-const-variable")
#endif
#include <zstd.h>
#ifdef HAVE_CFLAG_WUNUSED_CONST_VARIABLE
ENABLE_GCC_WARNING("-Wunused-const-variable")
#endif
#endif /* defined(HAVE_ZSTD) */

/** Total number of bytes allocated for Zstandard state. */
static atomic_counter_t total_zstd_allocation;

#ifdef HAVE_ZSTD
/** Given <b>level</b> return the memory level. */
static int
memory_level(compression_level_t level)
{
  switch (level) {
    default:
    case BEST_COMPRESSION:
    case HIGH_COMPRESSION: return 9;
    case MEDIUM_COMPRESSION: return 3;
    case LOW_COMPRESSION: return 1;
  }
}
#endif /* defined(HAVE_ZSTD) */

/** Return 1 if Zstandard compression is supported; otherwise 0. */
int
tor_zstd_method_supported(void)
{
#ifdef HAVE_ZSTD
  return 1;
#else
  return 0;
#endif
}

#ifdef HAVE_ZSTD
/** Format a zstd version number as a string in <b>buf</b>. */
static void
tor_zstd_format_version(char *buf, size_t buflen, unsigned version_number)
{
  tor_snprintf(buf, buflen,
               "%u.%u.%u",
               version_number / 10000 % 100,
               version_number / 100 % 100,
               version_number % 100);
}
#endif /* defined(HAVE_ZSTD) */

#define VERSION_STR_MAX_LEN 16 /* more than enough space for 99.99.99 */

/** Return a string representation of the version of the currently running
 * version of libzstd. Returns NULL if Zstandard is unsupported. */
const char *
tor_zstd_get_version_str(void)
{
#ifdef HAVE_ZSTD
  static char version_str[VERSION_STR_MAX_LEN];

  tor_zstd_format_version(version_str, sizeof(version_str),
                          ZSTD_versionNumber());

  return version_str;
#else /* !defined(HAVE_ZSTD) */
  return NULL;
#endif /* defined(HAVE_ZSTD) */
}

/** Return a string representation of the version of the version of libzstd
 * used at compilation time. Returns NULL if Zstandard is unsupported. */
const char *
tor_zstd_get_header_version_str(void)
{
#ifdef HAVE_ZSTD
  return ZSTD_VERSION_STRING;
#else
  return NULL;
#endif
}

#ifdef TOR_UNIT_TESTS
static int static_apis_disable_for_testing = 0;
#endif

/** Return true iff we can use the "static-only" APIs. */
int
tor_zstd_can_use_static_apis(void)
{
#if defined(ZSTD_STATIC_LINKING_ONLY) && defined(HAVE_ZSTD)
#ifdef TOR_UNIT_TESTS
  if (static_apis_disable_for_testing) {
    return 0;
  }
#endif
  return (ZSTD_VERSION_NUMBER == ZSTD_versionNumber());
#else /* !(defined(ZSTD_STATIC_LINKING_ONLY) && defined(HAVE_ZSTD)) */
  return 0;
#endif /* defined(ZSTD_STATIC_LINKING_ONLY) && defined(HAVE_ZSTD) */
}

/** Internal Zstandard state for incremental compression/decompression.
 * The body of this struct is not exposed. */
struct tor_zstd_compress_state_t {
#ifdef HAVE_ZSTD
  union {
    /** Compression stream. Used when <b>compress</b> is true. */
    ZSTD_CStream *compress_stream;
    /** Decompression stream. Used when <b>compress</b> is false. */
    ZSTD_DStream *decompress_stream;
  } u; /**< Zstandard stream objects. */
#endif /* defined(HAVE_ZSTD) */

  int compress; /**< True if we are compressing; false if we are inflating */
  int have_called_end; /**< True if we are compressing and we've called
                        * ZSTD_endStream */

  /** Number of bytes read so far.  Used to detect compression bombs. */
  size_t input_so_far;
  /** Number of bytes written so far.  Used to detect compression bombs. */
  size_t output_so_far;

  /** Approximate number of bytes allocated for this object. */
  size_t allocation;
};

#ifdef HAVE_ZSTD
/** Return an approximate number of bytes stored in memory to hold the
 * Zstandard compression/decompression state. This is a fake estimate
 * based on inspecting the zstd source: tor_zstd_state_size_precalc() is
 * more accurate when it's allowed to use "static-only" functions */
static size_t
tor_zstd_state_size_precalc_fake(int compress, int preset)
{
  tor_assert(preset > 0);

  size_t memory_usage = sizeof(tor_zstd_compress_state_t);

  // The Zstandard library provides a number of functions that would be useful
  // here, but they are, unfortunately, still considered experimental and are
  // thus only available in libzstd if we link against the library statically.
  //
  // The code in this function tries to approximate the calculations without
  // being able to use the following:
  //
  // - We do not have access to neither the internal members of ZSTD_CStream
  //   and ZSTD_DStream and their internal context objects.
  //
  // - We cannot use ZSTD_sizeof_CStream() and ZSTD_sizeof_DStream() since they
  //   are unexposed.
  //
  // In the future it might be useful to check if libzstd have started
  // providing these functions in a stable manner and simplify this function.
  if (compress) {
    // We try to approximate the ZSTD_sizeof_CStream(ZSTD_CStream *stream)
    // function here. This function uses the following fields to make its
    // estimate:

    // - sizeof(ZSTD_CStream): Around 192 bytes on a 64-bit machine:
    memory_usage += 192;

    // - ZSTD_sizeof_CCtx(stream->cctx): This function requires access to
    // variables that are not exposed via the public API. We use a _very_
    // simplified function to calculate the estimated amount of bytes used in
    // this struct.
    // memory_usage += (preset - 0.5) * 1024 * 1024;
    memory_usage += (preset * 1024 * 1024) - (512 * 1024);
    // - ZSTD_sizeof_CDict(stream->cdictLocal): Unused in Tor: 0 bytes.
    // - stream->outBuffSize: 128 KB:
    memory_usage += 128 * 1024;
    // - stream->inBuffSize: 2048 KB:
    memory_usage += 2048 * 1024;
  } else {
    // We try to approximate the ZSTD_sizeof_DStream(ZSTD_DStream *stream)
    // function here. This function uses the following fields to make its
    // estimate:

    // - sizeof(ZSTD_DStream): Around 208 bytes on a 64-bit machine:
    memory_usage += 208;
    // - ZSTD_sizeof_DCtx(stream->dctx): Around 150 KB.
    memory_usage += 150 * 1024;

    // - ZSTD_sizeof_DDict(stream->ddictLocal): Unused in Tor: 0 bytes.
    // - stream->inBuffSize: 0 KB.
    // - stream->outBuffSize: 0 KB.
  }

  return memory_usage;
}

/** Return an approximate number of bytes stored in memory to hold the
 * Zstandard compression/decompression state. */
static size_t
tor_zstd_state_size_precalc(int compress, int preset)
{
#ifdef ZSTD_STATIC_LINKING_ONLY
  if (tor_zstd_can_use_static_apis()) {
    if (compress) {
#ifdef HAVE_ZSTD_ESTIMATECSTREAMSIZE
      return ZSTD_estimateCStreamSize(preset);
#endif
    } else {
#ifdef HAVE_ZSTD_ESTIMATEDCTXSIZE
      /* Could use DStream, but that takes a windowSize. */
      return ZSTD_estimateDCtxSize();
#endif
    }
  }
#endif /* defined(ZSTD_STATIC_LINKING_ONLY) */
  return tor_zstd_state_size_precalc_fake(compress, preset);
}
#endif /* defined(HAVE_ZSTD) */

/** Construct and return a tor_zstd_compress_state_t object using
 * <b>method</b>. If <b>compress</b>, it's for compression; otherwise it's for
 * decompression. */
tor_zstd_compress_state_t *
tor_zstd_compress_new(int compress,
                      compress_method_t method,
                      compression_level_t level)
{
  tor_assert(method == ZSTD_METHOD);

#ifdef HAVE_ZSTD
  const int preset = memory_level(level);
  tor_zstd_compress_state_t *result;
  size_t retval;

  result = tor_malloc_zero(sizeof(tor_zstd_compress_state_t));
  result->compress = compress;
  result->allocation = tor_zstd_state_size_precalc(compress, preset);

  if (compress) {
    result->u.compress_stream = ZSTD_createCStream();

    if (result->u.compress_stream == NULL) {
      // LCOV_EXCL_START
      log_warn(LD_GENERAL, "Error while creating Zstandard compression "
               "stream");
      goto err;
      // LCOV_EXCL_STOP
    }

    retval = ZSTD_initCStream(result->u.compress_stream, preset);

    if (ZSTD_isError(retval)) {
      // LCOV_EXCL_START
      log_warn(LD_GENERAL, "Zstandard stream initialization error: %s",
               ZSTD_getErrorName(retval));
      goto err;
      // LCOV_EXCL_STOP
    }
  } else {
    result->u.decompress_stream = ZSTD_createDStream();

    if (result->u.decompress_stream == NULL) {
      // LCOV_EXCL_START
      log_warn(LD_GENERAL, "Error while creating Zstandard decompression "
               "stream");
      goto err;
      // LCOV_EXCL_STOP
    }

    retval = ZSTD_initDStream(result->u.decompress_stream);

    if (ZSTD_isError(retval)) {
      // LCOV_EXCL_START
      log_warn(LD_GENERAL, "Zstandard stream initialization error: %s",
               ZSTD_getErrorName(retval));
      goto err;
      // LCOV_EXCL_STOP
    }
  }

  atomic_counter_add(&total_zstd_allocation, result->allocation);
  return result;

 err:
  // LCOV_EXCL_START
  if (compress) {
    ZSTD_freeCStream(result->u.compress_stream);
  } else {
    ZSTD_freeDStream(result->u.decompress_stream);
  }

  tor_free(result);
  return NULL;
  // LCOV_EXCL_STOP
#else /* !defined(HAVE_ZSTD) */
  (void)compress;
  (void)method;
  (void)level;

  return NULL;
#endif /* defined(HAVE_ZSTD) */
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
tor_zstd_compress_process(tor_zstd_compress_state_t *state,
                          char **out, size_t *out_len,
                          const char **in, size_t *in_len,
                          int finish)
{
#ifdef HAVE_ZSTD
  size_t retval;

  tor_assert(state != NULL);
  tor_assert(*in_len <= UINT_MAX);
  tor_assert(*out_len <= UINT_MAX);

  ZSTD_inBuffer input = { *in, *in_len, 0 };
  ZSTD_outBuffer output = { *out, *out_len, 0 };

  if (BUG(finish == 0 && state->have_called_end)) {
    finish = 1;
  }

  if (state->compress) {
    if (! state->have_called_end)
      retval = ZSTD_compressStream(state->u.compress_stream,
                                   &output, &input);
    else
      retval = 0;
  } else {
    retval = ZSTD_decompressStream(state->u.decompress_stream,
                                   &output, &input);
  }

  if (ZSTD_isError(retval)) {
    log_warn(LD_GENERAL, "Zstandard %s didn't finish: %s.",
             state->compress ? "compression" : "decompression",
             ZSTD_getErrorName(retval));
    return TOR_COMPRESS_ERROR;
  }

  state->input_so_far += input.pos;
  state->output_so_far += output.pos;

  *out = (char *)output.dst + output.pos;
  *out_len = output.size - output.pos;
  *in = (char *)input.src + input.pos;
  *in_len = input.size - input.pos;

  if (! state->compress &&
      tor_compress_is_compression_bomb(state->input_so_far,
                                       state->output_so_far)) {
    log_warn(LD_DIR, "Possible compression bomb; abandoning stream.");
    return TOR_COMPRESS_ERROR;
  }

  if (state->compress && !state->have_called_end) {
    retval = ZSTD_flushStream(state->u.compress_stream, &output);

    *out = (char *)output.dst + output.pos;
    *out_len = output.size - output.pos;

    if (ZSTD_isError(retval)) {
      log_warn(LD_GENERAL, "Zstandard compression unable to flush: %s.",
               ZSTD_getErrorName(retval));
      return TOR_COMPRESS_ERROR;
    }

    // ZSTD_flushStream returns 0 if the frame is done, or >0 if it
    // is incomplete.
    if (retval > 0) {
      return TOR_COMPRESS_BUFFER_FULL;
    }
  }

  if (!finish) {
    // The caller says we're not done with the input, so no need to write an
    // epilogue.
    return TOR_COMPRESS_OK;
  } else if (state->compress) {
    if (*in_len) {
      // We say that we're not done with the input, so we can't write an
      // epilogue.
      return TOR_COMPRESS_OK;
    }

    retval = ZSTD_endStream(state->u.compress_stream, &output);
    state->have_called_end = 1;
    *out = (char *)output.dst + output.pos;
    *out_len = output.size - output.pos;

    if (ZSTD_isError(retval)) {
      log_warn(LD_GENERAL, "Zstandard compression unable to write "
               "epilogue: %s.",
               ZSTD_getErrorName(retval));
      return TOR_COMPRESS_ERROR;
    }

    // endStream returns the number of bytes that is needed to write the
    // epilogue.
    if (retval > 0)
      return TOR_COMPRESS_BUFFER_FULL;

    return TOR_COMPRESS_DONE;
  } else /* if (!state->compress) */ {
    // ZSTD_decompressStream returns 0 if the frame is done, or >0 if it
    // is incomplete.
    // We check this above.
    tor_assert_nonfatal(!ZSTD_isError(retval));
    // Start a new frame if this frame is done
    if (retval == 0)
      return TOR_COMPRESS_DONE;
    // Don't check out_len, it might have some space left if the next output
    // chunk is larger than the remaining space
    else if (*in_len > 0)
      return  TOR_COMPRESS_BUFFER_FULL;
    else
      return TOR_COMPRESS_OK;
  }

#else /* !defined(HAVE_ZSTD) */
  (void)state;
  (void)out;
  (void)out_len;
  (void)in;
  (void)in_len;
  (void)finish;

  return TOR_COMPRESS_ERROR;
#endif /* defined(HAVE_ZSTD) */
}

/** Deallocate <b>state</b>. */
void
tor_zstd_compress_free_(tor_zstd_compress_state_t *state)
{
  if (state == NULL)
    return;

  atomic_counter_sub(&total_zstd_allocation, state->allocation);

#ifdef HAVE_ZSTD
  if (state->compress) {
    ZSTD_freeCStream(state->u.compress_stream);
  } else {
    ZSTD_freeDStream(state->u.decompress_stream);
  }
#endif /* defined(HAVE_ZSTD) */

  tor_free(state);
}

/** Return the approximate number of bytes allocated for <b>state</b>. */
size_t
tor_zstd_compress_state_size(const tor_zstd_compress_state_t *state)
{
  tor_assert(state != NULL);
  return state->allocation;
}

/** Return the approximate number of bytes allocated for all Zstandard
 * states. */
size_t
tor_zstd_get_total_allocation(void)
{
  return atomic_counter_get(&total_zstd_allocation);
}

/** Initialize the zstd module */
void
tor_zstd_init(void)
{
  atomic_counter_init(&total_zstd_allocation);
}

/** Warn if the header and library versions don't match. */
void
tor_zstd_warn_if_version_mismatched(void)
{
#if defined(HAVE_ZSTD) && defined(ENABLE_ZSTD_ADVANCED_APIS)
  if (! tor_zstd_can_use_static_apis()) {
    char header_version[VERSION_STR_MAX_LEN];
    char runtime_version[VERSION_STR_MAX_LEN];
    tor_zstd_format_version(header_version, sizeof(header_version),
                            ZSTD_VERSION_NUMBER);
    tor_zstd_format_version(runtime_version, sizeof(runtime_version),
                            ZSTD_versionNumber());

    log_info(LD_GENERAL,
             "Tor was compiled with zstd %s, but is running with zstd %s. "
             "For ABI compatibility reasons, we'll avoid using advanced zstd "
             "functionality.",
             header_version, runtime_version);
  }
#endif /* defined(HAVE_ZSTD) && defined(ENABLE_ZSTD_ADVANCED_APIS) */
}

#ifdef TOR_UNIT_TESTS
/** Testing only: disable usage of static-only APIs, so we can make sure that
 * we still work without them. */
void
tor_zstd_set_static_apis_disabled_for_testing(int disabled)
{
  static_apis_disable_for_testing = disabled;
}
#endif /* defined(TOR_UNIT_TESTS) */
