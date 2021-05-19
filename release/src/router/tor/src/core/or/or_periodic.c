/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file or_periodic.c
 * @brief Periodic callbacks for the onion routing subsystem
 **/

#include "orconfig.h"
#include "core/or/or.h"

#include "core/mainloop/periodic.h"

#include "core/or/channel.h"
#include "core/or/circuituse.h"
#include "core/or/or_periodic.h"

#include "feature/relay/routermode.h"

#ifndef COCCI
#define DECLARE_EVENT(name, roles, flags)         \
  static periodic_event_item_t name ## _event =   \
    PERIODIC_EVENT(name,                          \
                   PERIODIC_EVENT_ROLE_##roles,   \
                   flags)
#endif /* !defined(COCCI) */

#define FL(name) (PERIODIC_EVENT_FLAG_ ## name)

#define CHANNEL_CHECK_INTERVAL (60*60)
static int
check_canonical_channels_callback(time_t now, const or_options_t *options)
{
  (void)now;
  if (public_server_mode(options))
    channel_check_for_duplicates();

  return CHANNEL_CHECK_INTERVAL;
}

DECLARE_EVENT(check_canonical_channels, RELAY, FL(NEED_NET));

/**
 * Periodic callback: as a server, see if we have any old unused circuits
 * that should be expired */
static int
expire_old_circuits_serverside_callback(time_t now,
                                        const or_options_t *options)
{
  (void)options;
  /* every 11 seconds, so not usually the same second as other such events */
  circuit_expire_old_circuits_serverside(now);
  return 11;
}

DECLARE_EVENT(expire_old_circuits_serverside, ROUTER, FL(NEED_NET));

void
or_register_periodic_events(void)
{
  // These are router-only events, but they're owned by the OR subsystem. */
  periodic_events_register(&check_canonical_channels_event);
  periodic_events_register(&expire_old_circuits_serverside_event);
}
