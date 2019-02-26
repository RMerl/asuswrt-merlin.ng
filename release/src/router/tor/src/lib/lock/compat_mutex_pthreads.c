/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
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
static int attr_initialized = 0;

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
 * mutexes from tor_mutex_init(). */
void
tor_mutex_uninit(tor_mutex_t *m)
{
  int err;
  raw_assert(m);
  err = pthread_mutex_destroy(&m->mutex);
  if (PREDICT_UNLIKELY(err)) {
    // LCOV_EXCL_START
    raw_assert_unreached_msg("Error destroying a mutex.");
    // LCOV_EXCL_STOP
  }
}
