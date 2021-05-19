/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file time_sys.c
 * \brief Subsystem object for monotime setup.
 **/

#include "orconfig.h"
#include "lib/subsys/subsys.h"
#include "lib/time/time_sys.h"
#include "lib/time/compat_time.h"

static int
subsys_time_initialize(void)
{
  monotime_init();
  return 0;
}

const subsys_fns_t sys_time = {
  .name = "time",
  SUBSYS_DECLARE_LOCATION(),
  /* Monotonic time depends on logging, and a lot of other modules depend on
   * monotonic time. */
  .level = -80,
  .supported = true,
  .initialize = subsys_time_initialize,
};
