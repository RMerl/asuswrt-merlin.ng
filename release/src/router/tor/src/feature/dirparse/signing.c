/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file signing.c
 * \brief Code to sign directory objects.
 **/

#include "core/or/or.h"
#include "feature/dirparse/signing.h"

/** Helper: used to generate signatures for routers, directories and
 * network-status objects.  Given a <b>digest_len</b>-byte digest in
 * <b>digest</b> and a secret <b>private_key</b>, generate an PKCS1-padded
 * signature, BASE64-encode it, surround it with -----BEGIN/END----- pairs,
 * and return the new signature on success or NULL on failure.
 */
char *
router_get_dirobj_signature(const char *digest,
                            size_t digest_len,
                            const crypto_pk_t *private_key)
{
  char *signature;
  size_t i, keysize;
  int siglen;
  char *buf = NULL;
  size_t buf_len;
  /* overestimate of BEGIN/END lines total len. */
#define BEGIN_END_OVERHEAD_LEN 64

  keysize = crypto_pk_keysize(private_key);
  signature = tor_malloc(keysize);
  siglen = crypto_pk_private_sign(private_key, signature, keysize,
                                  digest, digest_len);
  if (siglen < 0) {
    log_warn(LD_BUG,"Couldn't sign digest.");
    goto err;
  }

  /* The *2 here is a ridiculous overestimate of base-64 overhead. */
  buf_len = (siglen * 2) + BEGIN_END_OVERHEAD_LEN;
  buf = tor_malloc(buf_len);

  if (strlcpy(buf, "-----BEGIN SIGNATURE-----\n", buf_len) >= buf_len)
    goto truncated;

  i = strlen(buf);
  if (base64_encode(buf+i, buf_len-i, signature, siglen,
                    BASE64_ENCODE_MULTILINE) < 0) {
    log_warn(LD_BUG,"couldn't base64-encode signature");
    goto err;
  }

  if (strlcat(buf, "-----END SIGNATURE-----\n", buf_len) >= buf_len)
    goto truncated;

  tor_free(signature);
  return buf;

 truncated:
  log_warn(LD_BUG,"tried to exceed string length.");
 err:
  tor_free(signature);
  tor_free(buf);
  return NULL;
}

/** Helper: used to generate signatures for routers, directories and
 * network-status objects.  Given a digest in <b>digest</b> and a secret
 * <b>private_key</b>, generate a PKCS1-padded signature, BASE64-encode it,
 * surround it with -----BEGIN/END----- pairs, and write it to the
 * <b>buf_len</b>-byte buffer at <b>buf</b>.  Return 0 on success, -1 on
 * failure.
 */
int
router_append_dirobj_signature(char *buf, size_t buf_len, const char *digest,
                               size_t digest_len, crypto_pk_t *private_key)
{
  size_t sig_len, s_len;
  char *sig = router_get_dirobj_signature(digest, digest_len, private_key);
  if (!sig) {
    log_warn(LD_BUG, "No signature generated");
    return -1;
  }
  sig_len = strlen(sig);
  s_len = strlen(buf);
  if (sig_len + s_len + 1 > buf_len) {
    log_warn(LD_BUG, "Not enough room for signature");
    tor_free(sig);
    return -1;
  }
  memcpy(buf+s_len, sig, sig_len+1);
  tor_free(sig);
  return 0;
}
