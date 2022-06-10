/* * Copyright (c) 2012-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file circuitmux.c
 * \brief Circuit mux/cell selection abstraction
 *
 * A circuitmux is responsible for <b>MU</b>ltiple<b>X</b>ing all of the
 * circuits that are writing on a single channel. It keeps track of which of
 * these circuits has something to write (aka, "active" circuits), and which
 * one should write next.  A circuitmux corresponds 1:1 with a channel.
 *
 * There can be different implementations of the circuitmux's rules (which
 * decide which circuit is next to write).
 *
 * A circuitmux exposes three distinct
 * interfaces to other components:
 *
 * To channels, which each have a circuitmux_t, the supported operations
 * (invoked from relay.c) are:
 *
 *   circuitmux_get_first_active_circuit():
 *
 *     Pick one of the circuitmux's active circuits to send cells from.
 *
 *   circuitmux_notify_xmit_cells():
 *
 *     Notify the circuitmux that cells have been sent on a circuit.
 *
 * To circuits, the exposed operations are:
 *
 *   circuitmux_attach_circuit():
 *
 *     Attach a circuit to the circuitmux; this will allocate any policy-
 *     specific data wanted for this circuit and add it to the active
 *     circuits list if it has queued cells.
 *
 *   circuitmux_detach_circuit():
 *
 *     Detach a circuit from the circuitmux, freeing associated structures.
 *
 *   circuitmux_clear_num_cells():
 *
 *     Clear the circuitmux's cell counter for this circuit.
 *
 *   circuitmux_set_num_cells():
 *
 *     Set the circuitmux's cell counter for this circuit. One of
 *     circuitmuc_clear_num_cells() or circuitmux_set_num_cells() MUST be
 *     called when the number of cells queued on a circuit changes.
 *
 * See circuitmux.h for the circuitmux_policy_t data structure, which contains
 * a table of function pointers implementing a circuit selection policy, and
 * circuitmux_ewma.c for an example of a circuitmux policy.  Circuitmux
 * policies can be manipulated with:
 *
 *   circuitmux_get_policy():
 *
 *     Return the current policy for a circuitmux_t, if any.
 *
 *   circuitmux_clear_policy():
 *
 *     Remove a policy installed on a circuitmux_t, freeing all associated
 *     data.  The circuitmux will revert to the built-in round-robin behavior.
 *
 *   circuitmux_set_policy():
 *
 *     Install a policy on a circuitmux_t; the appropriate callbacks will be
 *     made to attach all existing circuits to the new policy.
 **/

#define CIRCUITMUX_PRIVATE

#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitmux.h"
#include "core/or/relay.h"

#include "core/or/or_circuit_st.h"

#include "lib/crypt_ops/crypto_util.h"

/*
 * Private typedefs for circuitmux.c
 */

/*
 * Hash table entry (yeah, calling it chanid_circid_muxinfo_s seems to
 * break the hash table code).
 */
typedef struct chanid_circid_muxinfo_t chanid_circid_muxinfo_t;

/*
 * Anything the mux wants to store per-circuit in the map; right now just
 * a count of queued cells.
 */

typedef struct circuit_muxinfo_t circuit_muxinfo_t;

/*
 * This struct holds whatever we want to store per attached circuit on a
 * circuitmux_t; right now, just the count of queued cells and the direction.
 */

struct circuit_muxinfo_t {
  /* Count of cells on this circuit at last update */
  unsigned int cell_count;
  /* Direction of flow */
  cell_direction_t direction;
  /* Policy-specific data */
  circuitmux_policy_circ_data_t *policy_data;
  /* Mark bit for consistency checker */
  unsigned int mark:1;
};

/*
 * A map from channel ID and circuit ID to a circuit_muxinfo_t for that
 * circuit.
 */

struct chanid_circid_muxinfo_t {
  HT_ENTRY(chanid_circid_muxinfo_t) node;
  uint64_t chan_id;
  circid_t circ_id;
  circuit_muxinfo_t muxinfo;
};

/*
 * Static function declarations
 */

static inline int
chanid_circid_entries_eq(chanid_circid_muxinfo_t *a,
                         chanid_circid_muxinfo_t *b);
static inline unsigned int
chanid_circid_entry_hash(chanid_circid_muxinfo_t *a);
static chanid_circid_muxinfo_t *
circuitmux_find_map_entry(circuitmux_t *cmux, circuit_t *circ);
static void
circuitmux_make_circuit_active(circuitmux_t *cmux, circuit_t *circ);
static void
circuitmux_make_circuit_inactive(circuitmux_t *cmux, circuit_t *circ);

/* Static global variables */

/** Count the destroy balance to debug destroy queue logic */
static int64_t global_destroy_ctr = 0;

/* Function definitions */

/**
 * Helper for chanid_circid_cell_count_map_t hash table: compare the channel
 * ID and circuit ID for a and b, and return less than, equal to, or greater
 * than zero appropriately.
 */

static inline int
chanid_circid_entries_eq(chanid_circid_muxinfo_t *a,
                         chanid_circid_muxinfo_t *b)
{
    return a->chan_id == b->chan_id && a->circ_id == b->circ_id;
}

/**
 * Helper: return a hash based on circuit ID and channel ID in a.
 */

static inline unsigned int
chanid_circid_entry_hash(chanid_circid_muxinfo_t *a)
{
  uint8_t data[8 + 4];
  set_uint64(data, a->chan_id);
  set_uint32(data + 8, a->circ_id);
  return (unsigned) siphash24g(data, sizeof(data));
}

/* Emit a bunch of hash table stuff */
HT_PROTOTYPE(chanid_circid_muxinfo_map, chanid_circid_muxinfo_t, node,
             chanid_circid_entry_hash, chanid_circid_entries_eq);
HT_GENERATE2(chanid_circid_muxinfo_map, chanid_circid_muxinfo_t, node,
             chanid_circid_entry_hash, chanid_circid_entries_eq, 0.6,
             tor_reallocarray_, tor_free_);

/*
 * Circuitmux alloc/free functions
 */

/**
 * Allocate a new circuitmux_t
 */

circuitmux_t *
circuitmux_alloc(void)
{
  circuitmux_t *rv = NULL;

  rv = tor_malloc_zero(sizeof(*rv));
  rv->chanid_circid_map = tor_malloc_zero(sizeof(*( rv->chanid_circid_map)));
  HT_INIT(chanid_circid_muxinfo_map, rv->chanid_circid_map);
  destroy_cell_queue_init(&rv->destroy_cell_queue);

  return rv;
}

/**
 * Detach all circuits from a circuitmux (use before circuitmux_free())
 *
 * If <b>detached_out</b> is non-NULL, add every detached circuit_t to
 * detached_out.
 */

void
circuitmux_detach_all_circuits(circuitmux_t *cmux, smartlist_t *detached_out)
{
  chanid_circid_muxinfo_t **i = NULL, *to_remove;
  channel_t *chan = NULL;
  circuit_t *circ = NULL;

  tor_assert(cmux);

  i = HT_START(chanid_circid_muxinfo_map, cmux->chanid_circid_map);
  while (i) {
    to_remove = *i;

    if (! to_remove) {
      log_warn(LD_BUG, "Somehow, an HT iterator gave us a NULL pointer.");
      break;
    } else {
      /* Find a channel and circuit */
      chan = channel_find_by_global_id(to_remove->chan_id);
      if (chan) {
        circ =
          circuit_get_by_circid_channel_even_if_marked(to_remove->circ_id,
                                                       chan);
        if (circ) {
          /* Clear the circuit's mux for this direction */
          if (to_remove->muxinfo.direction == CELL_DIRECTION_OUT) {
            /*
             * Update active_circuits et al.; this does policy notifies, so
             * comes before freeing policy data
             */

            if (to_remove->muxinfo.cell_count > 0) {
              circuitmux_make_circuit_inactive(cmux, circ);
            }

            if (detached_out)
              smartlist_add(detached_out, circ);
          } else if (circ->magic == OR_CIRCUIT_MAGIC) {
            /*
             * Update active_circuits et al.; this does policy notifies, so
             * comes before freeing policy data
             */

            if (to_remove->muxinfo.cell_count > 0) {
              circuitmux_make_circuit_inactive(cmux, circ);
            }

            if (detached_out)
              smartlist_add(detached_out, circ);
          } else {
            /* Complain and move on */
            log_warn(LD_CIRC,
                     "Circuit %u/channel %"PRIu64 " had direction == "
                     "CELL_DIRECTION_IN, but isn't an or_circuit_t",
                     (unsigned)to_remove->circ_id,
                     (to_remove->chan_id));
          }

          /* Free policy-specific data if we have it */
          if (to_remove->muxinfo.policy_data) {
            /*
             * If we have policy data, assert that we have the means to
             * free it
             */
            tor_assert(cmux->policy);
            tor_assert(cmux->policy->free_circ_data);
            /* Call free_circ_data() */
            cmux->policy->free_circ_data(cmux,
                                         cmux->policy_data,
                                         circ,
                                         to_remove->muxinfo.policy_data);
            to_remove->muxinfo.policy_data = NULL;
          }
        } else {
          /* Complain and move on */
          log_warn(LD_CIRC,
                   "Couldn't find circuit %u (for channel %"PRIu64 ")",
                   (unsigned)to_remove->circ_id,
                   (to_remove->chan_id));
        }
      } else {
        /* Complain and move on */
        log_warn(LD_CIRC,
                 "Couldn't find channel %"PRIu64 " (for circuit id %u)",
                 (to_remove->chan_id),
                 (unsigned)to_remove->circ_id);
      }

      /* Assert that we don't have un-freed policy data for this circuit */
      tor_assert(to_remove->muxinfo.policy_data == NULL);
    }

    i = HT_NEXT_RMV(chanid_circid_muxinfo_map, cmux->chanid_circid_map, i);

    /* Free it */
    tor_free(to_remove);
  }

  cmux->n_circuits = 0;
  cmux->n_active_circuits = 0;
  cmux->n_cells = 0;
}

/** Reclaim all circuit IDs currently marked as unusable on <b>chan</b> because
 * of pending destroy cells in <b>cmux</b>.
 *
 * This function must be called AFTER circuits are unlinked from the (channel,
 * circuid-id) map with circuit_unlink_all_from_channel(), but before calling
 * circuitmux_free().
 */
void
circuitmux_mark_destroyed_circids_usable(circuitmux_t *cmux, channel_t *chan)
{
  destroy_cell_t *cell;
  TOR_SIMPLEQ_FOREACH(cell, &cmux->destroy_cell_queue.head, next) {
    channel_mark_circid_usable(chan, cell->circid);
  }
}

/**
 * Free a circuitmux_t; the circuits must be detached first with
 * circuitmux_detach_all_circuits().
 */

void
circuitmux_free_(circuitmux_t *cmux)
{
  if (!cmux) return;

  tor_assert(cmux->n_circuits == 0);
  tor_assert(cmux->n_active_circuits == 0);

  /*
   * Free policy-specific data if we have any; we don't
   * need to do circuitmux_set_policy(cmux, NULL) to cover
   * the circuits because they would have been handled in
   * circuitmux_detach_all_circuits() before this was
   * called.
   */
  if (cmux->policy && cmux->policy->free_cmux_data) {
    if (cmux->policy_data) {
      cmux->policy->free_cmux_data(cmux, cmux->policy_data);
      cmux->policy_data = NULL;
    }
  } else tor_assert(cmux->policy_data == NULL);

  if (cmux->chanid_circid_map) {
    HT_CLEAR(chanid_circid_muxinfo_map, cmux->chanid_circid_map);
    tor_free(cmux->chanid_circid_map);
  }

  /*
   * We're throwing away some destroys; log the counter and
   * adjust the global counter by the queue size.
   */
  if (cmux->destroy_cell_queue.n > 0) {
    cmux->destroy_ctr -= cmux->destroy_cell_queue.n;
    global_destroy_ctr -= cmux->destroy_cell_queue.n;
    log_debug(LD_CIRC,
              "Freeing cmux at %p with %u queued destroys; the last cmux "
              "destroy balance was %"PRId64", global is %"PRId64,
              cmux, cmux->destroy_cell_queue.n,
              (cmux->destroy_ctr),
              (global_destroy_ctr));
  } else {
    log_debug(LD_CIRC,
              "Freeing cmux at %p with no queued destroys, the cmux destroy "
              "balance was %"PRId64", global is %"PRId64,
              cmux,
              (cmux->destroy_ctr),
              (global_destroy_ctr));
  }

  destroy_cell_queue_clear(&cmux->destroy_cell_queue);

  tor_free(cmux);
}

/*
 * Circuitmux policy control functions
 */

/**
 * Remove any policy installed on cmux; all policy data will be freed and
 * cmux behavior will revert to the built-in round-robin active_circuits
 * mechanism.
 */

void
circuitmux_clear_policy(circuitmux_t *cmux)
{
  tor_assert(cmux);

  /* Internally, this is just setting policy to NULL */
  circuitmux_set_policy(cmux, NULL);
}

/**
 * Return the policy currently installed on a circuitmux_t
 */

MOCK_IMPL(const circuitmux_policy_t *,
circuitmux_get_policy, (circuitmux_t *cmux))
{
  tor_assert(cmux);

  return cmux->policy;
}

/**
 * Set policy; allocate for new policy, detach all circuits from old policy
 * if any, attach them to new policy, and free old policy data.
 */

void
circuitmux_set_policy(circuitmux_t *cmux,
                      const circuitmux_policy_t *pol)
{
  const circuitmux_policy_t *old_pol = NULL, *new_pol = NULL;
  circuitmux_policy_data_t *old_pol_data = NULL, *new_pol_data = NULL;
  chanid_circid_muxinfo_t **i = NULL;
  channel_t *chan = NULL;
  uint64_t last_chan_id_searched = 0;
  circuit_t *circ = NULL;

  tor_assert(cmux);

  /* Set up variables */
  old_pol = cmux->policy;
  old_pol_data = cmux->policy_data;
  new_pol = pol;

  /* Check if this is the trivial case */
  if (old_pol == new_pol) return;

  /* Allocate data for new policy, if any */
  if (new_pol && new_pol->alloc_cmux_data) {
    /*
     * If alloc_cmux_data is not null, then we expect to get some policy
     * data.  Assert that we also have free_cmux_data so we can free it
     * when the time comes, and allocate it.
     */
    tor_assert(new_pol->free_cmux_data);
    new_pol_data = new_pol->alloc_cmux_data(cmux);
    tor_assert(new_pol_data);
  }

  /* Install new policy and new policy data on cmux */
  cmux->policy = new_pol;
  cmux->policy_data = new_pol_data;

  /* Iterate over all circuits, attaching/detaching each one */
  i = HT_START(chanid_circid_muxinfo_map, cmux->chanid_circid_map);
  while (i) {
    /* Assert that this entry isn't NULL */
    tor_assert(*i);

    /*
     * Get the channel; since normal case is all circuits on the mux share a
     * channel, we cache last_chan_id_searched
     */
    if (!chan || last_chan_id_searched != (*i)->chan_id) {
      chan = channel_find_by_global_id((*i)->chan_id);
      last_chan_id_searched = (*i)->chan_id;
    }
    tor_assert(chan);

    /* Get the circuit */
    circ = circuit_get_by_circid_channel_even_if_marked((*i)->circ_id, chan);
    tor_assert(circ);

    /* Need to tell old policy it becomes inactive (i.e., it is active) ? */
    if (old_pol && old_pol->notify_circ_inactive &&
        (*i)->muxinfo.cell_count > 0) {
      old_pol->notify_circ_inactive(cmux, old_pol_data, circ,
                                    (*i)->muxinfo.policy_data);
    }

    /* Need to free old policy data? */
    if ((*i)->muxinfo.policy_data) {
      /* Assert that we have the means to free it if we have policy data */
      tor_assert(old_pol);
      tor_assert(old_pol->free_circ_data);
      /* Free it */
      old_pol->free_circ_data(cmux, old_pol_data, circ,
                             (*i)->muxinfo.policy_data);
      (*i)->muxinfo.policy_data = NULL;
    }

    /* Need to allocate new policy data? */
    if (new_pol && new_pol->alloc_circ_data) {
      /*
       * If alloc_circ_data is not null, we expect to get some per-circuit
       * policy data.  Assert that we also have free_circ_data so we can
       * free it when the time comes, and allocate it.
       */
      tor_assert(new_pol->free_circ_data);
      (*i)->muxinfo.policy_data =
        new_pol->alloc_circ_data(cmux, new_pol_data, circ,
                                 (*i)->muxinfo.direction,
                                 (*i)->muxinfo.cell_count);
    }

    /* Need to make active on new policy? */
    if (new_pol && new_pol->notify_circ_active &&
        (*i)->muxinfo.cell_count > 0) {
      new_pol->notify_circ_active(cmux, new_pol_data, circ,
                                  (*i)->muxinfo.policy_data);
    }

    /* Advance to next circuit map entry */
    i = HT_NEXT(chanid_circid_muxinfo_map, cmux->chanid_circid_map, i);
  }

  /* Free data for old policy, if any */
  if (old_pol_data) {
    /*
     * If we had old policy data, we should have an old policy and a free
     * function for it.
     */
    tor_assert(old_pol);
    tor_assert(old_pol->free_cmux_data);
    old_pol->free_cmux_data(cmux, old_pol_data);
    old_pol_data = NULL;
  }
}

/*
 * Circuitmux/circuit attachment status inquiry functions
 */

/**
 * Query the direction of an attached circuit
 */

cell_direction_t
circuitmux_attached_circuit_direction(circuitmux_t *cmux, circuit_t *circ)
{
  chanid_circid_muxinfo_t *hashent = NULL;

  /* Try to find a map entry */
  hashent = circuitmux_find_map_entry(cmux, circ);

  /*
   * This function should only be called on attached circuits; assert that
   * we had a map entry.
   */
  tor_assert(hashent);

  /* Return the direction from the map entry */
  return hashent->muxinfo.direction;
}

/**
 * Find an entry in the cmux's map for this circuit or return NULL if there
 * is none.
 */

static chanid_circid_muxinfo_t *
circuitmux_find_map_entry(circuitmux_t *cmux, circuit_t *circ)
{
  chanid_circid_muxinfo_t search, *hashent = NULL;

  /* Sanity-check parameters */
  tor_assert(cmux);
  tor_assert(cmux->chanid_circid_map);
  tor_assert(circ);

  /* Check if we have n_chan */
  if (circ->n_chan) {
    /* Okay, let's see if it's attached for n_chan/n_circ_id */
    search.chan_id = circ->n_chan->global_identifier;
    search.circ_id = circ->n_circ_id;

    /* Query */
    hashent = HT_FIND(chanid_circid_muxinfo_map, cmux->chanid_circid_map,
                      &search);
  }

  /* Found something? */
  if (hashent) {
    /*
     * Assert that the direction makes sense for a hashent we found by
     * n_chan/n_circ_id before we return it.
     */
    tor_assert(hashent->muxinfo.direction == CELL_DIRECTION_OUT);
  } else {
    /* Not there, have we got a p_chan/p_circ_id to try? */
    if (circ->magic == OR_CIRCUIT_MAGIC) {
      search.circ_id = TO_OR_CIRCUIT(circ)->p_circ_id;
      /* Check for p_chan */
      if (TO_OR_CIRCUIT(circ)->p_chan) {
        search.chan_id = TO_OR_CIRCUIT(circ)->p_chan->global_identifier;
        /* Okay, search for that */
        hashent = HT_FIND(chanid_circid_muxinfo_map, cmux->chanid_circid_map,
                          &search);
        /* Find anything? */
        if (hashent) {
          /* Assert that the direction makes sense before we return it */
          tor_assert(hashent->muxinfo.direction == CELL_DIRECTION_IN);
        }
      }
    }
  }

  /* Okay, hashent is it if it was there */
  return hashent;
}

/**
 * Query whether a circuit is attached to a circuitmux
 */

int
circuitmux_is_circuit_attached(circuitmux_t *cmux, circuit_t *circ)
{
  chanid_circid_muxinfo_t *hashent = NULL;

  /* Look if it's in the circuit map */
  hashent = circuitmux_find_map_entry(cmux, circ);

  return (hashent != NULL);
}

/**
 * Query whether a circuit is active on a circuitmux
 */

int
circuitmux_is_circuit_active(circuitmux_t *cmux, circuit_t *circ)
{
  chanid_circid_muxinfo_t *hashent = NULL;
  int is_active = 0;

  tor_assert(cmux);
  tor_assert(circ);

  /* Look if it's in the circuit map */
  hashent = circuitmux_find_map_entry(cmux, circ);
  if (hashent) {
    /* Check the number of cells on this circuit */
    is_active = (hashent->muxinfo.cell_count > 0);
  }
  /* else not attached, so not active */

  return is_active;
}

/**
 * Query number of available cells for a circuit on a circuitmux
 */

unsigned int
circuitmux_num_cells_for_circuit(circuitmux_t *cmux, circuit_t *circ)
{
  chanid_circid_muxinfo_t *hashent = NULL;
  unsigned int n_cells = 0;

  tor_assert(cmux);
  tor_assert(circ);

  /* Look if it's in the circuit map */
  hashent = circuitmux_find_map_entry(cmux, circ);
  if (hashent) {
    /* Just get the cell count for this circuit */
    n_cells = hashent->muxinfo.cell_count;
  }
  /* else not attached, so 0 cells */

  return n_cells;
}

/**
 * Query total number of available cells on a circuitmux
 */

MOCK_IMPL(unsigned int,
circuitmux_num_cells, (circuitmux_t *cmux))
{
  tor_assert(cmux);

  return cmux->n_cells + cmux->destroy_cell_queue.n;
}

/**
 * Query total number of circuits active on a circuitmux
 */

unsigned int
circuitmux_num_active_circuits(circuitmux_t *cmux)
{
  tor_assert(cmux);

  return cmux->n_active_circuits;
}

/**
 * Query total number of circuits attached to a circuitmux
 */

unsigned int
circuitmux_num_circuits(circuitmux_t *cmux)
{
  tor_assert(cmux);

  return cmux->n_circuits;
}

/*
 * Functions for circuit code to call to update circuit status
 */

/**
 * Attach a circuit to a circuitmux, for the specified direction.
 */

MOCK_IMPL(void,
circuitmux_attach_circuit,(circuitmux_t *cmux, circuit_t *circ,
                           cell_direction_t direction))
{
  channel_t *chan = NULL;
  uint64_t channel_id;
  circid_t circ_id;
  chanid_circid_muxinfo_t search, *hashent = NULL;
  unsigned int cell_count;

  tor_assert(cmux);
  tor_assert(circ);
  tor_assert(direction == CELL_DIRECTION_IN ||
             direction == CELL_DIRECTION_OUT);

  /*
   * Figure out which channel we're using, and get the circuit's current
   * cell count and circuit ID; assert that the circuit is not already
   * attached to another mux.
   */
  if (direction == CELL_DIRECTION_OUT) {
    /* It's n_chan */
    chan = circ->n_chan;
    cell_count = circ->n_chan_cells.n;
    circ_id = circ->n_circ_id;
  } else {
    /* We want p_chan */
    chan = TO_OR_CIRCUIT(circ)->p_chan;
    cell_count = TO_OR_CIRCUIT(circ)->p_chan_cells.n;
    circ_id = TO_OR_CIRCUIT(circ)->p_circ_id;
  }
  /* Assert that we did get a channel */
  tor_assert(chan);
  /* Assert that the circuit ID is sensible */
  tor_assert(circ_id != 0);

  /* Get the channel ID */
  channel_id = chan->global_identifier;

  /* See if we already have this one */
  search.chan_id = channel_id;
  search.circ_id = circ_id;
  hashent = HT_FIND(chanid_circid_muxinfo_map, cmux->chanid_circid_map,
                    &search);

  if (hashent) {
    /*
     * This circuit was already attached to this cmux; make sure the
     * directions match and update the cell count and active circuit count.
     */
    log_info(LD_CIRC,
             "Circuit %u on channel %"PRIu64 " was already attached to "
             "(trying to attach to %p)",
             (unsigned)circ_id, (channel_id),
             cmux);

    /*
     * The mux pointer on this circuit and the direction in result should
     * match; otherwise assert.
     */
    tor_assert(hashent->muxinfo.direction == direction);

    /*
     * Looks okay; just update the cell count and active circuits if we must
     */
    if (hashent->muxinfo.cell_count > 0 && cell_count == 0) {
      --(cmux->n_active_circuits);
      circuitmux_make_circuit_inactive(cmux, circ);
    } else if (hashent->muxinfo.cell_count == 0 && cell_count > 0) {
      ++(cmux->n_active_circuits);
      circuitmux_make_circuit_active(cmux, circ);
    }
    cmux->n_cells -= hashent->muxinfo.cell_count;
    cmux->n_cells += cell_count;
    hashent->muxinfo.cell_count = cell_count;
  } else {
    /*
     * New circuit; add an entry and update the circuit/active circuit
     * counts.
     */
    log_debug(LD_CIRC,
             "Attaching circuit %u on channel %"PRIu64 " to cmux %p",
              (unsigned)circ_id, (channel_id), cmux);

    /* Insert it in the map */
    hashent = tor_malloc_zero(sizeof(*hashent));
    hashent->chan_id = channel_id;
    hashent->circ_id = circ_id;
    hashent->muxinfo.cell_count = cell_count;
    hashent->muxinfo.direction = direction;
    /* Allocate policy specific circuit data if we need it */
    if (cmux->policy->alloc_circ_data) {
      /* Assert that we have the means to free policy-specific data */
      tor_assert(cmux->policy->free_circ_data);
      /* Allocate it */
      hashent->muxinfo.policy_data =
        cmux->policy->alloc_circ_data(cmux,
                                      cmux->policy_data,
                                      circ,
                                      direction,
                                      cell_count);
      /* If we wanted policy data, it's an error  not to get any */
      tor_assert(hashent->muxinfo.policy_data);
    }
    HT_INSERT(chanid_circid_muxinfo_map, cmux->chanid_circid_map,
              hashent);

    /* Update counters */
    ++(cmux->n_circuits);
    if (cell_count > 0) {
      ++(cmux->n_active_circuits);
      circuitmux_make_circuit_active(cmux, circ);
    }
    cmux->n_cells += cell_count;
  }
}

/**
 * Detach a circuit from a circuitmux and update all counters as needed;
 * no-op if not attached.
 */

MOCK_IMPL(void,
circuitmux_detach_circuit,(circuitmux_t *cmux, circuit_t *circ))
{
  chanid_circid_muxinfo_t search, *hashent = NULL;
  /*
   * Use this to keep track of whether we found it for n_chan or
   * p_chan for consistency checking.
   *
   * The 0 initializer is not a valid cell_direction_t value.
   * We assert that it has been replaced with a valid value before it is used.
   */
  cell_direction_t last_searched_direction = 0;

  tor_assert(cmux);
  tor_assert(cmux->chanid_circid_map);
  tor_assert(circ);

  /* See if we have it for n_chan/n_circ_id */
  if (circ->n_chan) {
    search.chan_id = circ->n_chan->global_identifier;
    search.circ_id = circ->n_circ_id;
    hashent = HT_FIND(chanid_circid_muxinfo_map, cmux->chanid_circid_map,
                        &search);
    last_searched_direction = CELL_DIRECTION_OUT;
  }

  /* Got one? If not, see if it's an or_circuit_t and try p_chan/p_circ_id */
  if (!hashent) {
    if (circ->magic == OR_CIRCUIT_MAGIC) {
      search.circ_id = TO_OR_CIRCUIT(circ)->p_circ_id;
      if (TO_OR_CIRCUIT(circ)->p_chan) {
        search.chan_id = TO_OR_CIRCUIT(circ)->p_chan->global_identifier;
        hashent = HT_FIND(chanid_circid_muxinfo_map,
                            cmux->chanid_circid_map,
                            &search);
        last_searched_direction = CELL_DIRECTION_IN;
      }
    }
  }

  tor_assert(last_searched_direction == CELL_DIRECTION_OUT
             || last_searched_direction == CELL_DIRECTION_IN);

  /*
   * If hashent isn't NULL, we have a circuit to detach; don't remove it from
   * the map until later of circuitmux_make_circuit_inactive() breaks.
   */
  if (hashent) {
    /* Update counters */
    --(cmux->n_circuits);
    if (hashent->muxinfo.cell_count > 0) {
      --(cmux->n_active_circuits);
      /* This does policy notifies, so comes before freeing policy data */
      circuitmux_make_circuit_inactive(cmux, circ);
    }
    cmux->n_cells -= hashent->muxinfo.cell_count;

    /* Free policy-specific data if we have it */
    if (hashent->muxinfo.policy_data) {
      /* If we have policy data, assert that we have the means to free it */
      tor_assert(cmux->policy);
      tor_assert(cmux->policy->free_circ_data);
      /* Call free_circ_data() */
      cmux->policy->free_circ_data(cmux,
                                   cmux->policy_data,
                                   circ,
                                   hashent->muxinfo.policy_data);
      hashent->muxinfo.policy_data = NULL;
    }

    /* Consistency check: the direction must match the direction searched */
    tor_assert(last_searched_direction == hashent->muxinfo.direction);

    /* Now remove it from the map */
    HT_REMOVE(chanid_circid_muxinfo_map, cmux->chanid_circid_map, hashent);

    /* Wipe and free the hash entry */
    // This isn't sensitive, but we want to be sure to know if we're accessing
    // this accidentally.
    memwipe(hashent, 0xef, sizeof(*hashent));
    tor_free(hashent);
  }
}

/**
 * Make a circuit active; update active list and policy-specific info, but
 * we don't mess with the counters or hash table here.
 */

static void
circuitmux_make_circuit_active(circuitmux_t *cmux, circuit_t *circ)
{
  tor_assert(cmux);
  tor_assert(cmux->policy);
  tor_assert(circ);

  /* Policy-specific notification */
  if (cmux->policy->notify_circ_active) {
    /* Okay, we need to check the circuit for policy data now */
    chanid_circid_muxinfo_t *hashent = circuitmux_find_map_entry(cmux, circ);
    /* We should have found something */
    tor_assert(hashent);
    /* Notify */
    cmux->policy->notify_circ_active(cmux, cmux->policy_data,
                                     circ, hashent->muxinfo.policy_data);
  }
}

/**
 * Make a circuit inactive; update active list and policy-specific info, but
 * we don't mess with the counters or hash table here.
 */

static void
circuitmux_make_circuit_inactive(circuitmux_t *cmux, circuit_t *circ)
{
  tor_assert(cmux);
  tor_assert(cmux->policy);
  tor_assert(circ);

  /* Policy-specific notification */
  if (cmux->policy->notify_circ_inactive) {
    /* Okay, we need to check the circuit for policy data now */
    chanid_circid_muxinfo_t *hashent = circuitmux_find_map_entry(cmux, circ);
    /* We should have found something */
    tor_assert(hashent);
    /* Notify */
    cmux->policy->notify_circ_inactive(cmux, cmux->policy_data,
                                       circ, hashent->muxinfo.policy_data);
  }
}

/**
 * Clear the cell counter for a circuit on a circuitmux
 */

void
circuitmux_clear_num_cells(circuitmux_t *cmux, circuit_t *circ)
{
  /* This is the same as setting the cell count to zero */
  circuitmux_set_num_cells(cmux, circ, 0);
}

/**
 * Set the cell counter for a circuit on a circuitmux
 */

void
circuitmux_set_num_cells(circuitmux_t *cmux, circuit_t *circ,
                         unsigned int n_cells)
{
  chanid_circid_muxinfo_t *hashent = NULL;

  tor_assert(cmux);
  tor_assert(circ);

  /* Search for this circuit's entry */
  hashent = circuitmux_find_map_entry(cmux, circ);
  /* Assert that we found one */
  tor_assert(hashent);

  /* Update cmux cell counter */
  cmux->n_cells -= hashent->muxinfo.cell_count;
  cmux->n_cells += n_cells;

  /* Do we need to notify a cmux policy? */
  if (cmux->policy->notify_set_n_cells) {
    /* Call notify_set_n_cells */
    cmux->policy->notify_set_n_cells(cmux,
                                     cmux->policy_data,
                                     circ,
                                     hashent->muxinfo.policy_data,
                                     n_cells);
  }

  /*
   * Update cmux active circuit counter: is the old cell count > 0 and the
   * new cell count == 0 ?
   */
  if (hashent->muxinfo.cell_count > 0 && n_cells == 0) {
    --(cmux->n_active_circuits);
    hashent->muxinfo.cell_count = n_cells;
    circuitmux_make_circuit_inactive(cmux, circ);
  /* Is the old cell count == 0 and the new cell count > 0 ? */
  } else if (hashent->muxinfo.cell_count == 0 && n_cells > 0) {
    ++(cmux->n_active_circuits);
    hashent->muxinfo.cell_count = n_cells;
    circuitmux_make_circuit_active(cmux, circ);
  } else {
    hashent->muxinfo.cell_count = n_cells;
  }
}

/*
 * Functions for channel code to call to get a circuit to transmit from or
 * notify that cells have been transmitted.
 */

/**
 * Pick a circuit to send from, using the active circuits list or a
 * circuitmux policy if one is available.  This is called from channel.c.
 *
 * If we would rather send a destroy cell, return NULL and set
 * *<b>destroy_queue_out</b> to the destroy queue.
 *
 * If we have nothing to send, set *<b>destroy_queue_out</b> to NULL and
 * return NULL.
 */

circuit_t *
circuitmux_get_first_active_circuit(circuitmux_t *cmux,
                                    destroy_cell_queue_t **destroy_queue_out)
{
  circuit_t *circ = NULL;

  tor_assert(cmux);
  tor_assert(cmux->policy);
  /* This callback is mandatory. */
  tor_assert(cmux->policy->pick_active_circuit);
  tor_assert(destroy_queue_out);

  *destroy_queue_out = NULL;

  if (cmux->destroy_cell_queue.n &&
        (!cmux->last_cell_was_destroy || cmux->n_active_circuits == 0)) {
    /* We have destroy cells to send, and either we just sent a relay cell,
     * or we have no relay cells to send. */

    /* XXXX We should let the cmux policy have some say in this eventually. */
    /* XXXX Alternating is not a terribly brilliant approach here. */
    *destroy_queue_out = &cmux->destroy_cell_queue;

    cmux->last_cell_was_destroy = 1;
  } else if (cmux->n_active_circuits > 0) {
    /* We also must have a cell available for this to be the case */
    tor_assert(cmux->n_cells > 0);
    /* Do we have a policy-provided circuit selector? */
    circ = cmux->policy->pick_active_circuit(cmux, cmux->policy_data);
    cmux->last_cell_was_destroy = 0;
  } else {
    tor_assert(cmux->n_cells == 0);
    tor_assert(cmux->destroy_cell_queue.n == 0);
  }

  return circ;
}

/**
 * Notify the circuitmux that cells have been sent on a circuit; this
 * is called from channel.c.
 */

void
circuitmux_notify_xmit_cells(circuitmux_t *cmux, circuit_t *circ,
                             unsigned int n_cells)
{
  chanid_circid_muxinfo_t *hashent = NULL;
  int becomes_inactive = 0;

  tor_assert(cmux);
  tor_assert(circ);

  if (n_cells == 0) return;

  /*
   * To handle this, we have to:
   *
   * 1.) Adjust the circuit's cell counter in the cmux hash table
   * 2.) Move the circuit to the tail of the active_circuits linked list
   *     for this cmux, or make the circuit inactive if the cell count
   *     went to zero.
   * 3.) Call cmux->policy->notify_xmit_cells(), if any
   */

  /* Find the hash entry */
  hashent = circuitmux_find_map_entry(cmux, circ);
  /* Assert that we found one */
  tor_assert(hashent);

  /* Adjust the cell counter and assert that we had that many cells to send */
  tor_assert(n_cells <= hashent->muxinfo.cell_count);
  hashent->muxinfo.cell_count -= n_cells;
  /* Do we need to make the circuit inactive? */
  if (hashent->muxinfo.cell_count == 0) becomes_inactive = 1;
  /* Adjust the mux cell counter */
  cmux->n_cells -= n_cells;

  /*
   * We call notify_xmit_cells() before making the circuit inactive if needed,
   * so the policy can always count on this coming in on an active circuit.
   */
  if (cmux->policy->notify_xmit_cells) {
    cmux->policy->notify_xmit_cells(cmux, cmux->policy_data, circ,
                                    hashent->muxinfo.policy_data,
                                    n_cells);
  }

  /*
   * Now make the circuit inactive if needed; this will call the policy's
   * notify_circ_inactive() if present.
   */
  if (becomes_inactive) {
    --(cmux->n_active_circuits);
    circuitmux_make_circuit_inactive(cmux, circ);
  }
}

/**
 * Notify the circuitmux that a destroy was sent, so we can update
 * the counter.
 */

void
circuitmux_notify_xmit_destroy(circuitmux_t *cmux)
{
  tor_assert(cmux);

  --(cmux->destroy_ctr);
  --(global_destroy_ctr);
  log_debug(LD_CIRC,
            "Cmux at %p sent a destroy, cmux counter is now %"PRId64", "
            "global counter is now %"PRId64,
            cmux,
            (cmux->destroy_ctr),
            (global_destroy_ctr));
}

/*DOCDOC */
void
circuitmux_append_destroy_cell(channel_t *chan,
                               circuitmux_t *cmux,
                               circid_t circ_id,
                               uint8_t reason)
{
  destroy_cell_queue_append(&cmux->destroy_cell_queue, circ_id, reason);

  /* Destroy entering the queue, update counters */
  ++(cmux->destroy_ctr);
  ++global_destroy_ctr;
  log_debug(LD_CIRC,
            "Cmux at %p queued a destroy for circ %u, cmux counter is now "
            "%"PRId64", global counter is now %"PRId64,
            cmux, circ_id,
            (cmux->destroy_ctr),
            (global_destroy_ctr));

  /* XXXX Duplicate code from append_cell_to_circuit_queue */
  if (!channel_has_queued_writes(chan)) {
    /* There is no data at all waiting to be sent on the outbuf.  Add a
     * cell, so that we can notice when it gets flushed, flushed_some can
     * get called, and we can start putting more data onto the buffer then.
     */
    log_debug(LD_GENERAL, "Primed a buffer.");
    channel_flush_from_first_active_circuit(chan, 1);
  }
}

/*DOCDOC; for debugging 12184.  This runs slowly. */
int64_t
circuitmux_count_queued_destroy_cells(const channel_t *chan,
                                      const circuitmux_t *cmux)
{
  int64_t n_destroy_cells = cmux->destroy_ctr;
  int64_t destroy_queue_size = cmux->destroy_cell_queue.n;

  int64_t manual_total = 0;
  int64_t manual_total_in_map = 0;
  destroy_cell_t *cell;

  TOR_SIMPLEQ_FOREACH(cell, &cmux->destroy_cell_queue.head, next) {
    circid_t id;
    ++manual_total;

    id = cell->circid;
    if (circuit_id_in_use_on_channel(id, (channel_t*)chan))
      ++manual_total_in_map;
  }

  if (n_destroy_cells != destroy_queue_size ||
      n_destroy_cells != manual_total ||
      n_destroy_cells != manual_total_in_map) {
    log_warn(LD_BUG, "  Discrepancy in counts for queued destroy cells on "
             "circuitmux. n=%"PRId64". queue_size=%"PRId64". "
             "manual_total=%"PRId64". manual_total_in_map=%"PRId64".",
             (n_destroy_cells),
             (destroy_queue_size),
             (manual_total),
             (manual_total_in_map));
  }

  return n_destroy_cells;
}

/**
 * Compare cmuxes to see which is more preferred; return < 0 if
 * cmux_1 has higher priority (i.e., cmux_1 < cmux_2 in the scheduler's
 * sort order), > 0 if cmux_2 has higher priority, or 0 if they are
 * equally preferred.
 *
 * If the cmuxes have different cmux policies or the policy does not
 * support the cmp_cmux method, return 0.
 */

MOCK_IMPL(int,
circuitmux_compare_muxes, (circuitmux_t *cmux_1, circuitmux_t *cmux_2))
{
  const circuitmux_policy_t *policy;

  tor_assert(cmux_1);
  tor_assert(cmux_2);

  if (cmux_1 == cmux_2) {
    /* Equivalent because they're the same cmux */
    return 0;
  }

  if (cmux_1->policy && cmux_2->policy) {
    if (cmux_1->policy == cmux_2->policy) {
      policy = cmux_1->policy;

      if (policy->cmp_cmux) {
        /* Okay, we can compare! */
        return policy->cmp_cmux(cmux_1, cmux_1->policy_data,
                                cmux_2, cmux_2->policy_data);
      } else {
        /*
         * Equivalent because the policy doesn't know how to compare between
         * muxes.
         */
        return 0;
      }
    } else {
      /* Equivalent because they have different policies */
      return 0;
    }
  } else {
    /* Equivalent because one or both are missing a policy */
    return 0;
  }
}
