/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dsigs_parse.h
 * \brief Code to parse and validate detached-signature objects
 **/

#include "core/or/or.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/dirparse/unparseable.h"
#include "feature/nodelist/networkstatus.h"
#include "lib/memarea/memarea.h"

#include "feature/dirauth/dsigs_parse.h"
#include "feature/dirauth/ns_detached_signatures_st.h"
#include "feature/nodelist/document_signature_st.h"

/** List of tokens recognized in detached networkstatus signature documents. */
static token_rule_t networkstatus_detached_signature_token_table[] = {
  T1_START("consensus-digest", K_CONSENSUS_DIGEST, GE(1),       NO_OBJ ),
  T("additional-digest",       K_ADDITIONAL_DIGEST,GE(3),       NO_OBJ ),
  T1("valid-after",            K_VALID_AFTER,      CONCAT_ARGS, NO_OBJ ),
  T1("fresh-until",            K_FRESH_UNTIL,      CONCAT_ARGS, NO_OBJ ),
  T1("valid-until",            K_VALID_UNTIL,      CONCAT_ARGS, NO_OBJ ),
  T("additional-signature",  K_ADDITIONAL_SIGNATURE, GE(4),   NEED_OBJ ),
  T1N("directory-signature", K_DIRECTORY_SIGNATURE,  GE(2),   NEED_OBJ ),
  END_OF_TABLE
};

/** Return the common_digests_t that holds the digests of the
 * <b>flavor_name</b>-flavored networkstatus according to the detached
 * signatures document <b>sigs</b>, allocating a new common_digests_t as
 * needed. */
static common_digests_t *
detached_get_digests(ns_detached_signatures_t *sigs, const char *flavor_name)
{
  common_digests_t *d = strmap_get(sigs->digests, flavor_name);
  if (!d) {
    d = tor_malloc_zero(sizeof(common_digests_t));
    strmap_set(sigs->digests, flavor_name, d);
  }
  return d;
}

/** Return the list of signatures of the <b>flavor_name</b>-flavored
 * networkstatus according to the detached signatures document <b>sigs</b>,
 * allocating a new common_digests_t as needed. */
static smartlist_t *
detached_get_signatures(ns_detached_signatures_t *sigs,
                        const char *flavor_name)
{
  smartlist_t *sl = strmap_get(sigs->signatures, flavor_name);
  if (!sl) {
    sl = smartlist_new();
    strmap_set(sigs->signatures, flavor_name, sl);
  }
  return sl;
}

/** Parse a detached v3 networkstatus signature document between <b>s</b> and
 * <b>eos</b> and return the result.  Return -1 on failure. */
ns_detached_signatures_t *
networkstatus_parse_detached_signatures(const char *s, const char *eos)
{
  /* XXXX there is too much duplicate shared between this function and
   * networkstatus_parse_vote_from_string(). */
  directory_token_t *tok;
  memarea_t *area = NULL;
  common_digests_t *digests;

  smartlist_t *tokens = smartlist_new();
  ns_detached_signatures_t *sigs =
    tor_malloc_zero(sizeof(ns_detached_signatures_t));
  sigs->digests = strmap_new();
  sigs->signatures = strmap_new();

  if (!eos)
    eos = s + strlen(s);

  area = memarea_new();
  if (tokenize_string(area,s, eos, tokens,
                      networkstatus_detached_signature_token_table, 0)) {
    log_warn(LD_DIR, "Error tokenizing detached networkstatus signatures");
    goto err;
  }

  /* Grab all the digest-like tokens. */
  SMARTLIST_FOREACH_BEGIN(tokens, directory_token_t *, _tok) {
    const char *algname;
    digest_algorithm_t alg;
    const char *flavor;
    const char *hexdigest;
    size_t expected_length, digest_length;

    tok = _tok;

    if (tok->tp == K_CONSENSUS_DIGEST) {
      algname = "sha1";
      alg = DIGEST_SHA1;
      flavor = "ns";
      hexdigest = tok->args[0];
    } else if (tok->tp == K_ADDITIONAL_DIGEST) {
      int a = crypto_digest_algorithm_parse_name(tok->args[1]);
      if (a<0) {
        log_warn(LD_DIR, "Unrecognized algorithm name %s", tok->args[0]);
        continue;
      }
      alg = (digest_algorithm_t) a;
      flavor = tok->args[0];
      algname = tok->args[1];
      hexdigest = tok->args[2];
    } else {
      continue;
    }

    digest_length = crypto_digest_algorithm_get_length(alg);
    expected_length = digest_length * 2; /* hex encoding */

    if (strlen(hexdigest) != expected_length) {
      log_warn(LD_DIR, "Wrong length on consensus-digest in detached "
               "networkstatus signatures");
      goto err;
    }
    digests = detached_get_digests(sigs, flavor);
    tor_assert(digests);
    if (!fast_mem_is_zero(digests->d[alg], digest_length)) {
      log_warn(LD_DIR, "Multiple digests for %s with %s on detached "
               "signatures document", flavor, algname);
      continue;
    }
    if (base16_decode(digests->d[alg], digest_length,
                      hexdigest, strlen(hexdigest)) != (int) digest_length) {
      log_warn(LD_DIR, "Bad encoding on consensus-digest in detached "
               "networkstatus signatures");
      goto err;
    }
  } SMARTLIST_FOREACH_END(_tok);

  tok = find_by_keyword(tokens, K_VALID_AFTER);
  if (parse_iso_time(tok->args[0], &sigs->valid_after)) {
    log_warn(LD_DIR, "Bad valid-after in detached networkstatus signatures");
    goto err;
  }

  tok = find_by_keyword(tokens, K_FRESH_UNTIL);
  if (parse_iso_time(tok->args[0], &sigs->fresh_until)) {
    log_warn(LD_DIR, "Bad fresh-until in detached networkstatus signatures");
    goto err;
  }

  tok = find_by_keyword(tokens, K_VALID_UNTIL);
  if (parse_iso_time(tok->args[0], &sigs->valid_until)) {
    log_warn(LD_DIR, "Bad valid-until in detached networkstatus signatures");
    goto err;
  }

  SMARTLIST_FOREACH_BEGIN(tokens, directory_token_t *, _tok) {
    const char *id_hexdigest;
    const char *sk_hexdigest;
    const char *algname;
    const char *flavor;
    digest_algorithm_t alg;

    char id_digest[DIGEST_LEN];
    char sk_digest[DIGEST_LEN];
    smartlist_t *siglist;
    document_signature_t *sig;
    int is_duplicate;

    tok = _tok;
    if (tok->tp == K_DIRECTORY_SIGNATURE) {
      tor_assert(tok->n_args >= 2);
      flavor = "ns";
      algname = "sha1";
      id_hexdigest = tok->args[0];
      sk_hexdigest = tok->args[1];
    } else if (tok->tp == K_ADDITIONAL_SIGNATURE) {
      tor_assert(tok->n_args >= 4);
      flavor = tok->args[0];
      algname = tok->args[1];
      id_hexdigest = tok->args[2];
      sk_hexdigest = tok->args[3];
    } else {
      continue;
    }

    {
      int a = crypto_digest_algorithm_parse_name(algname);
      if (a<0) {
        log_warn(LD_DIR, "Unrecognized algorithm name %s", algname);
        continue;
      }
      alg = (digest_algorithm_t) a;
    }

    if (!tok->object_type ||
        strcmp(tok->object_type, "SIGNATURE") ||
        tok->object_size < 128 || tok->object_size > 512) {
      log_warn(LD_DIR, "Bad object type or length on directory-signature");
      goto err;
    }

    if (strlen(id_hexdigest) != HEX_DIGEST_LEN ||
        base16_decode(id_digest, sizeof(id_digest),
                      id_hexdigest, HEX_DIGEST_LEN) != sizeof(id_digest)) {
      log_warn(LD_DIR, "Error decoding declared identity %s in "
               "network-status vote.", escaped(id_hexdigest));
      goto err;
    }
    if (strlen(sk_hexdigest) != HEX_DIGEST_LEN ||
        base16_decode(sk_digest, sizeof(sk_digest),
                      sk_hexdigest, HEX_DIGEST_LEN) != sizeof(sk_digest)) {
      log_warn(LD_DIR, "Error decoding declared signing key digest %s in "
               "network-status vote.", escaped(sk_hexdigest));
      goto err;
    }

    siglist = detached_get_signatures(sigs, flavor);
    is_duplicate = 0;
    SMARTLIST_FOREACH(siglist, document_signature_t *, dsig, {
      if (dsig->alg == alg &&
          tor_memeq(id_digest, dsig->identity_digest, DIGEST_LEN) &&
          tor_memeq(sk_digest, dsig->signing_key_digest, DIGEST_LEN)) {
        is_duplicate = 1;
      }
    });
    if (is_duplicate) {
      log_warn(LD_DIR, "Two signatures with identical keys and algorithm "
               "found.");
      continue;
    }

    sig = tor_malloc_zero(sizeof(document_signature_t));
    sig->alg = alg;
    memcpy(sig->identity_digest, id_digest, DIGEST_LEN);
    memcpy(sig->signing_key_digest, sk_digest, DIGEST_LEN);
    if (tok->object_size >= INT_MAX || tok->object_size >= SIZE_T_CEILING) {
      tor_free(sig);
      goto err;
    }
    sig->signature = tor_memdup(tok->object_body, tok->object_size);
    sig->signature_len = (int) tok->object_size;

    smartlist_add(siglist, sig);
  } SMARTLIST_FOREACH_END(_tok);

  goto done;
 err:
  ns_detached_signatures_free(sigs);
  sigs = NULL;
 done:
  SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
  smartlist_free(tokens);
  if (area) {
    DUMP_AREA(area, "detached signatures");
    memarea_drop_all(area);
  }
  return sigs;
}

/** Release all storage held in <b>s</b>. */
void
ns_detached_signatures_free_(ns_detached_signatures_t *s)
{
  if (!s)
    return;
  if (s->signatures) {
    STRMAP_FOREACH(s->signatures, flavor, smartlist_t *, sigs) {
      SMARTLIST_FOREACH(sigs, document_signature_t *, sig,
                        document_signature_free(sig));
      smartlist_free(sigs);
    } STRMAP_FOREACH_END;
    strmap_free(s->signatures, NULL);
    strmap_free(s->digests, tor_free_);
  }

  tor_free(s);
}
