/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file reachability.c
 * \brief Router reachability testing; run by authorities to tell who is
 * running.
 */

#include "core/or/or.h"
#include "feature/dirauth/reachability.h"

#include "app/config/config.h"
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#include "core/or/command.h"
#include "feature/dirauth/authmode.h"
#include "feature/dirauth/dirauth_sys.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/torcert.h"
#include "feature/stats/rephist.h"

#include "feature/dirauth/dirauth_options_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist_st.h"

/** Called when a TLS handshake has completed successfully with a
 * router listening at <b>address</b>:<b>or_port</b>, and has yielded
 * a certificate with digest <b>digest_rcvd</b>.
 *
 * Inform the reachability checker that we could get to this relay.
 */
void
dirserv_orconn_tls_done(const tor_addr_t *addr,
                        uint16_t or_port,
                        const char *digest_rcvd,
                        const ed25519_public_key_t *ed_id_rcvd)
{
  node_t *node = NULL;
  tor_addr_port_t orport;
  routerinfo_t *ri = NULL;
  time_t now = time(NULL);
  tor_assert(addr);
  tor_assert(digest_rcvd);

  node = node_get_mutable_by_id(digest_rcvd);
  if (node == NULL || node->ri == NULL)
    return;

  ri = node->ri;

  if (dirauth_get_options()->AuthDirTestEd25519LinkKeys &&
      node_supports_ed25519_link_authentication(node, 1) &&
      ri->cache_info.signing_key_cert) {
    /* We allow the node to have an ed25519 key if we haven't been told one in
     * the routerinfo, but if we *HAVE* been told one in the routerinfo, it
     * needs to match. */
    const ed25519_public_key_t *expected_id =
      &ri->cache_info.signing_key_cert->signing_key;
    tor_assert(!ed25519_public_key_is_zero(expected_id));
    if (! ed_id_rcvd || ! ed25519_pubkey_eq(ed_id_rcvd, expected_id)) {
      log_info(LD_DIRSERV, "Router at %s:%d with RSA ID %s "
               "did not present expected Ed25519 ID.",
               fmt_addr(addr), or_port, hex_str(digest_rcvd, DIGEST_LEN));
      return; /* Don't mark it as reachable. */
    }
  }

  tor_addr_copy(&orport.addr, addr);
  orport.port = or_port;
  if (router_has_orport(ri, &orport)) {
    /* Found the right router.  */
    if (!authdir_mode_bridge(get_options()) ||
        ri->purpose == ROUTER_PURPOSE_BRIDGE) {
      char addrstr[TOR_ADDR_BUF_LEN];
      /* This is a bridge or we're not a bridge authority --
         mark it as reachable.  */
      log_info(LD_DIRSERV, "Found router %s to be reachable at %s:%d. Yay.",
               router_describe(ri),
               tor_addr_to_str(addrstr, addr, sizeof(addrstr), 1),
               ri->ipv4_orport);
      if (tor_addr_family(addr) == AF_INET) {
        rep_hist_note_router_reachable(digest_rcvd, addr, or_port, now);
        node->last_reachable = now;
      } else if (tor_addr_family(addr) == AF_INET6) {
        /* No rephist for IPv6.  */
        node->last_reachable6 = now;
      }
    }
  }
}

/** Called when we, as an authority, receive a new router descriptor either as
 * an upload or a download.  Used to decide whether to relaunch reachability
 * testing for the server. */
int
dirserv_should_launch_reachability_test(const routerinfo_t *ri,
                                        const routerinfo_t *ri_old)
{
  if (!authdir_mode_handles_descs(get_options(), ri->purpose))
    return 0;
  if (! dirauth_get_options()->AuthDirTestReachability)
    return 0;
  if (!ri_old) {
    /* New router: Launch an immediate reachability test, so we will have an
     * opinion soon in case we're generating a consensus soon */
    log_info(LD_DIR, "descriptor for new router %s", router_describe(ri));
    return 1;
  }
  if (ri_old->is_hibernating && !ri->is_hibernating) {
    /* It just came out of hibernation; launch a reachability test */
    log_info(LD_DIR, "out of hibernation: router %s", router_describe(ri));
    return 1;
  }
  if (! routers_have_same_or_addrs(ri, ri_old)) {
    /* Address or port changed; launch a reachability test */
    log_info(LD_DIR, "address or port changed: router %s",
             router_describe(ri));
    return 1;
  }
  return 0;
}

/** Helper function for dirserv_test_reachability(). Start a TLS
 * connection to <b>router</b>, and annotate it with when we started
 * the test. */
void
dirserv_single_reachability_test(time_t now, routerinfo_t *router)
{
  const dirauth_options_t *dirauth_options = dirauth_get_options();
  channel_t *chan = NULL;
  const node_t *node = NULL;
  const ed25519_public_key_t *ed_id_key;
  (void) now;

  tor_assert(router);
  node = node_get_by_id(router->cache_info.identity_digest);
  tor_assert(node);

  if (dirauth_options->AuthDirTestEd25519LinkKeys &&
      node_supports_ed25519_link_authentication(node, 1) &&
      router->cache_info.signing_key_cert) {
    ed_id_key = &router->cache_info.signing_key_cert->signing_key;
  } else {
    ed_id_key = NULL;
  }

  /* IPv4. */
  log_info(LD_OR,"Testing reachability of %s at %s:%u.",
            router->nickname, fmt_addr(&router->ipv4_addr),
            router->ipv4_orport);
  chan = channel_tls_connect(&router->ipv4_addr, router->ipv4_orport,
                             router->cache_info.identity_digest,
                             ed_id_key);
  if (chan) command_setup_channel(chan);

  /* Possible IPv6. */
  if (dirauth_get_options()->AuthDirHasIPv6Connectivity == 1 &&
      !tor_addr_is_null(&router->ipv6_addr)) {
    char addrstr[TOR_ADDR_BUF_LEN];
    log_info(LD_OR, "Testing reachability of %s at %s:%u.",
             router->nickname,
             tor_addr_to_str(addrstr, &router->ipv6_addr, sizeof(addrstr), 1),
             router->ipv6_orport);
    chan = channel_tls_connect(&router->ipv6_addr, router->ipv6_orport,
                               router->cache_info.identity_digest,
                               ed_id_key);
    if (chan) command_setup_channel(chan);
  }
}

/** Auth dir server only: load balance such that we only
 * try a few connections per call.
 *
 * The load balancing is such that if we get called once every ten
 * seconds, we will cycle through all the tests in
 * REACHABILITY_TEST_CYCLE_PERIOD seconds (a bit over 20 minutes).
 */
void
dirserv_test_reachability(time_t now)
{
  /* XXX decide what to do here; see or-talk thread "purging old router
   * information, revocation." -NM
   * We can't afford to mess with this in 0.1.2.x. The reason is that
   * if we stop doing reachability tests on some of routerlist, then
   * we'll for-sure think they're down, which may have unexpected
   * effects in other parts of the code. It doesn't hurt much to do
   * the testing, and directory authorities are easy to upgrade. Let's
   * wait til 0.2.0. -RD */
//  time_t cutoff = now - ROUTER_MAX_AGE_TO_PUBLISH;
  if (! dirauth_get_options()->AuthDirTestReachability)
    return;

  routerlist_t *rl = router_get_routerlist();
  static char ctr = 0;
  int bridge_auth = authdir_mode_bridge(get_options());

  SMARTLIST_FOREACH_BEGIN(rl->routers, routerinfo_t *, router) {
    const char *id_digest = router->cache_info.identity_digest;
    if (router_is_me(router))
      continue;
    if (bridge_auth && router->purpose != ROUTER_PURPOSE_BRIDGE)
      continue; /* bridge authorities only test reachability on bridges */
//    if (router->cache_info.published_on > cutoff)
//      continue;
    if ((((uint8_t)id_digest[0]) % REACHABILITY_MODULO_PER_TEST) == ctr) {
      dirserv_single_reachability_test(now, router);
    }
  } SMARTLIST_FOREACH_END(router);
  ctr = (ctr + 1) % REACHABILITY_MODULO_PER_TEST; /* increment ctr */
}
