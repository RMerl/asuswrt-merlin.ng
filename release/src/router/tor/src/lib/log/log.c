/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file log.c
 * \brief Functions to send messages to log files or the console.
 **/

#include "orconfig.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#define LOG_PRIVATE
#include "lib/log/log.h"
#include "lib/log/log_sys.h"
#include "lib/version/git_revision.h"
#include "lib/log/ratelim.h"
#include "lib/lock/compat_mutex.h"
#include "lib/smartlist_core/smartlist_core.h"
#include "lib/smartlist_core/smartlist_foreach.h"
#include "lib/smartlist_core/smartlist_split.h"
#include "lib/err/torerr.h"
#include "lib/intmath/bits.h"
#include "lib/string/compat_string.h"
#include "lib/string/printf.h"
#include "lib/malloc/malloc.h"
#include "lib/string/util_string.h"
#include "lib/wallclock/tor_gettimeofday.h"
#include "lib/wallclock/approx_time.h"
#include "lib/wallclock/time_to_tm.h"
#include "lib/fdio/fdio.h"
#include "lib/cc/ctassert.h"

/** @{ */
/** The string we stick at the end of a log message when it is too long,
 * and its length. */
#define TRUNCATED_STR "[...truncated]"
#define TRUNCATED_STR_LEN 14
/** @} */

/** Defining compile-time constants for Tor log levels (used by the Rust
 * log wrapper at src/rust/tor_log) */
const int LOG_WARN_ = LOG_WARN;
const int LOG_NOTICE_ = LOG_NOTICE;
const log_domain_mask_t LD_GENERAL_ = LD_GENERAL;
const log_domain_mask_t LD_NET_ = LD_NET;

/** Information for a single logfile; only used in log.c */
typedef struct logfile_t {
  struct logfile_t *next; /**< Next logfile_t in the linked list. */
  char *filename; /**< Filename to open. */
  int fd; /**< fd to receive log messages, or -1 for none. */
  int seems_dead; /**< Boolean: true if the stream seems to be kaput. */
  int needs_close; /**< Boolean: true if the stream gets closed on shutdown. */
  int is_temporary; /**< Boolean: close after initializing logging subsystem.*/
  int is_syslog; /**< Boolean: send messages to syslog. */
  log_callback callback; /**< If not NULL, send messages to this function. */
  log_severity_list_t *severities; /**< Which severity of messages should we
                                    * log for each log domain? */
} logfile_t;

static void log_free_(logfile_t *victim);
#define log_free(lg)    \
  FREE_AND_NULL(logfile_t, log_free_, (lg))

/** Helper: map a log severity to descriptive string. */
static inline const char *
sev_to_string(int severity)
{
  switch (severity) {
    case LOG_DEBUG:   return "debug";
    case LOG_INFO:    return "info";
    case LOG_NOTICE:  return "notice";
    case LOG_WARN:    return "warn";
    case LOG_ERR:     return "err";
    default:     /* Call raw_assert, not tor_assert, since tor_assert
                  * calls log on failure. */
                 raw_assert_unreached(); return "UNKNOWN"; // LCOV_EXCL_LINE
  }
}

/** Helper: decide whether to include the function name in the log message. */
static inline int
should_log_function_name(log_domain_mask_t domain, int severity)
{
  switch (severity) {
    case LOG_DEBUG:
    case LOG_INFO:
      /* All debugging messages occur in interesting places. */
      return (domain & LD_NOFUNCNAME) == 0;
    case LOG_NOTICE:
    case LOG_WARN:
    case LOG_ERR:
      /* We care about places where bugs occur. */
      return (domain & (LD_BUG|LD_NOFUNCNAME)) == LD_BUG;
    default:
      /* Call raw_assert, not tor_assert, since tor_assert calls
       * log on failure. */
      raw_assert(0); return 0; // LCOV_EXCL_LINE
  }
}

/** A mutex to guard changes to logfiles and logging. */
static tor_mutex_t log_mutex;
/** True iff we have initialized log_mutex */
static int log_mutex_initialized = 0;

/** Linked list of logfile_t. */
static logfile_t *logfiles = NULL;
/** Boolean: do we report logging domains? */
static int log_domains_are_logged = 0;

#ifdef HAVE_SYSLOG_H
/** The number of open syslog log handlers that we have.  When this reaches 0,
 * we can close our connection to the syslog facility. */
static int syslog_count = 0;
#endif

/** Represents a log message that we are going to send to callback-driven
 * loggers once we can do so in a non-reentrant way. */
typedef struct pending_log_message_t {
  int severity; /**< The severity of the message */
  log_domain_mask_t domain; /**< The domain of the message */
  char *fullmsg; /**< The message, with all decorations */
  char *msg; /**< The content of the message */
} pending_log_message_t;

/** Log messages waiting to be replayed onto callback-based logs */
static smartlist_t *pending_cb_messages = NULL;

/** Callback to invoke when pending_cb_messages becomes nonempty. */
static pending_callback_callback pending_cb_cb = NULL;

/** Log messages waiting to be replayed once the logging system is initialized.
 */
static smartlist_t *pending_startup_messages = NULL;

/** Number of bytes of messages queued in pending_startup_messages.  (This is
 * the length of the messages, not the number of bytes used to store
 * them.) */
static size_t pending_startup_messages_len;

/** True iff we should store messages while waiting for the logs to get
 * configured. */
static int queue_startup_messages = 1;

/** True iff __PRETTY_FUNCTION__ includes parenthesized arguments. */
static int pretty_fn_has_parens = 0;

/** Don't store more than this many bytes of messages while waiting for the
 * logs to get configured. */
#define MAX_STARTUP_MSG_LEN (1<<16)

/** Lock the log_mutex to prevent others from changing the logfile_t list */
#define LOCK_LOGS() STMT_BEGIN                                          \
  raw_assert(log_mutex_initialized);                                    \
  tor_mutex_acquire(&log_mutex);                                        \
  STMT_END
/** Unlock the log_mutex */
#define UNLOCK_LOGS() STMT_BEGIN                                        \
  raw_assert(log_mutex_initialized);                                    \
  tor_mutex_release(&log_mutex);                                        \
  STMT_END

/** What's the lowest log level anybody cares about?  Checking this lets us
 * bail out early from log_debug if we aren't debugging.  */
int log_global_min_severity_ = LOG_NOTICE;

static void delete_log(logfile_t *victim);
static void close_log(logfile_t *victim);
static void close_log_sigsafe(logfile_t *victim);

static char *domain_to_string(log_domain_mask_t domain,
                             char *buf, size_t buflen);
static inline char *format_msg(char *buf, size_t buf_len,
           log_domain_mask_t domain, int severity, const char *funcname,
           const char *suffix,
           const char *format, va_list ap, size_t *msg_len_out)
  CHECK_PRINTF(7,0);

/** Name of the application: used to generate the message we write at the
 * start of each new log. */
static char *appname = NULL;

/** Set the "application name" for the logs to <b>name</b>: we'll use this
 * name in the message we write when starting up, and at the start of each new
 * log.
 *
 * Tor uses this string to write the version number to the log file. */
void
log_set_application_name(const char *name)
{
  tor_free(appname);
  appname = name ? tor_strdup(name) : NULL;
}

/** Return true if some of the running logs might be interested in a log
 * message of the given severity in the given domains. If this function
 * returns true, the log message might be ignored anyway, but if it returns
 * false, it is definitely_ safe not to log the message. */
int
log_message_is_interesting(int severity, log_domain_mask_t domain)
{
  (void) domain;
  return (severity <= log_global_min_severity_);
}

/**
 * As tor_log, but takes an optional function name, and does not treat its
 * <b>string</b> as a printf format.
 *
 * For use by Rust integration.
 */
void
tor_log_string(int severity, log_domain_mask_t domain,
               const char *function, const char *string)
{
  log_fn_(severity, domain, function, "%s", string);
}

/** Log time granularity in milliseconds. */
static int log_time_granularity = 1;

/** Define log time granularity for all logs to be <b>granularity_msec</b>
 * milliseconds. */
MOCK_IMPL(void,
set_log_time_granularity,(int granularity_msec))
{
  log_time_granularity = granularity_msec;
  tor_log_sigsafe_err_set_granularity(granularity_msec);
}

/** Helper: Write the standard prefix for log lines to a
 * <b>buf_len</b> character buffer in <b>buf</b>.
 */
static inline size_t
log_prefix_(char *buf, size_t buf_len, int severity)
{
  time_t t;
  struct timeval now;
  struct tm tm;
  size_t n;
  int r, ms;

  tor_gettimeofday(&now);
  t = (time_t)now.tv_sec;
  ms = (int)now.tv_usec / 1000;
  if (log_time_granularity >= 1000) {
    t -= t % (log_time_granularity / 1000);
    ms = 0;
  } else {
    ms -= ((int)now.tv_usec / 1000) % log_time_granularity;
  }

  n = strftime(buf, buf_len, "%b %d %H:%M:%S",
               tor_localtime_r_msg(&t, &tm, NULL));
  r = tor_snprintf(buf+n, buf_len-n, ".%.3i [%s] ", ms,
                   sev_to_string(severity));

  if (r<0)
    return buf_len-1;
  else
    return n+r;
}

/** If lf refers to an actual file that we have just opened, and the file
 * contains no data, log an "opening new logfile" message at the top.
 *
 * Return -1 if the log is broken and needs to be deleted, else return 0.
 */
static int
log_tor_version(logfile_t *lf, int reset)
{
  char buf[256];
  size_t n;
  int is_new;

  if (!lf->needs_close)
    /* If it doesn't get closed, it isn't really a file. */
    return 0;
  if (lf->is_temporary)
    /* If it's temporary, it isn't really a file. */
    return 0;

  is_new = lf->fd >= 0 && tor_fd_getpos(lf->fd) == 0;

  if (reset && !is_new)
    /* We are resetting, but we aren't at the start of the file; no
     * need to log again. */
    return 0;
  n = log_prefix_(buf, sizeof(buf), LOG_NOTICE);
  if (appname) {
    tor_snprintf(buf+n, sizeof(buf)-n,
                 "%s opening %slog file.\n", appname, is_new?"new ":"");
  } else {
    tor_snprintf(buf+n, sizeof(buf)-n,
                 "Tor %s opening %slog file.\n", VERSION, is_new?"new ":"");
  }
  if (write_all_to_fd_minimal(lf->fd, buf, strlen(buf)) < 0) /* error */
    return -1; /* failed */
  return 0;
}

/** Helper: Format a log message into a fixed-sized buffer. (This is
 * factored out of <b>logv</b> so that we never format a message more
 * than once.)  Return a pointer to the first character of the message
 * portion of the formatted string.
 */
static inline char *
format_msg(char *buf, size_t buf_len,
           log_domain_mask_t domain, int severity, const char *funcname,
           const char *suffix,
           const char *format, va_list ap, size_t *msg_len_out)
{
  size_t n;
  int r;
  char *end_of_prefix;
  char *buf_end;

  raw_assert(buf_len >= 16); /* prevent integer underflow and stupidity */
  buf_len -= 2; /* subtract 2 characters so we have room for \n\0 */
  buf_end = buf+buf_len; /* point *after* the last char we can write to */

  n = log_prefix_(buf, buf_len, severity);
  end_of_prefix = buf+n;

  if (log_domains_are_logged) {
    char *cp = buf+n;
    if (cp == buf_end) goto format_msg_no_room_for_domains;
    *cp++ = '{';
    if (cp == buf_end) goto format_msg_no_room_for_domains;
    cp = domain_to_string(domain, cp, (buf+buf_len-cp));
    if (cp == buf_end) goto format_msg_no_room_for_domains;
    *cp++ = '}';
    if (cp == buf_end) goto format_msg_no_room_for_domains;
    *cp++ = ' ';
    if (cp == buf_end) goto format_msg_no_room_for_domains;
    end_of_prefix = cp;
    n = cp-buf;
  format_msg_no_room_for_domains:
    /* This will leave end_of_prefix and n unchanged, and thus cause
     * whatever log domain string we had written to be clobbered. */
    ;
  }

  if (funcname && should_log_function_name(domain, severity)) {
    r = tor_snprintf(buf+n, buf_len-n,
                     pretty_fn_has_parens ? "%s: " : "%s(): ",
                     funcname);
    if (r<0)
      n = strlen(buf);
    else
      n += r;
  }

  if (domain == LD_BUG && buf_len-n > 6) {
    memcpy(buf+n, "Bug: ", 6);
    n += 5;
  }

  r = tor_vsnprintf(buf+n,buf_len-n,format,ap);
  if (r < 0) {
    /* The message was too long; overwrite the end of the buffer with
     * "[...truncated]" */
    if (buf_len >= TRUNCATED_STR_LEN) {
      size_t offset = buf_len-TRUNCATED_STR_LEN;
      /* We have an extra 2 characters after buf_len to hold the \n\0,
       * so it's safe to add 1 to the size here. */
      strlcpy(buf+offset, TRUNCATED_STR, buf_len-offset+1);
    }
    /* Set 'n' to the end of the buffer, where we'll be writing \n\0.
     * Since we already subtracted 2 from buf_len, this is safe.*/
    n = buf_len;
  } else {
    n += r;
    if (suffix) {
      size_t suffix_len = strlen(suffix);
      if (buf_len-n >= suffix_len) {
        memcpy(buf+n, suffix, suffix_len);
        n += suffix_len;
      }
    }
  }

  if (domain == LD_BUG &&
      buf_len - n > strlen(tor_bug_suffix)+1) {
    memcpy(buf+n, tor_bug_suffix, strlen(tor_bug_suffix));
    n += strlen(tor_bug_suffix);
  }

  buf[n]='\n';
  buf[n+1]='\0';
  *msg_len_out = n+1;
  return end_of_prefix;
}

/* Create a new pending_log_message_t with appropriate values */
static pending_log_message_t *
pending_log_message_new(int severity, log_domain_mask_t domain,
                        const char *fullmsg, const char *shortmsg)
{
  pending_log_message_t *m = tor_malloc(sizeof(pending_log_message_t));
  m->severity = severity;
  m->domain = domain;
  m->fullmsg = fullmsg ? tor_strdup(fullmsg) : NULL;
  m->msg = tor_strdup(shortmsg);
  return m;
}

#define pending_log_message_free(msg) \
  FREE_AND_NULL(pending_log_message_t, pending_log_message_free_, (msg))

/** Release all storage held by <b>msg</b>. */
static void
pending_log_message_free_(pending_log_message_t *msg)
{
  if (!msg)
    return;
  tor_free(msg->msg);
  tor_free(msg->fullmsg);
  tor_free(msg);
}

/** Helper function: returns true iff the log file, given in <b>lf</b>, is
 * handled externally via the system log API, or is an
 * external callback function. */
static inline int
logfile_is_external(const logfile_t *lf)
{
  raw_assert(lf);
  return lf->is_syslog || lf->callback;
}

/** Return true iff <b>lf</b> would like to receive a message with the
 * specified <b>severity</b> in the specified <b>domain</b>.
 */
static inline int
logfile_wants_message(const logfile_t *lf, int severity,
                      log_domain_mask_t domain)
{
  if (! (lf->severities->masks[SEVERITY_MASK_IDX(severity)] & domain)) {
    return 0;
  }
  if (! (lf->fd >= 0 || logfile_is_external(lf))) {
    return 0;
  }
  if (lf->seems_dead) {
    return 0;
  }

  return 1;
}

/** Send a message to <b>lf</b>.  The full message, with time prefix and
 * severity, is in <b>buf</b>.  The message itself is in
 * <b>msg_after_prefix</b>.  If <b>callbacks_deferred</b> points to true, then
 * we already deferred this message for pending callbacks and don't need to do
 * it again.  Otherwise, if we need to do it, do it, and set
 * <b>callbacks_deferred</b> to 1. */
static inline void
logfile_deliver(logfile_t *lf, const char *buf, size_t msg_len,
                const char *msg_after_prefix, log_domain_mask_t domain,
                int severity, int *callbacks_deferred)
{

  if (lf->is_syslog) {
#ifdef HAVE_SYSLOG_H
#ifdef MAXLINE
    /* Some syslog implementations have limits on the length of what you can
     * pass them, and some very old ones do not detect overflow so well.
     * Regrettably, they call their maximum line length MAXLINE. */
#if MAXLINE < 64
#warning "MAXLINE is very low; it might not be from syslog.h."
#endif
    char *m = msg_after_prefix;
    if (msg_len >= MAXLINE)
      m = tor_strndup(msg_after_prefix, MAXLINE-1);
    syslog(severity, "%s", m);
    if (m != msg_after_prefix) {
      tor_free(m);
    }
#else /* !defined(MAXLINE) */
    /* We have syslog but not MAXLINE.  That's promising! */
    syslog(severity, "%s", msg_after_prefix);
#endif /* defined(MAXLINE) */
#endif /* defined(HAVE_SYSLOG_H) */
  } else if (lf->callback) {
    if (domain & LD_NOCB) {
      if (!*callbacks_deferred && pending_cb_messages) {
        smartlist_add(pending_cb_messages,
            pending_log_message_new(severity,domain,NULL,msg_after_prefix));
        *callbacks_deferred = 1;
        if (smartlist_len(pending_cb_messages) == 1 && pending_cb_cb) {
          pending_cb_cb();
        }
      }
    } else {
      lf->callback(severity, domain, msg_after_prefix);
    }
  } else {
    if (write_all_to_fd_minimal(lf->fd, buf, msg_len) < 0) { /* error */
      /* don't log the error! mark this log entry to be blown away, and
       * continue. */
      lf->seems_dead = 1;
    }
  }
}

/** Helper: sends a message to the appropriate logfiles, at loglevel
 * <b>severity</b>.  If provided, <b>funcname</b> is prepended to the
 * message.  The actual message is derived as from tor_snprintf(format,ap).
 */
MOCK_IMPL(STATIC void,
logv,(int severity, log_domain_mask_t domain, const char *funcname,
     const char *suffix, const char *format, va_list ap))
{
  char buf[10240];
  size_t msg_len = 0;
  int formatted = 0;
  logfile_t *lf;
  char *end_of_prefix=NULL;
  int callbacks_deferred = 0;

  /* Call raw_assert, not tor_assert, since tor_assert calls log on failure. */
  raw_assert(format);
  /* check that severity is sane.  Overrunning the masks array leads to
   * interesting and hard to diagnose effects */
  raw_assert(severity >= LOG_ERR && severity <= LOG_DEBUG);

  LOCK_LOGS();

  if ((! (domain & LD_NOCB)) && pending_cb_messages
      && smartlist_len(pending_cb_messages))
    flush_pending_log_callbacks();

  if (queue_startup_messages &&
      pending_startup_messages_len < MAX_STARTUP_MSG_LEN) {
    end_of_prefix =
      format_msg(buf, sizeof(buf), domain, severity, funcname, suffix,
      format, ap, &msg_len);
    formatted = 1;

    smartlist_add(pending_startup_messages,
      pending_log_message_new(severity,domain,buf,end_of_prefix));
    pending_startup_messages_len += msg_len;
  }

  for (lf = logfiles; lf; lf = lf->next) {
    if (! logfile_wants_message(lf, severity, domain))
      continue;

    if (!formatted) {
      end_of_prefix =
        format_msg(buf, sizeof(buf), domain, severity, funcname, suffix,
                   format, ap, &msg_len);
      formatted = 1;
    }

    logfile_deliver(lf, buf, msg_len, end_of_prefix, domain, severity,
      &callbacks_deferred);
  }
  UNLOCK_LOGS();
}

/** Output a message to the log.  It gets logged to all logfiles that
 * care about messages with <b>severity</b> in <b>domain</b>. The content
 * is formatted printf-style based on <b>format</b> and extra arguments.
 * */
void
tor_log(int severity, log_domain_mask_t domain, const char *format, ...)
{
  va_list ap;

  /* check that domain is composed of known domains and flags */
  raw_assert((domain & (LD_ALL_DOMAINS|LD_ALL_FLAGS)) == domain);

  if (severity > log_global_min_severity_)
    return;
  va_start(ap,format);
#ifdef TOR_UNIT_TESTS
  if (domain & LD_NO_MOCK)
    logv__real(severity, domain, NULL, NULL, format, ap);
  else
#endif
  logv(severity, domain, NULL, NULL, format, ap);
  va_end(ap);
}

/** Helper function; return true iff the <b>n</b>-element array <b>array</b>
 * contains <b>item</b>. */
static int
int_array_contains(const int *array, int n, int item)
{
  int j;
  for (j = 0; j < n; ++j) {
    if (array[j] == item)
      return 1;
  }
  return 0;
}

/** Function to call whenever the list of logs changes to get ready to log
 * from signal handlers. */
void
tor_log_update_sigsafe_err_fds(void)
{
  const logfile_t *lf;
  int found_real_stderr = 0;

  /* The fds are the file descriptors of tor's stdout, stderr, and file
   * logs. The log and err modules flush these fds during their shutdowns. */
  int fds[TOR_SIGSAFE_LOG_MAX_FDS];
  int n_fds;

  LOCK_LOGS();
  /* Reserve the first one for stderr. This is safe because when we daemonize,
   * we dup2 /dev/null to stderr. */
  fds[0] = STDERR_FILENO;
  n_fds = 1;

  for (lf = logfiles; lf; lf = lf->next) {
     /* Don't try callback to the control port, syslogs, or any
      * other non-file descriptor log: We can't call arbitrary functions from a
      * signal handler.
      */
    if (lf->is_temporary || logfile_is_external(lf)
        || lf->seems_dead || lf->fd < 0)
      continue;
    if (lf->severities->masks[SEVERITY_MASK_IDX(LOG_ERR)] &
        (LD_BUG|LD_GENERAL)) {
      if (lf->fd == STDERR_FILENO)
        found_real_stderr = 1;
      /* Avoid duplicates by checking the log module fd against fds */
      if (int_array_contains(fds, n_fds, lf->fd))
        continue;
      /* Update fds using the log module's fd */
      fds[n_fds] = lf->fd;
      n_fds++;
      if (n_fds == TOR_SIGSAFE_LOG_MAX_FDS)
        break;
    }
  }

  if (!found_real_stderr &&
      int_array_contains(fds, n_fds, STDOUT_FILENO)) {
    /* Don't use a virtual stderr when we're also logging to stdout.
     * If we reached max_fds logs, we'll now have (max_fds - 1) logs.
     * That's ok, max_fds is large enough that most tor instances don't exceed
     * it. */
    raw_assert(n_fds >= 2); /* Don't tor_assert inside log fns */
    --n_fds;
    fds[0] = fds[n_fds];
  }

  UNLOCK_LOGS();

  tor_log_set_sigsafe_err_fds(fds, n_fds);
}

/** Add to <b>out</b> a copy of every currently configured log file name. Used
 * to enable access to these filenames with the sandbox code. */
void
tor_log_get_logfile_names(smartlist_t *out)
{
  logfile_t *lf;
  raw_assert(out);

  LOCK_LOGS();

  for (lf = logfiles; lf; lf = lf->next) {
    if (lf->is_temporary || logfile_is_external(lf))
      continue;
    if (lf->filename == NULL)
      continue;
    smartlist_add_strdup(out, lf->filename);
  }

  UNLOCK_LOGS();
}

/** Implementation of the log_fn backend, used when we have
 * variadic macros. All arguments are as for log_fn, except for
 * <b>fn</b>, which is the name of the calling function. */
void
log_fn_(int severity, log_domain_mask_t domain, const char *fn,
        const char *format, ...)
{
  va_list ap;
  if (severity > log_global_min_severity_)
    return;
  va_start(ap,format);
  logv(severity, domain, fn, NULL, format, ap);
  va_end(ap);
}
void
log_fn_ratelim_(ratelim_t *ratelim, int severity, log_domain_mask_t domain,
                const char *fn, const char *format, ...)
{
  va_list ap;
  char *m;
  if (severity > log_global_min_severity_)
    return;
  m = rate_limit_log(ratelim, approx_time());
  if (m == NULL)
      return;
  va_start(ap, format);
  logv(severity, domain, fn, m, format, ap);
  va_end(ap);
  tor_free(m);
}

/** Free all storage held by <b>victim</b>. */
static void
log_free_(logfile_t *victim)
{
  if (!victim)
    return;
  tor_free(victim->severities);
  tor_free(victim->filename);
  tor_free(victim);
}

/** Close all open log files, and free other static memory. */
void
logs_free_all(void)
{
  logfile_t *victim, *next;
  smartlist_t *messages, *messages2;
  LOCK_LOGS();
  next = logfiles;
  logfiles = NULL;
  messages = pending_cb_messages;
  pending_cb_messages = NULL;
  pending_cb_cb = NULL;
  messages2 = pending_startup_messages;
  pending_startup_messages = NULL;
  UNLOCK_LOGS();
  while (next) {
    victim = next;
    next = next->next;
    close_log(victim);
    log_free(victim);
  }
  tor_free(appname);

  SMARTLIST_FOREACH(messages, pending_log_message_t *, msg, {
      pending_log_message_free(msg);
    });
  smartlist_free(messages);

  if (messages2) {
    SMARTLIST_FOREACH(messages2, pending_log_message_t *, msg, {
        pending_log_message_free(msg);
      });
    smartlist_free(messages2);
  }

  /* We _could_ destroy the log mutex here, but that would screw up any logs
   * that happened between here and the end of execution.
   * If tor is re-initialized, log_mutex_initialized will still be 1. So we
   * won't trigger any undefined behaviour by trying to re-initialize the
   * log mutex. */
}

/** Flush the signal-safe log files.
 *
 * This function is safe to call from a signal handler. It is currently called
 * by the BUG() macros, when terminating the process on an abnormal condition.
 */
void
logs_flush_sigsafe(void)
{
  /* If we don't have fsync() in unistd.h, we can't flush the logs. */
#ifdef HAVE_FSYNC
  logfile_t *victim, *next;
  /* We can't LOCK_LOGS() in a signal handler, because it may call
   * signal-unsafe functions. And we can't deallocate memory, either. */
  next = logfiles;
  logfiles = NULL;
  while (next) {
    victim = next;
    next = next->next;
    if (victim->needs_close) {
      /* We can't do anything useful if the flush fails. */
      (void)fsync(victim->fd);
    }
  }
#endif /* defined(HAVE_FSYNC) */
}

/** Remove and free the log entry <b>victim</b> from the linked-list
 * logfiles (it is probably present, but it might not be due to thread
 * racing issues). After this function is called, the caller shouldn't
 * refer to <b>victim</b> anymore.
 */
static void
delete_log(logfile_t *victim)
{
  logfile_t *tmpl;
  if (victim == logfiles)
    logfiles = victim->next;
  else {
    for (tmpl = logfiles; tmpl && tmpl->next != victim; tmpl=tmpl->next) ;
//    raw_assert(tmpl);
//    raw_assert(tmpl->next == victim);
    if (!tmpl)
      return;
    tmpl->next = victim->next;
  }
  log_free(victim);
}

/** Helper: release system resources (but not memory) held by a single
 * signal-safe logfile_t. If the log's resources can not be released in
 * a signal handler, does nothing. */
static void
close_log_sigsafe(logfile_t *victim)
{
  if (victim->needs_close && victim->fd >= 0) {
    /* We can't do anything useful here if close() fails: we're shutting
     * down logging, and the err module only does fatal errors. */
    close(victim->fd);
    victim->fd = -1;
  }
}

/** Helper: release system resources (but not memory) held by a single
 * logfile_t. */
static void
close_log(logfile_t *victim)
{
  if (victim->needs_close) {
    close_log_sigsafe(victim);
  } else if (victim->is_syslog) {
#ifdef HAVE_SYSLOG_H
    if (--syslog_count == 0) {
      /* There are no other syslogs; close the logging facility. */
      closelog();
    }
#endif /* defined(HAVE_SYSLOG_H) */
  }
}

/** Adjust a log severity configuration in <b>severity_out</b> to contain
 * every domain between <b>loglevelMin</b> and <b>loglevelMax</b>, inclusive.
 */
void
set_log_severity_config(int loglevelMin, int loglevelMax,
                        log_severity_list_t *severity_out)
{
  int i;
  raw_assert(loglevelMin >= loglevelMax);
  raw_assert(loglevelMin >= LOG_ERR && loglevelMin <= LOG_DEBUG);
  raw_assert(loglevelMax >= LOG_ERR && loglevelMax <= LOG_DEBUG);
  memset(severity_out, 0, sizeof(log_severity_list_t));
  for (i = loglevelMin; i >= loglevelMax; --i) {
    severity_out->masks[SEVERITY_MASK_IDX(i)] = LD_ALL_DOMAINS;
  }
}

/** Add a log handler named <b>name</b> to send all messages in <b>severity</b>
 * to <b>fd</b>. Copies <b>severity</b>. Helper: does no locking. */
MOCK_IMPL(STATIC void,
add_stream_log_impl,(const log_severity_list_t *severity,
                     const char *name, int fd))
{
  logfile_t *lf;
  lf = tor_malloc_zero(sizeof(logfile_t));
  lf->fd = fd;
  lf->filename = tor_strdup(name);
  lf->severities = tor_memdup(severity, sizeof(log_severity_list_t));
  lf->next = logfiles;

  logfiles = lf;
  log_global_min_severity_ = get_min_log_level();
}

/** Add a log handler named <b>name</b> to send all messages in <b>severity</b>
 * to <b>fd</b>. Steals a reference to <b>severity</b>; the caller must
 * not use it after calling this function. */
void
add_stream_log(const log_severity_list_t *severity, const char *name, int fd)
{
  LOCK_LOGS();
  add_stream_log_impl(severity, name, fd);
  UNLOCK_LOGS();
}

/** Initialize the global logging facility */
void
init_logging(int disable_startup_queue)
{
  if (!log_mutex_initialized) {
    tor_mutex_init(&log_mutex);
    log_mutex_initialized = 1;
  }
#ifdef __GNUC__
  if (strchr(__PRETTY_FUNCTION__, '(')) {
    pretty_fn_has_parens = 1;
  }
#endif
  if (pending_cb_messages == NULL)
    pending_cb_messages = smartlist_new();
  if (disable_startup_queue)
    queue_startup_messages = 0;
  if (pending_startup_messages == NULL && queue_startup_messages) {
    pending_startup_messages = smartlist_new();
  }
}

/** Set whether we report logging domains as a part of our log messages.
 */
void
logs_set_domain_logging(int enabled)
{
  LOCK_LOGS();
  log_domains_are_logged = enabled;
  UNLOCK_LOGS();
}

/** Add a log handler to accept messages when no other log is configured.
 */
void
add_default_log(int min_severity)
{
  log_severity_list_t *s = tor_malloc_zero(sizeof(log_severity_list_t));
  set_log_severity_config(min_severity, LOG_ERR, s);
  LOCK_LOGS();
  add_stream_log_impl(s, "<default>", fileno(stdout));
  tor_free(s);
  UNLOCK_LOGS();
}

/**
 * Register "cb" as the callback to call when there are new pending log
 * callbacks to be flushed with flush_pending_log_callbacks().
 *
 * Note that this callback, if present, can be invoked from any thread.
 *
 * This callback must not log.
 *
 * It is intentional that this function contains the name "callback" twice: it
 * sets a "callback" to be called on the condition that there is a "pending
 * callback".
 **/
void
logs_set_pending_callback_callback(pending_callback_callback cb)
{
  pending_cb_cb = cb;
}

/**
 * Add a log handler to send messages in <b>severity</b>
 * to the function <b>cb</b>.
 */
int
add_callback_log(const log_severity_list_t *severity, log_callback cb)
{
  logfile_t *lf;
  lf = tor_malloc_zero(sizeof(logfile_t));
  lf->fd = -1;
  lf->severities = tor_memdup(severity, sizeof(log_severity_list_t));
  lf->filename = tor_strdup("<callback>");
  lf->callback = cb;
  lf->next = logfiles;

  LOCK_LOGS();
  logfiles = lf;
  log_global_min_severity_ = get_min_log_level();
  UNLOCK_LOGS();
  return 0;
}

/** Adjust the configured severity of any logs whose callback function is
 * <b>cb</b>. */
void
change_callback_log_severity(int loglevelMin, int loglevelMax,
                             log_callback cb)
{
  logfile_t *lf;
  log_severity_list_t severities;
  set_log_severity_config(loglevelMin, loglevelMax, &severities);
  LOCK_LOGS();
  for (lf = logfiles; lf; lf = lf->next) {
    if (lf->callback == cb) {
      memcpy(lf->severities, &severities, sizeof(severities));
    }
  }
  log_global_min_severity_ = get_min_log_level();
  UNLOCK_LOGS();
}

/** If there are any log messages that were generated with LD_NOCB waiting to
 * be sent to callback-based loggers, send them now. */
void
flush_pending_log_callbacks(void)
{
  logfile_t *lf;
  smartlist_t *messages, *messages_tmp;

  LOCK_LOGS();
  if (!pending_cb_messages || 0 == smartlist_len(pending_cb_messages)) {
    UNLOCK_LOGS();
    return;
  }

  messages = pending_cb_messages;
  pending_cb_messages = smartlist_new();
  do {
    SMARTLIST_FOREACH_BEGIN(messages, pending_log_message_t *, msg) {
      const int severity = msg->severity;
      const log_domain_mask_t domain = msg->domain;
      for (lf = logfiles; lf; lf = lf->next) {
        if (! lf->callback || lf->seems_dead ||
            ! (lf->severities->masks[SEVERITY_MASK_IDX(severity)] & domain)) {
          continue;
        }
        lf->callback(severity, domain, msg->msg);
      }
      pending_log_message_free(msg);
    } SMARTLIST_FOREACH_END(msg);
    smartlist_clear(messages);

    messages_tmp = pending_cb_messages;
    pending_cb_messages = messages;
    messages = messages_tmp;
  } while (smartlist_len(messages));

  smartlist_free(messages);

  UNLOCK_LOGS();
}

/** Flush all the messages we stored from startup while waiting for log
 * initialization.
 */
void
flush_log_messages_from_startup(void)
{
  logfile_t *lf;

  LOCK_LOGS();
  queue_startup_messages = 0;
  pending_startup_messages_len = 0;
  if (! pending_startup_messages)
    goto out;

  SMARTLIST_FOREACH_BEGIN(pending_startup_messages, pending_log_message_t *,
                          msg) {
    int callbacks_deferred = 0;
    for (lf = logfiles; lf; lf = lf->next) {
      if (! logfile_wants_message(lf, msg->severity, msg->domain))
        continue;

      /* We configure a temporary startup log that goes to stdout, so we
       * shouldn't replay to stdout/stderr*/
      if (lf->fd == STDOUT_FILENO || lf->fd == STDERR_FILENO) {
        continue;
      }

      logfile_deliver(lf, msg->fullmsg, strlen(msg->fullmsg), msg->msg,
                      msg->domain, msg->severity, &callbacks_deferred);
    }
    pending_log_message_free(msg);
  } SMARTLIST_FOREACH_END(msg);
  smartlist_free(pending_startup_messages);
  pending_startup_messages = NULL;

 out:
  UNLOCK_LOGS();
}

/** Close any log handlers marked by mark_logs_temp(). */
void
close_temp_logs(void)
{
  logfile_t *lf, **p;

  LOCK_LOGS();
  for (p = &logfiles; *p; ) {
    if ((*p)->is_temporary) {
      lf = *p;
      /* we use *p here to handle the edge case of the head of the list */
      *p = (*p)->next;
      close_log(lf);
      log_free(lf);
    } else {
      p = &((*p)->next);
    }
  }

  log_global_min_severity_ = get_min_log_level();
  UNLOCK_LOGS();
}

/** Make all currently temporary logs (set to be closed by close_temp_logs)
 * live again, and close all non-temporary logs. */
void
rollback_log_changes(void)
{
  logfile_t *lf;
  LOCK_LOGS();
  for (lf = logfiles; lf; lf = lf->next)
    lf->is_temporary = ! lf->is_temporary;
  UNLOCK_LOGS();
  close_temp_logs();
}

/** Configure all log handles to be closed by close_temp_logs(). */
void
mark_logs_temp(void)
{
  logfile_t *lf;
  LOCK_LOGS();
  for (lf = logfiles; lf; lf = lf->next)
    lf->is_temporary = 1;
  UNLOCK_LOGS();
}

/**
 * Add a log handler to send messages to <b>filename</b> via <b>fd</b>. If
 * opening the logfile failed, -1 is returned and errno is set appropriately
 * (by open(2)).  Takes ownership of fd.
 */
MOCK_IMPL(int,
add_file_log,(const log_severity_list_t *severity,
              const char *filename,
              int fd))
{
  logfile_t *lf;

  if (fd<0)
    return -1;
  if (tor_fd_seekend(fd)<0) {
    close(fd);
    return -1;
  }

  LOCK_LOGS();
  add_stream_log_impl(severity, filename, fd);
  logfiles->needs_close = 1;
  lf = logfiles;
  log_global_min_severity_ = get_min_log_level();

  if (log_tor_version(lf, 0) < 0) {
    delete_log(lf);
  }
  UNLOCK_LOGS();

  return 0;
}

#ifdef HAVE_SYSLOG_H
/**
 * Add a log handler to send messages to they system log facility.
 *
 * If this is the first log handler, opens syslog with ident Tor or
 * Tor-<syslog_identity_tag> if that is not NULL.
 */
int
add_syslog_log(const log_severity_list_t *severity,
               const char* syslog_identity_tag)
{
  logfile_t *lf;
  if (syslog_count++ == 0) {
    /* This is the first syslog. */
    static char buf[256];
    if (syslog_identity_tag) {
      tor_snprintf(buf, sizeof(buf), "Tor-%s", syslog_identity_tag);
    } else {
      tor_snprintf(buf, sizeof(buf), "Tor");
    }
    openlog(buf, LOG_PID | LOG_NDELAY, LOGFACILITY);
  }

  lf = tor_malloc_zero(sizeof(logfile_t));
  lf->fd = -1;
  lf->severities = tor_memdup(severity, sizeof(log_severity_list_t));
  lf->filename = tor_strdup("<syslog>");
  lf->is_syslog = 1;

  LOCK_LOGS();
  lf->next = logfiles;
  logfiles = lf;
  log_global_min_severity_ = get_min_log_level();
  UNLOCK_LOGS();
  return 0;
}
#endif /* defined(HAVE_SYSLOG_H) */

/** If <b>level</b> is a valid log severity, return the corresponding
 * numeric value.  Otherwise, return -1. */
int
parse_log_level(const char *level)
{
  if (!strcasecmp(level, "err"))
    return LOG_ERR;
  if (!strcasecmp(level, "warn"))
    return LOG_WARN;
  if (!strcasecmp(level, "notice"))
    return LOG_NOTICE;
  if (!strcasecmp(level, "info"))
    return LOG_INFO;
  if (!strcasecmp(level, "debug"))
    return LOG_DEBUG;
  return -1;
}

/** Return the string equivalent of a given log level. */
const char *
log_level_to_string(int level)
{
  return sev_to_string(level);
}

/** NULL-terminated array of names for log domains such that domain_list[dom]
 * is a description of <b>dom</b>.
 *
 * Remember to update doc/man/tor.1.txt if you modify this list.
 * */
static const char *domain_list[] = {
  "GENERAL", "CRYPTO", "NET", "CONFIG", "FS", "PROTOCOL", "MM",
  "HTTP", "APP", "CONTROL", "CIRC", "REND", "BUG", "DIR", "DIRSERV",
  "OR", "EDGE", "ACCT", "HIST", "HANDSHAKE", "HEARTBEAT", "CHANNEL",
  "SCHED", "GUARD", "CONSDIFF", "DOS", "PROCESS", "PT", "BTRACK", "MESG",
  NULL
};

CTASSERT(ARRAY_LENGTH(domain_list) == N_LOGGING_DOMAINS + 1);

CTASSERT(HIGHEST_RESERVED_LD_DOMAIN_ < LD_ALL_DOMAINS);
CTASSERT(LD_ALL_DOMAINS < LOWEST_RESERVED_LD_FLAG_);
CTASSERT(LOWEST_RESERVED_LD_FLAG_ < LD_ALL_FLAGS);

/** Return a bitmask for the log domain for which <b>domain</b> is the name,
 * or 0 if there is no such name. */
static log_domain_mask_t
parse_log_domain(const char *domain)
{
  int i;
  for (i=0; domain_list[i]; ++i) {
    if (!strcasecmp(domain, domain_list[i]))
      return (UINT64_C(1)<<i);
  }
  return 0;
}

/** Translate a bitmask of log domains to a string. */
static char *
domain_to_string(log_domain_mask_t domain, char *buf, size_t buflen)
{
  char *cp = buf;
  char *eos = buf+buflen;

  buf[0] = '\0';
  if (! domain)
    return buf;
  while (1) {
    const char *d;
    int bit = tor_log2(domain);
    size_t n;
    if ((unsigned)bit >= ARRAY_LENGTH(domain_list)-1 ||
        bit >= N_LOGGING_DOMAINS) {
      tor_snprintf(buf, buflen, "<BUG:Unknown domain %lx>", (long)domain);
      return buf+strlen(buf);
    }
    d = domain_list[bit];
    n = strlcpy(cp, d, eos-cp);
    if (n >= buflen) {
      tor_snprintf(buf, buflen, "<BUG:Truncating domain %lx>", (long)domain);
      return buf+strlen(buf);
    }
    cp += n;
    domain &= ~(1<<bit);

    if (domain == 0 || (eos-cp) < 2)
      return cp;

    memcpy(cp, ",", 2); /*Nul-terminated ,"*/
    cp++;
  }
}

/** Parse a log severity pattern in *<b>cfg_ptr</b>.  Advance cfg_ptr after
 * the end of the severityPattern.  Set the value of <b>severity_out</b> to
 * the parsed pattern.  Return 0 on success, -1 on failure.
 *
 * The syntax for a SeverityPattern is:
 * <pre>
 *   SeverityPattern = *(DomainSeverity SP)* DomainSeverity
 *   DomainSeverity = (DomainList SP)? SeverityRange
 *   SeverityRange = MinSeverity ("-" MaxSeverity )?
 *   DomainList = "[" (SP? DomainSpec SP? ",") SP? DomainSpec "]"
 *   DomainSpec = "*" | Domain | "~" Domain
 * </pre>
 * A missing MaxSeverity defaults to ERR.  Severities and domains are
 * case-insensitive.  "~" indicates negation for a domain; negation happens
 * last inside a DomainList.  Only one SeverityRange without a DomainList is
 * allowed per line.
 */
int
parse_log_severity_config(const char **cfg_ptr,
                          log_severity_list_t *severity_out)
{
  const char *cfg = *cfg_ptr;
  int got_anything = 0;
  int got_an_unqualified_range = 0;
  memset(severity_out, 0, sizeof(*severity_out));

  cfg = eat_whitespace(cfg);
  while (*cfg) {
    const char *dash, *space;
    char *sev_lo, *sev_hi;
    int low, high, i;
    log_domain_mask_t domains = LD_ALL_DOMAINS;

    if (*cfg == '[') {
      int err = 0;
      char *domains_str;
      smartlist_t *domains_list;
      log_domain_mask_t neg_domains = 0;
      const char *closebracket = strchr(cfg, ']');
      if (!closebracket)
        return -1;
      domains = 0;
      domains_str = tor_strndup(cfg+1, closebracket-cfg-1);
      domains_list = smartlist_new();
      smartlist_split_string(domains_list, domains_str, ",", SPLIT_SKIP_SPACE,
                             -1);
      tor_free(domains_str);
      SMARTLIST_FOREACH_BEGIN(domains_list, const char *, domain) {
            if (!strcmp(domain, "*")) {
              domains = LD_ALL_DOMAINS;
            } else {
              log_domain_mask_t d;
              int negate=0;
              if (*domain == '~') {
                negate = 1;
                ++domain;
              }
              d = parse_log_domain(domain);
              if (!d) {
                log_warn(LD_CONFIG, "No such logging domain as %s", domain);
                err = 1;
              } else {
                if (negate)
                  neg_domains |= d;
                else
                  domains |= d;
              }
            }
      } SMARTLIST_FOREACH_END(domain);
      SMARTLIST_FOREACH(domains_list, char *, d, tor_free(d));
      smartlist_free(domains_list);
      if (err)
        return -1;
      if (domains == 0 && neg_domains)
        domains = ~neg_domains;
      else
        domains &= ~neg_domains;
      cfg = eat_whitespace(closebracket+1);
    } else {
      ++got_an_unqualified_range;
    }
    if (!strcasecmpstart(cfg, "file") ||
        !strcasecmpstart(cfg, "stderr") ||
        !strcasecmpstart(cfg, "stdout") ||
        !strcasecmpstart(cfg, "syslog")) {
      goto done;
    }
    if (got_an_unqualified_range > 1)
      return -1;

    space = find_whitespace(cfg);
    dash = strchr(cfg, '-');
    if (dash && dash < space) {
      sev_lo = tor_strndup(cfg, dash-cfg);
      sev_hi = tor_strndup(dash+1, space-(dash+1));
    } else {
      sev_lo = tor_strndup(cfg, space-cfg);
      sev_hi = tor_strdup("ERR");
    }
    low = parse_log_level(sev_lo);
    high = parse_log_level(sev_hi);
    tor_free(sev_lo);
    tor_free(sev_hi);
    if (low == -1)
      return -1;
    if (high == -1)
      return -1;

    got_anything = 1;
    for (i=low; i >= high; --i)
      severity_out->masks[SEVERITY_MASK_IDX(i)] |= domains;

    cfg = eat_whitespace(space);
  }

 done:
  *cfg_ptr = cfg;
  return got_anything ? 0 : -1;
}

/** Return the least severe log level that any current log is interested in. */
int
get_min_log_level(void)
{
  logfile_t *lf;
  int i;
  int min = LOG_ERR;
  for (lf = logfiles; lf; lf = lf->next) {
    for (i = LOG_DEBUG; i > min; --i)
      if (lf->severities->masks[SEVERITY_MASK_IDX(i)])
        min = i;
  }
  return min;
}

/** Switch all logs to output at most verbose level. */
void
switch_logs_debug(void)
{
  logfile_t *lf;
  int i;
  LOCK_LOGS();
  for (lf = logfiles; lf; lf=lf->next) {
    for (i = LOG_DEBUG; i >= LOG_ERR; --i)
      lf->severities->masks[SEVERITY_MASK_IDX(i)] = LD_ALL_DOMAINS;
  }
  log_global_min_severity_ = get_min_log_level();
  UNLOCK_LOGS();
}

/** Truncate all the log files. */
void
truncate_logs(void)
{
  logfile_t *lf;
  for (lf = logfiles; lf; lf = lf->next) {
    if (lf->fd >= 0) {
      tor_ftruncate(lf->fd);
    }
  }
}
