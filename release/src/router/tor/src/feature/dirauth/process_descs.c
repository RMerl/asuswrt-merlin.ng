/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file process_descs.c
 * \brief Make decisions about uploaded descriptors
 *
 * Authorities use the code in this module to decide what to do with just-
 * uploaded descriptors, and to manage the fingerprint file that helps
 * them make those decisions.
 **/

#define PROCESS_DESCS_PRIVATE

#include "core/or/or.h"
#include "feature/dirauth/process_descs.h"

#include "app/config/config.h"
#include "core/or/policies.h"
#include "core/or/versions.h"
#include "feature/dirauth/dirauth_sys.h"
#include "feature/dirauth/keypin.h"
#include "feature/dirauth/reachability.h"
#include "feature/dirclient/dlstatus.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/routerlist.h"
#include "feature/dirparse/routerparse.h"
#include "feature/nodelist/torcert.h"
#include "feature/relay/router.h"

#include "core/or/tor_version_st.h"
#include "feature/dirauth/dirauth_options_st.h"
#include "feature/nodelist/extrainfo_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/vote_routerstatus_st.h"

#include "lib/encoding/confline.h"
#include "lib/crypt_ops/crypto_format.h"

/** How far in the future do we allow a router to get? (seconds) */
#define ROUTER_ALLOW_SKEW (60*60*12)

static void directory_remove_invalid(void);
static was_router_added_t dirserv_add_extrainfo(extrainfo_t *ei,
                                                const char **msg);
static uint32_t
dirserv_get_status_impl(const char *id_digest,
                        const ed25519_public_key_t *ed25519_public_key,
                        const char *nickname, const tor_addr_t *ipv4_addr,
                        uint16_t ipv4_orport, const char *platform,
                        const char **msg, int severity);

/** Should be static; exposed for testing. */
static authdir_config_t *fingerprint_list = NULL;

/** Allocate and return a new, empty, authdir_config_t. */
static authdir_config_t *
authdir_config_new(void)
{
  authdir_config_t *list = tor_malloc_zero(sizeof(authdir_config_t));
  list->fp_by_name = strmap_new();
  list->status_by_digest = digestmap_new();
  list->status_by_digest256 = digest256map_new();
  return list;
}

#ifdef TOR_UNIT_TESTS

/** Initialize fingerprint_list to a new authdir_config_t. Used for tests. */
void
authdir_init_fingerprint_list(void)
{
  fingerprint_list = authdir_config_new();
}

/* Return the current fingerprint_list. Used for tests. */
authdir_config_t *
authdir_return_fingerprint_list(void)
{
  return fingerprint_list;
}

#endif /* defined(TOR_UNIT_TESTS) */

/** Add the fingerprint <b>fp</b> to the smartlist of fingerprint_entry_t's
 * <b>list</b>, or-ing the currently set status flags with
 * <b>add_status</b>.
 */
int
add_rsa_fingerprint_to_dir(const char *fp, authdir_config_t *list,
                           rtr_flags_t add_status)
{
  char *fingerprint;
  char d[DIGEST_LEN];
  rtr_flags_t *status;
  tor_assert(fp);
  tor_assert(list);

  fingerprint = tor_strdup(fp);
  tor_strstrip(fingerprint, " ");
  if (base16_decode(d, DIGEST_LEN,
                    fingerprint, strlen(fingerprint)) != DIGEST_LEN) {
    log_warn(LD_DIRSERV, "Couldn't decode fingerprint \"%s\"",
             escaped(fp));
    tor_free(fingerprint);
    return -1;
  }

  status = digestmap_get(list->status_by_digest, d);
  if (!status) {
    status = tor_malloc_zero(sizeof(rtr_flags_t));
    digestmap_set(list->status_by_digest, d, status);
  }

  tor_free(fingerprint);
  *status |= add_status;
  return 0;
}

/** Add the ed25519 key <b>edkey</b> to the smartlist of fingerprint_entry_t's
 * <b>list</b>, or-ing the currently set status flags with <b>add_status</b>.
 * Return -1 if we were unable to decode the key, else return 0.
 */
int
add_ed25519_to_dir(const ed25519_public_key_t *edkey, authdir_config_t *list,
                   rtr_flags_t add_status)
{
  rtr_flags_t *status;

  tor_assert(edkey);
  tor_assert(list);

  if (ed25519_validate_pubkey(edkey) < 0) {
    log_warn(LD_DIRSERV, "Invalid ed25519 key \"%s\"", ed25519_fmt(edkey));
    return -1;
  }

  status = digest256map_get(list->status_by_digest256, edkey->pubkey);
  if (!status) {
    status = tor_malloc_zero(sizeof(rtr_flags_t));
    digest256map_set(list->status_by_digest256, edkey->pubkey, status);
  }

  *status |= add_status;
  return 0;
}

/** Add the fingerprint for this OR to the global list of recognized
 * identity key fingerprints. */
int
dirserv_add_own_fingerprint(crypto_pk_t *pk, const ed25519_public_key_t *edkey)
{
  char fp[FINGERPRINT_LEN+1];
  if (crypto_pk_get_fingerprint(pk, fp, 0)<0) {
    log_err(LD_BUG, "Error computing fingerprint");
    return -1;
  }
  if (!fingerprint_list)
    fingerprint_list = authdir_config_new();
  if (add_rsa_fingerprint_to_dir(fp, fingerprint_list, 0) < 0) {
    log_err(LD_BUG, "Error adding RSA fingerprint");
    return -1;
  }
  if (add_ed25519_to_dir(edkey, fingerprint_list, 0) < 0) {
    log_err(LD_BUG, "Error adding ed25519 key");
    return -1;
  }
  return 0;
}

/** Load the nickname-\>fingerprint mappings stored in the approved-routers
 * file.  The file format is line-based, with each non-blank holding one
 * nickname, some space, and a fingerprint for that nickname.  On success,
 * replace the current fingerprint list with the new list and return 0.  On
 * failure, leave the current fingerprint list untouched, and return -1. */
int
dirserv_load_fingerprint_file(void)
{
  char *fname;
  char *cf;
  char *nickname, *fingerprint;
  authdir_config_t *fingerprint_list_new;
  int result;
  config_line_t *front=NULL, *list;

  fname = get_datadir_fname("approved-routers");
  log_info(LD_GENERAL,
           "Reloading approved fingerprints from \"%s\"...", fname);

  cf = read_file_to_str(fname, RFTS_IGNORE_MISSING, NULL);
  if (!cf) {
    log_warn(LD_FS, "Cannot open fingerprint file '%s'. That's ok.", fname);
    tor_free(fname);
    return 0;
  }
  tor_free(fname);

  result = config_get_lines(cf, &front, 0);
  tor_free(cf);
  if (result < 0) {
    log_warn(LD_CONFIG, "Error reading from fingerprint file");
    return -1;
  }

  fingerprint_list_new = authdir_config_new();

  for (list=front; list; list=list->next) {
    rtr_flags_t add_status = 0;
    nickname = list->key; fingerprint = list->value;
    tor_strstrip(fingerprint, " "); /* remove spaces */

   /* Determine what we should do with the relay with the nickname field. */
    if (!strcasecmp(nickname, "!reject")) {
        add_status = RTR_REJECT;
    } else if (!strcasecmp(nickname, "!badexit")) {
        add_status = RTR_BADEXIT;
    } else if (!strcasecmp(nickname, "!invalid")) {
        add_status = RTR_INVALID;
    }  else if (!strcasecmp(nickname, "!middleonly")) {
        add_status = RTR_MIDDLEONLY;
    }

    /* Check if fingerprint is RSA or ed25519 by verifying it. */
    int ed25519_not_ok = -1, rsa_not_ok = -1;

    /* Attempt to add the RSA key. */
    if (strlen(fingerprint) == HEX_DIGEST_LEN) {
      rsa_not_ok = add_rsa_fingerprint_to_dir(fingerprint,
                                              fingerprint_list_new,
                                              add_status);
    }

    /* Check ed25519 key. We check the size to prevent buffer overflows.
     * If valid, attempt to add it, */
    ed25519_public_key_t ed25519_pubkey_tmp;
    if (strlen(fingerprint) == BASE64_DIGEST256_LEN) {
      if (!digest256_from_base64((char *) ed25519_pubkey_tmp.pubkey,
                                 fingerprint)) {
        ed25519_not_ok = add_ed25519_to_dir(&ed25519_pubkey_tmp,
                                            fingerprint_list_new, add_status);
      }
    }

    /* If both keys are invalid (or missing), log and skip. */
    if (ed25519_not_ok && rsa_not_ok) {
      log_warn(LD_CONFIG, "Invalid fingerprint (nickname '%s', "
               "fingerprint %s). Skipping.", nickname, fingerprint);
      continue;
    }
  }

  config_free_lines(front);
  dirserv_free_fingerprint_list();
  fingerprint_list = fingerprint_list_new;
  /* Delete any routers whose fingerprints we no longer recognize */
  directory_remove_invalid();
  return 0;
}

/* If this is set, then we don't allow routers that have advertised an Ed25519
 * identity to stop doing so.  This is going to be essential for good identity
 * security: otherwise anybody who can attack RSA-1024 but not Ed25519 could
 * just sign fake descriptors missing the Ed25519 key.  But we won't actually
 * be able to prevent that kind of thing until we're confident that there isn't
 * actually a legit reason to downgrade to 0.2.5.  Now we are not recommending
 * 0.2.5 anymore so there is no reason to keep the #undef.
 */

#define DISABLE_DISABLING_ED25519

/** Check whether <b>router</b> has:
 * - a nickname/identity key combination that we recognize from the fingerprint
 *   list,
 * - an IP we automatically act on according to our configuration,
 * - an appropriate version, and
 * - matching pinned keys.
 *
 * Return the appropriate router status.
 *
 * If the status is 'RTR_REJECT' and <b>msg</b> is provided, set
 * *<b>msg</b> to a string constant explaining why. */
uint32_t
dirserv_router_get_status(const routerinfo_t *router, const char **msg,
                          int severity)
{
  char d[DIGEST_LEN];
  const int key_pinning = dirauth_get_options()->AuthDirPinKeys;
  uint32_t r;
  ed25519_public_key_t *signing_key = NULL;

  if (crypto_pk_get_digest(router->identity_pkey, d)) {
    log_warn(LD_BUG,"Error computing fingerprint");
    if (msg)
      *msg = "Bug: Error computing fingerprint";
    return RTR_REJECT;
  }

  /* First, check for the more common reasons to reject a router. */
  if (router->cache_info.signing_key_cert) {
    /* This has an ed25519 identity key. */
    signing_key = &router->cache_info.signing_key_cert->signing_key;
  }
  r = dirserv_get_status_impl(d, signing_key, router->nickname,
                              &router->ipv4_addr, router->ipv4_orport,
                              router->platform, msg, severity);

  if (r)
    return r;

  /* dirserv_get_status_impl already rejects versions older than 0.2.4.18-rc,
   * and onion_curve25519_pkey was introduced in 0.2.4.8-alpha.
   * But just in case a relay doesn't provide or lies about its version, or
   * doesn't include an ntor key in its descriptor, check that it exists,
   * and is non-zero (clients check that it's non-zero before using it). */
  if (!routerinfo_has_curve25519_onion_key(router)) {
    log_fn(severity, LD_DIR,
           "Descriptor from router %s (platform %s) "
           "is missing an ntor curve25519 onion key.",
           router_describe(router), router->platform);
    if (msg)
      *msg = "Missing ntor curve25519 onion key. Please upgrade!";
    return RTR_REJECT;
  }

  if (router->cache_info.signing_key_cert) {
    /* This has an ed25519 identity key. */
    if (KEYPIN_MISMATCH ==
        keypin_check((const uint8_t*)router->cache_info.identity_digest,
                   router->cache_info.signing_key_cert->signing_key.pubkey)) {
      log_fn(severity, LD_DIR,
             "Descriptor from router %s has an Ed25519 key, "
               "but the <rsa,ed25519> keys don't match what they were before.",
               router_describe(router));
      if (key_pinning) {
        if (msg) {
          *msg = "Ed25519 identity key or RSA identity key has changed.";
        }
        return RTR_REJECT;
      }
    }
  } else {
    /* No ed25519 key */
    if (KEYPIN_MISMATCH == keypin_check_lone_rsa(
                        (const uint8_t*)router->cache_info.identity_digest)) {
      log_fn(severity, LD_DIR,
               "Descriptor from router %s has no Ed25519 key, "
               "when we previously knew an Ed25519 for it. Ignoring for now, "
               "since Ed25519 keys are fairly new.",
               router_describe(router));
#ifdef DISABLE_DISABLING_ED25519
      if (key_pinning) {
        if (msg) {
          *msg = "Ed25519 identity key has disappeared.";
        }
        return RTR_REJECT;
      }
#endif /* defined(DISABLE_DISABLING_ED25519) */
    }
  }

  return 0;
}

/** Return true if there is no point in downloading the router described by
 * <b>rs</b> because this directory would reject it. */
int
dirserv_would_reject_router(const routerstatus_t *rs,
                            const vote_routerstatus_t *vrs)
{
  uint32_t res;
  struct ed25519_public_key_t pk;
  memcpy(&pk.pubkey, vrs->ed25519_id, ED25519_PUBKEY_LEN);

  res = dirserv_get_status_impl(rs->identity_digest, &pk, rs->nickname,
                                &rs->ipv4_addr, rs->ipv4_orport, NULL, NULL,
                                LOG_DEBUG);

  return (res & RTR_REJECT) != 0;
}

/**
 * Check whether the platform string in <b>platform</b> describes a platform
 * that, as a directory authority, we want to reject.  If it does, return
 * true, and set *<b>msg</b> (if present) to a rejection message.  Otherwise
 * return false.
 */
STATIC bool
dirserv_rejects_tor_version(const char *platform,
                            const char **msg)
{
  if (!platform)
    return false;

  static const char please_upgrade_string[] =
    "Tor version is insecure or unsupported. Please upgrade!";

  /* Anything before 0.4.5.6 is unsupported. Reject them. */
  if (!tor_version_as_new_as(platform,"0.4.5.6")) {
    if (msg) {
      *msg = please_upgrade_string;
    }
    return true;
  }

  return false;
}

/** Helper: As dirserv_router_get_status, but takes the router fingerprint
 * (hex, no spaces), ed25519 key, nickname, address (used for logging only),
 * IP address, OR port and platform (logging only) as arguments.
 *
 * Log messages at 'severity'. (There's not much point in
 * logging that we're rejecting servers we'll not download.)
 */
static uint32_t
dirserv_get_status_impl(const char *id_digest,
                        const ed25519_public_key_t *ed25519_public_key,
                        const char *nickname, const tor_addr_t *ipv4_addr,
                        uint16_t ipv4_orport, const char *platform,
                        const char **msg, int severity)
{
  uint32_t result = 0;
  rtr_flags_t *status_by_digest;

  if (!fingerprint_list)
    fingerprint_list = authdir_config_new();

  log_debug(LD_DIRSERV, "%d fingerprints, %d digests known.",
            strmap_size(fingerprint_list->fp_by_name),
            digestmap_size(fingerprint_list->status_by_digest));

  if (platform) {
    tor_version_t ver_tmp;
    if (tor_version_parse_platform(platform, &ver_tmp, 1) < 0) {
      if (msg) {
        *msg = "Malformed platform string.";
      }
      return RTR_REJECT;
    }
  }

  /* Check whether the version is obsolete, broken, insecure, etc... */
  if (platform && dirserv_rejects_tor_version(platform, msg)) {
    return RTR_REJECT;
  }

  status_by_digest = digestmap_get(fingerprint_list->status_by_digest,
                                   id_digest);
  if (status_by_digest)
    result |= *status_by_digest;

  if (ed25519_public_key) {
    status_by_digest = digest256map_get(fingerprint_list->status_by_digest256,
                                        ed25519_public_key->pubkey);
    if (status_by_digest)
      result |= *status_by_digest;
  }

  if (result & RTR_REJECT) {
    if (msg)
      *msg = "Fingerprint and/or ed25519 identity is marked rejected -- if "
             "you think this is a mistake please set a valid email address "
             "in ContactInfo and send an email to "
             "bad-relays@lists.torproject.org mentioning your fingerprint(s)?";
    return RTR_REJECT;
  } else if (result & RTR_INVALID) {
    if (msg)
      *msg = "Fingerprint and/or ed25519 identity is marked invalid";
  }

  if (authdir_policy_badexit_address(ipv4_addr, ipv4_orport)) {
    log_fn(severity, LD_DIRSERV,
           "Marking '%s' as bad exit because of address '%s'",
               nickname, fmt_addr(ipv4_addr));
    result |= RTR_BADEXIT;
  }

  if (authdir_policy_middleonly_address(ipv4_addr, ipv4_orport)) {
    log_fn(severity, LD_DIRSERV,
           "Marking '%s' as middle-only because of address '%s'",
               nickname, fmt_addr(ipv4_addr));
    result |= RTR_MIDDLEONLY;
  }

  if (!authdir_policy_permits_address(ipv4_addr, ipv4_orport)) {
    log_fn(severity, LD_DIRSERV, "Rejecting '%s' because of address '%s'",
               nickname, fmt_addr(ipv4_addr));
    if (msg)
      *msg = "Suspicious relay address range -- if you think this is a "
             "mistake please set a valid email address in ContactInfo and "
             "send an email to bad-relays@lists.torproject.org mentioning "
             "your address(es) and fingerprint(s)?";
    return RTR_REJECT;
  }
  if (!authdir_policy_valid_address(ipv4_addr, ipv4_orport)) {
    log_fn(severity, LD_DIRSERV,
           "Not marking '%s' valid because of address '%s'",
               nickname, fmt_addr(ipv4_addr));
    result |= RTR_INVALID;
  }

  return result;
}

/** Clear the current fingerprint list. */
void
dirserv_free_fingerprint_list(void)
{
  if (!fingerprint_list)
    return;

  strmap_free(fingerprint_list->fp_by_name, tor_free_);
  digestmap_free(fingerprint_list->status_by_digest, tor_free_);
  digest256map_free(fingerprint_list->status_by_digest256, tor_free_);
  tor_free(fingerprint_list);
}

/*
 *    Descriptor list
 */

/** Return -1 if <b>ri</b> has a private or otherwise bad address,
 * unless we're configured to not care. Return 0 if all ok. */
STATIC int
dirserv_router_has_valid_address(routerinfo_t *ri)
{
  if (get_options()->DirAllowPrivateAddresses)
    return 0; /* whatever it is, we're fine with it */

  if (tor_addr_is_null(&ri->ipv4_addr) ||
      tor_addr_is_internal(&ri->ipv4_addr, 0)) {
    log_info(LD_DIRSERV,
             "Router %s published internal IPv4 address. Refusing.",
             router_describe(ri));
    return -1; /* it's a private IP, we should reject it */
  }

  /* We only check internal v6 on non-null addresses because we do not require
   * IPv6 and null IPv6 is normal. */
  if (!tor_addr_is_null(&ri->ipv6_addr) &&
      tor_addr_is_internal(&ri->ipv6_addr, 0)) {
    log_info(LD_DIRSERV,
             "Router %s published internal IPv6 address. Refusing.",
             router_describe(ri));
    return -1; /* it's a private IP, we should reject it */
  }

  return 0;
}

/** Check whether we, as a directory server, want to accept <b>ri</b>.  If so,
 * set its is_valid,running fields and return 0.  Otherwise, return -1.
 *
 * If the router is rejected, set *<b>msg</b> to a string constant explining
 * why.
 *
 * If <b>complain</b> then explain at log-level 'notice' why we refused
 * a descriptor; else explain at log-level 'info'.
 */
int
authdir_wants_to_reject_router(routerinfo_t *ri, const char **msg,
                               int complain, int *valid_out)
{
  /* Okay.  Now check whether the fingerprint is recognized. */
  time_t now;
  int severity = (complain && ri->contact_info) ? LOG_NOTICE : LOG_INFO;
  uint32_t status = dirserv_router_get_status(ri, msg, severity);
  tor_assert(msg);
  if (status & RTR_REJECT)
    return -1; /* msg is already set. */

  /* Is there too much clock skew? */
  now = time(NULL);
  if (ri->cache_info.published_on > now+ROUTER_ALLOW_SKEW) {
    log_fn(severity, LD_DIRSERV, "Publication time for %s is too "
           "far (%d minutes) in the future; possible clock skew. Not adding "
           "(%s)",
           router_describe(ri),
           (int)((ri->cache_info.published_on-now)/60),
           esc_router_info(ri));
    *msg = "Rejected: Your clock is set too far in the future, or your "
      "timezone is not correct.";
    return -1;
  }
  if (ri->cache_info.published_on < now-ROUTER_MAX_AGE_TO_PUBLISH) {
    log_fn(severity, LD_DIRSERV,
           "Publication time for %s is too far "
           "(%d minutes) in the past. Not adding (%s)",
           router_describe(ri),
           (int)((now-ri->cache_info.published_on)/60),
           esc_router_info(ri));
    *msg = "Rejected: Server is expired, or your clock is too far in the past,"
      " or your timezone is not correct.";
    return -1;
  }
  if (dirserv_router_has_valid_address(ri) < 0) {
    log_fn(severity, LD_DIRSERV,
           "Router %s has invalid address. Not adding (%s).",
           router_describe(ri),
           esc_router_info(ri));
    *msg = "Rejected: Address is a private address.";
    return -1;
  }

  *valid_out = ! (status & RTR_INVALID);

  return 0;
}

/** Update the relevant flags of <b>node</b> based on our opinion as a
 * directory authority in <b>authstatus</b>, as returned by
 * dirserv_router_get_status or equivalent.  */
void
dirserv_set_node_flags_from_authoritative_status(node_t *node,
                                                 uint32_t authstatus)
{
  node->is_valid = (authstatus & RTR_INVALID) ? 0 : 1;
  node->is_bad_exit = (authstatus & RTR_BADEXIT) ? 1 : 0;
  node->is_middle_only = (authstatus & RTR_MIDDLEONLY) ? 1 : 0;
}

/** True iff <b>a</b> is more severe than <b>b</b>. */
static int
WRA_MORE_SEVERE(was_router_added_t a, was_router_added_t b)
{
  return a < b;
}

/** As for dirserv_add_descriptor(), but accepts multiple documents, and
 * returns the most severe error that occurred for any one of them. */
was_router_added_t
dirserv_add_multiple_descriptors(const char *desc, size_t desclen,
                                 uint8_t purpose,
                                 const char *source,
                                 const char **msg)
{
  was_router_added_t r, r_tmp;
  const char *msg_out;
  smartlist_t *list;
  const char *s;
  int n_parsed = 0;
  time_t now = time(NULL);
  char annotation_buf[ROUTER_ANNOTATION_BUF_LEN];
  char time_buf[ISO_TIME_LEN+1];
  int general = purpose == ROUTER_PURPOSE_GENERAL;
  tor_assert(msg);

  r=ROUTER_ADDED_SUCCESSFULLY; /* Least severe return value. */

  if (!string_is_utf8_no_bom(desc, desclen)) {
    *msg = "descriptor(s) or extrainfo(s) not valid UTF-8 or had BOM.";
    return ROUTER_AUTHDIR_REJECTS;
  }

  format_iso_time(time_buf, now);
  if (tor_snprintf(annotation_buf, sizeof(annotation_buf),
                   "@uploaded-at %s\n"
                   "@source %s\n"
                   "%s%s%s", time_buf, escaped(source),
                   !general ? "@purpose " : "",
                   !general ? router_purpose_to_string(purpose) : "",
                   !general ? "\n" : "")<0) {
    *msg = "Couldn't format annotations";
    return ROUTER_AUTHDIR_BUG_ANNOTATIONS;
  }

  s = desc;
  list = smartlist_new();
  if (!router_parse_list_from_string(&s, s+desclen, list, SAVED_NOWHERE, 0, 0,
                                     annotation_buf, NULL)) {
    SMARTLIST_FOREACH(list, routerinfo_t *, ri, {
        msg_out = NULL;
        tor_assert(ri->purpose == purpose);
        r_tmp = dirserv_add_descriptor(ri, &msg_out, source);
        if (WRA_MORE_SEVERE(r_tmp, r)) {
          r = r_tmp;
          *msg = msg_out;
        }
      });
  }
  n_parsed += smartlist_len(list);
  smartlist_clear(list);

  s = desc;
  if (!router_parse_list_from_string(&s, s+desclen, list, SAVED_NOWHERE, 1, 0,
                                     NULL, NULL)) {
    SMARTLIST_FOREACH(list, extrainfo_t *, ei, {
        msg_out = NULL;

        r_tmp = dirserv_add_extrainfo(ei, &msg_out);
        if (WRA_MORE_SEVERE(r_tmp, r)) {
          r = r_tmp;
          *msg = msg_out;
        }
      });
  }
  n_parsed += smartlist_len(list);
  smartlist_free(list);

  if (! *msg) {
    if (!n_parsed) {
      *msg = "No descriptors found in your POST.";
      if (WRA_WAS_ADDED(r))
        r = ROUTER_IS_ALREADY_KNOWN;
    } else {
      *msg = "(no message)";
    }
  }

  return r;
}

/** Examine the parsed server descriptor in <b>ri</b> and maybe insert it into
 * the list of server descriptors. Set *<b>msg</b> to a message that should be
 * passed back to the origin of this descriptor, or NULL if there is no such
 * message. Use <b>source</b> to produce better log messages.
 *
 * If <b>ri</b> is not added to the list of server descriptors, free it.
 * That means the caller must not access <b>ri</b> after this function
 * returns, since it might have been freed.
 *
 * Return the status of the operation, and set *<b>msg</b> to a string
 * constant describing the status.
 *
 * This function is only called when fresh descriptors are posted, not when
 * we re-load the cache.
 */
was_router_added_t
dirserv_add_descriptor(routerinfo_t *ri, const char **msg, const char *source)
{
  was_router_added_t r;
  routerinfo_t *ri_old;
  char *desc, *nickname;
  const size_t desclen = ri->cache_info.signed_descriptor_len +
      ri->cache_info.annotations_len;
  const int key_pinning = dirauth_get_options()->AuthDirPinKeys;
  *msg = NULL;

  /* If it's too big, refuse it now. Otherwise we'll cache it all over the
   * network and it'll clog everything up. */
  if (ri->cache_info.signed_descriptor_len > MAX_DESCRIPTOR_UPLOAD_SIZE) {
    log_notice(LD_DIR, "Somebody attempted to publish a router descriptor '%s'"
               " (source: %s) with size %d. Either this is an attack, or the "
               "MAX_DESCRIPTOR_UPLOAD_SIZE (%d) constant is too low.",
               ri->nickname, source, (int)ri->cache_info.signed_descriptor_len,
               MAX_DESCRIPTOR_UPLOAD_SIZE);
    *msg = "Router descriptor was too large.";
    r = ROUTER_AUTHDIR_REJECTS;
    goto fail;
  }

  log_info(LD_DIR, "Assessing new descriptor: %s: %s",
           ri->nickname, ri->platform);

  /* Check whether this descriptor is semantically identical to the last one
   * from this server.  (We do this here and not in router_add_to_routerlist
   * because we want to be able to accept the newest router descriptor that
   * another authority has, so we all converge on the same one.) */
  ri_old = router_get_mutable_by_digest(ri->cache_info.identity_digest);
  if (ri_old && ri_old->cache_info.published_on < ri->cache_info.published_on
      && router_differences_are_cosmetic(ri_old, ri)
      && !router_is_me(ri)) {
    log_info(LD_DIRSERV,
             "Not replacing descriptor from %s (source: %s); "
             "differences are cosmetic.",
             router_describe(ri), source);
    *msg = "Not replacing router descriptor; no information has changed since "
      "the last one with this identity.";
    r = ROUTER_IS_ALREADY_KNOWN;
    goto fail;
  }

  /* Do keypinning again ... this time, to add the pin if appropriate */
  int keypin_status;
  if (ri->cache_info.signing_key_cert) {
    ed25519_public_key_t *pkey = &ri->cache_info.signing_key_cert->signing_key;
    /* First let's validate this pubkey before pinning it */
    if (ed25519_validate_pubkey(pkey) < 0) {
      log_warn(LD_DIRSERV, "Received bad key from %s (source %s)",
               router_describe(ri), source);
      routerinfo_free(ri);
      return ROUTER_AUTHDIR_REJECTS;
    }

    /* Now pin it! */
    keypin_status = keypin_check_and_add(
      (const uint8_t*)ri->cache_info.identity_digest,
      pkey->pubkey, ! key_pinning);
  } else {
    keypin_status = keypin_check_lone_rsa(
      (const uint8_t*)ri->cache_info.identity_digest);
#ifndef DISABLE_DISABLING_ED25519
    if (keypin_status == KEYPIN_MISMATCH)
      keypin_status = KEYPIN_NOT_FOUND;
#endif
  }
  if (keypin_status == KEYPIN_MISMATCH && key_pinning) {
    log_info(LD_DIRSERV, "Dropping descriptor from %s (source: %s) because "
             "its key did not match an older RSA/Ed25519 keypair",
             router_describe(ri), source);
    *msg = "Looks like your keypair has changed? This authority previously "
      "recorded a different RSA identity for this Ed25519 identity (or vice "
      "versa.) Did you replace or copy some of your key files, but not "
      "the others? You should either restore the expected keypair, or "
      "delete your keys and restart Tor to start your relay with a new "
      "identity.";
    r = ROUTER_AUTHDIR_REJECTS;
    goto fail;
  }

  /* Make a copy of desc, since router_add_to_routerlist might free
   * ri and its associated signed_descriptor_t. */
  desc = tor_strndup(ri->cache_info.signed_descriptor_body, desclen);
  nickname = tor_strdup(ri->nickname);

  /* Tell if we're about to need to launch a test if we add this. */
  ri->needs_retest_if_added =
    dirserv_should_launch_reachability_test(ri, ri_old);

  r = router_add_to_routerlist(ri, msg, 0, 0);
  if (!WRA_WAS_ADDED(r)) {
    /* unless the routerinfo was fine, just out-of-date */
    log_info(LD_DIRSERV,
             "Did not add descriptor from '%s' (source: %s): %s.",
             nickname, source, *msg ? *msg : "(no message)");
  } else {
    smartlist_t *changed;

    changed = smartlist_new();
    smartlist_add(changed, ri);
    routerlist_descriptors_added(changed, 0);
    smartlist_free(changed);
    if (!*msg) {
      *msg =  "Descriptor accepted";
    }
    log_info(LD_DIRSERV,
             "Added descriptor from '%s' (source: %s): %s.",
             nickname, source, *msg);
  }
  tor_free(desc);
  tor_free(nickname);
  return r;
 fail:
  {
    const char *desc_digest = ri->cache_info.signed_descriptor_digest;
    download_status_t *dls =
      router_get_dl_status_by_descriptor_digest(desc_digest);
    if (dls) {
      log_info(LD_GENERAL, "Marking router with descriptor %s as rejected, "
               "and therefore undownloadable",
               hex_str(desc_digest, DIGEST_LEN));
      download_status_mark_impossible(dls);
    }
    routerinfo_free(ri);
  }
  return r;
}

/** As dirserv_add_descriptor, but for an extrainfo_t <b>ei</b>. */
static was_router_added_t
dirserv_add_extrainfo(extrainfo_t *ei, const char **msg)
{
  routerinfo_t *ri;
  int r;
  was_router_added_t rv;
  tor_assert(msg);
  *msg = NULL;

  /* Needs to be mutable so routerinfo_incompatible_with_extrainfo
   * can mess with some of the flags in ri->cache_info. */
  ri = router_get_mutable_by_digest(ei->cache_info.identity_digest);
  if (!ri) {
    *msg = "No corresponding router descriptor for extra-info descriptor";
    rv = ROUTER_BAD_EI;
    goto fail;
  }

  /* If it's too big, refuse it now. Otherwise we'll cache it all over the
   * network and it'll clog everything up. */
  if (ei->cache_info.signed_descriptor_len > MAX_EXTRAINFO_UPLOAD_SIZE) {
    log_notice(LD_DIR, "Somebody attempted to publish an extrainfo "
               "with size %d. Either this is an attack, or the "
               "MAX_EXTRAINFO_UPLOAD_SIZE (%d) constant is too low.",
               (int)ei->cache_info.signed_descriptor_len,
               MAX_EXTRAINFO_UPLOAD_SIZE);
    *msg = "Extrainfo document was too large";
    rv = ROUTER_BAD_EI;
    goto fail;
  }

  if ((r = routerinfo_incompatible_with_extrainfo(ri->identity_pkey, ei,
                                                  &ri->cache_info, msg))) {
    if (r<0) {
      extrainfo_free(ei);
      return ROUTER_IS_ALREADY_KNOWN;
    }
    rv = ROUTER_BAD_EI;
    goto fail;
  }
  router_add_extrainfo_to_routerlist(ei, msg, 0, 0);
  return ROUTER_ADDED_SUCCESSFULLY;
 fail:
  {
    const char *d = ei->cache_info.signed_descriptor_digest;
    signed_descriptor_t *sd = router_get_by_extrainfo_digest((char*)d);
    if (sd) {
      log_info(LD_GENERAL, "Marking extrainfo with descriptor %s as "
               "rejected, and therefore undownloadable",
               hex_str((char*)d,DIGEST_LEN));
      download_status_mark_impossible(&sd->ei_dl_status);
    }
    extrainfo_free(ei);
  }
  return rv;
}

/** Remove all descriptors whose nicknames or fingerprints no longer
 * are allowed by our fingerprint list. (Descriptors that used to be
 * good can become bad when we reload the fingerprint list.)
 */
static void
directory_remove_invalid(void)
{
  routerlist_t *rl = router_get_routerlist();
  smartlist_t *nodes = smartlist_new();
  smartlist_add_all(nodes, nodelist_get_list());

  SMARTLIST_FOREACH_BEGIN(nodes, node_t *, node) {
    const char *msg = NULL;
    const char *description;
    routerinfo_t *ent = node->ri;
    uint32_t r;
    if (!ent)
      continue;
    r = dirserv_router_get_status(ent, &msg, LOG_INFO);
    description = router_describe(ent);
    if (r & RTR_REJECT) {
      log_info(LD_DIRSERV, "Router %s is now rejected: %s",
               description, msg?msg:"");
      routerlist_remove(rl, ent, 0, time(NULL));
      continue;
    }
    if (bool_neq((r & RTR_INVALID), !node->is_valid)) {
      log_info(LD_DIRSERV, "Router '%s' is now %svalid.", description,
               (r&RTR_INVALID) ? "in" : "");
      node->is_valid = (r&RTR_INVALID)?0:1;
    }
    if (bool_neq((r & RTR_BADEXIT), node->is_bad_exit)) {
      log_info(LD_DIRSERV, "Router '%s' is now a %s exit", description,
               (r & RTR_BADEXIT) ? "bad" : "good");
      node->is_bad_exit = (r&RTR_BADEXIT) ? 1: 0;
    }
    if (bool_neq((r & RTR_MIDDLEONLY), node->is_middle_only)) {
      log_info(LD_DIRSERV, "Router '%s' is now %smiddle-only", description,
               (r & RTR_MIDDLEONLY) ? "" : "not");
      node->is_middle_only = (r&RTR_MIDDLEONLY) ? 1: 0;
    }
  } SMARTLIST_FOREACH_END(node);

  routerlist_assert_ok(rl);
  smartlist_free(nodes);
}
