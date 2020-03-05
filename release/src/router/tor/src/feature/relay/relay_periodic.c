/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_periodic.c
 * @brief Periodic functions for the relay subsytem
 **/

#include "orconfig.h"
#include "core/or/or.h"

#include "core/mainloop/periodic.h"
#include "core/mainloop/cpuworker.h" // XXXX use a pubsub event.
#include "core/mainloop/mainloop.h"
#include "core/mainloop/netstatus.h"
#include "core/or/circuituse.h" // XXXX move have_performed_bandwidth_test

#include "feature/relay/dns.h"
#include "feature/relay/relay_periodic.h"
#include "feature/relay/router.h"
#include "feature/relay/routerkeys.h"
#include "feature/relay/routermode.h"
#include "feature/relay/selftest.h"
#include "feature/stats/predict_ports.h"

#include "lib/crypt_ops/crypto_rand.h"

#include "feature/nodelist/routerinfo_st.h"
#include "feature/control/control_events.h"

#define DECLARE_EVENT(name, roles, flags)         \
  static periodic_event_item_t name ## _event =   \
    PERIODIC_EVENT(name,                          \
                   PERIODIC_EVENT_ROLE_##roles,   \
                   flags)

#define FL(name) (PERIODIC_EVENT_FLAG_##name)

/**
 * Periodic callback: If we're a server and initializing dns failed, retry.
 */
static int
retry_dns_callback(time_t now, const or_options_t *options)
{
  (void)now;
#define RETRY_DNS_INTERVAL (10*60)
  if (server_mode(options) && has_dns_init_failed())
    dns_init();
  return RETRY_DNS_INTERVAL;
}

DECLARE_EVENT(retry_dns, ROUTER, 0);

static int dns_honesty_first_time = 1;

/**
 * Periodic event: if we're an exit, see if our DNS server is telling us
 * obvious lies.
 */
static int
check_dns_honesty_callback(time_t now, const or_options_t *options)
{
  (void)now;
  /* 9. and if we're an exit node, check whether our DNS is telling stories
   * to us. */
  if (net_is_disabled() ||
      ! public_server_mode(options) ||
      router_my_exit_policy_is_reject_star())
    return PERIODIC_EVENT_NO_UPDATE;

  if (dns_honesty_first_time) {
    /* Don't launch right when we start */
    dns_honesty_first_time = 0;
    return crypto_rand_int_range(60, 180);
  }

  dns_launch_correctness_checks();
  return 12*3600 + crypto_rand_int(12*3600);
}

DECLARE_EVENT(check_dns_honesty, RELAY, FL(NEED_NET));

/* Periodic callback: rotate the onion keys after the period defined by the
 * "onion-key-rotation-days" consensus parameter, shut down and restart all
 * cpuworkers, and update our descriptor if necessary.
 */
static int
rotate_onion_key_callback(time_t now, const or_options_t *options)
{
  if (server_mode(options)) {
    int onion_key_lifetime = get_onion_key_lifetime();
    time_t rotation_time = get_onion_key_set_at()+onion_key_lifetime;
    if (rotation_time > now) {
      return ONION_KEY_CONSENSUS_CHECK_INTERVAL;
    }

    log_info(LD_GENERAL,"Rotating onion key.");
    rotate_onion_key();
    cpuworkers_rotate_keyinfo();
    if (router_rebuild_descriptor(1)<0) {
      log_info(LD_CONFIG, "Couldn't rebuild router descriptor");
    }
    if (advertised_server_mode() && !net_is_disabled())
      router_upload_dir_desc_to_dirservers(0);
    return ONION_KEY_CONSENSUS_CHECK_INTERVAL;
  }
  return PERIODIC_EVENT_NO_UPDATE;
}

DECLARE_EVENT(rotate_onion_key, ROUTER, 0);

/** Periodic callback: consider rebuilding or and re-uploading our descriptor
 * (if we've passed our internal checks). */
static int
check_descriptor_callback(time_t now, const or_options_t *options)
{
/** How often do we check whether part of our router info has changed in a
 * way that would require an upload? That includes checking whether our IP
 * address has changed. */
#define CHECK_DESCRIPTOR_INTERVAL (60)

  (void)options;

  /* 2b. Once per minute, regenerate and upload the descriptor if the old
   * one is inaccurate. */
  if (!net_is_disabled()) {
    check_descriptor_bandwidth_changed(now);
    check_descriptor_ipaddress_changed(now);
    mark_my_descriptor_dirty_if_too_old(now);
    consider_publishable_server(0);
  }

  return CHECK_DESCRIPTOR_INTERVAL;
}

DECLARE_EVENT(check_descriptor, ROUTER, FL(NEED_NET));

static int dirport_reachability_count = 0;

/**
 * Periodic callback: check whether we're reachable (as a relay), and
 * whether our bandwidth has changed enough that we need to
 * publish a new descriptor.
 */
static int
check_for_reachability_bw_callback(time_t now, const or_options_t *options)
{
  /* XXXX This whole thing was stuck in the middle of what is now
   * XXXX check_descriptor_callback.  I'm not sure it's right. */

  /* also, check religiously for reachability, if it's within the first
   * 20 minutes of our uptime. */
  if (server_mode(options) &&
      (have_completed_a_circuit() || !any_predicted_circuits(now)) &&
      !net_is_disabled()) {
    if (get_uptime() < TIMEOUT_UNTIL_UNREACHABILITY_COMPLAINT) {
      router_do_reachability_checks(1, dirport_reachability_count==0);
      if (++dirport_reachability_count > 5)
        dirport_reachability_count = 0;
      return 1;
    } else {
      /* If we haven't checked for 12 hours and our bandwidth estimate is
       * low, do another bandwidth test. This is especially important for
       * bridges, since they might go long periods without much use. */
      const routerinfo_t *me = router_get_my_routerinfo();
      static int first_time = 1;
      if (!first_time && me &&
          me->bandwidthcapacity < me->bandwidthrate &&
          me->bandwidthcapacity < 51200) {
        reset_bandwidth_test();
      }
      first_time = 0;
#define BANDWIDTH_RECHECK_INTERVAL (12*60*60)
      return BANDWIDTH_RECHECK_INTERVAL;
    }
  }
  return CHECK_DESCRIPTOR_INTERVAL;
}

DECLARE_EVENT(check_for_reachability_bw, ROUTER, FL(NEED_NET));

/**
 * Callback: Send warnings if Tor doesn't find its ports reachable.
 */
static int
reachability_warnings_callback(time_t now, const or_options_t *options)
{
  (void) now;

  if (get_uptime() < TIMEOUT_UNTIL_UNREACHABILITY_COMPLAINT) {
    return (int)(TIMEOUT_UNTIL_UNREACHABILITY_COMPLAINT - get_uptime());
  }

  if (server_mode(options) &&
      !net_is_disabled() &&
      have_completed_a_circuit()) {
    /* every 20 minutes, check and complain if necessary */
    const routerinfo_t *me = router_get_my_routerinfo();
    if (me && !check_whether_orport_reachable(options)) {
      char *address = tor_dup_ip(me->addr);
      log_warn(LD_CONFIG,"Your server (%s:%d) has not managed to confirm that "
               "its ORPort is reachable. Relays do not publish descriptors "
               "until their ORPort and DirPort are reachable. Please check "
               "your firewalls, ports, address, /etc/hosts file, etc.",
               address, me->or_port);
      control_event_server_status(LOG_WARN,
                                  "REACHABILITY_FAILED ORADDRESS=%s:%d",
                                  address, me->or_port);
      tor_free(address);
    }

    if (me && !check_whether_dirport_reachable(options)) {
      char *address = tor_dup_ip(me->addr);
      log_warn(LD_CONFIG,
               "Your server (%s:%d) has not managed to confirm that its "
               "DirPort is reachable. Relays do not publish descriptors "
               "until their ORPort and DirPort are reachable. Please check "
               "your firewalls, ports, address, /etc/hosts file, etc.",
               address, me->dir_port);
      control_event_server_status(LOG_WARN,
                                  "REACHABILITY_FAILED DIRADDRESS=%s:%d",
                                  address, me->dir_port);
      tor_free(address);
    }
  }

  return TIMEOUT_UNTIL_UNREACHABILITY_COMPLAINT;
}

DECLARE_EVENT(reachability_warnings, ROUTER, FL(NEED_NET));

/* Periodic callback: Every 30 seconds, check whether it's time to make new
 * Ed25519 subkeys.
 */
static int
check_ed_keys_callback(time_t now, const or_options_t *options)
{
  if (server_mode(options)) {
    if (should_make_new_ed_keys(options, now)) {
      int new_signing_key = load_ed_keys(options, now);
      if (new_signing_key < 0 ||
          generate_ed_link_cert(options, now, new_signing_key > 0)) {
        log_err(LD_OR, "Unable to update Ed25519 keys!  Exiting.");
        tor_shutdown_event_loop_and_exit(1);
      }
    }
    return 30;
  }
  return PERIODIC_EVENT_NO_UPDATE;
}

DECLARE_EVENT(check_ed_keys, ROUTER, 0);

/* Period callback: Check if our old onion keys are still valid after the
 * period of time defined by the consensus parameter
 * "onion-key-grace-period-days", otherwise expire them by setting them to
 * NULL.
 */
static int
check_onion_keys_expiry_time_callback(time_t now, const or_options_t *options)
{
  if (server_mode(options)) {
    int onion_key_grace_period = get_onion_key_grace_period();
    time_t expiry_time = get_onion_key_set_at()+onion_key_grace_period;
    if (expiry_time > now) {
      return ONION_KEY_CONSENSUS_CHECK_INTERVAL;
    }

    log_info(LD_GENERAL, "Expiring old onion keys.");
    expire_old_onion_keys();
    cpuworkers_rotate_keyinfo();
    return ONION_KEY_CONSENSUS_CHECK_INTERVAL;
  }

  return PERIODIC_EVENT_NO_UPDATE;
}

DECLARE_EVENT(check_onion_keys_expiry_time, ROUTER, 0);

void
relay_register_periodic_events(void)
{
  periodic_events_register(&retry_dns_event);
  periodic_events_register(&check_dns_honesty_event);
  periodic_events_register(&rotate_onion_key_event);
  periodic_events_register(&check_descriptor_event);
  periodic_events_register(&check_for_reachability_bw_event);
  periodic_events_register(&reachability_warnings_event);
  periodic_events_register(&check_ed_keys_event);
  periodic_events_register(&check_onion_keys_expiry_time_event);

  dns_honesty_first_time = 1;
  dirport_reachability_count = 0;
}

/**
 * Update our schedule so that we'll check whether we need to update our
 * descriptor immediately, rather than after up to CHECK_DESCRIPTOR_INTERVAL
 * seconds.
 */
void
reschedule_descriptor_update_check(void)
{
  periodic_event_reschedule(&check_descriptor_event);
}
