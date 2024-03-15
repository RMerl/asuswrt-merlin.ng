/* * Copyright (c) 2012-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file channel.c
 *
 * \brief OR/OP-to-OR channel abstraction layer. A channel's job is to
 * transfer cells from Tor instance to Tor instance. Currently, there is only
 * one implementation of the channel abstraction: in channeltls.c.
 *
 * Channels are a higher-level abstraction than or_connection_t: In general,
 * any means that two Tor relays use to exchange cells, or any means that a
 * relay and a client use to exchange cells, is a channel.
 *
 * Channels differ from pluggable transports in that they do not wrap an
 * underlying protocol over which cells are transmitted: they <em>are</em> the
 * underlying protocol.
 *
 * This module defines the generic parts of the channel_t interface, and
 * provides the machinery necessary for specialized implementations to be
 * created.  At present, there is one specialized implementation in
 * channeltls.c, which uses connection_or.c to send cells over a TLS
 * connection.
 *
 * Every channel implementation is responsible for being able to transmit
 * cells that are passed to it
 *
 * For *inbound* cells, the entry point is: channel_process_cell(). It takes a
 * cell and will pass it to the cell handler set by
 * channel_set_cell_handlers(). Currently, this is passed back to the command
 * subsystem which is command_process_cell().
 *
 * NOTE: For now, the separation between channels and specialized channels
 * (like channeltls) is not that well defined. So the channeltls layer calls
 * channel_process_cell() which originally comes from the connection subsystem.
 * This should be hopefully be fixed with #23993.
 *
 * For *outbound* cells, the entry point is: channel_write_packed_cell().
 * Only packed cells are dequeued from the circuit queue by the scheduler
 * which uses channel_flush_from_first_active_circuit() to decide which cells
 * to flush from which circuit on the channel. They are then passed down to
 * the channel subsystem. This calls the low layer with the function pointer
 * .write_packed_cell().
 *
 * Each specialized channel (currently only channeltls_t) MUST implement a
 * series of function found in channel_t. See channel.h for more
 * documentation.
 **/

/*
 * Define this so channel.h gives us things only channel_t subclasses
 * should touch.
 */
#define CHANNEL_OBJECT_PRIVATE

/* This one's for stuff only channel.c and the test suite should see */
#define CHANNEL_FILE_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/mainloop.h"
#include "core/or/channel.h"
#include "core/or/channelpadding.h"
#include "core/or/channeltls.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitmux.h"
#include "core/or/circuitstats.h"
#include "core/or/connection_or.h" /* For var_cell_free() */
#include "core/or/dos.h"
#include "core/or/relay.h"
#include "core/or/scheduler.h"
#include "feature/client/entrynodes.h"
#include "feature/hs/hs_service.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/router.h"
#include "feature/stats/geoip_stats.h"
#include "feature/stats/rephist.h"
#include "lib/evloop/timers.h"
#include "lib/time/compat_time.h"

#include "core/or/cell_queue_st.h"
#include "core/or/or_connection_st.h"

/* Global lists of channels */

/* All channel_t instances */
static smartlist_t *all_channels = NULL;

/* All channel_t instances not in ERROR or CLOSED states */
static smartlist_t *active_channels = NULL;

/* All channel_t instances in ERROR or CLOSED states */
static smartlist_t *finished_channels = NULL;

/* All channel_listener_t instances */
static smartlist_t *all_listeners = NULL;

/* All channel_listener_t instances in LISTENING state */
static smartlist_t *active_listeners = NULL;

/* All channel_listener_t instances in LISTENING state */
static smartlist_t *finished_listeners = NULL;

/** Map from channel->global_identifier to channel.  Contains the same
 * elements as all_channels. */
static HT_HEAD(channel_gid_map, channel_t) channel_gid_map = HT_INITIALIZER();

static unsigned
channel_id_hash(const channel_t *chan)
{
  return (unsigned) chan->global_identifier;
}
static int
channel_id_eq(const channel_t *a, const channel_t *b)
{
  return a->global_identifier == b->global_identifier;
}
HT_PROTOTYPE(channel_gid_map, channel_t, gidmap_node,
             channel_id_hash, channel_id_eq);
HT_GENERATE2(channel_gid_map, channel_t, gidmap_node,
             channel_id_hash, channel_id_eq,
             0.6, tor_reallocarray_, tor_free_);

HANDLE_IMPL(channel, channel_t,)

/* Counter for ID numbers */
static uint64_t n_channels_allocated = 0;

/* Digest->channel map
 *
 * Similar to the one used in connection_or.c, this maps from the identity
 * digest of a remote endpoint to a channel_t to that endpoint.  Channels
 * should be placed here when registered and removed when they close or error.
 * If more than one channel exists, follow the next_with_same_id pointer
 * as a linked list.
 */
static HT_HEAD(channel_idmap, channel_idmap_entry_t) channel_identity_map =
  HT_INITIALIZER();

typedef struct channel_idmap_entry_t {
  HT_ENTRY(channel_idmap_entry_t) node;
  uint8_t digest[DIGEST_LEN];
  TOR_LIST_HEAD(channel_list_t, channel_t) channel_list;
} channel_idmap_entry_t;

static inline unsigned
channel_idmap_hash(const channel_idmap_entry_t *ent)
{
  return (unsigned) siphash24g(ent->digest, DIGEST_LEN);
}

static inline int
channel_idmap_eq(const channel_idmap_entry_t *a,
                  const channel_idmap_entry_t *b)
{
  return tor_memeq(a->digest, b->digest, DIGEST_LEN);
}

HT_PROTOTYPE(channel_idmap, channel_idmap_entry_t, node, channel_idmap_hash,
             channel_idmap_eq);
HT_GENERATE2(channel_idmap, channel_idmap_entry_t, node, channel_idmap_hash,
             channel_idmap_eq, 0.5,  tor_reallocarray_, tor_free_);

/* Functions to maintain the digest map */
static void channel_remove_from_digest_map(channel_t *chan);

static void channel_force_xfree(channel_t *chan);
static void channel_free_list(smartlist_t *channels,
                               int mark_for_close);
static void channel_listener_free_list(smartlist_t *channels,
                                        int mark_for_close);
static void channel_listener_force_xfree(channel_listener_t *chan_l);

/***********************************
 * Channel state utility functions *
 **********************************/

/**
 * Indicate whether a given channel state is valid.
 */
int
channel_state_is_valid(channel_state_t state)
{
  int is_valid;

  switch (state) {
    case CHANNEL_STATE_CLOSED:
    case CHANNEL_STATE_CLOSING:
    case CHANNEL_STATE_ERROR:
    case CHANNEL_STATE_MAINT:
    case CHANNEL_STATE_OPENING:
    case CHANNEL_STATE_OPEN:
      is_valid = 1;
      break;
    case CHANNEL_STATE_LAST:
    default:
      is_valid = 0;
  }

  return is_valid;
}

/**
 * Indicate whether a given channel listener state is valid.
 */
int
channel_listener_state_is_valid(channel_listener_state_t state)
{
  int is_valid;

  switch (state) {
    case CHANNEL_LISTENER_STATE_CLOSED:
    case CHANNEL_LISTENER_STATE_LISTENING:
    case CHANNEL_LISTENER_STATE_CLOSING:
    case CHANNEL_LISTENER_STATE_ERROR:
      is_valid = 1;
      break;
    case CHANNEL_LISTENER_STATE_LAST:
    default:
      is_valid = 0;
  }

  return is_valid;
}

/**
 * Indicate whether a channel state transition is valid.
 *
 * This function takes two channel states and indicates whether a
 * transition between them is permitted (see the state definitions and
 * transition table in or.h at the channel_state_t typedef).
 */
int
channel_state_can_transition(channel_state_t from, channel_state_t to)
{
  int is_valid;

  switch (from) {
    case CHANNEL_STATE_CLOSED:
      is_valid = (to == CHANNEL_STATE_OPENING);
      break;
    case CHANNEL_STATE_CLOSING:
      is_valid = (to == CHANNEL_STATE_CLOSED ||
                  to == CHANNEL_STATE_ERROR);
      break;
    case CHANNEL_STATE_ERROR:
      is_valid = 0;
      break;
    case CHANNEL_STATE_MAINT:
      is_valid = (to == CHANNEL_STATE_CLOSING ||
                  to == CHANNEL_STATE_ERROR ||
                  to == CHANNEL_STATE_OPEN);
      break;
    case CHANNEL_STATE_OPENING:
      is_valid = (to == CHANNEL_STATE_CLOSING ||
                  to == CHANNEL_STATE_ERROR ||
                  to == CHANNEL_STATE_OPEN);
      break;
    case CHANNEL_STATE_OPEN:
      is_valid = (to == CHANNEL_STATE_CLOSING ||
                  to == CHANNEL_STATE_ERROR ||
                  to == CHANNEL_STATE_MAINT);
      break;
    case CHANNEL_STATE_LAST:
    default:
      is_valid = 0;
  }

  return is_valid;
}

/**
 * Indicate whether a channel listener state transition is valid.
 *
 * This function takes two channel listener states and indicates whether a
 * transition between them is permitted (see the state definitions and
 * transition table in or.h at the channel_listener_state_t typedef).
 */
int
channel_listener_state_can_transition(channel_listener_state_t from,
                                      channel_listener_state_t to)
{
  int is_valid;

  switch (from) {
    case CHANNEL_LISTENER_STATE_CLOSED:
      is_valid = (to == CHANNEL_LISTENER_STATE_LISTENING);
      break;
    case CHANNEL_LISTENER_STATE_CLOSING:
      is_valid = (to == CHANNEL_LISTENER_STATE_CLOSED ||
                  to == CHANNEL_LISTENER_STATE_ERROR);
      break;
    case CHANNEL_LISTENER_STATE_ERROR:
      is_valid = 0;
      break;
    case CHANNEL_LISTENER_STATE_LISTENING:
      is_valid = (to == CHANNEL_LISTENER_STATE_CLOSING ||
                  to == CHANNEL_LISTENER_STATE_ERROR);
      break;
    case CHANNEL_LISTENER_STATE_LAST:
    default:
      is_valid = 0;
  }

  return is_valid;
}

/**
 * Return a human-readable description for a channel state.
 */
const char *
channel_state_to_string(channel_state_t state)
{
  const char *descr;

  switch (state) {
    case CHANNEL_STATE_CLOSED:
      descr = "closed";
      break;
    case CHANNEL_STATE_CLOSING:
      descr = "closing";
      break;
    case CHANNEL_STATE_ERROR:
      descr = "channel error";
      break;
    case CHANNEL_STATE_MAINT:
      descr = "temporarily suspended for maintenance";
      break;
    case CHANNEL_STATE_OPENING:
      descr = "opening";
      break;
    case CHANNEL_STATE_OPEN:
      descr = "open";
      break;
    case CHANNEL_STATE_LAST:
    default:
      descr = "unknown or invalid channel state";
  }

  return descr;
}

/**
 * Return a human-readable description for a channel listener state.
 */
const char *
channel_listener_state_to_string(channel_listener_state_t state)
{
  const char *descr;

  switch (state) {
    case CHANNEL_LISTENER_STATE_CLOSED:
      descr = "closed";
      break;
    case CHANNEL_LISTENER_STATE_CLOSING:
      descr = "closing";
      break;
    case CHANNEL_LISTENER_STATE_ERROR:
      descr = "channel listener error";
      break;
    case CHANNEL_LISTENER_STATE_LISTENING:
      descr = "listening";
      break;
    case CHANNEL_LISTENER_STATE_LAST:
    default:
      descr = "unknown or invalid channel listener state";
  }

  return descr;
}

/***************************************
 * Channel registration/unregistration *
 ***************************************/

/**
 * Register a channel.
 *
 * This function registers a newly created channel in the global lists/maps
 * of active channels.
 */
void
channel_register(channel_t *chan)
{
  tor_assert(chan);
  tor_assert(chan->global_identifier);

  /* No-op if already registered */
  if (chan->registered) return;

  log_debug(LD_CHANNEL,
            "Registering channel %p (ID %"PRIu64 ") "
            "in state %s (%d) with digest %s",
            chan, (chan->global_identifier),
            channel_state_to_string(chan->state), chan->state,
            hex_str(chan->identity_digest, DIGEST_LEN));

  /* Make sure we have all_channels, then add it */
  if (!all_channels) all_channels = smartlist_new();
  smartlist_add(all_channels, chan);
  channel_t *oldval = HT_REPLACE(channel_gid_map, &channel_gid_map, chan);
  tor_assert(! oldval);

  /* Is it finished? */
  if (CHANNEL_FINISHED(chan)) {
    /* Put it in the finished list, creating it if necessary */
    if (!finished_channels) finished_channels = smartlist_new();
    smartlist_add(finished_channels, chan);
    mainloop_schedule_postloop_cleanup();
  } else {
    /* Put it in the active list, creating it if necessary */
    if (!active_channels) active_channels = smartlist_new();
    smartlist_add(active_channels, chan);

    if (!CHANNEL_IS_CLOSING(chan)) {
      /* It should have a digest set */
      if (!tor_digest_is_zero(chan->identity_digest)) {
        /* Yeah, we're good, add it to the map */
        channel_add_to_digest_map(chan);
      } else {
        log_info(LD_CHANNEL,
                "Channel %p (global ID %"PRIu64 ") "
                "in state %s (%d) registered with no identity digest",
                chan, (chan->global_identifier),
                channel_state_to_string(chan->state), chan->state);
      }
    }
  }

  /* Mark it as registered */
  chan->registered = 1;
}

/**
 * Unregister a channel.
 *
 * This function removes a channel from the global lists and maps and is used
 * when freeing a closed/errored channel.
 */
void
channel_unregister(channel_t *chan)
{
  tor_assert(chan);

  /* No-op if not registered */
  if (!(chan->registered)) return;

  /* Is it finished? */
  if (CHANNEL_FINISHED(chan)) {
    /* Get it out of the finished list */
    if (finished_channels) smartlist_remove(finished_channels, chan);
  } else {
    /* Get it out of the active list */
    if (active_channels) smartlist_remove(active_channels, chan);
  }

  /* Get it out of all_channels */
  if (all_channels) smartlist_remove(all_channels, chan);
  channel_t *oldval = HT_REMOVE(channel_gid_map, &channel_gid_map, chan);
  tor_assert(oldval == NULL || oldval == chan);

  /* Mark it as unregistered */
  chan->registered = 0;

  /* Should it be in the digest map? */
  if (!tor_digest_is_zero(chan->identity_digest) &&
      !(CHANNEL_CONDEMNED(chan))) {
    /* Remove it */
    channel_remove_from_digest_map(chan);
  }
}

/**
 * Register a channel listener.
 *
 * This function registers a newly created channel listener in the global
 * lists/maps of active channel listeners.
 */
void
channel_listener_register(channel_listener_t *chan_l)
{
  tor_assert(chan_l);

  /* No-op if already registered */
  if (chan_l->registered) return;

  log_debug(LD_CHANNEL,
            "Registering channel listener %p (ID %"PRIu64 ") "
            "in state %s (%d)",
            chan_l, (chan_l->global_identifier),
            channel_listener_state_to_string(chan_l->state),
            chan_l->state);

  /* Make sure we have all_listeners, then add it */
  if (!all_listeners) all_listeners = smartlist_new();
  smartlist_add(all_listeners, chan_l);

  /* Is it finished? */
  if (chan_l->state == CHANNEL_LISTENER_STATE_CLOSED ||
      chan_l->state == CHANNEL_LISTENER_STATE_ERROR) {
    /* Put it in the finished list, creating it if necessary */
    if (!finished_listeners) finished_listeners = smartlist_new();
    smartlist_add(finished_listeners, chan_l);
  } else {
    /* Put it in the active list, creating it if necessary */
    if (!active_listeners) active_listeners = smartlist_new();
    smartlist_add(active_listeners, chan_l);
  }

  /* Mark it as registered */
  chan_l->registered = 1;
}

/**
 * Unregister a channel listener.
 *
 * This function removes a channel listener from the global lists and maps
 * and is used when freeing a closed/errored channel listener.
 */
void
channel_listener_unregister(channel_listener_t *chan_l)
{
  tor_assert(chan_l);

  /* No-op if not registered */
  if (!(chan_l->registered)) return;

  /* Is it finished? */
  if (chan_l->state == CHANNEL_LISTENER_STATE_CLOSED ||
      chan_l->state == CHANNEL_LISTENER_STATE_ERROR) {
    /* Get it out of the finished list */
    if (finished_listeners) smartlist_remove(finished_listeners, chan_l);
  } else {
    /* Get it out of the active list */
    if (active_listeners) smartlist_remove(active_listeners, chan_l);
  }

  /* Get it out of all_listeners */
 if (all_listeners) smartlist_remove(all_listeners, chan_l);

  /* Mark it as unregistered */
  chan_l->registered = 0;
}

/*********************************
 * Channel digest map maintenance
 *********************************/

/**
 * Add a channel to the digest map.
 *
 * This function adds a channel to the digest map and inserts it into the
 * correct linked list if channels with that remote endpoint identity digest
 * already exist.
 */
STATIC void
channel_add_to_digest_map(channel_t *chan)
{
  channel_idmap_entry_t *ent, search;

  tor_assert(chan);

  /* Assert that the state makes sense */
  tor_assert(!CHANNEL_CONDEMNED(chan));

  /* Assert that there is a digest */
  tor_assert(!tor_digest_is_zero(chan->identity_digest));

  memcpy(search.digest, chan->identity_digest, DIGEST_LEN);
  ent = HT_FIND(channel_idmap, &channel_identity_map, &search);
  if (! ent) {
    ent = tor_malloc(sizeof(channel_idmap_entry_t));
    memcpy(ent->digest, chan->identity_digest, DIGEST_LEN);
    TOR_LIST_INIT(&ent->channel_list);
    HT_INSERT(channel_idmap, &channel_identity_map, ent);
  }
  TOR_LIST_INSERT_HEAD(&ent->channel_list, chan, next_with_same_id);

  log_debug(LD_CHANNEL,
            "Added channel %p (global ID %"PRIu64 ") "
            "to identity map in state %s (%d) with digest %s",
            chan, (chan->global_identifier),
            channel_state_to_string(chan->state), chan->state,
            hex_str(chan->identity_digest, DIGEST_LEN));
}

/**
 * Remove a channel from the digest map.
 *
 * This function removes a channel from the digest map and the linked list of
 * channels for that digest if more than one exists.
 */
static void
channel_remove_from_digest_map(channel_t *chan)
{
  channel_idmap_entry_t *ent, search;

  tor_assert(chan);

  /* Assert that there is a digest */
  tor_assert(!tor_digest_is_zero(chan->identity_digest));

  /* Pull it out of its list, wherever that list is */
  TOR_LIST_REMOVE(chan, next_with_same_id);

  memcpy(search.digest, chan->identity_digest, DIGEST_LEN);
  ent = HT_FIND(channel_idmap, &channel_identity_map, &search);

  /* Look for it in the map */
  if (ent) {
    /* Okay, it's here */

    if (TOR_LIST_EMPTY(&ent->channel_list)) {
      HT_REMOVE(channel_idmap, &channel_identity_map, ent);
      tor_free(ent);
    }

    log_debug(LD_CHANNEL,
              "Removed channel %p (global ID %"PRIu64 ") from "
              "identity map in state %s (%d) with digest %s",
              chan, (chan->global_identifier),
              channel_state_to_string(chan->state), chan->state,
              hex_str(chan->identity_digest, DIGEST_LEN));
  } else {
    /* Shouldn't happen */
    log_warn(LD_BUG,
             "Trying to remove channel %p (global ID %"PRIu64 ") with "
             "digest %s from identity map, but couldn't find any with "
             "that digest",
             chan, (chan->global_identifier),
             hex_str(chan->identity_digest, DIGEST_LEN));
  }
}

/****************************
 * Channel lookup functions *
 ***************************/

/**
 * Find channel by global ID.
 *
 * This function searches for a channel by the global_identifier assigned
 * at initialization time.  This identifier is unique for the lifetime of the
 * Tor process.
 */
channel_t *
channel_find_by_global_id(uint64_t global_identifier)
{
  channel_t lookup;
  channel_t *rv = NULL;

  lookup.global_identifier = global_identifier;
  rv = HT_FIND(channel_gid_map, &channel_gid_map, &lookup);
  if (rv) {
    tor_assert(rv->global_identifier == global_identifier);
  }

  return rv;
}

/** Return true iff <b>chan</b> matches <b>rsa_id_digest</b> and <b>ed_id</b>.
 * as its identity keys.  If either is NULL, do not check for a match. */
int
channel_remote_identity_matches(const channel_t *chan,
                                const char *rsa_id_digest,
                                const ed25519_public_key_t *ed_id)
{
  if (BUG(!chan))
    return 0;
  if (rsa_id_digest) {
    if (tor_memneq(rsa_id_digest, chan->identity_digest, DIGEST_LEN))
      return 0;
  }
  if (ed_id) {
    if (tor_memneq(ed_id->pubkey, chan->ed25519_identity.pubkey,
                   ED25519_PUBKEY_LEN))
      return 0;
  }
  return 1;
}

/**
 * Find channel by RSA/Ed25519 identity of of the remote endpoint.
 *
 * This function looks up a channel by the digest of its remote endpoint's RSA
 * identity key.  If <b>ed_id</b> is provided and nonzero, only a channel
 * matching the <b>ed_id</b> will be returned.
 *
 * It's possible that more than one channel to a given endpoint exists.  Use
 * channel_next_with_rsa_identity() to walk the list of channels; make sure
 * to test for Ed25519 identity match too (as appropriate)
 */
channel_t *
channel_find_by_remote_identity(const char *rsa_id_digest,
                                const ed25519_public_key_t *ed_id)
{
  channel_t *rv = NULL;
  channel_idmap_entry_t *ent, search;

  tor_assert(rsa_id_digest); /* For now, we require that every channel have
                              * an RSA identity, and that every lookup
                              * contain an RSA identity */
  if (ed_id && ed25519_public_key_is_zero(ed_id)) {
    /* Treat zero as meaning "We don't care about the presence or absence of
     * an Ed key", not "There must be no Ed key". */
    ed_id = NULL;
  }

  memcpy(search.digest, rsa_id_digest, DIGEST_LEN);
  ent = HT_FIND(channel_idmap, &channel_identity_map, &search);
  if (ent) {
    rv = TOR_LIST_FIRST(&ent->channel_list);
  }
  while (rv && ! channel_remote_identity_matches(rv, rsa_id_digest, ed_id)) {
    rv = channel_next_with_rsa_identity(rv);
  }

  return rv;
}

/**
 * Get next channel with digest.
 *
 * This function takes a channel and finds the next channel in the list
 * with the same digest.
 */
channel_t *
channel_next_with_rsa_identity(channel_t *chan)
{
  tor_assert(chan);

  return TOR_LIST_NEXT(chan, next_with_same_id);
}

/**
 * Relays run this once an hour to look over our list of channels to other
 * relays. It prints out some statistics if there are multiple connections
 * to many relays.
 *
 * This function is similar to connection_or_set_bad_connections(),
 * and probably could be adapted to replace it, if it was modified to actually
 * take action on any of these connections.
 */
void
channel_check_for_duplicates(void)
{
  channel_idmap_entry_t **iter;
  channel_t *chan;
  int total_dirauth_connections = 0, total_dirauths = 0;
  int total_relay_connections = 0, total_relays = 0, total_canonical = 0;
  int total_half_canonical = 0;
  int total_gt_one_connection = 0, total_gt_two_connections = 0;
  int total_gt_four_connections = 0;

  HT_FOREACH(iter, channel_idmap, &channel_identity_map) {
    int connections_to_relay = 0;
    const char *id_digest = (char *) (*iter)->digest;

    /* Only consider relay connections */
    if (!connection_or_digest_is_known_relay(id_digest))
      continue;

    total_relays++;

    const bool is_dirauth = router_digest_is_trusted_dir(id_digest);
    if (is_dirauth)
      total_dirauths++;

    for (chan = TOR_LIST_FIRST(&(*iter)->channel_list); chan;
        chan = channel_next_with_rsa_identity(chan)) {

      if (CHANNEL_CONDEMNED(chan) || !CHANNEL_IS_OPEN(chan))
        continue;

      connections_to_relay++;
      total_relay_connections++;
      if (is_dirauth)
        total_dirauth_connections++;

      if (chan->is_canonical(chan)) total_canonical++;

      if (!chan->is_canonical_to_peer && chan->is_canonical(chan)) {
        total_half_canonical++;
      }
    }

    if (connections_to_relay > 1) total_gt_one_connection++;
    if (connections_to_relay > 2) total_gt_two_connections++;
    if (connections_to_relay > 4) total_gt_four_connections++;
  }

  /* Don't bother warning about excessive connections unless we have
   * at least this many connections, total.
   */
#define MIN_RELAY_CONNECTIONS_TO_WARN 25
  /* If the average number of connections for a regular relay is more than
   * this, that's too high.
   */
#define MAX_AVG_RELAY_CONNECTIONS 1.5
  /* If the average number of connections for a dirauth is more than
   * this, that's too high.
   */
#define MAX_AVG_DIRAUTH_CONNECTIONS 4

  /* How many connections total would be okay, given the number of
   * relays and dirauths that we have connections to? */
  const int max_tolerable_connections = (int)(
    (total_relays-total_dirauths) * MAX_AVG_RELAY_CONNECTIONS +
    total_dirauths * MAX_AVG_DIRAUTH_CONNECTIONS);

  /* If we average 1.5 or more connections per relay, something is wrong */
  if (total_relays > MIN_RELAY_CONNECTIONS_TO_WARN &&
      total_relay_connections > max_tolerable_connections) {
    log_notice(LD_OR,
        "Your relay has a very large number of connections to other relays. "
        "Is your outbound address the same as your relay address? "
        "Found %d connections to authorities, %d connections to %d relays. "
        "Found %d current canonical connections, "
        "in %d of which we were a non-canonical peer. "
        "%d relays had more than 1 connection, %d had more than 2, and "
        "%d had more than 4 connections.",
        total_dirauth_connections, total_relay_connections,
        total_relays, total_canonical, total_half_canonical,
        total_gt_one_connection, total_gt_two_connections,
        total_gt_four_connections);
  } else {
    log_info(LD_OR, "Performed connection pruning. "
        "Found %d connections to authorities, %d connections to %d relays. "
        "Found %d current canonical connections, "
        "in %d of which we were a non-canonical peer. "
        "%d relays had more than 1 connection, %d had more than 2, and "
        "%d had more than 4 connections.",
        total_dirauth_connections, total_relay_connections,
        total_relays, total_canonical, total_half_canonical,
        total_gt_one_connection, total_gt_two_connections,
        total_gt_four_connections);
  }
}

/**
 * Initialize a channel.
 *
 * This function should be called by subclasses to set up some per-channel
 * variables.  I.e., this is the superclass constructor.  Before this, the
 * channel should be allocated with tor_malloc_zero().
 */
void
channel_init(channel_t *chan)
{
  tor_assert(chan);

  /* Assign an ID and bump the counter */
  chan->global_identifier = ++n_channels_allocated;

  /* Init timestamp */
  chan->timestamp_last_had_circuits = time(NULL);

  /* Warn about exhausted circuit IDs no more than hourly. */
  chan->last_warned_circ_ids_exhausted.rate = 3600;

  /* Initialize list entries. */
  memset(&chan->next_with_same_id, 0, sizeof(chan->next_with_same_id));

  /* Timestamp it */
  channel_timestamp_created(chan);

  /* It hasn't been open yet. */
  chan->has_been_open = 0;

  /* Scheduler state is idle */
  chan->scheduler_state = SCHED_CHAN_IDLE;

  /* Channel is not in the scheduler heap. */
  chan->sched_heap_idx = -1;

  tor_addr_make_unspec(&chan->addr_according_to_peer);
}

/**
 * Initialize a channel listener.
 *
 * This function should be called by subclasses to set up some per-channel
 * variables.  I.e., this is the superclass constructor.  Before this, the
 * channel listener should be allocated with tor_malloc_zero().
 */
void
channel_init_listener(channel_listener_t *chan_l)
{
  tor_assert(chan_l);

  /* Assign an ID and bump the counter */
  chan_l->global_identifier = ++n_channels_allocated;

  /* Timestamp it */
  channel_listener_timestamp_created(chan_l);
}

/**
 * Free a channel; nothing outside of channel.c and subclasses should call
 * this - it frees channels after they have closed and been unregistered.
 */
void
channel_free_(channel_t *chan)
{
  if (!chan) return;

  /* It must be closed or errored */
  tor_assert(CHANNEL_FINISHED(chan));

  /* It must be deregistered */
  tor_assert(!(chan->registered));

  log_debug(LD_CHANNEL,
            "Freeing channel %"PRIu64 " at %p",
            (chan->global_identifier), chan);

  /* Get this one out of the scheduler */
  scheduler_release_channel(chan);

  /*
   * Get rid of cmux policy before we do anything, so cmux policies don't
   * see channels in weird half-freed states.
   */
  if (chan->cmux) {
    circuitmux_set_policy(chan->cmux, NULL);
  }

  /* Remove all timers and associated handle entries now */
  timer_free(chan->padding_timer);
  channel_handle_free(chan->timer_handle);
  channel_handles_clear(chan);

  /* Call a free method if there is one */
  if (chan->free_fn) chan->free_fn(chan);

  channel_clear_remote_end(chan);

  /* Get rid of cmux */
  if (chan->cmux) {
    circuitmux_detach_all_circuits(chan->cmux, NULL);
    circuitmux_mark_destroyed_circids_usable(chan->cmux, chan);
    circuitmux_free(chan->cmux);
    chan->cmux = NULL;
  }

  tor_free(chan);
}

/**
 * Free a channel listener; nothing outside of channel.c and subclasses
 * should call this - it frees channel listeners after they have closed and
 * been unregistered.
 */
void
channel_listener_free_(channel_listener_t *chan_l)
{
  if (!chan_l) return;

  log_debug(LD_CHANNEL,
            "Freeing channel_listener_t %"PRIu64 " at %p",
            (chan_l->global_identifier),
            chan_l);

  /* It must be closed or errored */
  tor_assert(chan_l->state == CHANNEL_LISTENER_STATE_CLOSED ||
             chan_l->state == CHANNEL_LISTENER_STATE_ERROR);
  /* It must be deregistered */
  tor_assert(!(chan_l->registered));

  /* Call a free method if there is one */
  if (chan_l->free_fn) chan_l->free_fn(chan_l);

  tor_free(chan_l);
}

/**
 * Free a channel and skip the state/registration asserts; this internal-
 * use-only function should be called only from channel_free_all() when
 * shutting down the Tor process.
 */
static void
channel_force_xfree(channel_t *chan)
{
  tor_assert(chan);

  log_debug(LD_CHANNEL,
            "Force-freeing channel %"PRIu64 " at %p",
            (chan->global_identifier), chan);

  /* Get this one out of the scheduler */
  scheduler_release_channel(chan);

  /*
   * Get rid of cmux policy before we do anything, so cmux policies don't
   * see channels in weird half-freed states.
   */
  if (chan->cmux) {
    circuitmux_set_policy(chan->cmux, NULL);
  }

  /* Remove all timers and associated handle entries now */
  timer_free(chan->padding_timer);
  channel_handle_free(chan->timer_handle);
  channel_handles_clear(chan);

  /* Call a free method if there is one */
  if (chan->free_fn) chan->free_fn(chan);

  channel_clear_remote_end(chan);

  /* Get rid of cmux */
  if (chan->cmux) {
    circuitmux_free(chan->cmux);
    chan->cmux = NULL;
  }

  tor_free(chan);
}

/**
 * Free a channel listener and skip the state/registration asserts; this
 * internal-use-only function should be called only from channel_free_all()
 * when shutting down the Tor process.
 */
static void
channel_listener_force_xfree(channel_listener_t *chan_l)
{
  tor_assert(chan_l);

  log_debug(LD_CHANNEL,
            "Force-freeing channel_listener_t %"PRIu64 " at %p",
            (chan_l->global_identifier),
            chan_l);

  /* Call a free method if there is one */
  if (chan_l->free_fn) chan_l->free_fn(chan_l);

  /*
   * The incoming list just gets emptied and freed; we request close on
   * any channels we find there, but since we got called while shutting
   * down they will get deregistered and freed elsewhere anyway.
   */
  if (chan_l->incoming_list) {
    SMARTLIST_FOREACH_BEGIN(chan_l->incoming_list,
                            channel_t *, qchan) {
      channel_mark_for_close(qchan);
    } SMARTLIST_FOREACH_END(qchan);

    smartlist_free(chan_l->incoming_list);
    chan_l->incoming_list = NULL;
  }

  tor_free(chan_l);
}

/**
 * Set the listener for a channel listener.
 *
 * This function sets the handler for new incoming channels on a channel
 * listener.
 */
void
channel_listener_set_listener_fn(channel_listener_t *chan_l,
                                channel_listener_fn_ptr listener)
{
  tor_assert(chan_l);
  tor_assert(chan_l->state == CHANNEL_LISTENER_STATE_LISTENING);

  log_debug(LD_CHANNEL,
           "Setting listener callback for channel listener %p "
           "(global ID %"PRIu64 ") to %p",
           chan_l, (chan_l->global_identifier),
           listener);

  chan_l->listener = listener;
  if (chan_l->listener) channel_listener_process_incoming(chan_l);
}

/**
 * Return the fixed-length cell handler for a channel.
 *
 * This function gets the handler for incoming fixed-length cells installed
 * on a channel.
 */
channel_cell_handler_fn_ptr
channel_get_cell_handler(channel_t *chan)
{
  tor_assert(chan);

  if (CHANNEL_CAN_HANDLE_CELLS(chan))
    return chan->cell_handler;

  return NULL;
}

/**
 * Set both cell handlers for a channel.
 *
 * This function sets both the fixed-length and variable length cell handlers
 * for a channel.
 */
void
channel_set_cell_handlers(channel_t *chan,
                          channel_cell_handler_fn_ptr cell_handler)
{
  tor_assert(chan);
  tor_assert(CHANNEL_CAN_HANDLE_CELLS(chan));

  log_debug(LD_CHANNEL,
           "Setting cell_handler callback for channel %p to %p",
           chan, cell_handler);

  /* Change them */
  chan->cell_handler = cell_handler;
}

/*
 * On closing channels
 *
 * There are three functions that close channels, for use in
 * different circumstances:
 *
 *  - Use channel_mark_for_close() for most cases
 *  - Use channel_close_from_lower_layer() if you are connection_or.c
 *    and the other end closes the underlying connection.
 *  - Use channel_close_for_error() if you are connection_or.c and
 *    some sort of error has occurred.
 */

/**
 * Mark a channel for closure.
 *
 * This function tries to close a channel_t; it will go into the CLOSING
 * state, and eventually the lower layer should put it into the CLOSED or
 * ERROR state.  Then, channel_run_cleanup() will eventually free it.
 */
void
channel_mark_for_close(channel_t *chan)
{
  tor_assert(chan != NULL);
  tor_assert(chan->close != NULL);

  /* If it's already in CLOSING, CLOSED or ERROR, this is a no-op */
  if (CHANNEL_CONDEMNED(chan))
    return;

  log_debug(LD_CHANNEL,
            "Closing channel %p (global ID %"PRIu64 ") "
            "by request",
            chan, (chan->global_identifier));

  /* Note closing by request from above */
  chan->reason_for_closing = CHANNEL_CLOSE_REQUESTED;

  /* Change state to CLOSING */
  channel_change_state(chan, CHANNEL_STATE_CLOSING);

  /* Tell the lower layer */
  chan->close(chan);

  /*
   * It's up to the lower layer to change state to CLOSED or ERROR when we're
   * ready; we'll try to free channels that are in the finished list from
   * channel_run_cleanup().  The lower layer should do this by calling
   * channel_closed().
   */
}

/**
 * Mark a channel listener for closure.
 *
 * This function tries to close a channel_listener_t; it will go into the
 * CLOSING state, and eventually the lower layer should put it into the CLOSED
 * or ERROR state.  Then, channel_run_cleanup() will eventually free it.
 */
void
channel_listener_mark_for_close(channel_listener_t *chan_l)
{
  tor_assert(chan_l != NULL);
  tor_assert(chan_l->close != NULL);

  /* If it's already in CLOSING, CLOSED or ERROR, this is a no-op */
  if (chan_l->state == CHANNEL_LISTENER_STATE_CLOSING ||
      chan_l->state == CHANNEL_LISTENER_STATE_CLOSED ||
      chan_l->state == CHANNEL_LISTENER_STATE_ERROR) return;

  log_debug(LD_CHANNEL,
            "Closing channel listener %p (global ID %"PRIu64 ") "
            "by request",
            chan_l, (chan_l->global_identifier));

  /* Note closing by request from above */
  chan_l->reason_for_closing = CHANNEL_LISTENER_CLOSE_REQUESTED;

  /* Change state to CLOSING */
  channel_listener_change_state(chan_l, CHANNEL_LISTENER_STATE_CLOSING);

  /* Tell the lower layer */
  chan_l->close(chan_l);

  /*
   * It's up to the lower layer to change state to CLOSED or ERROR when we're
   * ready; we'll try to free channels that are in the finished list from
   * channel_run_cleanup().  The lower layer should do this by calling
   * channel_listener_closed().
   */
}

/**
 * Close a channel from the lower layer.
 *
 * Notify the channel code that the channel is being closed due to a non-error
 * condition in the lower layer.  This does not call the close() method, since
 * the lower layer already knows.
 */
void
channel_close_from_lower_layer(channel_t *chan)
{
  tor_assert(chan != NULL);

  /* If it's already in CLOSING, CLOSED or ERROR, this is a no-op */
  if (CHANNEL_CONDEMNED(chan))
    return;

  log_debug(LD_CHANNEL,
            "Closing channel %p (global ID %"PRIu64 ") "
            "due to lower-layer event",
            chan, (chan->global_identifier));

  /* Note closing by event from below */
  chan->reason_for_closing = CHANNEL_CLOSE_FROM_BELOW;

  /* Change state to CLOSING */
  channel_change_state(chan, CHANNEL_STATE_CLOSING);
}

/**
 * Notify that the channel is being closed due to an error condition.
 *
 * This function is called by the lower layer implementing the transport
 * when a channel must be closed due to an error condition.  This does not
 * call the channel's close method, since the lower layer already knows.
 */
void
channel_close_for_error(channel_t *chan)
{
  tor_assert(chan != NULL);

  /* If it's already in CLOSING, CLOSED or ERROR, this is a no-op */
  if (CHANNEL_CONDEMNED(chan))
    return;

  log_debug(LD_CHANNEL,
            "Closing channel %p due to lower-layer error",
            chan);

  /* Note closing by event from below */
  chan->reason_for_closing = CHANNEL_CLOSE_FOR_ERROR;

  /* Change state to CLOSING */
  channel_change_state(chan, CHANNEL_STATE_CLOSING);
}

/**
 * Notify that the lower layer is finished closing the channel.
 *
 * This function should be called by the lower layer when a channel
 * is finished closing and it should be regarded as inactive and
 * freed by the channel code.
 */
void
channel_closed(channel_t *chan)
{
  tor_assert(chan);
  tor_assert(CHANNEL_CONDEMNED(chan));

  /* No-op if already inactive */
  if (CHANNEL_FINISHED(chan))
    return;

  /* Inform any pending (not attached) circs that they should
   * give up. */
  if (! chan->has_been_open)
    circuit_n_chan_done(chan, 0, 0);

  /* Now close all the attached circuits on it. */
  circuit_unlink_all_from_channel(chan, END_CIRC_REASON_CHANNEL_CLOSED);

  if (chan->reason_for_closing != CHANNEL_CLOSE_FOR_ERROR) {
    channel_change_state(chan, CHANNEL_STATE_CLOSED);
  } else {
    channel_change_state(chan, CHANNEL_STATE_ERROR);
  }
}

/**
 * Clear the identity_digest of a channel.
 *
 * This function clears the identity digest of the remote endpoint for a
 * channel; this is intended for use by the lower layer.
 */
void
channel_clear_identity_digest(channel_t *chan)
{
  int state_not_in_map;

  tor_assert(chan);

  log_debug(LD_CHANNEL,
            "Clearing remote endpoint digest on channel %p with "
            "global ID %"PRIu64,
            chan, (chan->global_identifier));

  state_not_in_map = CHANNEL_CONDEMNED(chan);

  if (!state_not_in_map && chan->registered &&
      !tor_digest_is_zero(chan->identity_digest))
    /* if it's registered get it out of the digest map */
    channel_remove_from_digest_map(chan);

  memset(chan->identity_digest, 0,
         sizeof(chan->identity_digest));
}

/**
 * Set the identity_digest of a channel.
 *
 * This function sets the identity digest of the remote endpoint for a
 * channel; this is intended for use by the lower layer.
 */
void
channel_set_identity_digest(channel_t *chan,
                            const char *identity_digest,
                            const ed25519_public_key_t *ed_identity)
{
  int was_in_digest_map, should_be_in_digest_map, state_not_in_map;

  tor_assert(chan);

  log_debug(LD_CHANNEL,
            "Setting remote endpoint digest on channel %p with "
            "global ID %"PRIu64 " to digest %s",
            chan, (chan->global_identifier),
            identity_digest ?
              hex_str(identity_digest, DIGEST_LEN) : "(null)");

  state_not_in_map = CHANNEL_CONDEMNED(chan);

  was_in_digest_map =
    !state_not_in_map &&
    chan->registered &&
    !tor_digest_is_zero(chan->identity_digest);
  should_be_in_digest_map =
    !state_not_in_map &&
    chan->registered &&
    (identity_digest &&
     !tor_digest_is_zero(identity_digest));

  if (was_in_digest_map)
    /* We should always remove it; we'll add it back if we're writing
     * in a new digest.
     */
    channel_remove_from_digest_map(chan);

  if (identity_digest) {
    memcpy(chan->identity_digest,
           identity_digest,
           sizeof(chan->identity_digest));
  } else {
    memset(chan->identity_digest, 0,
           sizeof(chan->identity_digest));
  }
  if (ed_identity) {
    memcpy(&chan->ed25519_identity, ed_identity, sizeof(*ed_identity));
  } else {
    memset(&chan->ed25519_identity, 0, sizeof(*ed_identity));
  }

  /* Put it in the digest map if we should */
  if (should_be_in_digest_map)
    channel_add_to_digest_map(chan);
}

/**
 * Clear the remote end metadata (identity_digest) of a channel.
 *
 * This function clears all the remote end info from a channel; this is
 * intended for use by the lower layer.
 */
void
channel_clear_remote_end(channel_t *chan)
{
  int state_not_in_map;

  tor_assert(chan);

  log_debug(LD_CHANNEL,
            "Clearing remote endpoint identity on channel %p with "
            "global ID %"PRIu64,
            chan, (chan->global_identifier));

  state_not_in_map = CHANNEL_CONDEMNED(chan);

  if (!state_not_in_map && chan->registered &&
      !tor_digest_is_zero(chan->identity_digest))
    /* if it's registered get it out of the digest map */
    channel_remove_from_digest_map(chan);

  memset(chan->identity_digest, 0,
         sizeof(chan->identity_digest));
}

/**
 * Write to a channel the given packed cell.
 *
 * Two possible errors can happen. Either the channel is not opened or the
 * lower layer (specialized channel) failed to write it. In both cases, it is
 * the caller responsibility to free the cell.
 */
static int
write_packed_cell(channel_t *chan, packed_cell_t *cell)
{
  int ret = -1;
  size_t cell_bytes;
  uint8_t command = packed_cell_get_command(cell, chan->wide_circ_ids);

  tor_assert(chan);
  tor_assert(cell);

  /* Assert that the state makes sense for a cell write */
  tor_assert(CHANNEL_CAN_HANDLE_CELLS(chan));

  {
    circid_t circ_id;
    if (packed_cell_is_destroy(chan, cell, &circ_id)) {
      channel_note_destroy_not_pending(chan, circ_id);
    }
  }

  /* For statistical purposes, figure out how big this cell is */
  cell_bytes = get_cell_network_size(chan->wide_circ_ids);

  /* Can we send it right out?  If so, try */
  if (!CHANNEL_IS_OPEN(chan)) {
    goto done;
  }

  /* Write the cell on the connection's outbuf. */
  if (chan->write_packed_cell(chan, cell) < 0) {
    goto done;
  }
  /* Timestamp for transmission */
  channel_timestamp_xmit(chan);
  /* Update the counter */
  ++(chan->n_cells_xmitted);
  chan->n_bytes_xmitted += cell_bytes;
  /* Successfully sent the cell. */
  ret = 0;

  /* Update padding statistics for the packed codepath.. */
  rep_hist_padding_count_write(PADDING_TYPE_TOTAL);
  if (command == CELL_PADDING)
    rep_hist_padding_count_write(PADDING_TYPE_CELL);
  if (chan->padding_enabled) {
    rep_hist_padding_count_write(PADDING_TYPE_ENABLED_TOTAL);
    if (command == CELL_PADDING)
      rep_hist_padding_count_write(PADDING_TYPE_ENABLED_CELL);
  }

 done:
  return ret;
}

/**
 * Write a packed cell to a channel.
 *
 * Write a packed cell to a channel using the write_cell() method.  This is
 * called by the transport-independent code to deliver a packed cell to a
 * channel for transmission.
 *
 * Return 0 on success else a negative value. In both cases, the caller should
 * not access the cell anymore, it is freed both on success and error.
 */
int
channel_write_packed_cell(channel_t *chan, packed_cell_t *cell)
{
  int ret = -1;

  tor_assert(chan);
  tor_assert(cell);

  if (CHANNEL_IS_CLOSING(chan)) {
    log_debug(LD_CHANNEL, "Discarding %p on closing channel %p with "
              "global ID %"PRIu64, cell, chan,
              (chan->global_identifier));
    goto end;
  }
  log_debug(LD_CHANNEL,
            "Writing %p to channel %p with global ID "
            "%"PRIu64, cell, chan, (chan->global_identifier));

  ret = write_packed_cell(chan, cell);

 end:
  /* Whatever happens, we free the cell. Either an error occurred or the cell
   * was put on the connection outbuf, both cases we have ownership of the
   * cell and we free it. */
  packed_cell_free(cell);
  return ret;
}

/**
 * Change channel state.
 *
 * This internal and subclass use only function is used to change channel
 * state, performing all transition validity checks and whatever actions
 * are appropriate to the state transition in question.
 */
static void
channel_change_state_(channel_t *chan, channel_state_t to_state)
{
  channel_state_t from_state;
  unsigned char was_active, is_active;
  unsigned char was_in_id_map, is_in_id_map;

  tor_assert(chan);
  from_state = chan->state;

  tor_assert(channel_state_is_valid(from_state));
  tor_assert(channel_state_is_valid(to_state));
  tor_assert(channel_state_can_transition(chan->state, to_state));

  /* Check for no-op transitions */
  if (from_state == to_state) {
    log_debug(LD_CHANNEL,
              "Got no-op transition from \"%s\" to itself on channel %p"
              "(global ID %"PRIu64 ")",
              channel_state_to_string(to_state),
              chan, (chan->global_identifier));
    return;
  }

  /* If we're going to a closing or closed state, we must have a reason set */
  if (to_state == CHANNEL_STATE_CLOSING ||
      to_state == CHANNEL_STATE_CLOSED ||
      to_state == CHANNEL_STATE_ERROR) {
    tor_assert(chan->reason_for_closing != CHANNEL_NOT_CLOSING);
  }

  log_debug(LD_CHANNEL,
            "Changing state of channel %p (global ID %"PRIu64
            ") from \"%s\" to \"%s\"",
            chan,
            (chan->global_identifier),
            channel_state_to_string(chan->state),
            channel_state_to_string(to_state));

  chan->state = to_state;

  /* Need to add to the right lists if the channel is registered */
  if (chan->registered) {
    was_active = !(from_state == CHANNEL_STATE_CLOSED ||
                   from_state == CHANNEL_STATE_ERROR);
    is_active = !(to_state == CHANNEL_STATE_CLOSED ||
                  to_state == CHANNEL_STATE_ERROR);

    /* Need to take off active list and put on finished list? */
    if (was_active && !is_active) {
      if (active_channels) smartlist_remove(active_channels, chan);
      if (!finished_channels) finished_channels = smartlist_new();
      smartlist_add(finished_channels, chan);
      mainloop_schedule_postloop_cleanup();
    }
    /* Need to put on active list? */
    else if (!was_active && is_active) {
      if (finished_channels) smartlist_remove(finished_channels, chan);
      if (!active_channels) active_channels = smartlist_new();
      smartlist_add(active_channels, chan);
    }

    if (!tor_digest_is_zero(chan->identity_digest)) {
      /* Now we need to handle the identity map */
      was_in_id_map = !(from_state == CHANNEL_STATE_CLOSING ||
                        from_state == CHANNEL_STATE_CLOSED ||
                        from_state == CHANNEL_STATE_ERROR);
      is_in_id_map = !(to_state == CHANNEL_STATE_CLOSING ||
                       to_state == CHANNEL_STATE_CLOSED ||
                       to_state == CHANNEL_STATE_ERROR);

      if (!was_in_id_map && is_in_id_map) channel_add_to_digest_map(chan);
      else if (was_in_id_map && !is_in_id_map)
        channel_remove_from_digest_map(chan);
    }
  }

  /*
   * If we're going to a closed/closing state, we don't need scheduling any
   * more; in CHANNEL_STATE_MAINT we can't accept writes.
   */
  if (to_state == CHANNEL_STATE_CLOSING ||
      to_state == CHANNEL_STATE_CLOSED ||
      to_state == CHANNEL_STATE_ERROR) {
    scheduler_release_channel(chan);
  } else if (to_state == CHANNEL_STATE_MAINT) {
    scheduler_channel_doesnt_want_writes(chan);
  }
}

/**
 * As channel_change_state_, but change the state to any state but open.
 */
void
channel_change_state(channel_t *chan, channel_state_t to_state)
{
  tor_assert(to_state != CHANNEL_STATE_OPEN);
  channel_change_state_(chan, to_state);
}

/**
 * As channel_change_state, but change the state to open.
 */
void
channel_change_state_open(channel_t *chan)
{
  channel_change_state_(chan, CHANNEL_STATE_OPEN);

  /* Tell circuits if we opened and stuff */
  channel_do_open_actions(chan);
  chan->has_been_open = 1;
}

/**
 * Change channel listener state.
 *
 * This internal and subclass use only function is used to change channel
 * listener state, performing all transition validity checks and whatever
 * actions are appropriate to the state transition in question.
 */
void
channel_listener_change_state(channel_listener_t *chan_l,
                              channel_listener_state_t to_state)
{
  channel_listener_state_t from_state;
  unsigned char was_active, is_active;

  tor_assert(chan_l);
  from_state = chan_l->state;

  tor_assert(channel_listener_state_is_valid(from_state));
  tor_assert(channel_listener_state_is_valid(to_state));
  tor_assert(channel_listener_state_can_transition(chan_l->state, to_state));

  /* Check for no-op transitions */
  if (from_state == to_state) {
    log_debug(LD_CHANNEL,
              "Got no-op transition from \"%s\" to itself on channel "
              "listener %p (global ID %"PRIu64 ")",
              channel_listener_state_to_string(to_state),
              chan_l, (chan_l->global_identifier));
    return;
  }

  /* If we're going to a closing or closed state, we must have a reason set */
  if (to_state == CHANNEL_LISTENER_STATE_CLOSING ||
      to_state == CHANNEL_LISTENER_STATE_CLOSED ||
      to_state == CHANNEL_LISTENER_STATE_ERROR) {
    tor_assert(chan_l->reason_for_closing != CHANNEL_LISTENER_NOT_CLOSING);
  }

  log_debug(LD_CHANNEL,
            "Changing state of channel listener %p (global ID %"PRIu64
            "from \"%s\" to \"%s\"",
            chan_l, (chan_l->global_identifier),
            channel_listener_state_to_string(chan_l->state),
            channel_listener_state_to_string(to_state));

  chan_l->state = to_state;

  /* Need to add to the right lists if the channel listener is registered */
  if (chan_l->registered) {
    was_active = !(from_state == CHANNEL_LISTENER_STATE_CLOSED ||
                   from_state == CHANNEL_LISTENER_STATE_ERROR);
    is_active = !(to_state == CHANNEL_LISTENER_STATE_CLOSED ||
                  to_state == CHANNEL_LISTENER_STATE_ERROR);

    /* Need to take off active list and put on finished list? */
    if (was_active && !is_active) {
      if (active_listeners) smartlist_remove(active_listeners, chan_l);
      if (!finished_listeners) finished_listeners = smartlist_new();
      smartlist_add(finished_listeners, chan_l);
      mainloop_schedule_postloop_cleanup();
    }
    /* Need to put on active list? */
    else if (!was_active && is_active) {
      if (finished_listeners) smartlist_remove(finished_listeners, chan_l);
      if (!active_listeners) active_listeners = smartlist_new();
      smartlist_add(active_listeners, chan_l);
    }
  }

  if (to_state == CHANNEL_LISTENER_STATE_CLOSED ||
      to_state == CHANNEL_LISTENER_STATE_ERROR) {
    tor_assert(!(chan_l->incoming_list) ||
                smartlist_len(chan_l->incoming_list) == 0);
  }
}

/* Maximum number of cells that is allowed to flush at once within
 * channel_flush_some_cells(). */
#define MAX_CELLS_TO_GET_FROM_CIRCUITS_FOR_UNLIMITED 256

/**
 * Try to flush cells of the given channel chan up to a maximum of num_cells.
 *
 * This is called by the scheduler when it wants to flush cells from the
 * channel's circuit queue(s) to the connection outbuf (not yet on the wire).
 *
 * If the channel is not in state CHANNEL_STATE_OPEN, this does nothing and
 * will return 0 meaning no cells were flushed.
 *
 * If num_cells is -1, we'll try to flush up to the maximum cells allowed
 * defined in MAX_CELLS_TO_GET_FROM_CIRCUITS_FOR_UNLIMITED.
 *
 * On success, the number of flushed cells are returned and it can never be
 * above num_cells. If 0 is returned, no cells were flushed either because the
 * channel was not opened or we had no cells on the channel. A negative number
 * can NOT be sent back.
 *
 * This function is part of the fast path. */
MOCK_IMPL(ssize_t,
channel_flush_some_cells, (channel_t *chan, ssize_t num_cells))
{
  unsigned int unlimited = 0;
  ssize_t flushed = 0;
  int clamped_num_cells;

  tor_assert(chan);

  if (num_cells < 0) unlimited = 1;
  if (!unlimited && num_cells <= flushed) goto done;

  /* If we aren't in CHANNEL_STATE_OPEN, nothing goes through */
  if (CHANNEL_IS_OPEN(chan)) {
    if (circuitmux_num_cells(chan->cmux) > 0) {
      /* Calculate number of cells, including clamp */
      if (unlimited) {
        clamped_num_cells = MAX_CELLS_TO_GET_FROM_CIRCUITS_FOR_UNLIMITED;
      } else {
        if (num_cells - flushed >
            MAX_CELLS_TO_GET_FROM_CIRCUITS_FOR_UNLIMITED) {
          clamped_num_cells = MAX_CELLS_TO_GET_FROM_CIRCUITS_FOR_UNLIMITED;
        } else {
          clamped_num_cells = (int)(num_cells - flushed);
        }
      }

      /* Try to get more cells from any active circuits */
      flushed = channel_flush_from_first_active_circuit(
          chan, clamped_num_cells);
    }
  }

 done:
  return flushed;
}

/**
 * Check if any cells are available.
 *
 * This is used by the scheduler to know if the channel has more to flush
 * after a scheduling round.
 */
MOCK_IMPL(int,
channel_more_to_flush, (channel_t *chan))
{
  tor_assert(chan);

  if (circuitmux_num_cells(chan->cmux) > 0) return 1;

  /* Else no */
  return 0;
}

/**
 * Notify the channel we're done flushing the output in the lower layer.
 *
 * Connection.c will call this when we've flushed the output; there's some
 * dirreq-related maintenance to do.
 */
void
channel_notify_flushed(channel_t *chan)
{
  tor_assert(chan);

  if (chan->dirreq_id != 0)
    geoip_change_dirreq_state(chan->dirreq_id,
                              DIRREQ_TUNNELED,
                              DIRREQ_CHANNEL_BUFFER_FLUSHED);
}

/**
 * Process the queue of incoming channels on a listener.
 *
 * Use a listener's registered callback to process as many entries in the
 * queue of incoming channels as possible.
 */
void
channel_listener_process_incoming(channel_listener_t *listener)
{
  tor_assert(listener);

  /*
   * CHANNEL_LISTENER_STATE_CLOSING permitted because we drain the queue
   * while closing a listener.
   */
  tor_assert(listener->state == CHANNEL_LISTENER_STATE_LISTENING ||
             listener->state == CHANNEL_LISTENER_STATE_CLOSING);
  tor_assert(listener->listener);

  log_debug(LD_CHANNEL,
            "Processing queue of incoming connections for channel "
            "listener %p (global ID %"PRIu64 ")",
            listener, (listener->global_identifier));

  if (!(listener->incoming_list)) return;

  SMARTLIST_FOREACH_BEGIN(listener->incoming_list,
                          channel_t *, chan) {
    tor_assert(chan);

    log_debug(LD_CHANNEL,
              "Handling incoming channel %p (%"PRIu64 ") "
              "for listener %p (%"PRIu64 ")",
              chan,
              (chan->global_identifier),
              listener,
              (listener->global_identifier));
    /* Make sure this is set correctly */
    channel_mark_incoming(chan);
    listener->listener(listener, chan);
  } SMARTLIST_FOREACH_END(chan);

  smartlist_free(listener->incoming_list);
  listener->incoming_list = NULL;
}

/**
 * Take actions required when a channel becomes open.
 *
 * Handle actions we should do when we know a channel is open; a lot of
 * this comes from the old connection_or_set_state_open() of connection_or.c.
 *
 * Because of this mechanism, future channel_t subclasses should take care
 * not to change a channel from CHANNEL_STATE_OPENING to CHANNEL_STATE_OPEN
 * until there is positive confirmation that the network is operational.
 * In particular, anything UDP-based should not make this transition until a
 * packet is received from the other side.
 */
void
channel_do_open_actions(channel_t *chan)
{
  tor_addr_t remote_addr;
  int started_here;
  int close_origin_circuits = 0;

  tor_assert(chan);

  started_here = channel_is_outgoing(chan);

  if (started_here) {
    circuit_build_times_network_is_live(get_circuit_build_times_mutable());
    router_set_status(chan->identity_digest, 1);
  } else {
    /* only report it to the geoip module if it's a client and it hasn't
     * already been set up for tracking earlier. (Incoming TLS connections
     * are tracked before the handshake.) */
    if (channel_is_client(chan)) {
      if (channel_get_addr_if_possible(chan, &remote_addr)) {
        channel_tls_t *tlschan = BASE_CHAN_TO_TLS(chan);
        if (!tlschan->conn->tracked_for_dos_mitigation) {
          char *transport_name = NULL;
          if (chan->get_transport_name(chan, &transport_name) < 0) {
            transport_name = NULL;
          }
          geoip_note_client_seen(GEOIP_CLIENT_CONNECT,
                                 &remote_addr, transport_name,
                                 time(NULL));
          if (tlschan && tlschan->conn) {
            dos_new_client_conn(tlschan->conn, transport_name);
          }
          tor_free(transport_name);
        }
      }
      /* Otherwise the underlying transport can't tell us this, so skip it */
    }
  }

  /* Disable or reduce padding according to user prefs. */
  if (chan->padding_enabled || get_options()->ConnectionPadding == 1) {
    if (!get_options()->ConnectionPadding) {
      /* Disable if torrc disabled */
      channelpadding_disable_padding_on_channel(chan);
    } else if (hs_service_allow_non_anonymous_connection(get_options()) &&
               !networkstatus_get_param(NULL,
                                        CHANNELPADDING_SOS_PARAM,
                                        CHANNELPADDING_SOS_DEFAULT, 0, 1)) {
      /* Disable if we're using RSOS and the consensus disabled padding
       * for RSOS */
      channelpadding_disable_padding_on_channel(chan);
    } else if (get_options()->ReducedConnectionPadding) {
      /* Padding can be forced and/or reduced by clients, regardless of if
       * the channel supports it */
      channelpadding_reduce_padding_on_channel(chan);
    }
  }

  circuit_n_chan_done(chan, 1, close_origin_circuits);
}

/**
 * Queue an incoming channel on a listener.
 *
 * Internal and subclass use only function to queue an incoming channel from
 * a listener.  A subclass of channel_listener_t should call this when a new
 * incoming channel is created.
 */
void
channel_listener_queue_incoming(channel_listener_t *listener,
                                channel_t *incoming)
{
  int need_to_queue = 0;

  tor_assert(listener);
  tor_assert(listener->state == CHANNEL_LISTENER_STATE_LISTENING);
  tor_assert(incoming);

  log_debug(LD_CHANNEL,
            "Queueing incoming channel %p (global ID %"PRIu64 ") on "
            "channel listener %p (global ID %"PRIu64 ")",
            incoming, (incoming->global_identifier),
            listener, (listener->global_identifier));

  /* Do we need to queue it, or can we just call the listener right away? */
  if (!(listener->listener)) need_to_queue = 1;
  if (listener->incoming_list &&
      (smartlist_len(listener->incoming_list) > 0))
    need_to_queue = 1;

  /* If we need to queue and have no queue, create one */
  if (need_to_queue && !(listener->incoming_list)) {
    listener->incoming_list = smartlist_new();
  }

  /* Bump the counter and timestamp it */
  channel_listener_timestamp_active(listener);
  channel_listener_timestamp_accepted(listener);
  ++(listener->n_accepted);

  /* If we don't need to queue, process it right away */
  if (!need_to_queue) {
    tor_assert(listener->listener);
    listener->listener(listener, incoming);
  }
  /*
   * Otherwise, we need to queue; queue and then process the queue if
   * we can.
   */
  else {
    tor_assert(listener->incoming_list);
    smartlist_add(listener->incoming_list, incoming);
    if (listener->listener) channel_listener_process_incoming(listener);
  }
}

/**
 * Process a cell from the given channel.
 */
void
channel_process_cell(channel_t *chan, cell_t *cell)
{
  tor_assert(chan);
  tor_assert(CHANNEL_IS_CLOSING(chan) || CHANNEL_IS_MAINT(chan) ||
             CHANNEL_IS_OPEN(chan));
  tor_assert(cell);

  /* Nothing we can do if we have no registered cell handlers */
  if (!chan->cell_handler)
    return;

  /* Timestamp for receiving */
  channel_timestamp_recv(chan);
  /* Update received counter. */
  ++(chan->n_cells_recved);
  chan->n_bytes_recved += get_cell_network_size(chan->wide_circ_ids);

  log_debug(LD_CHANNEL,
            "Processing incoming cell_t %p for channel %p (global ID "
            "%"PRIu64 ")", cell, chan,
            (chan->global_identifier));
  chan->cell_handler(chan, cell);
}

/** If <b>packed_cell</b> on <b>chan</b> is a destroy cell, then set
 * *<b>circid_out</b> to its circuit ID, and return true.  Otherwise, return
 * false. */
/* XXXX Move this function. */
int
packed_cell_is_destroy(channel_t *chan,
                       const packed_cell_t *packed_cell,
                       circid_t *circid_out)
{
  if (chan->wide_circ_ids) {
    if (packed_cell->body[4] == CELL_DESTROY) {
      *circid_out = ntohl(get_uint32(packed_cell->body));
      return 1;
    }
  } else {
    if (packed_cell->body[2] == CELL_DESTROY) {
      *circid_out = ntohs(get_uint16(packed_cell->body));
      return 1;
    }
  }
  return 0;
}

/**
 * Send destroy cell on a channel.
 *
 * Write a destroy cell with circ ID <b>circ_id</b> and reason <b>reason</b>
 * onto channel <b>chan</b>.  Don't perform range-checking on reason:
 * we may want to propagate reasons from other cells.
 */
int
channel_send_destroy(circid_t circ_id, channel_t *chan, int reason)
{
  tor_assert(chan);
  if (circ_id == 0) {
    log_warn(LD_BUG, "Attempted to send a destroy cell for circID 0 "
             "on a channel %"PRIu64 " at %p in state %s (%d)",
             (chan->global_identifier),
             chan, channel_state_to_string(chan->state),
             chan->state);
    return 0;
  }

  /* Check to make sure we can send on this channel first */
  if (!CHANNEL_CONDEMNED(chan) && chan->cmux) {
    channel_note_destroy_pending(chan, circ_id);
    circuitmux_append_destroy_cell(chan, chan->cmux, circ_id, reason);
    log_debug(LD_OR,
              "Sending destroy (circID %u) on channel %p "
              "(global ID %"PRIu64 ")",
              (unsigned)circ_id, chan,
              (chan->global_identifier));
  } else {
    log_warn(LD_BUG,
             "Someone called channel_send_destroy() for circID %u "
             "on a channel %"PRIu64 " at %p in state %s (%d)",
             (unsigned)circ_id, (chan->global_identifier),
             chan, channel_state_to_string(chan->state),
             chan->state);
  }

  return 0;
}

/**
 * Dump channel statistics to the log.
 *
 * This is called from dumpstats() in main.c and spams the log with
 * statistics on channels.
 */
void
channel_dumpstats(int severity)
{
  if (all_channels && smartlist_len(all_channels) > 0) {
    tor_log(severity, LD_GENERAL,
        "Dumping statistics about %d channels:",
        smartlist_len(all_channels));
    tor_log(severity, LD_GENERAL,
        "%d are active, and %d are done and waiting for cleanup",
        (active_channels != NULL) ?
          smartlist_len(active_channels) : 0,
        (finished_channels != NULL) ?
          smartlist_len(finished_channels) : 0);

    SMARTLIST_FOREACH(all_channels, channel_t *, chan,
                      channel_dump_statistics(chan, severity));

    tor_log(severity, LD_GENERAL,
        "Done spamming about channels now");
  } else {
    tor_log(severity, LD_GENERAL,
        "No channels to dump");
  }
}

/**
 * Dump channel listener statistics to the log.
 *
 * This is called from dumpstats() in main.c and spams the log with
 * statistics on channel listeners.
 */
void
channel_listener_dumpstats(int severity)
{
  if (all_listeners && smartlist_len(all_listeners) > 0) {
    tor_log(severity, LD_GENERAL,
        "Dumping statistics about %d channel listeners:",
        smartlist_len(all_listeners));
    tor_log(severity, LD_GENERAL,
        "%d are active and %d are done and waiting for cleanup",
        (active_listeners != NULL) ?
          smartlist_len(active_listeners) : 0,
        (finished_listeners != NULL) ?
          smartlist_len(finished_listeners) : 0);

    SMARTLIST_FOREACH(all_listeners, channel_listener_t *, chan_l,
                      channel_listener_dump_statistics(chan_l, severity));

    tor_log(severity, LD_GENERAL,
        "Done spamming about channel listeners now");
  } else {
    tor_log(severity, LD_GENERAL,
        "No channel listeners to dump");
  }
}

/**
 * Clean up channels.
 *
 * This gets called periodically from run_scheduled_events() in main.c;
 * it cleans up after closed channels.
 */
void
channel_run_cleanup(void)
{
  channel_t *tmp = NULL;

  /* Check if we need to do anything */
  if (!finished_channels || smartlist_len(finished_channels) == 0) return;

  /* Iterate through finished_channels and get rid of them */
  SMARTLIST_FOREACH_BEGIN(finished_channels, channel_t *, curr) {
    tmp = curr;
    /* Remove it from the list */
    SMARTLIST_DEL_CURRENT(finished_channels, curr);
    /* Also unregister it */
    channel_unregister(tmp);
    /* ... and free it */
    channel_free(tmp);
  } SMARTLIST_FOREACH_END(curr);
}

/**
 * Clean up channel listeners.
 *
 * This gets called periodically from run_scheduled_events() in main.c;
 * it cleans up after closed channel listeners.
 */
void
channel_listener_run_cleanup(void)
{
  channel_listener_t *tmp = NULL;

  /* Check if we need to do anything */
  if (!finished_listeners || smartlist_len(finished_listeners) == 0) return;

  /* Iterate through finished_channels and get rid of them */
  SMARTLIST_FOREACH_BEGIN(finished_listeners, channel_listener_t *, curr) {
    tmp = curr;
    /* Remove it from the list */
    SMARTLIST_DEL_CURRENT(finished_listeners, curr);
    /* Also unregister it */
    channel_listener_unregister(tmp);
    /* ... and free it */
    channel_listener_free(tmp);
  } SMARTLIST_FOREACH_END(curr);
}

/**
 * Free a list of channels for channel_free_all().
 */
static void
channel_free_list(smartlist_t *channels, int mark_for_close)
{
  if (!channels) return;

  SMARTLIST_FOREACH_BEGIN(channels, channel_t *, curr) {
    /* Deregister and free it */
    tor_assert(curr);
    log_debug(LD_CHANNEL,
              "Cleaning up channel %p (global ID %"PRIu64 ") "
              "in state %s (%d)",
              curr, (curr->global_identifier),
              channel_state_to_string(curr->state), curr->state);
    /* Detach circuits early so they can find the channel */
    if (curr->cmux) {
      circuitmux_detach_all_circuits(curr->cmux, NULL);
    }
    SMARTLIST_DEL_CURRENT(channels, curr);
    channel_unregister(curr);
    if (mark_for_close) {
      if (!CHANNEL_CONDEMNED(curr)) {
        channel_mark_for_close(curr);
      }
      channel_force_xfree(curr);
    } else channel_free(curr);
  } SMARTLIST_FOREACH_END(curr);
}

/**
 * Free a list of channel listeners for channel_free_all().
 */
static void
channel_listener_free_list(smartlist_t *listeners, int mark_for_close)
{
  if (!listeners) return;

  SMARTLIST_FOREACH_BEGIN(listeners, channel_listener_t *, curr) {
    /* Deregister and free it */
    tor_assert(curr);
    log_debug(LD_CHANNEL,
              "Cleaning up channel listener %p (global ID %"PRIu64 ") "
              "in state %s (%d)",
              curr, (curr->global_identifier),
              channel_listener_state_to_string(curr->state), curr->state);
    channel_listener_unregister(curr);
    if (mark_for_close) {
      if (!(curr->state == CHANNEL_LISTENER_STATE_CLOSING ||
            curr->state == CHANNEL_LISTENER_STATE_CLOSED ||
            curr->state == CHANNEL_LISTENER_STATE_ERROR)) {
        channel_listener_mark_for_close(curr);
      }
      channel_listener_force_xfree(curr);
    } else channel_listener_free(curr);
  } SMARTLIST_FOREACH_END(curr);
}

/**
 * Close all channels and free everything.
 *
 * This gets called from tor_free_all() in main.c to clean up on exit.
 * It will close all registered channels and free associated storage,
 * then free the all_channels, active_channels, listening_channels and
 * finished_channels lists and also channel_identity_map.
 */
void
channel_free_all(void)
{
  log_debug(LD_CHANNEL,
            "Shutting down channels...");

  /* First, let's go for finished channels */
  if (finished_channels) {
    channel_free_list(finished_channels, 0);
    smartlist_free(finished_channels);
    finished_channels = NULL;
  }

  /* Now the finished listeners */
  if (finished_listeners) {
    channel_listener_free_list(finished_listeners, 0);
    smartlist_free(finished_listeners);
    finished_listeners = NULL;
  }

  /* Now all active channels */
  if (active_channels) {
    channel_free_list(active_channels, 1);
    smartlist_free(active_channels);
    active_channels = NULL;
  }

  /* Now all active listeners */
  if (active_listeners) {
    channel_listener_free_list(active_listeners, 1);
    smartlist_free(active_listeners);
    active_listeners = NULL;
  }

  /* Now all channels, in case any are left over */
  if (all_channels) {
    channel_free_list(all_channels, 1);
    smartlist_free(all_channels);
    all_channels = NULL;
  }

  /* Now all listeners, in case any are left over */
  if (all_listeners) {
    channel_listener_free_list(all_listeners, 1);
    smartlist_free(all_listeners);
    all_listeners = NULL;
  }

  /* Now free channel_identity_map */
  log_debug(LD_CHANNEL,
            "Freeing channel_identity_map");
  /* Geez, anything still left over just won't die ... let it leak then */
  HT_CLEAR(channel_idmap, &channel_identity_map);

  /* Same with channel_gid_map */
  log_debug(LD_CHANNEL,
            "Freeing channel_gid_map");
  HT_CLEAR(channel_gid_map, &channel_gid_map);

  log_debug(LD_CHANNEL,
            "Done cleaning up after channels");
}

/**
 * Connect to a given addr/port/digest.
 *
 * This sets up a new outgoing channel; in the future if multiple
 * channel_t subclasses are available, this is where the selection policy
 * should go.  It may also be desirable to fold port into tor_addr_t
 * or make a new type including a tor_addr_t and port, so we have a
 * single abstract object encapsulating all the protocol details of
 * how to contact an OR.
 */
channel_t *
channel_connect(const tor_addr_t *addr, uint16_t port,
                const char *id_digest,
                const ed25519_public_key_t *ed_id)
{
  return channel_tls_connect(addr, port, id_digest, ed_id);
}

/**
 * Decide which of two channels to prefer for extending a circuit.
 *
 * This function is called while extending a circuit and returns true iff
 * a is 'better' than b.  The most important criterion here is that a
 * canonical channel is always better than a non-canonical one, but the
 * number of circuits and the age are used as tie-breakers.
 *
 * This is based on the former connection_or_is_better() of connection_or.c
 */
int
channel_is_better(channel_t *a, channel_t *b)
{
  int a_is_canonical, b_is_canonical;

  tor_assert(a);
  tor_assert(b);

  /* If one channel is bad for new circuits, and the other isn't,
   * use the one that is still good. */
  if (!channel_is_bad_for_new_circs(a) && channel_is_bad_for_new_circs(b))
    return 1;
  if (channel_is_bad_for_new_circs(a) && !channel_is_bad_for_new_circs(b))
    return 0;

  /* Check if one is canonical and the other isn't first */
  a_is_canonical = channel_is_canonical(a);
  b_is_canonical = channel_is_canonical(b);

  if (a_is_canonical && !b_is_canonical) return 1;
  if (!a_is_canonical && b_is_canonical) return 0;

  /* Check if we suspect that one of the channels will be preferred
   * by the peer */
  if (a->is_canonical_to_peer && !b->is_canonical_to_peer) return 1;
  if (!a->is_canonical_to_peer && b->is_canonical_to_peer) return 0;

  /*
   * Okay, if we're here they tied on canonicity. Prefer the older
   * connection, so that the adversary can't create a new connection
   * and try to switch us over to it (which will leak information
   * about long-lived circuits). Additionally, switching connections
   * too often makes us more vulnerable to attacks like Torscan and
   * passive netflow-based equivalents.
   *
   * Connections will still only live for at most a week, due to
   * the check in connection_or_group_set_badness() against
   * TIME_BEFORE_OR_CONN_IS_TOO_OLD, which marks old connections as
   * unusable for new circuits after 1 week. That check sets
   * is_bad_for_new_circs, which is checked in channel_get_for_extend().
   *
   * We check channel_is_bad_for_new_circs() above here anyway, for safety.
   */
  if (channel_when_created(a) < channel_when_created(b)) return 1;
  else if (channel_when_created(a) > channel_when_created(b)) return 0;

  if (channel_num_circuits(a) > channel_num_circuits(b)) return 1;
  else return 0;
}

/**
 * Get a channel to extend a circuit.
 *
 * Given the desired relay identity, pick a suitable channel to extend a
 * circuit to the target IPv4 or IPv6 address requested by the client. Search
 * for an existing channel for the requested endpoint. Make sure the channel
 * is usable for new circuits, and matches one of the target addresses.
 *
 * Try to return the best channel. But if there is no good channel, set
 * *msg_out to a message describing the channel's state and our next action,
 * and set *launch_out to a boolean indicated whether the caller should try to
 * launch a new channel with channel_connect().
 *
 * If `for_origin_circ` is set, mark the channel as interesting for origin
 * circuits, and therefore interesting for our bootstrapping reports.
 */
MOCK_IMPL(channel_t *,
channel_get_for_extend,(const char *rsa_id_digest,
                        const ed25519_public_key_t *ed_id,
                        const tor_addr_t *target_ipv4_addr,
                        const tor_addr_t *target_ipv6_addr,
                        bool for_origin_circ,
                        const char **msg_out,
                        int *launch_out))
{
  channel_t *chan, *best = NULL;
  int n_inprogress_goodaddr = 0, n_old = 0;
  int n_noncanonical = 0;

  tor_assert(msg_out);
  tor_assert(launch_out);

  chan = channel_find_by_remote_identity(rsa_id_digest, ed_id);

  /* Walk the list of channels */
  for (; chan; chan = channel_next_with_rsa_identity(chan)) {
    tor_assert(tor_memeq(chan->identity_digest,
                         rsa_id_digest, DIGEST_LEN));

   if (CHANNEL_CONDEMNED(chan))
      continue;

    /* Never return a channel on which the other end appears to be
     * a client. */
    if (channel_is_client(chan)) {
      continue;
    }

    /* The Ed25519 key has to match too */
    if (!channel_remote_identity_matches(chan, rsa_id_digest, ed_id)) {
      continue;
    }

    const bool matches_target =
      channel_matches_target_addr_for_extend(chan,
                                             target_ipv4_addr,
                                             target_ipv6_addr);
    /* Never return a non-open connection. */
    if (!CHANNEL_IS_OPEN(chan)) {
      /* If the address matches, don't launch a new connection for this
       * circuit. */
      if (matches_target) {
        ++n_inprogress_goodaddr;
        if (for_origin_circ) {
          /* We were looking for a connection for an origin circuit; this one
           * matches, so we'll note that we decided to use it for an origin
           * circuit. */
          channel_mark_as_used_for_origin_circuit(chan);
        }
      }
      continue;
    }

    /* Never return a connection that shouldn't be used for circs. */
    if (channel_is_bad_for_new_circs(chan)) {
      ++n_old;
      continue;
    }

    /* Only return canonical connections or connections where the address
     * is the address we wanted. */
    if (!channel_is_canonical(chan) && !matches_target) {
      ++n_noncanonical;
      continue;
    }

    if (!best) {
      best = chan; /* If we have no 'best' so far, this one is good enough. */
      continue;
    }

    if (channel_is_better(chan, best))
      best = chan;
  }

  if (best) {
    *msg_out = "Connection is fine; using it.";
    *launch_out = 0;
    return best;
  } else if (n_inprogress_goodaddr) {
    *msg_out = "Connection in progress; waiting.";
    *launch_out = 0;
    return NULL;
  } else if (n_old || n_noncanonical) {
    *msg_out = "Connections all too old, or too non-canonical. "
               " Launching a new one.";
    *launch_out = 1;
    return NULL;
  } else {
    *msg_out = "Not connected. Connecting.";
    *launch_out = 1;
    return NULL;
  }
}

/**
 * Describe the transport subclass for a channel.
 *
 * Invoke a method to get a string description of the lower-layer
 * transport for this channel.
 */
const char *
channel_describe_transport(channel_t *chan)
{
  tor_assert(chan);
  tor_assert(chan->describe_transport);

  return chan->describe_transport(chan);
}

/**
 * Describe the transport subclass for a channel listener.
 *
 * Invoke a method to get a string description of the lower-layer
 * transport for this channel listener.
 */
const char *
channel_listener_describe_transport(channel_listener_t *chan_l)
{
  tor_assert(chan_l);
  tor_assert(chan_l->describe_transport);

  return chan_l->describe_transport(chan_l);
}

/**
 * Dump channel statistics.
 *
 * Dump statistics for one channel to the log.
 */
MOCK_IMPL(void,
channel_dump_statistics, (channel_t *chan, int severity))
{
  double avg, interval, age;
  time_t now = time(NULL);
  tor_addr_t remote_addr;
  int have_remote_addr;
  char *remote_addr_str;

  tor_assert(chan);

  age = (double)(now - chan->timestamp_created);

  tor_log(severity, LD_GENERAL,
      "Channel %"PRIu64 " (at %p) with transport %s is in state "
      "%s (%d)",
      (chan->global_identifier), chan,
      channel_describe_transport(chan),
      channel_state_to_string(chan->state), chan->state);
  tor_log(severity, LD_GENERAL,
      " * Channel %"PRIu64 " was created at %"PRIu64
      " (%"PRIu64 " seconds ago) "
      "and last active at %"PRIu64 " (%"PRIu64 " seconds ago)",
      (chan->global_identifier),
      (uint64_t)(chan->timestamp_created),
      (uint64_t)(now - chan->timestamp_created),
      (uint64_t)(chan->timestamp_active),
      (uint64_t)(now - chan->timestamp_active));

  /* Handle digest. */
  if (!tor_digest_is_zero(chan->identity_digest)) {
    tor_log(severity, LD_GENERAL,
        " * Channel %"PRIu64 " says it is connected "
        "to an OR with digest %s",
        (chan->global_identifier),
        hex_str(chan->identity_digest, DIGEST_LEN));
  } else {
    tor_log(severity, LD_GENERAL,
        " * Channel %"PRIu64 " does not know the digest"
        " of the OR it is connected to",
        (chan->global_identifier));
  }

  /* Handle remote address and descriptions */
  have_remote_addr = channel_get_addr_if_possible(chan, &remote_addr);
  if (have_remote_addr) {
    char *actual = tor_strdup(channel_describe_peer(chan));
    remote_addr_str = tor_addr_to_str_dup(&remote_addr);
    tor_log(severity, LD_GENERAL,
        " * Channel %"PRIu64 " says its remote address"
        " is %s, and gives a canonical description of \"%s\" and an "
        "actual description of \"%s\"",
        (chan->global_identifier),
        safe_str(remote_addr_str),
        safe_str(channel_describe_peer(chan)),
        safe_str(actual));
    tor_free(remote_addr_str);
    tor_free(actual);
  } else {
    char *actual = tor_strdup(channel_describe_peer(chan));
    tor_log(severity, LD_GENERAL,
        " * Channel %"PRIu64 " does not know its remote "
        "address, but gives a canonical description of \"%s\" and an "
        "actual description of \"%s\"",
        (chan->global_identifier),
        channel_describe_peer(chan),
        actual);
    tor_free(actual);
  }

  /* Handle marks */
  tor_log(severity, LD_GENERAL,
      " * Channel %"PRIu64 " has these marks: %s %s %s %s %s",
      (chan->global_identifier),
      channel_is_bad_for_new_circs(chan) ?
        "bad_for_new_circs" : "!bad_for_new_circs",
      channel_is_canonical(chan) ?
        "canonical" : "!canonical",
      channel_is_client(chan) ?
        "client" : "!client",
      channel_is_local(chan) ?
        "local" : "!local",
      channel_is_incoming(chan) ?
        "incoming" : "outgoing");

  /* Describe circuits */
  tor_log(severity, LD_GENERAL,
      " * Channel %"PRIu64 " has %d active circuits out of"
      " %d in total",
      (chan->global_identifier),
      (chan->cmux != NULL) ?
         circuitmux_num_active_circuits(chan->cmux) : 0,
      (chan->cmux != NULL) ?
         circuitmux_num_circuits(chan->cmux) : 0);

  /* Describe timestamps */
  if (chan->timestamp_client == 0) {
      tor_log(severity, LD_GENERAL,
              " * Channel %"PRIu64 " was never used by a "
             "client", (chan->global_identifier));
  } else {
      tor_log(severity, LD_GENERAL,
              " * Channel %"PRIu64 " was last used by a "
              "client at %"PRIu64 " (%"PRIu64 " seconds ago)",
              (chan->global_identifier),
              (uint64_t)(chan->timestamp_client),
              (uint64_t)(now - chan->timestamp_client));
  }
  if (chan->timestamp_recv == 0) {
      tor_log(severity, LD_GENERAL,
              " * Channel %"PRIu64 " never received a cell",
              (chan->global_identifier));
  } else {
      tor_log(severity, LD_GENERAL,
              " * Channel %"PRIu64 " last received a cell "
              "at %"PRIu64 " (%"PRIu64 " seconds ago)",
              (chan->global_identifier),
              (uint64_t)(chan->timestamp_recv),
              (uint64_t)(now - chan->timestamp_recv));
  }
  if (chan->timestamp_xmit == 0) {
      tor_log(severity, LD_GENERAL,
              " * Channel %"PRIu64 " never transmitted a cell",
              (chan->global_identifier));
  } else {
      tor_log(severity, LD_GENERAL,
              " * Channel %"PRIu64 " last transmitted a cell "
              "at %"PRIu64 " (%"PRIu64 " seconds ago)",
              (chan->global_identifier),
              (uint64_t)(chan->timestamp_xmit),
              (uint64_t)(now - chan->timestamp_xmit));
  }

  /* Describe counters and rates */
  tor_log(severity, LD_GENERAL,
      " * Channel %"PRIu64 " has received "
      "%"PRIu64 " bytes in %"PRIu64 " cells and transmitted "
      "%"PRIu64 " bytes in %"PRIu64 " cells",
      (chan->global_identifier),
      (chan->n_bytes_recved),
      (chan->n_cells_recved),
      (chan->n_bytes_xmitted),
      (chan->n_cells_xmitted));
  if (now > chan->timestamp_created &&
      chan->timestamp_created > 0) {
    if (chan->n_bytes_recved > 0) {
      avg = (double)(chan->n_bytes_recved) / age;
      tor_log(severity, LD_GENERAL,
          " * Channel %"PRIu64 " has averaged %f "
          "bytes received per second",
          (chan->global_identifier), avg);
    }
    if (chan->n_cells_recved > 0) {
      avg = (double)(chan->n_cells_recved) / age;
      if (avg >= 1.0) {
        tor_log(severity, LD_GENERAL,
            " * Channel %"PRIu64 " has averaged %f "
            "cells received per second",
            (chan->global_identifier), avg);
      } else if (avg >= 0.0) {
        interval = 1.0 / avg;
        tor_log(severity, LD_GENERAL,
            " * Channel %"PRIu64 " has averaged %f "
            "seconds between received cells",
            (chan->global_identifier), interval);
      }
    }
    if (chan->n_bytes_xmitted > 0) {
      avg = (double)(chan->n_bytes_xmitted) / age;
      tor_log(severity, LD_GENERAL,
          " * Channel %"PRIu64 " has averaged %f "
          "bytes transmitted per second",
          (chan->global_identifier), avg);
    }
    if (chan->n_cells_xmitted > 0) {
      avg = (double)(chan->n_cells_xmitted) / age;
      if (avg >= 1.0) {
        tor_log(severity, LD_GENERAL,
            " * Channel %"PRIu64 " has averaged %f "
            "cells transmitted per second",
            (chan->global_identifier), avg);
      } else if (avg >= 0.0) {
        interval = 1.0 / avg;
        tor_log(severity, LD_GENERAL,
            " * Channel %"PRIu64 " has averaged %f "
            "seconds between transmitted cells",
            (chan->global_identifier), interval);
      }
    }
  }

  /* Dump anything the lower layer has to say */
  channel_dump_transport_statistics(chan, severity);
}

/**
 * Dump channel listener statistics.
 *
 * Dump statistics for one channel listener to the log.
 */
void
channel_listener_dump_statistics(channel_listener_t *chan_l, int severity)
{
  double avg, interval, age;
  time_t now = time(NULL);

  tor_assert(chan_l);

  age = (double)(now - chan_l->timestamp_created);

  tor_log(severity, LD_GENERAL,
      "Channel listener %"PRIu64 " (at %p) with transport %s is in "
      "state %s (%d)",
      (chan_l->global_identifier), chan_l,
      channel_listener_describe_transport(chan_l),
      channel_listener_state_to_string(chan_l->state), chan_l->state);
  tor_log(severity, LD_GENERAL,
      " * Channel listener %"PRIu64 " was created at %"PRIu64
      " (%"PRIu64 " seconds ago) "
      "and last active at %"PRIu64 " (%"PRIu64 " seconds ago)",
      (chan_l->global_identifier),
      (uint64_t)(chan_l->timestamp_created),
      (uint64_t)(now - chan_l->timestamp_created),
      (uint64_t)(chan_l->timestamp_active),
      (uint64_t)(now - chan_l->timestamp_active));

  tor_log(severity, LD_GENERAL,
      " * Channel listener %"PRIu64 " last accepted an incoming "
        "channel at %"PRIu64 " (%"PRIu64 " seconds ago) "
        "and has accepted %"PRIu64 " channels in total",
        (chan_l->global_identifier),
        (uint64_t)(chan_l->timestamp_accepted),
        (uint64_t)(now - chan_l->timestamp_accepted),
        (uint64_t)(chan_l->n_accepted));

  /*
   * If it's sensible to do so, get the rate of incoming channels on this
   * listener
   */
  if (now > chan_l->timestamp_created &&
      chan_l->timestamp_created > 0 &&
      chan_l->n_accepted > 0) {
    avg = (double)(chan_l->n_accepted) / age;
    if (avg >= 1.0) {
      tor_log(severity, LD_GENERAL,
          " * Channel listener %"PRIu64 " has averaged %f incoming "
          "channels per second",
          (chan_l->global_identifier), avg);
    } else if (avg >= 0.0) {
      interval = 1.0 / avg;
      tor_log(severity, LD_GENERAL,
          " * Channel listener %"PRIu64 " has averaged %f seconds "
          "between incoming channels",
          (chan_l->global_identifier), interval);
    }
  }

  /* Dump anything the lower layer has to say */
  channel_listener_dump_transport_statistics(chan_l, severity);
}

/**
 * Invoke transport-specific stats dump for channel.
 *
 * If there is a lower-layer statistics dump method, invoke it.
 */
void
channel_dump_transport_statistics(channel_t *chan, int severity)
{
  tor_assert(chan);

  if (chan->dumpstats) chan->dumpstats(chan, severity);
}

/**
 * Invoke transport-specific stats dump for channel listener.
 *
 * If there is a lower-layer statistics dump method, invoke it.
 */
void
channel_listener_dump_transport_statistics(channel_listener_t *chan_l,
                                           int severity)
{
  tor_assert(chan_l);

  if (chan_l->dumpstats) chan_l->dumpstats(chan_l, severity);
}

/**
 * Return text description of the remote endpoint canonical address.
 *
 * This function returns a human-readable string for logging; nothing
 * should parse it or rely on a particular format.
 *
 * Subsequent calls to this function may invalidate its return value.
 */
MOCK_IMPL(const char *,
channel_describe_peer,(channel_t *chan))
{
  tor_assert(chan);
  tor_assert(chan->describe_peer);

  return chan->describe_peer(chan);
}

/**
 * Get the remote address for this channel, if possible.
 *
 * Write the remote address out to a tor_addr_t if the underlying transport
 * supports this operation, and return 1.  Return 0 if the underlying transport
 * doesn't let us do this.
 *
 * Always returns the "real" address of the peer -- the one we're connected to
 * on the internet.
 */
MOCK_IMPL(int,
channel_get_addr_if_possible,(const channel_t *chan,
                              tor_addr_t *addr_out))
{
  tor_assert(chan);
  tor_assert(addr_out);
  tor_assert(chan->get_remote_addr);

  return chan->get_remote_addr(chan, addr_out);
}

/**
 * Return true iff the channel has any cells on the connection outbuf waiting
 * to be sent onto the network.
 */
int
channel_has_queued_writes(channel_t *chan)
{
  tor_assert(chan);
  tor_assert(chan->has_queued_writes);

  /* Check with the lower layer */
  return chan->has_queued_writes(chan);
}

/**
 * Check the is_bad_for_new_circs flag.
 *
 * This function returns the is_bad_for_new_circs flag of the specified
 * channel.
 */
int
channel_is_bad_for_new_circs(channel_t *chan)
{
  tor_assert(chan);

  return chan->is_bad_for_new_circs;
}

/**
 * Mark a channel as bad for new circuits.
 *
 * Set the is_bad_for_new_circs_flag on chan.
 */
void
channel_mark_bad_for_new_circs(channel_t *chan)
{
  tor_assert(chan);

  chan->is_bad_for_new_circs = 1;
}

/**
 * Get the client flag.
 *
 * This returns the client flag of a channel, which will be set if
 * command_process_create_cell() in command.c thinks this is a connection
 * from a client.
 */
int
channel_is_client(const channel_t *chan)
{
  tor_assert(chan);

  return chan->is_client;
}

/**
 * Set the client flag.
 *
 * Mark a channel as being from a client.
 */
void
channel_mark_client(channel_t *chan)
{
  tor_assert(chan);

  chan->is_client = 1;
}

/**
 * Clear the client flag.
 *
 * Mark a channel as being _not_ from a client.
 */
void
channel_clear_client(channel_t *chan)
{
  tor_assert(chan);

  chan->is_client = 0;
}

/**
 * Get the canonical flag for a channel.
 *
 * This returns the is_canonical for a channel; this flag is determined by
 * the lower layer and can't be set in a transport-independent way.
 */
int
channel_is_canonical(channel_t *chan)
{
  tor_assert(chan);
  tor_assert(chan->is_canonical);

  return chan->is_canonical(chan);
}

/**
 * Test incoming flag.
 *
 * This function gets the incoming flag; this is set when a listener spawns
 * a channel.  If this returns true the channel was remotely initiated.
 */
int
channel_is_incoming(channel_t *chan)
{
  tor_assert(chan);

  return chan->is_incoming;
}

/**
 * Set the incoming flag.
 *
 * This function is called when a channel arrives on a listening channel
 * to mark it as incoming.
 */
void
channel_mark_incoming(channel_t *chan)
{
  tor_assert(chan);

  chan->is_incoming = 1;
}

/**
 * Test local flag.
 *
 * This function gets the local flag; the lower layer should set this when
 * setting up the channel if is_local_addr() is true for all of the
 * destinations it will communicate with on behalf of this channel.  It's
 * used to decide whether to declare the network reachable when seeing incoming
 * traffic on the channel.
 */
int
channel_is_local(channel_t *chan)
{
  tor_assert(chan);

  return chan->is_local;
}

/**
 * Set the local flag.
 *
 * This internal-only function should be called by the lower layer if the
 * channel is to a local address.  See channel_is_local() above or the
 * description of the is_local bit in channel.h.
 */
void
channel_mark_local(channel_t *chan)
{
  tor_assert(chan);

  chan->is_local = 1;
}

/**
 * Mark a channel as remote.
 *
 * This internal-only function should be called by the lower layer if the
 * channel is not to a local address but has previously been marked local.
 * See channel_is_local() above or the description of the is_local bit in
 * channel.h
 */
void
channel_mark_remote(channel_t *chan)
{
  tor_assert(chan);

  chan->is_local = 0;
}

/**
 * Test outgoing flag.
 *
 * This function gets the outgoing flag; this is the inverse of the incoming
 * bit set when a listener spawns a channel.  If this returns true the channel
 * was locally initiated.
 */
int
channel_is_outgoing(channel_t *chan)
{
  tor_assert(chan);

  return !(chan->is_incoming);
}

/**
 * Mark a channel as outgoing.
 *
 * This function clears the incoming flag and thus marks a channel as
 * outgoing.
 */
void
channel_mark_outgoing(channel_t *chan)
{
  tor_assert(chan);

  chan->is_incoming = 0;
}

/************************
 * Flow control queries *
 ***********************/

/**
 * Estimate the number of writeable cells.
 *
 * Ask the lower layer for an estimate of how many cells it can accept.
 */
int
channel_num_cells_writeable(channel_t *chan)
{
  int result;

  tor_assert(chan);
  tor_assert(chan->num_cells_writeable);

  if (chan->state == CHANNEL_STATE_OPEN) {
    /* Query lower layer */
    result = chan->num_cells_writeable(chan);
    if (result < 0) result = 0;
  } else {
    /* No cells are writeable in any other state */
    result = 0;
  }

  return result;
}

/*********************
 * Timestamp updates *
 ********************/

/**
 * Update the created timestamp for a channel.
 *
 * This updates the channel's created timestamp and should only be called
 * from channel_init().
 */
void
channel_timestamp_created(channel_t *chan)
{
  time_t now = time(NULL);

  tor_assert(chan);

  chan->timestamp_created = now;
}

/**
 * Update the created timestamp for a channel listener.
 *
 * This updates the channel listener's created timestamp and should only be
 * called from channel_init_listener().
 */
void
channel_listener_timestamp_created(channel_listener_t *chan_l)
{
  time_t now = time(NULL);

  tor_assert(chan_l);

  chan_l->timestamp_created = now;
}

/**
 * Update the last active timestamp for a channel.
 *
 * This function updates the channel's last active timestamp; it should be
 * called by the lower layer whenever there is activity on the channel which
 * does not lead to a cell being transmitted or received; the active timestamp
 * is also updated from channel_timestamp_recv() and channel_timestamp_xmit(),
 * but it should be updated for things like the v3 handshake and stuff that
 * produce activity only visible to the lower layer.
 */
void
channel_timestamp_active(channel_t *chan)
{
  time_t now = time(NULL);

  tor_assert(chan);
  monotime_coarse_get(&chan->timestamp_xfer);

  chan->timestamp_active = now;

  /* Clear any potential netflow padding timer. We're active */
  monotime_coarse_zero(&chan->next_padding_time);
}

/**
 * Update the last active timestamp for a channel listener.
 */
void
channel_listener_timestamp_active(channel_listener_t *chan_l)
{
  time_t now = time(NULL);

  tor_assert(chan_l);

  chan_l->timestamp_active = now;
}

/**
 * Update the last accepted timestamp.
 *
 * This function updates the channel listener's last accepted timestamp; it
 * should be called whenever a new incoming channel is accepted on a
 * listener.
 */
void
channel_listener_timestamp_accepted(channel_listener_t *chan_l)
{
  time_t now = time(NULL);

  tor_assert(chan_l);

  chan_l->timestamp_active = now;
  chan_l->timestamp_accepted = now;
}

/**
 * Update client timestamp.
 *
 * This function is called by relay.c to timestamp a channel that appears to
 * be used as a client.
 */
void
channel_timestamp_client(channel_t *chan)
{
  time_t now = time(NULL);

  tor_assert(chan);

  chan->timestamp_client = now;
}

/**
 * Update the recv timestamp.
 *
 * This is called whenever we get an incoming cell from the lower layer.
 * This also updates the active timestamp.
 */
void
channel_timestamp_recv(channel_t *chan)
{
  time_t now = time(NULL);
  tor_assert(chan);
  monotime_coarse_get(&chan->timestamp_xfer);

  chan->timestamp_active = now;
  chan->timestamp_recv = now;

  /* Clear any potential netflow padding timer. We're active */
  monotime_coarse_zero(&chan->next_padding_time);
}

/**
 * Update the xmit timestamp.
 *
 * This is called whenever we pass an outgoing cell to the lower layer.  This
 * also updates the active timestamp.
 */
void
channel_timestamp_xmit(channel_t *chan)
{
  time_t now = time(NULL);
  tor_assert(chan);

  monotime_coarse_get(&chan->timestamp_xfer);

  chan->timestamp_active = now;
  chan->timestamp_xmit = now;

  /* Clear any potential netflow padding timer. We're active */
  monotime_coarse_zero(&chan->next_padding_time);
}

/***************************************************************
 * Timestamp queries - see above for definitions of timestamps *
 **************************************************************/

/**
 * Query created timestamp for a channel.
 */
time_t
channel_when_created(channel_t *chan)
{
  tor_assert(chan);

  return chan->timestamp_created;
}

/**
 * Query client timestamp.
 */
time_t
channel_when_last_client(channel_t *chan)
{
  tor_assert(chan);

  return chan->timestamp_client;
}

/**
 * Query xmit timestamp.
 */
time_t
channel_when_last_xmit(channel_t *chan)
{
  tor_assert(chan);

  return chan->timestamp_xmit;
}

/**
 * Check if a channel matches an extend_info_t.
 *
 * This function calls the lower layer and asks if this channel matches a
 * given extend_info_t.
 *
 * NOTE that this function only checks for an address/port match, and should
 * be used only when no identity is available.
 */
int
channel_matches_extend_info(channel_t *chan, extend_info_t *extend_info)
{
  tor_assert(chan);
  tor_assert(chan->matches_extend_info);
  tor_assert(extend_info);

  return chan->matches_extend_info(chan, extend_info);
}

/**
 * Check if a channel matches the given target IPv4 or IPv6 addresses.
 * If either address matches, return true. If neither address matches,
 * return false.
 *
 * Both addresses can't be NULL.
 *
 * This function calls into the lower layer and asks if this channel thinks
 * it matches the target addresses for circuit extension purposes.
 */
STATIC bool
channel_matches_target_addr_for_extend(channel_t *chan,
                                       const tor_addr_t *target_ipv4_addr,
                                       const tor_addr_t *target_ipv6_addr)
{
  tor_assert(chan);
  tor_assert(chan->matches_target);

  IF_BUG_ONCE(!target_ipv4_addr && !target_ipv6_addr)
    return false;

  if (target_ipv4_addr && chan->matches_target(chan, target_ipv4_addr))
    return true;

  if (target_ipv6_addr && chan->matches_target(chan, target_ipv6_addr))
    return true;

  return false;
}

/**
 * Return the total number of circuits used by a channel.
 *
 * @param chan Channel to query
 * @return Number of circuits using this as n_chan or p_chan
 */
unsigned int
channel_num_circuits(channel_t *chan)
{
  tor_assert(chan);

  return chan->num_n_circuits +
         chan->num_p_circuits;
}

/**
 * Set up circuit ID generation.
 *
 * This is called when setting up a channel and replaces the old
 * connection_or_set_circid_type().
 */
MOCK_IMPL(void,
channel_set_circid_type,(channel_t *chan,
                         crypto_pk_t *identity_rcvd,
                         int consider_identity))
{
  int started_here;
  crypto_pk_t *our_identity;

  tor_assert(chan);

  started_here = channel_is_outgoing(chan);

  if (! consider_identity) {
    if (started_here)
      chan->circ_id_type = CIRC_ID_TYPE_HIGHER;
    else
      chan->circ_id_type = CIRC_ID_TYPE_LOWER;
    return;
  }

  our_identity = started_here ?
    get_tlsclient_identity_key() : get_server_identity_key();

  if (identity_rcvd) {
    if (crypto_pk_cmp_keys(our_identity, identity_rcvd) < 0) {
      chan->circ_id_type = CIRC_ID_TYPE_LOWER;
    } else {
      chan->circ_id_type = CIRC_ID_TYPE_HIGHER;
    }
  } else {
    chan->circ_id_type = CIRC_ID_TYPE_NEITHER;
  }
}

static int
channel_sort_by_ed25519_identity(const void **a_, const void **b_)
{
  const channel_t *a = *a_,
                  *b = *b_;
  return fast_memcmp(&a->ed25519_identity.pubkey,
                     &b->ed25519_identity.pubkey,
                     sizeof(a->ed25519_identity.pubkey));
}

/** Helper for channel_update_bad_for_new_circs(): Perform the
 * channel_update_bad_for_new_circs operation on all channels in <b>lst</b>,
 * all of which MUST have the same RSA ID.  (They MAY have different
 * Ed25519 IDs.) */
static void
channel_rsa_id_group_set_badness(struct channel_list_t *lst, int force)
{
  /*XXXX This function should really be about channels. 15056 */
  channel_t *chan = TOR_LIST_FIRST(lst);

  if (!chan)
    return;

  /* if there is only one channel, don't bother looping */
  if (PREDICT_LIKELY(!TOR_LIST_NEXT(chan, next_with_same_id))) {
    connection_or_single_set_badness_(
            time(NULL), BASE_CHAN_TO_TLS(chan)->conn, force);
    return;
  }

  smartlist_t *channels = smartlist_new();

  TOR_LIST_FOREACH(chan, lst, next_with_same_id) {
    if (BASE_CHAN_TO_TLS(chan)->conn) {
      smartlist_add(channels, chan);
    }
  }

  smartlist_sort(channels, channel_sort_by_ed25519_identity);

  const ed25519_public_key_t *common_ed25519_identity = NULL;
  /* it would be more efficient to do a slice, but this case is rare */
  smartlist_t *or_conns = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(channels, channel_t *, channel) {
    tor_assert(channel); // Suppresses some compiler warnings.

    if (!common_ed25519_identity)
      common_ed25519_identity = &channel->ed25519_identity;

    if (! ed25519_pubkey_eq(&channel->ed25519_identity,
                            common_ed25519_identity)) {
        connection_or_group_set_badness_(or_conns, force);
        smartlist_clear(or_conns);
        common_ed25519_identity = &channel->ed25519_identity;
    }

    smartlist_add(or_conns, BASE_CHAN_TO_TLS(channel)->conn);
  } SMARTLIST_FOREACH_END(channel);

  connection_or_group_set_badness_(or_conns, force);

  /* XXXX 15056 we may want to do something special with connections that have
   * no set Ed25519 identity! */

  smartlist_free(or_conns);
  smartlist_free(channels);
}

/** Go through all the channels (or if <b>digest</b> is non-NULL, just
 * the OR connections with that digest), and set the is_bad_for_new_circs
 * flag based on the rules in connection_or_group_set_badness() (or just
 * always set it if <b>force</b> is true).
 */
void
channel_update_bad_for_new_circs(const char *digest, int force)
{
  if (digest) {
    channel_idmap_entry_t *ent;
    channel_idmap_entry_t search;
    memset(&search, 0, sizeof(search));
    memcpy(search.digest, digest, DIGEST_LEN);
    ent = HT_FIND(channel_idmap, &channel_identity_map, &search);
    if (ent) {
      channel_rsa_id_group_set_badness(&ent->channel_list, force);
    }
    return;
  }

  /* no digest; just look at everything. */
  channel_idmap_entry_t **iter;
  HT_FOREACH(iter, channel_idmap, &channel_identity_map) {
    channel_rsa_id_group_set_badness(&(*iter)->channel_list, force);
  }
}
