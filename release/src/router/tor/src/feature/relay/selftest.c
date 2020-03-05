/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file selftest.c
 * \brief Relay self-testing
 *
 * Relays need to make sure that their own ports are reasonable, and estimate
 * their own bandwidth, before publishing.
 */

#define SELFTEST_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/netstatus.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/crypt_path_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/relay.h"
#include "feature/control/control_events.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/authority_cert_st.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist.h" // but...
#include "feature/nodelist/routerset.h"
#include "feature/nodelist/torcert.h"
#include "feature/relay/relay_periodic.h"
#include "feature/relay/router.h"
#include "feature/relay/selftest.h"

/** Whether we can reach our ORPort from the outside. */
static int can_reach_or_port = 0;
/** Whether we can reach our DirPort from the outside. */
static int can_reach_dir_port = 0;

/** Forget what we have learned about our reachability status. */
void
router_reset_reachability(void)
{
  can_reach_or_port = can_reach_dir_port = 0;
}

/** Return 1 if we won't do reachability checks, because:
 *   - AssumeReachable is set, or
 *   - the network is disabled.
 * Otherwise, return 0.
 */
static int
router_reachability_checks_disabled(const or_options_t *options)
{
  return options->AssumeReachable ||
         net_is_disabled();
}

/** Return 0 if we need to do an ORPort reachability check, because:
 *   - no reachability check has been done yet, or
 *   - we've initiated reachability checks, but none have succeeded.
 *  Return 1 if we don't need to do an ORPort reachability check, because:
 *   - we've seen a successful reachability check, or
 *   - AssumeReachable is set, or
 *   - the network is disabled.
 */
int
check_whether_orport_reachable(const or_options_t *options)
{
  int reach_checks_disabled = router_reachability_checks_disabled(options);
  return reach_checks_disabled ||
         can_reach_or_port;
}

/** Return 0 if we need to do a DirPort reachability check, because:
 *   - no reachability check has been done yet, or
 *   - we've initiated reachability checks, but none have succeeded.
 *  Return 1 if we don't need to do a DirPort reachability check, because:
 *   - we've seen a successful reachability check, or
 *   - there is no DirPort set, or
 *   - AssumeReachable is set, or
 *   - the network is disabled.
 */
int
check_whether_dirport_reachable(const or_options_t *options)
{
  int reach_checks_disabled = router_reachability_checks_disabled(options) ||
                              !options->DirPort_set;
  return reach_checks_disabled ||
         can_reach_dir_port;
}

/** See if we currently believe our ORPort or DirPort to be
 * unreachable. If so, return 1 else return 0.
 */
static int
router_should_check_reachability(int test_or, int test_dir)
{
  const routerinfo_t *me = router_get_my_routerinfo();
  const or_options_t *options = get_options();

  if (!me)
    return 0;

  if (routerset_contains_router(options->ExcludeNodes, me, -1) &&
      options->StrictNodes) {
    /* If we've excluded ourself, and StrictNodes is set, we can't test
     * ourself. */
    if (test_or || test_dir) {
#define SELF_EXCLUDED_WARN_INTERVAL 3600
      static ratelim_t warning_limit=RATELIM_INIT(SELF_EXCLUDED_WARN_INTERVAL);
      log_fn_ratelim(&warning_limit, LOG_WARN, LD_CIRC,
                 "Can't peform self-tests for this relay: we have "
                 "listed ourself in ExcludeNodes, and StrictNodes is set. "
                 "We cannot learn whether we are usable, and will not "
                 "be able to advertise ourself.");
    }
    return 0;
  }
  return 1;
}

/** Allocate and return a new extend_info_t that can be used to build
 * a circuit to or through the router <b>r</b>. Uses the primary
 * address of the router, so should only be called on a server. */
static extend_info_t *
extend_info_from_router(const routerinfo_t *r)
{
  crypto_pk_t *rsa_pubkey;
  extend_info_t *info;
  tor_addr_port_t ap;
  tor_assert(r);

  /* Make sure we don't need to check address reachability */
  tor_assert_nonfatal(router_skip_or_reachability(get_options(), 0));

  const ed25519_public_key_t *ed_id_key;
  if (r->cache_info.signing_key_cert)
    ed_id_key = &r->cache_info.signing_key_cert->signing_key;
  else
    ed_id_key = NULL;

  router_get_prim_orport(r, &ap);
  rsa_pubkey = router_get_rsa_onion_pkey(r->onion_pkey, r->onion_pkey_len);
  info = extend_info_new(r->nickname, r->cache_info.identity_digest,
                         ed_id_key,
                         rsa_pubkey, r->onion_curve25519_pkey,
                         &ap.addr, ap.port);
  crypto_pk_free(rsa_pubkey);
  return info;
}

/** Some time has passed, or we just got new directory information.
 * See if we currently believe our ORPort or DirPort to be
 * unreachable. If so, launch a new test for it.
 *
 * For ORPort, we simply try making a circuit that ends at ourselves.
 * Success is noticed in onionskin_answer().
 *
 * For DirPort, we make a connection via Tor to our DirPort and ask
 * for our own server descriptor.
 * Success is noticed in connection_dir_client_reached_eof().
 */
void
router_do_reachability_checks(int test_or, int test_dir)
{
  const routerinfo_t *me = router_get_my_routerinfo();
  const or_options_t *options = get_options();
  int orport_reachable = check_whether_orport_reachable(options);
  tor_addr_t addr;

  if (router_should_check_reachability(test_or, test_dir)) {
    if (test_or && (!orport_reachable || !circuit_enough_testing_circs())) {
      extend_info_t *ei = extend_info_from_router(me);
      /* XXX IPv6 self testing */
      log_info(LD_CIRC, "Testing %s of my ORPort: %s:%d.",
               !orport_reachable ? "reachability" : "bandwidth",
               fmt_addr32(me->addr), me->or_port);
      circuit_launch_by_extend_info(CIRCUIT_PURPOSE_TESTING, ei,
                              CIRCLAUNCH_NEED_CAPACITY|CIRCLAUNCH_IS_INTERNAL);
      extend_info_free(ei);
    }

    /* XXX IPv6 self testing */
    tor_addr_from_ipv4h(&addr, me->addr);
    if (test_dir && !check_whether_dirport_reachable(options) &&
        !connection_get_by_type_addr_port_purpose(
                  CONN_TYPE_DIR, &addr, me->dir_port,
                  DIR_PURPOSE_FETCH_SERVERDESC)) {
      tor_addr_port_t my_orport, my_dirport;
      memcpy(&my_orport.addr, &addr, sizeof(addr));
      memcpy(&my_dirport.addr, &addr, sizeof(addr));
      my_orport.port = me->or_port;
      my_dirport.port = me->dir_port;
      /* ask myself, via tor, for my server descriptor. */
      directory_request_t *req =
        directory_request_new(DIR_PURPOSE_FETCH_SERVERDESC);
      directory_request_set_or_addr_port(req, &my_orport);
      directory_request_set_dir_addr_port(req, &my_dirport);
      directory_request_set_directory_id_digest(req,
                                              me->cache_info.identity_digest);
      // ask via an anon circuit, connecting to our dirport.
      directory_request_set_indirection(req, DIRIND_ANON_DIRPORT);
      directory_request_set_resource(req, "authority.z");
      directory_initiate_request(req);
      directory_request_free(req);
    }
  }
}

/** Annotate that we found our ORPort reachable. */
void
router_orport_found_reachable(void)
{
  const routerinfo_t *me = router_get_my_routerinfo();
  const or_options_t *options = get_options();
  if (!can_reach_or_port && me) {
    char *address = tor_dup_ip(me->addr);
    log_notice(LD_OR,"Self-testing indicates your ORPort is reachable from "
               "the outside. Excellent.%s",
               options->PublishServerDescriptor_ != NO_DIRINFO
               && check_whether_dirport_reachable(options) ?
                 " Publishing server descriptor." : "");
    can_reach_or_port = 1;
    mark_my_descriptor_dirty("ORPort found reachable");
    /* This is a significant enough change to upload immediately,
     * at least in a test network */
    if (options->TestingTorNetwork == 1) {
      reschedule_descriptor_update_check();
    }
    control_event_server_status(LOG_NOTICE,
                                "REACHABILITY_SUCCEEDED ORADDRESS=%s:%d",
                                address, me->or_port);
    tor_free(address);
  }
}

/** Annotate that we found our DirPort reachable. */
void
router_dirport_found_reachable(void)
{
  const routerinfo_t *me = router_get_my_routerinfo();
  const or_options_t *options = get_options();
  if (!can_reach_dir_port && me) {
    char *address = tor_dup_ip(me->addr);
    log_notice(LD_DIRSERV,"Self-testing indicates your DirPort is reachable "
               "from the outside. Excellent.%s",
               options->PublishServerDescriptor_ != NO_DIRINFO
               && check_whether_orport_reachable(options) ?
               " Publishing server descriptor." : "");
    can_reach_dir_port = 1;
    if (router_should_advertise_dirport(options, me->dir_port)) {
      mark_my_descriptor_dirty("DirPort found reachable");
      /* This is a significant enough change to upload immediately,
       * at least in a test network */
      if (options->TestingTorNetwork == 1) {
        reschedule_descriptor_update_check();
      }
    }
    control_event_server_status(LOG_NOTICE,
                                "REACHABILITY_SUCCEEDED DIRADDRESS=%s:%d",
                                address, me->dir_port);
    tor_free(address);
  }
}

/** We have enough testing circuits open. Send a bunch of "drop"
 * cells down each of them, to exercise our bandwidth. */
void
router_perform_bandwidth_test(int num_circs, time_t now)
{
  int num_cells = (int)(get_options()->BandwidthRate * 10 /
                        CELL_MAX_NETWORK_SIZE);
  int max_cells = num_cells < CIRCWINDOW_START ?
                    num_cells : CIRCWINDOW_START;
  int cells_per_circuit = max_cells / num_circs;
  origin_circuit_t *circ = NULL;

  log_notice(LD_OR,"Performing bandwidth self-test...done.");
  while ((circ = circuit_get_next_by_pk_and_purpose(circ, NULL,
                                              CIRCUIT_PURPOSE_TESTING))) {
    /* dump cells_per_circuit drop cells onto this circ */
    int i = cells_per_circuit;
    if (circ->base_.state != CIRCUIT_STATE_OPEN)
      continue;
    circ->base_.timestamp_dirty = now;
    while (i-- > 0) {
      if (relay_send_command_from_edge(0, TO_CIRCUIT(circ),
                                       RELAY_COMMAND_DROP,
                                       NULL, 0, circ->cpath->prev)<0) {
        return; /* stop if error */
      }
    }
  }
}
