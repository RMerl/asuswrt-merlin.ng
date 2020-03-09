/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "feature/control/fmt_serverstatus.h"

#include "app/config/config.h"
#include "feature/dirauth/authmode.h"
#include "feature/dirauth/voteflags.h"// XXXX remove
#include "feature/nodelist/describe.h"
#include "feature/nodelist/nodelist.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"

/**
 * Allocate and return a description of the status of the server <b>desc</b>,
 * for use in a v1-style router-status line.  The server is listed
 * as running iff <b>is_live</b> is true.
 *
 * This is deprecated: it's only used for controllers that want outputs in
 * the old format.
 */
static char *
list_single_server_status(const routerinfo_t *desc, int is_live)
{
  char buf[MAX_NICKNAME_LEN+HEX_DIGEST_LEN+4]; /* !nickname=$hexdigest\0 */
  char *cp;
  const node_t *node;

  tor_assert(desc);

  cp = buf;
  if (!is_live) {
    *cp++ = '!';
  }
  node = node_get_by_id(desc->cache_info.identity_digest);
  if (node && node->is_valid) {
    strlcpy(cp, desc->nickname, sizeof(buf)-(cp-buf));
    cp += strlen(cp);
    *cp++ = '=';
  }
  *cp++ = '$';
  base16_encode(cp, HEX_DIGEST_LEN+1, desc->cache_info.identity_digest,
                DIGEST_LEN);
  return tor_strdup(buf);
}

/** Based on the routerinfo_ts in <b>routers</b>, allocate the
 * contents of a v1-style router-status line, and store it in
 * *<b>router_status_out</b>.  Return 0 on success, -1 on failure.
 *
 * If for_controller is true, include the routers with very old descriptors.
 *
 * This is deprecated: it's only used for controllers that want outputs in
 * the old format.
 */
int
list_server_status_v1(smartlist_t *routers, char **router_status_out,
                      int for_controller)
{
  /* List of entries in a router-status style: An optional !, then an optional
   * equals-suffixed nickname, then a dollar-prefixed hexdigest. */
  smartlist_t *rs_entries;
  time_t now = time(NULL);
  time_t cutoff = now - ROUTER_MAX_AGE_TO_PUBLISH;
  /* We include v2 dir auths here too, because they need to answer
   * controllers. Eventually we'll deprecate this whole function;
   * see also networkstatus_getinfo_by_purpose(). */
  tor_assert(router_status_out);

  rs_entries = smartlist_new();

  SMARTLIST_FOREACH_BEGIN(routers, routerinfo_t *, ri) {
    const node_t *node = node_get_by_id(ri->cache_info.identity_digest);
    tor_assert(node);
    if (for_controller) {
      char name_buf[MAX_VERBOSE_NICKNAME_LEN+2];
      char *cp = name_buf;
      if (!node->is_running)
        *cp++ = '!';
      router_get_verbose_nickname(cp, ri);
      smartlist_add_strdup(rs_entries, name_buf);
    } else if (ri->cache_info.published_on >= cutoff) {
      smartlist_add(rs_entries, list_single_server_status(ri,
                                                          node->is_running));
    }
  } SMARTLIST_FOREACH_END(ri);

  *router_status_out = smartlist_join_strings(rs_entries, " ", 0, NULL);

  SMARTLIST_FOREACH(rs_entries, char *, cp, tor_free(cp));
  smartlist_free(rs_entries);

  return 0;
}
