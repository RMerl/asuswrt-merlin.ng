/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file approx_time.c
 * \brief Cache the last result of time(), for performance and testing.
 **/

#include "orconfig.h"
#include "lib/wallclock/approx_time.h"

/* =====
 * Cached time
 * ===== */

#ifndef TIME_IS_FAST
/** Cached estimate of the current time.  Updated around once per second;
 * may be a few seconds off if we are really busy.  This is a hack to avoid
 * calling time(NULL) (which not everybody has optimized) on critical paths.
 */
static time_t cached_approx_time = 0;

/** Return a cached estimate of the current time from when
 * update_approx_time() was last called.  This is a hack to avoid calling
 * time(NULL) on critical paths: please do not even think of calling it
 * anywhere else. */
time_t
approx_time(void)
{
  return cached_approx_time;
}

/** Update the cached estimate of the current time.  This function SHOULD be
 * called once per second, and MUST be called before the first call to
 * get_approx_time. */
void
update_approx_time(time_t now)
{
  cached_approx_time = now;
}
#endif /* !defined(TIME_IS_FAST) */
