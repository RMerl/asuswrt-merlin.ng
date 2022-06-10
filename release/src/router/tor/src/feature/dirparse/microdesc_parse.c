/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file microdesc_parse.c
 * \brief Code to parse and validate microdescriptors.
 **/

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/or/policies.h"
#include "feature/dirparse/microdesc_parse.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/dirparse/routerparse.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/nickname.h"
#include "feature/nodelist/nodefamily.h"
#include "feature/relay/router.h"
#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/memarea/memarea.h"

#include "feature/nodelist/microdesc_st.h"

/** List of tokens recognized in microdescriptors */
// clang-format off
static token_rule_t microdesc_token_table[] = {
  T1_START("onion-key",        K_ONION_KEY,        NO_ARGS,     NEED_KEY_1024),
  T1("ntor-onion-key",         K_ONION_KEY_NTOR,   GE(1),       NO_OBJ ),
  T0N("id",                    K_ID,               GE(2),       NO_OBJ ),
  T0N("a",                     K_A,                GE(1),       NO_OBJ ),
  T01("family",                K_FAMILY,           CONCAT_ARGS, NO_OBJ ),
  T01("p",                     K_P,                CONCAT_ARGS, NO_OBJ ),
  T01("p6",                    K_P6,               CONCAT_ARGS, NO_OBJ ),
  A01("@last-listed",          A_LAST_LISTED,      CONCAT_ARGS, NO_OBJ ),
  END_OF_TABLE
};
// clang-format on

/** Assuming that s starts with a microdesc, return the start of the
 * *NEXT* one.  Return NULL on "not found." */
static const char *
find_start_of_next_microdesc(const char *s, const char *eos)
{
  int started_with_annotations;
  s = eat_whitespace_eos(s, eos);
  if (!s)
    return NULL;

#define CHECK_LENGTH() STMT_BEGIN \
    if (eos - s < 32)             \
      return NULL;                \
  STMT_END

#define NEXT_LINE() STMT_BEGIN            \
    s = memchr(s, '\n', eos-s);           \
    if (!s || eos - s <= 1)               \
      return NULL;                        \
    s++;                                  \
  STMT_END

  CHECK_LENGTH();

  started_with_annotations = (*s == '@');

  if (started_with_annotations) {
    /* Start by advancing to the first non-annotation line. */
    while (*s == '@')
      NEXT_LINE();
  }
  CHECK_LENGTH();

  /* Now we should be pointed at an onion-key line.  If we are, then skip
   * it. */
  if (!strcmpstart(s, "onion-key"))
    NEXT_LINE();

  /* Okay, now we're pointed at the first line of the microdescriptor which is
     not an annotation or onion-key.  The next line that _is_ an annotation or
     onion-key is the start of the next microdescriptor. */
  while (eos - s > 32) {
    if (*s == '@' || !strcmpstart(s, "onion-key"))
      return s;
    NEXT_LINE();
  }
  return NULL;

#undef CHECK_LENGTH
#undef NEXT_LINE
}

static inline int
policy_is_reject_star_or_null(struct short_policy_t *policy)
{
  return !policy || short_policy_is_reject_star(policy);
}

/**
 * Return a human-readable description of a given saved_location_t.
 * Never returns NULL.
 **/
static const char *
saved_location_to_string(saved_location_t where)
{
  const char *location;
  switch (where) {
    case SAVED_NOWHERE:
      location = "download or generated string";
      break;
    case SAVED_IN_CACHE:
      location = "cache";
      break;
    case SAVED_IN_JOURNAL:
      location = "journal";
      break;
    default:
      location = "unknown location";
      break;
  }
  return location;
}

/**
 * Given a microdescriptor stored in <b>where</b> which starts at <b>s</b>,
 * which ends at <b>start_of_next_microdescriptor</b>, and which is located
 * within a larger document beginning at <b>start</b>: Fill in the body,
 * bodylen, bodylen, saved_location, off, and digest fields of <b>md</b> as
 * appropriate.
 *
 * The body field will be an alias within <b>s</b> if <b>saved_location</b>
 * is SAVED_IN_CACHE, and will be copied into body and nul-terminated
 * otherwise.
 **/
static int
microdesc_extract_body(microdesc_t *md,
                       const char *start,
                       const char *s, const char *start_of_next_microdesc,
                       saved_location_t where)
{
  const bool copy_body = (where != SAVED_IN_CACHE);

  const char *cp = tor_memstr(s, start_of_next_microdesc-s, "onion-key");

  const bool no_onion_key = (cp == NULL);
  if (no_onion_key) {
    cp = s; /* So that we have *some* junk to put in the body */
  }

  md->bodylen = start_of_next_microdesc - cp;
  md->saved_location = where;
  if (copy_body)
    md->body = tor_memdup_nulterm(cp, md->bodylen);
  else
    md->body = (char*)cp;
  md->off = cp - start;

  crypto_digest256(md->digest, md->body, md->bodylen, DIGEST_SHA256);

  return no_onion_key ? -1 : 0;
}

/**
 * Parse a microdescriptor which begins at <b>s</b> and ends at
 * <b>start_of_next_microdesc</b>.  Store its fields into <b>md</b>.  Use
 * <b>where</b> for generating log information.  If <b>allow_annotations</b>
 * is true, then one or more annotations may precede the microdescriptor body
 * proper.  Use <b>area</b> for memory management, clearing it when done.
 *
 * On success, return 0; otherwise return -1.
 **/
static int
microdesc_parse_fields(microdesc_t *md,
                       memarea_t *area,
                       const char *s, const char *start_of_next_microdesc,
                       int allow_annotations,
                       saved_location_t where)
{
  smartlist_t *tokens = smartlist_new();
  int rv = -1;
  int flags = allow_annotations ? TS_ANNOTATIONS_OK : 0;
  directory_token_t *tok;

  if (tokenize_string(area, s, start_of_next_microdesc, tokens,
                      microdesc_token_table, flags)) {
    log_warn(LD_DIR, "Unparseable microdescriptor found in %s",
             saved_location_to_string(where));
    goto err;
  }

  if ((tok = find_opt_by_keyword(tokens, A_LAST_LISTED))) {
    if (parse_iso_time(tok->args[0], &md->last_listed)) {
      log_warn(LD_DIR, "Bad last-listed time in microdescriptor");
      goto err;
    }
  }

  tok = find_by_keyword(tokens, K_ONION_KEY);
  if (!crypto_pk_public_exponent_ok(tok->key)) {
    log_warn(LD_DIR,
             "Relay's onion key had invalid exponent.");
    goto err;
  }
  md->onion_pkey = tor_memdup(tok->object_body, tok->object_size);
  md->onion_pkey_len = tok->object_size;
  crypto_pk_free(tok->key);

  if ((tok = find_opt_by_keyword(tokens, K_ONION_KEY_NTOR))) {
    curve25519_public_key_t k;
    tor_assert(tok->n_args >= 1);
    if (curve25519_public_from_base64(&k, tok->args[0]) < 0) {
      log_warn(LD_DIR, "Bogus ntor-onion-key in microdesc");
      goto err;
    }
    md->onion_curve25519_pkey =
      tor_memdup(&k, sizeof(curve25519_public_key_t));
  }

  smartlist_t *id_lines = find_all_by_keyword(tokens, K_ID);
  if (id_lines) {
    SMARTLIST_FOREACH_BEGIN(id_lines, directory_token_t *, t) {
      tor_assert(t->n_args >= 2);
      if (!strcmp(t->args[0], "ed25519")) {
        if (md->ed25519_identity_pkey) {
          log_warn(LD_DIR, "Extra ed25519 key in microdesc");
          smartlist_free(id_lines);
          goto err;
        }
        ed25519_public_key_t k;
        if (ed25519_public_from_base64(&k, t->args[1])<0) {
          log_warn(LD_DIR, "Bogus ed25519 key in microdesc");
          smartlist_free(id_lines);
          goto err;
        }
        md->ed25519_identity_pkey = tor_memdup(&k, sizeof(k));
      }
    } SMARTLIST_FOREACH_END(t);
    smartlist_free(id_lines);
  }

  {
    smartlist_t *a_lines = find_all_by_keyword(tokens, K_A);
    if (a_lines) {
      find_single_ipv6_orport(a_lines, &md->ipv6_addr, &md->ipv6_orport);
      smartlist_free(a_lines);
    }
  }

  if ((tok = find_opt_by_keyword(tokens, K_FAMILY))) {
    md->family = nodefamily_parse(tok->args[0],
                                  NULL,
                                  NF_WARN_MALFORMED);
  }

  if ((tok = find_opt_by_keyword(tokens, K_P))) {
    md->exit_policy = parse_short_policy(tok->args[0]);
  }
  if ((tok = find_opt_by_keyword(tokens, K_P6))) {
    md->ipv6_exit_policy = parse_short_policy(tok->args[0]);
  }

  if (policy_is_reject_star_or_null(md->exit_policy) &&
      policy_is_reject_star_or_null(md->ipv6_exit_policy)) {
    md->policy_is_reject_star = 1;
  }

  rv = 0;
 err:

  SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
  memarea_clear(area);
  smartlist_free(tokens);

  return rv;
}

/** Parse as many microdescriptors as are found from the string starting at
 * <b>s</b> and ending at <b>eos</b>.  If allow_annotations is set, read any
 * annotations we recognize and ignore ones we don't.
 *
 * If <b>saved_location</b> isn't SAVED_IN_CACHE, make a local copy of each
 * descriptor in the body field of each microdesc_t.
 *
 * Return all newly parsed microdescriptors in a newly allocated
 * smartlist_t. If <b>invalid_disgests_out</b> is provided, add a SHA256
 * microdesc digest to it for every microdesc that we found to be badly
 * formed. (This may cause duplicates) */
smartlist_t *
microdescs_parse_from_string(const char *s, const char *eos,
                             int allow_annotations,
                             saved_location_t where,
                             smartlist_t *invalid_digests_out)
{
  smartlist_t *result;
  microdesc_t *md = NULL;
  memarea_t *area;
  const char *start = s;
  const char *start_of_next_microdesc;

  if (!eos)
    eos = s + strlen(s);

  s = eat_whitespace_eos(s, eos);
  area = memarea_new();
  result = smartlist_new();

  while (s < eos) {
   bool okay = false;

    start_of_next_microdesc = find_start_of_next_microdesc(s, eos);
    if (!start_of_next_microdesc)
      start_of_next_microdesc = eos;

    md = tor_malloc_zero(sizeof(microdesc_t));
    uint8_t md_digest[DIGEST256_LEN];
    {
      const bool body_not_found =
        microdesc_extract_body(md, start, s,
                               start_of_next_microdesc,
                               where) < 0;

      memcpy(md_digest, md->digest, DIGEST256_LEN);
      if (body_not_found) {
        log_fn(LOG_PROTOCOL_WARN, LD_DIR, "Malformed or truncated descriptor");
        goto next;
      }
    }

    if (microdesc_parse_fields(md, area, s, start_of_next_microdesc,
                               allow_annotations, where) == 0) {
      smartlist_add(result, md);
      md = NULL; // prevent free
      okay = true;
    }

  next:
    if (! okay && invalid_digests_out) {
      smartlist_add(invalid_digests_out,
                    tor_memdup(md_digest, DIGEST256_LEN));
    }
    microdesc_free(md);
    md = NULL;
    s = start_of_next_microdesc;
  }

  memarea_drop_all(area);

  return result;
}
