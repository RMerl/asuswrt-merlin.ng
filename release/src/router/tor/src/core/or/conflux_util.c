/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file conflux_util.c
 * \brief Conflux utility functions for stream blocking and management.
 */

#define TOR_CONFLUX_PRIVATE

#include "core/or/or.h"

#include "core/or/circuit_st.h"
#include "core/or/sendme.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_st.h"
#include "core/or/circuitlist.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/conflux.h"
#include "core/or/conflux_params.h"
#include "core/or/conflux_util.h"
#include "core/or/conflux_pool.h"
#include "core/or/conflux_st.h"
#include "lib/time/compat_time.h"
#include "app/config/config.h"

/**
 * This is a utility function that returns the package window circuit,
 * regardless of if it has a conflux pair or not.
 */
int
circuit_get_package_window(circuit_t *circ,
                           const crypt_path_t *cpath)
{
  /* We believe it is possible to get a closed circuit related to the
   * on_circuit pointer of a connection not being nullified before ending up
   * here. Else, this can lead to loud bug like experienced in #40908. */
  if (circ->marked_for_close) {
    return 0;
  }

  if (circ->conflux) {
    if (CIRCUIT_IS_ORIGIN(circ)) {
      tor_assert_nonfatal(circ->purpose ==
                          CIRCUIT_PURPOSE_CONFLUX_LINKED);
    }
    circuit_t *orig_circ = circ;

    /* If conflux is in the process of tearing down the set,
     * the package window is 0 -- there is no room. */
    if (circ->conflux->in_full_teardown)
      return 0;

    circ = conflux_decide_next_circ(circ->conflux);

    /* If conflux has no circuit to send on, the package window is 0. */
    if (!circ) {
      /* Bug #40842: Additional diagnostics for other potential cases */
      if (!orig_circ->conflux->curr_leg) {
        if (orig_circ->marked_for_close) {
          log_warn(LD_BUG, "Conflux has no circuit to send on. "
                           "Circuit %p idx %d marked at line %s:%d",
                           orig_circ, orig_circ->global_circuitlist_idx,
                           orig_circ->marked_for_close_file,
                           orig_circ->marked_for_close);
        } else {
          log_warn(LD_BUG, "Conflux has no circuit to send on. "
                           "Circuit %p idx %d not marked for close.",
                           orig_circ, orig_circ->global_circuitlist_idx);
        }
      }
      return 0;
    }

    /* If we are the origin, we need to get the last hop's cpath for
     * congestion control information. */
    if (CIRCUIT_IS_ORIGIN(circ)) {
      cpath = CONST_TO_ORIGIN_CIRCUIT(circ)->cpath->prev;
    } else {
      if (BUG(cpath != NULL)) {
        log_warn(LD_BUG, "cpath is not NULL for non-origin circuit");
      }
    }
  }

  return congestion_control_get_package_window(circ, cpath);
}

/**
 * Returns true if conflux can send a data cell.
 *
 * Used to decide if we should block streams or not, for
 * proccess_sendme_cell(), circuit_resume_edge_reading(),
 * circuit_consider_stop_edge_reading(), circuit_resume_edge_reading_helper(),
 * channel_flush_from_first_active_circuit()
*/
bool
conflux_can_send(conflux_t *cfx)
{
  const circuit_t *send_circ = conflux_decide_next_circ(cfx);

  /* If we have a circuit, we can send */
  if (send_circ) {
    return true;
  } else {
    if (BUG(!cfx->in_full_teardown && !cfx->curr_leg)) {
      log_fn(LOG_WARN,
             LD_BUG, "Conflux has no current circuit to send on. ");
    }
    return false;
  }
}

/**
 * For a given conflux circuit, return the cpath of the destination.
 *
 * The cpath destination is the last hop of the circuit, or NULL if
 * the circuit is a non-origin circuit.
 */
crypt_path_t *
conflux_get_destination_hop(circuit_t *circ)
{
  if (BUG(!circ)) {
    log_warn(LD_BUG, "No circuit to send on for conflux");
    return NULL;
  } else {
    /* Conflux circuits always send multiplexed relay commands to
     * to the last hop. (Non-multiplexed commands go on their
     * original circuit and hop). */
    if (CIRCUIT_IS_ORIGIN(circ)) {
      return TO_ORIGIN_CIRCUIT(circ)->cpath->prev;
    } else {
      return NULL;
    }
  }
}

/**
 * Validates that the source of a cell is from the last hop of the circuit
 * for origin circuits, and that there are no further hops for non-origin
 * circuits.
 */
bool
conflux_validate_source_hop(circuit_t *in_circ,
                            crypt_path_t *layer_hint)
{
  crypt_path_t *dest = conflux_get_destination_hop(in_circ);

  if (dest != layer_hint) {
    log_warn(LD_CIRC, "Got conflux command from incorrect hop");
    return false;
  }

  if (layer_hint == NULL) {
    /* We should not have further hops attached to this circuit */
    if (in_circ->n_chan) {
      log_warn(LD_BUG, "Got conflux command on circuit with further hops");
      return false;
    }
  }
  return true;
}

/**
 * Returns true if the edge connection uses the given cpath.
 *
 * If there is a conflux object, we inspect all the last hops of the conflux
 * circuits.
 */
bool
edge_uses_cpath(const edge_connection_t *conn,
                const crypt_path_t *cpath)
{
  if (!conn->on_circuit)
    return false;

  if (CIRCUIT_IS_ORIGIN(conn->on_circuit)) {
    if (conn->on_circuit->conflux) {
     tor_assert_nonfatal(conn->on_circuit->purpose ==
                         CIRCUIT_PURPOSE_CONFLUX_LINKED);

     /* If the circuit is an origin circuit with a conflux object, the cpath
      * is valid if it came from any of the conflux circuit's last hops. */
      CONFLUX_FOR_EACH_LEG_BEGIN(conn->on_circuit->conflux, leg) {
        const origin_circuit_t *ocirc = CONST_TO_ORIGIN_CIRCUIT(leg->circ);
        if (ocirc->cpath->prev == cpath) {
          return true;
        }
      } CONFLUX_FOR_EACH_LEG_END(leg);
    } else {
      return cpath == conn->cpath_layer;
    }
  } else {
    /* For non-origin circuits, cpath should be null */
    return cpath == NULL;
  }

  return false;
}

/**
 * Returns the max RTT for the circuit that carries this stream,
 * as observed by congestion control. For conflux circuits,
 * we return the max RTT across all circuits.
 */
uint64_t
edge_get_max_rtt(const edge_connection_t *stream)
{
  if (!stream->on_circuit)
    return 0;

  if (stream->on_circuit->conflux) {
    tor_assert_nonfatal(stream->on_circuit->purpose ==
                        CIRCUIT_PURPOSE_CONFLUX_LINKED);

    /* Find the max rtt from the ccontrol object of each circuit. */
    uint64_t max_rtt = 0;
    CONFLUX_FOR_EACH_LEG_BEGIN(stream->on_circuit->conflux, leg) {
      const congestion_control_t *cc = circuit_ccontrol(leg->circ);
      if (cc->max_rtt_usec > max_rtt) {
        max_rtt = cc->max_rtt_usec;
      }
    } CONFLUX_FOR_EACH_LEG_END(leg);

    return max_rtt;
  } else {
    if (stream->on_circuit && stream->on_circuit->ccontrol)
      return stream->on_circuit->ccontrol->max_rtt_usec;
    else if (stream->cpath_layer && stream->cpath_layer->ccontrol)
      return stream->cpath_layer->ccontrol->max_rtt_usec;
  }

  return 0;
}

/**
 * Return true iff our decryption layer_hint is from the last hop
 * in a circuit.
 */
bool
relay_crypt_from_last_hop(const origin_circuit_t *circ,
                          const crypt_path_t *layer_hint)
{
  tor_assert(circ);
  tor_assert(layer_hint);
  tor_assert(circ->cpath);

  if (TO_CIRCUIT(circ)->conflux) {
    tor_assert_nonfatal(TO_CIRCUIT(circ)->purpose ==
                        CIRCUIT_PURPOSE_CONFLUX_LINKED);

    /* If we are a conflux circuit, we need to check if the layer_hint
     * is from the last hop of any of the conflux circuits. */
    CONFLUX_FOR_EACH_LEG_BEGIN(TO_CIRCUIT(circ)->conflux, leg) {
      const origin_circuit_t *ocirc = CONST_TO_ORIGIN_CIRCUIT(leg->circ);
      if (layer_hint == ocirc->cpath->prev) {
        return true;
      }
    } CONFLUX_FOR_EACH_LEG_END(leg);

    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Got unexpected relay data from intermediate hop");
    return false;
  } else {
    if (layer_hint != circ->cpath->prev) {
      log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
             "Got unexpected relay data from intermediate hop");
      return false;
    }
    return true;
  }
}

/**
 * Update the head of the n_streams list on all circuits in the conflux
 * set.
 */
void
conflux_update_p_streams(origin_circuit_t *circ, edge_connection_t *stream)
{
  tor_assert(circ);

  if (TO_CIRCUIT(circ)->conflux) {
    tor_assert_nonfatal(TO_CIRCUIT(circ)->purpose ==
                        CIRCUIT_PURPOSE_CONFLUX_LINKED);
    CONFLUX_FOR_EACH_LEG_BEGIN(TO_CIRCUIT(circ)->conflux, leg) {
      TO_ORIGIN_CIRCUIT(leg->circ)->p_streams = stream;
    } CONFLUX_FOR_EACH_LEG_END(leg);
  }
}

/**
 * Sync the next_stream_id, timestamp_dirty, and circuit_idle_timeout
 * fields of a conflux set to the values in a particular circuit.
 *
 * This is called upon link, and whenever one of these fields
 * changes on ref_circ. The ref_circ values are copied to all
 * other circuits in the conflux set.
*/
void
conflux_sync_circ_fields(conflux_t *cfx, origin_circuit_t *ref_circ)
{
  tor_assert(cfx);
  tor_assert(ref_circ);

  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
    if (leg->circ == TO_CIRCUIT(ref_circ)) {
      continue;
    }
    origin_circuit_t *ocirc = TO_ORIGIN_CIRCUIT(leg->circ);
    ocirc->next_stream_id = ref_circ->next_stream_id;
    leg->circ->timestamp_dirty = TO_CIRCUIT(ref_circ)->timestamp_dirty;
    ocirc->circuit_idle_timeout = ref_circ->circuit_idle_timeout;
    ocirc->unusable_for_new_conns = ref_circ->unusable_for_new_conns;
  } CONFLUX_FOR_EACH_LEG_END(leg);
}

/**
 * Update the head of the n_streams list on all circuits in the conflux
 * set.
 */
void
conflux_update_n_streams(or_circuit_t *circ, edge_connection_t *stream)
{
  tor_assert(circ);

  if (TO_CIRCUIT(circ)->conflux) {
    CONFLUX_FOR_EACH_LEG_BEGIN(TO_CIRCUIT(circ)->conflux, leg) {
      TO_OR_CIRCUIT(leg->circ)->n_streams = stream;
    } CONFLUX_FOR_EACH_LEG_END(leg);
  }
}

/**
 * Update the head of the resolving_streams list on all circuits in the conflux
 * set.
 */
void
conflux_update_resolving_streams(or_circuit_t *circ, edge_connection_t *stream)
{
  tor_assert(circ);

  if (TO_CIRCUIT(circ)->conflux) {
    CONFLUX_FOR_EACH_LEG_BEGIN(TO_CIRCUIT(circ)->conflux, leg) {
      TO_OR_CIRCUIT(leg->circ)->resolving_streams = stream;
    } CONFLUX_FOR_EACH_LEG_END(leg);
  }
}

/**
 * Update the half_streams list on all circuits in the conflux
 */
void
conflux_update_half_streams(origin_circuit_t *circ, smartlist_t *half_streams)
{
  tor_assert(circ);

  if (TO_CIRCUIT(circ)->conflux) {
    tor_assert_nonfatal(TO_CIRCUIT(circ)->purpose ==
                        CIRCUIT_PURPOSE_CONFLUX_LINKED);
    CONFLUX_FOR_EACH_LEG_BEGIN(TO_CIRCUIT(circ)->conflux, leg) {
      TO_ORIGIN_CIRCUIT(leg->circ)->half_streams = half_streams;
    } CONFLUX_FOR_EACH_LEG_END(leg);
  }
}

/**
 * Helper function that emits non-fatal asserts if the stream lists
 * or next_stream_id is out of sync between any of the conflux legs.
*/
void
conflux_validate_stream_lists(const conflux_t *cfx)
{
  const conflux_leg_t *first_leg = smartlist_get(cfx->legs, 0);
  tor_assert(first_leg);

  /* Compare the stream lists of the first leg to all other legs. */
  if (CIRCUIT_IS_ORIGIN(first_leg->circ)) {
    const origin_circuit_t *f_circ =
        CONST_TO_ORIGIN_CIRCUIT(first_leg->circ);

    CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
      const origin_circuit_t *l_circ = CONST_TO_ORIGIN_CIRCUIT(leg->circ);
      tor_assert_nonfatal(l_circ->p_streams == f_circ->p_streams);
      tor_assert_nonfatal(l_circ->half_streams == f_circ->half_streams);
      tor_assert_nonfatal(l_circ->next_stream_id == f_circ->next_stream_id);
    } CONFLUX_FOR_EACH_LEG_END(leg);
  } else {
    const or_circuit_t *f_circ = CONST_TO_OR_CIRCUIT(first_leg->circ);
    CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
      const or_circuit_t *l_circ = CONST_TO_OR_CIRCUIT(leg->circ);
      tor_assert_nonfatal(l_circ->n_streams == f_circ->n_streams);
      tor_assert_nonfatal(l_circ->resolving_streams ==
                          f_circ->resolving_streams);
    } CONFLUX_FOR_EACH_LEG_END(leg);
  }
}

/**
 * Validate the conflux set has two legs, and both circuits have
 * no nonce, and for origin circuits, the purpose is CONFLUX_PURPOSE_LINKED.
 */
void
conflux_validate_legs(const conflux_t *cfx)
{
  tor_assert(cfx);
  bool is_client = false;
  int num_legs = 0;
  CONFLUX_FOR_EACH_LEG_BEGIN(cfx, leg) {
    if (CIRCUIT_IS_ORIGIN(leg->circ)) {
      tor_assert_nonfatal(leg->circ->purpose ==
                          CIRCUIT_PURPOSE_CONFLUX_LINKED);
      is_client = true;
    }

    /* Ensure we have no pending nonce on the circ */
    if (BUG(leg->circ->conflux_pending_nonce != NULL)) {
      conflux_log_set(LOG_WARN, cfx, is_client);
      continue;
    }

    /* Ensure we have a conflux object */
    if (BUG(leg->circ->conflux == NULL)) {
      conflux_log_set(LOG_WARN, cfx, is_client);
      continue;
    }

    /* Only count legs that have a valid RTT */
    if (leg->circ_rtts_usec > 0) {
      num_legs++;
    }
  } CONFLUX_FOR_EACH_LEG_END(leg);

  // TODO-329-UDP: Eventually we want to allow three legs for the
  // exit case, to allow reconnection of legs to hit an RTT target.
  // For now, this validation helps find bugs.
  if (num_legs > conflux_params_get_num_legs_set()) {
    log_fn(LOG_PROTOCOL_WARN,
           LD_BUG, "Number of legs is above maximum of %d allowed: %d\n",
             conflux_params_get_num_legs_set(), smartlist_len(cfx->legs));
    conflux_log_set(LOG_PROTOCOL_WARN, cfx, is_client);
  }
}

/** Return the nonce for a circuit, for use on the control port */
const uint8_t *
conflux_get_nonce(const circuit_t *circ)
{
  if (circ->conflux_pending_nonce) {
    return circ->conflux_pending_nonce;
  } else if (circ->conflux) {
    return circ->conflux->nonce;
  } else {
    return NULL;
  }
}

/** Return the conflux RTT for a circuit, for use on the control port */
uint64_t
conflux_get_circ_rtt(const circuit_t *circ)
{
  if (circ->conflux) {
    conflux_leg_t *leg = conflux_get_leg(circ->conflux, circ);
    if (BUG(!leg)) {
      return 0;
    } else {
      return leg->circ_rtts_usec;
    }
  } else {
    return 0;
  }
}

