/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file edge_connection_st.h
 * @brief Edge-connection structure.
 **/

#ifndef EDGE_CONNECTION_ST_H
#define EDGE_CONNECTION_ST_H

#include "core/or/or.h"

#include "core/or/connection_st.h"

/** Subtype of connection_t for an "edge connection" -- that is, an entry (ap)
 * connection, or an exit. */
struct edge_connection_t {
  connection_t base_;

  struct edge_connection_t *next_stream; /**< Points to the next stream at this
                                          * edge, if any */
  int package_window; /**< How many more relay cells can I send into the
                       * circuit? */
  int deliver_window; /**< How many more relay cells can end at me? */

  struct circuit_t *on_circuit; /**< The circuit (if any) that this edge
                                 * connection is using. */

  /** A pointer to which node in the circ this conn exits at.  Set for AP
   * connections and for hidden service exit connections. */
  struct crypt_path_t *cpath_layer;
  /** What rendezvous service are we querying for (if an AP) or providing (if
   * an exit)? */
  rend_data_t *rend_data;

  /* Hidden service connection identifier for edge connections. Used by the HS
   * client-side code to identify client SOCKS connections and by the
   * service-side code to match HS circuits with their streams. */
  struct hs_ident_edge_conn_t *hs_ident;

  uint32_t address_ttl; /**< TTL for address-to-addr mapping on exit
                         * connection.  Exit connections only. */
  uint32_t begincell_flags; /** Flags sent or received in the BEGIN cell
                             * for this connection */

  streamid_t stream_id; /**< The stream ID used for this edge connection on its
                         * circuit */

  /** The reason why this connection is closing; passed to the controller. */
  uint16_t end_reason;

  /** Bytes read since last call to control_event_stream_bandwidth_used() */
  uint32_t n_read;

  /** Bytes written since last call to control_event_stream_bandwidth_used() */
  uint32_t n_written;

  /** True iff this connection is for a DNS request only. */
  unsigned int is_dns_request:1;
  /** True iff this connection is for a PTR DNS request. (exit only) */
  unsigned int is_reverse_dns_lookup:1;

  unsigned int edge_has_sent_end:1; /**< For debugging; only used on edge
                         * connections.  Set once we've set the stream end,
                         * and check in connection_about_to_close_connection().
                         */
  /** True iff we've blocked reading until the circuit has fewer queued
   * cells. */
  unsigned int edge_blocked_on_circ:1;

  /** Unique ID for directory requests; this used to be in connection_t, but
   * that's going away and being used on channels instead.  We still tag
   * edge connections with dirreq_id from circuits, so it's copied here. */
  uint64_t dirreq_id;
};

#endif /* !defined(EDGE_CONNECTION_ST_H) */
