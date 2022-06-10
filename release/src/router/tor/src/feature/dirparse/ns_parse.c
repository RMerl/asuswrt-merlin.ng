/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file routerparse.c
 * \brief Code to parse and validate consensus documents and votes.
 */

#define NS_PARSE_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/or/protover.h"
#include "core/or/versions.h"
#include "feature/client/entrynodes.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirparse/authcert_parse.h"
#include "feature/dirparse/ns_parse.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/dirparse/routerparse.h"
#include "feature/dirparse/sigcommon.h"
#include "feature/dirparse/unparseable.h"
#include "feature/hs_common/shared_random_client.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nickname.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/memarea/memarea.h"

#include "feature/dirauth/vote_microdesc_hash_st.h"
#include "feature/nodelist/authority_cert_st.h"
#include "feature/nodelist/document_signature_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/networkstatus_voter_info_st.h"
#include "feature/nodelist/vote_routerstatus_st.h"
#include "feature/dirparse/authcert_members.h"

#undef log
#include <math.h>

/** List of tokens recognized in the body part of v3 networkstatus
 * documents. */
// clang-format off
static token_rule_t rtrstatus_token_table[] = {
  T01("p",                   K_P,               CONCAT_ARGS, NO_OBJ ),
  T1( "r",                   K_R,                   GE(7),   NO_OBJ ),
  T0N("a",                   K_A,                   GE(1),   NO_OBJ ),
  T1( "s",                   K_S,                   ARGS,    NO_OBJ ),
  T01("v",                   K_V,               CONCAT_ARGS, NO_OBJ ),
  T01("w",                   K_W,                   ARGS,    NO_OBJ ),
  T0N("m",                   K_M,               CONCAT_ARGS, NO_OBJ ),
  T0N("id",                  K_ID,                  GE(2),   NO_OBJ ),
  T1("pr",                   K_PROTO,           CONCAT_ARGS, NO_OBJ ),
  T0N("opt",                 K_OPT,             CONCAT_ARGS, OBJ_OK ),
  END_OF_TABLE
};
// clang-format on

/** List of tokens recognized in V3 networkstatus votes. */
// clang-format off
static token_rule_t networkstatus_token_table[] = {
  T1_START("network-status-version", K_NETWORK_STATUS_VERSION,
                                                   GE(1),       NO_OBJ ),
  T1("vote-status",            K_VOTE_STATUS,      GE(1),       NO_OBJ ),
  T1("published",              K_PUBLISHED,        CONCAT_ARGS, NO_OBJ ),
  T1("valid-after",            K_VALID_AFTER,      CONCAT_ARGS, NO_OBJ ),
  T1("fresh-until",            K_FRESH_UNTIL,      CONCAT_ARGS, NO_OBJ ),
  T1("valid-until",            K_VALID_UNTIL,      CONCAT_ARGS, NO_OBJ ),
  T1("voting-delay",           K_VOTING_DELAY,     GE(2),       NO_OBJ ),
  T1("known-flags",            K_KNOWN_FLAGS,      ARGS,        NO_OBJ ),
  T01("params",                K_PARAMS,           ARGS,        NO_OBJ ),
  T( "fingerprint",            K_FINGERPRINT,      CONCAT_ARGS, NO_OBJ ),
  T01("signing-ed25519",       K_SIGNING_CERT_ED,  NO_ARGS ,    NEED_OBJ ),
  T01("shared-rand-participate",K_SR_FLAG,         NO_ARGS,     NO_OBJ ),
  T0N("shared-rand-commit",    K_COMMIT,           GE(3),       NO_OBJ ),
  T01("shared-rand-previous-value", K_PREVIOUS_SRV,EQ(2),       NO_OBJ ),
  T01("shared-rand-current-value",  K_CURRENT_SRV, EQ(2),       NO_OBJ ),
  T0N("package",               K_PACKAGE,          CONCAT_ARGS, NO_OBJ ),
  T01("recommended-client-protocols", K_RECOMMENDED_CLIENT_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),
  T01("recommended-relay-protocols", K_RECOMMENDED_RELAY_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),
  T01("required-client-protocols",    K_REQUIRED_CLIENT_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),
  T01("required-relay-protocols",    K_REQUIRED_RELAY_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),

  AUTHCERT_MEMBERS,

  T0N("opt",                 K_OPT,             CONCAT_ARGS, OBJ_OK ),
  T1( "contact",             K_CONTACT,         CONCAT_ARGS, NO_OBJ ),
  T1( "dir-source",          K_DIR_SOURCE,      GE(6),       NO_OBJ ),
  T01("legacy-dir-key",      K_LEGACY_DIR_KEY,  GE(1),       NO_OBJ ),
  T1( "known-flags",         K_KNOWN_FLAGS,     CONCAT_ARGS, NO_OBJ ),
  T01("client-versions",     K_CLIENT_VERSIONS, CONCAT_ARGS, NO_OBJ ),
  T01("server-versions",     K_SERVER_VERSIONS, CONCAT_ARGS, NO_OBJ ),
  T1( "consensus-methods",   K_CONSENSUS_METHODS, GE(1),     NO_OBJ ),

  END_OF_TABLE
};
// clang-format on

/** List of tokens recognized in V3 networkstatus consensuses. */
// clang-format off
static token_rule_t networkstatus_consensus_token_table[] = {
  T1_START("network-status-version", K_NETWORK_STATUS_VERSION,
                                                   GE(1),       NO_OBJ ),
  T1("vote-status",            K_VOTE_STATUS,      GE(1),       NO_OBJ ),
  T1("valid-after",            K_VALID_AFTER,      CONCAT_ARGS, NO_OBJ ),
  T1("fresh-until",            K_FRESH_UNTIL,      CONCAT_ARGS, NO_OBJ ),
  T1("valid-until",            K_VALID_UNTIL,      CONCAT_ARGS, NO_OBJ ),
  T1("voting-delay",           K_VOTING_DELAY,     GE(2),       NO_OBJ ),

  T0N("opt",                 K_OPT,             CONCAT_ARGS, OBJ_OK ),

  T1N("dir-source",          K_DIR_SOURCE,          GE(6),   NO_OBJ ),
  T1N("contact",             K_CONTACT,         CONCAT_ARGS, NO_OBJ ),
  T1N("vote-digest",         K_VOTE_DIGEST,         GE(1),   NO_OBJ ),

  T1( "known-flags",         K_KNOWN_FLAGS,     CONCAT_ARGS, NO_OBJ ),

  T01("client-versions",     K_CLIENT_VERSIONS, CONCAT_ARGS, NO_OBJ ),
  T01("server-versions",     K_SERVER_VERSIONS, CONCAT_ARGS, NO_OBJ ),
  T01("consensus-method",    K_CONSENSUS_METHOD,    EQ(1),   NO_OBJ),
  T01("params",                K_PARAMS,           ARGS,        NO_OBJ ),

  T01("shared-rand-previous-value", K_PREVIOUS_SRV, EQ(2),   NO_OBJ ),
  T01("shared-rand-current-value",  K_CURRENT_SRV,  EQ(2),   NO_OBJ ),

  T01("recommended-client-protocols", K_RECOMMENDED_CLIENT_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),
  T01("recommended-relay-protocols", K_RECOMMENDED_RELAY_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),
  T01("required-client-protocols",    K_REQUIRED_CLIENT_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),
  T01("required-relay-protocols",    K_REQUIRED_RELAY_PROTOCOLS,
      CONCAT_ARGS, NO_OBJ ),

  END_OF_TABLE
};
// clang-format on

/** List of tokens recognized in the footer of v1 directory footers. */
// clang-format off
static token_rule_t networkstatus_vote_footer_token_table[] = {
  T01("directory-footer",    K_DIRECTORY_FOOTER,    NO_ARGS,   NO_OBJ ),
  T01("bandwidth-weights",   K_BW_WEIGHTS,          ARGS,      NO_OBJ ),
  T(  "directory-signature", K_DIRECTORY_SIGNATURE, GE(2),     NEED_OBJ ),
  END_OF_TABLE
};
// clang-format on

/** Try to find the start and end of the signed portion of a networkstatus
 * document in <b>s</b>. On success, set <b>start_out</b> to the first
 * character of the document, and <b>end_out</b> to a position one after the
 * final character of the signed document, and return 0.  On failure, return
 * -1. */
int
router_get_networkstatus_v3_signed_boundaries(const char *s,
                                              size_t len,
                                              const char **start_out,
                                              const char **end_out)
{
  return router_get_hash_impl_helper(s, len,
                                     "network-status-version",
                                     "\ndirectory-signature",
                                     ' ', LOG_INFO,
                                     start_out, end_out);
}

/** Set <b>digest_out</b> to the SHA3-256 digest of the signed portion of the
 * networkstatus vote in <b>s</b> -- or of the entirety of <b>s</b> if no
 * signed portion can be identified.  Return 0 on success, -1 on failure. */
int
router_get_networkstatus_v3_sha3_as_signed(uint8_t *digest_out,
                                           const char *s, size_t len)
{
  const char *start, *end;
  if (router_get_networkstatus_v3_signed_boundaries(s, len,
                                                    &start, &end) < 0) {
    start = s;
    end = s + len;
  }
  tor_assert(start);
  tor_assert(end);
  return crypto_digest256((char*)digest_out, start, end-start,
                          DIGEST_SHA3_256);
}

/** Set <b>digests</b> to all the digests of the consensus document in
 * <b>s</b> */
int
router_get_networkstatus_v3_hashes(const char *s, size_t len,
                                   common_digests_t *digests)
{
  return router_get_hashes_impl(s, len, digests,
                                "network-status-version",
                                "\ndirectory-signature",
                                ' ');
}

/** Helper: given a string <b>s</b>, return the start of the next router-status
 * object (starting with "r " at the start of a line).  If none is found,
 * return the start of the directory footer, or the next directory signature.
 * If none is found, return the end of the string. */
static inline const char *
find_start_of_next_routerstatus(const char *s, const char *s_eos)
{
  const char *eos, *footer, *sig;
  if ((eos = tor_memstr(s, s_eos - s, "\nr ")))
    ++eos;
  else
    eos = s_eos;

  footer = tor_memstr(s, eos-s, "\ndirectory-footer");
  sig = tor_memstr(s, eos-s, "\ndirectory-signature");

  if (footer && sig)
    return MIN(footer, sig) + 1;
  else if (footer)
    return footer+1;
  else if (sig)
    return sig+1;
  else
    return eos;
}

/** Parse the GuardFraction string from a consensus or vote.
 *
 *  If <b>vote</b> or <b>vote_rs</b> are set the document getting
 *  parsed is a vote routerstatus. Otherwise it's a consensus. This is
 *  the same semantic as in routerstatus_parse_entry_from_string(). */
STATIC int
routerstatus_parse_guardfraction(const char *guardfraction_str,
                                 networkstatus_t *vote,
                                 vote_routerstatus_t *vote_rs,
                                 routerstatus_t *rs)
{
  int ok;
  const char *end_of_header = NULL;
  int is_consensus = !vote_rs;
  uint32_t guardfraction;

  tor_assert(bool_eq(vote, vote_rs));

  /* If this info comes from a consensus, but we shouldn't apply
     guardfraction, just exit. */
  if (is_consensus && !should_apply_guardfraction(NULL)) {
    return 0;
  }

  end_of_header = strchr(guardfraction_str, '=');
  if (!end_of_header) {
    return -1;
  }

  guardfraction = (uint32_t)tor_parse_ulong(end_of_header+1,
                                            10, 0, 100, &ok, NULL);
  if (!ok) {
    log_warn(LD_DIR, "Invalid GuardFraction %s", escaped(guardfraction_str));
    return -1;
  }

  log_debug(LD_GENERAL, "[*] Parsed %s guardfraction '%s' for '%s'.",
            is_consensus ? "consensus" : "vote",
            guardfraction_str, rs->nickname);

  if (!is_consensus) { /* We are parsing a vote */
    vote_rs->status.guardfraction_percentage = guardfraction;
    vote_rs->status.has_guardfraction = 1;
  } else {
    /* We are parsing a consensus. Only apply guardfraction to guards. */
    if (rs->is_possible_guard) {
      rs->guardfraction_percentage = guardfraction;
      rs->has_guardfraction = 1;
    } else {
      log_warn(LD_BUG, "Got GuardFraction for non-guard %s. "
               "This is not supposed to happen. Not applying. ", rs->nickname);
    }
  }

  return 0;
}

/** Given a string at *<b>s</b>, containing a routerstatus object, and an
 * empty smartlist at <b>tokens</b>, parse and return the first router status
 * object in the string, and advance *<b>s</b> to just after the end of the
 * router status.  Return NULL and advance *<b>s</b> on error.
 *
 * If <b>vote</b> and <b>vote_rs</b> are provided, don't allocate a fresh
 * routerstatus but use <b>vote_rs</b> instead.
 *
 * If <b>consensus_method</b> is nonzero, this routerstatus is part of a
 * consensus, and we should parse it according to the method used to
 * make that consensus.
 *
 * Parse according to the syntax used by the consensus flavor <b>flav</b>.
 **/
STATIC routerstatus_t *
routerstatus_parse_entry_from_string(memarea_t *area,
                                     const char **s, const char *s_eos,
                                     smartlist_t *tokens,
                                     networkstatus_t *vote,
                                     vote_routerstatus_t *vote_rs,
                                     int consensus_method,
                                     consensus_flavor_t flav)
{
  const char *eos, *s_dup = *s;
  routerstatus_t *rs = NULL;
  directory_token_t *tok;
  char timebuf[ISO_TIME_LEN+1];
  struct in_addr in;
  int offset = 0;
  tor_assert(tokens);
  tor_assert(bool_eq(vote, vote_rs));

  if (!consensus_method)
    flav = FLAV_NS;
  tor_assert(flav == FLAV_NS || flav == FLAV_MICRODESC);

  eos = find_start_of_next_routerstatus(*s, s_eos);

  if (tokenize_string(area,*s, eos, tokens, rtrstatus_token_table,0)) {
    log_warn(LD_DIR, "Error tokenizing router status");
    goto err;
  }
  if (smartlist_len(tokens) < 1) {
    log_warn(LD_DIR, "Impossibly short router status");
    goto err;
  }
  tok = find_by_keyword(tokens, K_R);
  tor_assert(tok->n_args >= 7); /* guaranteed by GE(7) in K_R setup */
  if (flav == FLAV_NS) {
    if (tok->n_args < 8) {
      log_warn(LD_DIR, "Too few arguments to r");
      goto err;
    }
  } else if (flav == FLAV_MICRODESC) {
    offset = -1; /* There is no descriptor digest in an md consensus r line */
  }

  if (vote_rs) {
    rs = &vote_rs->status;
  } else {
    rs = tor_malloc_zero(sizeof(routerstatus_t));
  }

  if (!is_legal_nickname(tok->args[0])) {
    log_warn(LD_DIR,
             "Invalid nickname %s in router status; skipping.",
             escaped(tok->args[0]));
    goto err;
  }
  strlcpy(rs->nickname, tok->args[0], sizeof(rs->nickname));

  if (digest_from_base64(rs->identity_digest, tok->args[1])) {
    log_warn(LD_DIR, "Error decoding identity digest %s",
             escaped(tok->args[1]));
    goto err;
  }

  if (flav == FLAV_NS) {
    if (digest_from_base64(rs->descriptor_digest, tok->args[2])) {
      log_warn(LD_DIR, "Error decoding descriptor digest %s",
               escaped(tok->args[2]));
      goto err;
    }
  }

  if (tor_snprintf(timebuf, sizeof(timebuf), "%s %s",
                   tok->args[3+offset], tok->args[4+offset]) < 0 ||
      parse_iso_time(timebuf, &rs->published_on)<0) {
    log_warn(LD_DIR, "Error parsing time '%s %s' [%d %d]",
             tok->args[3+offset], tok->args[4+offset],
             offset, (int)flav);
    goto err;
  }

  if (tor_inet_aton(tok->args[5+offset], &in) == 0) {
    log_warn(LD_DIR, "Error parsing router address in network-status %s",
             escaped(tok->args[5+offset]));
    goto err;
  }
  tor_addr_from_in(&rs->ipv4_addr, &in);

  rs->ipv4_orport = (uint16_t) tor_parse_long(tok->args[6+offset],
                                              10,0,65535,NULL,NULL);
  rs->ipv4_dirport = (uint16_t) tor_parse_long(tok->args[7+offset],
                                               10,0,65535,NULL,NULL);

  {
    smartlist_t *a_lines = find_all_by_keyword(tokens, K_A);
    if (a_lines) {
      find_single_ipv6_orport(a_lines, &rs->ipv6_addr, &rs->ipv6_orport);
      smartlist_free(a_lines);
    }
  }

  tok = find_opt_by_keyword(tokens, K_S);
  if (tok && vote) {
    int i;
    vote_rs->flags = 0;
    for (i=0; i < tok->n_args; ++i) {
      int p = smartlist_string_pos(vote->known_flags, tok->args[i]);
      if (p >= 0) {
        vote_rs->flags |= (UINT64_C(1)<<p);
      } else {
        log_warn(LD_DIR, "Flags line had a flag %s not listed in known_flags.",
                 escaped(tok->args[i]));
        goto err;
      }
    }
  } else if (tok) {
    /* This is a consensus, not a vote. */
    int i;
    for (i=0; i < tok->n_args; ++i) {
      if (!strcmp(tok->args[i], "Exit"))
        rs->is_exit = 1;
      else if (!strcmp(tok->args[i], "Stable"))
        rs->is_stable = 1;
      else if (!strcmp(tok->args[i], "Fast"))
        rs->is_fast = 1;
      else if (!strcmp(tok->args[i], "Running"))
        rs->is_flagged_running = 1;
      else if (!strcmp(tok->args[i], "Named"))
        rs->is_named = 1;
      else if (!strcmp(tok->args[i], "Valid"))
        rs->is_valid = 1;
      else if (!strcmp(tok->args[i], "Guard"))
        rs->is_possible_guard = 1;
      else if (!strcmp(tok->args[i], "BadExit"))
        rs->is_bad_exit = 1;
      else if (!strcmp(tok->args[i], "MiddleOnly"))
        rs->is_middle_only = 1;
      else if (!strcmp(tok->args[i], "Authority"))
        rs->is_authority = 1;
      else if (!strcmp(tok->args[i], "Unnamed") &&
               consensus_method >= 2) {
        /* Unnamed is computed right by consensus method 2 and later. */
        rs->is_unnamed = 1;
      } else if (!strcmp(tok->args[i], "HSDir")) {
        rs->is_hs_dir = 1;
      } else if (!strcmp(tok->args[i], "V2Dir")) {
        rs->is_v2_dir = 1;
      } else if (!strcmp(tok->args[i], "StaleDesc")) {
        rs->is_staledesc = 1;
      } else if (!strcmp(tok->args[i], "Sybil")) {
        rs->is_sybil = 1;
      }
    }
    /* These are implied true by having been included in a consensus made
     * with a given method */
    rs->is_flagged_running = 1; /* Starting with consensus method 4. */
    rs->is_valid = 1; /* Starting with consensus method 24. */
  }
  {
    const char *protocols = NULL, *version = NULL;
    if ((tok = find_opt_by_keyword(tokens, K_PROTO))) {
      tor_assert(tok->n_args == 1);
      protocols = tok->args[0];
    }
    if ((tok = find_opt_by_keyword(tokens, K_V))) {
      tor_assert(tok->n_args == 1);
      version = tok->args[0];
      if (vote_rs) {
        vote_rs->version = tor_strdup(tok->args[0]);
      }
    }

    // If the protover line is malformed, reject this routerstatus.
    if (protocols && protover_list_is_invalid(protocols)) {
      goto err;
    }
    summarize_protover_flags(&rs->pv, protocols, version);
  }

  /* handle weighting/bandwidth info */
  if ((tok = find_opt_by_keyword(tokens, K_W))) {
    int i;
    for (i=0; i < tok->n_args; ++i) {
      if (!strcmpstart(tok->args[i], "Bandwidth=")) {
        int ok;
        rs->bandwidth_kb =
          (uint32_t)tor_parse_ulong(strchr(tok->args[i], '=')+1,
                                    10, 0, UINT32_MAX,
                                    &ok, NULL);
        if (!ok) {
          log_warn(LD_DIR, "Invalid Bandwidth %s", escaped(tok->args[i]));
          goto err;
        }
        rs->has_bandwidth = 1;
      } else if (!strcmpstart(tok->args[i], "Measured=") && vote_rs) {
        int ok;
        vote_rs->measured_bw_kb =
            (uint32_t)tor_parse_ulong(strchr(tok->args[i], '=')+1,
                                      10, 0, UINT32_MAX, &ok, NULL);
        if (!ok) {
          log_warn(LD_DIR, "Invalid Measured Bandwidth %s",
                   escaped(tok->args[i]));
          goto err;
        }
        vote_rs->has_measured_bw = 1;
        vote->has_measured_bws = 1;
      } else if (!strcmpstart(tok->args[i], "Unmeasured=1")) {
        rs->bw_is_unmeasured = 1;
      } else if (!strcmpstart(tok->args[i], "GuardFraction=")) {
        if (routerstatus_parse_guardfraction(tok->args[i],
                                             vote, vote_rs, rs) < 0) {
          goto err;
        }
      }
    }
  }

  /* parse exit policy summaries */
  if ((tok = find_opt_by_keyword(tokens, K_P))) {
    tor_assert(tok->n_args == 1);
    if (strcmpstart(tok->args[0], "accept ") &&
        strcmpstart(tok->args[0], "reject ")) {
      log_warn(LD_DIR, "Unknown exit policy summary type %s.",
               escaped(tok->args[0]));
      goto err;
    }
    /* XXX weasel: parse this into ports and represent them somehow smart,
     * maybe not here but somewhere on if we need it for the client.
     * we should still parse it here to check it's valid tho.
     */
    rs->exitsummary = tor_strdup(tok->args[0]);
    rs->has_exitsummary = 1;
  }

  if (vote_rs) {
    SMARTLIST_FOREACH_BEGIN(tokens, directory_token_t *, t) {
      if (t->tp == K_M && t->n_args) {
        vote_microdesc_hash_t *line =
          tor_malloc(sizeof(vote_microdesc_hash_t));
        line->next = vote_rs->microdesc;
        line->microdesc_hash_line = tor_strdup(t->args[0]);
        vote_rs->microdesc = line;
      }
      if (t->tp == K_ID) {
        tor_assert(t->n_args >= 2);
        if (!strcmp(t->args[0], "ed25519")) {
          vote_rs->has_ed25519_listing = 1;
          if (strcmp(t->args[1], "none") &&
              digest256_from_base64((char*)vote_rs->ed25519_id,
                                    t->args[1])<0) {
            log_warn(LD_DIR, "Bogus ed25519 key in networkstatus vote");
            goto err;
          }
        }
      }
      if (t->tp == K_PROTO) {
        tor_assert(t->n_args == 1);
        vote_rs->protocols = tor_strdup(t->args[0]);
      }
    } SMARTLIST_FOREACH_END(t);
  } else if (flav == FLAV_MICRODESC) {
    tok = find_opt_by_keyword(tokens, K_M);
    if (tok) {
      tor_assert(tok->n_args);
      if (digest256_from_base64(rs->descriptor_digest, tok->args[0])) {
        log_warn(LD_DIR, "Error decoding microdescriptor digest %s",
                 escaped(tok->args[0]));
        goto err;
      }
    } else {
      log_info(LD_BUG, "Found an entry in networkstatus with no "
               "microdescriptor digest. (Router %s ($%s) at %s:%d.)",
               rs->nickname, hex_str(rs->identity_digest, DIGEST_LEN),
               fmt_addr(&rs->ipv4_addr), rs->ipv4_orport);
    }
  }

  if (!strcasecmp(rs->nickname, UNNAMED_ROUTER_NICKNAME))
    rs->is_named = 0;

  goto done;
 err:
  dump_desc(s_dup, "routerstatus entry");
  if (rs && !vote_rs)
    routerstatus_free(rs);
  rs = NULL;
 done:
  SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
  smartlist_clear(tokens);
  if (area) {
    DUMP_AREA(area, "routerstatus entry");
    memarea_clear(area);
  }
  *s = eos;

  return rs;
}

int
compare_vote_routerstatus_entries(const void **_a, const void **_b)
{
  const vote_routerstatus_t *a = *_a, *b = *_b;
  return fast_memcmp(a->status.identity_digest, b->status.identity_digest,
                     DIGEST_LEN);
}

/** Verify the bandwidth weights of a network status document */
int
networkstatus_verify_bw_weights(networkstatus_t *ns, int consensus_method)
{
  int64_t G=0, M=0, E=0, D=0, T=0;
  double Wgg, Wgm, Wgd, Wmg, Wmm, Wme, Wmd, Weg, Wem, Wee, Wed;
  double Gtotal=0, Mtotal=0, Etotal=0;
  const char *casename = NULL;
  int valid = 1;
  (void) consensus_method;

  const int64_t weight_scale = networkstatus_get_weight_scale_param(ns);
  tor_assert(weight_scale >= 1);
  Wgg = networkstatus_get_bw_weight(ns, "Wgg", -1);
  Wgm = networkstatus_get_bw_weight(ns, "Wgm", -1);
  Wgd = networkstatus_get_bw_weight(ns, "Wgd", -1);
  Wmg = networkstatus_get_bw_weight(ns, "Wmg", -1);
  Wmm = networkstatus_get_bw_weight(ns, "Wmm", -1);
  Wme = networkstatus_get_bw_weight(ns, "Wme", -1);
  Wmd = networkstatus_get_bw_weight(ns, "Wmd", -1);
  Weg = networkstatus_get_bw_weight(ns, "Weg", -1);
  Wem = networkstatus_get_bw_weight(ns, "Wem", -1);
  Wee = networkstatus_get_bw_weight(ns, "Wee", -1);
  Wed = networkstatus_get_bw_weight(ns, "Wed", -1);

  if (Wgg<0 || Wgm<0 || Wgd<0 || Wmg<0 || Wmm<0 || Wme<0 || Wmd<0 || Weg<0
          || Wem<0 || Wee<0 || Wed<0) {
    log_warn(LD_BUG, "No bandwidth weights produced in consensus!");
    return 0;
  }

  // First, sanity check basic summing properties that hold for all cases
  // We use > 1 as the check for these because they are computed as integers.
  // Sometimes there are rounding errors.
  if (fabs(Wmm - weight_scale) > 1) {
    log_warn(LD_BUG, "Wmm=%f != %"PRId64,
             Wmm, (weight_scale));
    valid = 0;
  }

  if (fabs(Wem - Wee) > 1) {
    log_warn(LD_BUG, "Wem=%f != Wee=%f", Wem, Wee);
    valid = 0;
  }

  if (fabs(Wgm - Wgg) > 1) {
    log_warn(LD_BUG, "Wgm=%f != Wgg=%f", Wgm, Wgg);
    valid = 0;
  }

  if (fabs(Weg - Wed) > 1) {
    log_warn(LD_BUG, "Wed=%f != Weg=%f", Wed, Weg);
    valid = 0;
  }

  if (fabs(Wgg + Wmg - weight_scale) > 0.001*weight_scale) {
    log_warn(LD_BUG, "Wgg=%f != %"PRId64" - Wmg=%f", Wgg,
             (weight_scale), Wmg);
    valid = 0;
  }

  if (fabs(Wee + Wme - weight_scale) > 0.001*weight_scale) {
    log_warn(LD_BUG, "Wee=%f != %"PRId64" - Wme=%f", Wee,
             (weight_scale), Wme);
    valid = 0;
  }

  if (fabs(Wgd + Wmd + Wed - weight_scale) > 0.001*weight_scale) {
    log_warn(LD_BUG, "Wgd=%f + Wmd=%f + Wed=%f != %"PRId64,
             Wgd, Wmd, Wed, (weight_scale));
    valid = 0;
  }

  Wgg /= weight_scale;
  Wgm /= weight_scale; (void) Wgm; // unused from here on.
  Wgd /= weight_scale;

  Wmg /= weight_scale;
  Wmm /= weight_scale;
  Wme /= weight_scale;
  Wmd /= weight_scale;

  Weg /= weight_scale; (void) Weg; // unused from here on.
  Wem /= weight_scale; (void) Wem; // unused from here on.
  Wee /= weight_scale;
  Wed /= weight_scale;

  // Then, gather G, M, E, D, T to determine case
  SMARTLIST_FOREACH_BEGIN(ns->routerstatus_list, routerstatus_t *, rs) {
    int is_exit = 0;
    /* Bug #2203: Don't count bad exits as exits for balancing */
    is_exit = rs->is_exit && !rs->is_bad_exit;
    if (rs->has_bandwidth) {
      T += rs->bandwidth_kb;
      if (is_exit && rs->is_possible_guard) {
        D += rs->bandwidth_kb;
        Gtotal += Wgd*rs->bandwidth_kb;
        Mtotal += Wmd*rs->bandwidth_kb;
        Etotal += Wed*rs->bandwidth_kb;
      } else if (is_exit) {
        E += rs->bandwidth_kb;
        Mtotal += Wme*rs->bandwidth_kb;
        Etotal += Wee*rs->bandwidth_kb;
      } else if (rs->is_possible_guard) {
        G += rs->bandwidth_kb;
        Gtotal += Wgg*rs->bandwidth_kb;
        Mtotal += Wmg*rs->bandwidth_kb;
      } else {
        M += rs->bandwidth_kb;
        Mtotal += Wmm*rs->bandwidth_kb;
      }
    } else {
      log_warn(LD_BUG, "Missing consensus bandwidth for router %s",
               routerstatus_describe(rs));
    }
  } SMARTLIST_FOREACH_END(rs);

  // Finally, check equality conditions depending upon case 1, 2 or 3
  // Full equality cases: 1, 3b
  // Partial equality cases: 2b (E=G), 3a (M=E)
  // Fully unknown: 2a
  if (3*E >= T && 3*G >= T) {
    // Case 1: Neither are scarce
    casename = "Case 1";
    if (fabs(Etotal-Mtotal) > 0.01*MAX(Etotal,Mtotal)) {
      log_warn(LD_DIR,
               "Bw Weight Failure for %s: Etotal %f != Mtotal %f. "
               "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
               " T=%"PRId64". "
               "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
               casename, Etotal, Mtotal,
               (G), (M), (E),
               (D), (T),
               Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
      valid = 0;
    }
    if (fabs(Etotal-Gtotal) > 0.01*MAX(Etotal,Gtotal)) {
      log_warn(LD_DIR,
               "Bw Weight Failure for %s: Etotal %f != Gtotal %f. "
               "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
               " T=%"PRId64". "
               "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
               casename, Etotal, Gtotal,
               (G), (M), (E),
               (D), (T),
               Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
      valid = 0;
    }
    if (fabs(Gtotal-Mtotal) > 0.01*MAX(Gtotal,Mtotal)) {
      log_warn(LD_DIR,
               "Bw Weight Failure for %s: Mtotal %f != Gtotal %f. "
               "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
               " T=%"PRId64". "
               "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
               casename, Mtotal, Gtotal,
               (G), (M), (E),
               (D), (T),
               Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
      valid = 0;
    }
  } else if (3*E < T && 3*G < T) {
    int64_t R = MIN(E, G);
    int64_t S = MAX(E, G);
    /*
     * Case 2: Both Guards and Exits are scarce
     * Balance D between E and G, depending upon
     * D capacity and scarcity. Devote no extra
     * bandwidth to middle nodes.
     */
    if (R+D < S) { // Subcase a
      double Rtotal, Stotal;
      if (E < G) {
        Rtotal = Etotal;
        Stotal = Gtotal;
      } else {
        Rtotal = Gtotal;
        Stotal = Etotal;
      }
      casename = "Case 2a";
      // Rtotal < Stotal
      if (Rtotal > Stotal) {
        log_warn(LD_DIR,
                   "Bw Weight Failure for %s: Rtotal %f > Stotal %f. "
                   "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                   " T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Rtotal, Stotal,
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
      // Rtotal < T/3
      if (3*Rtotal > T) {
        log_warn(LD_DIR,
                   "Bw Weight Failure for %s: 3*Rtotal %f > T "
                   "%"PRId64". G=%"PRId64" M=%"PRId64" E=%"PRId64
                   " D=%"PRId64" T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Rtotal*3, (T),
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
      // Stotal < T/3
      if (3*Stotal > T) {
        log_warn(LD_DIR,
                   "Bw Weight Failure for %s: 3*Stotal %f > T "
                   "%"PRId64". G=%"PRId64" M=%"PRId64" E=%"PRId64
                   " D=%"PRId64" T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Stotal*3, (T),
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
      // Mtotal > T/3
      if (3*Mtotal < T) {
        log_warn(LD_DIR,
                   "Bw Weight Failure for %s: 3*Mtotal %f < T "
                   "%"PRId64". "
                   "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                   " T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Mtotal*3, (T),
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
    } else { // Subcase b: R+D > S
      casename = "Case 2b";

      /* Check the rare-M redirect case. */
      if (D != 0 && 3*M < T) {
        casename = "Case 2b (balanced)";
        if (fabs(Etotal-Mtotal) > 0.01*MAX(Etotal,Mtotal)) {
          log_warn(LD_DIR,
                   "Bw Weight Failure for %s: Etotal %f != Mtotal %f. "
                   "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                   " T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Etotal, Mtotal,
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
          valid = 0;
        }
        if (fabs(Etotal-Gtotal) > 0.01*MAX(Etotal,Gtotal)) {
          log_warn(LD_DIR,
                   "Bw Weight Failure for %s: Etotal %f != Gtotal %f. "
                   "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                   " T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Etotal, Gtotal,
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
          valid = 0;
        }
        if (fabs(Gtotal-Mtotal) > 0.01*MAX(Gtotal,Mtotal)) {
          log_warn(LD_DIR,
                   "Bw Weight Failure for %s: Mtotal %f != Gtotal %f. "
                   "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                   " T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Mtotal, Gtotal,
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
          valid = 0;
        }
      } else {
        if (fabs(Etotal-Gtotal) > 0.01*MAX(Etotal,Gtotal)) {
          log_warn(LD_DIR,
                   "Bw Weight Failure for %s: Etotal %f != Gtotal %f. "
                   "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                   " T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Etotal, Gtotal,
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
          valid = 0;
        }
      }
    }
  } else { // if (E < T/3 || G < T/3) {
    int64_t S = MIN(E, G);
    int64_t NS = MAX(E, G);
    if (3*(S+D) < T) { // Subcase a:
      double Stotal;
      double NStotal;
      if (G < E) {
        casename = "Case 3a (G scarce)";
        Stotal = Gtotal;
        NStotal = Etotal;
      } else { // if (G >= E) {
        casename = "Case 3a (E scarce)";
        NStotal = Gtotal;
        Stotal = Etotal;
      }
      // Stotal < T/3
      if (3*Stotal > T) {
        log_warn(LD_DIR,
                   "Bw Weight Failure for %s: 3*Stotal %f > T "
                   "%"PRId64". G=%"PRId64" M=%"PRId64" E=%"PRId64
                   " D=%"PRId64" T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, Stotal*3, (T),
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
      if (NS >= M) {
        if (fabs(NStotal-Mtotal) > 0.01*MAX(NStotal,Mtotal)) {
          log_warn(LD_DIR,
                   "Bw Weight Failure for %s: NStotal %f != Mtotal %f. "
                   "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                   " T=%"PRId64". "
                   "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                   casename, NStotal, Mtotal,
                   (G), (M), (E),
                   (D), (T),
                   Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
          valid = 0;
        }
      } else {
        // if NS < M, NStotal > T/3 because only one of G or E is scarce
        if (3*NStotal < T) {
          log_warn(LD_DIR,
                     "Bw Weight Failure for %s: 3*NStotal %f < T "
                     "%"PRId64". G=%"PRId64" M=%"PRId64
                     " E=%"PRId64" D=%"PRId64" T=%"PRId64". "
                     "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                     casename, NStotal*3, (T),
                     (G), (M), (E),
                     (D), (T),
                     Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
          valid = 0;
        }
      }
    } else { // Subcase b: S+D >= T/3
      casename = "Case 3b";
      if (fabs(Etotal-Mtotal) > 0.01*MAX(Etotal,Mtotal)) {
        log_warn(LD_DIR,
                 "Bw Weight Failure for %s: Etotal %f != Mtotal %f. "
                 "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                 " T=%"PRId64". "
                 "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                 casename, Etotal, Mtotal,
                 (G), (M), (E),
                 (D), (T),
                 Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
      if (fabs(Etotal-Gtotal) > 0.01*MAX(Etotal,Gtotal)) {
        log_warn(LD_DIR,
                 "Bw Weight Failure for %s: Etotal %f != Gtotal %f. "
                 "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                 " T=%"PRId64". "
                 "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                 casename, Etotal, Gtotal,
                 (G), (M), (E),
                 (D), (T),
                 Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
      if (fabs(Gtotal-Mtotal) > 0.01*MAX(Gtotal,Mtotal)) {
        log_warn(LD_DIR,
                 "Bw Weight Failure for %s: Mtotal %f != Gtotal %f. "
                 "G=%"PRId64" M=%"PRId64" E=%"PRId64" D=%"PRId64
                 " T=%"PRId64". "
                 "Wgg=%f Wgd=%f Wmg=%f Wme=%f Wmd=%f Wee=%f Wed=%f",
                 casename, Mtotal, Gtotal,
                 (G), (M), (E),
                 (D), (T),
                 Wgg, Wgd, Wmg, Wme, Wmd, Wee, Wed);
        valid = 0;
      }
    }
  }

  if (valid)
    log_notice(LD_DIR, "Bandwidth-weight %s is verified and valid.",
               casename);

  return valid;
}

/** Check if a shared random value of type <b>srv_type</b> is in
 *  <b>tokens</b>. If there is, parse it and set it to <b>srv_out</b>. Return
 *  -1 on failure, 0 on success. The resulting srv is allocated on the heap and
 *  it's the responsibility of the caller to free it. */
static int
extract_one_srv(smartlist_t *tokens, directory_keyword srv_type,
                sr_srv_t **srv_out)
{
  int ret = -1;
  directory_token_t *tok;
  sr_srv_t *srv = NULL;
  smartlist_t *chunks;

  tor_assert(tokens);

  chunks = smartlist_new();
  tok = find_opt_by_keyword(tokens, srv_type);
  if (!tok) {
    /* That's fine, no SRV is allowed. */
    ret = 0;
    goto end;
  }
  for (int i = 0; i < tok->n_args; i++) {
    smartlist_add(chunks, tok->args[i]);
  }
  srv = sr_parse_srv(chunks);
  if (srv == NULL) {
    log_warn(LD_DIR, "SR: Unparseable SRV %s", escaped(tok->object_body));
    goto end;
  }
  /* All is good. */
  *srv_out = srv;
  ret = 0;
 end:
  smartlist_free(chunks);
  return ret;
}

/** Extract any shared random values found in <b>tokens</b> and place them in
 *  the networkstatus <b>ns</b>. */
static void
extract_shared_random_srvs(networkstatus_t *ns, smartlist_t *tokens)
{
  const char *voter_identity;
  networkstatus_voter_info_t *voter;

  tor_assert(ns);
  tor_assert(tokens);
  /* Can be only one of them else code flow. */
  tor_assert(ns->type == NS_TYPE_VOTE || ns->type == NS_TYPE_CONSENSUS);

  if (ns->type == NS_TYPE_VOTE) {
    voter = smartlist_get(ns->voters, 0);
    tor_assert(voter);
    voter_identity = hex_str(voter->identity_digest,
                             sizeof(voter->identity_digest));
  } else {
    /* Consensus has multiple voters so no specific voter. */
    voter_identity = "consensus";
  }

  /* We extract both, and on error everything is stopped because it means
   * the vote is malformed for the shared random value(s). */
  if (extract_one_srv(tokens, K_PREVIOUS_SRV, &ns->sr_info.previous_srv) < 0) {
    log_warn(LD_DIR, "SR: Unable to parse previous SRV from %s",
             voter_identity);
    /* Maybe we have a chance with the current SRV so let's try it anyway. */
  }
  if (extract_one_srv(tokens, K_CURRENT_SRV, &ns->sr_info.current_srv) < 0) {
    log_warn(LD_DIR, "SR: Unable to parse current SRV from %s",
             voter_identity);
  }
}

/** Allocate a copy of a protover line, if present. If present but malformed,
 * set *error to true. */
static char *
dup_protocols_string(smartlist_t *tokens, bool *error, directory_keyword kw)
{
  directory_token_t *tok = find_opt_by_keyword(tokens, kw);
  if (!tok)
    return NULL;
  if (protover_list_is_invalid(tok->args[0]))
    *error = true;
  return tor_strdup(tok->args[0]);
}

/** Parse a v3 networkstatus vote, opinion, or consensus (depending on
 * ns_type), from <b>s</b>, and return the result.  Return NULL on failure. */
networkstatus_t *
networkstatus_parse_vote_from_string(const char *s,
                                     size_t s_len,
                                     const char **eos_out,
                                     networkstatus_type_t ns_type)
{
  smartlist_t *tokens = smartlist_new();
  smartlist_t *rs_tokens = NULL, *footer_tokens = NULL;
  networkstatus_voter_info_t *voter = NULL;
  networkstatus_t *ns = NULL;
  common_digests_t ns_digests;
  uint8_t sha3_as_signed[DIGEST256_LEN];
  const char *cert, *end_of_header, *end_of_footer, *s_dup = s;
  directory_token_t *tok;
  struct in_addr in;
  int i, inorder, n_signatures = 0;
  memarea_t *area = NULL, *rs_area = NULL;
  consensus_flavor_t flav = FLAV_NS;
  char *last_kwd=NULL;
  const char *eos = s + s_len;

  tor_assert(s);

  if (eos_out)
    *eos_out = NULL;

  if (router_get_networkstatus_v3_hashes(s, s_len, &ns_digests) ||
      router_get_networkstatus_v3_sha3_as_signed(sha3_as_signed,
                                                 s, s_len)<0) {
    log_warn(LD_DIR, "Unable to compute digest of network-status");
    goto err;
  }

  area = memarea_new();
  end_of_header = find_start_of_next_routerstatus(s, eos);
  if (tokenize_string(area, s, end_of_header, tokens,
                      (ns_type == NS_TYPE_CONSENSUS) ?
                      networkstatus_consensus_token_table :
                      networkstatus_token_table, 0)) {
    log_warn(LD_DIR, "Error tokenizing network-status header");
    goto err;
  }

  ns = tor_malloc_zero(sizeof(networkstatus_t));
  memcpy(&ns->digests, &ns_digests, sizeof(ns_digests));
  memcpy(&ns->digest_sha3_as_signed, sha3_as_signed, sizeof(sha3_as_signed));

  tok = find_by_keyword(tokens, K_NETWORK_STATUS_VERSION);
  tor_assert(tok);
  if (tok->n_args > 1) {
    int flavor = networkstatus_parse_flavor_name(tok->args[1]);
    if (flavor < 0) {
      log_warn(LD_DIR, "Can't parse document with unknown flavor %s",
               escaped(tok->args[1]));
      goto err;
    }
    ns->flavor = flav = flavor;
  }
  if (flav != FLAV_NS && ns_type != NS_TYPE_CONSENSUS) {
    log_warn(LD_DIR, "Flavor found on non-consensus networkstatus.");
    goto err;
  }

  if (ns_type != NS_TYPE_CONSENSUS) {
    const char *end_of_cert = NULL;
    if (!(cert = tor_memstr(s, end_of_header - s,
                            "\ndir-key-certificate-version")))
      goto err;
    ++cert;
    ns->cert = authority_cert_parse_from_string(cert, end_of_header - cert,
                                                &end_of_cert);
    if (!ns->cert || !end_of_cert || end_of_cert > end_of_header)
      goto err;
  }

  tok = find_by_keyword(tokens, K_VOTE_STATUS);
  tor_assert(tok->n_args);
  if (!strcmp(tok->args[0], "vote")) {
    ns->type = NS_TYPE_VOTE;
  } else if (!strcmp(tok->args[0], "consensus")) {
    ns->type = NS_TYPE_CONSENSUS;
  } else if (!strcmp(tok->args[0], "opinion")) {
    ns->type = NS_TYPE_OPINION;
  } else {
    log_warn(LD_DIR, "Unrecognized vote status %s in network-status",
             escaped(tok->args[0]));
    goto err;
  }
  if (ns_type != ns->type) {
    log_warn(LD_DIR, "Got the wrong kind of v3 networkstatus.");
    goto err;
  }

  if (ns->type == NS_TYPE_VOTE || ns->type == NS_TYPE_OPINION) {
    tok = find_by_keyword(tokens, K_PUBLISHED);
    if (parse_iso_time(tok->args[0], &ns->published))
      goto err;

    ns->supported_methods = smartlist_new();
    tok = find_opt_by_keyword(tokens, K_CONSENSUS_METHODS);
    if (tok) {
      for (i=0; i < tok->n_args; ++i)
        smartlist_add_strdup(ns->supported_methods, tok->args[i]);
    } else {
      smartlist_add_strdup(ns->supported_methods, "1");
    }
  } else {
    tok = find_opt_by_keyword(tokens, K_CONSENSUS_METHOD);
    if (tok) {
      int num_ok;
      ns->consensus_method = (int)tor_parse_long(tok->args[0], 10, 1, INT_MAX,
                                                 &num_ok, NULL);
      if (!num_ok)
        goto err;
    } else {
      ns->consensus_method = 1;
    }
  }

  // Reject the vote if any of the protocols lines are malformed.
  bool unparseable = false;
  ns->recommended_client_protocols = dup_protocols_string(tokens, &unparseable,
                                         K_RECOMMENDED_CLIENT_PROTOCOLS);
  ns->recommended_relay_protocols = dup_protocols_string(tokens, &unparseable,
                                         K_RECOMMENDED_RELAY_PROTOCOLS);
  ns->required_client_protocols = dup_protocols_string(tokens, &unparseable,
                                         K_REQUIRED_CLIENT_PROTOCOLS);
  ns->required_relay_protocols = dup_protocols_string(tokens, &unparseable,
                                         K_REQUIRED_RELAY_PROTOCOLS);
  if (unparseable)
    goto err;

  tok = find_by_keyword(tokens, K_VALID_AFTER);
  if (parse_iso_time(tok->args[0], &ns->valid_after))
    goto err;

  tok = find_by_keyword(tokens, K_FRESH_UNTIL);
  if (parse_iso_time(tok->args[0], &ns->fresh_until))
    goto err;

  tok = find_by_keyword(tokens, K_VALID_UNTIL);
  if (parse_iso_time(tok->args[0], &ns->valid_until))
    goto err;

  tok = find_by_keyword(tokens, K_VOTING_DELAY);
  tor_assert(tok->n_args >= 2);
  {
    int ok;
    ns->vote_seconds =
      (int) tor_parse_long(tok->args[0], 10, 0, INT_MAX, &ok, NULL);
    if (!ok)
      goto err;
    ns->dist_seconds =
      (int) tor_parse_long(tok->args[1], 10, 0, INT_MAX, &ok, NULL);
    if (!ok)
      goto err;
  }
  if (ns->valid_after +
      (get_options()->TestingTorNetwork ?
       MIN_VOTE_INTERVAL_TESTING : MIN_VOTE_INTERVAL) > ns->fresh_until) {
    log_warn(LD_DIR, "Vote/consensus freshness interval is too short");
    goto err;
  }
  if (ns->valid_after +
      (get_options()->TestingTorNetwork ?
       MIN_VOTE_INTERVAL_TESTING : MIN_VOTE_INTERVAL)*2 > ns->valid_until) {
    log_warn(LD_DIR, "Vote/consensus liveness interval is too short");
    goto err;
  }
  if (ns->vote_seconds < MIN_VOTE_SECONDS) {
    log_warn(LD_DIR, "Vote seconds is too short");
    goto err;
  }
  if (ns->dist_seconds < MIN_DIST_SECONDS) {
    log_warn(LD_DIR, "Dist seconds is too short");
    goto err;
  }

  if ((tok = find_opt_by_keyword(tokens, K_CLIENT_VERSIONS))) {
    ns->client_versions = tor_strdup(tok->args[0]);
  }
  if ((tok = find_opt_by_keyword(tokens, K_SERVER_VERSIONS))) {
    ns->server_versions = tor_strdup(tok->args[0]);
  }

  {
    smartlist_t *package_lst = find_all_by_keyword(tokens, K_PACKAGE);
    ns->package_lines = smartlist_new();
    if (package_lst) {
      SMARTLIST_FOREACH(package_lst, directory_token_t *, t,
                    smartlist_add_strdup(ns->package_lines, t->args[0]));
    }
    smartlist_free(package_lst);
  }

  tok = find_by_keyword(tokens, K_KNOWN_FLAGS);
  ns->known_flags = smartlist_new();
  inorder = 1;
  for (i = 0; i < tok->n_args; ++i) {
    smartlist_add_strdup(ns->known_flags, tok->args[i]);
    if (i>0 && strcmp(tok->args[i-1], tok->args[i])>= 0) {
      log_warn(LD_DIR, "%s >= %s", tok->args[i-1], tok->args[i]);
      inorder = 0;
    }
  }
  if (!inorder) {
    log_warn(LD_DIR, "known-flags not in order");
    goto err;
  }
  if (ns->type != NS_TYPE_CONSENSUS &&
      smartlist_len(ns->known_flags) > MAX_KNOWN_FLAGS_IN_VOTE) {
    /* If we allowed more than 64 flags in votes, then parsing them would make
     * us invoke undefined behavior whenever we used 1<<flagnum to do a
     * bit-shift. This is only for votes and opinions: consensus users don't
     * care about flags they don't recognize, and so don't build a bitfield
     * for them. */
    log_warn(LD_DIR, "Too many known-flags in consensus vote or opinion");
    goto err;
  }

  tok = find_opt_by_keyword(tokens, K_PARAMS);
  if (tok) {
    int any_dups = 0;
    inorder = 1;
    ns->net_params = smartlist_new();
    for (i = 0; i < tok->n_args; ++i) {
      int ok=0;
      char *eq = strchr(tok->args[i], '=');
      size_t eq_pos;
      if (!eq) {
        log_warn(LD_DIR, "Bad element '%s' in params", escaped(tok->args[i]));
        goto err;
      }
      eq_pos = eq-tok->args[i];
      tor_parse_long(eq+1, 10, INT32_MIN, INT32_MAX, &ok, NULL);
      if (!ok) {
        log_warn(LD_DIR, "Bad element '%s' in params", escaped(tok->args[i]));
        goto err;
      }
      if (i > 0 && strcmp(tok->args[i-1], tok->args[i]) >= 0) {
        log_warn(LD_DIR, "%s >= %s", tok->args[i-1], tok->args[i]);
        inorder = 0;
      }
      if (last_kwd && eq_pos == strlen(last_kwd) &&
          fast_memeq(last_kwd, tok->args[i], eq_pos)) {
        log_warn(LD_DIR, "Duplicate value for %s parameter",
                 escaped(tok->args[i]));
        any_dups = 1;
      }
      tor_free(last_kwd);
      last_kwd = tor_strndup(tok->args[i], eq_pos);
      smartlist_add_strdup(ns->net_params, tok->args[i]);
    }
    if (!inorder) {
      log_warn(LD_DIR, "params not in order");
      goto err;
    }
    if (any_dups) {
      log_warn(LD_DIR, "Duplicate in parameters");
      goto err;
    }
  }

  ns->voters = smartlist_new();

  SMARTLIST_FOREACH_BEGIN(tokens, directory_token_t *, _tok) {
    tok = _tok;
    if (tok->tp == K_DIR_SOURCE) {
      tor_assert(tok->n_args >= 6);

      if (voter)
        smartlist_add(ns->voters, voter);
      voter = tor_malloc_zero(sizeof(networkstatus_voter_info_t));
      voter->sigs = smartlist_new();
      if (ns->type != NS_TYPE_CONSENSUS)
        memcpy(voter->vote_digest, ns_digests.d[DIGEST_SHA1], DIGEST_LEN);

      voter->nickname = tor_strdup(tok->args[0]);
      if (strlen(tok->args[1]) != HEX_DIGEST_LEN ||
          base16_decode(voter->identity_digest, sizeof(voter->identity_digest),
                        tok->args[1], HEX_DIGEST_LEN)
                        != sizeof(voter->identity_digest)) {
        log_warn(LD_DIR, "Error decoding identity digest %s in "
                 "network-status document.", escaped(tok->args[1]));
        goto err;
      }
      if (ns->type != NS_TYPE_CONSENSUS &&
          tor_memneq(ns->cert->cache_info.identity_digest,
                 voter->identity_digest, DIGEST_LEN)) {
        log_warn(LD_DIR,"Mismatch between identities in certificate and vote");
        goto err;
      }
      if (ns->type != NS_TYPE_CONSENSUS) {
        if (authority_cert_is_denylisted(ns->cert)) {
          log_warn(LD_DIR, "Rejecting vote signature made with denylisted "
                   "signing key %s",
                   hex_str(ns->cert->signing_key_digest, DIGEST_LEN));
          goto err;
        }
      }
      voter->address = tor_strdup(tok->args[2]);
      if (!tor_inet_aton(tok->args[3], &in)) {
        log_warn(LD_DIR, "Error decoding IP address %s in network-status.",
                 escaped(tok->args[3]));
        goto err;
      }
      tor_addr_from_in(&voter->ipv4_addr, &in);
      int ok;
      voter->ipv4_dirport = (uint16_t)
        tor_parse_long(tok->args[4], 10, 0, 65535, &ok, NULL);
      if (!ok)
        goto err;
      voter->ipv4_orport = (uint16_t)
        tor_parse_long(tok->args[5], 10, 0, 65535, &ok, NULL);
      if (!ok)
        goto err;
    } else if (tok->tp == K_CONTACT) {
      if (!voter || voter->contact) {
        log_warn(LD_DIR, "contact element is out of place.");
        goto err;
      }
      voter->contact = tor_strdup(tok->args[0]);
    } else if (tok->tp == K_VOTE_DIGEST) {
      tor_assert(ns->type == NS_TYPE_CONSENSUS);
      tor_assert(tok->n_args >= 1);
      if (!voter || ! tor_digest_is_zero(voter->vote_digest)) {
        log_warn(LD_DIR, "vote-digest element is out of place.");
        goto err;
      }
      if (strlen(tok->args[0]) != HEX_DIGEST_LEN ||
        base16_decode(voter->vote_digest, sizeof(voter->vote_digest),
                      tok->args[0], HEX_DIGEST_LEN)
                      != sizeof(voter->vote_digest)) {
        log_warn(LD_DIR, "Error decoding vote digest %s in "
                 "network-status consensus.", escaped(tok->args[0]));
        goto err;
      }
    }
  } SMARTLIST_FOREACH_END(_tok);
  if (voter) {
    smartlist_add(ns->voters, voter);
    voter = NULL;
  }
  if (smartlist_len(ns->voters) == 0) {
    log_warn(LD_DIR, "Missing dir-source elements in a networkstatus.");
    goto err;
  } else if (ns->type != NS_TYPE_CONSENSUS && smartlist_len(ns->voters) != 1) {
    log_warn(LD_DIR, "Too many dir-source elements in a vote networkstatus.");
    goto err;
  }

  if (ns->type != NS_TYPE_CONSENSUS &&
      (tok = find_opt_by_keyword(tokens, K_LEGACY_DIR_KEY))) {
    int bad = 1;
    if (strlen(tok->args[0]) == HEX_DIGEST_LEN) {
      networkstatus_voter_info_t *voter_0 = smartlist_get(ns->voters, 0);
      if (base16_decode(voter_0->legacy_id_digest, DIGEST_LEN,
                        tok->args[0], HEX_DIGEST_LEN) != DIGEST_LEN)
        bad = 1;
      else
        bad = 0;
    }
    if (bad) {
      log_warn(LD_DIR, "Invalid legacy key digest %s on vote.",
               escaped(tok->args[0]));
    }
  }

  /* If this is a vote document, check if information about the shared
     randomness protocol is included, and extract it. */
  if (ns->type == NS_TYPE_VOTE) {
    dirvote_parse_sr_commits(ns, tokens);
  }
  /* For both a vote and consensus, extract the shared random values. */
  if (ns->type == NS_TYPE_VOTE || ns->type == NS_TYPE_CONSENSUS) {
    extract_shared_random_srvs(ns, tokens);
  }

  /* Parse routerstatus lines. */
  rs_tokens = smartlist_new();
  rs_area = memarea_new();
  s = end_of_header;
  ns->routerstatus_list = smartlist_new();

  while (eos - s >= 2 && fast_memeq(s, "r ", 2)) {
    if (ns->type != NS_TYPE_CONSENSUS) {
      vote_routerstatus_t *rs = tor_malloc_zero(sizeof(vote_routerstatus_t));
      if (routerstatus_parse_entry_from_string(rs_area, &s, eos, rs_tokens, ns,
                                               rs, 0, 0)) {
        smartlist_add(ns->routerstatus_list, rs);
      } else {
        vote_routerstatus_free(rs);
        goto err; // Malformed routerstatus, reject this vote.
      }
    } else {
      routerstatus_t *rs;
      if ((rs = routerstatus_parse_entry_from_string(rs_area, &s, eos,
                                                     rs_tokens,
                                                     NULL, NULL,
                                                     ns->consensus_method,
                                                     flav))) {
        /* Use exponential-backoff scheduling when downloading microdescs */
        smartlist_add(ns->routerstatus_list, rs);
      } else {
        goto err; // Malformed routerstatus, reject this vote.
      }
    }
  }
  for (i = 1; i < smartlist_len(ns->routerstatus_list); ++i) {
    routerstatus_t *rs1, *rs2;
    if (ns->type != NS_TYPE_CONSENSUS) {
      vote_routerstatus_t *a = smartlist_get(ns->routerstatus_list, i-1);
      vote_routerstatus_t *b = smartlist_get(ns->routerstatus_list, i);
      rs1 = &a->status; rs2 = &b->status;
    } else {
      rs1 = smartlist_get(ns->routerstatus_list, i-1);
      rs2 = smartlist_get(ns->routerstatus_list, i);
    }
    if (fast_memcmp(rs1->identity_digest, rs2->identity_digest, DIGEST_LEN)
        >= 0) {
      log_warn(LD_DIR, "Networkstatus entries not sorted by identity digest");
      goto err;
    }
  }
  if (ns_type != NS_TYPE_CONSENSUS) {
    digest256map_t *ed_id_map = digest256map_new();
    SMARTLIST_FOREACH_BEGIN(ns->routerstatus_list, vote_routerstatus_t *,
                            vrs) {
      if (! vrs->has_ed25519_listing ||
          fast_mem_is_zero((const char *)vrs->ed25519_id, DIGEST256_LEN))
        continue;
      if (digest256map_get(ed_id_map, vrs->ed25519_id) != NULL) {
        log_warn(LD_DIR, "Vote networkstatus ed25519 identities were not "
                 "unique");
        digest256map_free(ed_id_map, NULL);
        goto err;
      }
      digest256map_set(ed_id_map, vrs->ed25519_id, (void*)1);
    } SMARTLIST_FOREACH_END(vrs);
    digest256map_free(ed_id_map, NULL);
  }

  /* Parse footer; check signature. */
  footer_tokens = smartlist_new();
  if ((end_of_footer = tor_memstr(s, eos-s, "\nnetwork-status-version ")))
    ++end_of_footer;
  else
    end_of_footer = eos;
  if (tokenize_string(area,s, end_of_footer, footer_tokens,
                      networkstatus_vote_footer_token_table, 0)) {
    log_warn(LD_DIR, "Error tokenizing network-status vote footer.");
    goto err;
  }

  {
    int found_sig = 0;
    SMARTLIST_FOREACH_BEGIN(footer_tokens, directory_token_t *, _tok) {
      tok = _tok;
      if (tok->tp == K_DIRECTORY_SIGNATURE)
        found_sig = 1;
      else if (found_sig) {
        log_warn(LD_DIR, "Extraneous token after first directory-signature");
        goto err;
      }
    } SMARTLIST_FOREACH_END(_tok);
  }

  if ((tok = find_opt_by_keyword(footer_tokens, K_DIRECTORY_FOOTER))) {
    if (tok != smartlist_get(footer_tokens, 0)) {
      log_warn(LD_DIR, "Misplaced directory-footer token");
      goto err;
    }
  }

  tok = find_opt_by_keyword(footer_tokens, K_BW_WEIGHTS);
  if (tok) {
    ns->weight_params = smartlist_new();
    for (i = 0; i < tok->n_args; ++i) {
      int ok=0;
      char *eq = strchr(tok->args[i], '=');
      if (!eq) {
        log_warn(LD_DIR, "Bad element '%s' in weight params",
                 escaped(tok->args[i]));
        goto err;
      }
      tor_parse_long(eq+1, 10, INT32_MIN, INT32_MAX, &ok, NULL);
      if (!ok) {
        log_warn(LD_DIR, "Bad element '%s' in params", escaped(tok->args[i]));
        goto err;
      }
      smartlist_add_strdup(ns->weight_params, tok->args[i]);
    }
  }

  SMARTLIST_FOREACH_BEGIN(footer_tokens, directory_token_t *, _tok) {
    char declared_identity[DIGEST_LEN];
    networkstatus_voter_info_t *v;
    document_signature_t *sig;
    const char *id_hexdigest = NULL;
    const char *sk_hexdigest = NULL;
    digest_algorithm_t alg = DIGEST_SHA1;
    tok = _tok;
    if (tok->tp != K_DIRECTORY_SIGNATURE)
      continue;
    tor_assert(tok->n_args >= 2);
    if (tok->n_args == 2) {
      id_hexdigest = tok->args[0];
      sk_hexdigest = tok->args[1];
    } else {
      const char *algname = tok->args[0];
      int a;
      id_hexdigest = tok->args[1];
      sk_hexdigest = tok->args[2];
      a = crypto_digest_algorithm_parse_name(algname);
      if (a<0) {
        log_warn(LD_DIR, "Unknown digest algorithm %s; skipping",
                 escaped(algname));
        continue;
      }
      alg = a;
    }

    if (!tok->object_type ||
        strcmp(tok->object_type, "SIGNATURE") ||
        tok->object_size < 128 || tok->object_size > 512) {
      log_warn(LD_DIR, "Bad object type or length on directory-signature");
      goto err;
    }

    if (strlen(id_hexdigest) != HEX_DIGEST_LEN ||
        base16_decode(declared_identity, sizeof(declared_identity),
                      id_hexdigest, HEX_DIGEST_LEN)
                      != sizeof(declared_identity)) {
      log_warn(LD_DIR, "Error decoding declared identity %s in "
               "network-status document.", escaped(id_hexdigest));
      goto err;
    }
    if (!(v = networkstatus_get_voter_by_id(ns, declared_identity))) {
      log_warn(LD_DIR, "ID on signature on network-status document does "
               "not match any declared directory source.");
      goto err;
    }
    sig = tor_malloc_zero(sizeof(document_signature_t));
    memcpy(sig->identity_digest, v->identity_digest, DIGEST_LEN);
    sig->alg = alg;
    if (strlen(sk_hexdigest) != HEX_DIGEST_LEN ||
        base16_decode(sig->signing_key_digest, sizeof(sig->signing_key_digest),
                      sk_hexdigest, HEX_DIGEST_LEN)
                      != sizeof(sig->signing_key_digest)) {
      log_warn(LD_DIR, "Error decoding declared signing key digest %s in "
               "network-status document.", escaped(sk_hexdigest));
      tor_free(sig);
      goto err;
    }

    if (ns->type != NS_TYPE_CONSENSUS) {
      if (tor_memneq(declared_identity, ns->cert->cache_info.identity_digest,
                 DIGEST_LEN)) {
        log_warn(LD_DIR, "Digest mismatch between declared and actual on "
                 "network-status vote.");
        tor_free(sig);
        goto err;
      }
    }

    if (networkstatus_get_voter_sig_by_alg(v, sig->alg)) {
      /* We already parsed a vote with this algorithm from this voter. Use the
         first one. */
      log_fn(LOG_PROTOCOL_WARN, LD_DIR, "We received a networkstatus "
             "that contains two signatures from the same voter with the same "
             "algorithm. Ignoring the second signature.");
      tor_free(sig);
      continue;
    }

    if (ns->type != NS_TYPE_CONSENSUS) {
      if (check_signature_token(ns_digests.d[DIGEST_SHA1], DIGEST_LEN,
                                tok, ns->cert->signing_key, 0,
                                "network-status document")) {
        tor_free(sig);
        goto err;
      }
      sig->good_signature = 1;
    } else {
      if (tok->object_size >= INT_MAX || tok->object_size >= SIZE_T_CEILING) {
        tor_free(sig);
        goto err;
      }
      sig->signature = tor_memdup(tok->object_body, tok->object_size);
      sig->signature_len = (int) tok->object_size;
    }
    smartlist_add(v->sigs, sig);

    ++n_signatures;
  } SMARTLIST_FOREACH_END(_tok);

  if (! n_signatures) {
    log_warn(LD_DIR, "No signatures on networkstatus document.");
    goto err;
  } else if (ns->type == NS_TYPE_VOTE && n_signatures != 1) {
    log_warn(LD_DIR, "Received more than one signature on a "
             "network-status vote.");
    goto err;
  }

  if (eos_out)
    *eos_out = end_of_footer;

  goto done;
 err:
  dump_desc(s_dup, "v3 networkstatus");
  networkstatus_vote_free(ns);
  ns = NULL;
 done:
  if (tokens) {
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(tokens);
  }
  if (voter) {
    if (voter->sigs) {
      SMARTLIST_FOREACH(voter->sigs, document_signature_t *, sig,
                        document_signature_free(sig));
      smartlist_free(voter->sigs);
    }
    tor_free(voter->nickname);
    tor_free(voter->address);
    tor_free(voter->contact);
    tor_free(voter);
  }
  if (rs_tokens) {
    SMARTLIST_FOREACH(rs_tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(rs_tokens);
  }
  if (footer_tokens) {
    SMARTLIST_FOREACH(footer_tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(footer_tokens);
  }
  if (area) {
    DUMP_AREA(area, "v3 networkstatus");
    memarea_drop_all(area);
  }
  if (rs_area)
    memarea_drop_all(rs_area);
  tor_free(last_kwd);

  return ns;
}
