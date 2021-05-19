/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file pem.c
 *
 * \brief Implement a trivial version of PEM encoding, for use with NSS.
 *
 * We deliberately do not support any encryption here.
 **/

#include "orconfig.h"

#include "lib/encoding/pem.h"

#include "lib/ctime/di_ops.h"
#include "lib/encoding/binascii.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"
#include "lib/string/util_string.h"

#include <string.h>

/**
 * Return the length of a <b>src_len</b>-byte object when tagged with
 * <b>objtype</b> and PEM-encoded.  Includes terminating NUL.
 */
size_t
pem_encoded_size(size_t src_len, const char *objtype)
{
  return
    strlen("-----BEGIN -----\n") +
    strlen("-----END -----\n") +
    strlen(objtype) * 2 +
    base64_encode_size(src_len, BASE64_ENCODE_MULTILINE)
    + 1;
}

/**
 * PEM-encode the <b>srclen</b>-byte object at <b>src</b> into the
 * <b>destlen</b>-byte buffer at <b>dest</b>, tagging it with <b>objtype</b>.
 * Return 0 on success and -1 on failure.
 */
int
pem_encode(char *dest, size_t destlen, const uint8_t *src, size_t srclen,
           const char *objtype)
{
  if (tor_snprintf(dest, destlen, "-----BEGIN %s-----\n", objtype) < 0)
    return -1;

  size_t offset = strlen(dest);

  int n = base64_encode(dest + offset, destlen - offset,
                        (const char *)src, srclen, BASE64_ENCODE_MULTILINE);
  if (n < 0)
    return -1;
  offset += n;
  if (BUG(offset > destlen))
    return -1;

  if (tor_snprintf(dest + offset, destlen - offset,
                   "-----END %s-----\n", objtype) < 0)
    return -1;

  tor_assert(strlen(dest) + 1 <= pem_encoded_size(srclen, objtype));
  return 0;
}

/**
 * Given a PEM-encoded block of size <b>srclen</b> in <b>src</b>, if it has
 * object type <b>objtype</b>, decode it into the <b>destlen</b>-byte buffer
 * at <b>dest</b>.  Return the number of characters decoded on success, or -1
 * on failure.
 */
int
pem_decode(uint8_t *dest, size_t destlen, const char *src, size_t srclen,
           const char *objtype)
{
  const char *eos = src + srclen;

  src = eat_whitespace_eos(src, eos);

  char *tag = NULL;
  tor_asprintf(&tag, "-----BEGIN %s-----", objtype);
  if ((size_t)(eos-src) < strlen(tag) || fast_memneq(src, tag, strlen(tag))) {
    tor_free(tag);
    return -1;
  }
  src += strlen(tag);
  tor_free(tag);
  /* At this point we insist on spaces (including CR), then an LF. */
  src = eat_whitespace_eos_no_nl(src, eos);
  if (src == eos || *src != '\n') {
    /* Extra junk at end of line: this isn't valid. */
    return -1;
  }

  // NOTE lack of trailing \n.  We do not enforce its presence.
  tor_asprintf(&tag, "\n-----END %s-----", objtype);
  const char *end_of_base64 = tor_memstr(src, eos-src, tag);
  tor_free(tag);
  if (end_of_base64 == NULL)
    return -1;

  /* Should we actually allow extra stuff at the end? */

  return base64_decode((char*)dest, destlen, src, end_of_base64-src);
}
