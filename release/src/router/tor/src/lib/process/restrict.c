/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file restrict.c
 * \brief Drop privileges from the current process.
 **/

#include "orconfig.h"
#include "lib/process/restrict.h"
#include "lib/intmath/cmp.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/net/socket.h"

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* We only use the linux prctl for now. There is no Win32 support; this may
 * also work on various BSD systems and Mac OS X - send testing feedback!
 *
 * On recent Gnu/Linux kernels it is possible to create a system-wide policy
 * that will prevent non-root processes from attaching to other processes
 * unless they are the parent process; thus gdb can attach to programs that
 * they execute but they cannot attach to other processes running as the same
 * user. The system wide policy may be set with the sysctl
 * kernel.yama.ptrace_scope or by inspecting
 * /proc/sys/kernel/yama/ptrace_scope and it is 1 by default on Ubuntu 11.04.
 *
 * This ptrace scope will be ignored on Gnu/Linux for users with
 * CAP_SYS_PTRACE and so it is very likely that root will still be able to
 * attach to the Tor process.
 */
/** Attempt to disable debugger attachment: return 1 on success, -1 on
 * failure, and 0 if we don't know how to try on this platform. */
int
tor_disable_debugger_attach(void)
{
  int r = -1;
  log_debug(LD_CONFIG,
            "Attempting to disable debugger attachment to Tor for "
            "unprivileged users.");
#if defined(__linux__) && defined(HAVE_SYS_PRCTL_H) \
  && defined(HAVE_PRCTL) && defined(PR_SET_DUMPABLE)
#define TRIED_TO_DISABLE
  r = prctl(PR_SET_DUMPABLE, 0);
#elif defined(__APPLE__) && defined(PT_DENY_ATTACH)
#define TRIED_TO_ATTACH
  r = ptrace(PT_DENY_ATTACH, 0, 0, 0);
#endif /* defined(__linux__) && defined(HAVE_SYS_PRCTL_H) ... || ... */

  // XXX: TODO - Mac OS X has dtrace and this may be disabled.
  // XXX: TODO - Windows probably has something similar
#ifdef TRIED_TO_DISABLE
  if (r == 0) {
    log_debug(LD_CONFIG,"Debugger attachment disabled for "
              "unprivileged users.");
    return 1;
  } else {
    log_warn(LD_CONFIG, "Unable to disable debugger attaching: %s",
             strerror(errno));
  }
#endif /* defined(TRIED_TO_DISABLE) */
#undef TRIED_TO_DISABLE
  return r;
}

#if defined(HAVE_MLOCKALL) && HAVE_DECL_MLOCKALL && defined(RLIMIT_MEMLOCK)
#define HAVE_UNIX_MLOCKALL
#endif

#ifdef HAVE_UNIX_MLOCKALL
/** Attempt to raise the current and max rlimit to infinity for our process.
 * This only needs to be done once and can probably only be done when we have
 * not already dropped privileges.
 */
static int
tor_set_max_memlock(void)
{
  /* Future consideration for Windows is probably SetProcessWorkingSetSize
   * This is similar to setting the memory rlimit of RLIMIT_MEMLOCK
   * https://msdn.microsoft.com/en-us/library/ms686234(VS.85).aspx
   */

  struct rlimit limit;

  /* RLIM_INFINITY is -1 on some platforms. */
  limit.rlim_cur = RLIM_INFINITY;
  limit.rlim_max = RLIM_INFINITY;

  if (setrlimit(RLIMIT_MEMLOCK, &limit) == -1) {
    if (errno == EPERM) {
      log_warn(LD_GENERAL, "You appear to lack permissions to change memory "
                           "limits. Are you root?");
    }
    log_warn(LD_GENERAL, "Unable to raise RLIMIT_MEMLOCK: %s",
             strerror(errno));
    return -1;
  }

  return 0;
}
#endif /* defined(HAVE_UNIX_MLOCKALL) */

/** Attempt to lock all current and all future memory pages.
 * This should only be called once and while we're privileged.
 * Like mlockall() we return 0 when we're successful and -1 when we're not.
 * Unlike mlockall() we return 1 if we've already attempted to lock memory.
 */
int
tor_mlockall(void)
{
  static int memory_lock_attempted = 0;

  if (memory_lock_attempted) {
    return 1;
  }

  memory_lock_attempted = 1;

  /*
   * Future consideration for Windows may be VirtualLock
   * VirtualLock appears to implement mlock() but not mlockall()
   *
   * https://msdn.microsoft.com/en-us/library/aa366895(VS.85).aspx
   */

#ifdef HAVE_UNIX_MLOCKALL
  if (tor_set_max_memlock() == 0) {
    log_debug(LD_GENERAL, "RLIMIT_MEMLOCK is now set to RLIM_INFINITY.");
  }

  if (mlockall(MCL_CURRENT|MCL_FUTURE) == 0) {
    log_info(LD_GENERAL, "Insecure OS paging is effectively disabled.");
    return 0;
  } else {
    if (errno == ENOSYS) {
      /* Apple - it's 2009! I'm looking at you. Grrr. */
      log_notice(LD_GENERAL, "It appears that mlockall() is not available on "
                             "your platform.");
    } else if (errno == EPERM) {
      log_notice(LD_GENERAL, "It appears that you lack the permissions to "
                             "lock memory. Are you root?");
    }
    log_notice(LD_GENERAL, "Unable to lock all current and future memory "
                           "pages: %s", strerror(errno));
    return -1;
  }
#else /* !defined(HAVE_UNIX_MLOCKALL) */
  log_warn(LD_GENERAL, "Unable to lock memory pages. mlockall() unsupported?");
  return -1;
#endif /* defined(HAVE_UNIX_MLOCKALL) */
}

/** Number of extra file descriptors to keep in reserve beyond those that we
 * tell Tor it's allowed to use. */
#define ULIMIT_BUFFER 32 /* keep 32 extra fd's beyond ConnLimit_ */

/** Learn the maximum allowed number of file descriptors, and tell the
 * system we want to use up to that number. (Some systems have a low soft
 * limit, and let us set it higher.)  We compute this by finding the largest
 * number that we can use.
 *
 * If the limit is below the reserved file descriptor value (ULIMIT_BUFFER),
 * return -1 and <b>max_out</b> is untouched.
 *
 * If we can't find a number greater than or equal to <b>limit</b>, then we
 * fail by returning -1 and <b>max_out</b> is untouched.
 *
 * If we are unable to set the limit value because of setrlimit() failing,
 * return 0 and <b>max_out</b> is set to the current maximum value returned
 * by getrlimit().
 *
 * Otherwise, return 0 and store the maximum we found inside <b>max_out</b>
 * and set <b>max_sockets</b> with that value as well.*/
int
set_max_file_descriptors(rlim_t limit, int *max_out)
{
  if (limit < ULIMIT_BUFFER) {
    log_warn(LD_CONFIG,
             "ConnLimit must be at least %d. Failing.", ULIMIT_BUFFER);
    return -1;
  }

  /* Define some maximum connections values for systems where we cannot
   * automatically determine a limit. Re Cygwin, see
   * https://archives.seul.org/or/talk/Aug-2006/msg00210.html
   * For an iPhone, 9999 should work. For Windows and all other unknown
   * systems we use 15000 as the default. */
#ifndef HAVE_GETRLIMIT
#if defined(CYGWIN) || defined(__CYGWIN__)
  const char *platform = "Cygwin";
  const unsigned long MAX_CONNECTIONS = 3200;
#elif defined(_WIN32)
  const char *platform = "Windows";
  const unsigned long MAX_CONNECTIONS = 15000;
#else
  const char *platform = "unknown platforms with no getrlimit()";
  const unsigned long MAX_CONNECTIONS = 15000;
#endif /* defined(CYGWIN) || defined(__CYGWIN__) || ... */
  log_fn(LOG_INFO, LD_NET,
         "This platform is missing getrlimit(). Proceeding.");
  if (limit > MAX_CONNECTIONS) {
    log_warn(LD_CONFIG,
             "We do not support more than %lu file descriptors "
             "on %s. Tried to raise to %lu.",
             (unsigned long)MAX_CONNECTIONS, platform, (unsigned long)limit);
    return -1;
  }
  limit = MAX_CONNECTIONS;
#else /* defined(HAVE_GETRLIMIT) */
  struct rlimit rlim;

  if (getrlimit(RLIMIT_NOFILE, &rlim) != 0) {
    log_warn(LD_NET, "Could not get maximum number of file descriptors: %s",
             strerror(errno));
    return -1;
  }
  if (rlim.rlim_max < limit) {
    log_warn(LD_CONFIG,"We need %lu file descriptors available, and we're "
             "limited to %lu. Please change your ulimit -n.",
             (unsigned long)limit, (unsigned long)rlim.rlim_max);
    return -1;
  }

  if (rlim.rlim_max > rlim.rlim_cur) {
    log_info(LD_NET,"Raising max file descriptors from %lu to %lu.",
             (unsigned long)rlim.rlim_cur, (unsigned long)rlim.rlim_max);
  }
  /* Set the current limit value so if the attempt to set the limit to the
   * max fails at least we'll have a valid value of maximum sockets. */
  *max_out = (int)rlim.rlim_cur - ULIMIT_BUFFER;
  set_max_sockets(*max_out);
  rlim.rlim_cur = rlim.rlim_max;

  if (setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
    int couldnt_set = 1;
    const int setrlimit_errno = errno;
#ifdef OPEN_MAX
    uint64_t try_limit = OPEN_MAX - ULIMIT_BUFFER;
    if (errno == EINVAL && try_limit < (uint64_t) rlim.rlim_cur) {
      /* On some platforms, OPEN_MAX is the real limit, and getrlimit() is
       * full of nasty lies.  I'm looking at you, OSX 10.5.... */
      rlim.rlim_cur = MIN((rlim_t) try_limit, rlim.rlim_cur);
      if (setrlimit(RLIMIT_NOFILE, &rlim) == 0) {
        if (rlim.rlim_cur < (rlim_t)limit) {
          log_warn(LD_CONFIG, "We are limited to %lu file descriptors by "
                   "OPEN_MAX (%lu), and ConnLimit is %lu.  Changing "
                   "ConnLimit; sorry.",
                   (unsigned long)try_limit, (unsigned long)OPEN_MAX,
                   (unsigned long)limit);
        } else {
          log_info(LD_CONFIG, "Dropped connection limit to %lu based on "
                   "OPEN_MAX (%lu); Apparently, %lu was too high and rlimit "
                   "lied to us.",
                   (unsigned long)try_limit, (unsigned long)OPEN_MAX,
                   (unsigned long)rlim.rlim_max);
        }
        couldnt_set = 0;
      }
    }
#endif /* defined(OPEN_MAX) */
    if (couldnt_set) {
      log_warn(LD_CONFIG,"Couldn't set maximum number of file descriptors: %s",
               strerror(setrlimit_errno));
    }
  }
  /* leave some overhead for logs, etc, */
  limit = rlim.rlim_cur;
#endif /* !defined(HAVE_GETRLIMIT) */

  if (limit > INT_MAX)
    limit = INT_MAX;
  tor_assert(max_out);
  *max_out = (int)limit - ULIMIT_BUFFER;
  set_max_sockets(*max_out);

  return 0;
}
