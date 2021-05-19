/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file daemon.c
 * \brief Run the tor process in the background (unix only)
 **/

#include "orconfig.h"
#include "lib/process/daemon.h"

#ifndef _WIN32

#include "lib/fs/files.h"
#include "lib/log/log.h"
#include "lib/thread/threads.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Based on code contributed by christian grothoff */
/** True iff we've called start_daemon(). */
static int start_daemon_called = 0;
/** True iff we've called finish_daemon(). */
static int finish_daemon_called = 0;
/** Socketpair used to communicate between parent and child process while
 * daemonizing. */
static int daemon_filedes[2];

/**
 * Return true iff we've called start_daemon() at least once.
 */
bool
start_daemon_has_been_called(void)
{
  return start_daemon_called != 0;
}

/** Start putting the process into daemon mode: fork and drop all resources
 * except standard fds.  The parent process never returns, but stays around
 * until finish_daemon is called.  (Note: it's safe to call this more
 * than once: calls after the first are ignored.)  Return true if we actually
 * forked and this is the child; false otherwise.
 */
int
start_daemon(void)
{
  pid_t pid;

  if (start_daemon_called)
    return 0;
  start_daemon_called = 1;

  if (pipe(daemon_filedes)) {
    /* LCOV_EXCL_START */
    log_err(LD_GENERAL,"pipe failed; exiting. Error was %s", strerror(errno));
    exit(1); // exit ok: during daemonize, pipe failed.
    /* LCOV_EXCL_STOP */
  }
  pid = fork();
  if (pid < 0) {
    /* LCOV_EXCL_START */
    log_err(LD_GENERAL,"fork failed. Exiting.");
    exit(1); // exit ok: during daemonize, fork failed
    /* LCOV_EXCL_STOP */
  }
  if (pid) {  /* Parent */
    int ok;
    char c;

    close(daemon_filedes[1]); /* we only read */
    ok = -1;
    while (0 < read(daemon_filedes[0], &c, sizeof(char))) {
      if (c == '.')
        ok = 1;
    }
    fflush(stdout);
    if (ok == 1)
      exit(0); // exit ok: during daemonize, daemonizing.
    else
      exit(1); /* child reported error. exit ok: daemonize failed. */
    return 0; // LCOV_EXCL_LINE unreachable
  } else { /* Child */
    close(daemon_filedes[0]); /* we only write */

    (void) setsid(); /* Detach from controlling terminal */
    /*
     * Fork one more time, so the parent (the session group leader) can exit.
     * This means that we, as a non-session group leader, can never regain a
     * controlling terminal.   This part is recommended by Stevens's
     * _Advanced Programming in the Unix Environment_.
     */
    if (fork() != 0) {
      exit(0); // exit ok: during daemonize, fork failed (2)
    }
    set_main_thread(); /* We are now the main thread. */

    return 1;
  }
}

/** Finish putting the process into daemon mode: drop standard fds, and tell
 * the parent process to exit.  (Note: it's safe to call this more than once:
 * calls after the first are ignored.  Calls start_daemon first if it hasn't
 * been called already.) Return true if we actually did a fork; false if we
 * didn't.
 */
int
finish_daemon(const char *desired_cwd)
{
  int nullfd;
  char c = '.';
  if (finish_daemon_called)
    return 0;
  if (!start_daemon_called)
    start_daemon();
  finish_daemon_called = 1;

  if (!desired_cwd)
    desired_cwd = "/";
   /* Don't hold the wrong FS mounted */
  if (chdir(desired_cwd) < 0) {
    log_err(LD_GENERAL,"chdir to \"%s\" failed. Exiting.",desired_cwd);
    exit(1); // exit ok: during daemonize, chdir failed.
  }

  nullfd = tor_open_cloexec("/dev/null", O_RDWR, 0);
  if (nullfd < 0) {
    /* LCOV_EXCL_START */
    log_err(LD_GENERAL,"/dev/null can't be opened. Exiting.");
    exit(1); // exit ok: during daemonize, couldn't open /dev/null
    /* LCOV_EXCL_STOP */
  }
  /* close fds linking to invoking terminal, but
   * close usual incoming fds, but redirect them somewhere
   * useful so the fds don't get reallocated elsewhere.
   */
  if (dup2(nullfd,0) < 0 ||
      dup2(nullfd,1) < 0 ||
      dup2(nullfd,2) < 0) {
    /* LCOV_EXCL_START */
    log_err(LD_GENERAL,"dup2 failed. Exiting.");
    exit(1); // exit ok: during daemonize, dup2 failed.
    /* LCOV_EXCL_STOP */
  }
  if (nullfd > 2)
    close(nullfd);
  /* signal success */
  if (write(daemon_filedes[1], &c, sizeof(char)) != sizeof(char)) {
    log_err(LD_GENERAL,"write failed. Exiting.");
  }
  close(daemon_filedes[1]);

  return 0;
}
#else /* defined(_WIN32) */
/* defined(_WIN32) */
int
start_daemon(void)
{
  return 0;
}
int
finish_daemon(const char *cp)
{
  (void)cp;
  return 0;
}
bool
start_daemon_has_been_called(void)
{
  return false;
}

#endif /* !defined(_WIN32) */
