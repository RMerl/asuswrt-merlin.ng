/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file authcert_parse.c
 * @brief Authority certificate parsing.
 **/

#include "core/or/or.h"
#include "feature/dirparse/authcert_parse.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/dirparse/sigcommon.h"
#include "feature/dirparse/unparseable.h"
#include "feature/nodelist/authcert.h"
#include "lib/memarea/memarea.h"

#include "feature/nodelist/authority_cert_st.h"
#include "feature/dirparse/authcert_members.h"

/** List of tokens recognized in V3 authority certificates. */
// clang-format off
static token_rule_t dir_key_certificate_table[] = {
  AUTHCERT_MEMBERS,
  T1("fingerprint",      K_FINGERPRINT,              CONCAT_ARGS, NO_OBJ ),
  END_OF_TABLE
};
// clang-format on

/** Parse a key certificate from <b>s</b>; point <b>end-of-string</b> to
 * the first character after the certificate. */
authority_cert_t *
authority_cert_parse_from_string(const char *s, size_t maxlen,
                                 const char **end_of_string)
{
  /** Reject any certificate at least this big; it is probably an overflow, an
   * attack, a bug, or some other nonsense. */
#define MAX_CERT_SIZE (128*1024)

  authority_cert_t *cert = NULL, *old_cert;
  smartlist_t *tokens = NULL;
  char digest[DIGEST_LEN];
  directory_token_t *tok;
  char fp_declared[DIGEST_LEN];
  const char *eos;
  size_t len;
  int found;
  memarea_t *area = NULL;
  const char *end_of_s = s + maxlen;
  const char *s_dup = s;

  s = eat_whitespace_eos(s, end_of_s);
  eos = tor_memstr(s, end_of_s - s, "\ndir-key-certification");
  if (! eos) {
    log_warn(LD_DIR, "No signature found on key certificate");
    return NULL;
  }
  eos = tor_memstr(eos, end_of_s - eos, "\n-----END SIGNATURE-----\n");
  if (! eos) {
    log_warn(LD_DIR, "No end-of-signature found on key certificate");
    return NULL;
  }
  eos = memchr(eos+2, '\n', end_of_s - (eos+2));
  tor_assert(eos);
  ++eos;
  len = eos - s;

  if (len > MAX_CERT_SIZE) {
    log_warn(LD_DIR, "Certificate is far too big (at %lu bytes long); "
             "rejecting", (unsigned long)len);
    return NULL;
  }

  tokens = smartlist_new();
  area = memarea_new();
  if (tokenize_string(area,s, eos, tokens, dir_key_certificate_table, 0) < 0) {
    log_warn(LD_DIR, "Error tokenizing key certificate");
    goto err;
  }
  if (router_get_hash_impl(s, eos - s, digest, "dir-key-certificate-version",
                           "\ndir-key-certification", '\n', DIGEST_SHA1) < 0)
    goto err;
  tok = smartlist_get(tokens, 0);
  if (tok->tp != K_DIR_KEY_CERTIFICATE_VERSION || strcmp(tok->args[0], "3")) {
    log_warn(LD_DIR,
             "Key certificate does not begin with a recognized version (3).");
    goto err;
  }

  cert = tor_malloc_zero(sizeof(authority_cert_t));
  memcpy(cert->cache_info.signed_descriptor_digest, digest, DIGEST_LEN);

  tok = find_by_keyword(tokens, K_DIR_SIGNING_KEY);
  tor_assert(tok->key);
  cert->signing_key = tok->key;
  tok->key = NULL;
  if (crypto_pk_get_digest(cert->signing_key, cert->signing_key_digest))
    goto err;

  tok = find_by_keyword(tokens, K_DIR_IDENTITY_KEY);
  tor_assert(tok->key);
  cert->identity_key = tok->key;
  tok->key = NULL;

  tok = find_by_keyword(tokens, K_FINGERPRINT);
  tor_assert(tok->n_args);
  if (base16_decode(fp_declared, DIGEST_LEN, tok->args[0],
                    strlen(tok->args[0])) != DIGEST_LEN) {
    log_warn(LD_DIR, "Couldn't decode key certificate fingerprint %s",
             escaped(tok->args[0]));
    goto err;
  }

  if (crypto_pk_get_digest(cert->identity_key,
                           cert->cache_info.identity_digest))
    goto err;

  if (tor_memneq(cert->cache_info.identity_digest, fp_declared, DIGEST_LEN)) {
    log_warn(LD_DIR, "Digest of certificate key didn't match declared "
             "fingerprint");
    goto err;
  }

  tok = find_opt_by_keyword(tokens, K_DIR_ADDRESS);
  if (tok) {
    struct in_addr in;
    char *address = NULL;
    tor_assert(tok->n_args);
    /* XXX++ use some tor_addr parse function below instead. -RD */
    if (tor_addr_port_split(LOG_WARN, tok->args[0], &address,
                            &cert->ipv4_dirport) < 0 ||
        tor_inet_aton(address, &in) == 0) {
      log_warn(LD_DIR, "Couldn't parse dir-address in certificate");
      tor_free(address);
      goto err;
    }
    tor_addr_from_in(&cert->ipv4_addr, &in);
    tor_free(address);
  }

  tok = find_by_keyword(tokens, K_DIR_KEY_PUBLISHED);
  if (parse_iso_time(tok->args[0], &cert->cache_info.published_on) < 0) {
     goto err;
  }
  tok = find_by_keyword(tokens, K_DIR_KEY_EXPIRES);
  if (parse_iso_time(tok->args[0], &cert->expires) < 0) {
     goto err;
  }

  tok = smartlist_get(tokens, smartlist_len(tokens)-1);
  if (tok->tp != K_DIR_KEY_CERTIFICATION) {
    log_warn(LD_DIR, "Certificate didn't end with dir-key-certification.");
    goto err;
  }

  /* If we already have this cert, don't bother checking the signature. */
  old_cert = authority_cert_get_by_digests(
                                     cert->cache_info.identity_digest,
                                     cert->signing_key_digest);
  found = 0;
  if (old_cert) {
    /* XXXX We could just compare signed_descriptor_digest, but that wouldn't
     * buy us much. */
    if (old_cert->cache_info.signed_descriptor_len == len &&
        old_cert->cache_info.signed_descriptor_body &&
        tor_memeq(s, old_cert->cache_info.signed_descriptor_body, len)) {
      log_debug(LD_DIR, "We already checked the signature on this "
                "certificate; no need to do so again.");
      found = 1;
    }
  }
  if (!found) {
    if (check_signature_token(digest, DIGEST_LEN, tok, cert->identity_key, 0,
                              "key certificate")) {
      goto err;
    }

    tok = find_by_keyword(tokens, K_DIR_KEY_CROSSCERT);
    if (check_signature_token(cert->cache_info.identity_digest,
                              DIGEST_LEN,
                              tok,
                              cert->signing_key,
                              CST_NO_CHECK_OBJTYPE,
                              "key cross-certification")) {
      goto err;
    }
  }

  cert->cache_info.signed_descriptor_len = len;
  cert->cache_info.signed_descriptor_body = tor_malloc(len+1);
  memcpy(cert->cache_info.signed_descriptor_body, s, len);
  cert->cache_info.signed_descriptor_body[len] = 0;
  cert->cache_info.saved_location = SAVED_NOWHERE;

  if (end_of_string) {
    *end_of_string = eat_whitespace(eos);
  }
  SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
  smartlist_free(tokens);
  if (area) {
    DUMP_AREA(area, "authority cert");
    memarea_drop_all(area);
  }
  return cert;
 err:
  dump_desc(s_dup, "authority cert");
  authority_cert_free(cert);
  SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
  smartlist_free(tokens);
  if (area) {
    DUMP_AREA(area, "authority cert");
    memarea_drop_all(area);
  }
  return NULL;
}
