/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_threads.c
 *
 * \brief Cross-platform threading and inter-thread communication logic.
 *  (Platform-specific parts are written in the other compat_*threads
 *  modules.)
 */

#include "orconfig.h"
#include <stdlib.h>
#include "lib/thread/threads.h"
#include "lib/thread/thread_sys.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/subsys/subsys.h"

#include <string.h>

/** Allocate and return a new condition variable. */
tor_cond_t *
tor_cond_new(void)
{
  tor_cond_t *cond = tor_malloc(sizeof(tor_cond_t));
  if (BUG(tor_cond_init(cond)<0))
    tor_free(cond); // LCOV_EXCL_LINE
  return cond;
}

/** Free all storage held in <b>c</b>. */
void
tor_cond_free_(tor_cond_t *c)
{
  if (!c)
    return;
  tor_cond_uninit(c);
  tor_free(c);
}

/** Identity of the "main" thread */
static unsigned long main_thread_id = -1;

/** Start considering the current thread to be the 'main thread'.  This has
 * no effect on anything besides in_main_thread(). */
void
set_main_thread(void)
{
  main_thread_id = tor_get_thread_id();
}
/** Return true iff called from the main thread. */
int
in_main_thread(void)
{
  return main_thread_id == tor_get_thread_id();
}

#ifndef HAVE_WORKING_STDATOMIC
/** Initialize a new atomic counter with the value 0 */
void
atomic_counter_init(atomic_counter_t *counter)
{
  memset(counter, 0, sizeof(*counter));
  tor_mutex_init_nonrecursive(&counter->mutex);
}
/** Clean up all resources held by an atomic counter.
 *
 * Destroying a locked mutex is undefined behaviour. Global mutexes may be
 * locked when they are passed to this function, because multiple threads can
 * still access them. So we can either:
 *  - destroy on shutdown, and re-initialise when tor re-initialises, or
 *  - skip destroying and re-initialisation, using a sentinel variable.
 * See #31735 for details.
 */
void
atomic_counter_destroy(atomic_counter_t *counter)
{
  tor_mutex_uninit(&counter->mutex);
  memset(counter, 0, sizeof(*counter));
}
/** Add a value to an atomic counter. */
void
atomic_counter_add(atomic_counter_t *counter, size_t add)
{
  tor_mutex_acquire(&counter->mutex);
  counter->val += add;
  tor_mutex_release(&counter->mutex);
}
/** Subtract a value from an atomic counter. */
void
atomic_counter_sub(atomic_counter_t *counter, size_t sub)
{
  // this relies on unsigned overflow, but that's fine.
  atomic_counter_add(counter, -sub);
}
/** Return the current value of an atomic counter */
size_t
atomic_counter_get(atomic_counter_t *counter)
{
  size_t val;
  tor_mutex_acquire(&counter->mutex);
  val = counter->val;
  tor_mutex_release(&counter->mutex);
  return val;
}
/** Replace the value of an atomic counter; return the old one. */
size_t
atomic_counter_exchange(atomic_counter_t *counter, size_t newval)
{
  size_t oldval;
  tor_mutex_acquire(&counter->mutex);
  oldval = counter->val;
  counter->val = newval;
  tor_mutex_release(&counter->mutex);
  return oldval;
}
#endif /* !defined(HAVE_WORKING_STDATOMIC) */

static int
subsys_threads_initialize(void)
{
  tor_threads_init();
  return 0;
}

const subsys_fns_t sys_threads = {
  .name = "threads",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  .level = -89,
  .initialize = subsys_threads_initialize,
};
