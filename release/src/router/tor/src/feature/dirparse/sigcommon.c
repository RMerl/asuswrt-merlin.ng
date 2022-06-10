/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file sigcommon.c
 * \brief Shared hashing, signing, and signature-checking code for directory
 *    objects.
 **/

#define SIGCOMMON_PRIVATE

#include "core/or/or.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/dirparse/sigcommon.h"

/** Helper function for <b>router_get_hash_impl</b>: given <b>s</b>,
 * <b>s_len</b>, <b>start_str</b>, <b>end_str</b>, and <b>end_c</b> with the
 * same semantics as in that function, set *<b>start_out</b> (inclusive) and
 * *<b>end_out</b> (exclusive) to the boundaries of the string to be hashed.
 *
 * Return 0 on success and -1 on failure.
 */
int
router_get_hash_impl_helper(const char *s, size_t s_len,
                            const char *start_str,
                            const char *end_str, char end_c,
                            int log_severity,
                            const char **start_out, const char **end_out)
{
  const char *start, *end;
  start = tor_memstr(s, s_len, start_str);
  if (!start) {
    log_fn(log_severity,LD_DIR,
           "couldn't find start of hashed material \"%s\"",start_str);
    return -1;
  }
  if (start != s && *(start-1) != '\n') {
    log_fn(log_severity,LD_DIR,
             "first occurrence of \"%s\" is not at the start of a line",
             start_str);
    return -1;
  }
  end = tor_memstr(start+strlen(start_str),
                   s_len - (start-s) - strlen(start_str), end_str);
  if (!end) {
    log_fn(log_severity,LD_DIR,
           "couldn't find end of hashed material \"%s\"",end_str);
    return -1;
  }
  end = memchr(end+strlen(end_str), end_c, s_len - (end-s) - strlen(end_str));
  if (!end) {
    log_fn(log_severity,LD_DIR,
           "couldn't find EOL");
    return -1;
  }
  ++end;

  *start_out = start;
  *end_out = end;
  return 0;
}

/** Compute the digest of the substring of <b>s</b> taken from the first
 * occurrence of <b>start_str</b> through the first instance of c after the
 * first subsequent occurrence of <b>end_str</b>; store the 20-byte or 32-byte
 * result in <b>digest</b>; return 0 on success.
 *
 * If no such substring exists, return -1.
 */
int
router_get_hash_impl(const char *s, size_t s_len, char *digest,
                     const char *start_str,
                     const char *end_str, char end_c,
                     digest_algorithm_t alg)
{
  const char *start=NULL, *end=NULL;
  if (router_get_hash_impl_helper(s,s_len,start_str,end_str,end_c,LOG_WARN,
                                  &start,&end)<0)
    return -1;

  return router_compute_hash_final(digest, start, end-start, alg);
}

/** Compute the digest of the <b>len</b>-byte directory object at
 * <b>start</b>, using <b>alg</b>. Store the result in <b>digest</b>, which
 * must be long enough to hold it. */
MOCK_IMPL(STATIC int,
router_compute_hash_final,(char *digest,
                           const char *start, size_t len,
                           digest_algorithm_t alg))
{
  if (alg == DIGEST_SHA1) {
    if (crypto_digest(digest, start, len) < 0) {
      log_warn(LD_BUG,"couldn't compute digest");
      return -1;
    }
  } else {
    if (crypto_digest256(digest, start, len, alg) < 0) {
      log_warn(LD_BUG,"couldn't compute digest");
      return -1;
    }
  }

  return 0;
}

/** As router_get_hash_impl, but compute all hashes. */
int
router_get_hashes_impl(const char *s, size_t s_len, common_digests_t *digests,
                       const char *start_str,
                       const char *end_str, char end_c)
{
  const char *start=NULL, *end=NULL;
  if (router_get_hash_impl_helper(s,s_len,start_str,end_str,end_c,LOG_WARN,
                                  &start,&end)<0)
    return -1;

  if (crypto_common_digests(digests, start, end-start)) {
    log_warn(LD_BUG,"couldn't compute digests");
    return -1;
  }

  return 0;
}

MOCK_IMPL(STATIC int,
signed_digest_equals, (const uint8_t *d1, const uint8_t *d2, size_t len))
{
  return tor_memeq(d1, d2, len);
}

/** Check whether the object body of the token in <b>tok</b> has a good
 * signature for <b>digest</b> using key <b>pkey</b>.
 * If <b>CST_NO_CHECK_OBJTYPE</b> is set, do not check
 * the object type of the signature object. Use <b>doctype</b> as the type of
 * the document when generating log messages.  Return 0 on success, negative
 * on failure.
 */
MOCK_IMPL(int,
check_signature_token,(const char *digest,
                      ssize_t digest_len,
                      directory_token_t *tok,
                      crypto_pk_t *pkey,
                      int flags,
                      const char *doctype))
{
  char *signed_digest;
  size_t keysize;
  const int check_objtype = ! (flags & CST_NO_CHECK_OBJTYPE);

  tor_assert(pkey);
  tor_assert(tok);
  tor_assert(digest);
  tor_assert(doctype);

  if (check_objtype) {
    if (strcmp(tok->object_type, "SIGNATURE")) {
      log_warn(LD_DIR, "Bad object type on %s signature", doctype);
      return -1;
    }
  }

  keysize = crypto_pk_keysize(pkey);
  signed_digest = tor_malloc(keysize);
  if (crypto_pk_public_checksig(pkey, signed_digest, keysize,
                                tok->object_body, tok->object_size)
      < digest_len) {
    log_warn(LD_DIR, "Error reading %s: invalid signature.", doctype);
    tor_free(signed_digest);
    return -1;
  }
  //  log_debug(LD_DIR,"Signed %s hash starts %s", doctype,
  //            hex_str(signed_digest,4));
  if (! signed_digest_equals((const uint8_t *)digest,
                             (const uint8_t *)signed_digest, digest_len)) {
    log_warn(LD_DIR, "Error reading %s: signature does not match.", doctype);
    tor_free(signed_digest);
    return -1;
  }
  tor_free(signed_digest);
  return 0;
}
