/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef ORIGIN_CIRCUIT_ST_H
#define ORIGIN_CIRCUIT_ST_H

#include "core/or/or.h"

#include "core/or/circuit_st.h"

struct onion_queue_t;

/**
 * Describes the circuit building process in simplified terms based
 * on the path bias accounting state for a circuit.
 *
 * NOTE: These state values are enumerated in the order for which we
 * expect circuits to transition through them. If you add states,
 * you need to preserve this overall ordering. The various pathbias
 * state transition and accounting functions (pathbias_mark_* and
 * pathbias_count_*) contain ordinal comparisons to enforce proper
 * state transitions for corrections.
 *
 * This state machine and the associated logic was created to prevent
 * miscounting due to unknown cases of circuit reuse. See also tickets
 * #6475 and #7802.
 */
enum path_state_t {
    /** This circuit is "new". It has not yet completed a first hop
     * or been counted by the path bias code. */
    PATH_STATE_NEW_CIRC = 0,
    /** This circuit has completed one/two hops, and has been counted by
     * the path bias logic. */
    PATH_STATE_BUILD_ATTEMPTED = 1,
    /** This circuit has been completely built */
    PATH_STATE_BUILD_SUCCEEDED = 2,
    /** Did we try to attach any SOCKS streams or hidserv introductions to
      * this circuit?
      *
      * Note: If we ever implement end-to-end stream timing through test
      * stream probes (#5707), we must *not* set this for those probes
      * (or any other automatic streams) because the adversary could
      * just tag at a later point.
      */
    PATH_STATE_USE_ATTEMPTED = 3,
    /** Did any SOCKS streams or hidserv introductions actually succeed on
      * this circuit?
      *
      * If any streams detatch/fail from this circuit, the code transitions
      * the circuit back to PATH_STATE_USE_ATTEMPTED to ensure we probe. See
      * pathbias_mark_use_rollback() for that.
      */
    PATH_STATE_USE_SUCCEEDED = 4,

    /**
     * This is a special state to indicate that we got a corrupted
     * relay cell on a circuit and we don't intend to probe it.
     */
    PATH_STATE_USE_FAILED = 5,

    /**
     * This is a special state to indicate that we already counted
     * the circuit. Used to guard against potential state machine
     * violations.
     */
    PATH_STATE_ALREADY_COUNTED = 6,
};

/** An origin_circuit_t holds data necessary to build and use a circuit.
 */
struct origin_circuit_t {
  circuit_t base_;

  /** Linked list of AP streams (or EXIT streams if hidden service)
   * associated with this circuit. */
  edge_connection_t *p_streams;

  /** Smartlist of half-closed streams (half_edge_t*) that still
   * have pending activity */
  smartlist_t *half_streams;

  /** Bytes read on this circuit since last call to
   * control_event_circ_bandwidth_used().  Only used if we're configured
   * to emit CIRC_BW events. */
  uint32_t n_read_circ_bw;

  /** Bytes written to on this circuit since last call to
   * control_event_circ_bandwidth_used().  Only used if we're configured
   * to emit CIRC_BW events. */
  uint32_t n_written_circ_bw;

  /** Total known-valid relay cell bytes since last call to
   * control_event_circ_bandwidth_used().  Only used if we're configured
   * to emit CIRC_BW events. */
  uint32_t n_delivered_read_circ_bw;

  /** Total written relay cell bytes since last call to
   * control_event_circ_bandwidth_used().  Only used if we're configured
   * to emit CIRC_BW events. */
  uint32_t n_delivered_written_circ_bw;

  /** Total overhead data in all known-valid relay data cells since last
   * call to control_event_circ_bandwidth_used().  Only used if we're
   * configured to emit CIRC_BW events. */
  uint32_t n_overhead_read_circ_bw;

  /** Total written overhead data in all relay data cells since last call to
   * control_event_circ_bandwidth_used().  Only used if we're configured
   * to emit CIRC_BW events. */
  uint32_t n_overhead_written_circ_bw;

  /** Build state for this circuit. It includes the intended path
   * length, the chosen exit router, rendezvous information, etc.
   */
  cpath_build_state_t *build_state;
  /** The doubly-linked list of crypt_path_t entries, one per hop,
   * for this circuit. This includes ciphers for each hop,
   * integrity-checking digests for each hop, and package/delivery
   * windows for each hop.
   */
  crypt_path_t *cpath;

  /** Holds all rendezvous data on either client or service side. */
  rend_data_t *rend_data;

  /** Holds hidden service identifier on either client or service side. This
   * is for both introduction and rendezvous circuit. */
  struct hs_ident_circuit_t *hs_ident;

  /** Holds the data that the entry guard system uses to track the
   * status of the guard this circuit is using, and thereby to determine
   * whether this circuit can be used. */
  struct circuit_guard_state_t *guard_state;

  /** Index into global_origin_circuit_list for this circuit. -1 if not
   * present. */
  int global_origin_circuit_list_idx;

  /** How many more relay_early cells can we send on this circuit, according
   * to the specification? */
  unsigned int remaining_relay_early_cells : 4;

  /** Set if this circuit is insanely old and we already informed the user */
  unsigned int is_ancient : 1;

  /** Set if this circuit has already been opened. Used to detect
   * cannibalized circuits. */
  unsigned int has_opened : 1;

  /**
   * Path bias state machine. Used to ensure integrity of our
   * circuit building and usage accounting. See path_state_t
   * for more details.
   */
  path_state_bitfield_t path_state : 3;

  /* If this flag is set, we should not consider attaching any more
   * connections to this circuit. */
  unsigned int unusable_for_new_conns : 1;

  /* If this flag is set (due to padding negotiation failure), we should
   * not try to negotiate further circuit padding. */
  unsigned padding_negotiation_failed : 1;

  /**
   * Tristate variable to guard against pathbias miscounting
   * due to circuit purpose transitions changing the decision
   * of pathbias_should_count(). This variable is informational
   * only. The current results of pathbias_should_count() are
   * the official decision for pathbias accounting.
   */
  uint8_t pathbias_shouldcount;
#define PATHBIAS_SHOULDCOUNT_UNDECIDED 0
#define PATHBIAS_SHOULDCOUNT_IGNORED   1
#define PATHBIAS_SHOULDCOUNT_COUNTED   2

  /** For path probing. Store the temporary probe stream ID
   * for response comparison */
  streamid_t pathbias_probe_id;

  /** For path probing. Store the temporary probe address nonce
   * (in host byte order) for response comparison. */
  uint32_t pathbias_probe_nonce;

  /** Set iff this is a hidden-service circuit which has timed out
   * according to our current circuit-build timeout, but which has
   * been kept around because it might still succeed in connecting to
   * its destination, and which is not a fully-connected rendezvous
   * circuit.
   *
   * (We clear this flag for client-side rendezvous circuits when they
   * are 'joined' to the other side's rendezvous circuit, so that
   * connection_ap_handshake_attach_circuit can put client streams on
   * the circuit.  We also clear this flag for service-side rendezvous
   * circuits when they are 'joined' to a client's rend circ, but only
   * for symmetry with the client case.  Client-side introduction
   * circuits are closed when we get a joined rend circ, and
   * service-side introduction circuits never have this flag set.) */
  unsigned int hs_circ_has_timed_out : 1;

  /** Set iff this circuit has been given a relaxed timeout because
   * no circuits have opened. Used to prevent spamming logs. */
  unsigned int relaxed_timeout : 1;

  /** Set iff this is a service-side rendezvous circuit for which a
   * new connection attempt has been launched.  We consider launching
   * a new service-side rend circ to a client when the previous one
   * fails; now that we don't necessarily close a service-side rend
   * circ when we launch a new one to the same client, this flag keeps
   * us from launching two retries for the same failed rend circ. */
  unsigned int hs_service_side_rend_circ_has_been_relaunched : 1;

  /** What commands were sent over this circuit that decremented the
   * RELAY_EARLY counter? This is for debugging task 878. */
  uint8_t relay_early_commands[MAX_RELAY_EARLY_CELLS_PER_CIRCUIT];

  /** How many RELAY_EARLY cells have been sent over this circuit? This is
   * for debugging task 878, too. */
  int relay_early_cells_sent;

  /** The next stream_id that will be tried when we're attempting to
   * construct a new AP stream originating at this circuit. */
  streamid_t next_stream_id;

  /* The intro key replaces the hidden service's public key if purpose is
   * S_ESTABLISH_INTRO or S_INTRO, provided that no unversioned rendezvous
   * descriptor is used. */
  crypto_pk_t *intro_key;

  /** Quasi-global identifier for this circuit; used for control.c */
  /* XXXX NM This can get re-used after 2**32 circuits. */
  uint32_t global_identifier;

  /** True if we have associated one stream to this circuit, thereby setting
   * the isolation parameters for this circuit.  Note that this doesn't
   * necessarily mean that we've <em>attached</em> any streams to the circuit:
   * we may only have marked up this circuit during the launch process.
   */
  unsigned int isolation_values_set : 1;
  /** True iff any stream has <em>ever</em> been attached to this circuit.
   *
   * In a better world we could use timestamp_dirty for this, but
   * timestamp_dirty is far too overloaded at the moment.
   */
  unsigned int isolation_any_streams_attached : 1;

  /** A bitfield of ISO_* flags for every isolation field such that this
   * circuit has had streams with more than one value for that field
   * attached to it. */
  uint8_t isolation_flags_mixed;

  /** @name Isolation parameters
   *
   * If any streams have been associated with this circ (isolation_values_set
   * == 1), and all streams associated with the circuit have had the same
   * value for some field ((isolation_flags_mixed & ISO_FOO) == 0), then these
   * elements hold the value for that field.
   *
   * Note again that "associated" is not the same as "attached": we
   * preliminarily associate streams with a circuit while the circuit is being
   * launched, so that we can tell whether we need to launch more circuits.
   *
   * @{
   */
  uint8_t client_proto_type;
  uint8_t client_proto_socksver;
  uint16_t dest_port;
  tor_addr_t client_addr;
  char *dest_address;
  int session_group;
  unsigned nym_epoch;
  size_t socks_username_len;
  uint8_t socks_password_len;
  /* Note that the next two values are NOT NUL-terminated; see
     socks_username_len and socks_password_len for their lengths. */
  char *socks_username;
  char *socks_password;
  /** Global identifier for the first stream attached here; used by
   * ISO_STREAM. */
  uint64_t associated_isolated_stream_global_id;
  /**@}*/
  /** A list of addr_policy_t for this circuit in particular. Used by
   * adjust_exit_policy_from_exitpolicy_failure.
   */
  smartlist_t *prepend_policy;

  /** How long do we wait before closing this circuit if it remains
   * completely idle after it was built, in seconds? This value
   * is randomized on a per-circuit basis from CircuitsAvailableTimoeut
   * to 2*CircuitsAvailableTimoeut. */
  int circuit_idle_timeout;

};

#endif /* !defined(ORIGIN_CIRCUIT_ST_H) */
