/* Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file btrack_orconn.c
 * \brief Bootstrap tracker for OR connections
 *
 * Track state changes of OR connections, as published by the
 * connection subsystem.  Also track circuit launch events, because
 * they're one of the few ways to discover the association between a
 * channel (and OR connection) and a circuit.
 *
 * We track all OR connections that we receive events for, whether or
 * not they're carrying origin circuits.  (An OR connection might
 * carry origin circuits only after we first find out about that
 * connection.)
 *
 * All origin ORCONN events update the "any" state variables, while
 * only application ORCONN events update the "ap" state variables (and
 * also update the "any") variables.
 *
 * We do this because we want to report the first increments of
 * connection progress as the earliest bootstrap phases.  This results
 * in a better user experience because failures here translate into
 * zero or very small amounts of displayed progress, instead of
 * progress stuck near completion.  The first connection to a relay
 * might be a one-hop circuit for directory lookups, or it might be a
 * connection for an application circuit because we already have
 * enough directory info to build an application circuit.
 *
 * We call functions in btrack_orconn_cevent.c to generate the actual
 * controller events, because some of the state decoding we need to do
 * is complicated.
 **/

#include <stdbool.h>

#include "core/or/or.h"

#define BTRACK_ORCONN_PRIVATE

#include "core/or/ocirc_event.h"
#include "core/or/orconn_event.h"
#include "feature/control/btrack_orconn.h"
#include "feature/control/btrack_orconn_cevent.h"
#include "feature/control/btrack_orconn_maps.h"
#include "lib/log/log.h"
#include "lib/pubsub/pubsub.h"

DECLARE_SUBSCRIBE(orconn_state, bto_state_rcvr);
DECLARE_SUBSCRIBE(orconn_status, bto_status_rcvr);
DECLARE_SUBSCRIBE(ocirc_chan, bto_chan_rcvr);

/** Pair of a best ORCONN GID and with its state */
typedef struct bto_best_t {
  uint64_t gid;
  int state;
} bto_best_t;

/** GID and state of the best ORCONN we've seen so far */
static bto_best_t best_any = { 0, -1 };
/** GID and state of the best application circuit ORCONN we've seen so far */
static bto_best_t best_ap = { 0, -1 };

/**
 * Update a cached state of a best ORCONN progress we've seen so far.
 *
 * Return true if the new state is better than the old.
 **/
static bool
bto_update_best(const bt_orconn_t *bto, bto_best_t *best, const char *type)
{
  if (bto->state < best->state)
    return false;
  /* Update even if we won't change best->state, because it's more
   * recent information that a particular connection transitioned to
   * that state. */
  best->gid = bto->gid;
  if (bto->state > best->state) {
    log_info(LD_BTRACK, "ORCONN BEST_%s state %d->%d gid=%"PRIu64, type,
             best->state, bto->state, bto->gid);
    best->state = bto->state;
    return true;
  }
  return false;
}

/**
 * Update cached states of best ORCONN progress we've seen
 *
 * Only update the application ORCONN state if we know it's carrying
 * an application circuit.
 **/
static void
bto_update_bests(const bt_orconn_t *bto)
{
  tor_assert(bto->is_orig);

  if (bto_update_best(bto, &best_any, "ANY"))
    bto_cevent_anyconn(bto);
  if (!bto->is_onehop && bto_update_best(bto, &best_ap, "AP"))
    bto_cevent_apconn(bto);
}

/** Reset cached "best" values */
static void
bto_reset_bests(void)
{
  best_any.gid = best_ap.gid = 0;
  best_any.state = best_ap.state = -1;
}

/**
 * Update cached states of ORCONNs from the incoming message.  This
 * message comes from code in connection_or.c.
 **/
static void
bto_state_rcvr(const msg_t *msg, const orconn_state_msg_t *arg)
{
  bt_orconn_t *bto;

  (void)msg;
  bto = bto_find_or_new(arg->gid, arg->chan);
  log_debug(LD_BTRACK, "ORCONN gid=%"PRIu64" chan=%"PRIu64
            " proxy_type=%d state=%d",
            arg->gid, arg->chan, arg->proxy_type, arg->state);
  bto->proxy_type = arg->proxy_type;
  bto->state = arg->state;
  if (bto->is_orig)
    bto_update_bests(bto);
}

/**
 * Delete a cached ORCONN state if we get an incoming message saying
 * the ORCONN is failed or closed.  This message comes from code in
 * control.c.
 **/
static void
bto_status_rcvr(const msg_t *msg, const orconn_status_msg_t *arg)
{
  (void)msg;
  switch (arg->status) {
  case OR_CONN_EVENT_FAILED:
  case OR_CONN_EVENT_CLOSED:
    log_info(LD_BTRACK, "ORCONN DELETE gid=%"PRIu64" status=%d reason=%d",
             arg->gid, arg->status, arg->reason);
    return bto_delete(arg->gid);
  default:
    break;
  }
}

/**
 * Create or update a cached ORCONN state for a newly launched
 * connection, including whether it's launched by an origin circuit
 * and whether it's a one-hop circuit.
 **/
static void
bto_chan_rcvr(const msg_t *msg, const ocirc_chan_msg_t *arg)
{
  bt_orconn_t *bto;

  (void)msg;
  bto = bto_find_or_new(0, arg->chan);
  if (!bto->is_orig || (bto->is_onehop && !arg->onehop)) {
    log_debug(LD_BTRACK, "ORCONN LAUNCH chan=%"PRIu64" onehop=%d",
              arg->chan, arg->onehop);
  }
  bto->is_orig = true;
  if (!arg->onehop)
    bto->is_onehop = false;
  bto_update_bests(bto);
}

/**
 * Initialize the hash maps and subscribe to ORCONN and origin
 * circuit events.
 **/
int
btrack_orconn_init(void)
{
  bto_init_maps();

  return 0;
}

int
btrack_orconn_add_pubsub(pubsub_connector_t *connector)
{
  if (DISPATCH_ADD_SUB(connector, orconn, orconn_state))
    return -1;
  if (DISPATCH_ADD_SUB(connector, orconn, orconn_status))
    return -1;
  if (DISPATCH_ADD_SUB(connector, ocirc, ocirc_chan))
    return -1;
  return 0;
}

/** Clear the hash maps and reset the "best" states */
void
btrack_orconn_fini(void)
{
  bto_clear_maps();
  bto_reset_bests();
  bto_cevent_reset();
}
