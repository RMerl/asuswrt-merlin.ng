/* Copyright (c) 2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file metrics_sys.c
 * @brief Setup and tear down the metrics subsystem.
 **/

#include "lib/subsys/subsys.h"

#include "feature/metrics/metrics.h"
#include "feature/metrics/metrics_sys.h"

static int
subsys_metrics_initialize(void)
{
  metrics_init();
  return 0;
}

static void
subsys_metrics_shutdown(void)
{
  metrics_cleanup();
}

const subsys_fns_t sys_metrics = {
  SUBSYS_DECLARE_LOCATION(),

  .name = "metrics",
  .supported = true,
  .level = METRICS_SUBSYS_LEVEL,

  .initialize = subsys_metrics_initialize,
  .shutdown = subsys_metrics_shutdown,
};

