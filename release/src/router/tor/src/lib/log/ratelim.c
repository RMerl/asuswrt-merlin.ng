/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file ratelim.c
 * \brief Summarize similar messages that would otherwise flood the logs.
 **/

#include "lib/log/ratelim.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"
#include "lib/intmath/muldiv.h"

/** If the rate-limiter <b>lim</b> is ready at <b>now</b>, return the number
 * of calls to rate_limit_is_ready (including this one!) since the last time
 * rate_limit_is_ready returned nonzero.  Otherwise return 0.
 * If the call number hits <b>RATELIM_TOOMANY</b> limit, drop a warning
 * about this event and stop counting. */
static int
rate_limit_is_ready(ratelim_t *lim, time_t now)
{
  if (lim->rate + lim->last_allowed <= now) {
    int res = lim->n_calls_since_last_time + 1;
    lim->last_allowed = now;
    lim->n_calls_since_last_time = 0;
    return res;
  } else {
    if (lim->n_calls_since_last_time <= RATELIM_TOOMANY) {
      ++lim->n_calls_since_last_time;
    }

    return 0;
  }
}

/** If the rate-limiter <b>lim</b> is ready at <b>now</b>, return a newly
 * allocated string indicating how many messages were suppressed, suitable to
 * append to a log message.  Otherwise return NULL. */
char *
rate_limit_log(ratelim_t *lim, time_t now)
{
  int n;
  if ((n = rate_limit_is_ready(lim, now))) {
    time_t started_limiting = lim->started_limiting;
    lim->started_limiting = 0;
    if (n == 1) {
      return tor_strdup("");
    } else {
      char *cp=NULL;
      const char *opt_over = (n >= RATELIM_TOOMANY) ? "over " : "";
      unsigned difference = (unsigned)(now - started_limiting);
      difference = round_to_next_multiple_of(difference, 60);
      tor_asprintf(&cp,
                   " [%s%d similar message(s) suppressed in last %d seconds]",
                   opt_over, n-1, (int)difference);
      return cp;
    }
  } else {
    if (lim->started_limiting == 0) {
      lim->started_limiting = now;
    }
    return NULL;
  }
}
