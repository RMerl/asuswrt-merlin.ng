/* Copyright (c) 2018-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file torerr_sys.c
 * \brief Subsystem object for the error handling subsystem.
 **/

#include "orconfig.h"
#include "lib/err/backtrace.h"
#include "lib/err/torerr.h"
#include "lib/err/torerr_sys.h"
#include "lib/subsys/subsys.h"
#include "lib/version/torversion.h"

#include <stddef.h>

static int
subsys_torerr_initialize(void)
{
  if (configure_backtrace_handler(get_version()) < 0)
    return -1;
  tor_log_reset_sigsafe_err_fds();

  return 0;
}
static void
subsys_torerr_shutdown(void)
{
  /* Stop handling signals with backtraces, then close the logs. */
  clean_up_backtrace_handler();
  /* We can't log any log messages after this point: we've closed all the log
   * fds, including stdio. */
  tor_log_close_sigsafe_err_fds();
}

const subsys_fns_t sys_torerr = {
  .name = "err",
  /* Low-level error handling is a diagnostic feature, we want it to init
   * right after windows process security, and shutdown last.
   * (Security never shuts down.) */
  .level = -99,
  .supported = true,
  .initialize = subsys_torerr_initialize,
  .shutdown = subsys_torerr_shutdown
};
