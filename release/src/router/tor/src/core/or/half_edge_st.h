/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file half_edge_st.h
 * @brief Half-open connection structure.
 **/

#ifndef HALF_EDGE_ST_H
#define HALF_EDGE_ST_H

#include "core/or/or.h"

/**
 * Struct to track a connection that we closed that the other end
 * still thinks is open. Exists in origin_circuit_t.half_streams until
 * we get an end cell or a resolved cell for this stream id.
 */
typedef struct half_edge_t {
  /** stream_id for the half-closed connection */
  streamid_t stream_id;

  /** How many sendme's can the other end still send, based on how
   * much data we had sent at the time of close */
  int sendmes_pending;

  /** How much more data can the other end still send, based on
   * our deliver window */
  int data_pending;

  /** Is there a connected cell pending? */
  int connected_pending : 1;
} half_edge_t;

#endif /* !defined(HALF_EDGE_ST_H) */
