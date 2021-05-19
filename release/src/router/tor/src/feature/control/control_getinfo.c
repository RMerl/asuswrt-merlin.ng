/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_getinfo.c
 * \brief Implementation for miscellaneous controller getinfo commands.
 */

#define CONTROL_EVENTS_PRIVATE
#define CONTROL_MODULE_PRIVATE
#define CONTROL_GETINFO_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/circuitlist.h"
#include "core/or/connection_edge.h"
#include "core/or/connection_or.h"
#include "core/or/policies.h"
#include "core/or/versions.h"
#include "feature/client/addressmap.h"
#include "feature/client/bridges.h"
#include "feature/client/entrynodes.h"
#include "feature/control/control.h"
#include "feature/control/control_cmd.h"
#include "feature/control/control_events.h"
#include "feature/control/control_fmt.h"
#include "feature/control/control_getinfo.h"
#include "feature/control/control_proto.h"
#include "feature/control/getinfo_geoip.h"
#include "feature/dircache/dirserv.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirclient/dlstatus.h"
#include "feature/dircommon/directory.h"
#include "feature/hibernate/hibernate.h"
#include "feature/hs/hs_cache.h"
#include "feature/hs_common/shared_random_client.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/relay_find_addr.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "feature/relay/selftest.h"
#include "feature/rend/rendcache.h"
#include "feature/stats/geoip_stats.h"
#include "feature/stats/predict_ports.h"
#include "feature/stats/rephist.h"
#include "lib/version/torversion.h"
#include "lib/encoding/kvline.h"

#include "core/or/entry_connection_st.h"
#include "core/or/or_connection_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/socks_request_st.h"
#include "feature/control/control_connection_st.h"
#include "feature/control/control_cmd_args_st.h"
#include "feature/dircache/cached_dir_st.h"
#include "feature/nodelist/extrainfo_st.h"
#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist_st.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef _WIN32
#include <pwd.h>
#endif

static char *list_getinfo_options(void);
static char *download_status_to_string(const download_status_t *dl);

/** Implementation helper for GETINFO: knows the answers for various
 * trivial-to-implement questions. */
static int
getinfo_helper_misc(control_connection_t *conn, const char *question,
                    char **answer, const char **errmsg)
{
  (void) conn;
  if (!strcmp(question, "version")) {
    *answer = tor_strdup(get_version());
  } else if (!strcmp(question, "bw-event-cache")) {
    *answer = get_bw_samples();
  } else if (!strcmp(question, "config-file")) {
    const char *a = get_torrc_fname(0);
    if (a)
      *answer = tor_strdup(a);
  } else if (!strcmp(question, "config-defaults-file")) {
    const char *a = get_torrc_fname(1);
    if (a)
      *answer = tor_strdup(a);
  } else if (!strcmp(question, "config-text")) {
    *answer = options_dump(get_options(), OPTIONS_DUMP_MINIMAL);
  } else if (!strcmp(question, "config-can-saveconf")) {
    *answer = tor_strdup(get_options()->IncludeUsed ? "0" : "1");
  } else if (!strcmp(question, "info/names")) {
    *answer = list_getinfo_options();
  } else if (!strcmp(question, "dormant")) {
    int dormant = rep_hist_circbuilding_dormant(time(NULL));
    *answer = tor_strdup(dormant ? "1" : "0");
  } else if (!strcmp(question, "events/names")) {
    int i;
    smartlist_t *event_names = smartlist_new();

    for (i = 0; control_event_table[i].event_name != NULL; ++i) {
      smartlist_add(event_names, (char *)control_event_table[i].event_name);
    }

    *answer = smartlist_join_strings(event_names, " ", 0, NULL);

    smartlist_free(event_names);
  } else if (!strcmp(question, "signal/names")) {
    smartlist_t *signal_names = smartlist_new();
    int j;
    for (j = 0; signal_table[j].signal_name != NULL; ++j) {
      smartlist_add(signal_names, (char*)signal_table[j].signal_name);
    }

    *answer = smartlist_join_strings(signal_names, " ", 0, NULL);

    smartlist_free(signal_names);
  } else if (!strcmp(question, "features/names")) {
    *answer = tor_strdup("VERBOSE_NAMES EXTENDED_EVENTS");
  } else if (!strcmp(question, "address") || !strcmp(question, "address/v4")) {
    tor_addr_t addr;
    if (!relay_find_addr_to_publish(get_options(), AF_INET,
                                    RELAY_FIND_ADDR_CACHE_ONLY, &addr)) {
      *errmsg = "Address unknown";
      return -1;
    }
    *answer = tor_addr_to_str_dup(&addr);
    tor_assert_nonfatal(*answer);
  } else if (!strcmp(question, "address/v6")) {
    tor_addr_t addr;
    if (!relay_find_addr_to_publish(get_options(), AF_INET6,
                                    RELAY_FIND_ADDR_CACHE_ONLY, &addr)) {
      *errmsg = "Address unknown";
      return -1;
    }
    *answer = tor_addr_to_str_dup(&addr);
    tor_assert_nonfatal(*answer);
  } else if (!strcmp(question, "traffic/read")) {
    tor_asprintf(answer, "%"PRIu64, (get_bytes_read()));
  } else if (!strcmp(question, "traffic/written")) {
    tor_asprintf(answer, "%"PRIu64, (get_bytes_written()));
  } else if (!strcmp(question, "uptime")) {
    long uptime_secs = get_uptime();
    tor_asprintf(answer, "%ld", uptime_secs);
  } else if (!strcmp(question, "process/pid")) {
    int myPid = -1;

#ifdef _WIN32
      myPid = _getpid();
#else
      myPid = getpid();
#endif

    tor_asprintf(answer, "%d", myPid);
  } else if (!strcmp(question, "process/uid")) {
#ifdef _WIN32
      *answer = tor_strdup("-1");
#else
      int myUid = geteuid();
      tor_asprintf(answer, "%d", myUid);
#endif /* defined(_WIN32) */
  } else if (!strcmp(question, "process/user")) {
#ifdef _WIN32
      *answer = tor_strdup("");
#else
      int myUid = geteuid();
      const struct passwd *myPwEntry = tor_getpwuid(myUid);

      if (myPwEntry) {
        *answer = tor_strdup(myPwEntry->pw_name);
      } else {
        *answer = tor_strdup("");
      }
#endif /* defined(_WIN32) */
  } else if (!strcmp(question, "process/descriptor-limit")) {
    int max_fds = get_max_sockets();
    tor_asprintf(answer, "%d", max_fds);
  } else if (!strcmp(question, "limits/max-mem-in-queues")) {
    tor_asprintf(answer, "%"PRIu64,
                 (get_options()->MaxMemInQueues));
  } else if (!strcmp(question, "fingerprint")) {
    crypto_pk_t *server_key;
    if (!server_mode(get_options())) {
      *errmsg = "Not running in server mode";
      return -1;
    }
    server_key = get_server_identity_key();
    *answer = tor_malloc(HEX_DIGEST_LEN+1);
    crypto_pk_get_fingerprint(server_key, *answer, 0);
  }
  return 0;
}

/** Awful hack: return a newly allocated string based on a routerinfo and
 * (possibly) an extrainfo, sticking the read-history and write-history from
 * <b>ei</b> into the resulting string.  The thing you get back won't
 * necessarily have a valid signature.
 *
 * New code should never use this; it's for backward compatibility.
 *
 * NOTE: <b>ri_body</b> is as returned by signed_descriptor_get_body: it might
 * not be NUL-terminated. */
static char *
munge_extrainfo_into_routerinfo(const char *ri_body,
                                const signed_descriptor_t *ri,
                                const signed_descriptor_t *ei)
{
  char *out = NULL, *outp;
  int i;
  const char *router_sig;
  const char *ei_body = signed_descriptor_get_body(ei);
  size_t ri_len = ri->signed_descriptor_len;
  size_t ei_len = ei->signed_descriptor_len;
  if (!ei_body)
    goto bail;

  outp = out = tor_malloc(ri_len+ei_len+1);
  if (!(router_sig = tor_memstr(ri_body, ri_len, "\nrouter-signature")))
    goto bail;
  ++router_sig;
  memcpy(out, ri_body, router_sig-ri_body);
  outp += router_sig-ri_body;

  for (i=0; i < 2; ++i) {
    const char *kwd = i ? "\nwrite-history " : "\nread-history ";
    const char *cp, *eol;
    if (!(cp = tor_memstr(ei_body, ei_len, kwd)))
      continue;
    ++cp;
    if (!(eol = memchr(cp, '\n', ei_len - (cp-ei_body))))
      continue;
    memcpy(outp, cp, eol-cp+1);
    outp += eol-cp+1;
  }
  memcpy(outp, router_sig, ri_len - (router_sig-ri_body));
  *outp++ = '\0';
  tor_assert(outp-out < (int)(ri_len+ei_len+1));

  return out;
 bail:
  tor_free(out);
  return tor_strndup(ri_body, ri->signed_descriptor_len);
}

/** Implementation helper for GETINFO: answers requests for information about
 * which ports are bound. */
static int
getinfo_helper_listeners(control_connection_t *control_conn,
                         const char *question,
                         char **answer, const char **errmsg)
{
  int type;
  smartlist_t *res;

  (void)control_conn;
  (void)errmsg;

  if (!strcmp(question, "net/listeners/or"))
    type = CONN_TYPE_OR_LISTENER;
  else if (!strcmp(question, "net/listeners/extor"))
    type = CONN_TYPE_EXT_OR_LISTENER;
  else if (!strcmp(question, "net/listeners/dir"))
    type = CONN_TYPE_DIR_LISTENER;
  else if (!strcmp(question, "net/listeners/socks"))
    type = CONN_TYPE_AP_LISTENER;
  else if (!strcmp(question, "net/listeners/trans"))
    type = CONN_TYPE_AP_TRANS_LISTENER;
  else if (!strcmp(question, "net/listeners/natd"))
    type = CONN_TYPE_AP_NATD_LISTENER;
  else if (!strcmp(question, "net/listeners/httptunnel"))
    type = CONN_TYPE_AP_HTTP_CONNECT_LISTENER;
  else if (!strcmp(question, "net/listeners/dns"))
    type = CONN_TYPE_AP_DNS_LISTENER;
  else if (!strcmp(question, "net/listeners/control"))
    type = CONN_TYPE_CONTROL_LISTENER;
  else if (!strcmp(question, "net/listeners/metrics"))
    type = CONN_TYPE_METRICS_LISTENER;
  else
    return 0; /* unknown key */

  res = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(get_connection_array(), connection_t *, conn) {
    struct sockaddr_storage ss;
    socklen_t ss_len = sizeof(ss);

    if (conn->type != type || conn->marked_for_close || !SOCKET_OK(conn->s))
      continue;

    if (getsockname(conn->s, (struct sockaddr *)&ss, &ss_len) < 0) {
      smartlist_add_asprintf(res, "%s:%d", conn->address, (int)conn->port);
    } else {
      char *tmp = tor_sockaddr_to_str((struct sockaddr *)&ss);
      smartlist_add(res, esc_for_log(tmp));
      tor_free(tmp);
    }

  } SMARTLIST_FOREACH_END(conn);

  *answer = smartlist_join_strings(res, " ", 0, NULL);

  SMARTLIST_FOREACH(res, char *, cp, tor_free(cp));
  smartlist_free(res);
  return 0;
}

/** Implementation helper for GETINFO: answers requests for information about
 * the current time in both local and UTC forms. */
STATIC int
getinfo_helper_current_time(control_connection_t *control_conn,
                         const char *question,
                         char **answer, const char **errmsg)
{
  (void)control_conn;
  (void)errmsg;

  struct timeval now;
  tor_gettimeofday(&now);
  char timebuf[ISO_TIME_LEN+1];

  if (!strcmp(question, "current-time/local"))
    format_local_iso_time_nospace(timebuf, (time_t)now.tv_sec);
  else if (!strcmp(question, "current-time/utc"))
    format_iso_time_nospace(timebuf, (time_t)now.tv_sec);
  else
    return 0;

  *answer = tor_strdup(timebuf);
  return 0;
}

/** GETINFO helper for dumping different consensus flavors
 * returns: 0 on success -1 on error. */
STATIC int
getinfo_helper_current_consensus(consensus_flavor_t flavor,
                                 char** answer,
                                 const char** errmsg)
{
  const char *flavor_name = networkstatus_get_flavor_name(flavor);
  if (BUG(!strcmp(flavor_name, "??"))) {
    *errmsg = "Internal error: unrecognized flavor name.";
    return -1;
  }
  if (we_want_to_fetch_flavor(get_options(), flavor)) {
    /** Check from the cache */
    const cached_dir_t *consensus = dirserv_get_consensus(flavor_name);
    if (consensus) {
      *answer = tor_strdup(consensus->dir);
    }
  }
  if (!*answer) { /* try loading it from disk */

    tor_mmap_t *mapped = networkstatus_map_cached_consensus(flavor_name);
    if (mapped) {
      *answer = tor_memdup_nulterm(mapped->data, mapped->size);
      tor_munmap_file(mapped);
    }
    if (!*answer) { /* generate an error */
      *errmsg = "Could not open cached consensus. "
        "Make sure FetchUselessDescriptors is set to 1.";
      return -1;
    }
  }
  return 0;
}

/** Helper for getinfo_helper_dir.
 *
 * Add a signed_descriptor_t to <b>descs_out</b> for each router matching
 * <b>key</b>.  The key should be either
 *   - "/tor/server/authority" for our own routerinfo;
 *   - "/tor/server/all" for all the routerinfos we have, concatenated;
 *   - "/tor/server/fp/FP" where FP is a plus-separated sequence of
 *     hex identity digests; or
 *   - "/tor/server/d/D" where D is a plus-separated sequence
 *     of server descriptor digests, in hex.
 *
 * Return 0 if we found some matching descriptors, or -1 if we do not
 * have any descriptors, no matching descriptors, or if we did not
 * recognize the key (URL).
 * If -1 is returned *<b>msg</b> will be set to an appropriate error
 * message.
 */
static int
controller_get_routerdescs(smartlist_t *descs_out, const char *key,
                        const char **msg)
{
  *msg = NULL;

  if (!strcmp(key, "/tor/server/all")) {
    routerlist_t *rl = router_get_routerlist();
    SMARTLIST_FOREACH(rl->routers, routerinfo_t *, r,
                      smartlist_add(descs_out, &(r->cache_info)));
  } else if (!strcmp(key, "/tor/server/authority")) {
    const routerinfo_t *ri = router_get_my_routerinfo();
    if (ri)
      smartlist_add(descs_out, (void*) &(ri->cache_info));
  } else if (!strcmpstart(key, "/tor/server/d/")) {
    smartlist_t *digests = smartlist_new();
    key += strlen("/tor/server/d/");
    dir_split_resource_into_fingerprints(key, digests, NULL,
                                         DSR_HEX|DSR_SORT_UNIQ);
    SMARTLIST_FOREACH(digests, const char *, d,
       {
         signed_descriptor_t *sd = router_get_by_descriptor_digest(d);
         if (sd)
           smartlist_add(descs_out,sd);
       });
    SMARTLIST_FOREACH(digests, char *, d, tor_free(d));
    smartlist_free(digests);
  } else if (!strcmpstart(key, "/tor/server/fp/")) {
    smartlist_t *digests = smartlist_new();
    time_t cutoff = time(NULL) - ROUTER_MAX_AGE_TO_PUBLISH;
    key += strlen("/tor/server/fp/");
    dir_split_resource_into_fingerprints(key, digests, NULL,
                                         DSR_HEX|DSR_SORT_UNIQ);
    SMARTLIST_FOREACH_BEGIN(digests, const char *, d) {
         if (router_digest_is_me(d)) {
           /* calling router_get_my_routerinfo() to make sure it exists */
           const routerinfo_t *ri = router_get_my_routerinfo();
           if (ri)
             smartlist_add(descs_out, (void*) &(ri->cache_info));
         } else {
           const routerinfo_t *ri = router_get_by_id_digest(d);
           /* Don't actually serve a descriptor that everyone will think is
            * expired.  This is an (ugly) workaround to keep buggy 0.1.1.10
            * Tors from downloading descriptors that they will throw away.
            */
           if (ri && ri->cache_info.published_on > cutoff)
             smartlist_add(descs_out, (void*) &(ri->cache_info));
         }
    } SMARTLIST_FOREACH_END(d);
    SMARTLIST_FOREACH(digests, char *, d, tor_free(d));
    smartlist_free(digests);
  } else {
    *msg = "Key not recognized";
    return -1;
  }

  if (!smartlist_len(descs_out)) {
    *msg = "Servers unavailable";
    return -1;
  }
  return 0;
}

/** Implementation helper for GETINFO: knows the answers for questions about
 * directory information. */
STATIC int
getinfo_helper_dir(control_connection_t *control_conn,
                   const char *question, char **answer,
                   const char **errmsg)
{
  (void) control_conn;
  if (!strcmpstart(question, "desc/id/")) {
    const routerinfo_t *ri = NULL;
    const node_t *node = node_get_by_hex_id(question+strlen("desc/id/"), 0);
    if (node)
      ri = node->ri;
    if (ri) {
      const char *body = signed_descriptor_get_body(&ri->cache_info);
      if (body)
        *answer = tor_strndup(body, ri->cache_info.signed_descriptor_len);
    } else if (! we_fetch_router_descriptors(get_options())) {
      /* Descriptors won't be available, provide proper error */
      *errmsg = "We fetch microdescriptors, not router "
                "descriptors. You'll need to use md/id/* "
                "instead of desc/id/*.";
      return 0;
    }
  } else if (!strcmpstart(question, "desc/name/")) {
    const routerinfo_t *ri = NULL;
    /* XXX Setting 'warn_if_unnamed' here is a bit silly -- the
     * warning goes to the user, not to the controller. */
    const node_t *node =
      node_get_by_nickname(question+strlen("desc/name/"), 0);
    if (node)
      ri = node->ri;
    if (ri) {
      const char *body = signed_descriptor_get_body(&ri->cache_info);
      if (body)
        *answer = tor_strndup(body, ri->cache_info.signed_descriptor_len);
    } else if (! we_fetch_router_descriptors(get_options())) {
      /* Descriptors won't be available, provide proper error */
      *errmsg = "We fetch microdescriptors, not router "
                "descriptors. You'll need to use md/name/* "
                "instead of desc/name/*.";
      return 0;
    }
  } else if (!strcmp(question, "desc/download-enabled")) {
    int r = we_fetch_router_descriptors(get_options());
    tor_asprintf(answer, "%d", !!r);
  } else if (!strcmp(question, "desc/all-recent")) {
    routerlist_t *routerlist = router_get_routerlist();
    smartlist_t *sl = smartlist_new();
    if (routerlist && routerlist->routers) {
      SMARTLIST_FOREACH(routerlist->routers, const routerinfo_t *, ri,
      {
        const char *body = signed_descriptor_get_body(&ri->cache_info);
        if (body)
          smartlist_add(sl,
                  tor_strndup(body, ri->cache_info.signed_descriptor_len));
      });
    }
    *answer = smartlist_join_strings(sl, "", 0, NULL);
    SMARTLIST_FOREACH(sl, char *, c, tor_free(c));
    smartlist_free(sl);
  } else if (!strcmp(question, "desc/all-recent-extrainfo-hack")) {
    /* XXXX Remove this once Torstat asks for extrainfos. */
    routerlist_t *routerlist = router_get_routerlist();
    smartlist_t *sl = smartlist_new();
    if (routerlist && routerlist->routers) {
      SMARTLIST_FOREACH_BEGIN(routerlist->routers, const routerinfo_t *, ri) {
        const char *body = signed_descriptor_get_body(&ri->cache_info);
        signed_descriptor_t *ei = extrainfo_get_by_descriptor_digest(
                                     ri->cache_info.extra_info_digest);
        if (ei && body) {
          smartlist_add(sl, munge_extrainfo_into_routerinfo(body,
                                                        &ri->cache_info, ei));
        } else if (body) {
          smartlist_add(sl,
                  tor_strndup(body, ri->cache_info.signed_descriptor_len));
        }
      } SMARTLIST_FOREACH_END(ri);
    }
    *answer = smartlist_join_strings(sl, "", 0, NULL);
    SMARTLIST_FOREACH(sl, char *, c, tor_free(c));
    smartlist_free(sl);
  } else if (!strcmpstart(question, "hs/client/desc/id/")) {
    hostname_type_t addr_type;

    question += strlen("hs/client/desc/id/");
    if (rend_valid_v2_service_id(question)) {
      addr_type = ONION_V2_HOSTNAME;
    } else if (hs_address_is_valid(question)) {
      addr_type = ONION_V3_HOSTNAME;
    } else {
      *errmsg = "Invalid address";
      return -1;
    }

    if (addr_type == ONION_V2_HOSTNAME) {
      rend_cache_entry_t *e = NULL;
      if (!rend_cache_lookup_entry(question, -1, &e)) {
        /* Descriptor found in cache */
        *answer = tor_strdup(e->desc);
      } else {
        *errmsg = "Not found in cache";
        return -1;
      }
    } else {
      ed25519_public_key_t service_pk;
      const char *desc;

      /* The check before this if/else makes sure of this. */
      tor_assert(addr_type == ONION_V3_HOSTNAME);

      if (hs_parse_address(question, &service_pk, NULL, NULL) < 0) {
        *errmsg = "Invalid v3 address";
        return -1;
      }

      desc = hs_cache_lookup_encoded_as_client(&service_pk);
      if (desc) {
        *answer = tor_strdup(desc);
      } else {
        *errmsg = "Not found in cache";
        return -1;
      }
    }
  } else if (!strcmpstart(question, "hs/service/desc/id/")) {
    hostname_type_t addr_type;

    question += strlen("hs/service/desc/id/");
    if (rend_valid_v2_service_id(question)) {
      addr_type = ONION_V2_HOSTNAME;
    } else if (hs_address_is_valid(question)) {
      addr_type = ONION_V3_HOSTNAME;
    } else {
      *errmsg = "Invalid address";
      return -1;
    }
    rend_cache_entry_t *e = NULL;

    if (addr_type == ONION_V2_HOSTNAME) {
      if (!rend_cache_lookup_v2_desc_as_service(question, &e)) {
        /* Descriptor found in cache */
        *answer = tor_strdup(e->desc);
      } else {
        *errmsg = "Not found in cache";
        return -1;
      }
    } else {
      ed25519_public_key_t service_pk;
      char *desc;

      /* The check before this if/else makes sure of this. */
      tor_assert(addr_type == ONION_V3_HOSTNAME);

      if (hs_parse_address(question, &service_pk, NULL, NULL) < 0) {
        *errmsg = "Invalid v3 address";
        return -1;
      }

      desc = hs_service_lookup_current_desc(&service_pk);
      if (desc) {
        /* Newly allocated string, we have ownership. */
        *answer = desc;
      } else {
        *errmsg = "Not found in cache";
        return -1;
      }
    }
  } else if (!strcmp(question, "md/all")) {
    const smartlist_t *nodes = nodelist_get_list();
    tor_assert(nodes);

    if (smartlist_len(nodes) == 0) {
      *answer = tor_strdup("");
      return 0;
    }

    smartlist_t *microdescs = smartlist_new();

    SMARTLIST_FOREACH_BEGIN(nodes, node_t *, n) {
      if (n->md && n->md->body) {
        char *copy = tor_strndup(n->md->body, n->md->bodylen);
        smartlist_add(microdescs, copy);
      }
    } SMARTLIST_FOREACH_END(n);

    *answer = smartlist_join_strings(microdescs, "", 0, NULL);
    SMARTLIST_FOREACH(microdescs, char *, md, tor_free(md));
    smartlist_free(microdescs);
  } else if (!strcmpstart(question, "md/id/")) {
    const node_t *node = node_get_by_hex_id(question+strlen("md/id/"), 0);
    const microdesc_t *md = NULL;
    if (node) md = node->md;
    if (md && md->body) {
      *answer = tor_strndup(md->body, md->bodylen);
    }
  } else if (!strcmpstart(question, "md/name/")) {
    /* XXX Setting 'warn_if_unnamed' here is a bit silly -- the
     * warning goes to the user, not to the controller. */
    const node_t *node = node_get_by_nickname(question+strlen("md/name/"), 0);
    /* XXXX duplicated code */
    const microdesc_t *md = NULL;
    if (node) md = node->md;
    if (md && md->body) {
      *answer = tor_strndup(md->body, md->bodylen);
    }
  } else if (!strcmp(question, "md/download-enabled")) {
    int r = we_fetch_microdescriptors(get_options());
    tor_asprintf(answer, "%d", !!r);
  } else if (!strcmpstart(question, "desc-annotations/id/")) {
    const routerinfo_t *ri = NULL;
    const node_t *node =
      node_get_by_hex_id(question+strlen("desc-annotations/id/"), 0);
    if (node)
      ri = node->ri;
    if (ri) {
      const char *annotations =
        signed_descriptor_get_annotations(&ri->cache_info);
      if (annotations)
        *answer = tor_strndup(annotations,
                              ri->cache_info.annotations_len);
    }
  } else if (!strcmpstart(question, "dir/server/")) {
    size_t answer_len = 0;
    char *url = NULL;
    smartlist_t *descs = smartlist_new();
    const char *msg;
    int res;
    char *cp;
    tor_asprintf(&url, "/tor/%s", question+4);
    res = controller_get_routerdescs(descs, url, &msg);
    if (res) {
      log_warn(LD_CONTROL, "getinfo '%s': %s", question, msg);
      smartlist_free(descs);
      tor_free(url);
      *errmsg = msg;
      return -1;
    }
    SMARTLIST_FOREACH(descs, signed_descriptor_t *, sd,
                      answer_len += sd->signed_descriptor_len);
    cp = *answer = tor_malloc(answer_len+1);
    SMARTLIST_FOREACH(descs, signed_descriptor_t *, sd,
                      {
                        memcpy(cp, signed_descriptor_get_body(sd),
                               sd->signed_descriptor_len);
                        cp += sd->signed_descriptor_len;
                      });
    *cp = '\0';
    tor_free(url);
    smartlist_free(descs);
  } else if (!strcmpstart(question, "dir/status/")) {
    *answer = tor_strdup("");
  } else if (!strcmp(question, "dir/status-vote/current/consensus")) {
    int consensus_result = getinfo_helper_current_consensus(FLAV_NS,
                                                            answer, errmsg);
    if (consensus_result < 0) {
      return -1;
    }
  } else if (!strcmp(question,
                     "dir/status-vote/current/consensus-microdesc")) {
    int consensus_result = getinfo_helper_current_consensus(FLAV_MICRODESC,
                                                            answer, errmsg);
    if (consensus_result < 0) {
      return -1;
    }
  } else if (!strcmpstart(question, "extra-info/digest/")) {
    question += strlen("extra-info/digest/");
    if (strlen(question) == HEX_DIGEST_LEN) {
      char d[DIGEST_LEN];
      signed_descriptor_t *sd = NULL;
      if (base16_decode(d, sizeof(d), question, strlen(question))
                        == sizeof(d)) {
        /* XXXX this test should move into extrainfo_get_by_descriptor_digest,
         * but I don't want to risk affecting other parts of the code,
         * especially since the rules for using our own extrainfo (including
         * when it might be freed) are different from those for using one
         * we have downloaded. */
        if (router_extrainfo_digest_is_me(d))
          sd = &(router_get_my_extrainfo()->cache_info);
        else
          sd = extrainfo_get_by_descriptor_digest(d);
      }
      if (sd) {
        const char *body = signed_descriptor_get_body(sd);
        if (body)
          *answer = tor_strndup(body, sd->signed_descriptor_len);
      }
    }
  }

  return 0;
}

/** Given a smartlist of 20-byte digests, return a newly allocated string
 * containing each of those digests in order, formatted in HEX, and terminated
 * with a newline. */
static char *
digest_list_to_string(const smartlist_t *sl)
{
  int len;
  char *result, *s;

  /* Allow for newlines, and a \0 at the end */
  len = smartlist_len(sl) * (HEX_DIGEST_LEN + 1) + 1;
  result = tor_malloc_zero(len);

  s = result;
  SMARTLIST_FOREACH_BEGIN(sl, const char *, digest) {
    base16_encode(s, HEX_DIGEST_LEN + 1, digest, DIGEST_LEN);
    s[HEX_DIGEST_LEN] = '\n';
    s += HEX_DIGEST_LEN + 1;
  } SMARTLIST_FOREACH_END(digest);
  *s = '\0';

  return result;
}

/** Turn a download_status_t into a human-readable description in a newly
 * allocated string.  The format is specified in control-spec.txt, under
 * the documentation for "GETINFO download/..." .  */
static char *
download_status_to_string(const download_status_t *dl)
{
  char *rv = NULL;
  char tbuf[ISO_TIME_LEN+1];
  const char *schedule_str, *want_authority_str;
  const char *increment_on_str, *backoff_str;

  if (dl) {
    /* Get some substrings of the eventual output ready */
    format_iso_time(tbuf, download_status_get_next_attempt_at(dl));

    switch (dl->schedule) {
      case DL_SCHED_GENERIC:
        schedule_str = "DL_SCHED_GENERIC";
        break;
      case DL_SCHED_CONSENSUS:
        schedule_str = "DL_SCHED_CONSENSUS";
        break;
      case DL_SCHED_BRIDGE:
        schedule_str = "DL_SCHED_BRIDGE";
        break;
      default:
        schedule_str = "unknown";
        break;
    }

    switch (dl->want_authority) {
      case DL_WANT_ANY_DIRSERVER:
        want_authority_str = "DL_WANT_ANY_DIRSERVER";
        break;
      case DL_WANT_AUTHORITY:
        want_authority_str = "DL_WANT_AUTHORITY";
        break;
      default:
        want_authority_str = "unknown";
        break;
    }

    switch (dl->increment_on) {
      case DL_SCHED_INCREMENT_FAILURE:
        increment_on_str = "DL_SCHED_INCREMENT_FAILURE";
        break;
      case DL_SCHED_INCREMENT_ATTEMPT:
        increment_on_str = "DL_SCHED_INCREMENT_ATTEMPT";
        break;
      default:
        increment_on_str = "unknown";
        break;
    }

    backoff_str = "DL_SCHED_RANDOM_EXPONENTIAL";

    /* Now assemble them */
    tor_asprintf(&rv,
                 "next-attempt-at %s\n"
                 "n-download-failures %u\n"
                 "n-download-attempts %u\n"
                 "schedule %s\n"
                 "want-authority %s\n"
                 "increment-on %s\n"
                 "backoff %s\n"
                 "last-backoff-position %u\n"
                 "last-delay-used %d\n",
                 tbuf,
                 dl->n_download_failures,
                 dl->n_download_attempts,
                 schedule_str,
                 want_authority_str,
                 increment_on_str,
                 backoff_str,
                 dl->last_backoff_position,
                 dl->last_delay_used);
  }

  return rv;
}

/** Handle the consensus download cases for getinfo_helper_downloads() */
STATIC void
getinfo_helper_downloads_networkstatus(const char *flavor,
                                       download_status_t **dl_to_emit,
                                       const char **errmsg)
{
  /*
   * We get the one for the current bootstrapped status by default, or
   * take an extra /bootstrap or /running suffix
   */
  if (strcmp(flavor, "ns") == 0) {
    *dl_to_emit = networkstatus_get_dl_status_by_flavor(FLAV_NS);
  } else if (strcmp(flavor, "ns/bootstrap") == 0) {
    *dl_to_emit = networkstatus_get_dl_status_by_flavor_bootstrap(FLAV_NS);
  } else if (strcmp(flavor, "ns/running") == 0 ) {
    *dl_to_emit = networkstatus_get_dl_status_by_flavor_running(FLAV_NS);
  } else if (strcmp(flavor, "microdesc") == 0) {
    *dl_to_emit = networkstatus_get_dl_status_by_flavor(FLAV_MICRODESC);
  } else if (strcmp(flavor, "microdesc/bootstrap") == 0) {
    *dl_to_emit =
      networkstatus_get_dl_status_by_flavor_bootstrap(FLAV_MICRODESC);
  } else if (strcmp(flavor, "microdesc/running") == 0) {
    *dl_to_emit =
      networkstatus_get_dl_status_by_flavor_running(FLAV_MICRODESC);
  } else {
    *errmsg = "Unknown flavor";
  }
}

/** Handle the cert download cases for getinfo_helper_downloads() */
STATIC void
getinfo_helper_downloads_cert(const char *fp_sk_req,
                              download_status_t **dl_to_emit,
                              smartlist_t **digest_list,
                              const char **errmsg)
{
  const char *sk_req;
  char id_digest[DIGEST_LEN];
  char sk_digest[DIGEST_LEN];

  /*
   * We have to handle four cases; fp_sk_req is the request with
   * a prefix of "downloads/cert/" snipped off.
   *
   * Case 1: fp_sk_req = "fps"
   *  - We should emit a digest_list with a list of all the identity
   *    fingerprints that can be queried for certificate download status;
   *    get it by calling list_authority_ids_with_downloads().
   *
   * Case 2: fp_sk_req = "fp/<fp>" for some fingerprint fp
   *  - We want the default certificate for this identity fingerprint's
   *    download status; this is the download we get from URLs starting
   *    in /fp/ on the directory server.  We can get it with
   *    id_only_download_status_for_authority_id().
   *
   * Case 3: fp_sk_req = "fp/<fp>/sks" for some fingerprint fp
   *  - We want a list of all signing key digests for this identity
   *    fingerprint which can be queried for certificate download status.
   *    Get it with list_sk_digests_for_authority_id().
   *
   * Case 4: fp_sk_req = "fp/<fp>/<sk>" for some fingerprint fp and
   *         signing key digest sk
   *   - We want the download status for the certificate for this specific
   *     signing key and fingerprint.  These correspond to the ones we get
   *     from URLs starting in /fp-sk/ on the directory server.  Get it with
   *     list_sk_digests_for_authority_id().
   */

  if (strcmp(fp_sk_req, "fps") == 0) {
    *digest_list = list_authority_ids_with_downloads();
    if (!(*digest_list)) {
      *errmsg = "Failed to get list of authority identity digests (!)";
    }
  } else if (!strcmpstart(fp_sk_req, "fp/")) {
    fp_sk_req += strlen("fp/");
    /* Okay, look for another / to tell the fp from fp-sk cases */
    sk_req = strchr(fp_sk_req, '/');
    if (sk_req) {
      /* okay, split it here and try to parse <fp> */
      if (base16_decode(id_digest, DIGEST_LEN,
                        fp_sk_req, sk_req - fp_sk_req) == DIGEST_LEN) {
        /* Skip past the '/' */
        ++sk_req;
        if (strcmp(sk_req, "sks") == 0) {
          /* We're asking for the list of signing key fingerprints */
          *digest_list = list_sk_digests_for_authority_id(id_digest);
          if (!(*digest_list)) {
            *errmsg = "Failed to get list of signing key digests for this "
                      "authority identity digest";
          }
        } else {
          /* We've got a signing key digest */
          if (base16_decode(sk_digest, DIGEST_LEN,
                            sk_req, strlen(sk_req)) == DIGEST_LEN) {
            *dl_to_emit =
              download_status_for_authority_id_and_sk(id_digest, sk_digest);
            if (!(*dl_to_emit)) {
              *errmsg = "Failed to get download status for this identity/"
                        "signing key digest pair";
            }
          } else {
            *errmsg = "That didn't look like a signing key digest";
          }
        }
      } else {
        *errmsg = "That didn't look like an identity digest";
      }
    } else {
      /* We're either in downloads/certs/fp/<fp>, or we can't parse <fp> */
      if (strlen(fp_sk_req) == HEX_DIGEST_LEN) {
        if (base16_decode(id_digest, DIGEST_LEN,
                          fp_sk_req, strlen(fp_sk_req)) == DIGEST_LEN) {
          *dl_to_emit = id_only_download_status_for_authority_id(id_digest);
          if (!(*dl_to_emit)) {
            *errmsg = "Failed to get download status for this authority "
                      "identity digest";
          }
        } else {
          *errmsg = "That didn't look like a digest";
        }
      } else {
        *errmsg = "That didn't look like a digest";
      }
    }
  } else {
    *errmsg = "Unknown certificate download status query";
  }
}

/** Handle the routerdesc download cases for getinfo_helper_downloads() */
STATIC void
getinfo_helper_downloads_desc(const char *desc_req,
                              download_status_t **dl_to_emit,
                              smartlist_t **digest_list,
                              const char **errmsg)
{
  char desc_digest[DIGEST_LEN];
  /*
   * Two cases to handle here:
   *
   * Case 1: desc_req = "descs"
   *   - Emit a list of all router descriptor digests, which we get by
   *     calling router_get_descriptor_digests(); this can return NULL
   *     if we have no current ns-flavor consensus.
   *
   * Case 2: desc_req = <fp>
   *   - Check on the specified fingerprint and emit its download_status_t
   *     using router_get_dl_status_by_descriptor_digest().
   */

  if (strcmp(desc_req, "descs") == 0) {
    *digest_list = router_get_descriptor_digests();
    if (!(*digest_list)) {
      *errmsg = "We don't seem to have a networkstatus-flavored consensus";
    }
    /*
     * Microdescs don't use the download_status_t mechanism, so we don't
     * answer queries about their downloads here; see microdesc.c.
     */
  } else if (strlen(desc_req) == HEX_DIGEST_LEN) {
    if (base16_decode(desc_digest, DIGEST_LEN,
                      desc_req, strlen(desc_req)) == DIGEST_LEN) {
      /* Okay we got a digest-shaped thing; try asking for it */
      *dl_to_emit = router_get_dl_status_by_descriptor_digest(desc_digest);
      if (!(*dl_to_emit)) {
        *errmsg = "No such descriptor digest found";
      }
    } else {
      *errmsg = "That didn't look like a digest";
    }
  } else {
    *errmsg = "Unknown router descriptor download status query";
  }
}

/** Handle the bridge download cases for getinfo_helper_downloads() */
STATIC void
getinfo_helper_downloads_bridge(const char *bridge_req,
                                download_status_t **dl_to_emit,
                                smartlist_t **digest_list,
                                const char **errmsg)
{
  char bridge_digest[DIGEST_LEN];
  /*
   * Two cases to handle here:
   *
   * Case 1: bridge_req = "bridges"
   *   - Emit a list of all bridge identity digests, which we get by
   *     calling list_bridge_identities(); this can return NULL if we are
   *     not using bridges.
   *
   * Case 2: bridge_req = <fp>
   *   - Check on the specified fingerprint and emit its download_status_t
   *     using get_bridge_dl_status_by_id().
   */

  if (strcmp(bridge_req, "bridges") == 0) {
    *digest_list = list_bridge_identities();
    if (!(*digest_list)) {
      *errmsg = "We don't seem to be using bridges";
    }
  } else if (strlen(bridge_req) == HEX_DIGEST_LEN) {
    if (base16_decode(bridge_digest, DIGEST_LEN,
                      bridge_req, strlen(bridge_req)) == DIGEST_LEN) {
      /* Okay we got a digest-shaped thing; try asking for it */
      *dl_to_emit = get_bridge_dl_status_by_id(bridge_digest);
      if (!(*dl_to_emit)) {
        *errmsg = "No such bridge identity digest found";
      }
    } else {
      *errmsg = "That didn't look like a digest";
    }
  } else {
    *errmsg = "Unknown bridge descriptor download status query";
  }
}

/** Implementation helper for GETINFO: knows the answers for questions about
 * download status information. */
STATIC int
getinfo_helper_downloads(control_connection_t *control_conn,
                   const char *question, char **answer,
                   const char **errmsg)
{
  download_status_t *dl_to_emit = NULL;
  smartlist_t *digest_list = NULL;

  /* Assert args are sane */
  tor_assert(control_conn != NULL);
  tor_assert(question != NULL);
  tor_assert(answer != NULL);
  tor_assert(errmsg != NULL);

  /* We check for this later to see if we should supply a default */
  *errmsg = NULL;

  /* Are we after networkstatus downloads? */
  if (!strcmpstart(question, "downloads/networkstatus/")) {
    getinfo_helper_downloads_networkstatus(
        question + strlen("downloads/networkstatus/"),
        &dl_to_emit, errmsg);
  /* Certificates? */
  } else if (!strcmpstart(question, "downloads/cert/")) {
    getinfo_helper_downloads_cert(
        question + strlen("downloads/cert/"),
        &dl_to_emit, &digest_list, errmsg);
  /* Router descriptors? */
  } else if (!strcmpstart(question, "downloads/desc/")) {
    getinfo_helper_downloads_desc(
        question + strlen("downloads/desc/"),
        &dl_to_emit, &digest_list, errmsg);
  /* Bridge descriptors? */
  } else if (!strcmpstart(question, "downloads/bridge/")) {
    getinfo_helper_downloads_bridge(
        question + strlen("downloads/bridge/"),
        &dl_to_emit, &digest_list, errmsg);
  } else {
    *errmsg = "Unknown download status query";
  }

  if (dl_to_emit) {
    *answer = download_status_to_string(dl_to_emit);

    return 0;
  } else if (digest_list) {
    *answer = digest_list_to_string(digest_list);
    SMARTLIST_FOREACH(digest_list, void *, s, tor_free(s));
    smartlist_free(digest_list);

    return 0;
  } else {
    if (!(*errmsg)) {
      *errmsg = "Unknown error";
    }

    return -1;
  }
}

/** Implementation helper for GETINFO: knows how to generate summaries of the
 * current states of things we send events about. */
static int
getinfo_helper_events(control_connection_t *control_conn,
                      const char *question, char **answer,
                      const char **errmsg)
{
  const or_options_t *options = get_options();
  (void) control_conn;
  if (!strcmp(question, "circuit-status")) {
    smartlist_t *status = smartlist_new();
    SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ_) {
      origin_circuit_t *circ;
      char *circdesc;
      const char *state;
      if (! CIRCUIT_IS_ORIGIN(circ_) || circ_->marked_for_close)
        continue;
      circ = TO_ORIGIN_CIRCUIT(circ_);

      if (circ->base_.state == CIRCUIT_STATE_OPEN)
        state = "BUILT";
      else if (circ->base_.state == CIRCUIT_STATE_GUARD_WAIT)
        state = "GUARD_WAIT";
      else if (circ->cpath)
        state = "EXTENDED";
      else
        state = "LAUNCHED";

      circdesc = circuit_describe_status_for_controller(circ);

      smartlist_add_asprintf(status, "%lu %s%s%s",
                   (unsigned long)circ->global_identifier,
                   state, *circdesc ? " " : "", circdesc);
      tor_free(circdesc);
    }
    SMARTLIST_FOREACH_END(circ_);
    *answer = smartlist_join_strings(status, "\r\n", 0, NULL);
    SMARTLIST_FOREACH(status, char *, cp, tor_free(cp));
    smartlist_free(status);
  } else if (!strcmp(question, "stream-status")) {
    smartlist_t *conns = get_connection_array();
    smartlist_t *status = smartlist_new();
    char buf[256];
    SMARTLIST_FOREACH_BEGIN(conns, connection_t *, base_conn) {
      const char *state;
      entry_connection_t *conn;
      circuit_t *circ;
      origin_circuit_t *origin_circ = NULL;
      if (base_conn->type != CONN_TYPE_AP ||
          base_conn->marked_for_close ||
          base_conn->state == AP_CONN_STATE_SOCKS_WAIT ||
          base_conn->state == AP_CONN_STATE_NATD_WAIT)
        continue;
      conn = TO_ENTRY_CONN(base_conn);
      switch (base_conn->state)
        {
        case AP_CONN_STATE_CONTROLLER_WAIT:
        case AP_CONN_STATE_CIRCUIT_WAIT:
          if (conn->socks_request &&
              SOCKS_COMMAND_IS_RESOLVE(conn->socks_request->command))
            state = "NEWRESOLVE";
          else
            state = "NEW";
          break;
        case AP_CONN_STATE_RENDDESC_WAIT:
        case AP_CONN_STATE_CONNECT_WAIT:
          state = "SENTCONNECT"; break;
        case AP_CONN_STATE_RESOLVE_WAIT:
          state = "SENTRESOLVE"; break;
        case AP_CONN_STATE_OPEN:
          state = "SUCCEEDED"; break;
        default:
          log_warn(LD_BUG, "Asked for stream in unknown state %d",
                   base_conn->state);
          continue;
        }
      circ = circuit_get_by_edge_conn(ENTRY_TO_EDGE_CONN(conn));
      if (circ && CIRCUIT_IS_ORIGIN(circ))
        origin_circ = TO_ORIGIN_CIRCUIT(circ);
      write_stream_target_to_buf(conn, buf, sizeof(buf));
      smartlist_add_asprintf(status, "%lu %s %lu %s",
                   (unsigned long) base_conn->global_identifier,state,
                   origin_circ?
                         (unsigned long)origin_circ->global_identifier : 0ul,
                   buf);
    } SMARTLIST_FOREACH_END(base_conn);
    *answer = smartlist_join_strings(status, "\r\n", 0, NULL);
    SMARTLIST_FOREACH(status, char *, cp, tor_free(cp));
    smartlist_free(status);
  } else if (!strcmp(question, "orconn-status")) {
    smartlist_t *conns = get_connection_array();
    smartlist_t *status = smartlist_new();
    SMARTLIST_FOREACH_BEGIN(conns, connection_t *, base_conn) {
      const char *state;
      char name[128];
      or_connection_t *conn;
      if (base_conn->type != CONN_TYPE_OR || base_conn->marked_for_close)
        continue;
      conn = TO_OR_CONN(base_conn);
      if (conn->base_.state == OR_CONN_STATE_OPEN)
        state = "CONNECTED";
      else if (conn->nickname)
        state = "LAUNCHED";
      else
        state = "NEW";
      orconn_target_get_name(name, sizeof(name), conn);
      smartlist_add_asprintf(status, "%s %s", name, state);
    } SMARTLIST_FOREACH_END(base_conn);
    *answer = smartlist_join_strings(status, "\r\n", 0, NULL);
    SMARTLIST_FOREACH(status, char *, cp, tor_free(cp));
    smartlist_free(status);
  } else if (!strcmpstart(question, "address-mappings/")) {
    time_t min_e, max_e;
    smartlist_t *mappings;
    question += strlen("address-mappings/");
    if (!strcmp(question, "all")) {
      min_e = 0; max_e = TIME_MAX;
    } else if (!strcmp(question, "cache")) {
      min_e = 2; max_e = TIME_MAX;
    } else if (!strcmp(question, "config")) {
      min_e = 0; max_e = 0;
    } else if (!strcmp(question, "control")) {
      min_e = 1; max_e = 1;
    } else {
      return 0;
    }
    mappings = smartlist_new();
    addressmap_get_mappings(mappings, min_e, max_e, 1);
    *answer = smartlist_join_strings(mappings, "\r\n", 0, NULL);
    SMARTLIST_FOREACH(mappings, char *, cp, tor_free(cp));
    smartlist_free(mappings);
  } else if (!strcmpstart(question, "status/")) {
    /* Note that status/ is not a catch-all for events; there's only supposed
     * to be a status GETINFO if there's a corresponding STATUS event. */
    if (!strcmp(question, "status/circuit-established")) {
      *answer = tor_strdup(have_completed_a_circuit() ? "1" : "0");
    } else if (!strcmp(question, "status/enough-dir-info")) {
      *answer = tor_strdup(router_have_minimum_dir_info() ? "1" : "0");
    } else if (!strcmp(question, "status/good-server-descriptor") ||
               !strcmp(question, "status/accepted-server-descriptor")) {
      /* They're equivalent for now, until we can figure out how to make
       * good-server-descriptor be what we want. See comment in
       * control-spec.txt. */
      *answer = tor_strdup(directories_have_accepted_server_descriptor()
                           ? "1" : "0");
    } else if (!strcmp(question, "status/reachability-succeeded/or")) {
      *answer = tor_strdup(
                    router_all_orports_seem_reachable(options) ?
                    "1" : "0");
    } else if (!strcmp(question, "status/reachability-succeeded/dir")) {
      *answer = tor_strdup(
                    router_dirport_seems_reachable(options) ?
                    "1" : "0");
    } else if (!strcmp(question, "status/reachability-succeeded")) {
      tor_asprintf(
          answer, "OR=%d DIR=%d",
          router_all_orports_seem_reachable(options) ? 1 : 0,
          router_dirport_seems_reachable(options) ? 1 : 0);
    } else if (!strcmp(question, "status/bootstrap-phase")) {
      *answer = control_event_boot_last_msg();
    } else if (!strcmpstart(question, "status/version/")) {
      int is_server = server_mode(options);
      networkstatus_t *c = networkstatus_get_latest_consensus();
      version_status_t status;
      const char *recommended;
      if (c) {
        recommended = is_server ? c->server_versions : c->client_versions;
        status = tor_version_is_obsolete(VERSION, recommended);
      } else {
        recommended = "?";
        status = VS_UNKNOWN;
      }

      if (!strcmp(question, "status/version/recommended")) {
        *answer = tor_strdup(recommended);
        return 0;
      }
      if (!strcmp(question, "status/version/current")) {
        switch (status)
          {
          case VS_RECOMMENDED: *answer = tor_strdup("recommended"); break;
          case VS_OLD: *answer = tor_strdup("obsolete"); break;
          case VS_NEW: *answer = tor_strdup("new"); break;
          case VS_NEW_IN_SERIES: *answer = tor_strdup("new in series"); break;
          case VS_UNRECOMMENDED: *answer = tor_strdup("unrecommended"); break;
          case VS_EMPTY: *answer = tor_strdup("none recommended"); break;
          case VS_UNKNOWN: *answer = tor_strdup("unknown"); break;
          default: tor_fragile_assert();
          }
      }
    } else if (!strcmp(question, "status/clients-seen")) {
      char *bridge_stats = geoip_get_bridge_stats_controller(time(NULL));
      if (!bridge_stats) {
        *errmsg = "No bridge-client stats available";
        return -1;
      }
      *answer = bridge_stats;
    } else if (!strcmp(question, "status/fresh-relay-descs")) {
      if (!server_mode(options)) {
        *errmsg = "Only relays have descriptors";
        return -1;
      }
      routerinfo_t *r;
      extrainfo_t *e;
      int result;
      if ((result = router_build_fresh_descriptor(&r, &e)) < 0) {
        switch (result) {
          case TOR_ROUTERINFO_ERROR_NO_EXT_ADDR:
            *errmsg = "Cannot get relay address while generating descriptor";
            break;
          case TOR_ROUTERINFO_ERROR_DIGEST_FAILED:
            *errmsg = "Key digest failed";
            break;
          case TOR_ROUTERINFO_ERROR_CANNOT_GENERATE:
            *errmsg = "Cannot generate router descriptor";
            break;
          default:
            *errmsg = "Error generating descriptor";
            break;
        }
        return -1;
      }
      size_t size = r->cache_info.signed_descriptor_len + 1;
      if (e) {
        size += e->cache_info.signed_descriptor_len + 1;
      }
      tor_assert(r->cache_info.signed_descriptor_len);
      char *descs = tor_malloc(size);
      char *cp = descs;
      memcpy(cp, signed_descriptor_get_body(&r->cache_info),
             r->cache_info.signed_descriptor_len);
      cp += r->cache_info.signed_descriptor_len - 1;
      if (e) {
        if (cp[0] == '\0') {
          cp[0] = '\n';
        } else if (cp[0] != '\n') {
          cp[1] = '\n';
          cp++;
        }
        memcpy(cp, signed_descriptor_get_body(&e->cache_info),
               e->cache_info.signed_descriptor_len);
        cp += e->cache_info.signed_descriptor_len - 1;
      }
      if (cp[0] == '\n') {
        cp[0] = '\0';
      } else if (cp[0] != '\0') {
        cp[1] = '\0';
      }
      *answer = descs;
      routerinfo_free(r);
      extrainfo_free(e);
    } else {
      return 0;
    }
  }
  return 0;
}

/** Implementation helper for GETINFO: knows how to enumerate hidden services
 * created via the control port. */
STATIC int
getinfo_helper_onions(control_connection_t *control_conn,
                      const char *question, char **answer,
                      const char **errmsg)
{
  smartlist_t *onion_list = NULL;
  (void) errmsg;  /* no errors from this method */

  if (control_conn && !strcmp(question, "onions/current")) {
    onion_list = control_conn->ephemeral_onion_services;
  } else if (!strcmp(question, "onions/detached")) {
    onion_list = get_detached_onion_services();
  } else {
    return 0;
  }
  if (!onion_list || smartlist_len(onion_list) == 0) {
    if (answer) {
      *answer = tor_strdup("");
    }
  } else {
    if (answer) {
      *answer = smartlist_join_strings(onion_list, "\r\n", 0, NULL);
    }
  }

  return 0;
}

/** Implementation helper for GETINFO: answers queries about network
 * liveness. */
static int
getinfo_helper_liveness(control_connection_t *control_conn,
                      const char *question, char **answer,
                      const char **errmsg)
{
  (void)control_conn;
  (void)errmsg;
  if (strcmp(question, "network-liveness") == 0) {
    if (get_cached_network_liveness()) {
      *answer = tor_strdup("up");
    } else {
      *answer = tor_strdup("down");
    }
  }

  return 0;
}

/** Implementation helper for GETINFO: answers queries about circuit onion
 * handshake rephist values */
STATIC int
getinfo_helper_rephist(control_connection_t *control_conn,
                       const char *question, char **answer,
                       const char **errmsg)
{
  (void) control_conn;
  (void) errmsg;
  int result;

  if (!strcmp(question, "stats/ntor/assigned")) {
    result =
      rep_hist_get_circuit_handshake_assigned(ONION_HANDSHAKE_TYPE_NTOR);
  } else if (!strcmp(question, "stats/ntor/requested")) {
    result =
      rep_hist_get_circuit_handshake_requested(ONION_HANDSHAKE_TYPE_NTOR);
  } else if (!strcmp(question, "stats/tap/assigned")) {
    result =
      rep_hist_get_circuit_handshake_assigned(ONION_HANDSHAKE_TYPE_TAP);
  } else if (!strcmp(question, "stats/tap/requested")) {
    result =
      rep_hist_get_circuit_handshake_requested(ONION_HANDSHAKE_TYPE_TAP);
  } else {
    *errmsg = "Unrecognized handshake type";
    return -1;
  }

  tor_asprintf(answer, "%d", result);

  return 0;
}

/** Implementation helper for GETINFO: answers queries about shared random
 * value. */
static int
getinfo_helper_sr(control_connection_t *control_conn,
                  const char *question, char **answer,
                  const char **errmsg)
{
  (void) control_conn;
  (void) errmsg;

  if (!strcmp(question, "sr/current")) {
    *answer = sr_get_current_for_control();
  } else if (!strcmp(question, "sr/previous")) {
    *answer = sr_get_previous_for_control();
  }
  /* Else statement here is unrecognized key so do nothing. */

  return 0;
}

/** Callback function for GETINFO: on a given control connection, try to
 * answer the question <b>q</b> and store the newly-allocated answer in
 * *<b>a</b>. If an internal error occurs, return -1 and optionally set
 * *<b>error_out</b> to point to an error message to be delivered to the
 * controller. On success, _or if the key is not recognized_, return 0. Do not
 * set <b>a</b> if the key is not recognized but you may set <b>error_out</b>
 * to improve the error message.
 */
typedef int (*getinfo_helper_t)(control_connection_t *,
                                const char *q, char **a,
                                const char **error_out);

/** A single item for the GETINFO question-to-answer-function table. */
typedef struct getinfo_item_t {
  const char *varname; /**< The value (or prefix) of the question. */
  getinfo_helper_t fn; /**< The function that knows the answer: NULL if
                        * this entry is documentation-only. */
  const char *desc; /**< Description of the variable. */
  int is_prefix; /** Must varname match exactly, or must it be a prefix? */
} getinfo_item_t;

#define ITEM(name, fn, desc) { name, getinfo_helper_##fn, desc, 0 }
#define PREFIX(name, fn, desc) { name, getinfo_helper_##fn, desc, 1 }
#define DOC(name, desc) { name, NULL, desc, 0 }

/** Table mapping questions accepted by GETINFO to the functions that know how
 * to answer them. */
static const getinfo_item_t getinfo_items[] = {
  ITEM("version", misc, "The current version of Tor."),
  ITEM("bw-event-cache", misc, "Cached BW events for a short interval."),
  ITEM("config-file", misc, "Current location of the \"torrc\" file."),
  ITEM("config-defaults-file", misc, "Current location of the defaults file."),
  ITEM("config-text", misc,
       "Return the string that would be written by a saveconf command."),
  ITEM("config-can-saveconf", misc,
       "Is it possible to save the configuration to the \"torrc\" file?"),
  ITEM("accounting/bytes", accounting,
       "Number of bytes read/written so far in the accounting interval."),
  ITEM("accounting/bytes-left", accounting,
      "Number of bytes left to write/read so far in the accounting interval."),
  ITEM("accounting/enabled", accounting, "Is accounting currently enabled?"),
  ITEM("accounting/hibernating", accounting, "Are we hibernating or awake?"),
  ITEM("accounting/interval-start", accounting,
       "Time when the accounting period starts."),
  ITEM("accounting/interval-end", accounting,
       "Time when the accounting period ends."),
  ITEM("accounting/interval-wake", accounting,
       "Time to wake up in this accounting period."),
  ITEM("helper-nodes", entry_guards, NULL), /* deprecated */
  ITEM("entry-guards", entry_guards,
       "Which nodes are we using as entry guards?"),
  ITEM("fingerprint", misc, NULL),
  PREFIX("config/", config, "Current configuration values."),
  DOC("config/names",
      "List of configuration options, types, and documentation."),
  DOC("config/defaults",
      "List of default values for configuration options. "
      "See also config/names"),
  PREFIX("current-time/", current_time, "Current time."),
  DOC("current-time/local", "Current time on the local system."),
  DOC("current-time/utc", "Current UTC time."),
  PREFIX("downloads/networkstatus/", downloads,
         "Download statuses for networkstatus objects"),
  DOC("downloads/networkstatus/ns",
      "Download status for current-mode networkstatus download"),
  DOC("downloads/networkstatus/ns/bootstrap",
      "Download status for bootstrap-time networkstatus download"),
  DOC("downloads/networkstatus/ns/running",
      "Download status for run-time networkstatus download"),
  DOC("downloads/networkstatus/microdesc",
      "Download status for current-mode microdesc download"),
  DOC("downloads/networkstatus/microdesc/bootstrap",
      "Download status for bootstrap-time microdesc download"),
  DOC("downloads/networkstatus/microdesc/running",
      "Download status for run-time microdesc download"),
  PREFIX("downloads/cert/", downloads,
         "Download statuses for certificates, by id fingerprint and "
         "signing key"),
  DOC("downloads/cert/fps",
      "List of authority fingerprints for which any download statuses "
      "exist"),
  DOC("downloads/cert/fp/<fp>",
      "Download status for <fp> with the default signing key; corresponds "
      "to /fp/ URLs on directory server."),
  DOC("downloads/cert/fp/<fp>/sks",
      "List of signing keys for which specific download statuses are "
      "available for this id fingerprint"),
  DOC("downloads/cert/fp/<fp>/<sk>",
      "Download status for <fp> with signing key <sk>; corresponds "
      "to /fp-sk/ URLs on directory server."),
  PREFIX("downloads/desc/", downloads,
         "Download statuses for router descriptors, by descriptor digest"),
  DOC("downloads/desc/descs",
      "Return a list of known router descriptor digests"),
  DOC("downloads/desc/<desc>",
      "Return a download status for a given descriptor digest"),
  PREFIX("downloads/bridge/", downloads,
         "Download statuses for bridge descriptors, by bridge identity "
         "digest"),
  DOC("downloads/bridge/bridges",
      "Return a list of configured bridge identity digests with download "
      "statuses"),
  DOC("downloads/bridge/<desc>",
      "Return a download status for a given bridge identity digest"),
  ITEM("info/names", misc,
       "List of GETINFO options, types, and documentation."),
  ITEM("events/names", misc,
       "Events that the controller can ask for with SETEVENTS."),
  ITEM("signal/names", misc, "Signal names recognized by the SIGNAL command"),
  ITEM("features/names", misc, "What arguments can USEFEATURE take?"),
  PREFIX("desc/id/", dir, "Router descriptors by ID."),
  PREFIX("desc/name/", dir, "Router descriptors by nickname."),
  ITEM("desc/all-recent", dir,
       "All non-expired, non-superseded router descriptors."),
  ITEM("desc/download-enabled", dir,
       "Do we try to download router descriptors?"),
  ITEM("desc/all-recent-extrainfo-hack", dir, NULL), /* Hack. */
  ITEM("md/all", dir, "All known microdescriptors."),
  PREFIX("md/id/", dir, "Microdescriptors by ID"),
  PREFIX("md/name/", dir, "Microdescriptors by name"),
  ITEM("md/download-enabled", dir,
       "Do we try to download microdescriptors?"),
  PREFIX("extra-info/digest/", dir, "Extra-info documents by digest."),
  PREFIX("hs/client/desc/id", dir,
         "Hidden Service descriptor in client's cache by onion."),
  PREFIX("hs/service/desc/id/", dir,
         "Hidden Service descriptor in services's cache by onion."),
  PREFIX("net/listeners/", listeners, "Bound addresses by type"),
  ITEM("ns/all", networkstatus,
       "Brief summary of router status (v2 directory format)"),
  PREFIX("ns/id/", networkstatus,
         "Brief summary of router status by ID (v2 directory format)."),
  PREFIX("ns/name/", networkstatus,
         "Brief summary of router status by nickname (v2 directory format)."),
  PREFIX("ns/purpose/", networkstatus,
         "Brief summary of router status by purpose (v2 directory format)."),
  PREFIX("consensus/", networkstatus,
         "Information about and from the ns consensus."),
  ITEM("network-status", dir,
       "Brief summary of router status (v1 directory format)"),
  ITEM("network-liveness", liveness,
       "Current opinion on whether the network is live"),
  ITEM("circuit-status", events, "List of current circuits originating here."),
  ITEM("stream-status", events,"List of current streams."),
  ITEM("orconn-status", events, "A list of current OR connections."),
  ITEM("dormant", misc,
       "Is Tor dormant (not building circuits because it's idle)?"),
  PREFIX("address-mappings/", events, NULL),
  DOC("address-mappings/all", "Current address mappings."),
  DOC("address-mappings/cache", "Current cached DNS replies."),
  DOC("address-mappings/config",
      "Current address mappings from configuration."),
  DOC("address-mappings/control", "Current address mappings from controller."),
  PREFIX("status/", events, NULL),
  DOC("status/circuit-established",
      "Whether we think client functionality is working."),
  DOC("status/enough-dir-info",
      "Whether we have enough up-to-date directory information to build "
      "circuits."),
  DOC("status/bootstrap-phase",
      "The last bootstrap phase status event that Tor sent."),
  DOC("status/clients-seen",
      "Breakdown of client countries seen by a bridge."),
  DOC("status/fresh-relay-descs",
      "A fresh relay/ei descriptor pair for Tor's current state. Not stored."),
  DOC("status/version/recommended", "List of currently recommended versions."),
  DOC("status/version/current", "Status of the current version."),
  ITEM("address", misc, "IP address of this Tor host, if we can guess it."),
  ITEM("address/v4", misc,
       "IPv4 address of this Tor host, if we can guess it."),
  ITEM("address/v6", misc,
       "IPv6 address of this Tor host, if we can guess it."),
  ITEM("traffic/read", misc,"Bytes read since the process was started."),
  ITEM("traffic/written", misc,
       "Bytes written since the process was started."),
  ITEM("uptime", misc, "Uptime of the Tor daemon in seconds."),
  ITEM("process/pid", misc, "Process id belonging to the main tor process."),
  ITEM("process/uid", misc, "User id running the tor process."),
  ITEM("process/user", misc,
       "Username under which the tor process is running."),
  ITEM("process/descriptor-limit", misc, "File descriptor limit."),
  ITEM("limits/max-mem-in-queues", misc, "Actual limit on memory in queues"),
  PREFIX("desc-annotations/id/", dir, "Router annotations by hexdigest."),
  PREFIX("dir/server/", dir,"Router descriptors as retrieved from a DirPort."),
  PREFIX("dir/status/", dir,
         "v2 networkstatus docs as retrieved from a DirPort."),
  ITEM("dir/status-vote/current/consensus", dir,
       "v3 Networkstatus consensus as retrieved from a DirPort."),
  ITEM("dir/status-vote/current/consensus-microdesc", dir,
       "v3 Microdescriptor consensus as retrieved from a DirPort."),
  ITEM("exit-policy/default", policies,
       "The default value appended to the configured exit policy."),
  ITEM("exit-policy/reject-private/default", policies,
       "The default rules appended to the configured exit policy by"
       " ExitPolicyRejectPrivate."),
  ITEM("exit-policy/reject-private/relay", policies,
       "The relay-specific rules appended to the configured exit policy by"
       " ExitPolicyRejectPrivate and/or ExitPolicyRejectLocalInterfaces."),
  ITEM("exit-policy/full", policies, "The entire exit policy of onion router"),
  ITEM("exit-policy/ipv4", policies, "IPv4 parts of exit policy"),
  ITEM("exit-policy/ipv6", policies, "IPv6 parts of exit policy"),
  PREFIX("ip-to-country/", geoip, "Perform a GEOIP lookup"),
  ITEM("onions/current", onions,
       "Onion services owned by the current control connection."),
  ITEM("onions/detached", onions,
       "Onion services detached from the control connection."),
  ITEM("sr/current", sr, "Get current shared random value."),
  ITEM("sr/previous", sr, "Get previous shared random value."),
  PREFIX("stats/ntor/", rephist, "NTor circuit handshake stats."),
  ITEM("stats/ntor/assigned", rephist,
       "Assigned NTor circuit handshake stats."),
  ITEM("stats/ntor/requested", rephist,
       "Requested NTor circuit handshake stats."),
  PREFIX("stats/tap/", rephist, "TAP circuit handshake stats."),
  ITEM("stats/tap/assigned", rephist,
       "Assigned TAP circuit handshake stats."),
  ITEM("stats/tap/requested", rephist,
       "Requested TAP circuit handshake stats."),
  { NULL, NULL, NULL, 0 }
};

/** Allocate and return a list of recognized GETINFO options. */
static char *
list_getinfo_options(void)
{
  int i;
  smartlist_t *lines = smartlist_new();
  char *ans;
  for (i = 0; getinfo_items[i].varname; ++i) {
    if (!getinfo_items[i].desc)
      continue;

    smartlist_add_asprintf(lines, "%s%s -- %s\n",
                 getinfo_items[i].varname,
                 getinfo_items[i].is_prefix ? "*" : "",
                 getinfo_items[i].desc);
  }
  smartlist_sort_strings(lines);

  ans = smartlist_join_strings(lines, "", 0, NULL);
  SMARTLIST_FOREACH(lines, char *, cp, tor_free(cp));
  smartlist_free(lines);

  return ans;
}

/** Lookup the 'getinfo' entry <b>question</b>, and return
 * the answer in <b>*answer</b> (or NULL if key not recognized).
 * Return 0 if success or unrecognized, or -1 if recognized but
 * internal error. */
static int
handle_getinfo_helper(control_connection_t *control_conn,
                      const char *question, char **answer,
                      const char **err_out)
{
  int i;
  *answer = NULL; /* unrecognized key by default */

  for (i = 0; getinfo_items[i].varname; ++i) {
    int match;
    if (getinfo_items[i].is_prefix)
      match = !strcmpstart(question, getinfo_items[i].varname);
    else
      match = !strcmp(question, getinfo_items[i].varname);
    if (match) {
      tor_assert(getinfo_items[i].fn);
      return getinfo_items[i].fn(control_conn, question, answer, err_out);
    }
  }

  return 0; /* unrecognized */
}

const control_cmd_syntax_t getinfo_syntax = {
  .max_args = UINT_MAX,
};

/** Called when we receive a GETINFO command.  Try to fetch all requested
 * information, and reply with information or error message. */
int
handle_control_getinfo(control_connection_t *conn,
                       const control_cmd_args_t *args)
{
  const smartlist_t *questions = args->args;
  smartlist_t *answers = smartlist_new();
  smartlist_t *unrecognized = smartlist_new();
  char *ans = NULL;

  SMARTLIST_FOREACH_BEGIN(questions, const char *, q) {
    const char *errmsg = NULL;

    if (handle_getinfo_helper(conn, q, &ans, &errmsg) < 0) {
      if (!errmsg)
        errmsg = "Internal error";
      control_write_endreply(conn, 551, errmsg);
      goto done;
    }
    if (!ans) {
      if (errmsg) {
        /* use provided error message */
        control_reply_add_str(unrecognized, 552, errmsg);
      } else {
        /* use default error message */
        control_reply_add_printf(unrecognized, 552,
                                 "Unrecognized key \"%s\"", q);
      }
    } else {
      control_reply_add_one_kv(answers, 250, KV_RAW, q, ans);
      tor_free(ans);
    }
  } SMARTLIST_FOREACH_END(q);

  control_reply_add_done(answers);

  if (smartlist_len(unrecognized)) {
    control_write_reply_lines(conn, unrecognized);
    /* If there were any unrecognized queries, don't write real answers */
    goto done;
  }

  control_write_reply_lines(conn, answers);

 done:
  control_reply_free(answers);
  control_reply_free(unrecognized);

  return 0;
}
