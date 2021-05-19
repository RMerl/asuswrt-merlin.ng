/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file quiet_level.c
 * @brief Code to handle default logging level (quiet/hush/normal).
 **/

#include "orconfig.h"
#include "lib/log/log.h"
#include "app/config/quiet_level.h"

/** Decides our behavior when no logs are configured/before any logs have been
 * configured.  For QUIET_NONE, we log notice to stdout as normal.  For
 * QUIET_HUSH, we log warnings only.  For QUIET_SILENT, we log nothing.
 */
quiet_level_t quiet_level = 0;

/** Add a default log (or not), depending on the value of <b>quiet</b>. */
void
add_default_log_for_quiet_level(quiet_level_t quiet)
{
  switch (quiet) {
    case QUIET_SILENT:
      /* --quiet: no initial logging */
      return;
    case QUIET_HUSH:
    /* --hush: log at warning or higher. */
      add_default_log(LOG_WARN);
      break;
    case QUIET_NONE: FALLTHROUGH;
    default:
      add_default_log(LOG_NOTICE);
  }
}
