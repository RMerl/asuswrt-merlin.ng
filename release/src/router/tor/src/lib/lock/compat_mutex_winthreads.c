/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_mutex_winthreads.c
 *
 * \brief Implement the tor_mutex API using CRITICAL_SECTION.
 **/

#include "orconfig.h"

/* For SRW locks support */
#ifndef WINVER
#error "orconfig.h didn't define WINVER"
#endif
#ifndef _WIN32_WINNT
#error "orconfig.h didn't define _WIN32_WINNT"
#endif
#if WINVER < 0x0600
#error "winver too low"
#endif
#if _WIN32_WINNT < 0x0600
#error "winver too low"
#endif

#include <windows.h>
#include "lib/lock/compat_mutex.h"
#include "lib/err/torerr.h"

void
tor_locking_init(void)
{
}

void
tor_mutex_init(tor_mutex_t *m)
{
  m->type = RECURSIVE;
  m->lock_owner = 0;
  m->lock_count = 0;
  InitializeSRWLock(&m->mutex);
}
void
tor_mutex_init_nonrecursive(tor_mutex_t *m)
{
  m->type = NON_RECURSIVE;
  InitializeSRWLock(&m->mutex);
}

void
tor_mutex_uninit(tor_mutex_t *m)
{
  (void) m;
}

static void
tor_mutex_acquire_recursive(tor_mutex_t *m)
{
  LONG thread_id = GetCurrentThreadId();
  // use InterlockedCompareExchange to perform an atomic read
  LONG lock_owner = InterlockedCompareExchange(&m->lock_owner, 0, 0);
  if (thread_id == lock_owner) {
    ++m->lock_count;
    return;
  }
  AcquireSRWLockExclusive(&m->mutex);
  InterlockedExchange(&m->lock_owner, thread_id);
  m->lock_count = 1;
}

static void
tor_mutex_acquire_nonrecursive(tor_mutex_t *m)
{
  AcquireSRWLockExclusive(&m->mutex);
}

void
tor_mutex_acquire(tor_mutex_t *m)
{
  raw_assert(m);
  if (m->type == NON_RECURSIVE) {
    tor_mutex_acquire_nonrecursive(m);
  } else {
    tor_mutex_acquire_recursive(m);
  }
}

static void
tor_mutex_release_recursive(tor_mutex_t *m)
{
  if (--m->lock_count) {
    return;
  }
  InterlockedExchange(&m->lock_owner, 0);
  ReleaseSRWLockExclusive(&m->mutex);
}

static void
tor_mutex_release_nonrecursive(tor_mutex_t *m)
{
  ReleaseSRWLockExclusive(&m->mutex);
}

void
tor_mutex_release(tor_mutex_t *m)
{
  if (m->type == NON_RECURSIVE) {
    tor_mutex_release_nonrecursive(m);
  } else {
    tor_mutex_release_recursive(m);
  }
}
