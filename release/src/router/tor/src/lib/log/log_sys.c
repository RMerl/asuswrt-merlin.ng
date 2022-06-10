/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file log_sys.c
 * \brief Setup and tear down the logging module.
 **/

#include "orconfig.h"
#include "lib/subsys/subsys.h"
#include "lib/log/escape.h"
#include "lib/log/log.h"
#include "lib/log/log_sys.h"

static int
subsys_logging_initialize(void)
{
  init_logging(0);
  return 0;
}

static void
subsys_logging_shutdown(void)
{
  logs_free_all();
  escaped(NULL);
}

const subsys_fns_t sys_logging = {
  .name = "log",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  /* Logging depends on threads, approx time, raw logging, and security.
   * Most other lib modules depend on logging. */
  .level = -90,
  .initialize = subsys_logging_initialize,
  .shutdown = subsys_logging_shutdown,
};
