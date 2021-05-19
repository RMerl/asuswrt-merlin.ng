/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dirauth_periodic.c
 * @brief Peridoic events for directory authorities.
 **/

#include "core/or/or.h"

#include "app/config/or_options_st.h"
#include "core/mainloop/netstatus.h"
#include "feature/dirauth/reachability.h"
#include "feature/stats/rephist.h"

#include "feature/dirauth/bridgeauth.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirauth/dirauth_periodic.h"
#include "feature/dirauth/authmode.h"

#include "core/mainloop/periodic.h"

#ifndef COCCI
#define DECLARE_EVENT(name, roles, flags)         \
  static periodic_event_item_t name ## _event =   \
    PERIODIC_EVENT(name,                          \
                   PERIODIC_EVENT_ROLE_##roles,   \
                   flags)
#endif /* !defined(COCCI) */

#define FL(name) (PERIODIC_EVENT_FLAG_##name)

/**
 * Periodic callback: if we're an authority, check on our authority
 * certificate (the one that authenticates our authority signing key).
 */
static int
check_authority_cert_callback(time_t now, const or_options_t *options)
{
  (void)now;
  (void)options;
  /* 1e. Periodically, if we're a v3 authority, we check whether our cert is
   * close to expiring and warn the admin if it is. */
  v3_authority_check_key_expiry();
#define CHECK_V3_CERTIFICATE_INTERVAL (5*60)
  return CHECK_V3_CERTIFICATE_INTERVAL;
}

DECLARE_EVENT(check_authority_cert, DIRAUTH, 0);

/**
 * Scheduled callback: Run directory-authority voting functionality.
 *
 * The schedule is a bit complicated here, so dirvote_act() manages the
 * schedule itself.
 **/
static int
dirvote_callback(time_t now, const or_options_t *options)
{
  if (!authdir_mode_v3(options)) {
    tor_assert_nonfatal_unreached();
    return 3600;
  }

  time_t next = dirvote_act(options, now);
  if (BUG(next == TIME_MAX)) {
    /* This shouldn't be returned unless we called dirvote_act() without
     * being an authority.  If it happens, maybe our configuration will
     * fix itself in an hour or so? */
    return 3600;
  }
  return safe_timer_diff(now, next);
}

DECLARE_EVENT(dirvote, DIRAUTH, FL(NEED_NET));

/** Reschedule the directory-authority voting event.  Run this whenever the
 * schedule has changed. */
void
reschedule_dirvote(const or_options_t *options)
{
  if (authdir_mode_v3(options)) {
    periodic_event_reschedule(&dirvote_event);
  }
}

/**
 * Periodic callback: if we're an authority, record our measured stability
 * information from rephist in an mtbf file.
 */
static int
save_stability_callback(time_t now, const or_options_t *options)
{
  if (authdir_mode_tests_reachability(options)) {
    if (rep_hist_record_mtbf_data(now, 1)<0) {
      log_warn(LD_GENERAL, "Couldn't store mtbf data.");
    }
  }
#define SAVE_STABILITY_INTERVAL (30*60)
  return SAVE_STABILITY_INTERVAL;
}

DECLARE_EVENT(save_stability, AUTHORITIES, 0);

/**
 * Periodic callback: if we're an authority, make sure we test
 * the routers on the network for reachability.
 */
static int
launch_reachability_tests_callback(time_t now, const or_options_t *options)
{
  if (authdir_mode_tests_reachability(options) &&
      !net_is_disabled()) {
    /* try to determine reachability of the other Tor relays */
    dirserv_test_reachability(now);
  }
  return REACHABILITY_TEST_INTERVAL;
}

DECLARE_EVENT(launch_reachability_tests, AUTHORITIES, FL(NEED_NET));

/**
 * Periodic callback: if we're an authority, discount the stability
 * information (and other rephist information) that's older.
 */
static int
downrate_stability_callback(time_t now, const or_options_t *options)
{
  (void)options;
  /* 1d. Periodically, we discount older stability information so that new
   * stability info counts more, and save the stability information to disk as
   * appropriate. */
  time_t next = rep_hist_downrate_old_runs(now);
  return safe_timer_diff(now, next);
}

DECLARE_EVENT(downrate_stability, AUTHORITIES, 0);

/**
 * Periodic callback: if we're the bridge authority, write a networkstatus
 * file to disk.
 */
static int
write_bridge_ns_callback(time_t now, const or_options_t *options)
{
  if (options->BridgeAuthoritativeDir) {
    bridgeauth_dump_bridge_status_to_file(now);
#define BRIDGE_STATUSFILE_INTERVAL (30*60)
     return BRIDGE_STATUSFILE_INTERVAL;
  }
  return PERIODIC_EVENT_NO_UPDATE;
}

DECLARE_EVENT(write_bridge_ns, BRIDGEAUTH, 0);

void
dirauth_register_periodic_events(void)
{
  periodic_events_register(&downrate_stability_event);
  periodic_events_register(&launch_reachability_tests_event);
  periodic_events_register(&save_stability_event);
  periodic_events_register(&check_authority_cert_event);
  periodic_events_register(&dirvote_event);
  periodic_events_register(&write_bridge_ns_event);
}
