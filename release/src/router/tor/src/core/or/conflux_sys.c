/* Copyright (c) 2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file conflux_sys.c
 * \brief Register the conflux pool for early initialization.
 **/

#include "core/or/conflux_pool.h"
#include "core/or/conflux_sys.h"

#include "lib/subsys/subsys.h"

static int
subsys_conflux_initialize(void)
{
  conflux_pool_init();
  return 0;
}

static void
subsys_conflux_shutdown(void)
{
  /* The conflux pool free all must be called before the circuit free all and
   * so we are not calling it from subsys shutdown. */
}

const subsys_fns_t sys_conflux = {
  SUBSYS_DECLARE_LOCATION(),

  .name = "conflux",
  .supported = true,
  .level = CONFLUX_SUBSYS_LEVEL,

  .initialize = subsys_conflux_initialize,
  .shutdown = subsys_conflux_shutdown,
};
