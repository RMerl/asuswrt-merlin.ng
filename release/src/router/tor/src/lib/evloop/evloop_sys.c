/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file evloop_sys.c
 * @brief Subsystem definition for the event loop module
 **/

#include "orconfig.h"
#include "lib/subsys/subsys.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/evloop/evloop_sys.h"
#include "lib/log/log.h"

static int
subsys_evloop_initialize(void)
{
  if (tor_init_libevent_rng() < 0) {
    log_warn(LD_NET, "Problem initializing libevent RNG.");
    return -1;
  }
  return 0;
}

static void
subsys_evloop_postfork(void)
{
#ifdef TOR_UNIT_TESTS
  tor_libevent_postfork();
#endif
}

static void
subsys_evloop_shutdown(void)
{
  tor_libevent_free_all();
}

const struct subsys_fns_t sys_evloop = {
  .name = "evloop",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  .level = -20,
  .initialize = subsys_evloop_initialize,
  .shutdown = subsys_evloop_shutdown,
  .postfork = subsys_evloop_postfork,
};
