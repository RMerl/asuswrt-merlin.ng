/* Emergency actions in case of a fatal signal.
   Copyright (C) 2003-2004, 2006-2024 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */


#include <config.h>

/* Specification.  */
#include "fatal-signal.h"

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "glthread/lock.h"
#include "thread-optim.h"
#include "sig-handler.h"

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))

/* ========================================================================= */


/* The list of fatal signals.
   These are those signals whose default action is to terminate the process
   without a core dump, except
     SIGKILL - because it cannot be caught,
     SIGALRM SIGUSR1 SIGUSR2 SIGPOLL SIGIO SIGLOST - because applications
       often use them for their own purpose,
     SIGPROF SIGVTALRM - because they are used for profiling,
     SIGSTKFLT - because it is more similar to SIGFPE, SIGSEGV, SIGBUS,
     SIGSYS - because it is more similar to SIGABRT, SIGSEGV,
     SIGPWR - because it of too special use,
     SIGRTMIN...SIGRTMAX - because they are reserved for application use.
   plus
     SIGXCPU, SIGXFSZ - because they are quite similar to SIGTERM.  */

static int fatal_signals[] =
  {
    /* ISO C 99 signals.  */
#ifdef SIGINT
    SIGINT,
#endif
#ifdef SIGTERM
    SIGTERM,
#endif
    /* POSIX:2001 signals.  */
#ifdef SIGHUP
    SIGHUP,
#endif
#ifdef SIGPIPE
    SIGPIPE,
#endif
    /* BSD signals.  */
#ifdef SIGXCPU
    SIGXCPU,
#endif
#ifdef SIGXFSZ
    SIGXFSZ,
#endif
    /* Native Windows signals.  */
#ifdef SIGBREAK
    SIGBREAK,
#endif
    0
  };

#define num_fatal_signals (SIZEOF (fatal_signals) - 1)

/* Eliminate signals whose signal handler is SIG_IGN.  */

static void
init_fatal_signals (void)
{
  /* This function is multithread-safe even without synchronization, because
     if two threads execute it simultaneously, the fatal_signals[] array will
     not change any more after the first of the threads has completed this
     function.  */
  static bool fatal_signals_initialized = false;
  if (!fatal_signals_initialized)
    {
      size_t i;

      for (i = 0; i < num_fatal_signals; i++)
        {
          struct sigaction action;

          if (sigaction (fatal_signals[i], NULL, &action) >= 0
              && get_handler (&action) == SIG_IGN)
            fatal_signals[i] = -1;
        }

      fatal_signals_initialized = true;
    }
}


/* ========================================================================= */


typedef _GL_ASYNC_SAFE void (*action_t) (int sig);

/* Type of an entry in the actions array.
   The 'action' field is accessed from within the fatal_signal_handler(),
   therefore we mark it as 'volatile'.  */
typedef struct
{
  volatile action_t action;
}
actions_entry_t;

/* The registered cleanup actions.  */
static actions_entry_t static_actions[32];
static actions_entry_t * volatile actions = static_actions;
static sig_atomic_t volatile actions_count = 0;
static size_t actions_allocated = SIZEOF (static_actions);


/* The saved signal handlers.
   Size 32 would not be sufficient: On HP-UX, SIGXCPU = 33, SIGXFSZ = 34.  */
static struct sigaction saved_sigactions[64];


/* Uninstall the handlers.  */
static _GL_ASYNC_SAFE void
uninstall_handlers (void)
{
  size_t i;

  for (i = 0; i < num_fatal_signals; i++)
    if (fatal_signals[i] >= 0)
      {
        int sig = fatal_signals[i];
        if (saved_sigactions[sig].sa_handler == SIG_IGN)
          saved_sigactions[sig].sa_handler = SIG_DFL;
        sigaction (sig, &saved_sigactions[sig], NULL);
      }
}


/* The signal handler.  It gets called asynchronously.  */
static _GL_ASYNC_SAFE void
fatal_signal_handler (int sig)
{
  for (;;)
    {
      /* Get the last registered cleanup action, in a reentrant way.  */
      action_t action;
      size_t n = actions_count;
      if (n == 0)
        break;
      n--;
      actions_count = n;
      action = actions[n].action;
      /* Execute the action.  */
      action (sig);
    }

  /* Now execute the signal's default action.
     If the signal being delivered was blocked, the re-raised signal would be
     delivered when this handler returns.  But the way we install this handler,
     no signal is blocked, and the re-raised signal is delivered already
     during raise().  */
  uninstall_handlers ();
  raise (sig);
}


/* Install the handlers.  */
static void
install_handlers (void)
{
  size_t i;
  struct sigaction action;

  action.sa_handler = &fatal_signal_handler;
  /* If we get a fatal signal while executing fatal_signal_handler, enter
     fatal_signal_handler recursively, since it is reentrant.  Hence no
     SA_RESETHAND.  */
  action.sa_flags = SA_NODEFER;
  sigemptyset (&action.sa_mask);
  for (i = 0; i < num_fatal_signals; i++)
    if (fatal_signals[i] >= 0)
      {
        int sig = fatal_signals[i];

        if (!(sig < sizeof (saved_sigactions) / sizeof (saved_sigactions[0])))
          abort ();
        sigaction (sig, &action, &saved_sigactions[sig]);
      }
}


/* Lock that makes at_fatal_signal multi-thread safe.  */
gl_lock_define_initialized (static, at_fatal_signal_lock)

/* Register a cleanup function to be executed when a catchable fatal signal
   occurs.  */
int
at_fatal_signal (action_t action)
{
  bool mt = gl_multithreaded ();

  if (mt) gl_lock_lock (at_fatal_signal_lock);

  static bool cleanup_initialized = false;
  if (!cleanup_initialized)
    {
      init_fatal_signals ();
      install_handlers ();
      cleanup_initialized = true;
    }

  int ret = 0;

  if (actions_count == actions_allocated)
    {
      /* Extend the actions array.  Note that we cannot use xrealloc(),
         because then the cleanup() function could access an already
         deallocated array.  */
      actions_entry_t *old_actions = actions;
      size_t old_actions_allocated = actions_allocated;
      size_t new_actions_allocated = 2 * actions_allocated;
      actions_entry_t *new_actions =
        (actions_entry_t *)
        malloc (new_actions_allocated * sizeof (actions_entry_t));
      if (new_actions == NULL)
        {
          ret = -1;
          goto done;
        }

      size_t k;
      /* Don't use memcpy() here, because memcpy takes non-volatile arguments
         and is therefore not guaranteed to complete all memory stores before
         the next statement.  */
      for (k = 0; k < old_actions_allocated; k++)
        new_actions[k] = old_actions[k];
      actions = new_actions;
      actions_allocated = new_actions_allocated;
      /* Now we can free the old actions array.  */
      /* No, we can't do that.  If fatal_signal_handler is running in a
         different thread and has already fetched the actions pointer (getting
         old_actions) but not yet accessed its n-th element, that thread may
         crash when accessing an element of the already freed old_actions
         array.  */
      #if 0
      if (old_actions != static_actions)
        free (old_actions);
      #endif
    }
  /* The two uses of 'volatile' in the types above (and ISO C 99 section
     5.1.2.3.(5)) ensure that we increment the actions_count only after
     the new action has been written to the memory location
     actions[actions_count].  */
  actions[actions_count].action = action;
  actions_count++;

 done:
  if (mt) gl_lock_unlock (at_fatal_signal_lock);

  return ret;
}


/* ========================================================================= */


static sigset_t fatal_signal_set;

static void
do_init_fatal_signal_set (void)
{
  size_t i;

  init_fatal_signals ();

  sigemptyset (&fatal_signal_set);
  for (i = 0; i < num_fatal_signals; i++)
    if (fatal_signals[i] >= 0)
      sigaddset (&fatal_signal_set, fatal_signals[i]);
}

/* Ensure that do_init_fatal_signal_set is called once only.  */
gl_once_define(static, fatal_signal_set_once)

static void
init_fatal_signal_set (void)
{
  gl_once (fatal_signal_set_once, do_init_fatal_signal_set);
}

/* Lock and counter that allow block_fatal_signals/unblock_fatal_signals pairs
   to occur in different threads and even overlap in time.  */
gl_lock_define_initialized (static, fatal_signals_block_lock)
static unsigned int fatal_signals_block_counter = 0;

/* Temporarily delay the catchable fatal signals.  */
void
block_fatal_signals (void)
{
  bool mt = gl_multithreaded ();

  if (mt) gl_lock_lock (fatal_signals_block_lock);

  if (fatal_signals_block_counter++ == 0)
    {
      init_fatal_signal_set ();
      sigprocmask (SIG_BLOCK, &fatal_signal_set, NULL);
    }

  if (mt) gl_lock_unlock (fatal_signals_block_lock);
}

/* Stop delaying the catchable fatal signals.  */
void
unblock_fatal_signals (void)
{
  bool mt = gl_multithreaded ();

  if (mt) gl_lock_lock (fatal_signals_block_lock);

  if (fatal_signals_block_counter == 0)
    /* There are more calls to unblock_fatal_signals() than to
       block_fatal_signals().  */
    abort ();
  if (--fatal_signals_block_counter == 0)
    {
      init_fatal_signal_set ();
      sigprocmask (SIG_UNBLOCK, &fatal_signal_set, NULL);
    }

  if (mt) gl_lock_unlock (fatal_signals_block_lock);
}


unsigned int
get_fatal_signals (int signals[64])
{
  init_fatal_signal_set ();

  {
    int *p = signals;
    size_t i;

    for (i = 0; i < num_fatal_signals; i++)
      if (fatal_signals[i] >= 0)
        *p++ = fatal_signals[i];
    return p - signals;
  }
}

const sigset_t *
get_fatal_signal_set (void)
{
  init_fatal_signal_set ();
  return &fatal_signal_set;
}
