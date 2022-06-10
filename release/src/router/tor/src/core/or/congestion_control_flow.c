/* Copyright (c) 2019-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file congestion_control_flow.c
 * \brief Code that implements flow control for congestion controlled
 *        circuits.
 */

#define TOR_CONGESTION_CONTROL_FLOW_PRIVATE

#include "core/or/or.h"

#include "core/or/relay.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_edge.h"
#include "core/mainloop/mainloop.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_flow.h"
#include "core/or/congestion_control_st.h"
#include "core/or/circuitlist.h"
#include "core/or/trace_probes_cc.h"
#include "feature/nodelist/networkstatus.h"
#include "trunnel/flow_control_cells.h"
#include "feature/control/control_events.h"

#include "core/or/connection_st.h"
#include "core/or/cell_st.h"
#include "app/config/config.h"

/** Cache consensus parameters */
static uint32_t xoff_client;
static uint32_t xoff_exit;

static uint32_t xon_change_pct;
static uint32_t xon_ewma_cnt;
static uint32_t xon_rate_bytes;

/* In normal operation, we can get a burst of up to 32 cells before returning
 * to libevent to flush the outbuf. This is a heuristic from hardcoded values
 * and strange logic in connection_bucket_get_share(). */
#define MAX_EXPECTED_CELL_BURST 32

/* The following three are for dropmark rate limiting. They define when we
 * scale down our XON, XOFF, and xmit byte counts. Early scaling is beneficial
 * because it limits the ability of spurious XON/XOFF to be sent after large
 * amounts of data without XON/XOFF. At these limits, after 10MB of data (or
 * more), an adversary can only inject (log2(10MB)-log2(200*500))*100 ~= 1000
 * cells of fake XOFF/XON before the xmit byte count will be halved enough to
 * triggering a limit. */
#define XON_COUNT_SCALE_AT 200
#define XOFF_COUNT_SCALE_AT 200
#define ONE_MEGABYTE (UINT64_C(1) << 20)
#define TOTAL_XMIT_SCALE_AT (10 * ONE_MEGABYTE)

/**
 * Return the congestion control object of the given edge connection.
 *
 * Returns NULL if the edge connection doesn't have a cpath_layer or not
 * attached to a circuit. But also if the cpath_layer or circuit doesn't have a
 * congestion control object.
 */
static inline const congestion_control_t *
edge_get_ccontrol(const edge_connection_t *edge)
{
  congestion_control_t *ccontrol = NULL;

  if (edge->on_circuit && edge->on_circuit->ccontrol) {
    ccontrol = edge->on_circuit->ccontrol;
  } else if (edge->cpath_layer && edge->cpath_layer->ccontrol) {
    ccontrol = edge->cpath_layer->ccontrol;
  }

  return ccontrol;
}

/**
 * Update global congestion control related consensus parameter values, every
 * consensus update.
 *
 * More details for each of the parameters can be found in proposal 324,
 * section 6.5 including tuning notes.
 */
void
flow_control_new_consensus_params(const networkstatus_t *ns)
{
#define CC_XOFF_CLIENT_DFLT 500
#define CC_XOFF_CLIENT_MIN 1
#define CC_XOFF_CLIENT_MAX 10000
  xoff_client = networkstatus_get_param(ns, "cc_xoff_client",
      CC_XOFF_CLIENT_DFLT,
      CC_XOFF_CLIENT_MIN,
      CC_XOFF_CLIENT_MAX)*RELAY_PAYLOAD_SIZE;

#define CC_XOFF_EXIT_DFLT 500
#define CC_XOFF_EXIT_MIN 1
#define CC_XOFF_EXIT_MAX 10000
  xoff_exit = networkstatus_get_param(ns, "cc_xoff_exit",
      CC_XOFF_EXIT_DFLT,
      CC_XOFF_EXIT_MIN,
      CC_XOFF_EXIT_MAX)*RELAY_PAYLOAD_SIZE;

#define CC_XON_CHANGE_PCT_DFLT 25
#define CC_XON_CHANGE_PCT_MIN 1
#define CC_XON_CHANGE_PCT_MAX 99
  xon_change_pct = networkstatus_get_param(ns, "cc_xon_change_pct",
      CC_XON_CHANGE_PCT_DFLT,
      CC_XON_CHANGE_PCT_MIN,
      CC_XON_CHANGE_PCT_MAX);

#define CC_XON_RATE_BYTES_DFLT (500)
#define CC_XON_RATE_BYTES_MIN (1)
#define CC_XON_RATE_BYTES_MAX (5000)
  xon_rate_bytes = networkstatus_get_param(ns, "cc_xon_rate",
      CC_XON_RATE_BYTES_DFLT,
      CC_XON_RATE_BYTES_MIN,
      CC_XON_RATE_BYTES_MAX)*RELAY_PAYLOAD_SIZE;

#define CC_XON_EWMA_CNT_DFLT (2)
#define CC_XON_EWMA_CNT_MIN (2)
#define CC_XON_EWMA_CNT_MAX (100)
  xon_ewma_cnt = networkstatus_get_param(ns, "cc_xon_ewma_cnt",
      CC_XON_EWMA_CNT_DFLT,
      CC_XON_EWMA_CNT_MIN,
      CC_XON_EWMA_CNT_MAX);
}

/**
 * Send an XOFF for this stream, and note that we sent one
 */
static void
circuit_send_stream_xoff(edge_connection_t *stream)
{
  xoff_cell_t xoff;
  uint8_t payload[CELL_PAYLOAD_SIZE];
  ssize_t xoff_size;

  memset(&xoff, 0, sizeof(xoff));
  memset(payload, 0, sizeof(payload));

  xoff_cell_set_version(&xoff, 0);

  if ((xoff_size = xoff_cell_encode(payload, CELL_PAYLOAD_SIZE, &xoff)) < 0) {
    log_warn(LD_BUG, "Failed to encode xon cell");
    return;
  }

  if (connection_edge_send_command(stream, RELAY_COMMAND_XOFF,
                               (char*)payload, (size_t)xoff_size) == 0) {
    stream->xoff_sent = true;

    /* If this is an entry conn, notify control port */
    if (TO_CONN(stream)->type == CONN_TYPE_AP) {
      control_event_stream_status(TO_ENTRY_CONN(TO_CONN(stream)),
                                  STREAM_EVENT_XOFF_SENT,
                                  0);
    }
  }
}

/**
 * Compute the recent drain rate (write rate) for this edge
 * connection and return it, in KB/sec (1000 bytes/sec).
 *
 * Returns 0 if the monotime clock is busted.
 */
static inline uint32_t
compute_drain_rate(const edge_connection_t *stream)
{
  if (BUG(!is_monotime_clock_reliable())) {
    log_warn(LD_BUG, "Computing drain rate with stalled monotime clock");
    return 0;
  }

  uint64_t delta = monotime_absolute_usec() - stream->drain_start_usec;

  if (delta == 0) {
    log_warn(LD_BUG, "Computing stream drain rate with zero time delta");
    return 0;
  }

  /* Overflow checks */
  if (stream->prev_drained_bytes > INT32_MAX/1000 || /* Intermediate */
      stream->prev_drained_bytes/delta > INT32_MAX/1000) { /* full value */
    return INT32_MAX;
  }

  /* kb/sec = bytes/usec * 1000 usec/msec * 1000 msec/sec * kb/1000bytes */
  return MAX(1, (uint32_t)(stream->prev_drained_bytes * 1000)/delta);
}

/**
 * Send an XON for this stream, with appropriate advisory rate information.
 *
 * Reverts the xoff sent status, and stores the rate information we sent,
 * in case it changes.
 */
static void
circuit_send_stream_xon(edge_connection_t *stream)
{
  xon_cell_t xon;
  uint8_t payload[CELL_PAYLOAD_SIZE];
  ssize_t xon_size;

  memset(&xon, 0, sizeof(xon));
  memset(payload, 0, sizeof(payload));

  xon_cell_set_version(&xon, 0);
  xon_cell_set_kbps_ewma(&xon, stream->ewma_drain_rate);

  if ((xon_size = xon_cell_encode(payload, CELL_PAYLOAD_SIZE, &xon)) < 0) {
    log_warn(LD_BUG, "Failed to encode xon cell");
    return;
  }

  /* Store the advisory rate information, to send advisory updates if
   * it changes */
  stream->ewma_rate_last_sent = stream->ewma_drain_rate;

  if (connection_edge_send_command(stream, RELAY_COMMAND_XON, (char*)payload,
                                   (size_t)xon_size) == 0) {
    /* Revert the xoff sent status, so we can send another one if need be */
    stream->xoff_sent = false;

    /* If it's an entry conn, notify control port */
    if (TO_CONN(stream)->type == CONN_TYPE_AP) {
      control_event_stream_status(TO_ENTRY_CONN(TO_CONN(stream)),
                                  STREAM_EVENT_XON_SENT,
                                  0);
    }
  }
}

/**
 * Process a stream XOFF, parsing it, and then stopping reading on
 * the edge connection.
 *
 * Record that we have received an xoff, so we know not to resume
 * reading on this edge conn until we get an XON.
 *
 * Returns false if the XOFF did not validate; true if it does.
 */
bool
circuit_process_stream_xoff(edge_connection_t *conn,
                            const crypt_path_t *layer_hint,
                            const cell_t *cell)
{
  (void)cell;
  bool retval = true;

  if (BUG(!conn)) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
           "Got XOFF on invalid stream?");
    return false;
  }

  /* Make sure this XOFF came from the right hop */
  if (layer_hint && layer_hint != conn->cpath_layer) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
            "Got XOFF from wrong hop.");
    return false;
  }

  if (edge_get_ccontrol(conn) == NULL) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
           "Got XOFF for non-congestion control circuit");
    return false;
  }

  if (conn->xoff_received) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
           "Got multiple XOFF on connection");
    return false;
  }

  /* If we are near the max, scale everything down */
  if (conn->num_xoff_recv == XOFF_COUNT_SCALE_AT) {
    log_info(LD_EDGE, "Scaling down for XOFF count: %d %d %d",
             conn->total_bytes_xmit,
             conn->num_xoff_recv,
             conn->num_xon_recv);
    conn->total_bytes_xmit /= 2;
    conn->num_xoff_recv /= 2;
    conn->num_xon_recv /= 2;
  }

  conn->num_xoff_recv++;

  /* Client-side check to make sure that XOFF is not sent too early,
   * for dropmark attacks. The main sidechannel risk is early cells,
   * but we also check to make sure that we have not received more XOFFs
   * than could have been generated by the bytes we sent.
   */
  if (TO_CONN(conn)->type == CONN_TYPE_AP || conn->hs_ident != NULL) {
    uint32_t limit = 0;
    if (conn->hs_ident)
      limit = xoff_client;
    else
      limit = xoff_exit;

    if (conn->total_bytes_xmit < limit*conn->num_xoff_recv) {
      log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
             "Got extra XOFF for bytes sent. Got %d, expected max %d",
             conn->num_xoff_recv, conn->total_bytes_xmit/limit);
      /* We still process this, because the only dropmark defenses
       * in C tor are via the vanguards addon's use of the read valid
       * cells. So just signal that we think this is not valid protocol
       * data and proceed. */
      retval = false;
    }
  }

  log_info(LD_EDGE, "Got XOFF!");
  connection_stop_reading(TO_CONN(conn));
  conn->xoff_received = true;

  /* If this is an entry conn, notify control port */
  if (TO_CONN(conn)->type == CONN_TYPE_AP) {
    control_event_stream_status(TO_ENTRY_CONN(TO_CONN(conn)),
                                STREAM_EVENT_XOFF_RECV,
                                0);
  }

  return retval;
}

/**
 * Process a stream XON, and if it validates, clear the xoff
 * flag and resume reading on this edge connection.
 *
 * Also, use provided rate information to rate limit
 * reading on this edge (or packagaing from it onto
 * the circuit), to avoid XON/XOFF chatter.
 *
 * Returns true if the XON validates, false otherwise.
 */
bool
circuit_process_stream_xon(edge_connection_t *conn,
                           const crypt_path_t *layer_hint,
                           const cell_t *cell)
{
  xon_cell_t *xon;
  bool retval = true;

  if (BUG(!conn)) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
           "Got XON on invalid stream?");
    return false;
  }

  /* Make sure this XON came from the right hop */
  if (layer_hint && layer_hint != conn->cpath_layer) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
           "Got XON from wrong hop.");
    return false;
  }

  if (edge_get_ccontrol(conn) == NULL) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
           "Got XON for non-congestion control circuit");
    return false;
  }

  if (xon_cell_parse(&xon, cell->payload+RELAY_HEADER_SIZE,
                     CELL_PAYLOAD_SIZE-RELAY_HEADER_SIZE) < 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
          "Received malformed XON cell.");
    return false;
  }

  /* If we are near the max, scale everything down */
  if (conn->num_xon_recv == XON_COUNT_SCALE_AT) {
    log_info(LD_EDGE, "Scaling down for XON count: %d %d %d",
             conn->total_bytes_xmit,
             conn->num_xoff_recv,
             conn->num_xon_recv);
    conn->total_bytes_xmit /= 2;
    conn->num_xoff_recv /= 2;
    conn->num_xon_recv /= 2;
  }

  conn->num_xon_recv++;

  /* Client-side check to make sure that XON is not sent too early,
   * for dropmark attacks. The main sidechannel risk is early cells,
   * but we also check to see that we did not get more XONs than make
   * sense for the number of bytes we sent.
   */
  if (TO_CONN(conn)->type == CONN_TYPE_AP || conn->hs_ident != NULL) {
    uint32_t limit = 0;

    if (conn->hs_ident)
      limit = MIN(xoff_client, xon_rate_bytes);
    else
      limit = MIN(xoff_exit, xon_rate_bytes);

    if (conn->total_bytes_xmit < limit*conn->num_xon_recv) {
      log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
             "Got extra XON for bytes sent. Got %d, expected max %d",
             conn->num_xon_recv, conn->total_bytes_xmit/limit);

      /* We still process this, because the only dropmark defenses
       * in C tor are via the vanguards addon's use of the read valid
       * cells. So just signal that we think this is not valid protocol
       * data and proceed. */
      retval = false;
    }
  }

  log_info(LD_EDGE, "Got XON: %d", xon->kbps_ewma);

  /* Adjust the token bucket of this edge connection with the drain rate in
   * the XON. Rate is in bytes from kilobit (kpbs). */
  uint64_t rate = ((uint64_t) xon_cell_get_kbps_ewma(xon) * 1000);
  if (rate == 0 || INT32_MAX < rate) {
    /* No rate. */
    rate = INT32_MAX;
  }
  token_bucket_rw_adjust(&conn->bucket, (uint32_t) rate, (uint32_t) rate);

  if (conn->xoff_received) {
    /* Clear the fact that we got an XOFF, so that this edge can
     * start and stop reading normally */
    conn->xoff_received = false;
    connection_start_reading(TO_CONN(conn));
  }

  /* If this is an entry conn, notify control port */
  if (TO_CONN(conn)->type == CONN_TYPE_AP) {
    control_event_stream_status(TO_ENTRY_CONN(TO_CONN(conn)),
                                STREAM_EVENT_XON_RECV,
                                0);
  }

  xon_cell_free(xon);

  return retval;
}

/**
 * Called from sendme_stream_data_received(), when data arrives
 * from a circuit to our edge's outbuf, to decide if we need to send
 * an XOFF.
 *
 * Returns the amount of cells remaining until the buffer is full, at
 * which point it sends an XOFF, and returns 0.
 *
 * Returns less than 0 if we have queued more than a congestion window
 * worth of data and need to close the circuit.
 */
int
flow_control_decide_xoff(edge_connection_t *stream)
{
  size_t total_buffered = connection_get_outbuf_len(TO_CONN(stream));
  uint32_t buffer_limit_xoff = 0;

  if (BUG(edge_get_ccontrol(stream) == NULL)) {
    log_err(LD_BUG, "Flow control called for non-congestion control circuit");
    return -1;
  }

  /* Onion services and clients are typically localhost edges, so they
   * need different buffering limits than exits do */
  if (TO_CONN(stream)->type == CONN_TYPE_AP || stream->hs_ident != NULL) {
    buffer_limit_xoff = xoff_client;
  } else {
    buffer_limit_xoff = xoff_exit;
  }

  if (total_buffered > buffer_limit_xoff) {
    if (!stream->xoff_sent) {
      log_info(LD_EDGE, "Sending XOFF: %"TOR_PRIuSZ" %d",
                 total_buffered, buffer_limit_xoff);
      tor_trace(TR_SUBSYS(cc), TR_EV(flow_decide_xoff_sending), stream);

      circuit_send_stream_xoff(stream);

      /* Clear the drain rate. It is considered wrong if we
       * got all the way to XOFF */
      stream->ewma_drain_rate = 0;
    }
  }

  /* If the outbuf has accumulated more than the expected burst limit of
   * cells, then assume it is not draining, and call decide_xon. We must
   * do this because writes only happen when the socket unblocks, so
   * may not otherwise notice accumulation of data in the outbuf for
   * advisory XONs. */
   if (total_buffered > MAX_EXPECTED_CELL_BURST*RELAY_PAYLOAD_SIZE) {
     flow_control_decide_xon(stream, 0);
   }

  /* Flow control always takes more data; we rely on the oomkiller to
   * handle misbehavior. */
  return 0;
}

/**
 * Returns true if the stream's drain rate has changed significantly.
 *
 * Returns false if the monotime clock is stalled, or if we have
 * no previous drain rate information.
 */
static bool
stream_drain_rate_changed(const edge_connection_t *stream)
{
  if (!is_monotime_clock_reliable()) {
    return false;
  }

  if (!stream->ewma_rate_last_sent) {
    return false;
  }

  if (stream->ewma_drain_rate >
      (100+(uint64_t)xon_change_pct)*stream->ewma_rate_last_sent/100) {
    return true;
  }

  if (stream->ewma_drain_rate <
      (100-(uint64_t)xon_change_pct)*stream->ewma_rate_last_sent/100) {
    return true;
  }

  return false;
}

/**
 * Called whenever we drain an edge connection outbuf by writing on
 * its socket, to decide if it is time to send an xon.
 *
 * The n_written parameter tells us how many bytes we have written
 * this time, which is used to compute the advisory drain rate fields.
 */
void
flow_control_decide_xon(edge_connection_t *stream, size_t n_written)
{
  size_t total_buffered = connection_get_outbuf_len(TO_CONN(stream));

  /* Bounds check the number of drained bytes, and scale */
  if (stream->drained_bytes >= UINT32_MAX - n_written) {
    /* Cut the bytes in half, and move the start time up halfway to now
     * (if we have one). */
    stream->drained_bytes /= 2;

    if (stream->drain_start_usec) {
        uint64_t now = monotime_absolute_usec();

        stream->drain_start_usec = now - (now-stream->drain_start_usec)/2;
    }
  }

  /* Accumulate drained bytes since last rate computation */
  stream->drained_bytes += n_written;

  tor_trace(TR_SUBSYS(cc), TR_EV(flow_decide_xon), stream, n_written);

  /* Check for bad monotime clock and bytecount wrap */
  if (!is_monotime_clock_reliable()) {
    /* If the monotime clock ever goes wrong, the safest thing to do
     * is just clear our short-term rate info and wait for the clock to
     * become reliable again.. */
    stream->drain_start_usec = 0;
    stream->drained_bytes = 0;
  } else {
    /* If we have no drain start timestamp, and we still have
     * remaining buffer, start the buffering counter */
    if (!stream->drain_start_usec && total_buffered > 0) {
      log_debug(LD_EDGE, "Began edge buffering: %d %d %"TOR_PRIuSZ,
                 stream->ewma_rate_last_sent,
                 stream->ewma_drain_rate,
                 total_buffered);
      tor_trace(TR_SUBSYS(cc), TR_EV(flow_decide_xon_drain_start),
                stream);
      stream->drain_start_usec = monotime_absolute_usec();
      stream->drained_bytes = 0;
    }
  }

  if (stream->drain_start_usec) {
    /* If we have spent enough time in a queued state, update our drain
     * rate. */
    if (stream->drained_bytes > xon_rate_bytes) {
      /* No previous drained bytes means it is the first time we are computing
       * it so use the value we just drained onto the socket as a baseline. It
       * won't be accurate but it will be a start towards the right value.
       *
       * We have to do this in order to have a drain rate else we could be
       * sending a drain rate of 0 in an XON which would be undesirable and
       * basically like sending an XOFF. */
      if (stream->prev_drained_bytes == 0) {
        stream->prev_drained_bytes = stream->drained_bytes;
      }
      uint32_t drain_rate = compute_drain_rate(stream);
      /* Once the drain rate has been computed, note how many bytes we just
       * drained so it can be used at the next calculation. We do this here
       * because it gets reset once the rate is changed. */
      stream->prev_drained_bytes = stream->drained_bytes;

      if (drain_rate) {
        stream->ewma_drain_rate =
            (uint32_t)n_count_ewma(drain_rate,
                                   stream->ewma_drain_rate,
                                   xon_ewma_cnt);
        log_debug(LD_EDGE, "Updating drain rate: %d %d %"TOR_PRIuSZ,
                   drain_rate,
                   stream->ewma_drain_rate,
                   total_buffered);
        tor_trace(TR_SUBSYS(cc), TR_EV(flow_decide_xon_drain_update),
                  stream, drain_rate);
        /* Reset recent byte counts. This prevents us from sending advisory
         * XONs more frequent than every xon_rate_bytes. */
        stream->drained_bytes = 0;
        stream->drain_start_usec = 0;
      }
    }
  }

  /* If we don't have an XOFF outstanding, consider updating an
   * old rate */
  if (!stream->xoff_sent) {
    if (stream_drain_rate_changed(stream)) {
      /* If we are still buffering and the rate changed, update
       * advisory XON */
      log_info(LD_EDGE, "Sending rate-change XON: %d %d %"TOR_PRIuSZ,
                 stream->ewma_rate_last_sent,
                 stream->ewma_drain_rate,
                 total_buffered);
      tor_trace(TR_SUBSYS(cc), TR_EV(flow_decide_xon_rate_change), stream);
      circuit_send_stream_xon(stream);
    }
  } else if (total_buffered == 0) {
    log_info(LD_EDGE, "Sending XON: %d %d %"TOR_PRIuSZ,
               stream->ewma_rate_last_sent,
               stream->ewma_drain_rate,
               total_buffered);
    tor_trace(TR_SUBSYS(cc), TR_EV(flow_decide_xon_partial_drain), stream);
    circuit_send_stream_xon(stream);
  }

  /* If the buffer has fully emptied, clear the drain timestamp,
   * so we can total only bytes drained while outbuf is 0. */
  if (total_buffered == 0) {
    stream->drain_start_usec = 0;

    /* After we've spent 'xon_rate_bytes' with the queue fully drained,
     * double any rate we sent. */
    if (stream->drained_bytes >= xon_rate_bytes &&
        stream->ewma_rate_last_sent) {
      stream->ewma_drain_rate = MIN(INT32_MAX, 2*stream->ewma_drain_rate);

      log_debug(LD_EDGE,
                 "Queue empty for xon_rate_limit bytes: %d %d",
                 stream->ewma_rate_last_sent,
                 stream->ewma_drain_rate);
      tor_trace(TR_SUBSYS(cc), TR_EV(flow_decide_xon_drain_doubled), stream);
      /* Resetting the drained bytes count. We need to keep its value as a
       * previous so the drain rate calculation takes into account what was
       * actually drain the last time. */
      stream->prev_drained_bytes = stream->drained_bytes;
      stream->drained_bytes = 0;
    }
  }

  return;
}

/**
 * Note that we packaged some data on this stream. Used to enforce
 * client-side dropmark limits
 */
void
flow_control_note_sent_data(edge_connection_t *stream, size_t len)
{
  /* If we are near the max, scale everything down */
  if (stream->total_bytes_xmit >= TOTAL_XMIT_SCALE_AT-len) {
    log_info(LD_EDGE, "Scaling down for flow control xmit bytes:: %d %d %d",
             stream->total_bytes_xmit,
             stream->num_xoff_recv,
             stream->num_xon_recv);

    stream->total_bytes_xmit /= 2;
    stream->num_xoff_recv /= 2;
    stream->num_xon_recv /= 2;
  }

  stream->total_bytes_xmit += len;
}

/** Returns true if an edge connection uses flow control */
bool
edge_uses_flow_control(const edge_connection_t *stream)
{
  bool ret = (stream->on_circuit && stream->on_circuit->ccontrol) ||
             (stream->cpath_layer && stream->cpath_layer->ccontrol);

  /* All circuits with congestion control use flow control */
  return ret;
}

/**
 * Returns the max RTT for the circuit that carries this stream,
 * as observed by congestion control.
 */
uint64_t
edge_get_max_rtt(const edge_connection_t *stream)
{
  if (stream->on_circuit && stream->on_circuit->ccontrol)
    return stream->on_circuit->ccontrol->max_rtt_usec;
  else if (stream->cpath_layer && stream->cpath_layer->ccontrol)
    return stream->cpath_layer->ccontrol->max_rtt_usec;

  return 0;
}

/** Returns true if a connection is an edge conn that uses flow control */
bool
conn_uses_flow_control(connection_t *conn)
{
  bool ret = false;

  if (CONN_IS_EDGE(conn)) {
    edge_connection_t *edge = TO_EDGE_CONN(conn);

    if (edge_uses_flow_control(edge)) {
      ret = true;
    }
  }

  return ret;
}

