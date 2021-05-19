/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_sys.c
 * @brief Subsystem definitions for the relay module.
 **/

#include "orconfig.h"
#include "core/or/or.h"

#include "feature/relay/dns.h"
#include "feature/relay/ext_orport.h"
#include "feature/relay/onion_queue.h"
#include "feature/relay/relay_periodic.h"
#include "feature/relay/relay_sys.h"
#include "feature/relay/routerkeys.h"
#include "feature/relay/router.h"

#include "lib/subsys/subsys.h"

static int
subsys_relay_initialize(void)
{
  relay_register_periodic_events();
  return 0;
}

static void
subsys_relay_shutdown(void)
{
  dns_free_all();
  ext_orport_free_all();
  clear_pending_onions();
  routerkeys_free_all();
  router_free_all();
}

const struct subsys_fns_t sys_relay = {
  .name = "relay",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  .level = RELAY_SUBSYS_LEVEL,
  .initialize = subsys_relay_initialize,
  .shutdown = subsys_relay_shutdown,
};
