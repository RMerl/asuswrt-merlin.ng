/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file hs_sys.c
 * @brief Setup and tear down the HS subsystem.
 **/

#include "lib/subsys/subsys.h"

#include "feature/hs/hs_metrics.h"
#include "feature/hs/hs_sys.h"

static int
subsys_hs_initialize(void)
{
  return 0;
}

static void
subsys_hs_shutdown(void)
{
}

const subsys_fns_t sys_hs = {
  SUBSYS_DECLARE_LOCATION(),

  .name = "hs",
  .supported = true,
  .level = HS_SUBSYS_LEVEL,

  .initialize = subsys_hs_initialize,
  .shutdown = subsys_hs_shutdown,

  .get_metrics = hs_metrics_get_stores,
};
