/* Copyright 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file circuitlist.c
 *
 * \brief Manage global structures that list and index circuits, and
 *   look up circuits within them.
 *
 * One of the most frequent operations in Tor occurs every time that
 * a relay cell arrives on a channel.  When that happens, we need to
 * find which circuit it is associated with, based on the channel and the
 * circuit ID in the relay cell.
 *
 * To handle that, we maintain a global list of circuits, and a hashtable
 * mapping [channel,circID] pairs to circuits.  Circuits are added to and
 * removed from this mapping using circuit_set_p_circid_chan() and
 * circuit_set_n_circid_chan().  To look up a circuit from this map, most
 * callers should use circuit_get_by_circid_channel(), though
 * circuit_get_by_circid_channel_even_if_marked() is appropriate under some
 * circumstances.
 *
 * We also need to allow for the possibility that we have blocked use of a
 * circuit ID (because we are waiting to send a DESTROY cell), but the
 * circuit is not there any more.  For that case, we allow placeholder
 * entries in the table, using channel_mark_circid_unusable().
 *
 * To efficiently handle a channel that has just opened, we also maintain a
 * list of the circuits waiting for channels, so we can attach them as
 * needed without iterating through the whole list of circuits, using
 * circuit_get_all_pending_on_channel().
 *
 * In this module, we also handle the list of circuits that have been
 * marked for close elsewhere, and close them as needed.  (We use this
 * "mark now, close later" pattern here and elsewhere to avoid
 * unpredictable recursion if we closed every circuit immediately upon
 * realizing it needed to close.)  See circuit_mark_for_close() for the
 * mark function, and circuit_close_all_marked() for the close function.
 *
 * For hidden services, we need to be able to look up introduction point
 * circuits and rendezvous circuits by cookie, key, etc.  These are
 * currently handled with linear searches in
 * circuit_get_next_by_pk_and_purpose(), and with hash lookups in
 * circuit_get_rendezvous() and circuit_get_intro_point().
 *
 * This module is also the entry point for our out-of-memory handler
 * logic, which was originally circuit-focused.
 **/
#define CIRCUITLIST_PRIVATE
#define OCIRC_EVENT_PRIVATE
#include "lib/cc/torint.h"  /* TOR_PRIuSZ */

#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#include "feature/client/circpathbias.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/circuitstats.h"
#include "core/or/circuitpadding.h"
#include "core/or/crypt_path.h"
#include "core/or/extendinfo.h"
#include "core/or/status.h"
#include "core/or/trace_probes_circuit.h"
#include "core/mainloop/connection.h"
#include "app/config/config.h"
#include "core/or/connection_edge.h"
#include "core/or/connection_or.h"
#include "feature/control/control_events.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "feature/dircommon/directory.h"
#include "feature/client/entrynodes.h"
#include "core/mainloop/mainloop.h"
#include "feature/hs/hs_cache.h"
#include "feature/hs/hs_circuit.h"
#include "feature/hs/hs_circuitmap.h"
#include "feature/hs/hs_ident.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/relay/onion_queue.h"
#include "core/crypto/onion_crypto.h"
#include "core/crypto/onion_fast.h"
#include "core/or/policies.h"
#include "core/or/relay.h"
#include "core/crypto/relay_crypto.h"
#include "feature/rend/rendcommon.h"
#include "feature/stats/predict_ports.h"
#include "feature/stats/bwhist.h"
#include "feature/stats/rephist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "core/or/channelpadding.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_lzma.h"
#include "lib/compress/compress_zlib.h"
#include "lib/compress/compress_zstd.h"
#include "lib/buf/buffers.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_st.h"
#include "lib/math/stats.h"

#include "core/or/ocirc_event.h"

#include "ht.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/crypt_path_reference_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "core/or/edge_connection_st.h"
#include "core/or/half_edge_st.h"
#include "core/or/extend_info_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

/********* START VARIABLES **********/

/** A global list of all circuits at this hop. */
static smartlist_t *global_circuitlist = NULL;

/** A global list of all origin circuits. Every element of this is also
 * an element of global_circuitlist. */
static smartlist_t *global_origin_circuit_list = NULL;

/** A list of all the circuits in CIRCUIT_STATE_CHAN_WAIT. */
static smartlist_t *circuits_pending_chans = NULL;

/** List of all the (origin) circuits whose state is
 * CIRCUIT_STATE_GUARD_WAIT. */
static smartlist_t *circuits_pending_other_guards = NULL;

/** A list of all the circuits that have been marked with
 * circuit_mark_for_close and which are waiting for circuit_about_to_free. */
static smartlist_t *circuits_pending_close = NULL;

static void circuit_about_to_free_atexit(circuit_t *circ);
static void circuit_about_to_free(circuit_t *circ);

/**
 * A cached value of the current state of the origin circuit list.  Has the
 * value 1 if we saw any opened circuits recently (since the last call to
 * circuit_any_opened_circuits(), which gets called around once a second by
 * circuit_expire_building). 0 otherwise.
 */
static int any_opened_circs_cached_val = 0;

/** Moving average of the cc->cwnd from each closed circuit. */
double cc_stats_circ_close_cwnd_ma = 0;
/** Moving average of the cc->cwnd from each closed slow-start circuit. */
double cc_stats_circ_close_ss_cwnd_ma = 0;

uint64_t cc_stats_circs_closed = 0;

/********* END VARIABLES ************/

/* Implement circuit handle helpers. */
HANDLE_IMPL(circuit, circuit_t,)

or_circuit_t *
TO_OR_CIRCUIT(circuit_t *x)
{
  tor_assert(x->magic == OR_CIRCUIT_MAGIC);
  return DOWNCAST(or_circuit_t, x);
}
const or_circuit_t *
CONST_TO_OR_CIRCUIT(const circuit_t *x)
{
  tor_assert(x->magic == OR_CIRCUIT_MAGIC);
  return DOWNCAST(or_circuit_t, x);
}
origin_circuit_t *
TO_ORIGIN_CIRCUIT(circuit_t *x)
{
  tor_assert(x->magic == ORIGIN_CIRCUIT_MAGIC);
  return DOWNCAST(origin_circuit_t, x);
}
const origin_circuit_t *
CONST_TO_ORIGIN_CIRCUIT(const circuit_t *x)
{
  tor_assert(x->magic == ORIGIN_CIRCUIT_MAGIC);
  return DOWNCAST(origin_circuit_t, x);
}

/** A map from channel and circuit ID to circuit.  (Lookup performance is
 * very important here, since we need to do it every time a cell arrives.) */
typedef struct chan_circid_circuit_map_t {
  HT_ENTRY(chan_circid_circuit_map_t) node;
  channel_t *chan;
  circid_t circ_id;
  circuit_t *circuit;
  /* For debugging 12184: when was this placeholder item added? */
  time_t made_placeholder_at;
} chan_circid_circuit_map_t;

/** Helper for hash tables: compare the channel and circuit ID for a and
 * b, and return less than, equal to, or greater than zero appropriately.
 */
static inline int
chan_circid_entries_eq_(chan_circid_circuit_map_t *a,
                        chan_circid_circuit_map_t *b)
{
  return a->chan == b->chan && a->circ_id == b->circ_id;
}

/** Helper: return a hash based on circuit ID and the pointer value of
 * chan in <b>a</b>. */
static inline unsigned int
chan_circid_entry_hash_(chan_circid_circuit_map_t *a)
{
  /* Try to squeze the siphash input into 8 bytes to save any extra siphash
   * rounds.  This hash function is in the critical path. */
  uintptr_t chan = (uintptr_t) (void*) a->chan;
  uint32_t array[2];
  array[0] = a->circ_id;
  /* The low bits of the channel pointer are uninteresting, since the channel
   * is a pretty big structure. */
  array[1] = (uint32_t) (chan >> 6);
  return (unsigned) siphash24g(array, sizeof(array));
}

/** Map from [chan,circid] to circuit. */
static HT_HEAD(chan_circid_map, chan_circid_circuit_map_t)
     chan_circid_map = HT_INITIALIZER();
HT_PROTOTYPE(chan_circid_map, chan_circid_circuit_map_t, node,
             chan_circid_entry_hash_, chan_circid_entries_eq_);
HT_GENERATE2(chan_circid_map, chan_circid_circuit_map_t, node,
             chan_circid_entry_hash_, chan_circid_entries_eq_, 0.6,
             tor_reallocarray_, tor_free_);

/** The most recently returned entry from circuit_get_by_circid_chan;
 * used to improve performance when many cells arrive in a row from the
 * same circuit.
 */
static chan_circid_circuit_map_t *_last_circid_chan_ent = NULL;

/** Implementation helper for circuit_set_{p,n}_circid_channel: A circuit ID
 * and/or channel for circ has just changed from <b>old_chan, old_id</b>
 * to <b>chan, id</b>.  Adjust the chan,circid map as appropriate, removing
 * the old entry (if any) and adding a new one. */
static void
circuit_set_circid_chan_helper(circuit_t *circ, int direction,
                               circid_t id,
                               channel_t *chan)
{
  chan_circid_circuit_map_t search;
  chan_circid_circuit_map_t *found;
  channel_t *old_chan, **chan_ptr;
  circid_t old_id, *circid_ptr;
  int make_active, attached = 0;

  if (direction == CELL_DIRECTION_OUT) {
    chan_ptr = &circ->n_chan;
    circid_ptr = &circ->n_circ_id;
    make_active = circ->n_chan_cells.n > 0;
  } else {
    or_circuit_t *c = TO_OR_CIRCUIT(circ);
    chan_ptr = &c->p_chan;
    circid_ptr = &c->p_circ_id;
    make_active = c->p_chan_cells.n > 0;
  }
  old_chan = *chan_ptr;
  old_id = *circid_ptr;

  if (id == old_id && chan == old_chan)
    return;

  if (_last_circid_chan_ent &&
      ((old_id == _last_circid_chan_ent->circ_id &&
        old_chan == _last_circid_chan_ent->chan) ||
       (id == _last_circid_chan_ent->circ_id &&
        chan == _last_circid_chan_ent->chan))) {
    _last_circid_chan_ent = NULL;
  }

  if (old_chan) {
    /*
     * If we're changing channels or ID and had an old channel and a non
     * zero old ID and weren't marked for close (i.e., we should have been
     * attached), detach the circuit. ID changes require this because
     * circuitmux hashes on (channel_id, circuit_id).
     */
    if (old_id != 0 && (old_chan != chan || old_id != id) &&
        !(circ->marked_for_close)) {
      tor_assert(old_chan->cmux);
      circuitmux_detach_circuit(old_chan->cmux, circ);
    }

    /* we may need to remove it from the conn-circid map */
    search.circ_id = old_id;
    search.chan = old_chan;
    found = HT_REMOVE(chan_circid_map, &chan_circid_map, &search);
    if (found) {
      tor_free(found);
      if (direction == CELL_DIRECTION_OUT) {
        /* One fewer circuits use old_chan as n_chan */
        --(old_chan->num_n_circuits);
      } else {
        /* One fewer circuits use old_chan as p_chan */
        --(old_chan->num_p_circuits);
      }
    }
  }

  /* Change the values only after we have possibly made the circuit inactive
   * on the previous chan. */
  *chan_ptr = chan;
  *circid_ptr = id;

  if (chan == NULL)
    return;

  /* now add the new one to the conn-circid map */
  search.circ_id = id;
  search.chan = chan;
  found = HT_FIND(chan_circid_map, &chan_circid_map, &search);
  if (found) {
    found->circuit = circ;
    found->made_placeholder_at = 0;
  } else {
    found = tor_malloc_zero(sizeof(chan_circid_circuit_map_t));
    found->circ_id = id;
    found->chan = chan;
    found->circuit = circ;
    HT_INSERT(chan_circid_map, &chan_circid_map, found);
  }

  /*
   * Attach to the circuitmux if we're changing channels or IDs and
   * have a new channel and ID to use and the circuit is not marked for
   * close.
   */
  if (chan && id != 0 && (old_chan != chan || old_id != id) &&
      !(circ->marked_for_close)) {
    tor_assert(chan->cmux);
    circuitmux_attach_circuit(chan->cmux, circ, direction);
    attached = 1;
  }

  /*
   * This is a no-op if we have no cells, but if we do it marks us active to
   * the circuitmux
   */
  if (make_active && attached)
    update_circuit_on_cmux(circ, direction);

  /* Adjust circuit counts on new channel */
  if (direction == CELL_DIRECTION_OUT) {
    ++chan->num_n_circuits;
  } else {
    ++chan->num_p_circuits;
  }
}

/** Mark that circuit id <b>id</b> shouldn't be used on channel <b>chan</b>,
 * even if there is no circuit on the channel. We use this to keep the
 * circuit id from getting re-used while we have queued but not yet sent
 * a destroy cell. */
void
channel_mark_circid_unusable(channel_t *chan, circid_t id)
{
  chan_circid_circuit_map_t search;
  chan_circid_circuit_map_t *ent;

  /* See if there's an entry there. That wouldn't be good. */
  memset(&search, 0, sizeof(search));
  search.chan = chan;
  search.circ_id = id;
  ent = HT_FIND(chan_circid_map, &chan_circid_map, &search);

  if (ent && ent->circuit) {
    /* we have a problem. */
    log_warn(LD_BUG, "Tried to mark %u unusable on %p, but there was already "
             "a circuit there.", (unsigned)id, chan);
  } else if (ent) {
    /* It's already marked. */
    if (!ent->made_placeholder_at)
      ent->made_placeholder_at = approx_time();
  } else {
    ent = tor_malloc_zero(sizeof(chan_circid_circuit_map_t));
    ent->chan = chan;
    ent->circ_id = id;
    /* leave circuit at NULL. */
    ent->made_placeholder_at = approx_time();
    HT_INSERT(chan_circid_map, &chan_circid_map, ent);
  }
}

/** Mark that a circuit id <b>id</b> can be used again on <b>chan</b>.
 * We use this to re-enable the circuit ID after we've sent a destroy cell.
 */
void
channel_mark_circid_usable(channel_t *chan, circid_t id)
{
  chan_circid_circuit_map_t search;
  chan_circid_circuit_map_t *ent;

  /* See if there's an entry there. That wouldn't be good. */
  memset(&search, 0, sizeof(search));
  search.chan = chan;
  search.circ_id = id;
  ent = HT_REMOVE(chan_circid_map, &chan_circid_map, &search);
  if (ent && ent->circuit) {
    log_warn(LD_BUG, "Tried to mark %u usable on %p, but there was already "
             "a circuit there.", (unsigned)id, chan);
    return;
  }
  if (_last_circid_chan_ent == ent)
    _last_circid_chan_ent = NULL;
  tor_free(ent);
}

/** Called to indicate that a DESTROY is pending on <b>chan</b> with
 * circuit ID <b>id</b>, but hasn't been sent yet. */
void
channel_note_destroy_pending(channel_t *chan, circid_t id)
{
  circuit_t *circ = circuit_get_by_circid_channel_even_if_marked(id,chan);
  if (circ) {
    if (circ->n_chan == chan && circ->n_circ_id == id) {
      circ->n_delete_pending = 1;
    } else {
      or_circuit_t *orcirc = TO_OR_CIRCUIT(circ);
      if (orcirc->p_chan == chan && orcirc->p_circ_id == id) {
        circ->p_delete_pending = 1;
      }
    }
    return;
  }
  channel_mark_circid_unusable(chan, id);
}

/** Called to indicate that a DESTROY is no longer pending on <b>chan</b> with
 * circuit ID <b>id</b> -- typically, because it has been sent. */
MOCK_IMPL(void,
channel_note_destroy_not_pending,(channel_t *chan, circid_t id))
{
  circuit_t *circ = circuit_get_by_circid_channel_even_if_marked(id,chan);
  if (circ) {
    if (circ->n_chan == chan && circ->n_circ_id == id) {
      circ->n_delete_pending = 0;
    } else {
      or_circuit_t *orcirc = TO_OR_CIRCUIT(circ);
      if (orcirc->p_chan == chan && orcirc->p_circ_id == id) {
        circ->p_delete_pending = 0;
      }
    }
    /* XXXX this shouldn't happen; log a bug here. */
    return;
  }
  channel_mark_circid_usable(chan, id);
}

/** Set the p_conn field of a circuit <b>circ</b>, along
 * with the corresponding circuit ID, and add the circuit as appropriate
 * to the (chan,id)-\>circuit map. */
void
circuit_set_p_circid_chan(or_circuit_t *or_circ, circid_t id,
                          channel_t *chan)
{
  circuit_t *circ = TO_CIRCUIT(or_circ);
  channel_t *old_chan = or_circ->p_chan;
  circid_t old_id = or_circ->p_circ_id;

  circuit_set_circid_chan_helper(circ, CELL_DIRECTION_IN, id, chan);

  if (chan) {
    chan->timestamp_last_had_circuits = approx_time();
  }

  if (circ->p_delete_pending && old_chan) {
    channel_mark_circid_unusable(old_chan, old_id);
    circ->p_delete_pending = 0;
  }
}

/** Set the n_conn field of a circuit <b>circ</b>, along
 * with the corresponding circuit ID, and add the circuit as appropriate
 * to the (chan,id)-\>circuit map. */
void
circuit_set_n_circid_chan(circuit_t *circ, circid_t id,
                          channel_t *chan)
{
  channel_t *old_chan = circ->n_chan;
  circid_t old_id = circ->n_circ_id;

  circuit_set_circid_chan_helper(circ, CELL_DIRECTION_OUT, id, chan);

  if (chan) {
    chan->timestamp_last_had_circuits = approx_time();
  }

  if (circ->n_delete_pending && old_chan) {
    channel_mark_circid_unusable(old_chan, old_id);
    circ->n_delete_pending = 0;
  }
}

/**
 * Helper function to publish a message about events on an origin circuit
 *
 * Publishes a message to subscribers of origin circuit events, and
 * sends the control event.
 **/
int
circuit_event_status(origin_circuit_t *circ, circuit_status_event_t tp,
                     int reason_code)
{
  ocirc_cevent_msg_t *msg = tor_malloc(sizeof(*msg));

  tor_assert(circ);

  msg->gid = circ->global_identifier;
  msg->evtype = tp;
  msg->reason = reason_code;
  msg->onehop = circ->build_state->onehop_tunnel;

  ocirc_cevent_publish(msg);
  return control_event_circuit_status(circ, tp, reason_code);
}

/**
 * Helper function to publish a state change message
 *
 * circuit_set_state() calls this to notify subscribers about a change
 * of the state of an origin circuit.  @a circ must be an origin
 * circuit.
 **/
static void
circuit_state_publish(const circuit_t *circ)
{
  ocirc_state_msg_t *msg = tor_malloc(sizeof(*msg));
  const origin_circuit_t *ocirc;

  tor_assert(CIRCUIT_IS_ORIGIN(circ));
  ocirc = CONST_TO_ORIGIN_CIRCUIT(circ);
  /* Only inbound OR circuits can be in this state, not origin circuits. */
  tor_assert(circ->state != CIRCUIT_STATE_ONIONSKIN_PENDING);

  msg->gid = ocirc->global_identifier;
  msg->state = circ->state;
  msg->onehop = ocirc->build_state->onehop_tunnel;

  ocirc_state_publish(msg);
}

/** Change the state of <b>circ</b> to <b>state</b>, adding it to or removing
 * it from lists as appropriate. */
void
circuit_set_state(circuit_t *circ, uint8_t state)
{
  tor_assert(circ);
  if (state == circ->state)
    return;
  if (PREDICT_UNLIKELY(!circuits_pending_chans))
    circuits_pending_chans = smartlist_new();
  if (PREDICT_UNLIKELY(!circuits_pending_other_guards))
    circuits_pending_other_guards = smartlist_new();
  if (circ->state == CIRCUIT_STATE_CHAN_WAIT) {
    /* remove from waiting-circuit list. */
    smartlist_remove(circuits_pending_chans, circ);
  }
  if (state == CIRCUIT_STATE_CHAN_WAIT) {
    /* add to waiting-circuit list. */
    smartlist_add(circuits_pending_chans, circ);
  }
  if (circ->state == CIRCUIT_STATE_GUARD_WAIT) {
    smartlist_remove(circuits_pending_other_guards, circ);
  }
  if (state == CIRCUIT_STATE_GUARD_WAIT) {
    smartlist_add(circuits_pending_other_guards, circ);
  }
  if (state == CIRCUIT_STATE_GUARD_WAIT || state == CIRCUIT_STATE_OPEN)
    tor_assert(!circ->n_chan_create_cell);

  tor_trace(TR_SUBSYS(circuit), TR_EV(change_state), circ, circ->state, state);
  circ->state = state;
  if (CIRCUIT_IS_ORIGIN(circ))
    circuit_state_publish(circ);
}

/** Append to <b>out</b> all circuits in state CHAN_WAIT waiting for
 * the given connection. */
void
circuit_get_all_pending_on_channel(smartlist_t *out, channel_t *chan)
{
  tor_assert(out);
  tor_assert(chan);

  if (!circuits_pending_chans)
    return;

  SMARTLIST_FOREACH_BEGIN(circuits_pending_chans, circuit_t *, circ) {
    if (circ->marked_for_close)
      continue;
    if (!circ->n_hop)
      continue;
    tor_assert(circ->state == CIRCUIT_STATE_CHAN_WAIT);
    if (tor_digest_is_zero(circ->n_hop->identity_digest)) {
      /* Look at addr/port. This is an unkeyed connection. */
      if (!channel_matches_extend_info(chan, circ->n_hop))
        continue;
    } else {
      /* We expected a key. See if it's the right one. */
      if (tor_memneq(chan->identity_digest,
                     circ->n_hop->identity_digest, DIGEST_LEN))
        continue;
    }
    smartlist_add(out, circ);
  } SMARTLIST_FOREACH_END(circ);
}

/** Return the number of circuits in state CHAN_WAIT, waiting for the given
 * channel. */
int
circuit_count_pending_on_channel(channel_t *chan)
{
  int cnt;
  smartlist_t *sl = smartlist_new();

  tor_assert(chan);

  circuit_get_all_pending_on_channel(sl, chan);
  cnt = smartlist_len(sl);
  smartlist_free(sl);
  log_debug(LD_CIRC,"or_conn to %s, %d pending circs",
            channel_describe_peer(chan),
            cnt);
  return cnt;
}

/** Remove <b>origin_circ</b> from the global list of origin circuits.
 * Called when we are freeing a circuit.
 */
static void
circuit_remove_from_origin_circuit_list(origin_circuit_t *origin_circ)
{
  int origin_idx = origin_circ->global_origin_circuit_list_idx;
  if (origin_idx < 0)
    return;
  origin_circuit_t *c2;
  tor_assert(origin_idx <= smartlist_len(global_origin_circuit_list));
  c2 = smartlist_get(global_origin_circuit_list, origin_idx);
  tor_assert(origin_circ == c2);
  smartlist_del(global_origin_circuit_list, origin_idx);
  if (origin_idx < smartlist_len(global_origin_circuit_list)) {
    origin_circuit_t *replacement =
      smartlist_get(global_origin_circuit_list, origin_idx);
    replacement->global_origin_circuit_list_idx = origin_idx;
  }
  origin_circ->global_origin_circuit_list_idx = -1;
}

/** Add <b>origin_circ</b> to the global list of origin circuits. Called
 * when creating the circuit. */
static void
circuit_add_to_origin_circuit_list(origin_circuit_t *origin_circ)
{
  tor_assert(origin_circ->global_origin_circuit_list_idx == -1);
  smartlist_t *lst = circuit_get_global_origin_circuit_list();
  smartlist_add(lst, origin_circ);
  origin_circ->global_origin_circuit_list_idx = smartlist_len(lst) - 1;
}

/** Detach from the global circuit list, and deallocate, all
 * circuits that have been marked for close.
 */
void
circuit_close_all_marked(void)
{
  if (circuits_pending_close == NULL)
    return;

  smartlist_t *lst = circuit_get_global_list();
  SMARTLIST_FOREACH_BEGIN(circuits_pending_close, circuit_t *, circ) {
    tor_assert(circ->marked_for_close);

    /* Remove it from the circuit list. */
    int idx = circ->global_circuitlist_idx;
    smartlist_del(lst, idx);
    if (idx < smartlist_len(lst)) {
      circuit_t *replacement = smartlist_get(lst, idx);
      replacement->global_circuitlist_idx = idx;
    }
    circ->global_circuitlist_idx = -1;

    /* Remove it from the origin circuit list, if appropriate. */
    if (CIRCUIT_IS_ORIGIN(circ)) {
      circuit_remove_from_origin_circuit_list(TO_ORIGIN_CIRCUIT(circ));
    }

    circuit_about_to_free(circ);
    circuit_free(circ);
  } SMARTLIST_FOREACH_END(circ);

  smartlist_clear(circuits_pending_close);
}

/** Return a pointer to the global list of circuits. */
MOCK_IMPL(smartlist_t *,
circuit_get_global_list,(void))
{
  if (NULL == global_circuitlist)
    global_circuitlist = smartlist_new();
  return global_circuitlist;
}

/** Return a pointer to the global list of origin circuits. */
smartlist_t *
circuit_get_global_origin_circuit_list(void)
{
  if (NULL == global_origin_circuit_list)
    global_origin_circuit_list = smartlist_new();
  return global_origin_circuit_list;
}

/**
 * Return true if we have any opened general-purpose 3 hop
 * origin circuits.
 *
 * The result from this function is cached for use by
 * circuit_any_opened_circuits_cached().
 */
int
circuit_any_opened_circuits(void)
{
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_origin_circuit_list(),
          const origin_circuit_t *, next_circ) {
    if (!TO_CIRCUIT(next_circ)->marked_for_close &&
        next_circ->has_opened &&
        TO_CIRCUIT(next_circ)->state == CIRCUIT_STATE_OPEN &&
        TO_CIRCUIT(next_circ)->purpose != CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT &&
        next_circ->build_state &&
        next_circ->build_state->desired_path_len == DEFAULT_ROUTE_LEN) {
      circuit_cache_opened_circuit_state(1);
      return 1;
    }
  } SMARTLIST_FOREACH_END(next_circ);

  circuit_cache_opened_circuit_state(0);
  return 0;
}

/**
 * Cache the "any circuits opened" state, as specified in param
 * circuits_are_opened. This is a helper function to update
 * the circuit opened status whenever we happen to look at the
 * circuit list.
 */
void
circuit_cache_opened_circuit_state(int circuits_are_opened)
{
  any_opened_circs_cached_val = circuits_are_opened;
}

/**
 * Return true if there were any opened circuits since the last call to
 * circuit_any_opened_circuits(), or since circuit_expire_building() last
 * ran (it runs roughly once per second).
 */
int
circuit_any_opened_circuits_cached(void)
{
  return any_opened_circs_cached_val;
}

/** Function to make circ-\>state human-readable */
const char *
circuit_state_to_string(int state)
{
  static char buf[64];
  switch (state) {
    case CIRCUIT_STATE_BUILDING: return "doing handshakes";
    case CIRCUIT_STATE_ONIONSKIN_PENDING: return "processing the onion";
    case CIRCUIT_STATE_CHAN_WAIT: return "connecting to server";
    case CIRCUIT_STATE_GUARD_WAIT: return "waiting to see how other "
      "guards perform";
    case CIRCUIT_STATE_OPEN: return "open";
    default:
      log_warn(LD_BUG, "Unknown circuit state %d", state);
      tor_snprintf(buf, sizeof(buf), "unknown state [%d]", state);
      return buf;
  }
}

/** Map a circuit purpose to a string suitable to be displayed to a
 * controller. */
const char *
circuit_purpose_to_controller_string(uint8_t purpose)
{
  static char buf[32];
  switch (purpose) {
    case CIRCUIT_PURPOSE_OR:
    case CIRCUIT_PURPOSE_INTRO_POINT:
    case CIRCUIT_PURPOSE_REND_POINT_WAITING:
    case CIRCUIT_PURPOSE_REND_ESTABLISHED:
      return "SERVER"; /* A controller should never see these, actually. */

    case CIRCUIT_PURPOSE_C_GENERAL:
      return "GENERAL";

    case CIRCUIT_PURPOSE_C_HSDIR_GET:
      return "HS_CLIENT_HSDIR";

    case CIRCUIT_PURPOSE_C_INTRODUCING:
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT:
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACKED:
      return "HS_CLIENT_INTRO";

    case CIRCUIT_PURPOSE_C_ESTABLISH_REND:
    case CIRCUIT_PURPOSE_C_REND_READY:
    case CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED:
    case CIRCUIT_PURPOSE_C_REND_JOINED:
      return "HS_CLIENT_REND";

    case CIRCUIT_PURPOSE_S_HSDIR_POST:
      return "HS_SERVICE_HSDIR";

    case CIRCUIT_PURPOSE_S_ESTABLISH_INTRO:
    case CIRCUIT_PURPOSE_S_INTRO:
      return "HS_SERVICE_INTRO";

    case CIRCUIT_PURPOSE_S_CONNECT_REND:
    case CIRCUIT_PURPOSE_S_REND_JOINED:
      return "HS_SERVICE_REND";

    case CIRCUIT_PURPOSE_TESTING:
      return "TESTING";
    case CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT:
      return "MEASURE_TIMEOUT";
    case CIRCUIT_PURPOSE_CONTROLLER:
      return "CONTROLLER";
    case CIRCUIT_PURPOSE_PATH_BIAS_TESTING:
      return "PATH_BIAS_TESTING";
    case CIRCUIT_PURPOSE_HS_VANGUARDS:
      return "HS_VANGUARDS";
    case CIRCUIT_PURPOSE_C_CIRCUIT_PADDING:
      return "CIRCUIT_PADDING";

    default:
      tor_snprintf(buf, sizeof(buf), "UNKNOWN_%d", (int)purpose);
      return buf;
  }
}

/** Return a string specifying the state of the hidden-service circuit
 * purpose <b>purpose</b>, or NULL if <b>purpose</b> is not a
 * hidden-service-related circuit purpose. */
const char *
circuit_purpose_to_controller_hs_state_string(uint8_t purpose)
{
  switch (purpose)
    {
    default:
      log_fn(LOG_WARN, LD_BUG,
             "Unrecognized circuit purpose: %d",
             (int)purpose);
      tor_fragile_assert();
      FALLTHROUGH_UNLESS_ALL_BUGS_ARE_FATAL;

    case CIRCUIT_PURPOSE_OR:
    case CIRCUIT_PURPOSE_C_GENERAL:
    case CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT:
    case CIRCUIT_PURPOSE_TESTING:
    case CIRCUIT_PURPOSE_CONTROLLER:
    case CIRCUIT_PURPOSE_PATH_BIAS_TESTING:
    case CIRCUIT_PURPOSE_HS_VANGUARDS:
    case CIRCUIT_PURPOSE_C_CIRCUIT_PADDING:
      return NULL;

    case CIRCUIT_PURPOSE_INTRO_POINT:
      return "OR_HSSI_ESTABLISHED";
    case CIRCUIT_PURPOSE_REND_POINT_WAITING:
      return "OR_HSCR_ESTABLISHED";
    case CIRCUIT_PURPOSE_REND_ESTABLISHED:
      return "OR_HS_R_JOINED";

    case CIRCUIT_PURPOSE_C_HSDIR_GET:
    case CIRCUIT_PURPOSE_C_INTRODUCING:
      return "HSCI_CONNECTING";
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT:
      return "HSCI_INTRO_SENT";
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACKED:
      return "HSCI_DONE";

    case CIRCUIT_PURPOSE_C_ESTABLISH_REND:
      return "HSCR_CONNECTING";
    case CIRCUIT_PURPOSE_C_REND_READY:
      return "HSCR_ESTABLISHED_IDLE";
    case CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED:
      return "HSCR_ESTABLISHED_WAITING";
    case CIRCUIT_PURPOSE_C_REND_JOINED:
      return "HSCR_JOINED";

    case CIRCUIT_PURPOSE_S_HSDIR_POST:
    case CIRCUIT_PURPOSE_S_ESTABLISH_INTRO:
      return "HSSI_CONNECTING";
    case CIRCUIT_PURPOSE_S_INTRO:
      return "HSSI_ESTABLISHED";

    case CIRCUIT_PURPOSE_S_CONNECT_REND:
      return "HSSR_CONNECTING";
    case CIRCUIT_PURPOSE_S_REND_JOINED:
      return "HSSR_JOINED";
    }
}

/** Return a human-readable string for the circuit purpose <b>purpose</b>. */
const char *
circuit_purpose_to_string(uint8_t purpose)
{
  static char buf[32];

  switch (purpose)
    {
    case CIRCUIT_PURPOSE_OR:
      return "Circuit at relay";
    case CIRCUIT_PURPOSE_INTRO_POINT:
      return "Acting as intro point";
    case CIRCUIT_PURPOSE_REND_POINT_WAITING:
      return "Acting as rendezvous (pending)";
    case CIRCUIT_PURPOSE_REND_ESTABLISHED:
      return "Acting as rendezvous (established)";
    case CIRCUIT_PURPOSE_C_GENERAL:
      return "General-purpose client";
    case CIRCUIT_PURPOSE_C_INTRODUCING:
      return "Hidden service client: Connecting to intro point";
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT:
      return "Hidden service client: Waiting for ack from intro point";
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACKED:
      return "Hidden service client: Received ack from intro point";
    case CIRCUIT_PURPOSE_C_ESTABLISH_REND:
      return "Hidden service client: Establishing rendezvous point";
    case CIRCUIT_PURPOSE_C_REND_READY:
      return "Hidden service client: Pending rendezvous point";
    case CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED:
      return "Hidden service client: Pending rendezvous point (ack received)";
    case CIRCUIT_PURPOSE_C_REND_JOINED:
      return "Hidden service client: Active rendezvous point";
    case CIRCUIT_PURPOSE_C_HSDIR_GET:
      return "Hidden service client: Fetching HS descriptor";

    case CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT:
      return "Measuring circuit timeout";

    case CIRCUIT_PURPOSE_S_ESTABLISH_INTRO:
      return "Hidden service: Establishing introduction point";
    case CIRCUIT_PURPOSE_S_INTRO:
      return "Hidden service: Introduction point";
    case CIRCUIT_PURPOSE_S_CONNECT_REND:
      return "Hidden service: Connecting to rendezvous point";
    case CIRCUIT_PURPOSE_S_REND_JOINED:
      return "Hidden service: Active rendezvous point";
    case CIRCUIT_PURPOSE_S_HSDIR_POST:
      return "Hidden service: Uploading HS descriptor";

    case CIRCUIT_PURPOSE_TESTING:
      return "Testing circuit";

    case CIRCUIT_PURPOSE_CONTROLLER:
      return "Circuit made by controller";

    case CIRCUIT_PURPOSE_PATH_BIAS_TESTING:
      return "Path-bias testing circuit";

    case CIRCUIT_PURPOSE_HS_VANGUARDS:
      return "Hidden service: Pre-built vanguard circuit";

    case CIRCUIT_PURPOSE_C_CIRCUIT_PADDING:
      return "Circuit kept open for padding";

    default:
      tor_snprintf(buf, sizeof(buf), "UNKNOWN_%d", (int)purpose);
      return buf;
  }
}

/** Pick a reasonable package_window to start out for our circuits.
 * Originally this was hard-coded at 1000, but now the consensus votes
 * on the answer. See proposal 168. */
int32_t
circuit_initial_package_window(void)
{
  int32_t num = networkstatus_get_param(NULL, "circwindow", CIRCWINDOW_START,
                                        CIRCWINDOW_START_MIN,
                                        CIRCWINDOW_START_MAX);
  /* If the consensus tells us a negative number, we'd assert. */
  if (num < 0)
    num = CIRCWINDOW_START;
  return num;
}

/** Initialize the common elements in a circuit_t, and add it to the global
 * list. */
static void
init_circuit_base(circuit_t *circ)
{
  tor_gettimeofday(&circ->timestamp_created);

  // Gets reset when we send CREATE_FAST.
  // circuit_expire_building() expects these to be equal
  // until the orconn is built.
  circ->timestamp_began = circ->timestamp_created;

  circ->package_window = circuit_initial_package_window();
  circ->deliver_window = CIRCWINDOW_START;
  circuit_reset_sendme_randomness(circ);
  cell_queue_init(&circ->n_chan_cells);

  smartlist_add(circuit_get_global_list(), circ);
  circ->global_circuitlist_idx = smartlist_len(circuit_get_global_list()) - 1;
}

/** If we haven't yet decided on a good timeout value for circuit
 * building, we close idle circuits aggressively so we can get more
 * data points. These are the default, min, and max consensus values */
#define DFLT_IDLE_TIMEOUT_WHILE_LEARNING (3*60)
#define MIN_IDLE_TIMEOUT_WHILE_LEARNING (10)
#define MAX_IDLE_TIMEOUT_WHILE_LEARNING (1000*60)

/** Allocate space for a new circuit, initializing with <b>p_circ_id</b>
 * and <b>p_conn</b>. Add it to the global circuit list.
 */
origin_circuit_t *
origin_circuit_new(void)
{
  origin_circuit_t *circ;
  /* never zero, since a global ID of 0 is treated specially by the
   * controller */
  static uint32_t n_circuits_allocated = 1;

  circ = tor_malloc_zero(sizeof(origin_circuit_t));
  circ->base_.magic = ORIGIN_CIRCUIT_MAGIC;

  circ->next_stream_id = crypto_rand_int(1<<16);
  circ->global_identifier = n_circuits_allocated++;
  circ->remaining_relay_early_cells = MAX_RELAY_EARLY_CELLS_PER_CIRCUIT;
  circ->remaining_relay_early_cells -= crypto_rand_int(2);

  init_circuit_base(TO_CIRCUIT(circ));

  /* Add to origin-list. */
  circ->global_origin_circuit_list_idx = -1;
  circuit_add_to_origin_circuit_list(circ);

  circuit_build_times_update_last_circ(get_circuit_build_times_mutable());

  if (! circuit_build_times_disabled(get_options()) &&
      circuit_build_times_needs_circuits(get_circuit_build_times())) {
    /* Circuits should be shorter lived if we need more of them
     * for learning a good build timeout */
    circ->circuit_idle_timeout =
      networkstatus_get_param(NULL, "cbtlearntimeout",
                              DFLT_IDLE_TIMEOUT_WHILE_LEARNING,
                              MIN_IDLE_TIMEOUT_WHILE_LEARNING,
                              MAX_IDLE_TIMEOUT_WHILE_LEARNING);
  } else {
    // This should always be larger than the current port prediction time
    // remaining, or else we'll end up with the case where a circuit times out
    // and another one is built, effectively doubling the timeout window.
    //
    // We also randomize it by up to 5% more (ie 5% of 0 to 3600 seconds,
    // depending on how much circuit prediction time is remaining) so that
    // we don't close a bunch of unused circuits all at the same time.
    int prediction_time_remaining =
      predicted_ports_prediction_time_remaining(time(NULL));
    circ->circuit_idle_timeout = prediction_time_remaining+1+
        crypto_rand_int(1+prediction_time_remaining/20);

    if (circ->circuit_idle_timeout <= 0) {
      log_warn(LD_BUG,
               "Circuit chose a negative idle timeout of %d based on "
               "%d seconds of predictive building remaining.",
               circ->circuit_idle_timeout,
               prediction_time_remaining);
      circ->circuit_idle_timeout =
          networkstatus_get_param(NULL, "cbtlearntimeout",
                  DFLT_IDLE_TIMEOUT_WHILE_LEARNING,
                  MIN_IDLE_TIMEOUT_WHILE_LEARNING,
                  MAX_IDLE_TIMEOUT_WHILE_LEARNING);
    }

    log_info(LD_CIRC,
              "Circuit %"PRIu32" chose an idle timeout of %d based on "
              "%d seconds of predictive building remaining.",
              (circ->global_identifier),
              circ->circuit_idle_timeout,
              prediction_time_remaining);
  }

  tor_trace(TR_SUBSYS(circuit), TR_EV(new_origin), circ);
  return circ;
}

/** Allocate a new or_circuit_t, connected to <b>p_chan</b> as
 * <b>p_circ_id</b>.  If <b>p_chan</b> is NULL, the circuit is unattached. */
or_circuit_t *
or_circuit_new(circid_t p_circ_id, channel_t *p_chan)
{
  /* CircIDs */
  or_circuit_t *circ;

  circ = tor_malloc_zero(sizeof(or_circuit_t));
  circ->base_.magic = OR_CIRCUIT_MAGIC;

  if (p_chan)
    circuit_set_p_circid_chan(circ, p_circ_id, p_chan);

  circ->remaining_relay_early_cells = MAX_RELAY_EARLY_CELLS_PER_CIRCUIT;
  cell_queue_init(&circ->p_chan_cells);

  init_circuit_base(TO_CIRCUIT(circ));

  tor_trace(TR_SUBSYS(circuit), TR_EV(new_or), circ);
  return circ;
}

/** Free all storage held in circ->testing_cell_stats */
void
circuit_clear_testing_cell_stats(circuit_t *circ)
{
  if (!circ || !circ->testing_cell_stats)
    return;
  SMARTLIST_FOREACH(circ->testing_cell_stats, testing_cell_stats_entry_t *,
                    ent, tor_free(ent));
  smartlist_free(circ->testing_cell_stats);
  circ->testing_cell_stats = NULL;
}

/** Deallocate space associated with circ.
 */
STATIC void
circuit_free_(circuit_t *circ)
{
  circid_t n_circ_id = 0;
  void *mem;
  size_t memlen;
  int should_free = 1;
  if (!circ)
    return;

  /* We keep a copy of this so we can log its value before it gets unset. */
  n_circ_id = circ->n_circ_id;

  circuit_clear_testing_cell_stats(circ);

  /* Cleanup circuit from anything HS v3 related. We also do this when the
   * circuit is closed. This is to avoid any code path that free registered
   * circuits without closing them before. This needs to be done before the
   * hs identifier is freed. */
  hs_circ_cleanup_on_free(circ);

  congestion_control_free(circ->ccontrol);

  if (CIRCUIT_IS_ORIGIN(circ)) {
    origin_circuit_t *ocirc = TO_ORIGIN_CIRCUIT(circ);
    mem = ocirc;
    memlen = sizeof(origin_circuit_t);
    tor_assert(circ->magic == ORIGIN_CIRCUIT_MAGIC);

    circuit_remove_from_origin_circuit_list(ocirc);

    if (ocirc->half_streams) {
      SMARTLIST_FOREACH_BEGIN(ocirc->half_streams, half_edge_t *,
                              half_conn) {
        half_edge_free(half_conn);
      } SMARTLIST_FOREACH_END(half_conn);
      smartlist_free(ocirc->half_streams);
    }

    if (ocirc->build_state) {
        extend_info_free(ocirc->build_state->chosen_exit);
    }
    tor_free(ocirc->build_state);

    /* Cancel before freeing, if we haven't already succeeded or failed. */
    if (ocirc->guard_state) {
      entry_guard_cancel(&ocirc->guard_state);
    }
    circuit_guard_state_free(ocirc->guard_state);

    circuit_clear_cpath(ocirc);

    crypto_pk_free(ocirc->intro_key);

    /* Finally, free the identifier of the circuit and nullify it so multiple
     * cleanup will work. */
    hs_ident_circuit_free(ocirc->hs_ident);
    ocirc->hs_ident = NULL;

    tor_free(ocirc->dest_address);
    if (ocirc->socks_username) {
      memwipe(ocirc->socks_username, 0x12, ocirc->socks_username_len);
      tor_free(ocirc->socks_username);
    }
    if (ocirc->socks_password) {
      memwipe(ocirc->socks_password, 0x06, ocirc->socks_password_len);
      tor_free(ocirc->socks_password);
    }
    addr_policy_list_free(ocirc->prepend_policy);
  } else {
    or_circuit_t *ocirc = TO_OR_CIRCUIT(circ);
    /* Remember cell statistics for this circuit before deallocating. */
    if (get_options()->CellStatistics)
      rep_hist_buffer_stats_add_circ(circ, time(NULL));
    mem = ocirc;
    memlen = sizeof(or_circuit_t);
    tor_assert(circ->magic == OR_CIRCUIT_MAGIC);

    should_free = (ocirc->workqueue_entry == NULL);

    relay_crypto_clear(&ocirc->crypto);

    if (ocirc->rend_splice) {
      or_circuit_t *other = ocirc->rend_splice;
      tor_assert(other->base_.magic == OR_CIRCUIT_MAGIC);
      other->rend_splice = NULL;
    }

    /* remove from map. */
    circuit_set_p_circid_chan(ocirc, 0, NULL);

    /* Clear cell queue _after_ removing it from the map.  Otherwise our
     * "active" checks will be violated. */
    cell_queue_clear(&ocirc->p_chan_cells);
  }

  extend_info_free(circ->n_hop);
  tor_free(circ->n_chan_create_cell);

  if (circ->global_circuitlist_idx != -1) {
    int idx = circ->global_circuitlist_idx;
    circuit_t *c2 = smartlist_get(global_circuitlist, idx);
    tor_assert(c2 == circ);
    smartlist_del(global_circuitlist, idx);
    if (idx < smartlist_len(global_circuitlist)) {
      c2 = smartlist_get(global_circuitlist, idx);
      c2->global_circuitlist_idx = idx;
    }
  }

  /* Remove from map. */
  circuit_set_n_circid_chan(circ, 0, NULL);

  /* Clear cell queue _after_ removing it from the map.  Otherwise our
   * "active" checks will be violated. */
  cell_queue_clear(&circ->n_chan_cells);

  /* Cleanup possible SENDME state. */
  if (circ->sendme_last_digests) {
    SMARTLIST_FOREACH(circ->sendme_last_digests, uint8_t *, d, tor_free(d));
    smartlist_free(circ->sendme_last_digests);
  }

  log_info(LD_CIRC, "Circuit %u (id: %" PRIu32 ") has been freed.",
           n_circ_id,
           CIRCUIT_IS_ORIGIN(circ) ?
              TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0);

  /* Free any circuit padding structures */
  circpad_circuit_free_all_machineinfos(circ);

  /* Clear all dangling handle references. */
  circuit_handles_clear(circ);

  /* Tracepoint. Data within the circuit object is recorded so do this before
   * the actual memory free. */
  tor_trace(TR_SUBSYS(circuit), TR_EV(free), circ);

  if (should_free) {
    memwipe(mem, 0xAA, memlen); /* poison memory */
    tor_free(mem);
  } else {
    /* If we made it here, this is an or_circuit_t that still has a pending
     * cpuworker request which we weren't able to cancel.  Instead, set up
     * the magic value so that when the reply comes back, we'll know to discard
     * the reply and free this structure.
     */
    memwipe(mem, 0xAA, memlen);
    circ->magic = DEAD_CIRCUIT_MAGIC;
  }
}

/** Deallocate the linked list circ-><b>cpath</b>, and remove the cpath from
 * <b>circ</b>. */
void
circuit_clear_cpath(origin_circuit_t *circ)
{
  crypt_path_t *victim, *head, *cpath;

  head = cpath = circ->cpath;

  if (!cpath)
    return;

  /* it's a circular list, so we have to notice when we've
   * gone through it once. */
  while (cpath->next && cpath->next != head) {
    victim = cpath;
    cpath = victim->next;
    cpath_free(victim);
  }

  cpath_free(cpath);

  circ->cpath = NULL;
}

/** Release all storage held by circuits. */
void
circuit_free_all(void)
{
  smartlist_t *lst = circuit_get_global_list();

  SMARTLIST_FOREACH_BEGIN(lst, circuit_t *, tmp) {
    if (! CIRCUIT_IS_ORIGIN(tmp)) {
      or_circuit_t *or_circ = TO_OR_CIRCUIT(tmp);
      while (or_circ->resolving_streams) {
        edge_connection_t *next_conn;
        next_conn = or_circ->resolving_streams->next_stream;
        connection_free_(TO_CONN(or_circ->resolving_streams));
        or_circ->resolving_streams = next_conn;
      }
    }
    tmp->global_circuitlist_idx = -1;
    circuit_about_to_free_atexit(tmp);
    circuit_free(tmp);
    SMARTLIST_DEL_CURRENT(lst, tmp);
  } SMARTLIST_FOREACH_END(tmp);

  smartlist_free(lst);
  global_circuitlist = NULL;

  smartlist_free(global_origin_circuit_list);
  global_origin_circuit_list = NULL;

  smartlist_free(circuits_pending_chans);
  circuits_pending_chans = NULL;

  smartlist_free(circuits_pending_close);
  circuits_pending_close = NULL;

  smartlist_free(circuits_pending_other_guards);
  circuits_pending_other_guards = NULL;

  {
    chan_circid_circuit_map_t **elt, **next, *c;
    for (elt = HT_START(chan_circid_map, &chan_circid_map);
         elt;
         elt = next) {
      c = *elt;
      next = HT_NEXT_RMV(chan_circid_map, &chan_circid_map, elt);

      tor_assert(c->circuit == NULL);
      tor_free(c);
    }
  }
  HT_CLEAR(chan_circid_map, &chan_circid_map);
}

/** A helper function for circuit_dump_by_conn() below. Log a bunch
 * of information about circuit <b>circ</b>.
 */
static void
circuit_dump_conn_details(int severity,
                          circuit_t *circ,
                          int conn_array_index,
                          const char *type,
                          circid_t this_circid,
                          circid_t other_circid)
{
  tor_log(severity, LD_CIRC, "Conn %d has %s circuit: circID %u "
      "(other side %u), state %d (%s), born %ld:",
      conn_array_index, type, (unsigned)this_circid, (unsigned)other_circid,
      circ->state, circuit_state_to_string(circ->state),
      (long)circ->timestamp_began.tv_sec);
  if (CIRCUIT_IS_ORIGIN(circ)) { /* circ starts at this node */
    circuit_log_path(severity, LD_CIRC, TO_ORIGIN_CIRCUIT(circ));
  }
}

/** Log, at severity <b>severity</b>, information about each circuit
 * that is connected to <b>conn</b>.
 */
void
circuit_dump_by_conn(connection_t *conn, int severity)
{
  edge_connection_t *tmpconn;

  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    circid_t n_circ_id = circ->n_circ_id, p_circ_id = 0;

    if (circ->marked_for_close) {
      continue;
    }

    if (!CIRCUIT_IS_ORIGIN(circ)) {
      p_circ_id = TO_OR_CIRCUIT(circ)->p_circ_id;
    }

    if (CIRCUIT_IS_ORIGIN(circ)) {
      for (tmpconn=TO_ORIGIN_CIRCUIT(circ)->p_streams; tmpconn;
           tmpconn=tmpconn->next_stream) {
        if (TO_CONN(tmpconn) == conn) {
          circuit_dump_conn_details(severity, circ, conn->conn_array_index,
                                    "App-ward", p_circ_id, n_circ_id);
        }
      }
    }

    if (! CIRCUIT_IS_ORIGIN(circ)) {
      for (tmpconn=TO_OR_CIRCUIT(circ)->n_streams; tmpconn;
           tmpconn=tmpconn->next_stream) {
        if (TO_CONN(tmpconn) == conn) {
          circuit_dump_conn_details(severity, circ, conn->conn_array_index,
                                    "Exit-ward", n_circ_id, p_circ_id);
        }
      }
    }
  }
  SMARTLIST_FOREACH_END(circ);
}

/** Return the circuit whose global ID is <b>id</b>, or NULL if no
 * such circuit exists. */
origin_circuit_t *
circuit_get_by_global_id(uint32_t id)
{
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    if (CIRCUIT_IS_ORIGIN(circ) &&
        TO_ORIGIN_CIRCUIT(circ)->global_identifier == id) {
      if (circ->marked_for_close)
        return NULL;
      else
        return TO_ORIGIN_CIRCUIT(circ);
    }
  }
  SMARTLIST_FOREACH_END(circ);
  return NULL;
}

/** Return a circ such that:
 *  - circ-\>n_circ_id or circ-\>p_circ_id is equal to <b>circ_id</b>, and
 *  - circ is attached to <b>chan</b>, either as p_chan or n_chan.
 * Return NULL if no such circuit exists.
 *
 * If <b>found_entry_out</b> is provided, set it to true if we have a
 * placeholder entry for circid/chan, and leave it unset otherwise.
 */
static inline circuit_t *
circuit_get_by_circid_channel_impl(circid_t circ_id, channel_t *chan,
                                   int *found_entry_out)
{
  chan_circid_circuit_map_t search;
  chan_circid_circuit_map_t *found;

  if (_last_circid_chan_ent &&
      circ_id == _last_circid_chan_ent->circ_id &&
      chan == _last_circid_chan_ent->chan) {
    found = _last_circid_chan_ent;
  } else {
    search.circ_id = circ_id;
    search.chan = chan;
    found = HT_FIND(chan_circid_map, &chan_circid_map, &search);
    _last_circid_chan_ent = found;
  }
  if (found && found->circuit) {
    log_debug(LD_CIRC,
              "circuit_get_by_circid_channel_impl() returning circuit %p for"
              " circ_id %u, channel ID %"PRIu64 " (%p)",
              found->circuit, (unsigned)circ_id,
              (chan->global_identifier), chan);
    if (found_entry_out)
      *found_entry_out = 1;
    return found->circuit;
  }

  log_debug(LD_CIRC,
            "circuit_get_by_circid_channel_impl() found %s for"
            " circ_id %u, channel ID %"PRIu64 " (%p)",
            found ? "placeholder" : "nothing",
            (unsigned)circ_id,
            (chan->global_identifier), chan);

  if (found_entry_out)
    *found_entry_out = found ? 1 : 0;

  return NULL;
  /* The rest of this checks for bugs. Disabled by default. */
  /* We comment it out because coverity complains otherwise.
  {
    circuit_t *circ;
    TOR_LIST_FOREACH(circ, &global_circuitlist, head) {
      if (! CIRCUIT_IS_ORIGIN(circ)) {
        or_circuit_t *or_circ = TO_OR_CIRCUIT(circ);
        if (or_circ->p_chan == chan && or_circ->p_circ_id == circ_id) {
          log_warn(LD_BUG,
                   "circuit matches p_chan, but not in hash table (Bug!)");
          return circ;
        }
      }
      if (circ->n_chan == chan && circ->n_circ_id == circ_id) {
        log_warn(LD_BUG,
                 "circuit matches n_chan, but not in hash table (Bug!)");
        return circ;
      }
    }
    return NULL;
  } */
}

/** Return a circ such that:
 *  - circ-\>n_circ_id or circ-\>p_circ_id is equal to <b>circ_id</b>, and
 *  - circ is attached to <b>chan</b>, either as p_chan or n_chan.
 *  - circ is not marked for close.
 * Return NULL if no such circuit exists.
 */
circuit_t *
circuit_get_by_circid_channel(circid_t circ_id, channel_t *chan)
{
  circuit_t *circ = circuit_get_by_circid_channel_impl(circ_id, chan, NULL);
  if (!circ || circ->marked_for_close)
    return NULL;
  else
    return circ;
}

/** Return a circ such that:
 *  - circ-\>n_circ_id or circ-\>p_circ_id is equal to <b>circ_id</b>, and
 *  - circ is attached to <b>chan</b>, either as p_chan or n_chan.
 * Return NULL if no such circuit exists.
 */
circuit_t *
circuit_get_by_circid_channel_even_if_marked(circid_t circ_id,
                                             channel_t *chan)
{
  return circuit_get_by_circid_channel_impl(circ_id, chan, NULL);
}

/** Return true iff the circuit ID <b>circ_id</b> is currently used by a
 * circuit, marked or not, on <b>chan</b>, or if the circ ID is reserved until
 * a queued destroy cell can be sent.
 *
 * (Return 1 if the circuit is present, marked or not; Return 2
 * if the circuit ID is pending a destroy.)
 **/
int
circuit_id_in_use_on_channel(circid_t circ_id, channel_t *chan)
{
  int found = 0;
  if (circuit_get_by_circid_channel_impl(circ_id, chan, &found) != NULL)
    return 1;
  if (found)
    return 2;
  return 0;
}

/** Helper for debugging 12184.  Returns the time since which 'circ_id' has
 * been marked unusable on 'chan'. */
time_t
circuit_id_when_marked_unusable_on_channel(circid_t circ_id, channel_t *chan)
{
  chan_circid_circuit_map_t search;
  chan_circid_circuit_map_t *found;

  memset(&search, 0, sizeof(search));
  search.circ_id = circ_id;
  search.chan = chan;

  found = HT_FIND(chan_circid_map, &chan_circid_map, &search);

  if (! found || found->circuit)
    return 0;

  return found->made_placeholder_at;
}

/** Return the circuit that a given edge connection is using. */
circuit_t *
circuit_get_by_edge_conn(edge_connection_t *conn)
{
  circuit_t *circ;

  circ = conn->on_circuit;
  tor_assert(!circ ||
             (CIRCUIT_IS_ORIGIN(circ) ? circ->magic == ORIGIN_CIRCUIT_MAGIC
                                      : circ->magic == OR_CIRCUIT_MAGIC));

  return circ;
}

/** For each circuit that has <b>chan</b> as n_chan or p_chan, unlink the
 * circuit from the chan,circid map, and mark it for close if it hasn't
 * been marked already.
 */
void
circuit_unlink_all_from_channel(channel_t *chan, int reason)
{
  smartlist_t *detached = smartlist_new();

/* #define DEBUG_CIRCUIT_UNLINK_ALL */

  channel_unlink_all_circuits(chan, detached);

#ifdef DEBUG_CIRCUIT_UNLINK_ALL
  {
    smartlist_t *detached_2 = smartlist_new();
    int mismatch = 0, badlen = 0;

    SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
      if (circ->n_chan == chan ||
          (!CIRCUIT_IS_ORIGIN(circ) &&
           TO_OR_CIRCUIT(circ)->p_chan == chan)) {
        smartlist_add(detached_2, circ);
      }
    }
    SMARTLIST_FOREACH_END(circ);

    if (smartlist_len(detached) != smartlist_len(detached_2)) {
       log_warn(LD_BUG, "List of detached circuits had the wrong length! "
                "(got %d, should have gotten %d)",
                (int)smartlist_len(detached),
                (int)smartlist_len(detached_2));
       badlen = 1;
    }
    smartlist_sort_pointers(detached);
    smartlist_sort_pointers(detached_2);

    SMARTLIST_FOREACH(detached, circuit_t *, c,
        if (c != smartlist_get(detached_2, c_sl_idx))
          mismatch = 1;
    );

    if (mismatch)
      log_warn(LD_BUG, "Mismatch in list of detached circuits.");

    if (badlen || mismatch) {
      smartlist_free(detached);
      detached = detached_2;
    } else {
      log_notice(LD_CIRC, "List of %d circuits was as expected.",
                (int)smartlist_len(detached));
      smartlist_free(detached_2);
    }
  }
#endif /* defined(DEBUG_CIRCUIT_UNLINK_ALL) */

  SMARTLIST_FOREACH_BEGIN(detached, circuit_t *, circ) {
    int mark = 0;
    if (circ->n_chan == chan) {

      circuit_set_n_circid_chan(circ, 0, NULL);
      mark = 1;

      /* If we didn't request this closure, pass the remote
       * bit to mark_for_close. */
      if (chan->reason_for_closing != CHANNEL_CLOSE_REQUESTED)
        reason |= END_CIRC_REASON_FLAG_REMOTE;
    }
    if (! CIRCUIT_IS_ORIGIN(circ)) {
      or_circuit_t *or_circ = TO_OR_CIRCUIT(circ);
      if (or_circ->p_chan == chan) {
        circuit_set_p_circid_chan(or_circ, 0, NULL);
        mark = 1;
      }
    }
    if (!mark) {
      log_warn(LD_BUG, "Circuit on detached list which I had no reason "
          "to mark");
      continue;
    }
    if (!circ->marked_for_close)
      circuit_mark_for_close(circ, reason);
  } SMARTLIST_FOREACH_END(circ);

  smartlist_free(detached);
}

/** Return the first introduction circuit originating from the global circuit
 * list after <b>start</b> or at the start of the list if <b>start</b> is
 * NULL. Return NULL if no circuit is found.
 *
 * If <b>want_client_circ</b> is true, then we are looking for client-side
 * introduction circuits: A client introduction point circuit has a purpose of
 * either CIRCUIT_PURPOSE_C_INTRODUCING, CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT
 * or CIRCUIT_PURPOSE_C_INTRODUCE_ACKED. This does not return a circuit marked
 * for close, but it returns circuits regardless of their circuit state.
 *
 * If <b>want_client_circ</b> is false, then we are looking for service-side
 * introduction circuits: A service introduction point circuit has a purpose of
 * either CIRCUIT_PURPOSE_S_ESTABLISH_INTRO or CIRCUIT_PURPOSE_S_INTRO. This
 * does not return circuits marked for close, or in any state other than open.
 */
origin_circuit_t *
circuit_get_next_intro_circ(const origin_circuit_t *start,
                            bool want_client_circ)
{
  int idx = 0;
  smartlist_t *lst = circuit_get_global_list();

  if (start) {
    idx = TO_CIRCUIT(start)->global_circuitlist_idx + 1;
  }

  for ( ; idx < smartlist_len(lst); ++idx) {
    circuit_t *circ = smartlist_get(lst, idx);

    /* Ignore a marked for close circuit or if the state is not open. */
    if (circ->marked_for_close) {
      continue;
    }

    /* Depending on whether we are looking for client or service circs, skip
     * circuits with other purposes. */
    if (want_client_circ) {
      if (circ->purpose != CIRCUIT_PURPOSE_C_INTRODUCING &&
          circ->purpose != CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT &&
          circ->purpose != CIRCUIT_PURPOSE_C_INTRODUCE_ACKED) {
        continue;
      }
    } else { /* we are looking for service-side circs */
      if (circ->state != CIRCUIT_STATE_OPEN) {
        continue;
      }
      if (circ->purpose != CIRCUIT_PURPOSE_S_ESTABLISH_INTRO &&
          circ->purpose != CIRCUIT_PURPOSE_S_INTRO) {
        continue;
      }
    }

    /* The purposes we are looking for are only for origin circuits so the
     * following is valid. */
    return TO_ORIGIN_CIRCUIT(circ);
  }
  /* Not found. */
  return NULL;
}

/** Return the first service rendezvous circuit originating from the global
 * circuit list after <b>start</b> or at the start of the list if <b>start</b>
 * is NULL. Return NULL if no circuit is found.
 *
 * A service rendezvous point circuit has a purpose of either
 * CIRCUIT_PURPOSE_S_CONNECT_REND or CIRCUIT_PURPOSE_S_REND_JOINED. This does
 * not return a circuit marked for close and its state must be open. */
origin_circuit_t *
circuit_get_next_service_rp_circ(origin_circuit_t *start)
{
  int idx = 0;
  smartlist_t *lst = circuit_get_global_list();

  if (start) {
    idx = TO_CIRCUIT(start)->global_circuitlist_idx + 1;
  }

  for ( ; idx < smartlist_len(lst); ++idx) {
    circuit_t *circ = smartlist_get(lst, idx);

    /* Ignore a marked for close circuit or purpose not matching a service
     * intro point or if the state is not open. */
    if (circ->marked_for_close || circ->state != CIRCUIT_STATE_OPEN ||
        (circ->purpose != CIRCUIT_PURPOSE_S_CONNECT_REND &&
         circ->purpose != CIRCUIT_PURPOSE_S_REND_JOINED)) {
      continue;
    }
    /* The purposes we are looking for are only for origin circuits so the
     * following is valid. */
    return TO_ORIGIN_CIRCUIT(circ);
  }
  /* Not found. */
  return NULL;
}

/** Return the first circuit originating here in global_circuitlist after
 * <b>start</b> whose purpose is <b>purpose</b>. Return NULL if no circuit is
 * found. If <b>start</b> is NULL, begin at the start of the list. */
origin_circuit_t *
circuit_get_next_by_purpose(origin_circuit_t *start, uint8_t purpose)
{
  int idx;
  smartlist_t *lst = circuit_get_global_list();
  tor_assert(CIRCUIT_PURPOSE_IS_ORIGIN(purpose));
  if (start == NULL)
    idx = 0;
  else
    idx = TO_CIRCUIT(start)->global_circuitlist_idx + 1;

  for ( ; idx < smartlist_len(lst); ++idx) {
    circuit_t *circ = smartlist_get(lst, idx);

    if (circ->marked_for_close)
      continue;
    if (circ->purpose != purpose)
      continue;
    /* At this point we should be able to get a valid origin circuit because
     * the origin purpose we are looking for matches this circuit. */
    if (BUG(!CIRCUIT_PURPOSE_IS_ORIGIN(circ->purpose))) {
      break;
    }
    return TO_ORIGIN_CIRCUIT(circ);
  }
  return NULL;
}

/** We might cannibalize this circuit: Return true if its last hop can be used
 *  as a v3 rendezvous point. */
static int
circuit_can_be_cannibalized_for_v3_rp(const origin_circuit_t *circ)
{
  if (!circ->build_state) {
    return 0;
  }

  extend_info_t *chosen_exit = circ->build_state->chosen_exit;
  if (BUG(!chosen_exit)) {
    return 0;
  }

  const node_t *rp_node = node_get_by_id(chosen_exit->identity_digest);
  if (rp_node) {
    if (node_supports_v3_rendezvous_point(rp_node)) {
      return 1;
    }
  }

  return 0;
}

/** We are trying to create a circuit of purpose <b>purpose</b> and we are
 *  looking for cannibalizable circuits. Return the circuit purpose we would be
 *  willing to cannibalize. */
static uint8_t
get_circuit_purpose_needed_to_cannibalize(uint8_t purpose)
{
  if (circuit_should_use_vanguards(purpose)) {
    /* If we are using vanguards, then we should only cannibalize vanguard
     * circuits so that we get the same path construction logic. */
    return CIRCUIT_PURPOSE_HS_VANGUARDS;
  } else {
    /* If no vanguards are used just get a general circuit! */
    return CIRCUIT_PURPOSE_C_GENERAL;
  }
}

/** Return a circuit that is open, is CIRCUIT_PURPOSE_C_GENERAL,
 * has a timestamp_dirty value of 0, has flags matching the CIRCLAUNCH_*
 * flags in <b>flags</b>, and if info is defined, does not already use info
 * as any of its hops; or NULL if no circuit fits this description.
 *
 * The <b>purpose</b> argument refers to the purpose of the circuit we want to
 * create, not the purpose of the circuit we want to cannibalize.
 *
 * If !CIRCLAUNCH_NEED_UPTIME, prefer returning non-uptime circuits.
 *
 * To "cannibalize" a circuit means to extend it an extra hop, and use it
 * for some other purpose than we had originally intended.  We do this when
 * we want to perform some low-bandwidth task at a specific relay, and we
 * would like the circuit to complete as soon as possible.  (If we were going
 * to use a lot of bandwidth, we wouldn't want a circuit with an extra hop.
 * If we didn't care about circuit completion latency, we would just build
 * a new circuit.)
 */
origin_circuit_t *
circuit_find_to_cannibalize(uint8_t purpose_to_produce, extend_info_t *info,
                            int flags)
{
  origin_circuit_t *best=NULL;
  int need_uptime = (flags & CIRCLAUNCH_NEED_UPTIME) != 0;
  int need_capacity = (flags & CIRCLAUNCH_NEED_CAPACITY) != 0;
  int internal = (flags & CIRCLAUNCH_IS_INTERNAL) != 0;
  const or_options_t *options = get_options();
  /* We want the circuit we are trying to cannibalize to have this purpose */
  int purpose_to_search_for;

  /* Make sure we're not trying to create a onehop circ by
   * cannibalization. */
  tor_assert(!(flags & CIRCLAUNCH_ONEHOP_TUNNEL));

  purpose_to_search_for = get_circuit_purpose_needed_to_cannibalize(
                                                  purpose_to_produce);

  tor_assert_nonfatal(purpose_to_search_for == CIRCUIT_PURPOSE_C_GENERAL ||
                      purpose_to_search_for == CIRCUIT_PURPOSE_HS_VANGUARDS);

  log_debug(LD_CIRC,
            "Hunting for a circ to cannibalize: purpose %d, uptime %d, "
            "capacity %d, internal %d",
            purpose_to_produce, need_uptime, need_capacity, internal);

  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ_) {
    if (CIRCUIT_IS_ORIGIN(circ_) &&
        circ_->state == CIRCUIT_STATE_OPEN &&
        !circ_->marked_for_close &&
        circ_->purpose == purpose_to_search_for &&
        !circ_->timestamp_dirty) {
      origin_circuit_t *circ = TO_ORIGIN_CIRCUIT(circ_);

      /* Only cannibalize from reasonable length circuits. If we
       * want C_GENERAL, then only choose 3 hop circs. If we want
       * HS_VANGUARDS, only choose 4 hop circs.
       */
      if (circ->build_state->desired_path_len !=
          route_len_for_purpose(purpose_to_search_for, NULL)) {
        goto next;
      }

      /* Ignore any circuits for which we can't use the Guard. It is possible
       * that the Guard was removed from the sampled set after the circuit
       * was created, so avoid using it. */
      if (!entry_guard_could_succeed(circ->guard_state)) {
        goto next;
      }

      if ((!need_uptime || circ->build_state->need_uptime) &&
          (!need_capacity || circ->build_state->need_capacity) &&
          (internal == circ->build_state->is_internal) &&
          !circ->unusable_for_new_conns &&
          circ->remaining_relay_early_cells &&
          !circ->build_state->onehop_tunnel &&
          !circ->isolation_values_set) {
        if (info) {
          /* need to make sure we don't duplicate hops */
          crypt_path_t *hop = circ->cpath;
          const node_t *ri1 = node_get_by_id(info->identity_digest);
          do {
            const node_t *ri2;
            if (tor_memeq(hop->extend_info->identity_digest,
                          info->identity_digest, DIGEST_LEN))
              goto next;
            if (ri1 &&
                (ri2 = node_get_by_id(hop->extend_info->identity_digest))
                && nodes_in_same_family(ri1, ri2))
              goto next;
            hop=hop->next;
          } while (hop!=circ->cpath);
        }
        if (options->ExcludeNodes) {
          /* Make sure no existing nodes in the circuit are excluded for
           * general use.  (This may be possible if StrictNodes is 0, and we
           * thought we needed to use an otherwise excluded node for, say, a
           * directory operation.) */
          crypt_path_t *hop = circ->cpath;
          do {
            if (routerset_contains_extendinfo(options->ExcludeNodes,
                                              hop->extend_info))
              goto next;
            hop = hop->next;
          } while (hop != circ->cpath);
        }

        if ((flags & CIRCLAUNCH_IS_V3_RP) &&
            !circuit_can_be_cannibalized_for_v3_rp(circ)) {
          log_debug(LD_GENERAL, "Skipping uncannibalizable circuit for v3 "
                    "rendezvous point.");
          goto next;
        }

        if (!best || (best->build_state->need_uptime && !need_uptime))
          best = circ;
      next: ;
      }
    }
  }
  SMARTLIST_FOREACH_END(circ_);
  return best;
}

/**
 * Check whether any of the origin circuits that are waiting to see if
 * their guard is good enough to use can be upgraded to "ready". If so,
 * return a new smartlist containing them. Otherwise return NULL.
 */
smartlist_t *
circuit_find_circuits_to_upgrade_from_guard_wait(void)
{
  /* Only if some circuit is actually waiting on an upgrade should we
   * run the algorithm. */
  if (! circuits_pending_other_guards ||
      smartlist_len(circuits_pending_other_guards)==0)
    return NULL;
  /* Only if we have some origin circuits should we run the algorithm. */
  if (!global_origin_circuit_list)
    return NULL;

  /* Okay; we can pass our circuit list to entrynodes.c.*/
  smartlist_t *result = smartlist_new();
  int circuits_upgraded  = entry_guards_upgrade_waiting_circuits(
                                                 get_guard_selection_info(),
                                                 global_origin_circuit_list,
                                                 result);
  if (circuits_upgraded && smartlist_len(result)) {
    return result;
  } else {
    smartlist_free(result);
    return NULL;
  }
}

/** Return the number of hops in circuit's path. If circ has no entries,
 * or is NULL, returns 0. */
int
circuit_get_cpath_len(origin_circuit_t *circ)
{
  int n = 0;
  if (circ && circ->cpath) {
    crypt_path_t *cpath, *cpath_next = NULL;
    for (cpath = circ->cpath; cpath_next != circ->cpath; cpath = cpath_next) {
      cpath_next = cpath->next;
      ++n;
    }
  }
  return n;
}

/** Return the number of opened hops in circuit's path.
 * If circ has no entries, or is NULL, returns 0. */
int
circuit_get_cpath_opened_len(const origin_circuit_t *circ)
{
  int n = 0;
  if (circ && circ->cpath) {
    crypt_path_t *cpath, *cpath_next = NULL;
    for (cpath = circ->cpath;
         cpath->state == CPATH_STATE_OPEN
           && cpath_next != circ->cpath;
         cpath = cpath_next) {
      cpath_next = cpath->next;
      ++n;
    }
  }
  return n;
}

/** Return the <b>hopnum</b>th hop in <b>circ</b>->cpath, or NULL if there
 * aren't that many hops in the list. <b>hopnum</b> starts at 1.
 * Returns NULL if <b>hopnum</b> is 0 or negative. */
crypt_path_t *
circuit_get_cpath_hop(origin_circuit_t *circ, int hopnum)
{
  if (circ && circ->cpath && hopnum > 0) {
    crypt_path_t *cpath, *cpath_next = NULL;
    for (cpath = circ->cpath; cpath_next != circ->cpath; cpath = cpath_next) {
      cpath_next = cpath->next;
      if (--hopnum <= 0)
        return cpath;
    }
  }
  return NULL;
}

/** Go through the circuitlist; mark-for-close each circuit that starts
 *  at us but has not yet been used. */
void
circuit_mark_all_unused_circs(void)
{
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    if (CIRCUIT_IS_ORIGIN(circ) &&
        !circ->marked_for_close &&
        !circ->timestamp_dirty)
      circuit_mark_for_close(circ, END_CIRC_REASON_FINISHED);
  }
  SMARTLIST_FOREACH_END(circ);
}

/** Go through the circuitlist; for each circuit that starts at us
 * and is dirty, frob its timestamp_dirty so we won't use it for any
 * new streams.
 *
 * This is useful for letting the user change pseudonyms, so new
 * streams will not be linkable to old streams.
 */
void
circuit_mark_all_dirty_circs_as_unusable(void)
{
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    if (CIRCUIT_IS_ORIGIN(circ) &&
        !circ->marked_for_close &&
        circ->timestamp_dirty) {
      mark_circuit_unusable_for_new_conns(TO_ORIGIN_CIRCUIT(circ));
    }
  }
  SMARTLIST_FOREACH_END(circ);
}

/**
 * Report any queued cells on or_circuits as written in our bandwidth
 * totals, for the specified channel direction.
 *
 * When we close a circuit or clear its cell queues, we've read
 * data and recorded those bytes in our read statistics, but we're
 * not going to write it. This discrepancy can be used by an adversary
 * to infer information from our public relay statistics and perform
 * attacks such as guard discovery.
 *
 * This function is in the critical path of circuit_mark_for_close().
 * It must be (and is) O(1)!
 *
 * See https://bugs.torproject.org/tpo/core/tor/23512
 */
void
circuit_synchronize_written_or_bandwidth(const circuit_t *c,
                                         circuit_channel_direction_t dir)
{
  uint64_t cells;
  uint64_t cell_size;
  uint64_t written_sync;
  const channel_t *chan = NULL;
  const or_circuit_t *or_circ;

  if (!CIRCUIT_IS_ORCIRC(c))
    return;

  or_circ = CONST_TO_OR_CIRCUIT(c);

  if (dir == CIRCUIT_N_CHAN) {
    chan = c->n_chan;
    cells = c->n_chan_cells.n;
  } else {
    chan = or_circ->p_chan;
    cells = or_circ->p_chan_cells.n;
  }

  /* If we still know the chan, determine real cell size. Otherwise,
   * assume it's a wide circid channel */
  if (chan)
    cell_size = get_cell_network_size(chan->wide_circ_ids);
  else
    cell_size = CELL_MAX_NETWORK_SIZE;

  /* If we know the channel, find out if it's IPv6. */
  tor_addr_t remote_addr;
  bool is_ipv6 = chan &&
    channel_get_addr_if_possible(chan, &remote_addr) &&
    tor_addr_family(&remote_addr) == AF_INET6;

  /* The missing written bytes are the cell counts times their cell
   * size plus TLS per cell overhead */
  written_sync = cells*(cell_size+TLS_PER_CELL_OVERHEAD);

  /* Report the missing bytes as written, to avoid asymmetry.
   * We must use time() for consistency with rephist, even though on
   * some very old rare platforms, approx_time() may be faster. */
  bwhist_note_bytes_written(written_sync, time(NULL), is_ipv6);
}

/** Mark <b>circ</b> to be closed next time we call
 * circuit_close_all_marked(). Do any cleanup needed:
 *   - If state is onionskin_pending, remove circ from the onion_pending
 *     list.
 *   - If circ isn't open yet: call circuit_build_failed() if we're
 *     the origin.
 *   - If purpose is C_INTRODUCE_ACK_WAIT, report the intro point
 *     failure we just had to the hidden service client module.
 *   - If purpose is C_INTRODUCING and <b>reason</b> isn't TIMEOUT,
 *     report to the hidden service client module that the intro point
 *     we just tried may be unreachable.
 *   - Send appropriate destroys and edge_destroys for conns and
 *     streams attached to circ.
 *   - If circ->rend_splice is set (we are the midpoint of a joined
 *     rendezvous stream), then mark the other circuit to close as well.
 */
MOCK_IMPL(void,
circuit_mark_for_close_, (circuit_t *circ, int reason, int line,
                          const char *file))
{
  int orig_reason = reason; /* Passed to the controller */
  assert_circuit_ok(circ);
  tor_assert(line);
  tor_assert(file);

  /* Check whether the circuitpadding subsystem wants to block this close */
  if (circpad_marked_circuit_for_padding(circ, reason)) {
    return;
  }

  if (circ->marked_for_close) {
    log_warn(LD_BUG,
        "Duplicate call to circuit_mark_for_close at %s:%d"
        " (first at %s:%d)", file, line,
        circ->marked_for_close_file, circ->marked_for_close);
    return;
  }
  if (reason == END_CIRC_AT_ORIGIN) {
    if (!CIRCUIT_IS_ORIGIN(circ)) {
      log_warn(LD_BUG, "Specified 'at-origin' non-reason for ending circuit, "
               "but circuit was not at origin. (called %s:%d, purpose=%d)",
               file, line, circ->purpose);
    }
    reason = END_CIRC_REASON_NONE;
  }

  if (CIRCUIT_IS_ORIGIN(circ)) {
    if (pathbias_check_close(TO_ORIGIN_CIRCUIT(circ), reason) == -1) {
      /* Don't close it yet, we need to test it first */
      return;
    }

    /* We don't send reasons when closing circuits at the origin. */
    reason = END_CIRC_REASON_NONE;
  }

  circuit_synchronize_written_or_bandwidth(circ, CIRCUIT_N_CHAN);
  circuit_synchronize_written_or_bandwidth(circ, CIRCUIT_P_CHAN);

  if (reason & END_CIRC_REASON_FLAG_REMOTE)
    reason &= ~END_CIRC_REASON_FLAG_REMOTE;

  if (reason < END_CIRC_REASON_MIN_ || reason > END_CIRC_REASON_MAX_) {
    if (!(orig_reason & END_CIRC_REASON_FLAG_REMOTE))
      log_warn(LD_BUG, "Reason %d out of range at %s:%d", reason, file, line);
    reason = END_CIRC_REASON_NONE;
  }

  circ->marked_for_close = line;
  circ->marked_for_close_file = file;
  circ->marked_for_close_reason = reason;
  circ->marked_for_close_orig_reason = orig_reason;

  if (!CIRCUIT_IS_ORIGIN(circ)) {
    or_circuit_t *or_circ = TO_OR_CIRCUIT(circ);
    if (or_circ->rend_splice) {
      if (!or_circ->rend_splice->base_.marked_for_close) {
        /* do this after marking this circuit, to avoid infinite recursion. */
        circuit_mark_for_close(TO_CIRCUIT(or_circ->rend_splice), reason);
      }
      or_circ->rend_splice = NULL;
    }
  }

  /* Notify the HS subsystem that this circuit is closing. */
  hs_circ_cleanup_on_close(circ);

  /* Update stats. */
  if (circ->ccontrol) {
    if (circ->ccontrol->in_slow_start) {
      /* If we are in slow start, only count the ss cwnd if we've sent
       * enough data to get RTT measurements such that we have a min
       * and a max RTT, and they are not the same. This prevents us from
       * averaging and reporting unused and low-use circuits here */
      if (circ->ccontrol->max_rtt_usec != circ->ccontrol->min_rtt_usec) {
        cc_stats_circ_close_ss_cwnd_ma =
          stats_update_running_avg(cc_stats_circ_close_ss_cwnd_ma,
                                   circ->ccontrol->cwnd);
      }
    } else {
      cc_stats_circ_close_cwnd_ma =
        stats_update_running_avg(cc_stats_circ_close_cwnd_ma,
                                 circ->ccontrol->cwnd);
    }
    cc_stats_circs_closed++;
  }

  if (circuits_pending_close == NULL)
    circuits_pending_close = smartlist_new();

  smartlist_add(circuits_pending_close, circ);
  mainloop_schedule_postloop_cleanup();

  log_info(LD_GENERAL, "Circuit %u (id: %" PRIu32 ") marked for close at "
                       "%s:%d (orig reason: %d, new reason: %d)",
           circ->n_circ_id,
           CIRCUIT_IS_ORIGIN(circ) ?
              TO_ORIGIN_CIRCUIT(circ)->global_identifier : 0,
           file, line, orig_reason, reason);
  tor_trace(TR_SUBSYS(circuit), TR_EV(mark_for_close), circ);
}

/** Called immediately before freeing a marked circuit <b>circ</b> from
 * circuit_free_all() while shutting down Tor; this is a safe-at-shutdown
 * version of circuit_about_to_free().  It's important that it at least
 * do circuitmux_detach_circuit() when appropriate.
 */
static void
circuit_about_to_free_atexit(circuit_t *circ)
{

  if (circ->n_chan) {
    circuit_clear_cell_queue(circ, circ->n_chan);
    circuitmux_detach_circuit(circ->n_chan->cmux, circ);
    circuit_set_n_circid_chan(circ, 0, NULL);
  }

  if (! CIRCUIT_IS_ORIGIN(circ)) {
    or_circuit_t *or_circ = TO_OR_CIRCUIT(circ);

    if (or_circ->p_chan) {
      circuit_clear_cell_queue(circ, or_circ->p_chan);
      circuitmux_detach_circuit(or_circ->p_chan->cmux, circ);
      circuit_set_p_circid_chan(or_circ, 0, NULL);
    }
  }
}

/** Called immediately before freeing a marked circuit <b>circ</b>.
 * Disconnects the circuit from other data structures, launches events
 * as appropriate, and performs other housekeeping.
 */
static void
circuit_about_to_free(circuit_t *circ)
{

  int reason = circ->marked_for_close_reason;
  int orig_reason = circ->marked_for_close_orig_reason;

  if (circ->state == CIRCUIT_STATE_ONIONSKIN_PENDING) {
    onion_pending_remove(TO_OR_CIRCUIT(circ));
  }
  /* If the circuit ever became OPEN, we sent it to the reputation history
   * module then.  If it isn't OPEN, we send it there now to remember which
   * links worked and which didn't.
   */
  if (circ->state != CIRCUIT_STATE_OPEN &&
      circ->state != CIRCUIT_STATE_GUARD_WAIT) {
    if (CIRCUIT_IS_ORIGIN(circ)) {
      origin_circuit_t *ocirc = TO_ORIGIN_CIRCUIT(circ);
      circuit_build_failed(ocirc); /* take actions if necessary */
    }
  }
  if (circ->state == CIRCUIT_STATE_CHAN_WAIT) {
    if (circuits_pending_chans)
      smartlist_remove(circuits_pending_chans, circ);
  }
  if (circuits_pending_other_guards) {
    smartlist_remove(circuits_pending_other_guards, circ);
  }
  if (CIRCUIT_IS_ORIGIN(circ)) {
    circuit_event_status(TO_ORIGIN_CIRCUIT(circ),
     (circ->state == CIRCUIT_STATE_OPEN ||
      circ->state == CIRCUIT_STATE_GUARD_WAIT) ?
                                 CIRC_EVENT_CLOSED:CIRC_EVENT_FAILED,
     orig_reason);
  }

  if (circ->n_chan) {
    circuit_clear_cell_queue(circ, circ->n_chan);
    /* Only send destroy if the channel isn't closing anyway */
    if (!CHANNEL_CONDEMNED(circ->n_chan)) {
      channel_send_destroy(circ->n_circ_id, circ->n_chan, reason);
    }
    circuitmux_detach_circuit(circ->n_chan->cmux, circ);
    circuit_set_n_circid_chan(circ, 0, NULL);
  }

  if (! CIRCUIT_IS_ORIGIN(circ)) {
    or_circuit_t *or_circ = TO_OR_CIRCUIT(circ);
    edge_connection_t *conn;
    for (conn=or_circ->n_streams; conn; conn=conn->next_stream)
      connection_edge_destroy(or_circ->p_circ_id, conn);
    or_circ->n_streams = NULL;

    while (or_circ->resolving_streams) {
      conn = or_circ->resolving_streams;
      or_circ->resolving_streams = conn->next_stream;
      if (!conn->base_.marked_for_close) {
        /* The client will see a DESTROY, and infer that the connections
         * are closing because the circuit is getting torn down.  No need
         * to send an end cell. */
        conn->edge_has_sent_end = 1;
        conn->end_reason = END_STREAM_REASON_DESTROY;
        conn->end_reason |= END_STREAM_REASON_FLAG_ALREADY_SENT_CLOSED;
        connection_mark_for_close(TO_CONN(conn));
      }
      conn->on_circuit = NULL;
    }

    if (or_circ->p_chan) {
      circuit_clear_cell_queue(circ, or_circ->p_chan);
      /* Only send destroy if the channel isn't closing anyway */
      if (!CHANNEL_CONDEMNED(or_circ->p_chan)) {
        channel_send_destroy(or_circ->p_circ_id, or_circ->p_chan, reason);
      }
      circuitmux_detach_circuit(or_circ->p_chan->cmux, circ);
      circuit_set_p_circid_chan(or_circ, 0, NULL);
    }

    if (or_circ->n_cells_discarded_at_end) {
      time_t age = approx_time() - circ->timestamp_created.tv_sec;
      note_circ_closed_for_unrecognized_cells(
                      age, or_circ->n_cells_discarded_at_end);
    }
  } else {
    origin_circuit_t *ocirc = TO_ORIGIN_CIRCUIT(circ);
    edge_connection_t *conn;
    for (conn=ocirc->p_streams; conn; conn=conn->next_stream)
      connection_edge_destroy(circ->n_circ_id, conn);
    ocirc->p_streams = NULL;
  }
}

/** Given a marked circuit <b>circ</b>, aggressively free its cell queues to
 * recover memory. */
static void
marked_circuit_free_cells(circuit_t *circ)
{
  if (!circ->marked_for_close) {
    log_warn(LD_BUG, "Called on non-marked circuit");
    return;
  }
  cell_queue_clear(&circ->n_chan_cells);
  if (! CIRCUIT_IS_ORIGIN(circ)) {
    or_circuit_t *orcirc = TO_OR_CIRCUIT(circ);
    cell_queue_clear(&orcirc->p_chan_cells);
  }
}

static size_t
single_conn_free_bytes(connection_t *conn)
{
  size_t result = 0;
  if (conn->inbuf) {
    result += buf_allocation(conn->inbuf);
    buf_clear(conn->inbuf);
  }
  if (conn->outbuf) {
    result += buf_allocation(conn->outbuf);
    buf_clear(conn->outbuf);
  }
  if (conn->type == CONN_TYPE_DIR) {
    dir_connection_t *dir_conn = TO_DIR_CONN(conn);
    if (dir_conn->compress_state) {
      result += tor_compress_state_size(dir_conn->compress_state);
      tor_compress_free(dir_conn->compress_state);
      dir_conn->compress_state = NULL;
    }
  }
  return result;
}

/** Aggressively free buffer contents on all the buffers of all streams in the
 * list starting at <b>stream</b>. Return the number of bytes recovered. */
static size_t
marked_circuit_streams_free_bytes(edge_connection_t *stream)
{
  size_t result = 0;
  for ( ; stream; stream = stream->next_stream) {
    connection_t *conn = TO_CONN(stream);
    result += single_conn_free_bytes(conn);
    if (conn->linked_conn) {
      result += single_conn_free_bytes(conn->linked_conn);
    }
  }
  return result;
}

/** Aggressively free buffer contents on all the buffers of all streams on
 * circuit <b>c</b>. Return the number of bytes recovered. */
static size_t
marked_circuit_free_stream_bytes(circuit_t *c)
{
  if (CIRCUIT_IS_ORIGIN(c)) {
    return marked_circuit_streams_free_bytes(TO_ORIGIN_CIRCUIT(c)->p_streams);
  } else {
    return marked_circuit_streams_free_bytes(TO_OR_CIRCUIT(c)->n_streams);
  }
}

/** Return the number of cells used by the circuit <b>c</b>'s cell queues. */
STATIC size_t
n_cells_in_circ_queues(const circuit_t *c)
{
  size_t n = c->n_chan_cells.n;
  if (! CIRCUIT_IS_ORIGIN(c)) {
    circuit_t *cc = (circuit_t *) c;
    n += TO_OR_CIRCUIT(cc)->p_chan_cells.n;
  }
  return n;
}

/** Return the number of bytes allocated for <b>c</b>'s half-open streams. */
static size_t
circuit_alloc_in_half_streams(const circuit_t *c)
{
  if (! CIRCUIT_IS_ORIGIN(c)) {
    return 0;
  }
  const origin_circuit_t *ocirc = CONST_TO_ORIGIN_CIRCUIT(c);
  if (ocirc->half_streams)
    return smartlist_len(ocirc->half_streams) * sizeof(half_edge_t);
  else
    return 0;
}

/**
 * Return the age of the oldest cell queued on <b>c</b>, in timestamp units.
 * Return 0 if there are no cells queued on c.  Requires that <b>now</b> be
 * the current coarse timestamp.
 *
 * This function will return incorrect results if the oldest cell queued on
 * the circuit is older than about 2**32 msec (about 49 days) old.
 */
STATIC uint32_t
circuit_max_queued_cell_age(const circuit_t *c, uint32_t now)
{
  uint32_t age = 0;
  packed_cell_t *cell;

  if (NULL != (cell = TOR_SIMPLEQ_FIRST(&c->n_chan_cells.head)))
    age = now - cell->inserted_timestamp;

  if (! CIRCUIT_IS_ORIGIN(c)) {
    const or_circuit_t *orcirc = CONST_TO_OR_CIRCUIT(c);
    if (NULL != (cell = TOR_SIMPLEQ_FIRST(&orcirc->p_chan_cells.head))) {
      uint32_t age2 = now - cell->inserted_timestamp;
      if (age2 > age)
        return age2;
    }
  }
  return age;
}

/** Return the age of the oldest buffer chunk on <b>conn</b>, where age is
 * taken in timestamp units before the time <b>now</b>.  If the connection has
 * no data, treat it as having age zero.
 **/
static uint32_t
conn_get_buffer_age(const connection_t *conn, uint32_t now_ts)
{
  uint32_t age = 0, age2;
  if (conn->outbuf) {
    age2 = buf_get_oldest_chunk_timestamp(conn->outbuf, now_ts);
    if (age2 > age)
      age = age2;
  }
  if (conn->inbuf) {
    age2 = buf_get_oldest_chunk_timestamp(conn->inbuf, now_ts);
    if (age2 > age)
      age = age2;
  }
  return age;
}

/** Return the age in timestamp units of the oldest buffer chunk on any stream
 * in the linked list <b>stream</b>, where age is taken in timestamp units
 * before the timestamp <b>now</b>. */
static uint32_t
circuit_get_streams_max_data_age(const edge_connection_t *stream, uint32_t now)
{
  uint32_t age = 0, age2;
  for (; stream; stream = stream->next_stream) {
    const connection_t *conn = TO_CONN(stream);
    age2 = conn_get_buffer_age(conn, now);
    if (age2 > age)
      age = age2;
    if (conn->linked_conn) {
      age2 = conn_get_buffer_age(conn->linked_conn, now);
      if (age2 > age)
        age = age2;
    }
  }
  return age;
}

/** Return the age in timestamp units of the oldest buffer chunk on any stream
 * attached to the circuit <b>c</b>, where age is taken before the timestamp
 * <b>now</b>. */
STATIC uint32_t
circuit_max_queued_data_age(const circuit_t *c, uint32_t now)
{
  if (CIRCUIT_IS_ORIGIN(c)) {
    return circuit_get_streams_max_data_age(
        CONST_TO_ORIGIN_CIRCUIT(c)->p_streams, now);
  } else {
    return circuit_get_streams_max_data_age(
        CONST_TO_OR_CIRCUIT(c)->n_streams, now);
  }
}

/** Return the age of the oldest cell or stream buffer chunk on the circuit
 * <b>c</b>, where age is taken in timestamp units before the timestamp
 * <b>now</b> */
STATIC uint32_t
circuit_max_queued_item_age(const circuit_t *c, uint32_t now)
{
  uint32_t cell_age = circuit_max_queued_cell_age(c, now);
  uint32_t data_age = circuit_max_queued_data_age(c, now);
  if (cell_age > data_age)
    return cell_age;
  else
    return data_age;
}

/** Helper to sort a list of circuit_t by age of oldest item, in descending
 * order. */
static int
circuits_compare_by_oldest_queued_item_(const void **a_, const void **b_)
{
  const circuit_t *a = *a_;
  const circuit_t *b = *b_;
  uint32_t age_a = a->age_tmp;
  uint32_t age_b = b->age_tmp;

  if (age_a < age_b)
    return 1;
  else if (age_a == age_b)
    return 0;
  else
    return -1;
}

static uint32_t now_ts_for_buf_cmp;

/** Helper to sort a list of circuit_t by age of oldest item, in descending
 * order. */
static int
conns_compare_by_buffer_age_(const void **a_, const void **b_)
{
  const connection_t *a = *a_;
  const connection_t *b = *b_;
  time_t age_a = conn_get_buffer_age(a, now_ts_for_buf_cmp);
  time_t age_b = conn_get_buffer_age(b, now_ts_for_buf_cmp);

  if (age_a < age_b)
    return 1;
  else if (age_a == age_b)
    return 0;
  else
    return -1;
}

#define FRACTION_OF_DATA_TO_RETAIN_ON_OOM 0.90

/** We're out of memory for cells, having allocated <b>current_allocation</b>
 * bytes' worth.  Kill the 'worst' circuits until we're under
 * FRACTION_OF_DATA_TO_RETAIN_ON_OOM of our maximum usage.
 *
 * Return the number of bytes removed. */
size_t
circuits_handle_oom(size_t current_allocation)
{
  smartlist_t *circlist;
  smartlist_t *connection_array = get_connection_array();
  int conn_idx;
  size_t mem_to_recover;
  size_t mem_recovered=0;
  int n_circuits_killed=0;
  int n_dirconns_killed=0;
  int n_edgeconns_killed = 0;
  uint32_t now_ts;
  log_notice(LD_GENERAL, "We're low on memory (cell queues total alloc:"
             " %"TOR_PRIuSZ" buffer total alloc: %" TOR_PRIuSZ ","
             " tor compress total alloc: %" TOR_PRIuSZ
             " (zlib: %" TOR_PRIuSZ ", zstd: %" TOR_PRIuSZ ","
             " lzma: %" TOR_PRIuSZ "),"
             " rendezvous cache total alloc: %" TOR_PRIuSZ "). Killing"
             " circuits withover-long queues. (This behavior is controlled by"
             " MaxMemInQueues.)",
             cell_queues_get_total_allocation(),
             buf_get_total_allocation(),
             tor_compress_get_total_allocation(),
             tor_zlib_get_total_allocation(),
             tor_zstd_get_total_allocation(),
             tor_lzma_get_total_allocation(),
             hs_cache_get_total_allocation());
  {
    size_t mem_target = (size_t)(get_options()->MaxMemInQueues *
                                 FRACTION_OF_DATA_TO_RETAIN_ON_OOM);
    if (current_allocation <= mem_target)
      return 0;
    mem_to_recover = current_allocation - mem_target;
  }

  now_ts = monotime_coarse_get_stamp();

  circlist = circuit_get_global_list();
  SMARTLIST_FOREACH_BEGIN(circlist, circuit_t *, circ) {
    circ->age_tmp = circuit_max_queued_item_age(circ, now_ts);
  } SMARTLIST_FOREACH_END(circ);

  /* This is O(n log n); there are faster algorithms we could use instead.
   * Let's hope this doesn't happen enough to be in the critical path. */
  smartlist_sort(circlist, circuits_compare_by_oldest_queued_item_);

  /* Fix up the indices before we run into trouble */
  SMARTLIST_FOREACH_BEGIN(circlist, circuit_t *, circ) {
    circ->global_circuitlist_idx = circ_sl_idx;
  } SMARTLIST_FOREACH_END(circ);

  /* Now sort the connection array ... */
  now_ts_for_buf_cmp = now_ts;
  smartlist_sort(connection_array, conns_compare_by_buffer_age_);
  now_ts_for_buf_cmp = 0;

  /* Fix up the connection array to its new order. */
  SMARTLIST_FOREACH_BEGIN(connection_array, connection_t *, conn) {
    conn->conn_array_index = conn_sl_idx;
  } SMARTLIST_FOREACH_END(conn);

  /* Okay, now the worst circuits and connections are at the front of their
   * respective lists. Let's mark them, and reclaim their storage
   * aggressively. */
  conn_idx = 0;
  SMARTLIST_FOREACH_BEGIN(circlist, circuit_t *, circ) {
    size_t n;
    size_t freed;

    /* Free storage in any non-linked directory connections that have buffered
     * data older than this circuit. */
    while (conn_idx < smartlist_len(connection_array)) {
      connection_t *conn = smartlist_get(connection_array, conn_idx);
      uint32_t conn_age = conn_get_buffer_age(conn, now_ts);
      if (conn_age < circ->age_tmp) {
        break;
      }
      /* Also consider edge connections so we don't accumulate bytes on the
       * outbuf due to a malicious destination holding off the read on us. */
      if ((conn->type == CONN_TYPE_DIR && conn->linked_conn == NULL) ||
          CONN_IS_EDGE(conn)) {
        if (!conn->marked_for_close)
          connection_mark_for_close(conn);
        mem_recovered += single_conn_free_bytes(conn);

        if (conn->type == CONN_TYPE_DIR) {
          ++n_dirconns_killed;
        } else {
          ++n_edgeconns_killed;
        }

        if (mem_recovered >= mem_to_recover)
          goto done_recovering_mem;
      }
      ++conn_idx;
    }

    /* Now, kill the circuit. */
    n = n_cells_in_circ_queues(circ);
    const size_t half_stream_alloc = circuit_alloc_in_half_streams(circ);
    if (! circ->marked_for_close) {
      circuit_mark_for_close(circ, END_CIRC_REASON_RESOURCELIMIT);
    }
    marked_circuit_free_cells(circ);
    freed = marked_circuit_free_stream_bytes(circ);

    ++n_circuits_killed;

    mem_recovered += n * packed_cell_mem_cost();
    mem_recovered += half_stream_alloc;
    mem_recovered += freed;

    if (mem_recovered >= mem_to_recover)
      goto done_recovering_mem;
  } SMARTLIST_FOREACH_END(circ);

 done_recovering_mem:
  log_notice(LD_GENERAL, "Removed %"TOR_PRIuSZ" bytes by killing %d circuits; "
             "%d circuits remain alive. Also killed %d non-linked directory "
             "connections. Killed %d edge connections",
             mem_recovered,
             n_circuits_killed,
             smartlist_len(circlist) - n_circuits_killed,
             n_dirconns_killed,
             n_edgeconns_killed);

  return mem_recovered;
}

/** Verify that circuit <b>c</b> has all of its invariants
 * correct. Trigger an assert if anything is invalid.
 */
MOCK_IMPL(void,
assert_circuit_ok,(const circuit_t *c))
{
  edge_connection_t *conn;
  const or_circuit_t *or_circ = NULL;
  const origin_circuit_t *origin_circ = NULL;

  tor_assert(c);
  tor_assert(c->magic == ORIGIN_CIRCUIT_MAGIC || c->magic == OR_CIRCUIT_MAGIC);
  tor_assert(c->purpose >= CIRCUIT_PURPOSE_MIN_ &&
             c->purpose <= CIRCUIT_PURPOSE_MAX_);

  if (CIRCUIT_IS_ORIGIN(c))
    origin_circ = CONST_TO_ORIGIN_CIRCUIT(c);
  else
    or_circ = CONST_TO_OR_CIRCUIT(c);

  if (c->n_chan) {
    tor_assert(!c->n_hop);

    if (c->n_circ_id) {
      /* We use the _impl variant here to make sure we don't fail on marked
       * circuits, which would not be returned by the regular function. */
      circuit_t *c2 = circuit_get_by_circid_channel_impl(c->n_circ_id,
                                                         c->n_chan, NULL);
      tor_assert(c == c2);
    }
  }
  if (or_circ && or_circ->p_chan) {
    if (or_circ->p_circ_id) {
      /* ibid */
      circuit_t *c2 =
        circuit_get_by_circid_channel_impl(or_circ->p_circ_id,
                                           or_circ->p_chan, NULL);
      tor_assert(c == c2);
    }
  }
  if (or_circ)
    for (conn = or_circ->n_streams; conn; conn = conn->next_stream)
      tor_assert(conn->base_.type == CONN_TYPE_EXIT);

  tor_assert(c->deliver_window >= 0);
  tor_assert(c->package_window >= 0);
  if (c->state == CIRCUIT_STATE_OPEN ||
      c->state == CIRCUIT_STATE_GUARD_WAIT) {
    tor_assert(!c->n_chan_create_cell);
    if (or_circ) {
      relay_crypto_assert_ok(&or_circ->crypto);
    }
  }
  if (c->state == CIRCUIT_STATE_CHAN_WAIT && !c->marked_for_close) {
    tor_assert(circuits_pending_chans &&
               smartlist_contains(circuits_pending_chans, c));
  } else {
    tor_assert(!circuits_pending_chans ||
               !smartlist_contains(circuits_pending_chans, c));
  }
  if (origin_circ && origin_circ->cpath) {
    cpath_assert_ok(origin_circ->cpath);
  }
  if (c->purpose == CIRCUIT_PURPOSE_REND_ESTABLISHED) {
    tor_assert(or_circ);
    if (!c->marked_for_close) {
      tor_assert(or_circ->rend_splice);
      tor_assert(or_circ->rend_splice->rend_splice == or_circ);
    }
    tor_assert(or_circ->rend_splice != or_circ);
  } else {
    tor_assert(!or_circ || !or_circ->rend_splice);
  }
}
