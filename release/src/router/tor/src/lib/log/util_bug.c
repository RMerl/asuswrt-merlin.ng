/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file util_bug.c
 **/

#include "orconfig.h"
#include "lib/log/util_bug.h"
#include "lib/log/log.h"
#include "lib/err/backtrace.h"
#include "lib/err/torerr.h"
#ifdef TOR_UNIT_TESTS
#include "lib/smartlist_core/smartlist_core.h"
#include "lib/smartlist_core/smartlist_foreach.h"
#endif
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"

#include <string.h>
#include <stdlib.h>

#ifdef TOR_UNIT_TESTS
static void (*failed_assertion_cb)(void) = NULL;
static int n_bugs_to_capture = 0;
static smartlist_t *bug_messages = NULL;
#define capturing_bugs() (bug_messages != NULL && n_bugs_to_capture)
void
tor_capture_bugs_(int n)
{
  tor_end_capture_bugs_();
  bug_messages = smartlist_new();
  n_bugs_to_capture = n;
}
void
tor_end_capture_bugs_(void)
{
  n_bugs_to_capture = 0;
  if (!bug_messages)
    return;
  SMARTLIST_FOREACH(bug_messages, char *, cp, tor_free(cp));
  smartlist_free(bug_messages);
  bug_messages = NULL;
}
const smartlist_t *
tor_get_captured_bug_log_(void)
{
  return bug_messages;
}
static void
add_captured_bug(const char *s)
{
  --n_bugs_to_capture;
  smartlist_add_strdup(bug_messages, s);
}
/** Set a callback to be invoked when we get any tor_bug_occurred_
 * invocation. We use this in the unit tests so that a nonfatal
 * assertion failure can also count as a test failure.
 */
void
tor_set_failed_assertion_callback(void (*fn)(void))
{
  failed_assertion_cb = fn;
}
#else /* !defined(TOR_UNIT_TESTS) */
#define capturing_bugs() (0)
#define add_captured_bug(s) do { } while (0)
#endif /* defined(TOR_UNIT_TESTS) */

/** Helper for tor_assert: report the assertion failure. */
void
tor_assertion_failed_(const char *fname, unsigned int line,
                      const char *func, const char *expr,
                      const char *fmt, ...)
{
  char *buf = NULL;
  char *extra = NULL;
  va_list ap;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
  if (fmt) {
    va_start(ap,fmt);
    tor_vasprintf(&extra, fmt, ap);
    va_end(ap);
  }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

  log_err(LD_BUG, "%s:%u: %s: Assertion %s failed; aborting.",
          fname, line, func, expr);
  tor_asprintf(&buf, "Assertion %s failed in %s at %s:%u: %s",
               expr, func, fname, line, extra ? extra : "");
  tor_free(extra);
  log_backtrace(LOG_ERR, LD_BUG, buf);
  tor_free(buf);
}

/** Helper for tor_assert_nonfatal: report the assertion failure. */
void
tor_bug_occurred_(const char *fname, unsigned int line,
                  const char *func, const char *expr,
                  int once, const char *fmt, ...)
{
  char *buf = NULL;
  const char *once_str = once ?
    " (Future instances of this warning will be silenced.)": "";
  if (! expr) {
    if (capturing_bugs()) {
      add_captured_bug("This line should not have been reached.");
      return;
    }
    log_warn(LD_BUG, "%s:%u: %s: This line should not have been reached.%s",
             fname, line, func, once_str);
    tor_asprintf(&buf,
                 "Line unexpectedly reached at %s at %s:%u",
                 func, fname, line);
  } else {
    if (capturing_bugs()) {
      add_captured_bug(expr);
      return;
    }

    va_list ap;
    char *extra = NULL;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    if (fmt) {
      va_start(ap,fmt);
      tor_vasprintf(&extra, fmt, ap);
      va_end(ap);
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    log_warn(LD_BUG, "%s:%u: %s: Non-fatal assertion %s failed.%s",
             fname, line, func, expr, once_str);
    tor_asprintf(&buf, "Non-fatal assertion %s failed in %s at %s:%u%s%s",
                 expr, func, fname, line, fmt ? " : " : "",
                 extra ? extra : "");
    tor_free(extra);
  }
  log_backtrace(LOG_WARN, LD_BUG, buf);
  tor_free(buf);

#ifdef TOR_UNIT_TESTS
  if (failed_assertion_cb) {
    failed_assertion_cb();
  }
#endif
}

/**
 * Call the tor_raw_abort_() function to close raw logs, then kill the current
 * process with a fatal error. But first, close the file-based log file
 * descriptors, so error messages are written before process termination.
 *
 * (This is a separate function so that we declare it in util_bug.h without
 * including torerr.h in all the users of util_bug.h)
 **/
void
tor_abort_(void)
{
  logs_flush_sigsafe();
  tor_raw_abort_();
}

#ifdef _WIN32
/** Take a filename and return a pointer to its final element.  This
 * function is called on __FILE__ to fix a MSVC nit where __FILE__
 * contains the full path to the file.  This is bad, because it
 * confuses users to find the home directory of the person who
 * compiled the binary in their warning messages.
 */
const char *
tor_fix_source_file(const char *fname)
{
  const char *cp1, *cp2, *r;
  cp1 = strrchr(fname, '/');
  cp2 = strrchr(fname, '\\');
  if (cp1 && cp2) {
    r = (cp1<cp2)?(cp2+1):(cp1+1);
  } else if (cp1) {
    r = cp1+1;
  } else if (cp2) {
    r = cp2+1;
  } else {
    r = fname;
  }
  return r;
}
#endif /* defined(_WIN32) */
