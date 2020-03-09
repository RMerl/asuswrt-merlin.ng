/* Copyright (c) 2012-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

/* To prevent 'assert' from going away. */
#undef TOR_COVERAGE
#include "core/or/or.h"
#include "lib/err/backtrace.h"
#include "lib/log/log.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* -1: no crash.
 *  0: crash with a segmentation fault.
 *  1x: crash with an assertion failure. */
static int crashtype = 0;

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#endif

int crash(int x) NOINLINE;
int oh_what(int x) NOINLINE;
int a_tangled_web(int x) NOINLINE;
int we_weave(int x) NOINLINE;

#ifdef HAVE_CFLAG_WNULL_DEREFERENCE
DISABLE_GCC_WARNING(null-dereference)
#endif
int
crash(int x)
{
  if (crashtype == 0) {
#if defined(__clang_analyzer__) || defined(__COVERITY__)
    tor_assert(1 == 0); /* Avert your eyes, clangalyzer and coverity!  You
                         * don't need to see us dereference NULL. */
#else
    *(volatile int *)0 = 0;
#endif /* defined(__clang_analyzer__) || defined(__COVERITY__) */
  } else if (crashtype == 1) {
    tor_assertf(1 == 0, "%d != %d", 1, 0);
  } else if (crashtype == -1) {
    ;
  }

  crashtype *= x;
  return crashtype;
}
#ifdef HAVE_CFLAG_WNULL_DEREFERENCE
ENABLE_GCC_WARNING(null-dereference)
#endif

int
oh_what(int x)
{
  /* We call crash() twice here, so that the compiler won't try to do a
   * tail-call optimization.  Only the first call will actually happen, but
   * telling the compiler to maybe do the second call will prevent it from
   * replacing the first call with a jump. */
  return crash(x) + crash(x*2);
}

int
a_tangled_web(int x)
{
  return oh_what(x) * 99 + oh_what(x);
}

int
we_weave(int x)
{
  return a_tangled_web(x) + a_tangled_web(x+1);
}

int
main(int argc, char **argv)
{
  log_severity_list_t severity;

  if (argc < 2) {
    puts("I take an argument. It should be \"assert\" or \"crash\" or "
         "\"backtraces\" or \"none\"");
    return 1;
  }

#ifdef HAVE_SYS_RESOURCE_H
  struct rlimit rlim = { .rlim_cur = 0, .rlim_max = 0 };
  setrlimit(RLIMIT_CORE, &rlim);
#endif

#if !(defined(HAVE_EXECINFO_H) && defined(HAVE_BACKTRACE) && \
   defined(HAVE_BACKTRACE_SYMBOLS_FD) && defined(HAVE_SIGACTION))
    puts("Backtrace reporting is not supported on this platform");
    return 77;
#endif

  if (!strcmp(argv[1], "assert")) {
    crashtype = 1;
  } else if (!strcmp(argv[1], "crash")) {
    crashtype = 0;
  } else if (!strcmp(argv[1], "none")) {
    crashtype = -1;
  } else if (!strcmp(argv[1], "backtraces")) {
    return 0;
  } else {
    puts("Argument should be \"assert\" or \"crash\" or \"none\"");
    return 1;
  }

  init_logging(1);
  set_log_severity_config(LOG_WARN, LOG_ERR, &severity);
  add_stream_log(&severity, "stdout", STDOUT_FILENO);
  tor_log_update_sigsafe_err_fds();

  configure_backtrace_handler(NULL);

  printf("%d\n", we_weave(2));

  clean_up_backtrace_handler();
  logs_free_all();

  return 0;
}
