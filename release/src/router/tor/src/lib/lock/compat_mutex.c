/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_mutex.c
 *
 * \brief Portable wrapper for platform mutex implementations.
 **/

#include "lib/lock/compat_mutex.h"
#include "lib/malloc/malloc.h"

/** Return a newly allocated, ready-for-use mutex. */
tor_mutex_t *
tor_mutex_new(void)
{
  tor_mutex_t *m = tor_malloc_zero(sizeof(tor_mutex_t));
  tor_mutex_init(m);
  return m;
}
/** Return a newly allocated, ready-for-use mutex.  This one might be
 * non-recursive, if that's faster. */
tor_mutex_t *
tor_mutex_new_nonrecursive(void)
{
  tor_mutex_t *m = tor_malloc_zero(sizeof(tor_mutex_t));
  tor_mutex_init_nonrecursive(m);
  return m;
}
/** Release all storage and system resources held by <b>m</b>.
 *
 * Destroying a locked mutex is undefined behaviour. Global mutexes may be
 * locked when they are passed to this function, because multiple threads can
 * still access them. So we can either:
 *  - destroy on shutdown, and re-initialise when tor re-initialises, or
 *  - skip destroying and re-initialisation, using a sentinel variable.
 * See #31735 for details.
 */
void
tor_mutex_free_(tor_mutex_t *m)
{
  if (!m)
    return;
  tor_mutex_uninit(m);
  tor_free(m);
}
