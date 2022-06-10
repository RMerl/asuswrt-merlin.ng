/* * Copyright (c) 2012-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file circuitmux_ewma.c
 * \brief EWMA circuit selection as a circuitmux_t policy
 *
 * The "EWMA" in this module stands for the "exponentially weighted moving
 * average" of the number of cells sent on each circuit.  The goal is to
 * prioritize cells on circuits that have been quiet recently, by looking at
 * those that have sent few cells over time, prioritizing recent times
 * more than older ones.
 *
 * Specifically, a cell sent at time "now" has weight 1, but a time X ticks
 * before now has weight ewma_scale_factor ^ X , where ewma_scale_factor is
 * between 0.0 and 1.0.
 *
 * For efficiency, we do not re-scale these averages every time we send a
 * cell: that would be horribly inefficient.  Instead, we we keep the cell
 * count on all circuits on the same circuitmux scaled relative to a single
 * tick.  When we add a new cell, we scale its weight depending on the time
 * that has elapsed since the tick.  We do re-scale the circuits on the
 * circuitmux periodically, so that we don't overflow double.
 *
 *
 * This module should be used through the interfaces in circuitmux.c, which it
 * implements.
 *
 **/

#define CIRCUITMUX_EWMA_PRIVATE

#include "orconfig.h"

#include <math.h>

#include "core/or/or.h"
#include "core/or/circuitmux.h"
#include "core/or/circuitmux_ewma.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/nodelist/networkstatus.h"
#include "app/config/or_options_st.h"

/*** EWMA parameter #defines ***/

/** How long does a tick last (seconds)? */
#define EWMA_TICK_LEN_DEFAULT 10
#define EWMA_TICK_LEN_MIN 1
#define EWMA_TICK_LEN_MAX 600
static int ewma_tick_len = EWMA_TICK_LEN_DEFAULT;

/** The default per-tick scale factor, if it hasn't been overridden by a
 * consensus or a configuration setting.  zero means "disabled". */
#define EWMA_DEFAULT_HALFLIFE 0.0

/*** Some useful constant #defines ***/

/** Any halflife smaller than this number of seconds is considered to be
 * "disabled". */
#define EPSILON 0.00001
/** The natural logarithm of 0.5. */
#define LOG_ONEHALF -0.69314718055994529

/*** Static declarations for circuitmux_ewma.c ***/

static void add_cell_ewma(ewma_policy_data_t *pol, cell_ewma_t *ewma);
static int compare_cell_ewma_counts(const void *p1, const void *p2);
static circuit_t * cell_ewma_to_circuit(cell_ewma_t *ewma);
static inline double get_scale_factor(unsigned from_tick, unsigned to_tick);
static cell_ewma_t * pop_first_cell_ewma(ewma_policy_data_t *pol);
static void remove_cell_ewma(ewma_policy_data_t *pol, cell_ewma_t *ewma);
static void scale_single_cell_ewma(cell_ewma_t *ewma, unsigned cur_tick);
static void scale_active_circuits(ewma_policy_data_t *pol,
                                  unsigned cur_tick);

/*** Circuitmux policy methods ***/

static circuitmux_policy_data_t * ewma_alloc_cmux_data(circuitmux_t *cmux);
static void ewma_free_cmux_data(circuitmux_t *cmux,
                                circuitmux_policy_data_t *pol_data);
static circuitmux_policy_circ_data_t *
ewma_alloc_circ_data(circuitmux_t *cmux, circuitmux_policy_data_t *pol_data,
                     circuit_t *circ, cell_direction_t direction,
                     unsigned int cell_count);
static void
ewma_free_circ_data(circuitmux_t *cmux,
                    circuitmux_policy_data_t *pol_data,
                    circuit_t *circ,
                    circuitmux_policy_circ_data_t *pol_circ_data);
static void
ewma_notify_circ_active(circuitmux_t *cmux,
                        circuitmux_policy_data_t *pol_data,
                        circuit_t *circ,
                        circuitmux_policy_circ_data_t *pol_circ_data);
static void
ewma_notify_circ_inactive(circuitmux_t *cmux,
                          circuitmux_policy_data_t *pol_data,
                          circuit_t *circ,
                          circuitmux_policy_circ_data_t *pol_circ_data);
static void
ewma_notify_xmit_cells(circuitmux_t *cmux,
                       circuitmux_policy_data_t *pol_data,
                       circuit_t *circ,
                       circuitmux_policy_circ_data_t *pol_circ_data,
                       unsigned int n_cells);
static circuit_t *
ewma_pick_active_circuit(circuitmux_t *cmux,
                         circuitmux_policy_data_t *pol_data);
static int
ewma_cmp_cmux(circuitmux_t *cmux_1, circuitmux_policy_data_t *pol_data_1,
              circuitmux_t *cmux_2, circuitmux_policy_data_t *pol_data_2);

/*** EWMA global variables ***/

/** The per-tick scale factor to be used when computing cell-count EWMA
 * values.  (A cell sent N ticks before the start of the current tick
 * has value ewma_scale_factor ** N.)
 */
static double ewma_scale_factor = 0.1;

/*** EWMA circuitmux_policy_t method table ***/

circuitmux_policy_t ewma_policy = {
  /*.alloc_cmux_data =*/ ewma_alloc_cmux_data,
  /*.free_cmux_data =*/ ewma_free_cmux_data,
  /*.alloc_circ_data =*/ ewma_alloc_circ_data,
  /*.free_circ_data =*/ ewma_free_circ_data,
  /*.notify_circ_active =*/ ewma_notify_circ_active,
  /*.notify_circ_inactive =*/ ewma_notify_circ_inactive,
  /*.notify_set_n_cells =*/ NULL, /* EWMA doesn't need this */
  /*.notify_xmit_cells =*/ ewma_notify_xmit_cells,
  /*.pick_active_circuit =*/ ewma_pick_active_circuit,
  /*.cmp_cmux =*/ ewma_cmp_cmux
};

/** Have we initialized the ewma tick-counting logic? */
static int ewma_ticks_initialized = 0;
/** At what monotime_coarse_t did the current tick begin? */
static monotime_coarse_t start_of_current_tick;
/** What is the number of the current tick? */
static unsigned current_tick_num;

/*** EWMA method implementations using the below EWMA helper functions ***/

/** Compute and return the current cell_ewma tick. */
static inline unsigned int
cell_ewma_get_tick(void)
{
  monotime_coarse_t now;
  monotime_coarse_get(&now);
  int32_t msec_diff = monotime_coarse_diff_msec32(&start_of_current_tick,
                                                  &now);
  return current_tick_num + msec_diff / (1000*ewma_tick_len);
}

/**
 * Allocate an ewma_policy_data_t and upcast it to a circuitmux_policy_data_t;
 * this is called when setting the policy on a circuitmux_t to ewma_policy.
 */

static circuitmux_policy_data_t *
ewma_alloc_cmux_data(circuitmux_t *cmux)
{
  ewma_policy_data_t *pol = NULL;

  tor_assert(cmux);

  pol = tor_malloc_zero(sizeof(*pol));
  pol->base_.magic = EWMA_POL_DATA_MAGIC;
  pol->active_circuit_pqueue = smartlist_new();
  pol->active_circuit_pqueue_last_recalibrated = cell_ewma_get_tick();

  return TO_CMUX_POL_DATA(pol);
}

/**
 * Free an ewma_policy_data_t allocated with ewma_alloc_cmux_data()
 */

static void
ewma_free_cmux_data(circuitmux_t *cmux,
                    circuitmux_policy_data_t *pol_data)
{
  ewma_policy_data_t *pol = NULL;

  tor_assert(cmux);
  if (!pol_data) return;

  pol = TO_EWMA_POL_DATA(pol_data);

  smartlist_free(pol->active_circuit_pqueue);
  memwipe(pol, 0xda, sizeof(ewma_policy_data_t));
  tor_free(pol);
}

/**
 * Allocate an ewma_policy_circ_data_t and upcast it to a
 * circuitmux_policy_data_t; this is called when attaching a circuit to a
 * circuitmux_t with ewma_policy.
 */

static circuitmux_policy_circ_data_t *
ewma_alloc_circ_data(circuitmux_t *cmux,
                     circuitmux_policy_data_t *pol_data,
                     circuit_t *circ,
                     cell_direction_t direction,
                     unsigned int cell_count)
{
  ewma_policy_circ_data_t *cdata = NULL;

  tor_assert(cmux);
  tor_assert(pol_data);
  tor_assert(circ);
  tor_assert(direction == CELL_DIRECTION_OUT ||
             direction == CELL_DIRECTION_IN);
  /* Shut the compiler up without triggering -Wtautological-compare */
  (void)cell_count;

  cdata = tor_malloc_zero(sizeof(*cdata));
  cdata->base_.magic = EWMA_POL_CIRC_DATA_MAGIC;
  cdata->circ = circ;

  /*
   * Initialize the cell_ewma_t structure (formerly in
   * init_circuit_base())
   */
  cdata->cell_ewma.last_adjusted_tick = cell_ewma_get_tick();
  cdata->cell_ewma.cell_count = 0.0;
  cdata->cell_ewma.heap_index = -1;
  if (direction == CELL_DIRECTION_IN) {
    cdata->cell_ewma.is_for_p_chan = 1;
  } else {
    cdata->cell_ewma.is_for_p_chan = 0;
  }

  return TO_CMUX_POL_CIRC_DATA(cdata);
}

/**
 * Free an ewma_policy_circ_data_t allocated with ewma_alloc_circ_data()
 */

static void
ewma_free_circ_data(circuitmux_t *cmux,
                    circuitmux_policy_data_t *pol_data,
                    circuit_t *circ,
                    circuitmux_policy_circ_data_t *pol_circ_data)

{
  ewma_policy_circ_data_t *cdata = NULL;

  tor_assert(cmux);
  tor_assert(circ);
  tor_assert(pol_data);

  if (!pol_circ_data) return;

  cdata = TO_EWMA_POL_CIRC_DATA(pol_circ_data);
  memwipe(cdata, 0xdc, sizeof(ewma_policy_circ_data_t));
  tor_free(cdata);
}

/**
 * Handle circuit activation; this inserts the circuit's cell_ewma into
 * the active_circuits_pqueue.
 */

static void
ewma_notify_circ_active(circuitmux_t *cmux,
                        circuitmux_policy_data_t *pol_data,
                        circuit_t *circ,
                        circuitmux_policy_circ_data_t *pol_circ_data)
{
  ewma_policy_data_t *pol = NULL;
  ewma_policy_circ_data_t *cdata = NULL;

  tor_assert(cmux);
  tor_assert(pol_data);
  tor_assert(circ);
  tor_assert(pol_circ_data);

  pol = TO_EWMA_POL_DATA(pol_data);
  cdata = TO_EWMA_POL_CIRC_DATA(pol_circ_data);

  add_cell_ewma(pol, &(cdata->cell_ewma));
}

/**
 * Handle circuit deactivation; this removes the circuit's cell_ewma from
 * the active_circuits_pqueue.
 */

static void
ewma_notify_circ_inactive(circuitmux_t *cmux,
                          circuitmux_policy_data_t *pol_data,
                          circuit_t *circ,
                          circuitmux_policy_circ_data_t *pol_circ_data)
{
  ewma_policy_data_t *pol = NULL;
  ewma_policy_circ_data_t *cdata = NULL;

  tor_assert(cmux);
  tor_assert(pol_data);
  tor_assert(circ);
  tor_assert(pol_circ_data);

  pol = TO_EWMA_POL_DATA(pol_data);
  cdata = TO_EWMA_POL_CIRC_DATA(pol_circ_data);

  remove_cell_ewma(pol, &(cdata->cell_ewma));
}

/**
 * Update cell_ewma for this circuit after we've sent some cells, and
 * remove/reinsert it in the queue.  This used to be done (brokenly,
 * see bug 6816) in channel_flush_from_first_active_circuit().
 */

static void
ewma_notify_xmit_cells(circuitmux_t *cmux,
                       circuitmux_policy_data_t *pol_data,
                       circuit_t *circ,
                       circuitmux_policy_circ_data_t *pol_circ_data,
                       unsigned int n_cells)
{
  ewma_policy_data_t *pol = NULL;
  ewma_policy_circ_data_t *cdata = NULL;
  unsigned int tick;
  double fractional_tick, ewma_increment;
  cell_ewma_t *cell_ewma, *tmp;

  tor_assert(cmux);
  tor_assert(pol_data);
  tor_assert(circ);
  tor_assert(pol_circ_data);
  tor_assert(n_cells > 0);

  pol = TO_EWMA_POL_DATA(pol_data);
  cdata = TO_EWMA_POL_CIRC_DATA(pol_circ_data);

  /* Rescale the EWMAs if needed */
  tick = cell_ewma_get_current_tick_and_fraction(&fractional_tick);

  if (tick != pol->active_circuit_pqueue_last_recalibrated) {
    scale_active_circuits(pol, tick);
  }

  /* How much do we adjust the cell count in cell_ewma by? */
  ewma_increment =
    ((double)(n_cells)) * pow(ewma_scale_factor, -fractional_tick);

  /* Do the adjustment */
  cell_ewma = &(cdata->cell_ewma);
  cell_ewma->cell_count += ewma_increment;

  /*
   * Since we just sent on this circuit, it should be at the head of
   * the queue.  Pop the head, assert that it matches, then re-add.
   */
  tmp = pop_first_cell_ewma(pol);
  tor_assert(tmp == cell_ewma);
  add_cell_ewma(pol, cell_ewma);
}

/**
 * Pick the preferred circuit to send from; this will be the one with
 * the lowest EWMA value in the priority queue.  This used to be done
 * in channel_flush_from_first_active_circuit().
 */

static circuit_t *
ewma_pick_active_circuit(circuitmux_t *cmux,
                         circuitmux_policy_data_t *pol_data)
{
  ewma_policy_data_t *pol = NULL;
  circuit_t *circ = NULL;
  cell_ewma_t *cell_ewma = NULL;

  tor_assert(cmux);
  tor_assert(pol_data);

  pol = TO_EWMA_POL_DATA(pol_data);

  if (smartlist_len(pol->active_circuit_pqueue) > 0) {
    /* Get the head of the queue */
    cell_ewma = smartlist_get(pol->active_circuit_pqueue, 0);
    circ = cell_ewma_to_circuit(cell_ewma);
  }

  return circ;
}

/**
 * Compare two EWMA cmuxes, and return -1, 0 or 1 to indicate which should
 * be more preferred - see circuitmux_compare_muxes() of circuitmux.c.
 */

static int
ewma_cmp_cmux(circuitmux_t *cmux_1, circuitmux_policy_data_t *pol_data_1,
              circuitmux_t *cmux_2, circuitmux_policy_data_t *pol_data_2)
{
  ewma_policy_data_t *p1 = NULL, *p2 = NULL;
  cell_ewma_t *ce1 = NULL, *ce2 = NULL;

  tor_assert(cmux_1);
  tor_assert(pol_data_1);
  tor_assert(cmux_2);
  tor_assert(pol_data_2);

  p1 = TO_EWMA_POL_DATA(pol_data_1);
  p2 = TO_EWMA_POL_DATA(pol_data_2);

  if (p1 != p2) {
    /* Get the head cell_ewma_t from each queue */
    if (smartlist_len(p1->active_circuit_pqueue) > 0) {
      ce1 = smartlist_get(p1->active_circuit_pqueue, 0);
    }

    if (smartlist_len(p2->active_circuit_pqueue) > 0) {
      ce2 = smartlist_get(p2->active_circuit_pqueue, 0);
    }

    /* Got both of them? */
    if (ce1 != NULL && ce2 != NULL) {
      /* Pick whichever one has the better best circuit */
      return compare_cell_ewma_counts(ce1, ce2);
    } else {
      if (ce1 != NULL) {
        /* We only have a circuit on cmux_1, so prefer it */
        return -1;
      } else if (ce2 != NULL) {
        /* We only have a circuit on cmux_2, so prefer it */
        return 1;
      } else {
        /* No circuits at all; no preference */
        return 0;
      }
    }
  } else {
    /* We got identical params */
    return 0;
  }
}

/** Helper for sorting cell_ewma_t values in their priority queue. */
static int
compare_cell_ewma_counts(const void *p1, const void *p2)
{
  const cell_ewma_t *e1 = p1, *e2 = p2;

  if (e1->cell_count < e2->cell_count)
    return -1;
  else if (e1->cell_count > e2->cell_count)
    return 1;
  else
    return 0;
}

/** Given a cell_ewma_t, return a pointer to the circuit containing it. */
static circuit_t *
cell_ewma_to_circuit(cell_ewma_t *ewma)
{
  ewma_policy_circ_data_t *cdata = NULL;

  tor_assert(ewma);
  cdata = SUBTYPE_P(ewma, ewma_policy_circ_data_t, cell_ewma);
  tor_assert(cdata);

  return cdata->circ;
}

/* ==== Functions for scaling cell_ewma_t ====

   When choosing which cells to relay first, we favor circuits that have been
   quiet recently.  This gives better latency on connections that aren't
   pushing lots of data, and makes the network feel more interactive.

   Conceptually, we take an exponentially weighted mean average of the number
   of cells a circuit has sent, and allow active circuits (those with cells to
   relay) to send cells in reverse order of their exponentially-weighted mean
   average (EWMA) cell count.  [That is, a cell sent N seconds ago 'counts'
   F^N times as much as a cell sent now, for 0<F<1.0, and we favor the
   circuit that has sent the fewest cells]

   If 'double' had infinite precision, we could do this simply by counting a
   cell sent at startup as having weight 1.0, and a cell sent N seconds later
   as having weight F^-N.  This way, we would never need to re-scale
   any already-sent cells.

   To prevent double from overflowing, we could count a cell sent now as
   having weight 1.0 and a cell sent N seconds ago as having weight F^N.
   This, however, would mean we'd need to re-scale *ALL* old circuits every
   time we wanted to send a cell.

   So as a compromise, we divide time into 'ticks' (currently, 10-second
   increments) and say that a cell sent at the start of a current tick is
   worth 1.0, a cell sent N seconds before the start of the current tick is
   worth F^N, and a cell sent N seconds after the start of the current tick is
   worth F^-N.  This way we don't overflow, and we don't need to constantly
   rescale.
 */

/**
 * Initialize the system that tells which ewma tick we are in.
 */
STATIC void
cell_ewma_initialize_ticks(void)
{
  if (ewma_ticks_initialized)
    return;
  monotime_coarse_get(&start_of_current_tick);
  crypto_rand((char*)&current_tick_num, sizeof(current_tick_num));
  ewma_ticks_initialized = 1;
}

/** Compute the current cell_ewma tick and the fraction of the tick that has
 * elapsed between the start of the tick and the current time.  Return the
 * former and store the latter in *<b>remainder_out</b>.
 *
 * These tick values are not meant to be shared between Tor instances, or used
 * for other purposes. */
STATIC unsigned
cell_ewma_get_current_tick_and_fraction(double *remainder_out)
{
  if (BUG(!ewma_ticks_initialized)) {
    cell_ewma_initialize_ticks(); // LCOV_EXCL_LINE
  }
  monotime_coarse_t now;
  monotime_coarse_get(&now);
  int32_t msec_diff = monotime_coarse_diff_msec32(&start_of_current_tick,
                                                  &now);
  if (msec_diff > (1000*ewma_tick_len)) {
    unsigned ticks_difference = msec_diff / (1000*ewma_tick_len);
    monotime_coarse_add_msec(&start_of_current_tick,
                             &start_of_current_tick,
                             ticks_difference * 1000 * ewma_tick_len);
    current_tick_num += ticks_difference;
    msec_diff %= 1000*ewma_tick_len;
  }
  *remainder_out = ((double)msec_diff) / (1.0e3 * ewma_tick_len);
  return current_tick_num;
}

/* Default value for the CircuitPriorityHalflifeMsec consensus parameter in
 * msec. */
#define CMUX_PRIORITY_HALFLIFE_MSEC_DEFAULT 30000
/* Minimum and maximum value for the CircuitPriorityHalflifeMsec consensus
 * parameter. */
#define CMUX_PRIORITY_HALFLIFE_MSEC_MIN 1
#define CMUX_PRIORITY_HALFLIFE_MSEC_MAX INT32_MAX

/* Return the value of the circuit priority halflife from the options if
 * available or else from the consensus (in that order). If none can be found,
 * a default value is returned.
 *
 * The source_msg points to a string describing from where the value was
 * picked so it can be used for logging. */
static double
get_circuit_priority_halflife(const or_options_t *options,
                              const networkstatus_t *consensus,
                              const char **source_msg)
{
  int32_t halflife_ms;
  double halflife;
  /* Compute the default value now. We might need it. */
  double halflife_default =
    ((double) CMUX_PRIORITY_HALFLIFE_MSEC_DEFAULT) / 1000.0;

  /* Try to get it from configuration file first. */
  if (options && options->CircuitPriorityHalflife >= -EPSILON) {
    halflife = options->CircuitPriorityHalflife;
    *source_msg = "CircuitPriorityHalflife in configuration";
    goto end;
  }

  /* Try to get the msec value from the consensus. */
  halflife_ms = networkstatus_get_param(consensus,
                                        "CircuitPriorityHalflifeMsec",
                                        CMUX_PRIORITY_HALFLIFE_MSEC_DEFAULT,
                                        CMUX_PRIORITY_HALFLIFE_MSEC_MIN,
                                        CMUX_PRIORITY_HALFLIFE_MSEC_MAX);
  halflife = ((double) halflife_ms) / 1000.0;
  *source_msg = "CircuitPriorityHalflifeMsec in consensus";

 end:
  /* We should never go below the EPSILON else we would consider it disabled
   * and we can't have that. */
  if (halflife < EPSILON) {
    log_warn(LD_CONFIG, "CircuitPriorityHalflife is too small (%f). "
                        "Adjusting to the smallest value allowed: %f.",
             halflife, halflife_default);
    halflife = halflife_default;
  }
  return halflife;
}

/** Adjust the global cell scale factor based on <b>options</b> */
void
cmux_ewma_set_options(const or_options_t *options,
                      const networkstatus_t *consensus)
{
  double halflife;
  const char *source;

  cell_ewma_initialize_ticks();

  /* Both options and consensus can be NULL. This assures us to either get a
   * valid configured value or the default one. */
  halflife = get_circuit_priority_halflife(options, consensus, &source);
  ewma_tick_len = networkstatus_get_param(consensus,
                                        "CircuitPriorityTickSecs",
                                        EWMA_TICK_LEN_DEFAULT,
                                        EWMA_TICK_LEN_MIN,
                                        EWMA_TICK_LEN_MAX);

  /* convert halflife into halflife-per-tick. */
  halflife /= ewma_tick_len;
  /* compute per-tick scale factor. */
  ewma_scale_factor = exp(LOG_ONEHALF / halflife);
  log_info(LD_OR,
           "Enabled cell_ewma algorithm because of value in %s; "
           "scale factor is %f per %d seconds",
           source, ewma_scale_factor, ewma_tick_len);
}

/** Return the multiplier necessary to convert the value of a cell sent in
 * 'from_tick' to one sent in 'to_tick'. */
static inline double
get_scale_factor(unsigned from_tick, unsigned to_tick)
{
  /* This math can wrap around, but that's okay: unsigned overflow is
     well-defined */
  int diff = (int)(to_tick - from_tick);
  return pow(ewma_scale_factor, diff);
}

/** Adjust the cell count of <b>ewma</b> so that it is scaled with respect to
 * <b>cur_tick</b> */
static void
scale_single_cell_ewma(cell_ewma_t *ewma, unsigned cur_tick)
{
  double factor = get_scale_factor(ewma->last_adjusted_tick, cur_tick);
  ewma->cell_count *= factor;
  ewma->last_adjusted_tick = cur_tick;
}

/** Adjust the cell count of every active circuit on <b>chan</b> so
 * that they are scaled with respect to <b>cur_tick</b> */
static void
scale_active_circuits(ewma_policy_data_t *pol, unsigned cur_tick)
{
  double factor;

  tor_assert(pol);
  tor_assert(pol->active_circuit_pqueue);

  factor =
    get_scale_factor(
      pol->active_circuit_pqueue_last_recalibrated,
      cur_tick);
  /** Ordinarily it isn't okay to change the value of an element in a heap,
   * but it's okay here, since we are preserving the order. */
  SMARTLIST_FOREACH_BEGIN(
      pol->active_circuit_pqueue,
      cell_ewma_t *, e) {
    tor_assert(e->last_adjusted_tick ==
               pol->active_circuit_pqueue_last_recalibrated);
    e->cell_count *= factor;
    e->last_adjusted_tick = cur_tick;
  } SMARTLIST_FOREACH_END(e);
  pol->active_circuit_pqueue_last_recalibrated = cur_tick;
}

/** Rescale <b>ewma</b> to the same scale as <b>pol</b>, and add it to
 * <b>pol</b>'s priority queue of active circuits */
static void
add_cell_ewma(ewma_policy_data_t *pol, cell_ewma_t *ewma)
{
  tor_assert(pol);
  tor_assert(pol->active_circuit_pqueue);
  tor_assert(ewma);
  tor_assert(ewma->heap_index == -1);

  scale_single_cell_ewma(
      ewma,
      pol->active_circuit_pqueue_last_recalibrated);

  smartlist_pqueue_add(pol->active_circuit_pqueue,
                       compare_cell_ewma_counts,
                       offsetof(cell_ewma_t, heap_index),
                       ewma);
}

/** Remove <b>ewma</b> from <b>pol</b>'s priority queue of active circuits */
static void
remove_cell_ewma(ewma_policy_data_t *pol, cell_ewma_t *ewma)
{
  tor_assert(pol);
  tor_assert(pol->active_circuit_pqueue);
  tor_assert(ewma);
  tor_assert(ewma->heap_index != -1);

  smartlist_pqueue_remove(pol->active_circuit_pqueue,
                          compare_cell_ewma_counts,
                          offsetof(cell_ewma_t, heap_index),
                          ewma);
}

/** Remove and return the first cell_ewma_t from pol's priority queue of
 * active circuits.  Requires that the priority queue is nonempty. */
static cell_ewma_t *
pop_first_cell_ewma(ewma_policy_data_t *pol)
{
  tor_assert(pol);
  tor_assert(pol->active_circuit_pqueue);

  return smartlist_pqueue_pop(pol->active_circuit_pqueue,
                              compare_cell_ewma_counts,
                              offsetof(cell_ewma_t, heap_index));
}

/**
 * Drop all resources held by circuitmux_ewma.c, and deinitialize the
 * module. */
void
circuitmux_ewma_free_all(void)
{
  ewma_ticks_initialized = 0;
}
