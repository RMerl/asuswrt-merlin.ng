/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_events.c
 * \brief Implement the event-reporting part of the controller API.
 **/

#define CONTROL_MODULE_PRIVATE
#define CONTROL_EVENTS_PRIVATE
#define OCIRC_EVENT_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/channeltls.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitstats.h"
#include "core/or/command.h"
#include "core/or/connection_edge.h"
#include "core/or/connection_or.h"
#include "core/or/reasons.h"
#include "feature/control/control.h"
#include "feature/control/control_events.h"
#include "feature/control/control_fmt.h"
#include "feature/control/control_proto.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"

#include "feature/control/control_connection_st.h"
#include "core/or/entry_connection_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "core/or/or_connection_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

#include "lib/evloop/compat_libevent.h"
#include "lib/encoding/confline.h"

static void flush_queued_events_cb(mainloop_event_t *event, void *arg);
static void control_get_bytes_rw_last_sec(uint64_t *r, uint64_t *w);

/** Yield true iff <b>s</b> is the state of a control_connection_t that has
 * finished authentication and is accepting commands. */
#define STATE_IS_OPEN(s) ((s) == CONTROL_CONN_STATE_OPEN)

/** An event mask of all the events that any controller is interested in
 * receiving. */
static event_mask_t global_event_mask = 0;

/** True iff we have disabled log messages from being sent to the controller */
static int disable_log_messages = 0;

/** Macro: true if any control connection is interested in events of type
 * <b>e</b>. */
#define EVENT_IS_INTERESTING(e) \
  (!! (global_event_mask & EVENT_MASK_(e)))

/** Macro: true if any event from the bitfield 'e' is interesting. */
#define ANY_EVENT_IS_INTERESTING(e) \
  (!! (global_event_mask & (e)))

static void send_control_event_impl(uint16_t event,
                                    const char *format, va_list ap)
  CHECK_PRINTF(2,0);
static int control_event_status(int type, int severity, const char *format,
                                va_list args)
  CHECK_PRINTF(3,0);

static void send_control_event(uint16_t event,
                               const char *format, ...)
  CHECK_PRINTF(2,3);

/** Table mapping event values to their names.  Used to implement SETEVENTS
 * and GETINFO events/names, and to keep they in sync. */
const struct control_event_t control_event_table[] = {
  { EVENT_CIRCUIT_STATUS, "CIRC" },
  { EVENT_CIRCUIT_STATUS_MINOR, "CIRC_MINOR" },
  { EVENT_STREAM_STATUS, "STREAM" },
  { EVENT_OR_CONN_STATUS, "ORCONN" },
  { EVENT_BANDWIDTH_USED, "BW" },
  { EVENT_DEBUG_MSG, "DEBUG" },
  { EVENT_INFO_MSG, "INFO" },
  { EVENT_NOTICE_MSG, "NOTICE" },
  { EVENT_WARN_MSG, "WARN" },
  { EVENT_ERR_MSG, "ERR" },
  { EVENT_NEW_DESC, "NEWDESC" },
  { EVENT_ADDRMAP, "ADDRMAP" },
  { EVENT_DESCCHANGED, "DESCCHANGED" },
  { EVENT_NS, "NS" },
  { EVENT_STATUS_GENERAL, "STATUS_GENERAL" },
  { EVENT_STATUS_CLIENT, "STATUS_CLIENT" },
  { EVENT_STATUS_SERVER, "STATUS_SERVER" },
  { EVENT_GUARD, "GUARD" },
  { EVENT_STREAM_BANDWIDTH_USED, "STREAM_BW" },
  { EVENT_CLIENTS_SEEN, "CLIENTS_SEEN" },
  { EVENT_NEWCONSENSUS, "NEWCONSENSUS" },
  { EVENT_BUILDTIMEOUT_SET, "BUILDTIMEOUT_SET" },
  { EVENT_GOT_SIGNAL, "SIGNAL" },
  { EVENT_CONF_CHANGED, "CONF_CHANGED"},
  { EVENT_CONN_BW, "CONN_BW" },
  { EVENT_CELL_STATS, "CELL_STATS" },
  { EVENT_CIRC_BANDWIDTH_USED, "CIRC_BW" },
  { EVENT_TRANSPORT_LAUNCHED, "TRANSPORT_LAUNCHED" },
  { EVENT_HS_DESC, "HS_DESC" },
  { EVENT_HS_DESC_CONTENT, "HS_DESC_CONTENT" },
  { EVENT_NETWORK_LIVENESS, "NETWORK_LIVENESS" },
  { 0, NULL },
};

/** Given a log severity, return the corresponding control event code. */
static inline int
log_severity_to_event(int severity)
{
  switch (severity) {
    case LOG_DEBUG: return EVENT_DEBUG_MSG;
    case LOG_INFO: return EVENT_INFO_MSG;
    case LOG_NOTICE: return EVENT_NOTICE_MSG;
    case LOG_WARN: return EVENT_WARN_MSG;
    case LOG_ERR: return EVENT_ERR_MSG;
    default: return -1;
  }
}

/** Helper: clear bandwidth counters of all origin circuits. */
static void
clear_circ_bw_fields(void)
{
  origin_circuit_t *ocirc;
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    if (!CIRCUIT_IS_ORIGIN(circ))
      continue;
    ocirc = TO_ORIGIN_CIRCUIT(circ);
    ocirc->n_written_circ_bw = ocirc->n_read_circ_bw = 0;
    ocirc->n_overhead_written_circ_bw = ocirc->n_overhead_read_circ_bw = 0;
    ocirc->n_delivered_written_circ_bw = ocirc->n_delivered_read_circ_bw = 0;
  }
  SMARTLIST_FOREACH_END(circ);
}

/* Helper to emit the BUILDTIMEOUT_SET circuit build time event */
void
cbt_control_event_buildtimeout_set(const circuit_build_times_t *cbt,
                                   buildtimeout_set_event_t type)
{
  char *args = NULL;
  double qnt;
  double timeout_rate = 0.0;
  double close_rate = 0.0;

  switch (type) {
    case BUILDTIMEOUT_SET_EVENT_RESET:
    case BUILDTIMEOUT_SET_EVENT_SUSPENDED:
    case BUILDTIMEOUT_SET_EVENT_DISCARD:
      qnt = 1.0;
      break;
    case BUILDTIMEOUT_SET_EVENT_COMPUTED:
    case BUILDTIMEOUT_SET_EVENT_RESUME:
    default:
      qnt = circuit_build_times_quantile_cutoff();
      break;
  }

  /* The timeout rate is the ratio of the timeout count over
   * the total number of circuits attempted. The total number of
   * circuits is (timeouts+succeeded), since every circuit
   * either succeeds, or times out. "Closed" circuits are
   * MEASURE_TIMEOUT circuits whose measurement period expired.
   * All MEASURE_TIMEOUT circuits are counted in the timeouts stat
   * before transitioning to MEASURE_TIMEOUT (in
   * circuit_build_times_mark_circ_as_measurement_only()).
   * MEASURE_TIMEOUT circuits that succeed are *not* counted as
   * "succeeded". See circuit_build_times_handle_completed_hop().
   *
   * We cast the denominator
   * to promote it to double before the addition, to avoid int32
   * overflow. */
  const double total_circuits =
    ((double)cbt->num_circ_timeouts) + cbt->num_circ_succeeded;
  if (total_circuits >= 1.0) {
    timeout_rate = cbt->num_circ_timeouts / total_circuits;
    close_rate = cbt->num_circ_closed / total_circuits;
  }

  tor_asprintf(&args, "TOTAL_TIMES=%lu "
               "TIMEOUT_MS=%lu XM=%lu ALPHA=%f CUTOFF_QUANTILE=%f "
               "TIMEOUT_RATE=%f CLOSE_MS=%lu CLOSE_RATE=%f",
               (unsigned long)cbt->total_build_times,
               (unsigned long)cbt->timeout_ms,
               (unsigned long)cbt->Xm, cbt->alpha, qnt,
               timeout_rate,
               (unsigned long)cbt->close_ms,
               close_rate);

  control_event_buildtimeout_set(type, args);

  tor_free(args);
}
/** Set <b>global_event_mask*</b> to the bitwise OR of each live control
 * connection's event_mask field. */
void
control_update_global_event_mask(void)
{
  smartlist_t *conns = get_connection_array();
  event_mask_t old_mask, new_mask;
  old_mask = global_event_mask;
  int any_old_per_sec_events = control_any_per_second_event_enabled();

  global_event_mask = 0;
  SMARTLIST_FOREACH(conns, connection_t *, _conn,
  {
    if (_conn->type == CONN_TYPE_CONTROL &&
        STATE_IS_OPEN(_conn->state)) {
      control_connection_t *conn = TO_CONTROL_CONN(_conn);
      global_event_mask |= conn->event_mask;
    }
  });

  new_mask = global_event_mask;

  /* Handle the aftermath.  Set up the log callback to tell us only what
   * we want to hear...*/
  control_adjust_event_log_severity();

  /* Macro: true if ev was false before and is true now. */
#define NEWLY_ENABLED(ev) \
  (! (old_mask & (ev)) && (new_mask & (ev)))

  /* ...then, if we've started logging stream or circ bw, clear the
   * appropriate fields. */
  if (NEWLY_ENABLED(EVENT_STREAM_BANDWIDTH_USED)) {
    SMARTLIST_FOREACH(conns, connection_t *, conn,
    {
      if (conn->type == CONN_TYPE_AP) {
        edge_connection_t *edge_conn = TO_EDGE_CONN(conn);
        edge_conn->n_written = edge_conn->n_read = 0;
      }
    });
  }
  if (NEWLY_ENABLED(EVENT_CIRC_BANDWIDTH_USED)) {
    clear_circ_bw_fields();
  }
  if (NEWLY_ENABLED(EVENT_BANDWIDTH_USED)) {
    uint64_t r, w;
    control_get_bytes_rw_last_sec(&r, &w);
  }
  if (any_old_per_sec_events != control_any_per_second_event_enabled()) {
    rescan_periodic_events(get_options());
  }

#undef NEWLY_ENABLED
}

/** Given a control event code for a message event, return the corresponding
 * log severity. */
static inline int
event_to_log_severity(int event)
{
  switch (event) {
    case EVENT_DEBUG_MSG: return LOG_DEBUG;
    case EVENT_INFO_MSG: return LOG_INFO;
    case EVENT_NOTICE_MSG: return LOG_NOTICE;
    case EVENT_WARN_MSG: return LOG_WARN;
    case EVENT_ERR_MSG: return LOG_ERR;
    default: return -1;
  }
}

/** Adjust the log severities that result in control_event_logmsg being called
 * to match the severity of log messages that any controllers are interested
 * in. */
void
control_adjust_event_log_severity(void)
{
  int i;
  int min_log_event=EVENT_ERR_MSG, max_log_event=EVENT_DEBUG_MSG;

  for (i = EVENT_DEBUG_MSG; i <= EVENT_ERR_MSG; ++i) {
    if (EVENT_IS_INTERESTING(i)) {
      min_log_event = i;
      break;
    }
  }
  for (i = EVENT_ERR_MSG; i >= EVENT_DEBUG_MSG; --i) {
    if (EVENT_IS_INTERESTING(i)) {
      max_log_event = i;
      break;
    }
  }
  if (EVENT_IS_INTERESTING(EVENT_STATUS_GENERAL)) {
    if (min_log_event > EVENT_NOTICE_MSG)
      min_log_event = EVENT_NOTICE_MSG;
    if (max_log_event < EVENT_ERR_MSG)
      max_log_event = EVENT_ERR_MSG;
  }
  if (min_log_event <= max_log_event)
    change_callback_log_severity(event_to_log_severity(min_log_event),
                                 event_to_log_severity(max_log_event),
                                 control_event_logmsg);
  else
    change_callback_log_severity(LOG_ERR, LOG_ERR,
                                 control_event_logmsg);
}

/** Return true iff the event with code <b>c</b> is being sent to any current
 * control connection.  This is useful if the amount of work needed to prepare
 * to call the appropriate control_event_...() function is high.
 */
int
control_event_is_interesting(int event)
{
  return EVENT_IS_INTERESTING(event);
}

/** Return true if any event that needs to fire once a second is enabled. */
int
control_any_per_second_event_enabled(void)
{
  return ANY_EVENT_IS_INTERESTING(
      EVENT_MASK_(EVENT_BANDWIDTH_USED) |
      EVENT_MASK_(EVENT_CELL_STATS) |
      EVENT_MASK_(EVENT_CIRC_BANDWIDTH_USED) |
      EVENT_MASK_(EVENT_CONN_BW) |
      EVENT_MASK_(EVENT_STREAM_BANDWIDTH_USED)
  );
}

/* The value of 'get_bytes_read()' the previous time that
 * control_get_bytes_rw_last_sec() as called. */
static uint64_t stats_prev_n_read = 0;
/* The value of 'get_bytes_written()' the previous time that
 * control_get_bytes_rw_last_sec() as called. */
static uint64_t stats_prev_n_written = 0;

/**
 * Set <b>n_read</b> and <b>n_written</b> to the total number of bytes read
 * and written by Tor since the last call to this function.
 *
 * Call this only from the main thread.
 */
static void
control_get_bytes_rw_last_sec(uint64_t *n_read,
                              uint64_t *n_written)
{
  const uint64_t stats_n_bytes_read = get_bytes_read();
  const uint64_t stats_n_bytes_written = get_bytes_written();

  *n_read = stats_n_bytes_read - stats_prev_n_read;
  *n_written = stats_n_bytes_written - stats_prev_n_written;
  stats_prev_n_read = stats_n_bytes_read;
  stats_prev_n_written = stats_n_bytes_written;
}

/**
 * Run all the controller events (if any) that are scheduled to trigger once
 * per second.
 */
void
control_per_second_events(void)
{
  if (!control_any_per_second_event_enabled())
    return;

  uint64_t bytes_read, bytes_written;
  control_get_bytes_rw_last_sec(&bytes_read, &bytes_written);
  control_event_bandwidth_used((uint32_t)bytes_read,(uint32_t)bytes_written);

  control_event_stream_bandwidth_used();
  control_event_conn_bandwidth_used();
  control_event_circ_bandwidth_used();
  control_event_circuit_cell_stats();
}

/** Represents an event that's queued to be sent to one or more
 * controllers. */
typedef struct queued_event_t {
  uint16_t event;
  char *msg;
} queued_event_t;

/** Pointer to int. If this is greater than 0, we don't allow new events to be
 * queued. */
static tor_threadlocal_t block_event_queue_flag;

/** Holds a smartlist of queued_event_t objects that may need to be sent
 * to one or more controllers */
static smartlist_t *queued_control_events = NULL;

/** True if the flush_queued_events_event is pending. */
static int flush_queued_event_pending = 0;

/** Lock to protect the above fields. */
static tor_mutex_t *queued_control_events_lock = NULL;

/** An event that should fire in order to flush the contents of
 * queued_control_events. */
static mainloop_event_t *flush_queued_events_event = NULL;

void
control_initialize_event_queue(void)
{
  if (queued_control_events == NULL) {
    queued_control_events = smartlist_new();
  }

  if (flush_queued_events_event == NULL) {
    struct event_base *b = tor_libevent_get_base();
    if (b) {
      flush_queued_events_event =
        mainloop_event_new(flush_queued_events_cb, NULL);
      tor_assert(flush_queued_events_event);
    }
  }

  if (queued_control_events_lock == NULL) {
    queued_control_events_lock = tor_mutex_new();
    tor_threadlocal_init(&block_event_queue_flag);
  }
}

static int *
get_block_event_queue(void)
{
  int *val = tor_threadlocal_get(&block_event_queue_flag);
  if (PREDICT_UNLIKELY(val == NULL)) {
    val = tor_malloc_zero(sizeof(int));
    tor_threadlocal_set(&block_event_queue_flag, val);
  }
  return val;
}

/** Helper: inserts an event on the list of events queued to be sent to
 * one or more controllers, and schedules the events to be flushed if needed.
 *
 * This function takes ownership of <b>msg</b>, and may free it.
 *
 * We queue these events rather than send them immediately in order to break
 * the dependency in our callgraph from code that generates events for the
 * controller, and the network layer at large.  Otherwise, nearly every
 * interesting part of Tor would potentially call every other interesting part
 * of Tor.
 */
MOCK_IMPL(STATIC void,
queue_control_event_string,(uint16_t event, char *msg))
{
  /* This is redundant with checks done elsewhere, but it's a last-ditch
   * attempt to avoid queueing something we shouldn't have to queue. */
  if (PREDICT_UNLIKELY( ! EVENT_IS_INTERESTING(event) )) {
    tor_free(msg);
    return;
  }

  int *block_event_queue = get_block_event_queue();
  if (*block_event_queue) {
    tor_free(msg);
    return;
  }

  queued_event_t *ev = tor_malloc(sizeof(*ev));
  ev->event = event;
  ev->msg = msg;

  /* No queueing an event while queueing an event */
  ++*block_event_queue;

  tor_mutex_acquire(queued_control_events_lock);
  tor_assert(queued_control_events);
  smartlist_add(queued_control_events, ev);

  int activate_event = 0;
  if (! flush_queued_event_pending && in_main_thread()) {
    activate_event = 1;
    flush_queued_event_pending = 1;
  }

  tor_mutex_release(queued_control_events_lock);

  --*block_event_queue;

  /* We just put an event on the queue; mark the queue to be
   * flushed.  We only do this from the main thread for now; otherwise,
   * we'd need to incur locking overhead in Libevent or use a socket.
   */
  if (activate_event) {
    tor_assert(flush_queued_events_event);
    mainloop_event_activate(flush_queued_events_event);
  }
}

#define queued_event_free(ev) \
  FREE_AND_NULL(queued_event_t, queued_event_free_, (ev))

/** Release all storage held by <b>ev</b>. */
static void
queued_event_free_(queued_event_t *ev)
{
  if (ev == NULL)
    return;

  tor_free(ev->msg);
  tor_free(ev);
}

/** Send every queued event to every controller that's interested in it,
 * and remove the events from the queue.  If <b>force</b> is true,
 * then make all controllers send their data out immediately, since we
 * may be about to shut down. */
static void
queued_events_flush_all(int force)
{
  /* Make sure that we get all the pending log events, if there are any. */
  flush_pending_log_callbacks();

  if (PREDICT_UNLIKELY(queued_control_events == NULL)) {
    return;
  }
  smartlist_t *all_conns = get_connection_array();
  smartlist_t *controllers = smartlist_new();
  smartlist_t *queued_events;

  int *block_event_queue = get_block_event_queue();
  ++*block_event_queue;

  tor_mutex_acquire(queued_control_events_lock);
  /* No queueing an event while flushing events. */
  flush_queued_event_pending = 0;
  queued_events = queued_control_events;
  queued_control_events = smartlist_new();
  tor_mutex_release(queued_control_events_lock);

  /* Gather all the controllers that will care... */
  SMARTLIST_FOREACH_BEGIN(all_conns, connection_t *, conn) {
    if (conn->type == CONN_TYPE_CONTROL &&
        !conn->marked_for_close &&
        conn->state == CONTROL_CONN_STATE_OPEN) {
      control_connection_t *control_conn = TO_CONTROL_CONN(conn);

      smartlist_add(controllers, control_conn);
    }
  } SMARTLIST_FOREACH_END(conn);

  SMARTLIST_FOREACH_BEGIN(queued_events, queued_event_t *, ev) {
    const event_mask_t bit = ((event_mask_t)1) << ev->event;
    const size_t msg_len = strlen(ev->msg);
    SMARTLIST_FOREACH_BEGIN(controllers, control_connection_t *,
                            control_conn) {
      if (control_conn->event_mask & bit) {
        connection_buf_add(ev->msg, msg_len, TO_CONN(control_conn));
      }
    } SMARTLIST_FOREACH_END(control_conn);

    queued_event_free(ev);
  } SMARTLIST_FOREACH_END(ev);

  if (force) {
    SMARTLIST_FOREACH_BEGIN(controllers, control_connection_t *,
                            control_conn) {
      connection_flush(TO_CONN(control_conn));
    } SMARTLIST_FOREACH_END(control_conn);
  }

  smartlist_free(queued_events);
  smartlist_free(controllers);

  --*block_event_queue;
}

/** Libevent callback: Flushes pending events to controllers that are
 * interested in them. */
static void
flush_queued_events_cb(mainloop_event_t *event, void *arg)
{
  (void) event;
  (void) arg;
  queued_events_flush_all(0);
}

/** Send an event to all v1 controllers that are listening for code
 * <b>event</b>.  The event's body is given by <b>msg</b>.
 *
 * The EXTENDED_FORMAT and NONEXTENDED_FORMAT flags behave similarly with
 * respect to the EXTENDED_EVENTS feature. */
MOCK_IMPL(STATIC void,
send_control_event_string,(uint16_t event,
                           const char *msg))
{
  tor_assert(event >= EVENT_MIN_ && event <= EVENT_MAX_);
  queue_control_event_string(event, tor_strdup(msg));
}

/** Helper for send_control_event and control_event_status:
 * Send an event to all v1 controllers that are listening for code
 * <b>event</b>.  The event's body is created by the printf-style format in
 * <b>format</b>, and other arguments as provided. */
static void
send_control_event_impl(uint16_t event,
                        const char *format, va_list ap)
{
  char *buf = NULL;
  int len;

  len = tor_vasprintf(&buf, format, ap);
  if (len < 0) {
    log_warn(LD_BUG, "Unable to format event for controller.");
    return;
  }

  queue_control_event_string(event, buf);
}

/** Send an event to all v1 controllers that are listening for code
 * <b>event</b>.  The event's body is created by the printf-style format in
 * <b>format</b>, and other arguments as provided. */
static void
send_control_event(uint16_t event,
                   const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  send_control_event_impl(event, format, ap);
  va_end(ap);
}

/** Something major has happened to circuit <b>circ</b>: tell any
 * interested control connections. */
int
control_event_circuit_status(origin_circuit_t *circ, circuit_status_event_t tp,
                             int reason_code)
{
  const char *status;
  char reasons[64] = "";

  if (!EVENT_IS_INTERESTING(EVENT_CIRCUIT_STATUS))
    return 0;
  tor_assert(circ);

  switch (tp)
    {
    case CIRC_EVENT_LAUNCHED: status = "LAUNCHED"; break;
    case CIRC_EVENT_BUILT: status = "BUILT"; break;
    case CIRC_EVENT_EXTENDED: status = "EXTENDED"; break;
    case CIRC_EVENT_FAILED: status = "FAILED"; break;
    case CIRC_EVENT_CLOSED: status = "CLOSED"; break;
    default:
      log_warn(LD_BUG, "Unrecognized status code %d", (int)tp);
      tor_fragile_assert();
      return 0;
    }

  if (tp == CIRC_EVENT_FAILED || tp == CIRC_EVENT_CLOSED) {
    const char *reason_str = circuit_end_reason_to_control_string(reason_code);
    char unk_reason_buf[16];
    if (!reason_str) {
      tor_snprintf(unk_reason_buf, 16, "UNKNOWN_%d", reason_code);
      reason_str = unk_reason_buf;
    }
    if (reason_code > 0 && reason_code & END_CIRC_REASON_FLAG_REMOTE) {
      tor_snprintf(reasons, sizeof(reasons),
                   " REASON=DESTROYED REMOTE_REASON=%s", reason_str);
    } else {
      tor_snprintf(reasons, sizeof(reasons),
                   " REASON=%s", reason_str);
    }
  }

  {
    char *circdesc = circuit_describe_status_for_controller(circ);
    const char *sp = strlen(circdesc) ? " " : "";
    send_control_event(EVENT_CIRCUIT_STATUS,
                                "650 CIRC %lu %s%s%s%s\r\n",
                                (unsigned long)circ->global_identifier,
                                status, sp,
                                circdesc,
                                reasons);
    tor_free(circdesc);
  }

  return 0;
}

/** Something minor has happened to circuit <b>circ</b>: tell any
 * interested control connections. */
static int
control_event_circuit_status_minor(origin_circuit_t *circ,
                                   circuit_status_minor_event_t e,
                                   int purpose, const struct timeval *tv)
{
  const char *event_desc;
  char event_tail[160] = "";
  if (!EVENT_IS_INTERESTING(EVENT_CIRCUIT_STATUS_MINOR))
    return 0;
  tor_assert(circ);

  switch (e)
    {
    case CIRC_MINOR_EVENT_PURPOSE_CHANGED:
      event_desc = "PURPOSE_CHANGED";

      {
        /* event_tail can currently be up to 68 chars long */
        const char *hs_state_str =
          circuit_purpose_to_controller_hs_state_string(purpose);
        tor_snprintf(event_tail, sizeof(event_tail),
                     " OLD_PURPOSE=%s%s%s",
                     circuit_purpose_to_controller_string(purpose),
                     (hs_state_str != NULL) ? " OLD_HS_STATE=" : "",
                     (hs_state_str != NULL) ? hs_state_str : "");
      }

      break;
    case CIRC_MINOR_EVENT_CANNIBALIZED:
      event_desc = "CANNIBALIZED";

      {
        /* event_tail can currently be up to 130 chars long */
        const char *hs_state_str =
          circuit_purpose_to_controller_hs_state_string(purpose);
        const struct timeval *old_timestamp_began = tv;
        char tbuf[ISO_TIME_USEC_LEN+1];
        format_iso_time_nospace_usec(tbuf, old_timestamp_began);

        tor_snprintf(event_tail, sizeof(event_tail),
                     " OLD_PURPOSE=%s%s%s OLD_TIME_CREATED=%s",
                     circuit_purpose_to_controller_string(purpose),
                     (hs_state_str != NULL) ? " OLD_HS_STATE=" : "",
                     (hs_state_str != NULL) ? hs_state_str : "",
                     tbuf);
      }

      break;
    default:
      log_warn(LD_BUG, "Unrecognized status code %d", (int)e);
      tor_fragile_assert();
      return 0;
    }

  {
    char *circdesc = circuit_describe_status_for_controller(circ);
    const char *sp = strlen(circdesc) ? " " : "";
    send_control_event(EVENT_CIRCUIT_STATUS_MINOR,
                       "650 CIRC_MINOR %lu %s%s%s%s\r\n",
                       (unsigned long)circ->global_identifier,
                       event_desc, sp,
                       circdesc,
                       event_tail);
    tor_free(circdesc);
  }

  return 0;
}

/**
 * <b>circ</b> has changed its purpose from <b>old_purpose</b>: tell any
 * interested controllers.
 */
int
control_event_circuit_purpose_changed(origin_circuit_t *circ,
                                      int old_purpose)
{
  return control_event_circuit_status_minor(circ,
                                            CIRC_MINOR_EVENT_PURPOSE_CHANGED,
                                            old_purpose,
                                            NULL);
}

/**
 * <b>circ</b> has changed its purpose from <b>old_purpose</b>, and its
 * created-time from <b>old_tv_created</b>: tell any interested controllers.
 */
int
control_event_circuit_cannibalized(origin_circuit_t *circ,
                                   int old_purpose,
                                   const struct timeval *old_tv_created)
{
  return control_event_circuit_status_minor(circ,
                                            CIRC_MINOR_EVENT_CANNIBALIZED,
                                            old_purpose,
                                            old_tv_created);
}

/** Something has happened to the stream associated with AP connection
 * <b>conn</b>: tell any interested control connections. */
int
control_event_stream_status(entry_connection_t *conn, stream_status_event_t tp,
                            int reason_code)
{
  char reason_buf[64];
  char addrport_buf[64];
  const char *status;
  circuit_t *circ;
  origin_circuit_t *origin_circ = NULL;
  char buf[256];
  const char *purpose = "";
  tor_assert(conn->socks_request);

  if (!EVENT_IS_INTERESTING(EVENT_STREAM_STATUS))
    return 0;

  if (tp == STREAM_EVENT_CLOSED &&
      (reason_code & END_STREAM_REASON_FLAG_ALREADY_SENT_CLOSED))
    return 0;

  write_stream_target_to_buf(conn, buf, sizeof(buf));

  reason_buf[0] = '\0';
  switch (tp)
    {
    case STREAM_EVENT_SENT_CONNECT: status = "SENTCONNECT"; break;
    case STREAM_EVENT_SENT_RESOLVE: status = "SENTRESOLVE"; break;
    case STREAM_EVENT_SUCCEEDED: status = "SUCCEEDED"; break;
    case STREAM_EVENT_FAILED: status = "FAILED"; break;
    case STREAM_EVENT_CLOSED: status = "CLOSED"; break;
    case STREAM_EVENT_NEW: status = "NEW"; break;
    case STREAM_EVENT_NEW_RESOLVE: status = "NEWRESOLVE"; break;
    case STREAM_EVENT_FAILED_RETRIABLE: status = "DETACHED"; break;
    case STREAM_EVENT_REMAP: status = "REMAP"; break;
    case STREAM_EVENT_CONTROLLER_WAIT: status = "CONTROLLER_WAIT"; break;
    default:
      log_warn(LD_BUG, "Unrecognized status code %d", (int)tp);
      return 0;
    }
  if (reason_code && (tp == STREAM_EVENT_FAILED ||
                      tp == STREAM_EVENT_CLOSED ||
                      tp == STREAM_EVENT_FAILED_RETRIABLE)) {
    const char *reason_str = stream_end_reason_to_control_string(reason_code);
    char *r = NULL;
    if (!reason_str) {
      tor_asprintf(&r, " UNKNOWN_%d", reason_code);
      reason_str = r;
    }
    if (reason_code & END_STREAM_REASON_FLAG_REMOTE)
      tor_snprintf(reason_buf, sizeof(reason_buf),
                   " REASON=END REMOTE_REASON=%s", reason_str);
    else
      tor_snprintf(reason_buf, sizeof(reason_buf),
                   " REASON=%s", reason_str);
    tor_free(r);
  } else if (reason_code && tp == STREAM_EVENT_REMAP) {
    switch (reason_code) {
    case REMAP_STREAM_SOURCE_CACHE:
      strlcpy(reason_buf, " SOURCE=CACHE", sizeof(reason_buf));
      break;
    case REMAP_STREAM_SOURCE_EXIT:
      strlcpy(reason_buf, " SOURCE=EXIT", sizeof(reason_buf));
      break;
    default:
      tor_snprintf(reason_buf, sizeof(reason_buf), " REASON=UNKNOWN_%d",
                   reason_code);
      /* XXX do we want SOURCE=UNKNOWN_%d above instead? -RD */
      break;
    }
  }

  if (tp == STREAM_EVENT_NEW || tp == STREAM_EVENT_NEW_RESOLVE) {
    /*
     * When the control conn is an AF_UNIX socket and we have no address,
     * it gets set to "(Tor_internal)"; see dnsserv_launch_request() in
     * dnsserv.c.
     */
    if (strcmp(ENTRY_TO_CONN(conn)->address, "(Tor_internal)") != 0) {
      tor_snprintf(addrport_buf,sizeof(addrport_buf), " SOURCE_ADDR=%s:%d",
                   ENTRY_TO_CONN(conn)->address, ENTRY_TO_CONN(conn)->port);
    } else {
      /*
       * else leave it blank so control on AF_UNIX doesn't need to make
       * something up.
       */
      addrport_buf[0] = '\0';
    }
  } else {
    addrport_buf[0] = '\0';
  }

  if (tp == STREAM_EVENT_NEW_RESOLVE) {
    purpose = " PURPOSE=DNS_REQUEST";
  } else if (tp == STREAM_EVENT_NEW) {
    if (conn->use_begindir) {
      connection_t *linked = ENTRY_TO_CONN(conn)->linked_conn;
      int linked_dir_purpose = -1;
      if (linked && linked->type == CONN_TYPE_DIR)
        linked_dir_purpose = linked->purpose;
      if (DIR_PURPOSE_IS_UPLOAD(linked_dir_purpose))
        purpose = " PURPOSE=DIR_UPLOAD";
      else
        purpose = " PURPOSE=DIR_FETCH";
    } else
      purpose = " PURPOSE=USER";
  }

  circ = circuit_get_by_edge_conn(ENTRY_TO_EDGE_CONN(conn));
  if (circ && CIRCUIT_IS_ORIGIN(circ))
    origin_circ = TO_ORIGIN_CIRCUIT(circ);

  {
    char *conndesc = entry_connection_describe_status_for_controller(conn);
    const char *sp = strlen(conndesc) ? " " : "";
    send_control_event(EVENT_STREAM_STATUS,
                        "650 STREAM %"PRIu64" %s %lu %s%s%s%s%s%s\r\n",
                     (ENTRY_TO_CONN(conn)->global_identifier),
                     status,
                        origin_circ?
                           (unsigned long)origin_circ->global_identifier : 0ul,
                        buf, reason_buf, addrport_buf, purpose, sp, conndesc);
    tor_free(conndesc);
  }

  /* XXX need to specify its intended exit, etc? */

  return 0;
}

/** Called when the status of an OR connection <b>conn</b> changes: tell any
 * interested control connections. <b>tp</b> is the new status for the
 * connection.  If <b>conn</b> has just closed or failed, then <b>reason</b>
 * may be the reason why.
 */
int
control_event_or_conn_status(or_connection_t *conn, or_conn_status_event_t tp,
                             int reason)
{
  int ncircs = 0;
  const char *status;
  char name[128];
  char ncircs_buf[32] = {0}; /* > 8 + log10(2^32)=10 + 2 */

  if (!EVENT_IS_INTERESTING(EVENT_OR_CONN_STATUS))
    return 0;

  switch (tp)
    {
    case OR_CONN_EVENT_LAUNCHED: status = "LAUNCHED"; break;
    case OR_CONN_EVENT_CONNECTED: status = "CONNECTED"; break;
    case OR_CONN_EVENT_FAILED: status = "FAILED"; break;
    case OR_CONN_EVENT_CLOSED: status = "CLOSED"; break;
    case OR_CONN_EVENT_NEW: status = "NEW"; break;
    default:
      log_warn(LD_BUG, "Unrecognized status code %d", (int)tp);
      return 0;
    }
  if (conn->chan) {
    ncircs = circuit_count_pending_on_channel(TLS_CHAN_TO_BASE(conn->chan));
  } else {
    ncircs = 0;
  }
  ncircs += connection_or_get_num_circuits(conn);
  if (ncircs && (tp == OR_CONN_EVENT_FAILED || tp == OR_CONN_EVENT_CLOSED)) {
    tor_snprintf(ncircs_buf, sizeof(ncircs_buf), " NCIRCS=%d", ncircs);
  }

  orconn_target_get_name(name, sizeof(name), conn);
  send_control_event(EVENT_OR_CONN_STATUS,
                              "650 ORCONN %s %s%s%s%s ID=%"PRIu64"\r\n",
                              name, status,
                              reason ? " REASON=" : "",
                              orconn_end_reason_to_control_string(reason),
                              ncircs_buf,
                              (conn->base_.global_identifier));

  return 0;
}

/**
 * Print out STREAM_BW event for a single conn
 */
int
control_event_stream_bandwidth(edge_connection_t *edge_conn)
{
  struct timeval now;
  char tbuf[ISO_TIME_USEC_LEN+1];
  if (EVENT_IS_INTERESTING(EVENT_STREAM_BANDWIDTH_USED)) {
    if (!edge_conn->n_read && !edge_conn->n_written)
      return 0;

    tor_gettimeofday(&now);
    format_iso_time_nospace_usec(tbuf, &now);
    send_control_event(EVENT_STREAM_BANDWIDTH_USED,
                       "650 STREAM_BW %"PRIu64" %lu %lu %s\r\n",
                       (edge_conn->base_.global_identifier),
                       (unsigned long)edge_conn->n_read,
                       (unsigned long)edge_conn->n_written,
                       tbuf);

    edge_conn->n_written = edge_conn->n_read = 0;
  }

  return 0;
}

/** A second or more has elapsed: tell any interested control
 * connections how much bandwidth streams have used. */
int
control_event_stream_bandwidth_used(void)
{
  if (EVENT_IS_INTERESTING(EVENT_STREAM_BANDWIDTH_USED)) {
    smartlist_t *conns = get_connection_array();
    edge_connection_t *edge_conn;
    struct timeval now;
    char tbuf[ISO_TIME_USEC_LEN+1];

    SMARTLIST_FOREACH_BEGIN(conns, connection_t *, conn)
    {
        if (conn->type != CONN_TYPE_AP)
          continue;
        edge_conn = TO_EDGE_CONN(conn);
        if (!edge_conn->n_read && !edge_conn->n_written)
          continue;

        tor_gettimeofday(&now);
        format_iso_time_nospace_usec(tbuf, &now);
        send_control_event(EVENT_STREAM_BANDWIDTH_USED,
                           "650 STREAM_BW %"PRIu64" %lu %lu %s\r\n",
                           (edge_conn->base_.global_identifier),
                           (unsigned long)edge_conn->n_read,
                           (unsigned long)edge_conn->n_written,
                           tbuf);

        edge_conn->n_written = edge_conn->n_read = 0;
    }
    SMARTLIST_FOREACH_END(conn);
  }

  return 0;
}

/** A second or more has elapsed: tell any interested control connections
 * how much bandwidth origin circuits have used. */
int
control_event_circ_bandwidth_used(void)
{
  if (!EVENT_IS_INTERESTING(EVENT_CIRC_BANDWIDTH_USED))
    return 0;

  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    if (!CIRCUIT_IS_ORIGIN(circ))
      continue;

    control_event_circ_bandwidth_used_for_circ(TO_ORIGIN_CIRCUIT(circ));
  }
  SMARTLIST_FOREACH_END(circ);

  return 0;
}

/**
 * Emit a CIRC_BW event line for a specific circuit.
 *
 * This function sets the values it emits to 0, and does not emit
 * an event if there is no new data to report since the last call.
 *
 * Therefore, it may be called at any frequency.
 */
int
control_event_circ_bandwidth_used_for_circ(origin_circuit_t *ocirc)
{
  struct timeval now;
  char tbuf[ISO_TIME_USEC_LEN+1];

  tor_assert(ocirc);

  if (!EVENT_IS_INTERESTING(EVENT_CIRC_BANDWIDTH_USED))
    return 0;

  /* n_read_circ_bw and n_written_circ_bw are always updated
   * when there is any new cell on a circuit, and set to 0 after
   * the event, below.
   *
   * Therefore, checking them is sufficient to determine if there
   * is new data to report. */
  if (!ocirc->n_read_circ_bw && !ocirc->n_written_circ_bw)
    return 0;

  tor_gettimeofday(&now);
  format_iso_time_nospace_usec(tbuf, &now);
  send_control_event(EVENT_CIRC_BANDWIDTH_USED,
                     "650 CIRC_BW ID=%d READ=%lu WRITTEN=%lu TIME=%s "
                     "DELIVERED_READ=%lu OVERHEAD_READ=%lu "
                     "DELIVERED_WRITTEN=%lu OVERHEAD_WRITTEN=%lu\r\n",
                     ocirc->global_identifier,
                     (unsigned long)ocirc->n_read_circ_bw,
                     (unsigned long)ocirc->n_written_circ_bw,
                     tbuf,
                     (unsigned long)ocirc->n_delivered_read_circ_bw,
                     (unsigned long)ocirc->n_overhead_read_circ_bw,
                     (unsigned long)ocirc->n_delivered_written_circ_bw,
                     (unsigned long)ocirc->n_overhead_written_circ_bw);
  ocirc->n_written_circ_bw = ocirc->n_read_circ_bw = 0;
  ocirc->n_overhead_written_circ_bw = ocirc->n_overhead_read_circ_bw = 0;
  ocirc->n_delivered_written_circ_bw = ocirc->n_delivered_read_circ_bw = 0;

  return 0;
}

/** Print out CONN_BW event for a single OR/DIR/EXIT <b>conn</b> and reset
  * bandwidth counters. */
int
control_event_conn_bandwidth(connection_t *conn)
{
  const char *conn_type_str;
  if (!get_options()->TestingEnableConnBwEvent ||
      !EVENT_IS_INTERESTING(EVENT_CONN_BW))
    return 0;
  if (!conn->n_read_conn_bw && !conn->n_written_conn_bw)
    return 0;
  switch (conn->type) {
    case CONN_TYPE_OR:
      conn_type_str = "OR";
      break;
    case CONN_TYPE_DIR:
      conn_type_str = "DIR";
      break;
    case CONN_TYPE_EXIT:
      conn_type_str = "EXIT";
      break;
    default:
      return 0;
  }
  send_control_event(EVENT_CONN_BW,
                     "650 CONN_BW ID=%"PRIu64" TYPE=%s "
                     "READ=%lu WRITTEN=%lu\r\n",
                     (conn->global_identifier),
                     conn_type_str,
                     (unsigned long)conn->n_read_conn_bw,
                     (unsigned long)conn->n_written_conn_bw);
  conn->n_written_conn_bw = conn->n_read_conn_bw = 0;
  return 0;
}

/** A second or more has elapsed: tell any interested control
 * connections how much bandwidth connections have used. */
int
control_event_conn_bandwidth_used(void)
{
  if (get_options()->TestingEnableConnBwEvent &&
      EVENT_IS_INTERESTING(EVENT_CONN_BW)) {
    SMARTLIST_FOREACH(get_connection_array(), connection_t *, conn,
                      control_event_conn_bandwidth(conn));
  }
  return 0;
}

/** Helper: iterate over cell statistics of <b>circ</b> and sum up added
 * cells, removed cells, and waiting times by cell command and direction.
 * Store results in <b>cell_stats</b>.  Free cell statistics of the
 * circuit afterwards. */
void
sum_up_cell_stats_by_command(circuit_t *circ, cell_stats_t *cell_stats)
{
  memset(cell_stats, 0, sizeof(cell_stats_t));
  SMARTLIST_FOREACH_BEGIN(circ->testing_cell_stats,
                          const testing_cell_stats_entry_t *, ent) {
    tor_assert(ent->command <= CELL_COMMAND_MAX_);
    if (!ent->removed && !ent->exitward) {
      cell_stats->added_cells_appward[ent->command] += 1;
    } else if (!ent->removed && ent->exitward) {
      cell_stats->added_cells_exitward[ent->command] += 1;
    } else if (!ent->exitward) {
      cell_stats->removed_cells_appward[ent->command] += 1;
      cell_stats->total_time_appward[ent->command] += ent->waiting_time * 10;
    } else {
      cell_stats->removed_cells_exitward[ent->command] += 1;
      cell_stats->total_time_exitward[ent->command] += ent->waiting_time * 10;
    }
  } SMARTLIST_FOREACH_END(ent);
  circuit_clear_testing_cell_stats(circ);
}

/** Helper: append a cell statistics string to <code>event_parts</code>,
 * prefixed with <code>key</code>=.  Statistics consist of comma-separated
 * key:value pairs with lower-case command strings as keys and cell
 * numbers or total waiting times as values.  A key:value pair is included
 * if the entry in <code>include_if_non_zero</code> is not zero, but with
 * the (possibly zero) entry from <code>number_to_include</code>.  Both
 * arrays are expected to have a length of CELL_COMMAND_MAX_ + 1.  If no
 * entry in <code>include_if_non_zero</code> is positive, no string will
 * be added to <code>event_parts</code>. */
void
append_cell_stats_by_command(smartlist_t *event_parts, const char *key,
                             const uint64_t *include_if_non_zero,
                             const uint64_t *number_to_include)
{
  smartlist_t *key_value_strings = smartlist_new();
  int i;
  for (i = 0; i <= CELL_COMMAND_MAX_; i++) {
    if (include_if_non_zero[i] > 0) {
      smartlist_add_asprintf(key_value_strings, "%s:%"PRIu64,
                             cell_command_to_string(i),
                             (number_to_include[i]));
    }
  }
  if (smartlist_len(key_value_strings) > 0) {
    char *joined = smartlist_join_strings(key_value_strings, ",", 0, NULL);
    smartlist_add_asprintf(event_parts, "%s=%s", key, joined);
    SMARTLIST_FOREACH(key_value_strings, char *, cp, tor_free(cp));
    tor_free(joined);
  }
  smartlist_free(key_value_strings);
}

/** Helper: format <b>cell_stats</b> for <b>circ</b> for inclusion in a
 * CELL_STATS event and write result string to <b>event_string</b>. */
void
format_cell_stats(char **event_string, circuit_t *circ,
                  cell_stats_t *cell_stats)
{
  smartlist_t *event_parts = smartlist_new();
  if (CIRCUIT_IS_ORIGIN(circ)) {
    origin_circuit_t *ocirc = TO_ORIGIN_CIRCUIT(circ);
    smartlist_add_asprintf(event_parts, "ID=%lu",
                 (unsigned long)ocirc->global_identifier);
  } else if (TO_OR_CIRCUIT(circ)->p_chan) {
    or_circuit_t *or_circ = TO_OR_CIRCUIT(circ);
    smartlist_add_asprintf(event_parts, "InboundQueue=%lu",
                 (unsigned long)or_circ->p_circ_id);
    smartlist_add_asprintf(event_parts, "InboundConn=%"PRIu64,
                 (or_circ->p_chan->global_identifier));
    append_cell_stats_by_command(event_parts, "InboundAdded",
                                 cell_stats->added_cells_appward,
                                 cell_stats->added_cells_appward);
    append_cell_stats_by_command(event_parts, "InboundRemoved",
                                 cell_stats->removed_cells_appward,
                                 cell_stats->removed_cells_appward);
    append_cell_stats_by_command(event_parts, "InboundTime",
                                 cell_stats->removed_cells_appward,
                                 cell_stats->total_time_appward);
  }
  if (circ->n_chan) {
    smartlist_add_asprintf(event_parts, "OutboundQueue=%lu",
                     (unsigned long)circ->n_circ_id);
    smartlist_add_asprintf(event_parts, "OutboundConn=%"PRIu64,
                 (circ->n_chan->global_identifier));
    append_cell_stats_by_command(event_parts, "OutboundAdded",
                                 cell_stats->added_cells_exitward,
                                 cell_stats->added_cells_exitward);
    append_cell_stats_by_command(event_parts, "OutboundRemoved",
                                 cell_stats->removed_cells_exitward,
                                 cell_stats->removed_cells_exitward);
    append_cell_stats_by_command(event_parts, "OutboundTime",
                                 cell_stats->removed_cells_exitward,
                                 cell_stats->total_time_exitward);
  }
  *event_string = smartlist_join_strings(event_parts, " ", 0, NULL);
  SMARTLIST_FOREACH(event_parts, char *, cp, tor_free(cp));
  smartlist_free(event_parts);
}

/** A second or more has elapsed: tell any interested control connection
 * how many cells have been processed for a given circuit. */
int
control_event_circuit_cell_stats(void)
{
  cell_stats_t *cell_stats;
  char *event_string;
  if (!get_options()->TestingEnableCellStatsEvent ||
      !EVENT_IS_INTERESTING(EVENT_CELL_STATS))
    return 0;
  cell_stats = tor_malloc(sizeof(cell_stats_t));
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    if (!circ->testing_cell_stats)
      continue;
    sum_up_cell_stats_by_command(circ, cell_stats);
    format_cell_stats(&event_string, circ, cell_stats);
    send_control_event(EVENT_CELL_STATS,
                       "650 CELL_STATS %s\r\n", event_string);
    tor_free(event_string);
  }
  SMARTLIST_FOREACH_END(circ);
  tor_free(cell_stats);
  return 0;
}

/* about 5 minutes worth. */
#define N_BW_EVENTS_TO_CACHE 300
/* Index into cached_bw_events to next write. */
static int next_measurement_idx = 0;
/* number of entries set in n_measurements */
static int n_measurements = 0;
static struct cached_bw_event_t {
  uint32_t n_read;
  uint32_t n_written;
} cached_bw_events[N_BW_EVENTS_TO_CACHE];

/** A second or more has elapsed: tell any interested control
 * connections how much bandwidth we used. */
int
control_event_bandwidth_used(uint32_t n_read, uint32_t n_written)
{
  cached_bw_events[next_measurement_idx].n_read = n_read;
  cached_bw_events[next_measurement_idx].n_written = n_written;
  if (++next_measurement_idx == N_BW_EVENTS_TO_CACHE)
    next_measurement_idx = 0;
  if (n_measurements < N_BW_EVENTS_TO_CACHE)
    ++n_measurements;

  if (EVENT_IS_INTERESTING(EVENT_BANDWIDTH_USED)) {
    send_control_event(EVENT_BANDWIDTH_USED,
                       "650 BW %lu %lu\r\n",
                       (unsigned long)n_read,
                       (unsigned long)n_written);
  }

  return 0;
}

char *
get_bw_samples(void)
{
  int i;
  int idx = (next_measurement_idx + N_BW_EVENTS_TO_CACHE - n_measurements)
    % N_BW_EVENTS_TO_CACHE;
  tor_assert(0 <= idx && idx < N_BW_EVENTS_TO_CACHE);

  smartlist_t *elements = smartlist_new();

  for (i = 0; i < n_measurements; ++i) {
    tor_assert(0 <= idx && idx < N_BW_EVENTS_TO_CACHE);
    const struct cached_bw_event_t *bwe = &cached_bw_events[idx];

    smartlist_add_asprintf(elements, "%u,%u",
                           (unsigned)bwe->n_read,
                           (unsigned)bwe->n_written);

    idx = (idx + 1) % N_BW_EVENTS_TO_CACHE;
  }

  char *result = smartlist_join_strings(elements, " ", 0, NULL);

  SMARTLIST_FOREACH(elements, char *, cp, tor_free(cp));
  smartlist_free(elements);

  return result;
}

/** Called when we are sending a log message to the controllers: suspend
 * sending further log messages to the controllers until we're done.  Used by
 * CONN_LOG_PROTECT. */
void
disable_control_logging(void)
{
  ++disable_log_messages;
}

/** We're done sending a log message to the controllers: re-enable controller
 * logging.  Used by CONN_LOG_PROTECT. */
void
enable_control_logging(void)
{
  if (--disable_log_messages < 0)
    tor_assert(0);
}

/** Remove newline and carriage-return characters from @a msg, replacing them
 * with spaces, and discarding any that appear at the end of the message */
void
control_logmsg_strip_newlines(char *msg)
{
  char *cp;
  for (cp = msg; *cp; ++cp) {
    if (*cp == '\r' || *cp == '\n') {
      *cp = ' ';
    }
  }
  if (cp == msg)
    return;
  /* Remove trailing spaces */
  for (--cp; *cp == ' '; --cp) {
    *cp = '\0';
    if (cp == msg)
      break;
  }
}

/** We got a log message: tell any interested control connections. */
void
control_event_logmsg(int severity, log_domain_mask_t domain, const char *msg)
{
  int event;

  /* Don't even think of trying to add stuff to a buffer from a cpuworker
   * thread. (See #25987 for plan to fix.) */
  if (! in_main_thread())
    return;

  if (disable_log_messages)
    return;

  if (domain == LD_BUG && EVENT_IS_INTERESTING(EVENT_STATUS_GENERAL) &&
      severity <= LOG_NOTICE) {
    char *esc = esc_for_log(msg);
    ++disable_log_messages;
    control_event_general_status(severity, "BUG REASON=%s", esc);
    --disable_log_messages;
    tor_free(esc);
  }

  event = log_severity_to_event(severity);
  if (event >= 0 && EVENT_IS_INTERESTING(event)) {
    char *b = NULL;
    const char *s;
    if (strchr(msg, '\n')) {
      b = tor_strdup(msg);
      control_logmsg_strip_newlines(b);
    }
    switch (severity) {
      case LOG_DEBUG: s = "DEBUG"; break;
      case LOG_INFO: s = "INFO"; break;
      case LOG_NOTICE: s = "NOTICE"; break;
      case LOG_WARN: s = "WARN"; break;
      case LOG_ERR: s = "ERR"; break;
      default: s = "UnknownLogSeverity"; break;
    }
    ++disable_log_messages;
    send_control_event(event,  "650 %s %s\r\n", s, b?b:msg);
    if (severity == LOG_ERR) {
      /* Force a flush, since we may be about to die horribly */
      queued_events_flush_all(1);
    }
    --disable_log_messages;
    tor_free(b);
  }
}

/**
 * Logging callback: called when there is a queued pending log callback.
 */
void
control_event_logmsg_pending(void)
{
  if (! in_main_thread()) {
    /* We can't handle this case yet, since we're using a
     * mainloop_event_t to invoke queued_events_flush_all.  We ought to
     * use a different mechanism instead: see #25987.
     **/
    return;
  }
  tor_assert(flush_queued_events_event);
  mainloop_event_activate(flush_queued_events_event);
}

/** Called whenever we receive new router descriptors: tell any
 * interested control connections.  <b>routers</b> is a list of
 * routerinfo_t's.
 */
int
control_event_descriptors_changed(smartlist_t *routers)
{
  char *msg;

  if (!EVENT_IS_INTERESTING(EVENT_NEW_DESC))
    return 0;

  {
    smartlist_t *names = smartlist_new();
    char *ids;
    SMARTLIST_FOREACH(routers, routerinfo_t *, ri, {
        char *b = tor_malloc(MAX_VERBOSE_NICKNAME_LEN+1);
        router_get_verbose_nickname(b, ri);
        smartlist_add(names, b);
      });
    ids = smartlist_join_strings(names, " ", 0, NULL);
    tor_asprintf(&msg, "650 NEWDESC %s\r\n", ids);
    send_control_event_string(EVENT_NEW_DESC,  msg);
    tor_free(ids);
    tor_free(msg);
    SMARTLIST_FOREACH(names, char *, cp, tor_free(cp));
    smartlist_free(names);
  }
  return 0;
}

/** Called when an address mapping on <b>from</b> from changes to <b>to</b>.
 * <b>expires</b> values less than 3 are special; see connection_edge.c.  If
 * <b>error</b> is non-NULL, it is an error code describing the failure
 * mode of the mapping.
 */
int
control_event_address_mapped(const char *from, const char *to, time_t expires,
                             const char *error, const int cached)
{
  if (!EVENT_IS_INTERESTING(EVENT_ADDRMAP))
    return 0;

  if (expires < 3 || expires == TIME_MAX)
    send_control_event(EVENT_ADDRMAP,
                                "650 ADDRMAP %s %s NEVER %s%s"
                                "CACHED=\"%s\"\r\n",
                                  from, to, error?error:"", error?" ":"",
                                cached?"YES":"NO");
  else {
    char buf[ISO_TIME_LEN+1];
    char buf2[ISO_TIME_LEN+1];
    format_local_iso_time(buf,expires);
    format_iso_time(buf2,expires);
    send_control_event(EVENT_ADDRMAP,
                                "650 ADDRMAP %s %s \"%s\""
                                " %s%sEXPIRES=\"%s\" CACHED=\"%s\"\r\n",
                                from, to, buf,
                                error?error:"", error?" ":"",
                                buf2, cached?"YES":"NO");
  }

  return 0;
}
/** The network liveness has changed; this is called from circuitstats.c
 * whenever we receive a cell, or when timeout expires and we assume the
 * network is down. */
int
control_event_network_liveness_update(int liveness)
{
  if (liveness > 0) {
    if (get_cached_network_liveness() <= 0) {
      /* Update cached liveness */
      set_cached_network_liveness(1);
      log_debug(LD_CONTROL, "Sending NETWORK_LIVENESS UP");
      send_control_event_string(EVENT_NETWORK_LIVENESS,
                                "650 NETWORK_LIVENESS UP\r\n");
    }
    /* else was already live, no-op */
  } else {
    if (get_cached_network_liveness() > 0) {
      /* Update cached liveness */
      set_cached_network_liveness(0);
      log_debug(LD_CONTROL, "Sending NETWORK_LIVENESS DOWN");
      send_control_event_string(EVENT_NETWORK_LIVENESS,
                                "650 NETWORK_LIVENESS DOWN\r\n");
    }
    /* else was already dead, no-op */
  }

  return 0;
}

/** Helper function for NS-style events. Constructs and sends an event
 * of type <b>event</b> with string <b>event_string</b> out of the set of
 * networkstatuses <b>statuses</b>. Currently it is used for NS events
 * and NEWCONSENSUS events. */
static int
control_event_networkstatus_changed_helper(smartlist_t *statuses,
                                           uint16_t event,
                                           const char *event_string)
{
  smartlist_t *strs;
  char *s, *esc = NULL;
  if (!EVENT_IS_INTERESTING(event) || !smartlist_len(statuses))
    return 0;

  strs = smartlist_new();
  smartlist_add_strdup(strs, "650+");
  smartlist_add_strdup(strs, event_string);
  smartlist_add_strdup(strs, "\r\n");
  SMARTLIST_FOREACH(statuses, const routerstatus_t *, rs,
    {
      s = networkstatus_getinfo_helper_single(rs);
      if (!s) continue;
      smartlist_add(strs, s);
    });

  s = smartlist_join_strings(strs, "", 0, NULL);
  write_escaped_data(s, strlen(s), &esc);
  SMARTLIST_FOREACH(strs, char *, cp, tor_free(cp));
  smartlist_free(strs);
  tor_free(s);
  send_control_event_string(event,  esc);
  send_control_event_string(event,
                            "650 OK\r\n");

  tor_free(esc);
  return 0;
}

/** Called when the routerstatus_ts <b>statuses</b> have changed: sends
 * an NS event to any controller that cares. */
int
control_event_networkstatus_changed(smartlist_t *statuses)
{
  return control_event_networkstatus_changed_helper(statuses, EVENT_NS, "NS");
}

/** Called when we get a new consensus networkstatus. Sends a NEWCONSENSUS
 * event consisting of an NS-style line for each relay in the consensus. */
int
control_event_newconsensus(const networkstatus_t *consensus)
{
  if (!control_event_is_interesting(EVENT_NEWCONSENSUS))
    return 0;
  return control_event_networkstatus_changed_helper(
           consensus->routerstatus_list, EVENT_NEWCONSENSUS, "NEWCONSENSUS");
}

/** Called when we compute a new circuitbuildtimeout */
int
control_event_buildtimeout_set(buildtimeout_set_event_t type,
                               const char *args)
{
  const char *type_string = NULL;

  if (!control_event_is_interesting(EVENT_BUILDTIMEOUT_SET))
    return 0;

  switch (type) {
    case BUILDTIMEOUT_SET_EVENT_COMPUTED:
      type_string = "COMPUTED";
      break;
    case BUILDTIMEOUT_SET_EVENT_RESET:
      type_string = "RESET";
      break;
    case BUILDTIMEOUT_SET_EVENT_SUSPENDED:
      type_string = "SUSPENDED";
      break;
    case BUILDTIMEOUT_SET_EVENT_DISCARD:
      type_string = "DISCARD";
      break;
    case BUILDTIMEOUT_SET_EVENT_RESUME:
      type_string = "RESUME";
      break;
    default:
      type_string = "UNKNOWN";
      break;
  }

  send_control_event(EVENT_BUILDTIMEOUT_SET,
                     "650 BUILDTIMEOUT_SET %s %s\r\n",
                     type_string, args);

  return 0;
}

/** Called when a signal has been processed from signal_callback */
int
control_event_signal(uintptr_t signal_num)
{
  const char *signal_string = NULL;

  if (!control_event_is_interesting(EVENT_GOT_SIGNAL))
    return 0;

  for (unsigned i = 0; signal_table[i].signal_name != NULL; ++i) {
    if ((int)signal_num == signal_table[i].sig) {
      signal_string = signal_table[i].signal_name;
      break;
    }
  }

  if (signal_string == NULL) {
    log_warn(LD_BUG, "Unrecognized signal %lu in control_event_signal",
             (unsigned long)signal_num);
    return -1;
  }

  send_control_event(EVENT_GOT_SIGNAL,  "650 SIGNAL %s\r\n",
                     signal_string);
  return 0;
}

/** Called when a single local_routerstatus_t has changed: Sends an NS event
 * to any controller that cares. */
int
control_event_networkstatus_changed_single(const routerstatus_t *rs)
{
  smartlist_t *statuses;
  int r;

  if (!EVENT_IS_INTERESTING(EVENT_NS))
    return 0;

  statuses = smartlist_new();
  smartlist_add(statuses, (void*)rs);
  r = control_event_networkstatus_changed(statuses);
  smartlist_free(statuses);
  return r;
}

/** Our own router descriptor has changed; tell any controllers that care.
 */
int
control_event_my_descriptor_changed(void)
{
  send_control_event(EVENT_DESCCHANGED,  "650 DESCCHANGED\r\n");
  return 0;
}

/** Helper: sends a status event where <b>type</b> is one of
 * EVENT_STATUS_{GENERAL,CLIENT,SERVER}, where <b>severity</b> is one of
 * LOG_{NOTICE,WARN,ERR}, and where <b>format</b> is a printf-style format
 * string corresponding to <b>args</b>. */
static int
control_event_status(int type, int severity, const char *format, va_list args)
{
  char *user_buf = NULL;
  char format_buf[160];
  const char *status, *sev;

  switch (type) {
    case EVENT_STATUS_GENERAL:
      status = "STATUS_GENERAL";
      break;
    case EVENT_STATUS_CLIENT:
      status = "STATUS_CLIENT";
      break;
    case EVENT_STATUS_SERVER:
      status = "STATUS_SERVER";
      break;
    default:
      log_warn(LD_BUG, "Unrecognized status type %d", type);
      return -1;
  }
  switch (severity) {
    case LOG_NOTICE:
      sev = "NOTICE";
      break;
    case LOG_WARN:
      sev = "WARN";
      break;
    case LOG_ERR:
      sev = "ERR";
      break;
    default:
      log_warn(LD_BUG, "Unrecognized status severity %d", severity);
      return -1;
  }
  if (tor_snprintf(format_buf, sizeof(format_buf), "650 %s %s",
                   status, sev)<0) {
    log_warn(LD_BUG, "Format string too long.");
    return -1;
  }
  if (tor_vasprintf(&user_buf, format, args)<0) {
    log_warn(LD_BUG, "Failed to create user buffer.");
    return -1;
  }

  send_control_event(type,  "%s %s\r\n", format_buf, user_buf);
  tor_free(user_buf);
  return 0;
}

#ifndef COCCI
#define CONTROL_EVENT_STATUS_BODY(event, sev)                   \
  int r;                                                        \
  do {                                                          \
    va_list ap;                                                 \
    if (!EVENT_IS_INTERESTING(event))                           \
      return 0;                                                 \
                                                                \
    va_start(ap, format);                                       \
    r = control_event_status((event), (sev), format, ap);       \
    va_end(ap);                                                 \
  } while (0)
#endif /* !defined(COCCI) */

/** Format and send an EVENT_STATUS_GENERAL event whose main text is obtained
 * by formatting the arguments using the printf-style <b>format</b>. */
int
control_event_general_status(int severity, const char *format, ...)
{
  CONTROL_EVENT_STATUS_BODY(EVENT_STATUS_GENERAL, severity);
  return r;
}

/** Format and send an EVENT_STATUS_GENERAL LOG_ERR event, and flush it to the
 * controller(s) immediately. */
int
control_event_general_error(const char *format, ...)
{
  CONTROL_EVENT_STATUS_BODY(EVENT_STATUS_GENERAL, LOG_ERR);
  /* Force a flush, since we may be about to die horribly */
  queued_events_flush_all(1);
  return r;
}

/** Format and send an EVENT_STATUS_CLIENT event whose main text is obtained
 * by formatting the arguments using the printf-style <b>format</b>. */
int
control_event_client_status(int severity, const char *format, ...)
{
  CONTROL_EVENT_STATUS_BODY(EVENT_STATUS_CLIENT, severity);
  return r;
}

/** Format and send an EVENT_STATUS_CLIENT LOG_ERR event, and flush it to the
 * controller(s) immediately. */
int
control_event_client_error(const char *format, ...)
{
  CONTROL_EVENT_STATUS_BODY(EVENT_STATUS_CLIENT, LOG_ERR);
  /* Force a flush, since we may be about to die horribly */
  queued_events_flush_all(1);
  return r;
}

/** Format and send an EVENT_STATUS_SERVER event whose main text is obtained
 * by formatting the arguments using the printf-style <b>format</b>. */
int
control_event_server_status(int severity, const char *format, ...)
{
  CONTROL_EVENT_STATUS_BODY(EVENT_STATUS_SERVER, severity);
  return r;
}

/** Format and send an EVENT_STATUS_SERVER LOG_ERR event, and flush it to the
 * controller(s) immediately. */
int
control_event_server_error(const char *format, ...)
{
  CONTROL_EVENT_STATUS_BODY(EVENT_STATUS_SERVER, LOG_ERR);
  /* Force a flush, since we may be about to die horribly */
  queued_events_flush_all(1);
  return r;
}

/** Called when the status of an entry guard with the given <b>nickname</b>
 * and identity <b>digest</b> has changed to <b>status</b>: tells any
 * controllers that care. */
int
control_event_guard(const char *nickname, const char *digest,
                    const char *status)
{
  char hbuf[HEX_DIGEST_LEN+1];
  base16_encode(hbuf, sizeof(hbuf), digest, DIGEST_LEN);
  if (!EVENT_IS_INTERESTING(EVENT_GUARD))
    return 0;

  {
    char buf[MAX_VERBOSE_NICKNAME_LEN+1];
    const node_t *node = node_get_by_id(digest);
    if (node) {
      node_get_verbose_nickname(node, buf);
    } else {
      tor_snprintf(buf, sizeof(buf), "$%s~%s", hbuf, nickname);
    }
    send_control_event(EVENT_GUARD,
                       "650 GUARD ENTRY %s %s\r\n", buf, status);
  }
  return 0;
}

/** Called when a configuration option changes. This is generally triggered
 * by SETCONF requests and RELOAD/SIGHUP signals. The <b>changes</b> are
 * a linked list of configuration key-values.
 * <b>changes</b> can be NULL, meaning "no changes".
 */
void
control_event_conf_changed(const config_line_t *changes)
{
  char *result;
  smartlist_t *lines;
  if (!EVENT_IS_INTERESTING(EVENT_CONF_CHANGED) || !changes) {
    return;
  }
  lines = smartlist_new();
  for (const config_line_t *line = changes; line; line = line->next) {
    if (line->value == NULL) {
      smartlist_add_asprintf(lines, "650-%s", line->key);
    } else {
      smartlist_add_asprintf(lines, "650-%s=%s", line->key, line->value);
    }
  }
  result = smartlist_join_strings(lines, "\r\n", 0, NULL);
  send_control_event(EVENT_CONF_CHANGED,
    "650-CONF_CHANGED\r\n%s\r\n650 OK\r\n", result);
  tor_free(result);
  SMARTLIST_FOREACH(lines, char *, cp, tor_free(cp));
  smartlist_free(lines);
}

/** We just generated a new summary of which countries we've seen clients
 * from recently. Send a copy to the controller in case it wants to
 * display it for the user. */
void
control_event_clients_seen(const char *controller_str)
{
  send_control_event(EVENT_CLIENTS_SEEN,
    "650 CLIENTS_SEEN %s\r\n", controller_str);
}

/** A new pluggable transport called <b>transport_name</b> was
 *  launched on <b>addr</b>:<b>port</b>. <b>mode</b> is either
 *  "server" or "client" depending on the mode of the pluggable
 *  transport.
 *  "650" SP "TRANSPORT_LAUNCHED" SP Mode SP Name SP Address SP Port
 */
void
control_event_transport_launched(const char *mode, const char *transport_name,
                                 tor_addr_t *addr, uint16_t port)
{
  send_control_event(EVENT_TRANSPORT_LAUNCHED,
                     "650 TRANSPORT_LAUNCHED %s %s %s %u\r\n",
                     mode, transport_name, fmt_addr(addr), port);
}

/** A pluggable transport called <b>pt_name</b> has emitted a log message
 * found in <b>message</b> at <b>severity</b> log level. */
void
control_event_pt_log(const char *log)
{
  send_control_event(EVENT_PT_LOG,
                     "650 PT_LOG %s\r\n",
                     log);
}

/** A pluggable transport has emitted a STATUS message found in
 * <b>status</b>. */
void
control_event_pt_status(const char *status)
{
  send_control_event(EVENT_PT_STATUS,
                     "650 PT_STATUS %s\r\n",
                     status);
}

/** Convert rendezvous auth type to string for HS_DESC control events
 */
const char *
rend_auth_type_to_string(rend_auth_type_t auth_type)
{
  const char *str;

  switch (auth_type) {
    case REND_NO_AUTH:
      str = "NO_AUTH";
      break;
    case REND_BASIC_AUTH:
      str = "BASIC_AUTH";
      break;
    case REND_STEALTH_AUTH:
      str = "STEALTH_AUTH";
      break;
    default:
      str = "UNKNOWN";
  }

  return str;
}

/** Return either the onion address if the given pointer is a non empty
 * string else the unknown string. */
static const char *
rend_hsaddress_str_or_unknown(const char *onion_address)
{
  static const char *str_unknown = "UNKNOWN";
  const char *str_ret = str_unknown;

  /* No valid pointer, unknown it is. */
  if (!onion_address) {
    goto end;
  }
  /* Empty onion address thus we don't know, unknown it is. */
  if (onion_address[0] == '\0') {
    goto end;
  }
  /* All checks are good so return the given onion address. */
  str_ret = onion_address;

 end:
  return str_ret;
}

/** send HS_DESC requested event.
 *
 * <b>rend_query</b> is used to fetch requested onion address and auth type.
 * <b>hs_dir</b> is the description of contacting hs directory.
 * <b>desc_id_base32</b> is the ID of requested hs descriptor.
 * <b>hsdir_index</b> is the HSDir fetch index value for v3, an hex string.
 */
void
control_event_hs_descriptor_requested(const char *onion_address,
                                      rend_auth_type_t auth_type,
                                      const char *id_digest,
                                      const char *desc_id,
                                      const char *hsdir_index)
{
  char *hsdir_index_field = NULL;

  if (BUG(!id_digest || !desc_id)) {
    return;
  }

  if (hsdir_index) {
    tor_asprintf(&hsdir_index_field, " HSDIR_INDEX=%s", hsdir_index);
  }

  send_control_event(EVENT_HS_DESC,
                     "650 HS_DESC REQUESTED %s %s %s %s%s\r\n",
                     rend_hsaddress_str_or_unknown(onion_address),
                     rend_auth_type_to_string(auth_type),
                     node_describe_longname_by_id(id_digest),
                     desc_id,
                     hsdir_index_field ? hsdir_index_field : "");
  tor_free(hsdir_index_field);
}

/** send HS_DESC CREATED event when a local service generates a descriptor.
 *
 * <b>onion_address</b> is service address.
 * <b>desc_id</b> is the descriptor ID.
 * <b>replica</b> is the the descriptor replica number. If it is negative, it
 * is ignored.
 */
void
control_event_hs_descriptor_created(const char *onion_address,
                                    const char *desc_id,
                                    int replica)
{
  char *replica_field = NULL;

  if (BUG(!onion_address || !desc_id)) {
    return;
  }

  if (replica >= 0) {
    tor_asprintf(&replica_field, " REPLICA=%d", replica);
  }

  send_control_event(EVENT_HS_DESC,
                     "650 HS_DESC CREATED %s UNKNOWN UNKNOWN %s%s\r\n",
                     onion_address, desc_id,
                     replica_field ? replica_field : "");
  tor_free(replica_field);
}

/** send HS_DESC upload event.
 *
 * <b>onion_address</b> is service address.
 * <b>hs_dir</b> is the description of contacting hs directory.
 * <b>desc_id</b> is the ID of requested hs descriptor.
 */
void
control_event_hs_descriptor_upload(const char *onion_address,
                                   const char *id_digest,
                                   const char *desc_id,
                                   const char *hsdir_index)
{
  char *hsdir_index_field = NULL;

  if (BUG(!onion_address || !id_digest || !desc_id)) {
    return;
  }

  if (hsdir_index) {
    tor_asprintf(&hsdir_index_field, " HSDIR_INDEX=%s", hsdir_index);
  }

  send_control_event(EVENT_HS_DESC,
                     "650 HS_DESC UPLOAD %s UNKNOWN %s %s%s\r\n",
                     onion_address,
                     node_describe_longname_by_id(id_digest),
                     desc_id,
                     hsdir_index_field ? hsdir_index_field : "");
  tor_free(hsdir_index_field);
}

/** send HS_DESC event after got response from hs directory.
 *
 * NOTE: this is an internal function used by following functions:
 * control_event_hsv2_descriptor_received
 * control_event_hsv2_descriptor_failed
 * control_event_hsv3_descriptor_failed
 *
 * So do not call this function directly.
 */
static void
event_hs_descriptor_receive_end(const char *action,
                                const char *onion_address,
                                const char *desc_id,
                                rend_auth_type_t auth_type,
                                const char *hsdir_id_digest,
                                const char *reason)
{
  char *reason_field = NULL;

  if (BUG(!action || !onion_address)) {
    return;
  }

  if (reason) {
    tor_asprintf(&reason_field, " REASON=%s", reason);
  }

  send_control_event(EVENT_HS_DESC,
                     "650 HS_DESC %s %s %s %s%s%s\r\n",
                     action,
                     rend_hsaddress_str_or_unknown(onion_address),
                     rend_auth_type_to_string(auth_type),
                     hsdir_id_digest ?
                        node_describe_longname_by_id(hsdir_id_digest) :
                        "UNKNOWN",
                     desc_id ? desc_id : "",
                     reason_field ? reason_field : "");

  tor_free(reason_field);
}

/** send HS_DESC event after got response from hs directory.
 *
 * NOTE: this is an internal function used by following functions:
 * control_event_hs_descriptor_uploaded
 * control_event_hs_descriptor_upload_failed
 *
 * So do not call this function directly.
 */
void
control_event_hs_descriptor_upload_end(const char *action,
                                       const char *onion_address,
                                       const char *id_digest,
                                       const char *reason)
{
  char *reason_field = NULL;

  if (BUG(!action || !id_digest)) {
    return;
  }

  if (reason) {
    tor_asprintf(&reason_field, " REASON=%s", reason);
  }

  send_control_event(EVENT_HS_DESC,
                     "650 HS_DESC %s %s UNKNOWN %s%s\r\n",
                     action,
                     rend_hsaddress_str_or_unknown(onion_address),
                     node_describe_longname_by_id(id_digest),
                     reason_field ? reason_field : "");

  tor_free(reason_field);
}

/** For an HS descriptor query <b>rend_data</b>, using the
 * <b>onion_address</b> and HSDir fingerprint <b>hsdir_fp</b>, find out
 * which descriptor ID in the query is the right one.
 *
 * Return a pointer of the binary descriptor ID found in the query's object
 * or NULL if not found. */
static const char *
get_desc_id_from_query(const rend_data_t *rend_data, const char *hsdir_fp)
{
  int replica;
  const char *desc_id = NULL;
  const rend_data_v2_t *rend_data_v2 = TO_REND_DATA_V2(rend_data);

  /* Possible if the fetch was done using a descriptor ID. This means that
   * the HSFETCH command was used. */
  if (!tor_digest_is_zero(rend_data_v2->desc_id_fetch)) {
    desc_id = rend_data_v2->desc_id_fetch;
    goto end;
  }

  /* Without a directory fingerprint at this stage, we can't do much. */
  if (hsdir_fp == NULL) {
     goto end;
  }

  /* OK, we have an onion address so now let's find which descriptor ID
   * is the one associated with the HSDir fingerprint. */
  for (replica = 0; replica < REND_NUMBER_OF_NON_CONSECUTIVE_REPLICAS;
       replica++) {
    const char *digest = rend_data_get_desc_id(rend_data, replica, NULL);

    SMARTLIST_FOREACH_BEGIN(rend_data->hsdirs_fp, char *, fingerprint) {
      if (tor_memcmp(fingerprint, hsdir_fp, DIGEST_LEN) == 0) {
        /* Found it! This descriptor ID is the right one. */
        desc_id = digest;
        goto end;
      }
    } SMARTLIST_FOREACH_END(fingerprint);
  }

 end:
  return desc_id;
}

/** send HS_DESC RECEIVED event
 *
 * called when we successfully received a hidden service descriptor.
 */
void
control_event_hsv2_descriptor_received(const char *onion_address,
                                       const rend_data_t *rend_data,
                                       const char *hsdir_id_digest)
{
  char *desc_id_field = NULL;
  const char *desc_id;

  if (BUG(!rend_data || !hsdir_id_digest || !onion_address)) {
    return;
  }

  desc_id = get_desc_id_from_query(rend_data, hsdir_id_digest);
  if (desc_id != NULL) {
    char desc_id_base32[REND_DESC_ID_V2_LEN_BASE32 + 1];
    /* Set the descriptor ID digest to base32 so we can send it. */
    base32_encode(desc_id_base32, sizeof(desc_id_base32), desc_id,
                  DIGEST_LEN);
    /* Extra whitespace is needed before the value. */
    tor_asprintf(&desc_id_field, " %s", desc_id_base32);
  }

  event_hs_descriptor_receive_end("RECEIVED", onion_address, desc_id_field,
                                  TO_REND_DATA_V2(rend_data)->auth_type,
                                  hsdir_id_digest, NULL);
  tor_free(desc_id_field);
}

/* Send HS_DESC RECEIVED event
 *
 * Called when we successfully received a hidden service descriptor. */
void
control_event_hsv3_descriptor_received(const char *onion_address,
                                       const char *desc_id,
                                       const char *hsdir_id_digest)
{
  char *desc_id_field = NULL;

  if (BUG(!onion_address || !desc_id || !hsdir_id_digest)) {
    return;
  }

  /* Because DescriptorID is an optional positional value, we need to add a
   * whitespace before in order to not be next to the HsDir value. */
  tor_asprintf(&desc_id_field, " %s", desc_id);

  event_hs_descriptor_receive_end("RECEIVED", onion_address, desc_id_field,
                                  REND_NO_AUTH, hsdir_id_digest, NULL);
  tor_free(desc_id_field);
}

/** send HS_DESC UPLOADED event
 *
 * called when we successfully uploaded a hidden service descriptor.
 */
void
control_event_hs_descriptor_uploaded(const char *id_digest,
                                     const char *onion_address)
{
  if (BUG(!id_digest)) {
    return;
  }

  control_event_hs_descriptor_upload_end("UPLOADED", onion_address,
                                         id_digest, NULL);
}

/** Send HS_DESC event to inform controller that query <b>rend_data</b>
 * failed to retrieve hidden service descriptor from directory identified by
 * <b>id_digest</b>. If NULL, "UNKNOWN" is used. If <b>reason</b> is not NULL,
 * add it to REASON= field.
 */
void
control_event_hsv2_descriptor_failed(const rend_data_t *rend_data,
                                     const char *hsdir_id_digest,
                                     const char *reason)
{
  char *desc_id_field = NULL;
  const char *desc_id;

  if (BUG(!rend_data)) {
    return;
  }

  desc_id = get_desc_id_from_query(rend_data, hsdir_id_digest);
  if (desc_id != NULL) {
    char desc_id_base32[REND_DESC_ID_V2_LEN_BASE32 + 1];
    /* Set the descriptor ID digest to base32 so we can send it. */
    base32_encode(desc_id_base32, sizeof(desc_id_base32), desc_id,
                  DIGEST_LEN);
    /* Extra whitespace is needed before the value. */
    tor_asprintf(&desc_id_field, " %s", desc_id_base32);
  }

  event_hs_descriptor_receive_end("FAILED", rend_data_get_address(rend_data),
                                  desc_id_field,
                                  TO_REND_DATA_V2(rend_data)->auth_type,
                                  hsdir_id_digest, reason);
  tor_free(desc_id_field);
}

/** Send HS_DESC event to inform controller that the query to
 * <b>onion_address</b> failed to retrieve hidden service descriptor
 * <b>desc_id</b> from directory identified by <b>hsdir_id_digest</b>. If
 * NULL, "UNKNOWN" is used.  If <b>reason</b> is not NULL, add it to REASON=
 * field. */
void
control_event_hsv3_descriptor_failed(const char *onion_address,
                                     const char *desc_id,
                                     const char *hsdir_id_digest,
                                     const char *reason)
{
  char *desc_id_field = NULL;

  if (BUG(!onion_address || !desc_id || !reason)) {
    return;
  }

  /* Because DescriptorID is an optional positional value, we need to add a
   * whitespace before in order to not be next to the HsDir value. */
  tor_asprintf(&desc_id_field, " %s", desc_id);

  event_hs_descriptor_receive_end("FAILED", onion_address, desc_id_field,
                                  REND_NO_AUTH, hsdir_id_digest, reason);
  tor_free(desc_id_field);
}

/** Send HS_DESC_CONTENT event after completion of a successful fetch
 * from hs directory. If <b>hsdir_id_digest</b> is NULL, it is replaced
 * by "UNKNOWN". If <b>content</b> is NULL, it is replaced by an empty
 * string. The  <b>onion_address</b> or <b>desc_id</b> set to NULL will
 * not trigger the control event. */
void
control_event_hs_descriptor_content(const char *onion_address,
                                    const char *desc_id,
                                    const char *hsdir_id_digest,
                                    const char *content)
{
  static const char *event_name = "HS_DESC_CONTENT";
  char *esc_content = NULL;

  if (!onion_address || !desc_id) {
    log_warn(LD_BUG, "Called with onion_address==%p, desc_id==%p, ",
             onion_address, desc_id);
    return;
  }

  if (content == NULL) {
    /* Point it to empty content so it can still be escaped. */
    content = "";
  }
  write_escaped_data(content, strlen(content), &esc_content);

  send_control_event(EVENT_HS_DESC_CONTENT,
                     "650+%s %s %s %s\r\n%s650 OK\r\n",
                     event_name,
                     rend_hsaddress_str_or_unknown(onion_address),
                     desc_id,
                     hsdir_id_digest ?
                        node_describe_longname_by_id(hsdir_id_digest) :
                        "UNKNOWN",
                     esc_content);
  tor_free(esc_content);
}

/** Send HS_DESC event to inform controller upload of hidden service
 * descriptor identified by <b>id_digest</b> failed. If <b>reason</b>
 * is not NULL, add it to REASON= field.
 */
void
control_event_hs_descriptor_upload_failed(const char *id_digest,
                                          const char *onion_address,
                                          const char *reason)
{
  if (BUG(!id_digest)) {
    return;
  }
  control_event_hs_descriptor_upload_end("FAILED", onion_address,
                                         id_digest, reason);
}

void
control_events_free_all(void)
{
  smartlist_t *queued_events = NULL;

  stats_prev_n_read = stats_prev_n_written = 0;

  if (queued_control_events_lock) {
    tor_mutex_acquire(queued_control_events_lock);
    flush_queued_event_pending = 0;
    queued_events = queued_control_events;
    queued_control_events = NULL;
    tor_mutex_release(queued_control_events_lock);
  }
  if (queued_events) {
    SMARTLIST_FOREACH(queued_events, queued_event_t *, ev,
                      queued_event_free(ev));
    smartlist_free(queued_events);
  }
  if (flush_queued_events_event) {
    mainloop_event_free(flush_queued_events_event);
    flush_queued_events_event = NULL;
  }
  global_event_mask = 0;
  disable_log_messages = 0;
}

#ifdef TOR_UNIT_TESTS
/* For testing: change the value of global_event_mask */
void
control_testing_set_global_event_mask(uint64_t mask)
{
  global_event_mask = mask;
}
#endif /* defined(TOR_UNIT_TESTS) */
