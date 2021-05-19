/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_mutex_winthreads.c
 *
 * \brief Implement the tor_mutex API using CRITICAL_SECTION.
 **/

#include "lib/lock/compat_mutex.h"
#include "lib/err/torerr.h"

void
tor_locking_init(void)
{
}

void
tor_mutex_init(tor_mutex_t *m)
{
  InitializeCriticalSection(&m->mutex);
}
void
tor_mutex_init_nonrecursive(tor_mutex_t *m)
{
  InitializeCriticalSection(&m->mutex);
}

void
tor_mutex_uninit(tor_mutex_t *m)
{
  DeleteCriticalSection(&m->mutex);
}
void
tor_mutex_acquire(tor_mutex_t *m)
{
  raw_assert(m);
  EnterCriticalSection(&m->mutex);
}
void
tor_mutex_release(tor_mutex_t *m)
{
  LeaveCriticalSection(&m->mutex);
}
