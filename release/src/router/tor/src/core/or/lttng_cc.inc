/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file lttng_cc.inc
 * \brief LTTng tracing probe declaration for the congestion control subsystem.
 *        It is in this .inc file due to the non C standard syntax and the way
 *        we guard the header with the LTTng specific
 *        TRACEPOINT_HEADER_MULTI_READ.
 **/

#include "orconfig.h"

/* We only build the following if LTTng instrumentation has been enabled. */
#ifdef USE_TRACING_INSTRUMENTATION_LTTNG

/* The following defines are LTTng-UST specific. */
#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER tor_cc

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./src/core/or/lttng_cc.inc"

#if !defined(LTTNG_CC_INC) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define LTTNG_CC_INC

#include <lttng/tracepoint.h>

/*
 * Flow Control
 */

/* Emitted every time the flow_control_decide_xon() function is called. */
TRACEPOINT_EVENT(tor_cc, flow_decide_xon,
  TP_ARGS(const edge_connection_t *, stream, size_t, n_written),
  TP_FIELDS(
    ctf_integer(uint64_t, stream_id, TO_CONN(stream)->global_identifier)
    ctf_integer(size_t, written_bytes, n_written)
    ctf_integer(uint32_t, drained_bytes_current, stream->drained_bytes)
    ctf_integer(uint32_t, drained_bytes_previous, stream->prev_drained_bytes)
    ctf_integer(uint32_t, ewma_drain_rate_last, stream->ewma_rate_last_sent)
    ctf_integer(uint32_t, ewma_drain_rate_current, stream->ewma_drain_rate)
    ctf_integer(size_t, outbuf_len,
                connection_get_outbuf_len(TO_CONN(stream)))
  )
)

/* Emitted when flow control starts measuring the drain rate. */
TRACEPOINT_EVENT(tor_cc, flow_decide_xon_drain_start,
  TP_ARGS(const edge_connection_t *, stream),
  TP_FIELDS(
    ctf_integer(uint64_t, stream_id, TO_CONN(stream)->global_identifier)
    ctf_integer(uint32_t, drained_bytes_current, stream->drained_bytes)
    ctf_integer(uint32_t, drained_bytes_previous, stream->prev_drained_bytes)
    ctf_integer(uint32_t, ewma_drain_rate_last, stream->ewma_rate_last_sent)
    ctf_integer(uint32_t, ewma_drain_rate_current, stream->ewma_drain_rate)
    ctf_integer(size_t, outbuf_len,
                connection_get_outbuf_len(TO_CONN(stream)))
  )
)

/* Emitted when the drain rate is updated. The new_drain_rate value is what was
 * just computed. */
TRACEPOINT_EVENT(tor_cc, flow_decide_xon_drain_update,
  TP_ARGS(const edge_connection_t *, stream, uint32_t, drain_rate),
  TP_FIELDS(
    ctf_integer(uint64_t, stream_id, TO_CONN(stream)->global_identifier)
    ctf_integer(uint32_t, drained_bytes_current, stream->drained_bytes)
    ctf_integer(uint32_t, drained_bytes_previous, stream->prev_drained_bytes)
    ctf_integer(uint32_t, new_drain_rate, drain_rate)
    ctf_integer(uint32_t, ewma_drain_rate_last, stream->ewma_rate_last_sent)
    ctf_integer(uint32_t, ewma_drain_rate_current, stream->ewma_drain_rate)
    ctf_integer(size_t, outbuf_len,
                connection_get_outbuf_len(TO_CONN(stream)))
  )
)

/* Emitted when an XON cell is sent due to a notice in a drain rate change. */
TRACEPOINT_EVENT(tor_cc, flow_decide_xon_rate_change,
  TP_ARGS(const edge_connection_t *, stream),
  TP_FIELDS(
    ctf_integer(uint64_t, stream_id, TO_CONN(stream)->global_identifier)
    ctf_integer(uint32_t, drained_bytes_current, stream->drained_bytes)
    ctf_integer(uint32_t, drained_bytes_previous, stream->prev_drained_bytes)
    ctf_integer(uint32_t, ewma_drain_rate_last, stream->ewma_rate_last_sent)
    ctf_integer(uint32_t, ewma_drain_rate_current, stream->ewma_drain_rate)
    ctf_integer(size_t, outbuf_len,
                connection_get_outbuf_len(TO_CONN(stream)))
  )
)

/* Emitted when an XON cell is sent because we partially or fully drained the
 * edge connection buffer. */
TRACEPOINT_EVENT(tor_cc, flow_decide_xon_partial_drain,
  TP_ARGS(const edge_connection_t *, stream),
  TP_FIELDS(
    ctf_integer(uint64_t, stream_id, TO_CONN(stream)->global_identifier)
    ctf_integer(uint32_t, drained_bytes_current, stream->drained_bytes)
    ctf_integer(uint32_t, drained_bytes_previous, stream->prev_drained_bytes)
    ctf_integer(uint32_t, ewma_drain_rate_last, stream->ewma_rate_last_sent)
    ctf_integer(uint32_t, ewma_drain_rate_current, stream->ewma_drain_rate)
    ctf_integer(size_t, outbuf_len,
                connection_get_outbuf_len(TO_CONN(stream)))
  )
)

/* Emitted when we double the drain rate which is an attempt to see if we can
 * speed things up. */
TRACEPOINT_EVENT(tor_cc, flow_decide_xon_drain_doubled,
  TP_ARGS(const edge_connection_t *, stream),
  TP_FIELDS(
    ctf_integer(uint64_t, stream_id, TO_CONN(stream)->global_identifier)
    ctf_integer(uint32_t, drained_bytes_current, stream->drained_bytes)
    ctf_integer(uint32_t, drained_bytes_previous, stream->prev_drained_bytes)
    ctf_integer(uint32_t, ewma_drain_rate_last, stream->ewma_rate_last_sent)
    ctf_integer(uint32_t, ewma_drain_rate_current, stream->ewma_drain_rate)
    ctf_integer(size_t, outbuf_len,
                connection_get_outbuf_len(TO_CONN(stream)))
  )
)

/* XOFF */

/* Emitted when we send an XOFF cell. */
TRACEPOINT_EVENT(tor_cc, flow_decide_xoff_sending,
  TP_ARGS(const edge_connection_t *, stream),
  TP_FIELDS(
    ctf_integer(uint64_t, stream_id, TO_CONN(stream)->global_identifier)
    ctf_integer(uint32_t, drained_bytes_current, stream->drained_bytes)
    ctf_integer(uint32_t, drained_bytes_previous, stream->prev_drained_bytes)
    ctf_integer(uint32_t, ewma_drain_rate_last, stream->ewma_rate_last_sent)
    ctf_integer(uint32_t, ewma_drain_rate_current, stream->ewma_drain_rate)
    ctf_integer(size_t, outbuf_len,
                connection_get_outbuf_len(TO_CONN(stream)))
  )
)

/*
 * Congestion Control
 */

/* Emitted when the BDP value has been updated. */
TRACEPOINT_EVENT(tor_cc, bdp_update,
  TP_ARGS(const circuit_t *, circ, const congestion_control_t *, cc,
          uint64_t, curr_rtt_usec, uint64_t, sendme_rate_bdp),
  TP_FIELDS(
    ctf_integer(uint64_t, circuit_ptr, circ)
    ctf_integer(uint32_t, n_circ_id, circ->n_circ_id)
    ctf_integer(uint64_t, min_rtt_usec, cc->min_rtt_usec)
    ctf_integer(uint64_t, curr_rtt_usec, curr_rtt_usec)
    ctf_integer(uint64_t, ewma_rtt_usec, cc->ewma_rtt_usec)
    ctf_integer(uint64_t, max_rtt_usec, cc->max_rtt_usec)
    ctf_integer(uint64_t, bdp_inflight_rtt, cc->bdp[BDP_ALG_INFLIGHT_RTT])
    ctf_integer(uint64_t, bdp_cwnd_rtt, cc->bdp[BDP_ALG_CWND_RTT])
    ctf_integer(uint64_t, bdp_sendme_rate, cc->bdp[BDP_ALG_SENDME_RATE])
    ctf_integer(uint64_t, bdp_piecewise, cc->bdp[BDP_ALG_PIECEWISE])
    ctf_integer(uint64_t, sendme_rate_bdp, sendme_rate_bdp)
  )
)

#endif /* LTTNG_CC_INC || TRACEPOINT_HEADER_MULTI_READ */

/* Must be included after the probes declaration. */
#include <lttng/tracepoint-event.h>

#endif /* USE_TRACING_INSTRUMENTATION_LTTNG */
