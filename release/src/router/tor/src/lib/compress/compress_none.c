/* Copyright (c) 2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compress_none.c
 * \brief Compression backend for identity compression.
 *
 * We actually define this backend so that we can treat the identity transform
 * as another case of compression.
 *
 * This module should never be invoked directly. Use the compress module
 * instead.
 **/

#include "orconfig.h"

#include "lib/log/log.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_none.h"
#include "lib/intmath/cmp.h"

#include <string.h>

/** Transfer some bytes using the identity transformation.  Read up to
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
tor_cnone_compress_process(char **out, size_t *out_len,
                           const char **in, size_t *in_len,
                           int finish)
{
  size_t n_to_copy = MIN(*in_len, *out_len);

  memcpy(*out, *in, n_to_copy);
  *out += n_to_copy;
  *in += n_to_copy;
  *out_len -= n_to_copy;
  *in_len -= n_to_copy;
  if (*in_len == 0) {
    return finish ? TOR_COMPRESS_DONE : TOR_COMPRESS_OK;
  } else {
    return TOR_COMPRESS_BUFFER_FULL;
  }
}
