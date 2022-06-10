/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_mutex_pthreads.c
 *
 * \brief Implement the tor_mutex API using pthread_mutex_t.
 **/

#include "lib/lock/compat_mutex.h"
#include "lib/cc/compat_compiler.h"
#include "lib/err/torerr.h"

/** A mutex attribute that we're going to use to tell pthreads that we want
 * "recursive" mutexes (i.e., once we can re-lock if we're already holding
 * them.) */
static pthread_mutexattr_t attr_recursive;
/**
 * True iff <b>attr_recursive</b> has been initialized.
 **/
static int attr_initialized = 0;

/**
 * Initialize the locking module, if it is not already initialized.
 **/
void
tor_locking_init(void)
{
  if (!attr_initialized) {
    pthread_mutexattr_init(&attr_recursive);
    pthread_mutexattr_settype(&attr_recursive, PTHREAD_MUTEX_RECURSIVE);
    attr_initialized = 1;
  }
}

/** Initialize <b>mutex</b> so it can be locked.  Every mutex must be set
 * up with tor_mutex_init() or tor_mutex_new(); not both. */
void
tor_mutex_init(tor_mutex_t *mutex)
{
  if (PREDICT_UNLIKELY(!attr_initialized))
    tor_locking_init(); // LCOV_EXCL_LINE
  const int err = pthread_mutex_init(&mutex->mutex, &attr_recursive);
  if (PREDICT_UNLIKELY(err)) {
    // LCOV_EXCL_START
    raw_assert_unreached_msg("Error creating a mutex.");
    // LCOV_EXCL_STOP
  }
}

/** As tor_mutex_init, but initialize a mutex suitable that may be
 * non-recursive, if the OS supports that. */
void
tor_mutex_init_nonrecursive(tor_mutex_t *mutex)
{
  int err;
  if (!attr_initialized)
    tor_locking_init(); // LCOV_EXCL_LINE
  err = pthread_mutex_init(&mutex->mutex, NULL);
  if (PREDICT_UNLIKELY(err)) {
    // LCOV_EXCL_START
    raw_assert_unreached_msg("Error creating a mutex.");
    // LCOV_EXCL_STOP
  }
}

/** Wait until <b>m</b> is free, then acquire it. */
void
tor_mutex_acquire(tor_mutex_t *m)
{
  int err;
  raw_assert(m);
  err = pthread_mutex_lock(&m->mutex);
  if (PREDICT_UNLIKELY(err)) {
    // LCOV_EXCL_START
    raw_assert_unreached_msg("Error locking a mutex.");
    // LCOV_EXCL_STOP
  }
}
/** Release the lock <b>m</b> so another thread can have it. */
void
tor_mutex_release(tor_mutex_t *m)
{
  int err;
  raw_assert(m);
  err = pthread_mutex_unlock(&m->mutex);
  if (PREDICT_UNLIKELY(err)) {
    // LCOV_EXCL_START
    raw_assert_unreached_msg("Error unlocking a mutex.");
    // LCOV_EXCL_STOP
  }
}
/** Clean up the mutex <b>m</b> so that it no longer uses any system
 * resources.  Does not free <b>m</b>.  This function must only be called on
 * mutexes from tor_mutex_init().
 *
 * Destroying a locked mutex is undefined behaviour. Global mutexes may be
 * locked when they are passed to this function, because multiple threads can
 * still access them. So we can either:
 *  - destroy on shutdown, and re-initialise when tor re-initialises, or
 *  - skip destroying and re-initialisation, using a sentinel variable.
 * See #31735 for details.
 */
void
tor_mutex_uninit(tor_mutex_t *m)
{
  int err;
  raw_assert(m);
  /* If the mutex is already locked, wait until after it is unlocked to destroy
   * it. Locking and releasing the mutex makes undefined behaviour less likely,
   * but does not prevent it. Another thread can lock the mutex between release
   * and destroy. */
  tor_mutex_acquire(m);
  tor_mutex_release(m);
  err = pthread_mutex_destroy(&m->mutex);
  if (PREDICT_UNLIKELY(err)) {
    // LCOV_EXCL_START
    raw_assert_unreached_msg("Error destroying a mutex.");
    // LCOV_EXCL_STOP
  }
}
