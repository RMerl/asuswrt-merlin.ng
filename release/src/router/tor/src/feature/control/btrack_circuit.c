/* Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file btrack_circuit.c
 * \brief Bootstrap tracker for origin circuits
 *
 * Track state changes of origin circuits, as published by the circuit
 * subsystem.
 **/

#include "core/or/or.h"

#include "core/or/ocirc_event.h"

#include "feature/control/btrack_circuit.h"
#include "feature/control/control.h"
#include "lib/log/log.h"

/** Pair of a best origin circuit GID with its state or status */
typedef struct btc_best_t {
  uint32_t gid;
  int val;
} btc_best_t;

/** GID and state of the best origin circuit we've seen so far */
static btc_best_t best_any_state = { 0, -1 };
/** GID and state of the best application circuit we've seen so far */
static btc_best_t best_ap_state = { 0, -1 };
/** GID and status of the best origin circuit we've seen so far */
static btc_best_t best_any_evtype = { 0, -1 };
/** GID and status of the best application circuit we've seen so far */
static btc_best_t best_ap_evtype = { 0, -1 };

/** Reset cached "best" values */
static void
btc_reset_bests(void)
{
  best_any_state.gid = best_ap_state.gid = 0;
  best_any_state.val = best_ap_state.val = -1;
  best_any_evtype.gid = best_ap_state.gid = 0;
  best_any_evtype.val = best_ap_evtype.val = -1;
}

/** True if @a state is a "better" origin circuit state than @a best->val */
static bool
btc_state_better(int state, const btc_best_t *best)
{
  return state > best->val;
}

/**
 * Definine an ordering on circuit status events
 *
 * The CIRC_EVENT_ constants aren't sorted in a useful order, so this
 * array helps to decode them.  This approach depends on the statuses
 * being nonnegative and dense.
 **/
static int circ_event_order[] = {
  [CIRC_EVENT_FAILED] = -1,
  [CIRC_EVENT_CLOSED] = -1,
  [CIRC_EVENT_LAUNCHED] = 1,
  [CIRC_EVENT_EXTENDED] = 2,
  [CIRC_EVENT_BUILT] = 3,
};
#define N_CIRC_EVENT_ORDER \
  (sizeof(circ_event_order) / sizeof(circ_event_order[0]))

/** True if @a state is a "better" origin circuit event status than @a
    best->val */
static bool
btc_evtype_better(int state, const btc_best_t *best)
{
  if (state < 0)
    return false;
  if (best->val < 0)
    return true;

  tor_assert(state >= 0 && (unsigned)state < N_CIRC_EVENT_ORDER);
  tor_assert(best->val >= 0 && (unsigned)best->val < N_CIRC_EVENT_ORDER);
  return circ_event_order[state] > circ_event_order[best->val];
}

static bool
btc_update_state(const ocirc_state_msg_t *msg, btc_best_t *best,
                 const char *type)
{
  if (btc_state_better(msg->state, best)) {
    log_info(LD_BTRACK, "CIRC BEST_%s state %d->%d gid=%"PRIu32, type,
             best->val, msg->state, msg->gid);
    best->gid = msg->gid;
    best->val = msg->state;
    return true;
  }
  return false;
}

static bool
btc_update_evtype(const ocirc_cevent_msg_t *msg, btc_best_t *best,
                  const char *type)
{
  if (btc_evtype_better(msg->evtype, best)) {
    log_info(LD_BTRACK, "CIRC BEST_%s evtype %d->%d gid=%"PRIu32, type,
             best->val, msg->evtype, msg->gid);
    best->gid = msg->gid;
    best->val = msg->evtype;
    return true;
  }
  return false;
}

DECLARE_SUBSCRIBE(ocirc_state, btc_state_rcvr);
DECLARE_SUBSCRIBE(ocirc_cevent, btc_cevent_rcvr);
DECLARE_SUBSCRIBE(ocirc_chan, btc_chan_rcvr);

static void
btc_state_rcvr(const msg_t *msg, const ocirc_state_msg_t *arg)
{
  (void)msg;
  log_debug(LD_BTRACK, "CIRC gid=%"PRIu32" state=%d onehop=%d",
            arg->gid, arg->state, arg->onehop);

  btc_update_state(arg, &best_any_state, "ANY");
  if (arg->onehop)
    return;
  btc_update_state(arg, &best_ap_state, "AP");
}

static void
btc_cevent_rcvr(const msg_t *msg, const ocirc_cevent_msg_t *arg)
{
  (void)msg;
  log_debug(LD_BTRACK, "CIRC gid=%"PRIu32" evtype=%d reason=%d onehop=%d",
            arg->gid, arg->evtype, arg->reason, arg->onehop);

  btc_update_evtype(arg, &best_any_evtype, "ANY");
  if (arg->onehop)
    return;
  btc_update_evtype(arg, &best_ap_evtype, "AP");
}

static void
btc_chan_rcvr(const msg_t *msg, const ocirc_chan_msg_t *arg)
{
  (void)msg;
  log_debug(LD_BTRACK, "CIRC gid=%"PRIu32" chan=%"PRIu64" onehop=%d",
            arg->gid, arg->chan, arg->onehop);
}

int
btrack_circ_add_pubsub(pubsub_connector_t *connector)
{
  if (DISPATCH_ADD_SUB(connector, ocirc, ocirc_chan))
    return -1;
  if (DISPATCH_ADD_SUB(connector, ocirc, ocirc_cevent))
    return -1;
  if (DISPATCH_ADD_SUB(connector, ocirc, ocirc_state))
    return -1;
  return 0;
}

void
btrack_circ_fini(void)
{
  btc_reset_bests();
}
