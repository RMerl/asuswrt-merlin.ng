/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dirclient.c
 * @brief Download directory information
 **/

#define DIRCLIENT_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/connection_edge.h"
#include "core/or/policies.h"
#include "feature/client/bridges.h"
#include "feature/client/entrynodes.h"
#include "feature/control/control_events.h"
#include "feature/dirauth/authmode.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirauth/shared_random.h"
#include "feature/dircache/dirserv.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirclient/dirclient_modes.h"
#include "feature/dirclient/dlstatus.h"
#include "feature/dircommon/consdiff.h"
#include "feature/dircommon/directory.h"
#include "feature/dircommon/fp_pair.h"
#include "feature/hs/hs_cache.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_control.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/node_select.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/relay/relay_find_addr.h"
#include "feature/relay/routermode.h"
#include "feature/relay/selftest.h"
#include "feature/rend/rendcommon.h"
#include "feature/stats/predict_ports.h"

#include "lib/cc/ctassert.h"
#include "lib/compress/compress.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/encoding/confline.h"
#include "lib/err/backtrace.h"

#include "core/or/entry_connection_st.h"
#include "feature/dircache/cached_dir_st.h"
#include "feature/dirclient/dir_server_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"

/** Maximum size, in bytes, for any directory object that we've downloaded. */
#define MAX_DIR_DL_SIZE ((1<<24)-1) /* 16 MB - 1 */

/** How far in the future do we allow a directory server to tell us it is
 * before deciding that one of us has the wrong time? */
#define ALLOW_DIRECTORY_TIME_SKEW (30*60)

static int body_is_plausible(const char *body, size_t body_len, int purpose);
static void connection_dir_download_routerdesc_failed(dir_connection_t *conn);
static void connection_dir_bridge_routerdesc_failed(dir_connection_t *conn);
static void connection_dir_download_cert_failed(
                               dir_connection_t *conn, int status_code);
static void connection_dir_retry_bridges(smartlist_t *descs);
static void dir_routerdesc_download_failed(smartlist_t *failed,
                                           int status_code,
                                           int router_purpose,
                                           int was_extrainfo,
                                           int was_descriptor_digests);
static void dir_microdesc_download_failed(smartlist_t *failed,
                                          int status_code,
                                          const char *dir_id);
static void directory_send_command(dir_connection_t *conn,
                                   const int direct,
                                   const directory_request_t *req);
static void connection_dir_close_consensus_fetches(
                   dir_connection_t *except_this_one, const char *resource);

/** Return a string describing a given directory connection purpose. */
STATIC const char *
dir_conn_purpose_to_string(int purpose)
{
  switch (purpose)
    {
    case DIR_PURPOSE_UPLOAD_DIR:
      return "server descriptor upload";
    case DIR_PURPOSE_UPLOAD_VOTE:
      return "server vote upload";
    case DIR_PURPOSE_UPLOAD_SIGNATURES:
      return "consensus signature upload";
    case DIR_PURPOSE_FETCH_SERVERDESC:
      return "server descriptor fetch";
    case DIR_PURPOSE_FETCH_EXTRAINFO:
      return "extra-info fetch";
    case DIR_PURPOSE_FETCH_CONSENSUS:
      return "consensus network-status fetch";
    case DIR_PURPOSE_FETCH_CERTIFICATE:
      return "authority cert fetch";
    case DIR_PURPOSE_FETCH_STATUS_VOTE:
      return "status vote fetch";
    case DIR_PURPOSE_FETCH_DETACHED_SIGNATURES:
      return "consensus signature fetch";
    case DIR_PURPOSE_FETCH_HSDESC:
      return "hidden-service descriptor fetch";
    case DIR_PURPOSE_UPLOAD_HSDESC:
      return "hidden-service descriptor upload";
    case DIR_PURPOSE_FETCH_MICRODESC:
      return "microdescriptor fetch";
    }

  log_warn(LD_BUG, "Called with unknown purpose %d", purpose);
  return "(unknown)";
}

/** Return the requisite directory information types. */
STATIC dirinfo_type_t
dir_fetch_type(int dir_purpose, int router_purpose, const char *resource)
{
  dirinfo_type_t type;
  switch (dir_purpose) {
    case DIR_PURPOSE_FETCH_EXTRAINFO:
      type = EXTRAINFO_DIRINFO;
      if (router_purpose == ROUTER_PURPOSE_BRIDGE)
        type |= BRIDGE_DIRINFO;
      else
        type |= V3_DIRINFO;
      break;
    case DIR_PURPOSE_FETCH_SERVERDESC:
      if (router_purpose == ROUTER_PURPOSE_BRIDGE)
        type = BRIDGE_DIRINFO;
      else
        type = V3_DIRINFO;
      break;
    case DIR_PURPOSE_FETCH_STATUS_VOTE:
    case DIR_PURPOSE_FETCH_DETACHED_SIGNATURES:
    case DIR_PURPOSE_FETCH_CERTIFICATE:
      type = V3_DIRINFO;
      break;
    case DIR_PURPOSE_FETCH_CONSENSUS:
      type = V3_DIRINFO;
      if (resource && !strcmp(resource, "microdesc"))
        type |= MICRODESC_DIRINFO;
      break;
    case DIR_PURPOSE_FETCH_MICRODESC:
      type = MICRODESC_DIRINFO;
      break;
    default:
      log_warn(LD_BUG, "Unexpected purpose %d", (int)dir_purpose);
      type = NO_DIRINFO;
      break;
  }
  return type;
}

/** Return true iff <b>identity_digest</b> is the digest of a router which
 * says that it caches extrainfos.  (If <b>is_authority</b> we always
 * believe that to be true.) */
int
router_supports_extrainfo(const char *identity_digest, int is_authority)
{
  const node_t *node = node_get_by_id(identity_digest);

  if (node && node->ri) {
    if (node->ri->caches_extra_info)
      return 1;
  }
  if (is_authority) {
    return 1;
  }
  return 0;
}

/** Return true iff any trusted directory authority has accepted our
 * server descriptor.
 *
 * We consider any authority sufficient because waiting for all of
 * them means it never happens while any authority is down; we don't
 * go for something more complex in the middle (like \>1/3 or \>1/2 or
 * \>=1/2) because that doesn't seem necessary yet.
 */
int
directories_have_accepted_server_descriptor(void)
{
  const smartlist_t *servers = router_get_trusted_dir_servers();
  const or_options_t *options = get_options();
  SMARTLIST_FOREACH(servers, dir_server_t *, d, {
    if ((d->type & options->PublishServerDescriptor_) &&
        d->has_accepted_serverdesc) {
      return 1;
    }
  });
  return 0;
}

/** Start a connection to every suitable directory authority, using
 * connection purpose <b>dir_purpose</b> and uploading <b>payload</b>
 * (of length <b>payload_len</b>). The dir_purpose should be one of
 * 'DIR_PURPOSE_UPLOAD_{DIR|VOTE|SIGNATURES}'.
 *
 * <b>router_purpose</b> describes the type of descriptor we're
 * publishing, if we're publishing a descriptor -- e.g. general or bridge.
 *
 * <b>type</b> specifies what sort of dir authorities (V3,
 * BRIDGE, etc) we should upload to.
 *
 * If <b>extrainfo_len</b> is nonzero, the first <b>payload_len</b> bytes of
 * <b>payload</b> hold a router descriptor, and the next <b>extrainfo_len</b>
 * bytes of <b>payload</b> hold an extra-info document.  Upload the descriptor
 * to all authorities, and the extra-info document to all authorities that
 * support it.
 */
void
directory_post_to_dirservers(uint8_t dir_purpose, uint8_t router_purpose,
                             dirinfo_type_t type,
                             const char *payload,
                             size_t payload_len, size_t extrainfo_len)
{
  const or_options_t *options = get_options();
  dir_indirection_t indirection;
  const smartlist_t *dirservers = router_get_trusted_dir_servers();
  int found = 0;
  const int exclude_self = (dir_purpose == DIR_PURPOSE_UPLOAD_VOTE ||
                            dir_purpose == DIR_PURPOSE_UPLOAD_SIGNATURES);
  tor_assert(dirservers);
  /* This tries dirservers which we believe to be down, but ultimately, that's
   * harmless, and we may as well err on the side of getting things uploaded.
   */
  SMARTLIST_FOREACH_BEGIN(dirservers, dir_server_t *, ds) {
      const routerstatus_t *rs = router_get_consensus_status_by_id(ds->digest);
      if (!rs) {
        /* prefer to use the address in the consensus, but fall back to
         * the hard-coded trusted_dir_server address if we don't have a
         * consensus or this digest isn't in our consensus. */
        rs = &ds->fake_status;
      }

      size_t upload_len = payload_len;

      if ((type & ds->type) == 0)
        continue;

      if (exclude_self && router_digest_is_me(ds->digest)) {
        /* we don't upload to ourselves, but there's now at least
         * one authority of this type that has what we wanted to upload. */
        found = 1;
        continue;
      }

      if (options->StrictNodes &&
          routerset_contains_routerstatus(options->ExcludeNodes, rs, -1)) {
        log_warn(LD_DIR, "Wanted to contact authority '%s' for %s, but "
                 "it's in our ExcludedNodes list and StrictNodes is set. "
                 "Skipping.",
                 ds->nickname,
                 dir_conn_purpose_to_string(dir_purpose));
        continue;
      }

      found = 1; /* at least one authority of this type was listed */
      if (dir_purpose == DIR_PURPOSE_UPLOAD_DIR)
        ds->has_accepted_serverdesc = 0;

      if (extrainfo_len && router_supports_extrainfo(ds->digest, 1)) {
        upload_len += extrainfo_len;
        log_info(LD_DIR, "Uploading an extrainfo too (length %d)",
                 (int) extrainfo_len);
      }
      if (purpose_needs_anonymity(dir_purpose, router_purpose, NULL)) {
        indirection = DIRIND_ANONYMOUS;
      } else if (!reachable_addr_allows_rs(rs, FIREWALL_DIR_CONNECTION, 0)) {
        if (reachable_addr_allows_rs(rs, FIREWALL_OR_CONNECTION, 0))
          indirection = DIRIND_ONEHOP;
        else
          indirection = DIRIND_ANONYMOUS;
      } else {
        indirection = DIRIND_DIRECT_CONN;
      }

      directory_request_t *req = directory_request_new(dir_purpose);
      directory_request_set_routerstatus(req, rs);
      directory_request_set_router_purpose(req, router_purpose);
      directory_request_set_indirection(req, indirection);
      directory_request_set_payload(req, payload, upload_len);
      directory_initiate_request(req);
      directory_request_free(req);
  } SMARTLIST_FOREACH_END(ds);
  if (!found) {
    char *s = authdir_type_to_string(type);
    log_warn(LD_DIR, "Publishing server descriptor to directory authorities "
             "of type '%s', but no authorities of that type listed!", s);
    tor_free(s);
  }
}

/** Return true iff, according to the values in <b>options</b>, we should be
 * using directory guards for direct downloads of directory information. */
STATIC int
should_use_directory_guards(const or_options_t *options)
{
  /* Public (non-bridge) servers never use directory guards. */
  if (public_server_mode(options))
    return 0;
  /* If guards are disabled, we can't use directory guards.
   */
  if (!options->UseEntryGuards)
    return 0;
  /* If we're configured to fetch directory info aggressively or of a
   * nonstandard type, don't use directory guards. */
  if (options->DownloadExtraInfo || options->FetchDirInfoEarly ||
      options->FetchDirInfoExtraEarly || options->FetchUselessDescriptors)
    return 0;
  return 1;
}

/** Pick an unconstrained directory server from among our guards, the latest
 * networkstatus, or the fallback dirservers, for use in downloading
 * information of type <b>type</b>, and return its routerstatus. */
static const routerstatus_t *
directory_pick_generic_dirserver(dirinfo_type_t type, int pds_flags,
                                 uint8_t dir_purpose,
                                 circuit_guard_state_t **guard_state_out)
{
  const routerstatus_t *rs = NULL;
  const or_options_t *options = get_options();

  if (options->UseBridges)
    log_warn(LD_BUG, "Called when we have UseBridges set.");

  if (should_use_directory_guards(options)) {
    const node_t *node = guards_choose_dirguard(dir_purpose, guard_state_out);
    if (node)
      rs = node->rs;
  } else {
    /* anybody with a non-zero dirport will do */
    rs = router_pick_directory_server(type, pds_flags);
  }
  if (!rs) {
    log_info(LD_DIR, "No router found for %s; falling back to "
             "dirserver list.", dir_conn_purpose_to_string(dir_purpose));
    rs = router_pick_fallback_dirserver(type, pds_flags);
  }

  return rs;
}

/**
 * Set the extra fields in <b>req</b> that are used when requesting a
 * consensus of type <b>resource</b>.
 *
 * Right now, these fields are if-modified-since and x-or-diff-from-consensus.
 */
static void
dir_consensus_request_set_additional_headers(directory_request_t *req,
                                             const char *resource)
{
  time_t if_modified_since = 0;
  uint8_t or_diff_from[DIGEST256_LEN];
  int or_diff_from_is_set = 0;

  /* DEFAULT_IF_MODIFIED_SINCE_DELAY is 1/20 of the default consensus
   * period of 1 hour.
   */
  const int DEFAULT_IF_MODIFIED_SINCE_DELAY = 180;
  const int32_t DEFAULT_TRY_DIFF_FOR_CONSENSUS_NEWER = 72;
  const int32_t MIN_TRY_DIFF_FOR_CONSENSUS_NEWER = 0;
  const int32_t MAX_TRY_DIFF_FOR_CONSENSUS_NEWER = 8192;
  const char TRY_DIFF_FOR_CONSENSUS_NEWER_NAME[] =
    "try-diff-for-consensus-newer-than";

  int flav = FLAV_NS;
  if (resource)
    flav = networkstatus_parse_flavor_name(resource);

  int32_t max_age_for_diff = 3600 *
    networkstatus_get_param(NULL,
                            TRY_DIFF_FOR_CONSENSUS_NEWER_NAME,
                            DEFAULT_TRY_DIFF_FOR_CONSENSUS_NEWER,
                            MIN_TRY_DIFF_FOR_CONSENSUS_NEWER,
                            MAX_TRY_DIFF_FOR_CONSENSUS_NEWER);

  if (flav != -1) {
    /* IF we have a parsed consensus of this type, we can do an
     * if-modified-time based on it. */
    networkstatus_t *v;
    v = networkstatus_get_latest_consensus_by_flavor(flav);
    if (v) {
      /* In networks with particularly short V3AuthVotingIntervals,
       * ask for the consensus if it's been modified since half the
       * V3AuthVotingInterval of the most recent consensus. */
      time_t ims_delay = DEFAULT_IF_MODIFIED_SINCE_DELAY;
      if (v->fresh_until > v->valid_after
          && ims_delay > (v->fresh_until - v->valid_after)/2) {
        ims_delay = (v->fresh_until - v->valid_after)/2;
      }
      if_modified_since = v->valid_after + ims_delay;
      if (v->valid_after >= approx_time() - max_age_for_diff) {
        memcpy(or_diff_from, v->digest_sha3_as_signed, DIGEST256_LEN);
        or_diff_from_is_set = 1;
      }
    }
  } else {
    /* Otherwise it might be a consensus we don't parse, but which we
     * do cache.  Look at the cached copy, perhaps. */
    cached_dir_t *cd = dirserv_get_consensus(resource);
    /* We have no method of determining the voting interval from an
     * unparsed consensus, so we use the default. */
    if (cd) {
      if_modified_since = cd->published + DEFAULT_IF_MODIFIED_SINCE_DELAY;
      if (cd->published >= approx_time() - max_age_for_diff) {
        memcpy(or_diff_from, cd->digest_sha3_as_signed, DIGEST256_LEN);
        or_diff_from_is_set = 1;
      }
    }
  }

  if (if_modified_since > 0)
    directory_request_set_if_modified_since(req, if_modified_since);
  if (or_diff_from_is_set) {
    char hex[HEX_DIGEST256_LEN + 1];
    base16_encode(hex, sizeof(hex),
                  (const char*)or_diff_from, sizeof(or_diff_from));
    directory_request_add_header(req, X_OR_DIFF_FROM_CONSENSUS_HEADER, hex);
  }
}
/** Start a connection to a random running directory server, using
 * connection purpose <b>dir_purpose</b>, intending to fetch descriptors
 * of purpose <b>router_purpose</b>, and requesting <b>resource</b>.
 * Use <b>pds_flags</b> as arguments to router_pick_directory_server()
 * or router_pick_trusteddirserver().
 */
MOCK_IMPL(void,
directory_get_from_dirserver,(
                            uint8_t dir_purpose,
                            uint8_t router_purpose,
                            const char *resource,
                            int pds_flags,
                            download_want_authority_t want_authority))
{
  const routerstatus_t *rs = NULL;
  const or_options_t *options = get_options();
  int prefer_authority = (dirclient_fetches_from_authorities(options)
                          || want_authority == DL_WANT_AUTHORITY);
  int require_authority = 0;
  int get_via_tor = purpose_needs_anonymity(dir_purpose, router_purpose,
                                            resource);
  dirinfo_type_t type = dir_fetch_type(dir_purpose, router_purpose, resource);

  if (type == NO_DIRINFO)
    return;

  if (!options->FetchServerDescriptors)
    return;

  circuit_guard_state_t *guard_state = NULL;
  if (!get_via_tor) {
    if (options->UseBridges && !(type & BRIDGE_DIRINFO)) {
      /* We want to ask a running bridge for which we have a descriptor.
       *
       * When we ask choose_random_entry() for a bridge, we specify what
       * sort of dir fetch we'll be doing, so it won't return a bridge
       * that can't answer our question.
       */
      const node_t *node = guards_choose_dirguard(dir_purpose, &guard_state);
      if (node && node->ri) {
        /* every bridge has a routerinfo. */
        routerinfo_t *ri = node->ri;
        /* clients always make OR connections to bridges */
        tor_addr_port_t or_ap;
        directory_request_t *req = directory_request_new(dir_purpose);
        /* we are willing to use a non-preferred address if we need to */
        reachable_addr_choose_from_node(node, FIREWALL_OR_CONNECTION, 0,
                                             &or_ap);
        directory_request_set_or_addr_port(req, &or_ap);
        directory_request_set_directory_id_digest(req,
                                            ri->cache_info.identity_digest);
        directory_request_set_router_purpose(req, router_purpose);
        directory_request_set_resource(req, resource);
        if (dir_purpose == DIR_PURPOSE_FETCH_CONSENSUS)
          dir_consensus_request_set_additional_headers(req, resource);
        directory_request_set_guard_state(req, guard_state);
        directory_initiate_request(req);
        directory_request_free(req);
      } else {
        if (guard_state) {
          entry_guard_cancel(&guard_state);
        }
        log_notice(LD_DIR, "Ignoring directory request, since no bridge "
                           "nodes are available yet.");
      }

      return;
    } else {
      if (prefer_authority || (type & BRIDGE_DIRINFO)) {
        /* only ask authdirservers, and don't ask myself */
        rs = router_pick_trusteddirserver(type, pds_flags);
        if (rs == NULL && (pds_flags & (PDS_NO_EXISTING_SERVERDESC_FETCH|
                                        PDS_NO_EXISTING_MICRODESC_FETCH))) {
          /* We don't want to fetch from any authorities that we're currently
           * fetching server descriptors from, and we got no match.  Did we
           * get no match because all the authorities have connections
           * fetching server descriptors (in which case we should just
           * return,) or because all the authorities are down or on fire or
           * unreachable or something (in which case we should go on with
           * our fallback code)? */
          pds_flags &= ~(PDS_NO_EXISTING_SERVERDESC_FETCH|
                         PDS_NO_EXISTING_MICRODESC_FETCH);
          rs = router_pick_trusteddirserver(type, pds_flags);
          if (rs) {
            log_debug(LD_DIR, "Deferring serverdesc fetch: all authorities "
                      "are in use.");
            return;
          }
        }
        if (rs == NULL && require_authority) {
          log_info(LD_DIR, "No authorities were available for %s: will try "
                   "later.", dir_conn_purpose_to_string(dir_purpose));
          return;
        }
      }
      if (!rs && !(type & BRIDGE_DIRINFO)) {
        rs = directory_pick_generic_dirserver(type, pds_flags,
                                              dir_purpose,
                                              &guard_state);
        if (!rs)
          get_via_tor = 1; /* last resort: try routing it via Tor */
      }
    }
  }

  if (get_via_tor) {
    /* Never use fascistfirewall; we're going via Tor. */
    pds_flags |= PDS_IGNORE_FASCISTFIREWALL;
    rs = router_pick_directory_server(type, pds_flags);
  }

  /* If we have any hope of building an indirect conn, we know some router
   * descriptors.  If (rs==NULL), we can't build circuits anyway, so
   * there's no point in falling back to the authorities in this case. */
  if (rs) {
    const dir_indirection_t indirection =
      get_via_tor ? DIRIND_ANONYMOUS : DIRIND_ONEHOP;
    directory_request_t *req = directory_request_new(dir_purpose);
    directory_request_set_routerstatus(req, rs);
    directory_request_set_router_purpose(req, router_purpose);
    directory_request_set_indirection(req, indirection);
    directory_request_set_resource(req, resource);
    if (dir_purpose == DIR_PURPOSE_FETCH_CONSENSUS)
      dir_consensus_request_set_additional_headers(req, resource);
    if (guard_state)
      directory_request_set_guard_state(req, guard_state);
    directory_initiate_request(req);
    directory_request_free(req);
  } else {
    log_notice(LD_DIR,
               "While fetching directory info, "
               "no running dirservers known. Will try again later. "
               "(purpose %d)", dir_purpose);
    if (!purpose_needs_anonymity(dir_purpose, router_purpose, resource)) {
      /* remember we tried them all and failed. */
      directory_all_unreachable(time(NULL));
    }
  }
}

/** As directory_get_from_dirserver, but initiates a request to <i>every</i>
 * directory authority other than ourself.  Only for use by authorities when
 * searching for missing information while voting. */
void
directory_get_from_all_authorities(uint8_t dir_purpose,
                                   uint8_t router_purpose,
                                   const char *resource)
{
  tor_assert(dir_purpose == DIR_PURPOSE_FETCH_STATUS_VOTE ||
             dir_purpose == DIR_PURPOSE_FETCH_DETACHED_SIGNATURES);

  SMARTLIST_FOREACH_BEGIN(router_get_trusted_dir_servers(),
                          dir_server_t *, ds) {
      if (router_digest_is_me(ds->digest))
        continue;
      if (!(ds->type & V3_DIRINFO))
        continue;
      const routerstatus_t *rs = router_get_consensus_status_by_id(ds->digest);
      if (!rs) {
        /* prefer to use the address in the consensus, but fall back to
         * the hard-coded trusted_dir_server address if we don't have a
         * consensus or this digest isn't in our consensus. */
        rs = &ds->fake_status;
      }
      directory_request_t *req = directory_request_new(dir_purpose);
      directory_request_set_routerstatus(req, rs);
      directory_request_set_router_purpose(req, router_purpose);
      directory_request_set_resource(req, resource);
      directory_initiate_request(req);
      directory_request_free(req);
  } SMARTLIST_FOREACH_END(ds);
}

/** Return true iff <b>ind</b> requires a multihop circuit. */
static int
dirind_is_anon(dir_indirection_t ind)
{
  return ind == DIRIND_ANON_DIRPORT || ind == DIRIND_ANONYMOUS;
}

/* Choose reachable OR and Dir addresses and ports from status, copying them
 * into use_or_ap and use_dir_ap. If indirection is anonymous, then we're
 * connecting via another relay, so choose the primary IPv4 address and ports.
 *
 * status should have at least one reachable address, if we can't choose a
 * reachable address, warn and return -1. Otherwise, return 0.
 */
static int
directory_choose_address_routerstatus(const routerstatus_t *status,
                                      dir_indirection_t indirection,
                                      tor_addr_port_t *use_or_ap,
                                      tor_addr_port_t *use_dir_ap)
{
  tor_assert(status != NULL);
  tor_assert(use_or_ap != NULL);
  tor_assert(use_dir_ap != NULL);

  const or_options_t *options = get_options();
  int have_or = 0, have_dir = 0;

  /* We expect status to have at least one reachable address if we're
   * connecting to it directly.
   *
   * Therefore, we can simply use the other address if the one we want isn't
   * allowed by the firewall.
   *
   * (When Tor uploads and downloads a hidden service descriptor, it uses
   * DIRIND_ANONYMOUS. Even Single Onion Servers (NYI) use DIRIND_ANONYMOUS,
   * to avoid HSDirs denying service by rejecting descriptors.)
   */

  /* Initialise the OR / Dir addresses */
  tor_addr_make_null(&use_or_ap->addr, AF_UNSPEC);
  use_or_ap->port = 0;
  tor_addr_make_null(&use_dir_ap->addr, AF_UNSPEC);
  use_dir_ap->port = 0;

  /* ORPort connections */
  if (indirection == DIRIND_ANONYMOUS) {
    if (!tor_addr_is_null(&status->ipv4_addr)) {
      /* Since we're going to build a 3-hop circuit and ask the 2nd relay
       * to extend to this address, always use the primary (IPv4) OR address */
      tor_addr_copy(&use_or_ap->addr, &status->ipv4_addr);
      use_or_ap->port = status->ipv4_orport;
      have_or = 1;
    }
  } else if (indirection == DIRIND_ONEHOP) {
    /* We use an IPv6 address if we have one and we prefer it.
     * Use the preferred address and port if they are reachable, otherwise,
     * use the alternate address and port (if any).
     */
    reachable_addr_choose_from_rs(status, FIREWALL_OR_CONNECTION, 0,
                                       use_or_ap);
    have_or = tor_addr_port_is_valid_ap(use_or_ap, 0);
  }

  /* DirPort connections
   * DIRIND_ONEHOP uses ORPort, but may fall back to the DirPort on relays */
  if (indirection == DIRIND_DIRECT_CONN ||
      indirection == DIRIND_ANON_DIRPORT ||
      (indirection == DIRIND_ONEHOP
       && !dirclient_must_use_begindir(options))) {
    reachable_addr_choose_from_rs(status, FIREWALL_DIR_CONNECTION, 0,
                                       use_dir_ap);
    have_dir = tor_addr_port_is_valid_ap(use_dir_ap, 0);
  }

  /* We rejected all addresses in the relay's status. This means we can't
   * connect to it. */
  if (!have_or && !have_dir) {
    static int logged_backtrace = 0;
    char *ipv6_str = tor_addr_to_str_dup(&status->ipv6_addr);
    log_info(LD_BUG, "Rejected all OR and Dir addresses from %s when "
             "launching an outgoing directory connection to: IPv4 %s OR %d "
             "Dir %d IPv6 %s OR %d Dir %d", routerstatus_describe(status),
             fmt_addr(&status->ipv4_addr), status->ipv4_orport,
             status->ipv4_dirport, ipv6_str, status->ipv6_orport,
             status->ipv4_dirport);
    tor_free(ipv6_str);
    if (!logged_backtrace) {
      log_backtrace(LOG_INFO, LD_BUG, "Addresses came from");
      logged_backtrace = 1;
    }
    return -1;
  }

  return 0;
}

/** Called when we are unable to complete the client's request to a directory
 * server due to a network error: Mark the router as down and try again if
 * possible.
 */
void
connection_dir_client_request_failed(dir_connection_t *conn)
{
  if (conn->guard_state) {
    /* We haven't seen a success on this guard state, so consider it to have
     * failed. */
    entry_guard_failed(&conn->guard_state);
  }
  if (!entry_list_is_constrained(get_options()))
    /* We must not set a directory to non-running for HS purposes else we end
     * up flagging nodes from the hashring has unusable. It doesn't have direct
     * effect on the HS subsystem because the nodes are selected regardless of
     * their status but still, we shouldn't flag them as non running.
     *
     * One example where this can go bad is if a tor instance gets added a lot
     * of ephemeral services and with a network with problem then many nodes in
     * the consenus ends up unusable.
     *
     * Furthermore, a service does close any pending directory connections
     * before uploading a descriptor and thus we can end up here in a natural
     * way since closing a pending directory connection leads to this code
     * path. */
    if (!DIR_PURPOSE_IS_HS(TO_CONN(conn)->purpose)) {
      router_set_status(conn->identity_digest, 0);
    }
  if (conn->base_.purpose == DIR_PURPOSE_FETCH_SERVERDESC ||
             conn->base_.purpose == DIR_PURPOSE_FETCH_EXTRAINFO) {
    log_info(LD_DIR, "Giving up on serverdesc/extrainfo fetch from "
             "directory server at %s; retrying",
             connection_describe_peer(TO_CONN(conn)));
    if (conn->router_purpose == ROUTER_PURPOSE_BRIDGE)
      connection_dir_bridge_routerdesc_failed(conn);
    connection_dir_download_routerdesc_failed(conn);
  } else if (conn->base_.purpose == DIR_PURPOSE_FETCH_CONSENSUS) {
    if (conn->requested_resource)
      networkstatus_consensus_download_failed(0, conn->requested_resource);
  } else if (conn->base_.purpose == DIR_PURPOSE_FETCH_CERTIFICATE) {
    log_info(LD_DIR, "Giving up on certificate fetch from directory server "
             "at %s; retrying",
             connection_describe_peer(TO_CONN(conn)));
    connection_dir_download_cert_failed(conn, 0);
  } else if (conn->base_.purpose == DIR_PURPOSE_FETCH_DETACHED_SIGNATURES) {
    log_info(LD_DIR, "Giving up downloading detached signatures from %s",
             connection_describe_peer(TO_CONN(conn)));
  } else if (conn->base_.purpose == DIR_PURPOSE_FETCH_STATUS_VOTE) {
    log_info(LD_DIR, "Giving up downloading votes from %s",
             connection_describe_peer(TO_CONN(conn)));
  } else if (conn->base_.purpose == DIR_PURPOSE_FETCH_MICRODESC) {
    log_info(LD_DIR, "Giving up on downloading microdescriptors from "
             "directory server at %s; will retry",
             connection_describe_peer(TO_CONN(conn)));
    connection_dir_download_routerdesc_failed(conn);
  }
}

/** Helper: Attempt to fetch directly the descriptors of each bridge
 * listed in <b>failed</b>.
 */
static void
connection_dir_retry_bridges(smartlist_t *descs)
{
  char digest[DIGEST_LEN];
  SMARTLIST_FOREACH(descs, const char *, cp,
  {
    if (base16_decode(digest, DIGEST_LEN, cp, strlen(cp)) != DIGEST_LEN) {
      log_warn(LD_BUG, "Malformed fingerprint in list: %s",
              escaped(cp));
      continue;
    }
    retry_bridge_descriptor_fetch_directly(digest);
  });
}

/** Called when an attempt to download one or more router descriptors
 * or extra-info documents on connection <b>conn</b> failed.
 */
static void
connection_dir_download_routerdesc_failed(dir_connection_t *conn)
{
  /* No need to increment the failure count for routerdescs, since
   * it's not their fault. */

  /* No need to relaunch descriptor downloads here: we already do it
   * every 10 or 60 seconds (FOO_DESCRIPTOR_RETRY_INTERVAL) in main.c. */
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_SERVERDESC ||
             conn->base_.purpose == DIR_PURPOSE_FETCH_EXTRAINFO ||
             conn->base_.purpose == DIR_PURPOSE_FETCH_MICRODESC);

  (void) conn;
}

/** Called when an attempt to download a bridge's routerdesc from
 * one of the authorities failed due to a network error. If
 * possible attempt to download descriptors from the bridge directly.
 */
static void
connection_dir_bridge_routerdesc_failed(dir_connection_t *conn)
{
  smartlist_t *which = NULL;

  /* Requests for bridge descriptors are in the form 'fp/', so ignore
     anything else. */
  if (!conn->requested_resource || strcmpstart(conn->requested_resource,"fp/"))
    return;

  which = smartlist_new();
  dir_split_resource_into_fingerprints(conn->requested_resource
                                        + strlen("fp/"),
                                       which, NULL, 0);

  tor_assert(conn->base_.purpose != DIR_PURPOSE_FETCH_EXTRAINFO);
  if (smartlist_len(which)) {
    connection_dir_retry_bridges(which);
    SMARTLIST_FOREACH(which, char *, cp, tor_free(cp));
  }
  smartlist_free(which);
}

/** Called when an attempt to fetch a certificate fails. */
static void
connection_dir_download_cert_failed(dir_connection_t *conn, int status)
{
  const char *fp_pfx = "fp/";
  const char *fpsk_pfx = "fp-sk/";
  smartlist_t *failed;
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_CERTIFICATE);

  if (!conn->requested_resource)
    return;
  failed = smartlist_new();
  /*
   * We have two cases download by fingerprint (resource starts
   * with "fp/") or download by fingerprint/signing key pair
   * (resource starts with "fp-sk/").
   */
  if (!strcmpstart(conn->requested_resource, fp_pfx)) {
    /* Download by fingerprint case */
    dir_split_resource_into_fingerprints(conn->requested_resource +
                                         strlen(fp_pfx),
                                         failed, NULL, DSR_HEX);
    SMARTLIST_FOREACH_BEGIN(failed, char *, cp) {
      /* Null signing key digest indicates download by fp only */
      authority_cert_dl_failed(cp, NULL, status);
      tor_free(cp);
    } SMARTLIST_FOREACH_END(cp);
  } else if (!strcmpstart(conn->requested_resource, fpsk_pfx)) {
    /* Download by (fp,sk) pairs */
    dir_split_resource_into_fingerprint_pairs(conn->requested_resource +
                                              strlen(fpsk_pfx), failed);
    SMARTLIST_FOREACH_BEGIN(failed, fp_pair_t *, cp) {
      authority_cert_dl_failed(cp->first, cp->second, status);
      tor_free(cp);
    } SMARTLIST_FOREACH_END(cp);
  } else {
    log_warn(LD_DIR,
             "Don't know what to do with failure for cert fetch %s",
             conn->requested_resource);
  }

  smartlist_free(failed);

  update_certificate_downloads(time(NULL));
}

/** Evaluate the situation and decide if we should use an encrypted
 * "begindir-style" connection for this directory request.
 * 0) If there is no DirPort, yes.
 * 1) If or_port is 0, or it's a direct conn and or_port is firewalled
 *    or we're a dir mirror, no.
 * 2) If we prefer to avoid begindir conns, and we're not fetching or
 *    publishing a bridge relay descriptor, no.
 * 3) Else yes.
 * If returning 0, return in *reason why we can't use begindir.
 * reason must not be NULL.
 */
static int
directory_command_should_use_begindir(const or_options_t *options,
                                      const directory_request_t *req,
                                      const char **reason)
{
  const tor_addr_t *or_addr = &req->or_addr_port.addr;
  //const tor_addr_t *dir_addr = &req->dir_addr_port.addr;
  const int or_port = req->or_addr_port.port;
  const int dir_port = req->dir_addr_port.port;

  const dir_indirection_t indirection = req->indirection;

  tor_assert(reason);
  *reason = NULL;

  /* Reasons why we must use begindir */
  if (!dir_port) {
    *reason = "(using begindir - directory with no DirPort)";
    return 1; /* We don't know a DirPort -- must begindir. */
  }
  /* Reasons why we can't possibly use begindir */
  if (!or_port) {
    *reason = "directory with unknown ORPort";
    return 0; /* We don't know an ORPort -- no chance. */
  }
  if (indirection == DIRIND_DIRECT_CONN ||
      indirection == DIRIND_ANON_DIRPORT) {
    *reason = "DirPort connection";
    return 0;
  }
  if (indirection == DIRIND_ONEHOP) {
    /* We're firewalled and want a direct OR connection */
    if (!reachable_addr_allows_addr(or_addr, or_port,
                                              FIREWALL_OR_CONNECTION, 0, 0)) {
      *reason = "ORPort not reachable";
      return 0;
    }
  }
  /* Reasons why we want to avoid using begindir */
  if (indirection == DIRIND_ONEHOP) {
    if (!dirclient_must_use_begindir(options)) {
      *reason = "in relay mode";
      return 0;
    }
  }
  /* DIRIND_ONEHOP on a client, or DIRIND_ANONYMOUS
   */
  *reason = "(using begindir)";
  return 1;
}

/**
 * Create and return a new directory_request_t with purpose
 * <b>dir_purpose</b>.
 */
directory_request_t *
directory_request_new(uint8_t dir_purpose)
{
  tor_assert(dir_purpose >= DIR_PURPOSE_MIN_);
  tor_assert(dir_purpose <= DIR_PURPOSE_MAX_);
  tor_assert(dir_purpose != DIR_PURPOSE_SERVER);
  tor_assert(dir_purpose != DIR_PURPOSE_HAS_FETCHED_HSDESC);

  directory_request_t *result = tor_malloc_zero(sizeof(*result));
  tor_addr_make_null(&result->or_addr_port.addr, AF_INET);
  result->or_addr_port.port = 0;
  tor_addr_make_null(&result->dir_addr_port.addr, AF_INET);
  result->dir_addr_port.port = 0;
  result->dir_purpose = dir_purpose;
  result->router_purpose = ROUTER_PURPOSE_GENERAL;
  result->indirection = DIRIND_ONEHOP;
  return result;
}
/**
 * Release all resources held by <b>req</b>.
 */
void
directory_request_free_(directory_request_t *req)
{
  if (req == NULL)
    return;
  config_free_lines(req->additional_headers);
  tor_free(req);
}
/**
 * Set the address and OR port to use for this directory request.  If there is
 * no OR port, we'll have to connect over the dirport.  (If there are both,
 * the indirection setting determines which to use.)
 */
void
directory_request_set_or_addr_port(directory_request_t *req,
                                   const tor_addr_port_t *p)
{
  memcpy(&req->or_addr_port, p, sizeof(*p));
}
/**
 * Set the address and dirport to use for this directory request.  If there
 * is no dirport, we'll have to connect over the OR port.  (If there are both,
 * the indirection setting determines which to use.)
 */
void
directory_request_set_dir_addr_port(directory_request_t *req,
                                    const tor_addr_port_t *p)
{
  memcpy(&req->dir_addr_port, p, sizeof(*p));
}
/**
 * Set the RSA identity digest of the directory to use for this directory
 * request.
 */
void
directory_request_set_directory_id_digest(directory_request_t *req,
                                          const char *digest)
{
  memcpy(req->digest, digest, DIGEST_LEN);
}
/**
 * Set the router purpose associated with uploaded and downloaded router
 * descriptors and extrainfo documents in this directory request.  The purpose
 * must be one of ROUTER_PURPOSE_GENERAL (the default) or
 * ROUTER_PURPOSE_BRIDGE.
 */
void
directory_request_set_router_purpose(directory_request_t *req,
                                     uint8_t router_purpose)
{
  tor_assert(router_purpose == ROUTER_PURPOSE_GENERAL ||
             router_purpose == ROUTER_PURPOSE_BRIDGE);
  // assert that it actually makes sense to set this purpose, given
  // the dir_purpose.
  req->router_purpose = router_purpose;
}
/**
 * Set the indirection to be used for the directory request.  The indirection
 * parameter configures whether to connect to a DirPort or ORPort, and whether
 * to anonymize the connection.  DIRIND_ONEHOP (use ORPort, don't anonymize)
 * is the default.  See dir_indirection_t for more information.
 */
void
directory_request_set_indirection(directory_request_t *req,
                                  dir_indirection_t indirection)
{
  req->indirection = indirection;
}

/**
 * Set a pointer to the resource to request from a directory.  Different
 * request types use resources to indicate different components of their URL.
 * Note that only an alias to <b>resource</b> is stored, so the
 * <b>resource</b> must outlive the request.
 */
void
directory_request_set_resource(directory_request_t *req,
                               const char *resource)
{
  req->resource = resource;
}
/**
 * Set a pointer to the payload to include with this directory request, along
 * with its length.  Note that only an alias to <b>payload</b> is stored, so
 * the <b>payload</b> must outlive the request.
 */
void
directory_request_set_payload(directory_request_t *req,
                              const char *payload,
                              size_t payload_len)
{
  tor_assert(DIR_PURPOSE_IS_UPLOAD(req->dir_purpose));

  req->payload = payload;
  req->payload_len = payload_len;
}
/**
 * Set an if-modified-since date to send along with the request.  The
 * default is 0 (meaning, send no if-modified-since header).
 */
void
directory_request_set_if_modified_since(directory_request_t *req,
                                        time_t if_modified_since)
{
  req->if_modified_since = if_modified_since;
}

/** Include a header of name <b>key</b> with content <b>val</b> in the
 * request. Neither may include newlines or other odd characters. Their
 * ordering is not currently guaranteed.
 *
 * Note that, as elsewhere in this module, header keys include a trailing
 * colon and space.
 */
void
directory_request_add_header(directory_request_t *req,
                             const char *key,
                             const char *val)
{
  config_line_prepend(&req->additional_headers, key, val);
}
/**
 * Set an object containing HS connection identifier to be associated with
 * this request. Note that only an alias to <b>ident</b> is stored, so the
 * <b>ident</b> object must outlive the request.
 */
void
directory_request_upload_set_hs_ident(directory_request_t *req,
                                      const hs_ident_dir_conn_t *ident)
{
  if (ident) {
    tor_assert(req->dir_purpose == DIR_PURPOSE_UPLOAD_HSDESC);
  }
  req->hs_ident = ident;
}
/**
 * Set an object containing HS connection identifier to be associated with
 * this fetch request. Note that only an alias to <b>ident</b> is stored, so
 * the <b>ident</b> object must outlive the request.
 */
void
directory_request_fetch_set_hs_ident(directory_request_t *req,
                                     const hs_ident_dir_conn_t *ident)
{
  if (ident) {
    tor_assert(req->dir_purpose == DIR_PURPOSE_FETCH_HSDESC);
  }
  req->hs_ident = ident;
}
/** Set a static circuit_guard_state_t object to affliate with the request in
 * <b>req</b>.  This object will receive notification when the attempt to
 * connect to the guard either succeeds or fails. */
void
directory_request_set_guard_state(directory_request_t *req,
                                  circuit_guard_state_t *state)
{
  req->guard_state = state;
}

/**
 * Internal: Return true if any information for contacting the directory in
 * <b>req</b> has been set, other than by the routerstatus. */
static int
directory_request_dir_contact_info_specified(const directory_request_t *req)
{
  /* We only check for ports here, since we don't use an addr unless the port
   * is set */
  return (req->or_addr_port.port ||
          req->dir_addr_port.port ||
          ! tor_digest_is_zero(req->digest));
}

/**
 * Set the routerstatus to use for the directory associated with this
 * request.  If this option is set, then no other function to set the
 * directory's address or identity should be called.
 */
void
directory_request_set_routerstatus(directory_request_t *req,
                                   const routerstatus_t *status)
{
  req->routerstatus = status;
}

/**
 * Helper: update the addresses, ports, and identities in <b>req</b>
 * from the routerstatus object in <b>req</b>.  Return 0 on success.
 * On failure, warn and return -1.
 */
static int
directory_request_set_dir_from_routerstatus(directory_request_t *req)

{
  const routerstatus_t *status = req->routerstatus;
  if (BUG(status == NULL))
    return -1;
  const or_options_t *options = get_options();
  const node_t *node;
  tor_addr_port_t use_or_ap, use_dir_ap;
  const int anonymized_connection = dirind_is_anon(req->indirection);

  tor_assert(status != NULL);

  node = node_get_by_id(status->identity_digest);

  /* XXX The below check is wrong: !node means it's not in the consensus,
   * but we haven't checked if we have a descriptor for it -- and also,
   * we only care about the descriptor if it's a begindir-style anonymized
   * connection. */
  if (!node && anonymized_connection) {
    log_info(LD_DIR, "Not sending anonymized request to directory '%s'; we "
             "don't have its router descriptor.",
             routerstatus_describe(status));
    return -1;
  }

  if (options->ExcludeNodes && options->StrictNodes &&
      routerset_contains_routerstatus(options->ExcludeNodes, status, -1)) {
    log_warn(LD_DIR, "Wanted to contact directory mirror %s for %s, but "
             "it's in our ExcludedNodes list and StrictNodes is set. "
             "Skipping. This choice might make your Tor not work.",
             routerstatus_describe(status),
             dir_conn_purpose_to_string(req->dir_purpose));
    return -1;
  }

  /* At this point, if we are a client making a direct connection to a
   * directory server, we have selected a server that has at least one address
   * allowed by ClientUseIPv4/6 and Reachable{"",OR,Dir}Addresses. This
   * selection uses the preference in ClientPreferIPv6{OR,Dir}Port, if
   * possible. (If UseBridges is set, clients always use IPv6, and prefer it
   * by default.)
   *
   * Now choose an address that we can use to connect to the directory server.
   */
  if (directory_choose_address_routerstatus(status,
                                            req->indirection, &use_or_ap,
                                            &use_dir_ap) < 0) {
    return -1;
  }

  /* One last thing: If we're talking to an authority, we might want to use
   * a special HTTP port for it based on our purpose.
   */
  if (req->indirection == DIRIND_DIRECT_CONN && status->is_authority) {
    const dir_server_t *ds = router_get_trusteddirserver_by_digest(
                                            status->identity_digest);
    if (ds) {
      const tor_addr_port_t *v4 = NULL;
      if (authdir_mode_v3(get_options())) {
        // An authority connecting to another authority should always
        // prefer the VOTING usage, if one is specifically configured.
        v4 = trusted_dir_server_get_dirport_exact(
                                    ds, AUTH_USAGE_VOTING, AF_INET);
      }
      if (! v4) {
        // Everybody else should prefer a usage dependent on their
        // the dir_purpose.
        auth_dirport_usage_t usage =
          auth_dirport_usage_for_purpose(req->dir_purpose);
        v4 = trusted_dir_server_get_dirport(ds, usage, AF_INET);
      }
      tor_assert_nonfatal(v4);
      if (v4) {
        // XXXX We could, if we wanted, also select a v6 address.  But a v4
        // address must exist here, and we as a relay are required to support
        // ipv4.  So we just that.
        tor_addr_port_copy(&use_dir_ap, v4);
      }
    }
  }

  directory_request_set_or_addr_port(req, &use_or_ap);
  directory_request_set_dir_addr_port(req, &use_dir_ap);
  directory_request_set_directory_id_digest(req, status->identity_digest);
  return 0;
}

/**
 * Launch the provided directory request, configured in <b>request</b>.
 * After this function is called, you can free <b>request</b>.
 */
MOCK_IMPL(void,
directory_initiate_request,(directory_request_t *request))
{
  tor_assert(request);
  if (request->routerstatus) {
    tor_assert_nonfatal(
               ! directory_request_dir_contact_info_specified(request));
    if (directory_request_set_dir_from_routerstatus(request) < 0) {
      return; // or here XXXX
    }
  }

  const tor_addr_port_t *or_addr_port = &request->or_addr_port;
  const tor_addr_port_t *dir_addr_port = &request->dir_addr_port;
  const char *digest = request->digest;
  const uint8_t dir_purpose = request->dir_purpose;
  const uint8_t router_purpose = request->router_purpose;
  const dir_indirection_t indirection = request->indirection;
  const char *resource = request->resource;
  const hs_ident_dir_conn_t *hs_ident = request->hs_ident;
  circuit_guard_state_t *guard_state = request->guard_state;

  tor_assert(or_addr_port->port || dir_addr_port->port);
  tor_assert(digest);

  dir_connection_t *conn;
  const or_options_t *options = get_options();
  int socket_error = 0;
  const char *begindir_reason = NULL;
  /* Should the connection be to a relay's OR port (and inside that we will
   * send our directory request)? */
  const int use_begindir =
    directory_command_should_use_begindir(options, request, &begindir_reason);

  /* Will the connection go via a three-hop Tor circuit? Note that this
   * is separate from whether it will use_begindir. */
  const int anonymized_connection = dirind_is_anon(indirection);

  /* What is the address we want to make the directory request to? If
   * we're making a begindir request this is the ORPort of the relay
   * we're contacting; if not a begindir request, this is its DirPort.
   * Note that if anonymized_connection is true, we won't be initiating
   * a connection directly to this address. */
  tor_addr_t addr;
  tor_addr_copy(&addr, &(use_begindir ? or_addr_port : dir_addr_port)->addr);
  uint16_t port = (use_begindir ? or_addr_port : dir_addr_port)->port;

  log_debug(LD_DIR, "anonymized %d, use_begindir %d.",
            anonymized_connection, use_begindir);

  log_debug(LD_DIR, "Initiating %s", dir_conn_purpose_to_string(dir_purpose));

  if (purpose_needs_anonymity(dir_purpose, router_purpose, resource)) {
    tor_assert(anonymized_connection ||
               hs_service_non_anonymous_mode_enabled(options));
  }

  /* use encrypted begindir connections for everything except relays
   * this provides better protection for directory fetches */
  if (!use_begindir && dirclient_must_use_begindir(options)) {
    log_warn(LD_BUG, "Client could not use begindir connection: %s",
             begindir_reason ? begindir_reason : "(NULL)");
    return;
  }

  /* ensure that we don't make direct connections when a SOCKS server is
   * configured. */
  if (!anonymized_connection && !use_begindir && !options->HTTPProxy &&
      (options->Socks4Proxy || options->Socks5Proxy)) {
    log_warn(LD_DIR, "Cannot connect to a directory server through a "
                     "SOCKS proxy!");
    return;
  }

  /* Make sure that the destination addr and port we picked is viable. */
  if (!port || tor_addr_is_null(&addr)) {
    static int logged_backtrace = 0;
    log_warn(LD_DIR,
             "Cannot make an outgoing %sconnection without a remote %sPort.",
             use_begindir ? "begindir " : "",
             use_begindir ? "OR" : "Dir");
    if (!logged_backtrace) {
      log_backtrace(LOG_INFO, LD_BUG, "Address came from");
      logged_backtrace = 1;
    }
    return;
  }

  conn = dir_connection_new(tor_addr_family(&addr));

  /* set up conn so it's got all the data we need to remember */
  tor_addr_copy(&conn->base_.addr, &addr);
  conn->base_.port = port;
  conn->base_.address = tor_addr_to_str_dup(&addr);
  memcpy(conn->identity_digest, digest, DIGEST_LEN);

  conn->base_.purpose = dir_purpose;
  conn->router_purpose = router_purpose;

  /* give it an initial state */
  conn->base_.state = DIR_CONN_STATE_CONNECTING;

  /* decide whether we can learn our IP address from this conn */
  /* XXXX This is a bad name for this field now. */
  conn->dirconn_direct = !anonymized_connection;

  if (hs_ident) {
    conn->hs_ident = hs_ident_dir_conn_dup(hs_ident);
  }

  if (!anonymized_connection && !use_begindir) {
    /* then we want to connect to dirport directly */

    if (options->HTTPProxy) {
      tor_addr_copy(&addr, &options->HTTPProxyAddr);
      port = options->HTTPProxyPort;
    }

    // In this case we should not have picked a directory guard.
    if (BUG(guard_state)) {
      entry_guard_cancel(&guard_state);
    }

    // XXXX This is the case where we replace.

    switch (connection_connect(TO_CONN(conn), conn->base_.address, &addr,
                               port, &socket_error)) {
      case -1:
        connection_mark_for_close(TO_CONN(conn));
        return;
      case 1:
        /* start flushing conn */
        conn->base_.state = DIR_CONN_STATE_CLIENT_SENDING;
        FALLTHROUGH;
      case 0:
        /* queue the command on the outbuf */
        directory_send_command(conn, 1, request);
        connection_watch_events(TO_CONN(conn), READ_EVENT | WRITE_EVENT);
        /* writable indicates finish, readable indicates broken link,
           error indicates broken link in windowsland. */
    }
  } else {
    /* We will use a Tor circuit (maybe 1-hop, maybe 3-hop, maybe with
     * begindir, maybe not with begindir) */

    entry_connection_t *linked_conn;

    /* Anonymized tunneled connections can never share a circuit.
     * One-hop directory connections can share circuits with each other
     * but nothing else. */
    int iso_flags = anonymized_connection ? ISO_STREAM : ISO_SESSIONGRP;

    /* If it's an anonymized connection, remember the fact that we
     * wanted it for later: maybe we'll want it again soon. */
    if (anonymized_connection && use_begindir)
      rep_hist_note_used_internal(time(NULL), 0, 1);
    else if (anonymized_connection && !use_begindir)
      rep_hist_note_used_port(time(NULL), conn->base_.port);

    // In this case we should not have a directory guard; we'll
    // get a regular guard later when we build the circuit.
    if (BUG(anonymized_connection && guard_state)) {
      entry_guard_cancel(&guard_state);
    }

    conn->guard_state = guard_state;

    /* make an AP connection
     * populate it and add it at the right state
     * hook up both sides
     */
    linked_conn =
      connection_ap_make_link(TO_CONN(conn),
                              conn->base_.address, conn->base_.port,
                              digest,
                              SESSION_GROUP_DIRCONN, iso_flags,
                              use_begindir, !anonymized_connection);
    if (!linked_conn) {
      log_warn(LD_NET,"Making tunnel to dirserver failed.");
      connection_mark_for_close(TO_CONN(conn));
      return;
    }

    if (connection_add(TO_CONN(conn)) < 0) {
      log_warn(LD_NET,"Unable to add connection for link to dirserver.");
      connection_mark_for_close(TO_CONN(conn));
      return;
    }
    conn->base_.state = DIR_CONN_STATE_CLIENT_SENDING;
    /* queue the command on the outbuf */
    directory_send_command(conn, 0, request);

    connection_watch_events(TO_CONN(conn), READ_EVENT|WRITE_EVENT);
    connection_start_reading(ENTRY_TO_CONN(linked_conn));
  }
}

/** Helper for sorting
 *
 * sort strings alphabetically
 *
 * XXXX we have a smartlist_sort_strings() function, right?
 */
static int
compare_strs_(const void **a, const void **b)
{
  const char *s1 = *a, *s2 = *b;
  return strcmp(s1, s2);
}

#define CONDITIONAL_CONSENSUS_FPR_LEN 3
CTASSERT(CONDITIONAL_CONSENSUS_FPR_LEN <= DIGEST_LEN);

/** Return the URL we should use for a consensus download.
 *
 * Use the "conditional consensus downloading" feature described in
 * dir-spec.txt, i.e.
 * GET .../consensus/<b>fpr</b>+<b>fpr</b>+<b>fpr</b>
 *
 * If 'resource' is provided, it is the name of a consensus flavor to request.
 */
static char *
directory_get_consensus_url(const char *resource)
{
  char *url = NULL;
  const char *hyphen, *flavor;
  if (resource==NULL || strcmp(resource, "ns")==0) {
    flavor = ""; /* Request ns consensuses as "", so older servers will work*/
    hyphen = "";
  } else {
    flavor = resource;
    hyphen = "-";
  }

  {
    char *authority_id_list;
    smartlist_t *authority_digests = smartlist_new();

    SMARTLIST_FOREACH_BEGIN(router_get_trusted_dir_servers(),
                            dir_server_t *, ds) {
        char *hex;
        if (!(ds->type & V3_DIRINFO))
          continue;

        hex = tor_malloc(2*CONDITIONAL_CONSENSUS_FPR_LEN+1);
        base16_encode(hex, 2*CONDITIONAL_CONSENSUS_FPR_LEN+1,
                      ds->v3_identity_digest, CONDITIONAL_CONSENSUS_FPR_LEN);
        smartlist_add(authority_digests, hex);
    } SMARTLIST_FOREACH_END(ds);
    smartlist_sort(authority_digests, compare_strs_);
    authority_id_list = smartlist_join_strings(authority_digests,
                                               "+", 0, NULL);

    tor_asprintf(&url, "/tor/status-vote/current/consensus%s%s/%s.z",
                 hyphen, flavor, authority_id_list);

    SMARTLIST_FOREACH(authority_digests, char *, cp, tor_free(cp));
    smartlist_free(authority_digests);
    tor_free(authority_id_list);
  }
  return url;
}

/**
 * Copies the ipv6 from source to destination, subject to buffer size limit
 * size. If decorate is true, makes sure the copied address is decorated.
 */
static void
copy_ipv6_address(char* destination, const char* source, size_t len,
                  int decorate) {
  tor_assert(destination);
  tor_assert(source);

  if (decorate && source[0] != '[') {
    tor_snprintf(destination, len, "[%s]", source);
  } else {
    strlcpy(destination, source, len);
  }
}

/** Queue an appropriate HTTP command for <b>request</b> on
 * <b>conn</b>-\>outbuf.  If <b>direct</b> is true, we're making a
 * non-anonymized connection to the dirport.
 */
static void
directory_send_command(dir_connection_t *conn,
                       const int direct,
                       const directory_request_t *req)
{
  tor_assert(req);
  const int purpose = req->dir_purpose;
  const char *resource = req->resource;
  const char *payload = req->payload;
  const size_t payload_len = req->payload_len;
  const time_t if_modified_since = req->if_modified_since;
  const int anonymized_connection = dirind_is_anon(req->indirection);

  char proxystring[256];
  char hoststring[128];
  /* NEEDS to be the same size hoststring.
   Will be decorated with brackets around it if it is ipv6. */
  char decorated_address[128];
  smartlist_t *headers = smartlist_new();
  char *url;
  char *accept_encoding;
  size_t url_len;
  char request[8192];
  size_t request_len, total_request_len = 0;
  const char *httpcommand = NULL;

  tor_assert(conn);
  tor_assert(conn->base_.type == CONN_TYPE_DIR);

  tor_free(conn->requested_resource);
  if (resource)
    conn->requested_resource = tor_strdup(resource);

  /* decorate the ip address if it is ipv6 */
  if (strchr(conn->base_.address, ':')) {
    copy_ipv6_address(decorated_address, conn->base_.address,
                      sizeof(decorated_address), 1);
  } else {
    strlcpy(decorated_address, conn->base_.address, sizeof(decorated_address));
  }

  /* come up with a string for which Host: we want */
  if (conn->base_.port == 80) {
    strlcpy(hoststring, decorated_address, sizeof(hoststring));
  } else {
    tor_snprintf(hoststring, sizeof(hoststring), "%s:%d",
                 decorated_address, conn->base_.port);
  }

  /* Format if-modified-since */
  if (if_modified_since) {
    char b[RFC1123_TIME_LEN+1];
    format_rfc1123_time(b, if_modified_since);
    smartlist_add_asprintf(headers, "If-Modified-Since: %s\r\n", b);
  }

  /* come up with some proxy lines, if we're using one. */
  if (direct && get_options()->HTTPProxy) {
    char *base64_authenticator=NULL;
    const char *authenticator = get_options()->HTTPProxyAuthenticator;

    tor_snprintf(proxystring, sizeof(proxystring),"http://%s", hoststring);
    if (authenticator) {
      base64_authenticator = alloc_http_authenticator(authenticator);
      if (!base64_authenticator)
        log_warn(LD_BUG, "Encoding http authenticator failed");
    }
    if (base64_authenticator) {
      smartlist_add_asprintf(headers,
                   "Proxy-Authorization: Basic %s\r\n",
                   base64_authenticator);
      tor_free(base64_authenticator);
    }
  } else {
    proxystring[0] = 0;
  }

  if (! anonymized_connection) {
    /* Add Accept-Encoding. */
    accept_encoding = accept_encoding_header();
    smartlist_add_asprintf(headers, "Accept-Encoding: %s\r\n",
                           accept_encoding);
    tor_free(accept_encoding);
  }

  /* Add additional headers, if any */
  {
    config_line_t *h;
    for (h = req->additional_headers; h; h = h->next) {
      smartlist_add_asprintf(headers, "%s%s\r\n", h->key, h->value);
    }
  }

  switch (purpose) {
    case DIR_PURPOSE_FETCH_CONSENSUS:
      /* resource is optional.  If present, it's a flavor name */
      tor_assert(!payload);
      httpcommand = "GET";
      url = directory_get_consensus_url(resource);
      log_info(LD_DIR, "Downloading consensus from %s using %s",
               hoststring, url);
      break;
    case DIR_PURPOSE_FETCH_CERTIFICATE:
      tor_assert(resource);
      tor_assert(!payload);
      httpcommand = "GET";
      tor_asprintf(&url, "/tor/keys/%s", resource);
      break;
    case DIR_PURPOSE_FETCH_STATUS_VOTE:
      tor_assert(resource);
      tor_assert(!payload);
      httpcommand = "GET";
      tor_asprintf(&url, "/tor/status-vote/next/%s.z", resource);
      break;
    case DIR_PURPOSE_FETCH_DETACHED_SIGNATURES:
      tor_assert(!resource);
      tor_assert(!payload);
      httpcommand = "GET";
      url = tor_strdup("/tor/status-vote/next/consensus-signatures.z");
      break;
    case DIR_PURPOSE_FETCH_SERVERDESC:
      tor_assert(resource);
      httpcommand = "GET";
      tor_asprintf(&url, "/tor/server/%s", resource);
      break;
    case DIR_PURPOSE_FETCH_EXTRAINFO:
      tor_assert(resource);
      httpcommand = "GET";
      tor_asprintf(&url, "/tor/extra/%s", resource);
      break;
    case DIR_PURPOSE_FETCH_MICRODESC:
      tor_assert(resource);
      httpcommand = "GET";
      tor_asprintf(&url, "/tor/micro/%s", resource);
      break;
    case DIR_PURPOSE_UPLOAD_DIR: {
      const char *why = router_get_descriptor_gen_reason();
      tor_assert(!resource);
      tor_assert(payload);
      httpcommand = "POST";
      url = tor_strdup("/tor/");
      if (!why) {
        why = "for no reason at all";
      }
      smartlist_add_asprintf(headers, "X-Desc-Gen-Reason: %s\r\n", why);
      break;
    }
    case DIR_PURPOSE_UPLOAD_VOTE:
      tor_assert(!resource);
      tor_assert(payload);
      httpcommand = "POST";
      url = tor_strdup("/tor/post/vote");
      break;
    case DIR_PURPOSE_UPLOAD_SIGNATURES:
      tor_assert(!resource);
      tor_assert(payload);
      httpcommand = "POST";
      url = tor_strdup("/tor/post/consensus-signature");
      break;
    case DIR_PURPOSE_FETCH_HSDESC:
      tor_assert(resource);
      tor_assert(strlen(resource) <= ED25519_BASE64_LEN);
      tor_assert(!payload);
      httpcommand = "GET";
      tor_asprintf(&url, "/tor/hs/3/%s", resource);
      break;
    case DIR_PURPOSE_UPLOAD_HSDESC:
      tor_assert(resource);
      tor_assert(payload);
      httpcommand = "POST";
      tor_asprintf(&url, "/tor/hs/%s/publish", resource);
      break;
    default:
      tor_assert(0);
      return;
  }

  /* warn in the non-tunneled case */
  if (direct && (strlen(proxystring) + strlen(url) >= 4096)) {
    log_warn(LD_BUG,
             "Squid does not like URLs longer than 4095 bytes, and this "
             "one is %d bytes long: %s%s",
             (int)(strlen(proxystring) + strlen(url)), proxystring, url);
  }

  tor_snprintf(request, sizeof(request), "%s %s", httpcommand, proxystring);

  request_len = strlen(request);
  total_request_len += request_len;
  connection_buf_add(request, request_len, TO_CONN(conn));

  url_len = strlen(url);
  total_request_len += url_len;
  connection_buf_add(url, url_len, TO_CONN(conn));
  tor_free(url);

  if (!strcmp(httpcommand, "POST") || payload) {
    smartlist_add_asprintf(headers, "Content-Length: %lu\r\n",
                 payload ? (unsigned long)payload_len : 0);
  }

  {
    char *header = smartlist_join_strings(headers, "", 0, NULL);
    tor_snprintf(request, sizeof(request), " HTTP/1.0\r\nHost: %s\r\n%s\r\n",
                 hoststring, header);
    tor_free(header);
  }

  request_len = strlen(request);
  total_request_len += request_len;
  connection_buf_add(request, request_len, TO_CONN(conn));

  if (payload) {
    /* then send the payload afterwards too */
    connection_buf_add(payload, payload_len, TO_CONN(conn));
    total_request_len += payload_len;
  }

  SMARTLIST_FOREACH(headers, char *, h, tor_free(h));
  smartlist_free(headers);

  log_debug(LD_DIR,
            "Sent request to directory server %s "
            "(purpose: %d, request size: %"TOR_PRIuSZ", "
            "payload size: %"TOR_PRIuSZ")",
            connection_describe_peer(TO_CONN(conn)),
            conn->base_.purpose,
            (total_request_len),
            (payload ? payload_len : 0));
}

/** Return true iff <b>body</b> doesn't start with a plausible router or
 * network-status or microdescriptor opening.  This is a sign of possible
 * compression. */
static int
body_is_plausible(const char *body, size_t len, int purpose)
{
  int i;
  if (len == 0)
    return 1; /* empty bodies don't need decompression */
  if (len < 32)
    return 0;
  if (purpose == DIR_PURPOSE_FETCH_MICRODESC) {
    return (!strcmpstart(body,"onion-key"));
  }

  if (!strcmpstart(body,"router") ||
      !strcmpstart(body,"network-status"))
    return 1;
  for (i=0;i<32;++i) {
    if (!TOR_ISPRINT(body[i]) && !TOR_ISSPACE(body[i]))
      return 0;
  }

  return 1;
}

/** Called when we've just fetched a bunch of router descriptors in
 * <b>body</b>.  The list <b>which</b>, if present, holds digests for
 * descriptors we requested: descriptor digests if <b>descriptor_digests</b>
 * is true, or identity digests otherwise.  Parse the descriptors, validate
 * them, and annotate them as having purpose <b>purpose</b> and as having been
 * downloaded from <b>source</b>.
 *
 * Return the number of routers actually added. */
static int
load_downloaded_routers(const char *body, smartlist_t *which,
                        int descriptor_digests,
                        int router_purpose,
                        const char *source)
{
  char buf[256];
  char time_buf[ISO_TIME_LEN+1];
  int added = 0;
  int general = router_purpose == ROUTER_PURPOSE_GENERAL;
  format_iso_time(time_buf, time(NULL));
  tor_assert(source);

  if (tor_snprintf(buf, sizeof(buf),
                   "@downloaded-at %s\n"
                   "@source %s\n"
                   "%s%s%s", time_buf, escaped(source),
                   !general ? "@purpose " : "",
                   !general ? router_purpose_to_string(router_purpose) : "",
                   !general ? "\n" : "")<0)
    return added;

  added = router_load_routers_from_string(body, NULL, SAVED_NOWHERE, which,
                                  descriptor_digests, buf);
  if (added && general)
    control_event_boot_dir(BOOTSTRAP_STATUS_LOADING_DESCRIPTORS,
                           count_loading_descriptors_progress());
  return added;
}

static int handle_response_fetch_certificate(dir_connection_t *,
                                             const response_handler_args_t *);
static int handle_response_fetch_status_vote(dir_connection_t *,
                                             const response_handler_args_t *);
static int handle_response_fetch_detached_signatures(dir_connection_t *,
                                             const response_handler_args_t *);
static int handle_response_fetch_desc(dir_connection_t *,
                                             const response_handler_args_t *);
static int handle_response_upload_dir(dir_connection_t *,
                                      const response_handler_args_t *);
static int handle_response_upload_vote(dir_connection_t *,
                                       const response_handler_args_t *);
static int handle_response_upload_signatures(dir_connection_t *,
                                             const response_handler_args_t *);
static int handle_response_upload_hsdesc(dir_connection_t *,
                                         const response_handler_args_t *);

static int
dir_client_decompress_response_body(char **bodyp, size_t *bodylenp,
                                    dir_connection_t *conn,
                                    compress_method_t compression,
                                    int anonymized_connection)
{
  int rv = 0;
  const char *body = *bodyp;
  size_t body_len = *bodylenp;
  int allow_partial = (conn->base_.purpose == DIR_PURPOSE_FETCH_SERVERDESC ||
                       conn->base_.purpose == DIR_PURPOSE_FETCH_EXTRAINFO ||
                       conn->base_.purpose == DIR_PURPOSE_FETCH_MICRODESC);

  int plausible = body_is_plausible(body, body_len, conn->base_.purpose);

  if (plausible && compression == NO_METHOD) {
    return 0;
  }

  int severity = LOG_DEBUG;
  char *new_body = NULL;
  size_t new_len = 0;
  const char *description1, *description2;
  int want_to_try_both = 0;
  int tried_both = 0;
  compress_method_t guessed = detect_compression_method(body, body_len);

  description1 = compression_method_get_human_name(compression);

  if (BUG(description1 == NULL))
    description1 = compression_method_get_human_name(UNKNOWN_METHOD);

  if (guessed == UNKNOWN_METHOD && !plausible)
    description2 = "confusing binary junk";
  else
    description2 = compression_method_get_human_name(guessed);

  /* Tell the user if we don't believe what we're told about compression.*/
  want_to_try_both = (compression == UNKNOWN_METHOD ||
                      guessed != compression);
  if (want_to_try_both) {
    severity = LOG_PROTOCOL_WARN;
  }

  tor_log(severity, LD_HTTP,
          "HTTP body from %s was labeled as %s, "
          "%s it seems to be %s.%s",
          connection_describe(TO_CONN(conn)),
          description1,
          guessed != compression?"but":"and",
          description2,
          (compression>0 && guessed>0 && want_to_try_both)?
          "  Trying both.":"");

  /* Try declared compression first if we can.
   * tor_compress_supports_method() also returns true for NO_METHOD.
   * Ensure that the server is not sending us data compressed using a
   * compression method that is not allowed for anonymous connections. */
  if (anonymized_connection &&
      ! allowed_anonymous_connection_compression_method(compression)) {
    warn_disallowed_anonymous_compression_method(compression);
    rv = -1;
    goto done;
  }

  if (tor_compress_supports_method(compression)) {
    tor_uncompress(&new_body, &new_len, body, body_len, compression,
                   !allow_partial, LOG_PROTOCOL_WARN);
    if (new_body) {
      /* We succeeded with the declared compression method. Great! */
      rv = 0;
      goto done;
    }
  }

  /* Okay, if that didn't work, and we think that it was compressed
   * differently, try that. */
  if (anonymized_connection &&
      ! allowed_anonymous_connection_compression_method(guessed)) {
    warn_disallowed_anonymous_compression_method(guessed);
    rv = -1;
    goto done;
  }

  if (tor_compress_supports_method(guessed) &&
      compression != guessed) {
    tor_uncompress(&new_body, &new_len, body, body_len, guessed,
                   !allow_partial, LOG_INFO);
    tried_both = 1;
  }
  /* If we're pretty sure that we have a compressed directory, and
   * we didn't manage to uncompress it, then warn and bail. */
  if (!plausible && !new_body) {
    static ratelim_t warning_limit = RATELIM_INIT(60 * 60);
    log_fn_ratelim(&warning_limit, LOG_WARN, LD_HTTP,
           "Unable to decompress HTTP body (tried %s%s%s, on %s).",
           description1,
           tried_both?" and ":"",
           tried_both?description2:"",
           connection_describe(TO_CONN(conn)));
    rv = -1;
    goto done;
  }

 done:
  if (new_body) {
    if (rv == 0) {
      /* success! */
      tor_free(*bodyp);
      *bodyp = new_body;
      *bodylenp = new_len;
    } else {
      tor_free(new_body);
    }
  }

  return rv;
}

/**
 * Total number of bytes downloaded of each directory purpose, when
 * bootstrapped, and when not bootstrapped.
 *
 * (For example, the number of bytes downloaded of purpose p while
 * not fully bootstrapped is total_dl[p][false].)
 **/
static uint64_t total_dl[DIR_PURPOSE_MAX_][2];

/**
 * Heartbeat: dump a summary of how many bytes of which purpose we've
 * downloaded, when bootstrapping and when not bootstrapping.
 **/
void
dirclient_dump_total_dls(void)
{
  const or_options_t *options = get_options();
  for (int bootstrapped = 0; bootstrapped < 2; ++bootstrapped) {
    smartlist_t *lines = smartlist_new();
    for (int i=0; i < DIR_PURPOSE_MAX_; ++i) {
      uint64_t n = total_dl[i][bootstrapped];
      if (n == 0)
        continue;
      if (options->SafeLogging_ != SAFELOG_SCRUB_NONE &&
          purpose_needs_anonymity(i, ROUTER_PURPOSE_GENERAL, NULL))
        continue;
      smartlist_add_asprintf(lines, "%"PRIu64" (%s)",
                             n, dir_conn_purpose_to_string(i));
    }

    if (smartlist_len(lines) > 0) {
      char *log_line = smartlist_join_strings(lines, "; ", 0, NULL);
      log_notice(LD_NET, "While %sbootstrapping, fetched this many bytes: %s",
                 bootstrapped?"not ":"", log_line);
      tor_free(log_line);

      SMARTLIST_FOREACH(lines, char *, s, tor_free(s));
    }
    smartlist_free(lines);
  }
}

/** We are a client, and we've finished reading the server's
 * response. Parse it and act appropriately.
 *
 * If we're still happy with using this directory server in the future, return
 * 0. Otherwise return -1; and the caller should consider trying the request
 * again.
 *
 * The caller will take care of marking the connection for close.
 */
static int
connection_dir_client_reached_eof(dir_connection_t *conn)
{
  char *body = NULL;
  char *headers = NULL;
  char *reason = NULL;
  size_t body_len = 0;
  int status_code;
  time_t date_header = 0;
  long apparent_skew;
  compress_method_t compression;
  int skewed = 0;
  int rv;
  int allow_partial = (conn->base_.purpose == DIR_PURPOSE_FETCH_SERVERDESC ||
                       conn->base_.purpose == DIR_PURPOSE_FETCH_EXTRAINFO ||
                       conn->base_.purpose == DIR_PURPOSE_FETCH_MICRODESC);
  size_t received_bytes;
  const int anonymized_connection =
    purpose_needs_anonymity(conn->base_.purpose,
                            conn->router_purpose,
                            conn->requested_resource);

  received_bytes = connection_get_inbuf_len(TO_CONN(conn));

  log_debug(LD_DIR, "Downloaded %"TOR_PRIuSZ" bytes on connection of purpose "
             "%s; bootstrap %d%%",
             received_bytes,
             dir_conn_purpose_to_string(conn->base_.purpose),
             control_get_bootstrap_percent());
  {
    bool bootstrapped = control_get_bootstrap_percent() == 100;
    total_dl[conn->base_.purpose][bootstrapped] += received_bytes;
  }

  switch (connection_fetch_from_buf_http(TO_CONN(conn),
                              &headers, MAX_HEADERS_SIZE,
                              &body, &body_len, MAX_DIR_DL_SIZE,
                              allow_partial)) {
    case -1: /* overflow */
      log_warn(LD_PROTOCOL,
               "'fetch' response too large (%s). Closing.",
               connection_describe(TO_CONN(conn)));
      return -1;
    case 0:
      log_info(LD_HTTP,
               "'fetch' response not all here, but we're at eof. Closing.");
      return -1;
    /* case 1, fall through */
  }

  if (parse_http_response(headers, &status_code, &date_header,
                          &compression, &reason) < 0) {
    log_warn(LD_HTTP,"Unparseable headers (%s). Closing.",
             connection_describe(TO_CONN(conn)));
    rv = -1;
    goto done;
  }
  if (!reason) reason = tor_strdup("[no reason given]");

  tor_log(LOG_DEBUG, LD_DIR,
            "Received response on %s: %d %s "
            "(purpose: %d, response size: %"TOR_PRIuSZ
#ifdef MEASUREMENTS_21206
            ", data cells received: %d, data cells sent: %d"
#endif
            ", compression: %d)",
            connection_describe(TO_CONN(conn)),
            status_code,
            escaped(reason), conn->base_.purpose,
            (received_bytes),
#ifdef MEASUREMENTS_21206
            conn->data_cells_received, conn->data_cells_sent,
#endif
            compression);

  if (conn->guard_state) {
    /* we count the connection as successful once we can read from it.  We do
     * not, however, delay use of the circuit here, since it's just for a
     * one-hop directory request. */
    /* XXXXprop271 note that this will not do the right thing for other
     * waiting circuits that would be triggered by this circuit becoming
     * complete/usable. But that's ok, I think.
     */
    entry_guard_succeeded(&conn->guard_state);
    circuit_guard_state_free(conn->guard_state);
    conn->guard_state = NULL;
  }

  /* now check if it's got any hints for us about our IP address. */
  if (conn->dirconn_direct) {
    char *guess = http_get_header(headers, X_ADDRESS_HEADER);
    if (guess) {
      tor_addr_t addr;
      if (tor_addr_parse(&addr, guess) < 0) {
        log_debug(LD_DIR, "Malformed X-Your-Address-Is header %s. Ignoring.",
                  escaped(guess));
      } else {
        relay_address_new_suggestion(&addr, &TO_CONN(conn)->addr, NULL);
      }
      tor_free(guess);
    }
  }

  if (date_header > 0) {
    /* The date header was written very soon after we sent our request,
     * so compute the skew as the difference between sending the request
     * and the date header.  (We used to check now-date_header, but that's
     * inaccurate if we spend a lot of time downloading.)
     */
    apparent_skew = conn->base_.timestamp_last_write_allowed - date_header;
    if (labs(apparent_skew)>ALLOW_DIRECTORY_TIME_SKEW) {
      int trusted = router_digest_is_trusted_dir(conn->identity_digest);
      clock_skew_warning(TO_CONN(conn), apparent_skew, trusted, LD_HTTP,
                         "directory", "DIRSERV");
      skewed = 1; /* don't check the recommended-versions line */
    } else {
      log_debug(LD_HTTP, "Time on received directory is within tolerance; "
                "we are %ld seconds skewed.  (That's okay.)", apparent_skew);
    }
  }
  (void) skewed; /* skewed isn't used yet. */

  if (status_code == 503) {
    routerstatus_t *rs;
    dir_server_t *ds;
    const char *id_digest = conn->identity_digest;
    log_info(LD_DIR,"Received http status code %d (%s) from server "
             "%s. I'll try again soon.",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)));
    time_t now = approx_time();
    if ((rs = router_get_mutable_consensus_status_by_id(id_digest)))
      rs->last_dir_503_at = now;
    if ((ds = router_get_fallback_dirserver_by_digest(id_digest)))
      ds->fake_status.last_dir_503_at = now;

    rv = -1;
    goto done;
  }

  if (dir_client_decompress_response_body(&body, &body_len,
                             conn, compression, anonymized_connection) < 0) {
    rv = -1;
    goto done;
  }

  response_handler_args_t args;
  memset(&args, 0, sizeof(args));
  args.status_code = status_code;
  args.reason = reason;
  args.body = body;
  args.body_len = body_len;
  args.headers = headers;

  switch (conn->base_.purpose) {
    case DIR_PURPOSE_FETCH_CONSENSUS:
      rv = handle_response_fetch_consensus(conn, &args);
      break;
    case DIR_PURPOSE_FETCH_CERTIFICATE:
      rv = handle_response_fetch_certificate(conn, &args);
      break;
    case DIR_PURPOSE_FETCH_STATUS_VOTE:
      rv = handle_response_fetch_status_vote(conn, &args);
      break;
    case DIR_PURPOSE_FETCH_DETACHED_SIGNATURES:
      rv = handle_response_fetch_detached_signatures(conn, &args);
      break;
    case DIR_PURPOSE_FETCH_SERVERDESC:
    case DIR_PURPOSE_FETCH_EXTRAINFO:
      rv = handle_response_fetch_desc(conn, &args);
      break;
    case DIR_PURPOSE_FETCH_MICRODESC:
      rv = handle_response_fetch_microdesc(conn, &args);
      break;
    case DIR_PURPOSE_UPLOAD_DIR:
      rv = handle_response_upload_dir(conn, &args);
      break;
    case DIR_PURPOSE_UPLOAD_SIGNATURES:
      rv = handle_response_upload_signatures(conn, &args);
      break;
    case DIR_PURPOSE_UPLOAD_VOTE:
      rv = handle_response_upload_vote(conn, &args);
      break;
    case DIR_PURPOSE_UPLOAD_HSDESC:
      rv = handle_response_upload_hsdesc(conn, &args);
      break;
    case DIR_PURPOSE_FETCH_HSDESC:
      rv = handle_response_fetch_hsdesc_v3(conn, &args);
      break;
    default:
      tor_assert_nonfatal_unreached();
      rv = -1;
      break;
  }

 done:
  tor_free(body);
  tor_free(headers);
  tor_free(reason);
  return rv;
}

/**
 * Handler function: processes a response to a request for a networkstatus
 * consensus document by checking the consensus, storing it, and marking
 * router requests as reachable.
 **/
STATIC int
handle_response_fetch_consensus(dir_connection_t *conn,
                                const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_CONSENSUS);
  const int status_code = args->status_code;
  const char *body = args->body;
  const size_t body_len = args->body_len;
  const char *reason = args->reason;
  const time_t now = approx_time();

  const char *consensus;
  char *new_consensus = NULL;
  const char *sourcename;

  int r;
  const char *flavname = conn->requested_resource;
  if (status_code != 200) {
    int severity = (status_code == 304) ? LOG_INFO : LOG_WARN;
    tor_log(severity, LD_DIR,
            "Received http status code %d (%s) from server "
            "%s while fetching consensus directory.",
            status_code, escaped(reason),
            connection_describe_peer(TO_CONN(conn)));
    networkstatus_consensus_download_failed(status_code, flavname);
    return -1;
  }

  if (looks_like_a_consensus_diff(body, body_len)) {
    /* First find our previous consensus. Maybe it's in ram, maybe not. */
    cached_dir_t *cd = NULL;
    const char *consensus_body = NULL;
    size_t consensus_body_len;
    tor_mmap_t *mapped_consensus = NULL;

    /* We prefer the mmap'd version over the cached_dir_t version,
     * since that matches the logic we used when we picked a consensus
     * back in dir_consensus_request_set_additional_headers. */
    mapped_consensus = networkstatus_map_cached_consensus(flavname);
    if (mapped_consensus) {
      consensus_body = mapped_consensus->data;
      consensus_body_len = mapped_consensus->size;
    } else {
      cd = dirserv_get_consensus(flavname);
      if (cd) {
        consensus_body = cd->dir;
        consensus_body_len = cd->dir_len;
      }
    }
    if (!consensus_body) {
      log_warn(LD_DIR, "Received a consensus diff, but we can't find "
               "any %s-flavored consensus in our current cache.",flavname);
      tor_munmap_file(mapped_consensus);
      networkstatus_consensus_download_failed(0, flavname);
      // XXXX if this happens too much, see below
      return -1;
    }

    new_consensus = consensus_diff_apply(consensus_body, consensus_body_len,
                                         body, body_len);
    tor_munmap_file(mapped_consensus);
    if (new_consensus == NULL) {
      log_warn(LD_DIR, "Could not apply consensus diff received from server "
               "%s", connection_describe_peer(TO_CONN(conn)));
      // XXXX If this happens too many times, we should maybe not use
      // XXXX this directory for diffs any more?
      networkstatus_consensus_download_failed(0, flavname);
      return -1;
    }
    log_info(LD_DIR, "Applied consensus diff (size %d) from server "
             "%s, resulting in a new consensus document (size %d).",
             (int)body_len, connection_describe_peer(TO_CONN(conn)),
             (int)strlen(new_consensus));
    consensus = new_consensus;
    sourcename = "generated based on a diff";
  } else {
    log_info(LD_DIR,"Received consensus directory (body size %d) from server "
             "%s", (int)body_len, connection_describe_peer(TO_CONN(conn)));
    consensus = body;
    sourcename = "downloaded";
  }

  if ((r=networkstatus_set_current_consensus(consensus,
                                             strlen(consensus),
                                             flavname, 0,
                                             conn->identity_digest))<0) {
    log_fn(r<-1?LOG_WARN:LOG_INFO, LD_DIR,
           "Unable to load %s consensus directory %s from "
           "server %s. I'll try again soon.",
           flavname, sourcename,
           connection_describe_peer(TO_CONN(conn)));
    networkstatus_consensus_download_failed(0, flavname);
    tor_free(new_consensus);
    return -1;
  }

  /* If we launched other fetches for this consensus, cancel them. */
  connection_dir_close_consensus_fetches(conn, flavname);

  /* update the list of routers and directory guards */
  routers_update_all_from_networkstatus(now, 3);
  update_microdescs_from_networkstatus(now);
  directory_info_has_arrived(now, 0, 0);

  if (authdir_mode_v3(get_options())) {
    sr_act_post_consensus(
                     networkstatus_get_latest_consensus_by_flavor(FLAV_NS));
  }
  log_info(LD_DIR, "Successfully loaded consensus.");

  tor_free(new_consensus);
  return 0;
}

/**
 * Handler function: processes a response to a request for one or more
 * authority certificates
 **/
static int
handle_response_fetch_certificate(dir_connection_t *conn,
                                  const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_CERTIFICATE);
  const int status_code = args->status_code;
  const char *reason = args->reason;
  const char *body = args->body;
  const size_t body_len = args->body_len;

  if (status_code != 200) {
    log_warn(LD_DIR,
             "Received http status code %d (%s) from server "
             "%s while fetching \"/tor/keys/%s\".",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)),
             conn->requested_resource);
    connection_dir_download_cert_failed(conn, status_code);
    return -1;
  }
  log_info(LD_DIR,"Received authority certificates (body size %d) from "
           "server %s",
           (int)body_len, connection_describe_peer(TO_CONN(conn)));

  /*
   * Tell trusted_dirs_load_certs_from_string() whether it was by fp
   * or fp-sk pair.
   */
  int src_code = -1;
  if (!strcmpstart(conn->requested_resource, "fp/")) {
    src_code = TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST;
  } else if (!strcmpstart(conn->requested_resource, "fp-sk/")) {
    src_code = TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_SK_DIGEST;
  }

  if (src_code != -1) {
    if (trusted_dirs_load_certs_from_string(body, src_code, 1,
                                            conn->identity_digest)<0) {
      log_warn(LD_DIR, "Unable to parse fetched certificates");
      /* if we fetched more than one and only some failed, the successful
       * ones got flushed to disk so it's safe to call this on them */
      connection_dir_download_cert_failed(conn, status_code);
    } else {
      time_t now = approx_time();
      directory_info_has_arrived(now, 0, 0);
      log_info(LD_DIR, "Successfully loaded certificates from fetch.");
    }
  } else {
    log_warn(LD_DIR,
             "Couldn't figure out what to do with fetched certificates for "
             "unknown resource %s",
             conn->requested_resource);
    connection_dir_download_cert_failed(conn, status_code);
  }
  return 0;
}

/**
 * Handler function: processes a response to a request for an authority's
 * current networkstatus vote.
 **/
static int
handle_response_fetch_status_vote(dir_connection_t *conn,
                                  const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_STATUS_VOTE);
  const int status_code = args->status_code;
  const char *reason = args->reason;
  const char *body = args->body;
  const size_t body_len = args->body_len;

  const char *msg;
  int st;
  log_notice(LD_DIR,"Got votes (body size %d) from server %s",
             (int)body_len, connection_describe_peer(TO_CONN(conn)));
  if (status_code != 200) {
    log_warn(LD_DIR,
             "Received http status code %d (%s) from server "
             "%s while fetching \"/tor/status-vote/next/%s.z\".",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)),
             conn->requested_resource);
    return -1;
  }
  dirvote_add_vote(body, 0, TO_CONN(conn)->address, &msg, &st);
  if (st > 299) {
    log_warn(LD_DIR, "Error adding retrieved vote: %s", msg);
  } else {
    log_info(LD_DIR, "Added vote(s) successfully [msg: %s]", msg);
  }

  return 0;
}

/**
 * Handler function: processes a response to a request for the signatures
 * that an authority knows about on a given consensus.
 **/
static int
handle_response_fetch_detached_signatures(dir_connection_t *conn,
                                          const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_DETACHED_SIGNATURES);
  const int status_code = args->status_code;
  const char *reason = args->reason;
  const char *body = args->body;
  const size_t body_len = args->body_len;

  const char *msg = NULL;
  log_info(LD_DIR,"Got detached signatures (body size %d) from server %s",
           (int)body_len,
           connection_describe_peer(TO_CONN(conn)));
  if (status_code != 200) {
    log_warn(LD_DIR,
        "Received http status code %d (%s) from server %s while fetching "
        "\"/tor/status-vote/next/consensus-signatures.z\".",
        status_code, escaped(reason),
        connection_describe_peer(TO_CONN(conn)));
    return -1;
  }
  if (dirvote_add_signatures(body, conn->base_.address, &msg)<0) {
    log_warn(LD_DIR, "Problem adding detached signatures from %s: %s",
             connection_describe_peer(TO_CONN(conn)),
             msg?msg:"???");
  }

  return 0;
}

/**
 * Handler function: processes a response to a request for a group of server
 * descriptors or an extrainfo documents.
 **/
static int
handle_response_fetch_desc(dir_connection_t *conn,
                           const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_SERVERDESC ||
             conn->base_.purpose == DIR_PURPOSE_FETCH_EXTRAINFO);
  const int status_code = args->status_code;
  const char *reason = args->reason;
  const char *body = args->body;
  const size_t body_len = args->body_len;

  int was_ei = conn->base_.purpose == DIR_PURPOSE_FETCH_EXTRAINFO;
  smartlist_t *which = NULL;
  int n_asked_for = 0;
  int descriptor_digests = conn->requested_resource &&
    !strcmpstart(conn->requested_resource,"d/");
  log_info(LD_DIR,"Received %s (body size %d) from server %s",
           was_ei ? "extra server info" : "server info",
           (int)body_len, connection_describe_peer(TO_CONN(conn)));
  if (conn->requested_resource &&
      (!strcmpstart(conn->requested_resource,"d/") ||
       !strcmpstart(conn->requested_resource,"fp/"))) {
    which = smartlist_new();
    dir_split_resource_into_fingerprints(conn->requested_resource +
                                         (descriptor_digests ? 2 : 3),
                                         which, NULL, 0);
    n_asked_for = smartlist_len(which);
  }
  if (status_code != 200) {
    int dir_okay = status_code == 404 ||
      (status_code == 400 && !strcmp(reason, "Servers unavailable.")) ||
       status_code == 301;
    /* 404 means that it didn't have them; no big deal.
     * Older (pre-0.1.1.8) servers said 400 Servers unavailable instead.
     * 301 is considered as an error since Tor does not follow redirects,
     * which means we failed to reach the server we wanted. */
    log_fn(dir_okay ? LOG_INFO : LOG_WARN, LD_DIR,
           "Received http status code %d (%s) from server %s "
           "while fetching \"/tor/server/%s\". I'll try again soon.",
           status_code, escaped(reason),
           connection_describe_peer(TO_CONN(conn)),
           conn->requested_resource);
    if (!which) {
      connection_dir_download_routerdesc_failed(conn);
    } else {
      dir_routerdesc_download_failed(which, status_code,
                                     conn->router_purpose,
                                     was_ei, descriptor_digests);
      SMARTLIST_FOREACH(which, char *, cp, tor_free(cp));
      smartlist_free(which);
    }
    return dir_okay ? 0 : -1;
  }
  /* Learn the routers, assuming we requested by fingerprint or "all"
   * or "authority".
   *
   * We use "authority" to fetch our own descriptor for
   * testing, and to fetch bridge descriptors for bootstrapping. Ignore
   * the output of "authority" requests unless we are using bridges,
   * since otherwise they'll be the response from reachability tests,
   * and we don't really want to add that to our routerlist. */
  if (which || (conn->requested_resource &&
                (!strcmpstart(conn->requested_resource, "all") ||
                 (!strcmpstart(conn->requested_resource, "authority") &&
                  get_options()->UseBridges)))) {
    /* as we learn from them, we remove them from 'which' */
    if (was_ei) {
      router_load_extrainfo_from_string(body, NULL, SAVED_NOWHERE, which,
                                        descriptor_digests);
    } else {
      //router_load_routers_from_string(body, NULL, SAVED_NOWHERE, which,
      //                       descriptor_digests, conn->router_purpose);
      if (load_downloaded_routers(body, which, descriptor_digests,
                                  conn->router_purpose,
                                  conn->base_.address)) {
        time_t now = approx_time();
        directory_info_has_arrived(now, 0, 1);
      }
    }
  }
  if (which) { /* mark remaining ones as failed */
    log_info(LD_DIR, "Received %d/%d %s requested from %s",
             n_asked_for-smartlist_len(which), n_asked_for,
             was_ei ? "extra-info documents" : "router descriptors",
             connection_describe_peer(TO_CONN(conn)));
    if (smartlist_len(which)) {
      dir_routerdesc_download_failed(which, status_code,
                                     conn->router_purpose,
                                     was_ei, descriptor_digests);
    }
    SMARTLIST_FOREACH(which, char *, cp, tor_free(cp));
    smartlist_free(which);
  }

  return 0;
}

/**
 * Handler function: processes a response to a request for a group of
 * microdescriptors
 **/
STATIC int
handle_response_fetch_microdesc(dir_connection_t *conn,
                                const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_FETCH_MICRODESC);
  const int status_code = args->status_code;
  const char *reason = args->reason;
  const char *body = args->body;
  const size_t body_len = args->body_len;

  smartlist_t *which = NULL;
  log_info(LD_DIR,"Received answer to microdescriptor request (status %d, "
           "body size %d) from server %s",
           status_code, (int)body_len,
           connection_describe_peer(TO_CONN(conn)));
  tor_assert(conn->requested_resource &&
             !strcmpstart(conn->requested_resource, "d/"));
  tor_assert_nonfatal(!fast_mem_is_zero(conn->identity_digest, DIGEST_LEN));
  which = smartlist_new();
  dir_split_resource_into_fingerprints(conn->requested_resource+2,
                                       which, NULL,
                                       DSR_DIGEST256|DSR_BASE64);
  if (status_code != 200) {
    log_info(LD_DIR, "Received status code %d (%s) from server "
             "%s while fetching \"/tor/micro/%s\".  I'll try again "
             "soon.",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)),
             conn->requested_resource);
    dir_microdesc_download_failed(which, status_code, conn->identity_digest);
    SMARTLIST_FOREACH(which, char *, cp, tor_free(cp));
    smartlist_free(which);
    return 0;
  } else {
    smartlist_t *mds;
    time_t now = approx_time();
    mds = microdescs_add_to_cache(get_microdesc_cache(),
                                  body, body+body_len, SAVED_NOWHERE, 0,
                                  now, which);
    if (smartlist_len(which)) {
      /* Mark remaining ones as failed. */
      dir_microdesc_download_failed(which, status_code, conn->identity_digest);
    }
    if (mds && smartlist_len(mds)) {
      control_event_boot_dir(BOOTSTRAP_STATUS_LOADING_DESCRIPTORS,
                             count_loading_descriptors_progress());
      directory_info_has_arrived(now, 0, 1);
    }
    SMARTLIST_FOREACH(which, char *, cp, tor_free(cp));
    smartlist_free(which);
    smartlist_free(mds);
  }

  return 0;
}

/**
 * Handler function: processes a response to a POST request to upload our
 * router descriptor.
 **/
static int
handle_response_upload_dir(dir_connection_t *conn,
                           const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_UPLOAD_DIR);
  const int status_code = args->status_code;
  const char *reason = args->reason;
  const char *headers = args->headers;

  switch (status_code) {
  case 200: {
    dir_server_t *ds =
      router_get_trusteddirserver_by_digest(conn->identity_digest);
    char *rejected_hdr = http_get_header(headers,
                                         "X-Descriptor-Not-New: ");
    if (rejected_hdr) {
      if (!strcmp(rejected_hdr, "Yes")) {
        log_info(LD_GENERAL,
                 "Authority '%s' declined our descriptor (not new)",
                 ds->nickname);
        /* XXXX use this information; be sure to upload next one
         * sooner. -NM */
        /* XXXX++ On further thought, the task above implies that we're
         * basing our regenerate-descriptor time on when we uploaded the
         * last descriptor, not on the published time of the last
         * descriptor.  If those are different, that's a bad thing to
         * do. -NM */
      }
      tor_free(rejected_hdr);
    }
    log_info(LD_GENERAL,"eof (status 200) after uploading server "
             "descriptor: finished.");
    control_event_server_status(
                   LOG_NOTICE, "ACCEPTED_SERVER_DESCRIPTOR DIRAUTH=%s:%d",
                   conn->base_.address, conn->base_.port);

    ds->has_accepted_serverdesc = 1;
    if (directories_have_accepted_server_descriptor())
      control_event_server_status(LOG_NOTICE, "GOOD_SERVER_DESCRIPTOR");
  }
    break;
  case 400:
    log_warn(LD_GENERAL,"http status 400 (%s) response from "
             "dirserver %s. Please correct.",
             escaped(reason), connection_describe_peer(TO_CONN(conn)));
    control_event_server_status(LOG_WARN,
                    "BAD_SERVER_DESCRIPTOR DIRAUTH=%s:%d REASON=\"%s\"",
                    conn->base_.address, conn->base_.port, escaped(reason));
    break;
  default:
    log_warn(LD_GENERAL,
             "HTTP status %d (%s) was unexpected while uploading "
             "descriptor to server %s'. Possibly the server is "
             "misconfigured?",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)));
    break;
  }
  /* return 0 in all cases, since we don't want to mark any
   * dirservers down just because they don't like us. */

  return 0;
}

/**
 * Handler function: processes a response to POST request to upload our
 * own networkstatus vote.
 **/
static int
handle_response_upload_vote(dir_connection_t *conn,
                            const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_UPLOAD_VOTE);
  const int status_code = args->status_code;
  const char *reason = args->reason;

  switch (status_code) {
  case 200: {
    log_notice(LD_DIR,"Uploaded my vote to dirserver %s",
               connection_describe_peer(TO_CONN(conn)));
  }
    break;
  case 400:
    log_warn(LD_DIR,"http status 400 (%s) response after uploading "
             "vote to dirserver %s. Please correct.",
             escaped(reason), connection_describe_peer(TO_CONN(conn)));
    break;
  default:
    log_warn(LD_GENERAL,
             "HTTP status %d (%s) was unexpected while uploading "
             "vote to server %s.",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)));
    break;
  }
  /* return 0 in all cases, since we don't want to mark any
   * dirservers down just because they don't like us. */
  return 0;
}

/**
 * Handler function: processes a response to POST request to upload our
 * view of the signatures on the current consensus.
 **/
static int
handle_response_upload_signatures(dir_connection_t *conn,
                                  const response_handler_args_t *args)
{
  tor_assert(conn->base_.purpose == DIR_PURPOSE_UPLOAD_SIGNATURES);
  const int status_code = args->status_code;
  const char *reason = args->reason;

  switch (status_code) {
  case 200: {
    log_notice(LD_DIR,"Uploaded signature(s) to dirserver %s",
               connection_describe_peer(TO_CONN(conn)));
  }
    break;
  case 400:
    log_warn(LD_DIR,"http status 400 (%s) response after uploading "
             "signatures to dirserver %s. Please correct.",
             escaped(reason), connection_describe_peer(TO_CONN(conn)));
    break;
  default:
    log_warn(LD_GENERAL,
             "HTTP status %d (%s) was unexpected while uploading "
             "signatures to server %s.",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)));
    break;
  }
  /* return 0 in all cases, since we don't want to mark any
   * dirservers down just because they don't like us. */

  return 0;
}

/**
 * Handler function: processes a response to a request for a v3 hidden service
 * descriptor.
 **/
STATIC int
handle_response_fetch_hsdesc_v3(dir_connection_t *conn,
                                const response_handler_args_t *args)
{
  const int status_code = args->status_code;
  const char *reason = args->reason;
  const char *body = args->body;
  const size_t body_len = args->body_len;

  tor_assert(conn->hs_ident);

  log_info(LD_REND,"Received v3 hsdesc (body size %d, status %d (%s))",
           (int)body_len, status_code, escaped(reason));

  hs_client_dir_fetch_done(conn, reason, body, status_code);
  return 0;
}

/**
 * Handler function: processes a response to a POST request to upload an
 * hidden service descriptor.
 **/
static int
handle_response_upload_hsdesc(dir_connection_t *conn,
                              const response_handler_args_t *args)
{
  const int status_code = args->status_code;
  const char *reason = args->reason;

  tor_assert(conn);
  tor_assert(conn->base_.purpose == DIR_PURPOSE_UPLOAD_HSDESC);

  log_info(LD_REND, "Uploaded hidden service descriptor (status %d "
                    "(%s))",
           status_code, escaped(reason));
  /* For this directory response, it MUST have an hidden service identifier on
   * this connection. */
  tor_assert(conn->hs_ident);
  switch (status_code) {
  case 200:
    log_info(LD_REND, "Uploading hidden service descriptor: "
                      "finished with status 200 (%s)", escaped(reason));
    hs_control_desc_event_uploaded(conn->hs_ident, conn->identity_digest);
    break;
  case 400:
    log_fn(LOG_PROTOCOL_WARN, LD_REND,
           "Uploading hidden service descriptor: http "
           "status 400 (%s) response from dirserver "
           "%s. Malformed hidden service descriptor?",
           escaped(reason), connection_describe_peer(TO_CONN(conn)));
    hs_control_desc_event_failed(conn->hs_ident, conn->identity_digest,
                                 "UPLOAD_REJECTED");
    break;
  default:
    log_warn(LD_REND, "Uploading hidden service descriptor: http "
                      "status %d (%s) response unexpected (server "
                      "%s').",
             status_code, escaped(reason),
             connection_describe_peer(TO_CONN(conn)));
    hs_control_desc_event_failed(conn->hs_ident, conn->identity_digest,
                                 "UNEXPECTED");
    break;
  }

  return 0;
}

/** Called when a directory connection reaches EOF. */
int
connection_dir_reached_eof(dir_connection_t *conn)
{
  int retval;
  if (conn->base_.state != DIR_CONN_STATE_CLIENT_READING) {
    log_info(LD_HTTP,"conn reached eof, not reading. [state=%d] Closing.",
             conn->base_.state);
    connection_close_immediate(TO_CONN(conn)); /* error: give up on flushing */
    connection_mark_for_close(TO_CONN(conn));
    return -1;
  }

  retval = connection_dir_client_reached_eof(conn);
  if (retval == 0) /* success */
    conn->base_.state = DIR_CONN_STATE_CLIENT_FINISHED;
  connection_mark_for_close(TO_CONN(conn));
  return retval;
}
/** We are closing a dir connection: If <b>dir_conn</b> is a dir connection
 *  that tried to fetch an HS descriptor, check if it successfully fetched it,
 *  or if we need to try again. */
void
connection_dir_client_refetch_hsdesc_if_needed(dir_connection_t *dir_conn)
{
  connection_t *conn = TO_CONN(dir_conn);

  /* Check for v3 rend desc fetch */
  if (conn->purpose == DIR_PURPOSE_FETCH_HSDESC &&
      dir_conn->hs_ident &&
      !ed25519_public_key_is_zero(&dir_conn->hs_ident->identity_pk)) {
    hs_client_refetch_hsdesc(&dir_conn->hs_ident->identity_pk);
  }
}

/** Array of compression methods to use (if supported) for requesting
 * compressed data, ordered from best to worst. */
static compress_method_t client_meth_pref[] = {
  LZMA_METHOD,
  ZSTD_METHOD,
  ZLIB_METHOD,
  GZIP_METHOD,
  NO_METHOD
};

/** Array of allowed compression methods to use (if supported) when receiving a
 * response from a request that was required to be anonymous. */
static compress_method_t client_meth_allowed_anonymous_compression[] = {
  ZLIB_METHOD,
  GZIP_METHOD,
  NO_METHOD
};

/** Return a newly allocated string containing a comma separated list of
 * supported encodings. */
STATIC char *
accept_encoding_header(void)
{
  smartlist_t *methods = smartlist_new();
  char *header = NULL;
  compress_method_t method;
  unsigned i;

  for (i = 0; i < ARRAY_LENGTH(client_meth_pref); ++i) {
    method = client_meth_pref[i];
    if (tor_compress_supports_method(method))
      smartlist_add(methods, (char *)compression_method_get_name(method));
  }

  header = smartlist_join_strings(methods, ", ", 0, NULL);
  smartlist_free(methods);

  return header;
}

/** Check if the given compression method is allowed for a connection that is
 * supposed to be anonymous. Returns 1 if the compression method is allowed,
 * otherwise 0. */
STATIC int
allowed_anonymous_connection_compression_method(compress_method_t method)
{
  unsigned u;

  for (u = 0; u < ARRAY_LENGTH(client_meth_allowed_anonymous_compression);
       ++u) {
    compress_method_t allowed_method =
      client_meth_allowed_anonymous_compression[u];

    if (! tor_compress_supports_method(allowed_method))
      continue;

    if (method == allowed_method)
      return 1;
  }

  return 0;
}

/** Log a warning when a remote server has sent us a document using a
 * compression method that is not allowed for anonymous directory requests. */
STATIC void
warn_disallowed_anonymous_compression_method(compress_method_t method)
{
  log_fn(LOG_PROTOCOL_WARN, LD_HTTP,
         "Received a %s HTTP response, which is not "
         "allowed for anonymous directory requests.",
         compression_method_get_human_name(method));
}

/* We just got a new consensus! If there are other in-progress requests
 * for this consensus flavor (for example because we launched several in
 * parallel), cancel them.
 *
 * We do this check here (not just in
 * connection_ap_handshake_attach_circuit()) to handle the edge case where
 * a consensus fetch begins and ends before some other one tries to attach to
 * a circuit, in which case the other one won't know that we're all happy now.
 *
 * Don't mark the conn that just gave us the consensus -- otherwise we
 * would end up double-marking it when it cleans itself up.
 */
static void
connection_dir_close_consensus_fetches(dir_connection_t *except_this_one,
                                       const char *resource)
{
  smartlist_t *conns_to_close =
    connection_dir_list_by_purpose_and_resource(DIR_PURPOSE_FETCH_CONSENSUS,
                                                resource);
  SMARTLIST_FOREACH_BEGIN(conns_to_close, dir_connection_t *, d) {
    if (d == except_this_one)
      continue;
    log_info(LD_DIR, "Closing consensus fetch (to %s) since one "
             "has just arrived.", connection_describe_peer(TO_CONN(d)));
    connection_mark_for_close(TO_CONN(d));
  } SMARTLIST_FOREACH_END(d);
  smartlist_free(conns_to_close);
}
/** Called when one or more routerdesc (or extrainfo, if <b>was_extrainfo</b>)
 * fetches have failed (with uppercase fingerprints listed in <b>failed</b>,
 * either as descriptor digests or as identity digests based on
 * <b>was_descriptor_digests</b>).
 */
static void
dir_routerdesc_download_failed(smartlist_t *failed, int status_code,
                               int router_purpose,
                               int was_extrainfo, int was_descriptor_digests)
{
  char digest[DIGEST_LEN];
  time_t now = time(NULL);
  int server = dirclient_fetches_from_authorities(get_options());
  if (!was_descriptor_digests) {
    if (router_purpose == ROUTER_PURPOSE_BRIDGE) {
      tor_assert(!was_extrainfo);
      connection_dir_retry_bridges(failed);
    }
    return; /* FFFF should implement for other-than-router-purpose someday */
  }
  SMARTLIST_FOREACH_BEGIN(failed, const char *, cp) {
    download_status_t *dls = NULL;
    if (base16_decode(digest, DIGEST_LEN, cp, strlen(cp)) != DIGEST_LEN) {
      log_warn(LD_BUG, "Malformed fingerprint in list: %s", escaped(cp));
      continue;
    }
    if (was_extrainfo) {
      signed_descriptor_t *sd =
        router_get_by_extrainfo_digest(digest);
      if (sd)
        dls = &sd->ei_dl_status;
    } else {
      dls = router_get_dl_status_by_descriptor_digest(digest);
    }
    if (!dls)
      continue;
    download_status_increment_failure(dls, status_code, cp, server, now);
  } SMARTLIST_FOREACH_END(cp);

  /* No need to relaunch descriptor downloads here: we already do it
   * every 10 or 60 seconds (FOO_DESCRIPTOR_RETRY_INTERVAL) in main.c. */
}

/** Called when a connection to download microdescriptors from relay with
 * <b>dir_id</b> has failed in whole or in part. <b>failed</b> is a list
 * of every microdesc digest we didn't get. <b>status_code</b> is the http
 * status code we received. Reschedule the microdesc downloads as
 * appropriate. */
static void
dir_microdesc_download_failed(smartlist_t *failed,
                              int status_code, const char *dir_id)
{
  networkstatus_t *consensus
    = networkstatus_get_latest_consensus_by_flavor(FLAV_MICRODESC);
  routerstatus_t *rs;
  download_status_t *dls;
  time_t now = time(NULL);
  int server = dirclient_fetches_from_authorities(get_options());

  if (! consensus)
    return;

  /* We failed to fetch a microdescriptor from 'dir_id', note it down
   * so that we don't try the same relay next time... */
  microdesc_note_outdated_dirserver(dir_id);

  SMARTLIST_FOREACH_BEGIN(failed, const char *, d) {
    rs = router_get_mutable_consensus_status_by_descriptor_digest(consensus,d);
    if (!rs)
      continue;
    dls = &rs->dl_status;

    { /* Increment the failure count for this md fetch */
      char buf[BASE64_DIGEST256_LEN+1];
      digest256_to_base64(buf, d);
      log_info(LD_DIR, "Failed to download md %s from %s",
               buf, hex_str(dir_id, DIGEST_LEN));
      download_status_increment_failure(dls, status_code, buf,
                                        server, now);
    }
  } SMARTLIST_FOREACH_END(d);
}
