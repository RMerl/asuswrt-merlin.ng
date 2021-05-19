/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file process_sys.c
 * \brief Subsystem object for process setup.
 **/

#include "orconfig.h"
#include "lib/subsys/subsys.h"
#include "lib/process/process_sys.h"
#include "lib/process/process.h"

static int
subsys_process_initialize(void)
{
  process_init();
  return 0;
}

static void
subsys_process_shutdown(void)
{
  process_free_all();
}

const subsys_fns_t sys_process = {
  .name = "process",
  SUBSYS_DECLARE_LOCATION(),
  .level = -18,
  .supported = true,
  .initialize = subsys_process_initialize,
  .shutdown = subsys_process_shutdown
};
