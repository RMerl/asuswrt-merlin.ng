/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_winthreads.c
 *
 * \brief Implementation for the windows-based multithreading backend
 * functions.
 */

#include "orconfig.h"

#ifdef _WIN32
/* For condition variable support */
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
#include <process.h>
#include <time.h>

#include "lib/thread/threads.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/log/win32err.h"

/** Minimalist interface to run a void function in the background.  On
 * Unix calls fork, on win32 calls beginthread.  Returns -1 on failure.
 * func should not return, but rather should call spawn_exit.
 *
 * NOTE: if <b>data</b> is used, it should not be allocated on the stack,
 * since in a multithreaded environment, there is no way to be sure that
 * the caller's stack will still be around when the called function is
 * running.
 */
int
spawn_func(void (*func)(void *), void *data)
{
  int rv;
  rv = (int)_beginthread(func, 0, data);
  if (rv == (int)-1)
    return -1;
  return 0;
}

/** End the current thread/process.
 */
void
spawn_exit(void)
{
  _endthread();
  // LCOV_EXCL_START
  //we should never get here. my compiler thinks that _endthread returns, this
  //is an attempt to fool it.
  tor_assert(0);
  _exit(0); // exit ok: unreachable.
  // LCOV_EXCL_STOP
}

unsigned long
tor_get_thread_id(void)
{
  return (unsigned long)GetCurrentThreadId();
}

int
tor_cond_init(tor_cond_t *cond)
{
  InitializeConditionVariable(&cond->cond);
  return 0;
}
void
tor_cond_uninit(tor_cond_t *cond)
{
  (void) cond;
}

void
tor_cond_signal_one(tor_cond_t *cond)
{
  WakeConditionVariable(&cond->cond);
}
void
tor_cond_signal_all(tor_cond_t *cond)
{
  WakeAllConditionVariable(&cond->cond);
}

int
tor_threadlocal_init(tor_threadlocal_t *threadlocal)
{
  threadlocal->index = TlsAlloc();
  return (threadlocal->index == TLS_OUT_OF_INDEXES) ? -1 : 0;
}

void
tor_threadlocal_destroy(tor_threadlocal_t *threadlocal)
{
  TlsFree(threadlocal->index);
  memset(threadlocal, 0, sizeof(tor_threadlocal_t));
}

void *
tor_threadlocal_get(tor_threadlocal_t *threadlocal)
{
  void *value = TlsGetValue(threadlocal->index);
  if (value == NULL) {
    DWORD err = GetLastError();
    if (err != ERROR_SUCCESS) {
      char *msg = format_win32_error(err);
      log_err(LD_GENERAL, "Error retrieving thread-local value: %s", msg);
      tor_free(msg);
      tor_assert(err == ERROR_SUCCESS);
    }
  }
  return value;
}

void
tor_threadlocal_set(tor_threadlocal_t *threadlocal, void *value)
{
  BOOL ok = TlsSetValue(threadlocal->index, value);
  if (!ok) {
    DWORD err = GetLastError();
    char *msg = format_win32_error(err);
    log_err(LD_GENERAL, "Error adjusting thread-local value: %s", msg);
    tor_free(msg);
    tor_assert(ok);
  }
}

int
tor_cond_wait(tor_cond_t *cond, tor_mutex_t *lock_, const struct timeval *tv)
{
  CRITICAL_SECTION *lock = &lock_->mutex;
  DWORD ms = INFINITE;
  if (tv) {
    ms = tv->tv_sec*1000 + (tv->tv_usec+999)/1000;
  }

  BOOL ok = SleepConditionVariableCS(&cond->cond, lock, ms);
  if (!ok) {
    DWORD err = GetLastError();
    if (err == ERROR_TIMEOUT) {
      return 1;
    }
    char *msg = format_win32_error(err);
    log_err(LD_GENERAL, "Error waiting for condition variable: %s", msg);
    tor_free(msg);
    return -1;
  }
  return 0;
}

void
tor_threads_init(void)
{
  set_main_thread();
}

#endif /* defined(_WIN32) */
