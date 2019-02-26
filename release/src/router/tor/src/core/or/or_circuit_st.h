/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef OR_CIRCUIT_ST_H
#define OR_CIRCUIT_ST_H

#include "core/or/or.h"

#include "core/or/circuit_st.h"
#include "core/or/crypt_path_st.h"

struct onion_queue_t;

/** An or_circuit_t holds information needed to implement a circuit at an
 * OR. */
struct or_circuit_t {
  circuit_t base_;

  /** Pointer to an entry on the onion queue, if this circuit is waiting for a
   * chance to give an onionskin to a cpuworker. Used only in onion.c */
  struct onion_queue_t *onionqueue_entry;
  /** Pointer to a workqueue entry, if this circuit has given an onionskin to
   * a cpuworker and is waiting for a response. Used to decide whether it is
   * safe to free a circuit or if it is still in use by a cpuworker. */
  struct workqueue_entry_s *workqueue_entry;

  /** The circuit_id used in the previous (backward) hop of this circuit. */
  circid_t p_circ_id;
  /** Queue of cells waiting to be transmitted on p_conn. */
  cell_queue_t p_chan_cells;
  /** The channel that is previous in this circuit. */
  channel_t *p_chan;
  /**
   * Circuit mux associated with p_chan to which this circuit is attached;
   * NULL if we have no p_chan.
   */
  circuitmux_t *p_mux;
  /** Linked list of Exit streams associated with this circuit. */
  edge_connection_t *n_streams;
  /** Linked list of Exit streams associated with this circuit that are
   * still being resolved. */
  edge_connection_t *resolving_streams;

  /** Cryptographic state used for encrypting and authenticating relay
   * cells to and from this hop. */
  relay_crypto_t crypto;

  /** Points to spliced circuit if purpose is REND_ESTABLISHED, and circuit
   * is not marked for close. */
  struct or_circuit_t *rend_splice;

  /** Stores KH for the handshake. */
  char rend_circ_nonce[DIGEST_LEN];/* KH in tor-spec.txt */

  /** How many more relay_early cells can we send on this circuit, according
   * to the specification? */
  unsigned int remaining_relay_early_cells : 4;

  /* We have already received an INTRODUCE1 cell on this circuit. */
  unsigned int already_received_introduce1 : 1;

  /** If set, this circuit carries HS traffic. Consider it in any HS
   *  statistics. */
  unsigned int circuit_carries_hs_traffic_stats : 1;

  /** Number of cells that were removed from circuit queue; reset every
   * time when writing buffer stats to disk. */
  uint32_t processed_cells;

  /** Total time in milliseconds that cells spent in both app-ward and
   * exit-ward queues of this circuit; reset every time when writing
   * buffer stats to disk. */
  uint64_t total_cell_waiting_time;
};

#endif

