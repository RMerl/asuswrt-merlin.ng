/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file predict_ports.c
 * \brief Remember what ports we've needed so we can have circuits ready.
 *
 * Predicted ports are used by clients to remember how long it's been
 * since they opened an exit connection to each given target
 * port. Clients use this information in order to try to keep circuits
 * open to exit nodes that can connect to the ports that they care
 * about.  (The predicted ports mechanism also handles predicted circuit
 * usage that _isn't_ port-specific, such as resolves, internal circuits,
 * and so on.)
 **/

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/or/channelpadding.h"
#include "core/or/circuituse.h"
#include "feature/relay/routermode.h"
#include "feature/relay/selftest.h"
#include "feature/stats/predict_ports.h"
#include "lib/container/bitarray.h"
#include "lib/time/tvdiff.h"

static size_t predicted_ports_total_alloc = 0;

static void predicted_ports_alloc(void);

/** A single predicted port: used to remember which ports we've made
 * connections to, so that we can try to keep making circuits that can handle
 * those ports. */
typedef struct predicted_port_t {
  /** The port we connected to */
  uint16_t port;
  /** The time at which we last used it */
  time_t time;
} predicted_port_t;

/** A list of port numbers that have been used recently. */
static smartlist_t *predicted_ports_list=NULL;
/** How long do we keep predicting circuits? */
static time_t prediction_timeout=0;
/** When was the last time we added a prediction entry (HS or port) */
static time_t last_prediction_add_time=0;

/**
 * How much time left until we stop predicting circuits?
 */
int
predicted_ports_prediction_time_remaining(time_t now)
{
  time_t seconds_waited;
  time_t seconds_left;

  /* Protect against overflow of return value. This can happen if the clock
   * jumps backwards in time. Update the last prediction time (aka last
   * active time) to prevent it. This update is preferable to using monotonic
   * time because it prevents clock jumps into the past from simply causing
   * very long idle timeouts while the monotonic time stands still. */
  seconds_waited = time_diff(last_prediction_add_time, now);
  if (seconds_waited == TIME_MAX) {
    last_prediction_add_time = now;
    seconds_waited = 0;
  }

  /* Protect against underflow of the return value. This can happen for very
   * large periods of inactivity/system sleep. */
  if (seconds_waited > prediction_timeout)
    return 0;

  seconds_left = time_diff(seconds_waited, prediction_timeout);
  if (BUG(seconds_left == TIME_MAX))
    return INT_MAX;

  return (int)(seconds_left);
}

/** We just got an application request for a connection with
 * port <b>port</b>. Remember it for the future, so we can keep
 * some circuits open that will exit to this port.
 */
static void
add_predicted_port(time_t now, uint16_t port)
{
  predicted_port_t *pp = tor_malloc(sizeof(predicted_port_t));

  //  If the list is empty, re-randomize predicted ports lifetime
  if (!any_predicted_circuits(now)) {
    prediction_timeout =
     (time_t)channelpadding_get_circuits_available_timeout();
  }

  last_prediction_add_time = now;

  log_info(LD_CIRC,
          "New port prediction added. Will continue predictive circ building "
          "for %d more seconds.",
          predicted_ports_prediction_time_remaining(now));

  pp->port = port;
  pp->time = now;
  predicted_ports_total_alloc += sizeof(*pp);
  smartlist_add(predicted_ports_list, pp);
}

/** Remember that <b>port</b> has been asked for as of time <b>now</b>.
 * This is used for predicting what sorts of streams we'll make in the
 * future and making exit circuits to anticipate that.
 */
void
rep_hist_note_used_port(time_t now, uint16_t port)
{
  tor_assert(predicted_ports_list);

  if (!port) /* record nothing */
    return;

  SMARTLIST_FOREACH_BEGIN(predicted_ports_list, predicted_port_t *, pp) {
    if (pp->port == port) {
      pp->time = now;

      last_prediction_add_time = now;
      log_info(LD_CIRC,
               "New port prediction added. Will continue predictive circ "
               "building for %d more seconds.",
               predicted_ports_prediction_time_remaining(now));
      return;
    }
  } SMARTLIST_FOREACH_END(pp);
  /* it's not there yet; we need to add it */
  add_predicted_port(now, port);
}

/** Return a newly allocated pointer to a list of uint16_t * for ports that
 * are likely to be asked for in the near future.
 */
smartlist_t *
rep_hist_get_predicted_ports(time_t now)
{
  int predicted_circs_relevance_time;
  smartlist_t *out = smartlist_new();
  tor_assert(predicted_ports_list);

  predicted_circs_relevance_time = (int)prediction_timeout;

  /* clean out obsolete entries */
  SMARTLIST_FOREACH_BEGIN(predicted_ports_list, predicted_port_t *, pp) {
    if (pp->time + predicted_circs_relevance_time < now) {
      log_debug(LD_CIRC, "Expiring predicted port %d", pp->port);

      predicted_ports_total_alloc -= sizeof(predicted_port_t);
      tor_free(pp);
      SMARTLIST_DEL_CURRENT(predicted_ports_list, pp);
    } else {
      smartlist_add(out, tor_memdup(&pp->port, sizeof(uint16_t)));
    }
  } SMARTLIST_FOREACH_END(pp);
  return out;
}

/**
 * Take a list of uint16_t *, and remove every port in the list from the
 * current list of predicted ports.
 */
void
rep_hist_remove_predicted_ports(const smartlist_t *rmv_ports)
{
  /* Let's do this on O(N), not O(N^2). */
  bitarray_t *remove_ports = bitarray_init_zero(UINT16_MAX);
  SMARTLIST_FOREACH(rmv_ports, const uint16_t *, p,
                    bitarray_set(remove_ports, *p));
  SMARTLIST_FOREACH_BEGIN(predicted_ports_list, predicted_port_t *, pp) {
    if (bitarray_is_set(remove_ports, pp->port)) {
      tor_free(pp);
      predicted_ports_total_alloc -= sizeof(*pp);
      SMARTLIST_DEL_CURRENT(predicted_ports_list, pp);
    }
  } SMARTLIST_FOREACH_END(pp);
  bitarray_free(remove_ports);
}

/** The user asked us to do a resolve. Rather than keeping track of
 * timings and such of resolves, we fake it for now by treating
 * it the same way as a connection to port 80. This way we will continue
 * to have circuits lying around if the user only uses Tor for resolves.
 */
void
rep_hist_note_used_resolve(time_t now)
{
  rep_hist_note_used_port(now, 80);
}

/** The last time at which we needed an internal circ. */
static time_t predicted_internal_time = 0;
/** The last time we needed an internal circ with good uptime. */
static time_t predicted_internal_uptime_time = 0;
/** The last time we needed an internal circ with good capacity. */
static time_t predicted_internal_capacity_time = 0;

/** Remember that we used an internal circ at time <b>now</b>. */
void
rep_hist_note_used_internal(time_t now, int need_uptime, int need_capacity)
{
  // If the list is empty, re-randomize predicted ports lifetime
  if (!any_predicted_circuits(now)) {
    prediction_timeout = channelpadding_get_circuits_available_timeout();
  }

  last_prediction_add_time = now;

  log_info(LD_CIRC,
          "New port prediction added. Will continue predictive circ building "
          "for %d more seconds.",
          predicted_ports_prediction_time_remaining(now));

  predicted_internal_time = now;
  if (need_uptime)
    predicted_internal_uptime_time = now;
  if (need_capacity)
    predicted_internal_capacity_time = now;
}

/** Return 1 if we've used an internal circ recently; else return 0. */
int
rep_hist_get_predicted_internal(time_t now, int *need_uptime,
                                int *need_capacity)
{
  int predicted_circs_relevance_time;

  predicted_circs_relevance_time = (int)prediction_timeout;

  if (!predicted_internal_time) { /* initialize it */
    predicted_internal_time = now;
    predicted_internal_uptime_time = now;
    predicted_internal_capacity_time = now;
  }
  if (predicted_internal_time + predicted_circs_relevance_time < now)
    return 0; /* too long ago */
  if (predicted_internal_uptime_time + predicted_circs_relevance_time >= now)
    *need_uptime = 1;
  // Always predict that we need capacity.
  *need_capacity = 1;
  return 1;
}

/** Any ports used lately? These are pre-seeded if we just started
 * up or if we're running a hidden service. */
int
any_predicted_circuits(time_t now)
{
  int predicted_circs_relevance_time;
  predicted_circs_relevance_time = (int)prediction_timeout;

  return smartlist_len(predicted_ports_list) ||
         predicted_internal_time + predicted_circs_relevance_time >= now;
}

/** Return 1 if we have no need for circuits currently, else return 0. */
int
rep_hist_circbuilding_dormant(time_t now)
{
  const or_options_t *options = get_options();

  if (any_predicted_circuits(now))
    return 0;

  /* see if we'll still need to build testing circuits */
  if (server_mode(options) &&
      (!router_all_orports_seem_reachable(options) ||
       !circuit_enough_testing_circs()))
    return 0;
  if (!router_dirport_seems_reachable(options))
    return 0;

  return 1;
}

/**
 * Allocate whatever memory and structs are needed for predicting
 * which ports will be used. Also seed it with port 80, so we'll build
 * circuits on start-up.
 */
static void
predicted_ports_alloc(void)
{
  predicted_ports_list = smartlist_new();
}

void
predicted_ports_init(void)
{
  predicted_ports_alloc();
  add_predicted_port(time(NULL), 443); // Add a port to get us started
}

/** Free whatever memory is needed for predicting which ports will
 * be used.
 */
void
predicted_ports_free_all(void)
{
  if (!predicted_ports_list)
    return;
  predicted_ports_total_alloc -=
    smartlist_len(predicted_ports_list)*sizeof(predicted_port_t);
  SMARTLIST_FOREACH(predicted_ports_list, predicted_port_t *,
                    pp, tor_free(pp));
  smartlist_free(predicted_ports_list);
}
