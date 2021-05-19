/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_bootstrap.c
 * \brief Provide bootstrap progress events for the control port.
 */
#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_or.h"
#include "core/or/connection_st.h"
#include "core/or/or_connection_st.h"
#include "core/or/reasons.h"
#include "feature/control/control_events.h"
#include "feature/hibernate/hibernate.h"
#include "lib/malloc/malloc.h"

/** A sufficiently large size to record the last bootstrap phase string. */
#define BOOTSTRAP_MSG_LEN 1024

/** What was the last bootstrap phase message we sent? We keep track
 * of this so we can respond to getinfo status/bootstrap-phase queries. */
static char last_sent_bootstrap_message[BOOTSTRAP_MSG_LEN];

/** Table to convert bootstrap statuses to strings.  */
static const struct {
  bootstrap_status_t status;
  const char *tag;
  const char *summary;
} boot_to_str_tab[] = {
  { BOOTSTRAP_STATUS_UNDEF, "undef", "Undefined" },
  { BOOTSTRAP_STATUS_STARTING, "starting", "Starting" },

  /* Initial connection to any relay */

  { BOOTSTRAP_STATUS_CONN_PT, "conn_pt", "Connecting to pluggable transport" },
  { BOOTSTRAP_STATUS_CONN_DONE_PT, "conn_done_pt",
    "Connected to pluggable transport" },
  { BOOTSTRAP_STATUS_CONN_PROXY, "conn_proxy", "Connecting to proxy" },
  { BOOTSTRAP_STATUS_CONN_DONE_PROXY, "conn_done_proxy",
    "Connected to proxy" },
  { BOOTSTRAP_STATUS_CONN, "conn", "Connecting to a relay" },
  { BOOTSTRAP_STATUS_CONN_DONE, "conn_done", "Connected to a relay" },
  { BOOTSTRAP_STATUS_HANDSHAKE, "handshake",
    "Handshaking with a relay" },
  { BOOTSTRAP_STATUS_HANDSHAKE_DONE, "handshake_done",
    "Handshake with a relay done" },

  /* Loading directory info */

  { BOOTSTRAP_STATUS_ONEHOP_CREATE, "onehop_create",
    "Establishing an encrypted directory connection" },
  { BOOTSTRAP_STATUS_REQUESTING_STATUS, "requesting_status",
    "Asking for networkstatus consensus" },
  { BOOTSTRAP_STATUS_LOADING_STATUS, "loading_status",
    "Loading networkstatus consensus" },
  { BOOTSTRAP_STATUS_LOADING_KEYS, "loading_keys",
    "Loading authority key certs" },
  { BOOTSTRAP_STATUS_REQUESTING_DESCRIPTORS, "requesting_descriptors",
    "Asking for relay descriptors" },
  { BOOTSTRAP_STATUS_LOADING_DESCRIPTORS, "loading_descriptors",
    "Loading relay descriptors" },
  { BOOTSTRAP_STATUS_ENOUGH_DIRINFO, "enough_dirinfo",
    "Loaded enough directory info to build circuits" },

  /* Connecting to a relay for AP circuits */

  { BOOTSTRAP_STATUS_AP_CONN_PT, "ap_conn_pt",
    "Connecting to pluggable transport to build circuits" },
  { BOOTSTRAP_STATUS_AP_CONN_DONE_PT, "ap_conn_done_pt",
    "Connected to pluggable transport to build circuits" },
  { BOOTSTRAP_STATUS_AP_CONN_PROXY, "ap_conn_proxy",
    "Connecting to proxy to build circuits" },
  { BOOTSTRAP_STATUS_AP_CONN_DONE_PROXY, "ap_conn_done_proxy",
    "Connected to proxy to build circuits" },
  { BOOTSTRAP_STATUS_AP_CONN, "ap_conn",
    "Connecting to a relay to build circuits" },
  { BOOTSTRAP_STATUS_AP_CONN_DONE, "ap_conn_done",
    "Connected to a relay to build circuits" },
  { BOOTSTRAP_STATUS_AP_HANDSHAKE, "ap_handshake",
    "Finishing handshake with a relay to build circuits" },
  { BOOTSTRAP_STATUS_AP_HANDSHAKE_DONE, "ap_handshake_done",
    "Handshake finished with a relay to build circuits" },

  /* Creating AP circuits */

  { BOOTSTRAP_STATUS_CIRCUIT_CREATE, "circuit_create",
    "Establishing a Tor circuit" },
  { BOOTSTRAP_STATUS_DONE, "done", "Done" },
};
#define N_BOOT_TO_STR (sizeof(boot_to_str_tab)/sizeof(boot_to_str_tab[0]))

/** Convert the name of a bootstrapping phase <b>s</b> into strings
 * <b>tag</b> and <b>summary</b> suitable for display by the controller. */
static int
bootstrap_status_to_string(bootstrap_status_t s, const char **tag,
                           const char **summary)
{
  for (size_t i = 0; i < N_BOOT_TO_STR; i++) {
    if (s == boot_to_str_tab[i].status) {
      *tag = boot_to_str_tab[i].tag;
      *summary = boot_to_str_tab[i].summary;
      return 0;
    }
  }

  *tag = *summary = "unknown";
  return -1;
}

/** What percentage through the bootstrap process are we? We remember
 * this so we can avoid sending redundant bootstrap status events, and
 * so we can guess context for the bootstrap messages which are
 * ambiguous. It starts at 'undef', but gets set to 'starting' while
 * Tor initializes. */
static int bootstrap_percent = BOOTSTRAP_STATUS_UNDEF;

/** Like bootstrap_percent, but only takes on the enumerated values in
 * bootstrap_status_t.
 */
static int bootstrap_phase = BOOTSTRAP_STATUS_UNDEF;

/** As bootstrap_percent, but holds the bootstrapping level at which we last
 * logged a NOTICE-level message. We use this, plus BOOTSTRAP_PCT_INCREMENT,
 * to avoid flooding the log with a new message every time we get a few more
 * microdescriptors */
static int notice_bootstrap_percent = 0;

/** How many problems have we had getting to the next bootstrapping phase?
 * These include failure to establish a connection to a Tor relay,
 * failures to finish the TLS handshake, failures to validate the
 * consensus document, etc. */
static int bootstrap_problems = 0;

/** We only tell the controller once we've hit a threshold of problems
 * for the current phase. */
#define BOOTSTRAP_PROBLEM_THRESHOLD 10

/** When our bootstrapping progress level changes, but our bootstrapping
 * status has not advanced, we only log at NOTICE when we have made at least
 * this much progress.
 */
#define BOOTSTRAP_PCT_INCREMENT 5

/** Do the actual logging and notifications for
 * control_event_bootstrap().  Doesn't change any state beyond that.
 */
static void
control_event_bootstrap_core(int loglevel, bootstrap_status_t status,
                             int progress)
{
  char buf[BOOTSTRAP_MSG_LEN];
  const char *tag, *summary;

  bootstrap_status_to_string(status, &tag, &summary);
  /* Locally reset status if there's incremental progress */
  if (progress)
    status = progress;

  tor_log(loglevel, LD_CONTROL,
          "Bootstrapped %d%% (%s): %s", status, tag, summary);
  tor_snprintf(buf, sizeof(buf),
               "BOOTSTRAP PROGRESS=%d TAG=%s SUMMARY=\"%s\"",
               status, tag, summary);
  tor_snprintf(last_sent_bootstrap_message,
               sizeof(last_sent_bootstrap_message),
               "NOTICE %s", buf);
  control_event_client_status(LOG_NOTICE, "%s", buf);
}

int
control_get_bootstrap_percent(void)
{
  return bootstrap_percent;
}

/** Called when Tor has made progress at bootstrapping its directory
 * information and initial circuits.
 *
 * <b>status</b> is the new status, that is, what task we will be doing
 * next. <b>progress</b> is zero if we just started this task, else it
 * represents progress on the task.
 */
void
control_event_bootstrap(bootstrap_status_t status, int progress)
{
  int loglevel = LOG_NOTICE;

  if (bootstrap_percent == BOOTSTRAP_STATUS_DONE)
    return; /* already bootstrapped; nothing to be done here. */

  if (status <= bootstrap_percent) {
    /* If there's no new progress, return early. */
    if (!progress || progress <= bootstrap_percent)
      return;
    /* Log at INFO if not enough progress happened. */
    if (progress < notice_bootstrap_percent + BOOTSTRAP_PCT_INCREMENT)
      loglevel = LOG_INFO;
  }

  control_event_bootstrap_core(loglevel, status, progress);

  if (status > bootstrap_percent) {
    bootstrap_phase = status; /* new milestone reached */
    bootstrap_percent = status;
  }
  if (progress > bootstrap_percent) {
    /* incremental progress within a milestone */
    bootstrap_percent = progress;
    bootstrap_problems = 0; /* Progress! Reset our problem counter. */
  }
  if (loglevel == LOG_NOTICE &&
      bootstrap_percent > notice_bootstrap_percent) {
    /* Remember that we gave a notice at this level. */
    notice_bootstrap_percent = bootstrap_percent;
  }
}

/** Flag whether we've opened an OR_CONN yet  */
static int bootstrap_first_orconn = 0;

/** Like bootstrap_phase, but for (possibly deferred) directory progress */
static int bootstrap_dir_phase = BOOTSTRAP_STATUS_UNDEF;

/** Like bootstrap_problems, but for (possibly deferred) directory progress  */
static int bootstrap_dir_progress = BOOTSTRAP_STATUS_UNDEF;

/** Defer directory info bootstrap events until we have successfully
 * completed our first connection to a router.  */
void
control_event_boot_dir(bootstrap_status_t status, int progress)
{
  if (status > bootstrap_dir_progress) {
    bootstrap_dir_progress = status;
    bootstrap_dir_phase = status;
  }
  if (progress && progress >= bootstrap_dir_progress) {
    bootstrap_dir_progress = progress;
  }

  /* Don't report unless we have successfully opened at least one OR_CONN */
  if (!bootstrap_first_orconn)
    return;

  control_event_bootstrap(status, progress);
}

/** Set a flag to allow reporting of directory bootstrap progress.
 * (Code that reports completion of an OR_CONN calls this.)  Also,
 * report directory progress so far. */
void
control_event_boot_first_orconn(void)
{
  bootstrap_first_orconn = 1;
  control_event_bootstrap(bootstrap_dir_phase, bootstrap_dir_progress);
}

/** Called when Tor has failed to make bootstrapping progress in a way
 * that indicates a problem. <b>warn</b> gives a human-readable hint
 * as to why, and <b>reason</b> provides a controller-facing short
 * tag.  <b>conn</b> is the connection that caused this problem and
 * can be NULL if a connection cannot be easily identified.
 */
void
control_event_bootstrap_problem(const char *warn, const char *reason,
                                const connection_t *conn, int dowarn)
{
  int status = bootstrap_percent;
  const char *tag = "", *summary = "";
  char buf[BOOTSTRAP_MSG_LEN];
  const char *recommendation = "ignore";
  int severity;
  char *or_id = NULL, *hostaddr = NULL;
  const or_connection_t *or_conn = NULL;

  /* bootstrap_percent must not be in "undefined" state here. */
  tor_assert(status >= 0);

  if (bootstrap_percent == 100)
    return; /* already bootstrapped; nothing to be done here. */

  bootstrap_problems++;

  if (bootstrap_problems >= BOOTSTRAP_PROBLEM_THRESHOLD)
    dowarn = 1;

  /* Don't warn about our bootstrapping status if we are hibernating or
   * shutting down. */
  if (we_are_hibernating())
    dowarn = 0;

  tor_assert(bootstrap_status_to_string(bootstrap_phase, &tag, &summary) == 0);

  severity = dowarn ? LOG_WARN : LOG_INFO;

  if (dowarn)
    recommendation = "warn";

  if (conn && conn->type == CONN_TYPE_OR) {
    /* XXX TO_OR_CONN can't deal with const */
    or_conn = CONST_TO_OR_CONN(conn);
    or_id = tor_strdup(hex_str(or_conn->identity_digest, DIGEST_LEN));
  } else {
    or_id = tor_strdup("?");
  }

  if (conn)
    tor_asprintf(&hostaddr, "%s:%d", conn->address, (int)conn->port);
  else
    hostaddr = tor_strdup("?");

  log_fn(severity,
         LD_CONTROL, "Problem bootstrapping. Stuck at %d%% (%s): %s. (%s; %s; "
         "count %d; recommendation %s; host %s at %s)",
         status, tag, summary, warn, reason,
         bootstrap_problems, recommendation,
         or_id, hostaddr);

  connection_or_report_broken_states(severity, LD_HANDSHAKE);

  tor_snprintf(buf, sizeof(buf),
      "BOOTSTRAP PROGRESS=%d TAG=%s SUMMARY=\"%s\" WARNING=\"%s\" REASON=%s "
      "COUNT=%d RECOMMENDATION=%s HOSTID=\"%s\" HOSTADDR=\"%s\"",
      bootstrap_percent, tag, summary, warn, reason, bootstrap_problems,
      recommendation,
      or_id, hostaddr);

  tor_snprintf(last_sent_bootstrap_message,
               sizeof(last_sent_bootstrap_message),
               "WARN %s", buf);
  control_event_client_status(LOG_WARN, "%s", buf);

  tor_free(hostaddr);
  tor_free(or_id);
}

/** Called when Tor has failed to make bootstrapping progress in a way
 * that indicates a problem. <b>warn</b> gives a hint as to why, and
 * <b>reason</b> provides an "or_conn_end_reason" tag.  <b>or_conn</b>
 * is the connection that caused this problem.
 */
MOCK_IMPL(void,
control_event_bootstrap_prob_or, (const char *warn, int reason,
                                  or_connection_t *or_conn))
{
  int dowarn = 0;

  if (! or_conn->potentially_used_for_bootstrapping) {
    /* We never decided that this channel was a good match for one of our
     * origin_circuit_t objects.  That means that we probably launched it
     * for somebody else, most likely in response to an EXTEND cell.
     *
     * Since EXTEND cells can contain arbitrarily broken descriptions of
     * relays, a failure on this connection here won't necessarily indicate a
     * bootstrapping problem.
     */
    return;
  }

  if (or_conn->have_noted_bootstrap_problem)
    return;

  or_conn->have_noted_bootstrap_problem = 1;

  if (reason == END_OR_CONN_REASON_NO_ROUTE)
    dowarn = 1;

  /* If we are using bridges and all our OR connections are now
     closed, it means that we totally failed to connect to our
     bridges. Throw a warning. */
  if (get_options()->UseBridges && !any_other_active_or_conns(or_conn))
    dowarn = 1;

  control_event_bootstrap_problem(warn,
                                  orconn_end_reason_to_control_string(reason),
                                  TO_CONN(or_conn), dowarn);
}

/** Return a copy of the last sent bootstrap message. */
char *
control_event_boot_last_msg(void)
{
  return tor_strdup(last_sent_bootstrap_message);
}

/** Reset bootstrap tracking state. */
void
control_event_bootstrap_reset(void)
{
  bootstrap_percent = BOOTSTRAP_STATUS_UNDEF;
  bootstrap_phase = BOOTSTRAP_STATUS_UNDEF;
  notice_bootstrap_percent = 0;
  bootstrap_problems = 0;
  bootstrap_first_orconn = 0;
  bootstrap_dir_progress = BOOTSTRAP_STATUS_UNDEF;
  bootstrap_dir_phase = BOOTSTRAP_STATUS_UNDEF;
  memset(last_sent_bootstrap_message, 0, sizeof(last_sent_bootstrap_message));
}
