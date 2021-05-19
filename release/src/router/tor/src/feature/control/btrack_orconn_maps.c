/* Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file btrack_orconn_maps.c
 * \brief Hash map implementation for btrack_orconn.c
 *
 * These functions manipulate the hash maps that contain bt_orconn
 * objects.
 **/

#include <stdbool.h>

#include "core/or/or.h"

#include "ht.h"
#include "siphash.h"

#define BTRACK_ORCONN_PRIVATE

#include "feature/control/btrack_orconn.h"
#include "feature/control/btrack_orconn_maps.h"
#include "lib/log/log.h"

static inline unsigned int
bto_gid_hash_(bt_orconn_t *elm)
{
  return (unsigned)siphash24g(&elm->gid, sizeof(elm->gid));
}

static inline int
bto_gid_eq_(bt_orconn_t *a, bt_orconn_t *b)
{
  return a->gid == b->gid;
}

static inline unsigned int
bto_chan_hash_(bt_orconn_t *elm)
{
  return (unsigned)siphash24g(&elm->chan, sizeof(elm->chan));
}

static inline int
bto_chan_eq_(bt_orconn_t *a, bt_orconn_t *b)
{
  return a->chan == b->chan;
}

HT_HEAD(bto_gid_ht, bt_orconn_t);
HT_PROTOTYPE(bto_gid_ht, bt_orconn_t, node, bto_gid_hash_, bto_gid_eq_);
HT_GENERATE2(bto_gid_ht, bt_orconn_t, node,
             bto_gid_hash_, bto_gid_eq_, 0.6,
             tor_reallocarray_, tor_free_);
static struct bto_gid_ht *bto_gid_map;

HT_HEAD(bto_chan_ht, bt_orconn_t);
HT_PROTOTYPE(bto_chan_ht, bt_orconn_t, chan_node, bto_chan_hash_,
             bto_chan_eq_);
HT_GENERATE2(bto_chan_ht, bt_orconn_t, chan_node,
             bto_chan_hash_, bto_chan_eq_, 0.6,
             tor_reallocarray_, tor_free_);
static struct bto_chan_ht *bto_chan_map;

/** Clear the GID hash map, freeing any bt_orconn_t objects that become
 * unreferenced */
static void
bto_gid_clear_map(void)
{
  bt_orconn_t **elt, **next, *c;

  for (elt = HT_START(bto_gid_ht, bto_gid_map);
       elt;
       elt = next) {
    c = *elt;
    next = HT_NEXT_RMV(bto_gid_ht, bto_gid_map, elt);

    c->gid = 0;
    /* Don't delete if chan ID isn't zero: it's still in the chan hash map */
    if (!c->chan)
      tor_free(c);
  }
  HT_CLEAR(bto_gid_ht, bto_gid_map);
  tor_free(bto_gid_map);
}

/** Clear the chan ID hash map, freeing any bt_orconn_t objects that
 * become unreferenced */
static void
bto_chan_clear_map(void)
{
  bt_orconn_t **elt, **next, *c;

  for (elt = HT_START(bto_chan_ht, bto_chan_map);
       elt;
       elt = next) {
    c = *elt;
    next = HT_NEXT_RMV(bto_chan_ht, bto_chan_map, elt);

    c->chan = 0;
    /* Don't delete if GID isn't zero, it's still in the GID hash map */
    if (!c->gid)
      tor_free(c);
  }
  HT_CLEAR(bto_chan_ht, bto_chan_map);
  tor_free(bto_chan_map);
}

/** Delete a bt_orconn from the hash maps by GID */
void
bto_delete(uint64_t gid)
{
  bt_orconn_t key, *bto;

  key.gid = gid;
  key.chan = 0;
  bto = HT_FIND(bto_gid_ht, bto_gid_map, &key);
  if (!bto) {
    /* The orconn might be unregistered because it's an EXT_OR_CONN? */
    log_debug(LD_BTRACK, "tried to delete unregistered ORCONN gid=%"PRIu64,
              gid);
    return;
  }
  HT_REMOVE(bto_gid_ht, bto_gid_map, &key);
  if (bto->chan) {
    key.chan = bto->chan;
    HT_REMOVE(bto_chan_ht, bto_chan_map, &key);
  }
  tor_free(bto);
}

/**
 * Helper for bto_find_or_new().
 *
 * Update GID and chan ID of an existing bt_orconn object if needed,
 * given a search key previously used within bto_find_or_new().
 **/
static bt_orconn_t *
bto_update(bt_orconn_t *bto, const bt_orconn_t *key)
{
  /* ORCONN GIDs shouldn't change once assigned */
  tor_assert(!bto->gid || !key->gid || bto->gid == key->gid);
  if (!bto->gid && key->gid) {
    /* Got a gid when we didn't already have one; insert into gid map */
    log_debug(LD_BTRACK, "ORCONN chan=%"PRIu64" newgid=%"PRIu64, key->chan,
              key->gid);
    bto->gid = key->gid;
    HT_INSERT(bto_gid_ht, bto_gid_map, bto);
  }
  /* association of ORCONN with channel shouldn't change */
  tor_assert(!bto->chan || !key->chan || bto->chan == key->chan);
  if (!bto->chan && key->chan) {
    /* Got a chan when we didn't already have one; insert into chan map */
    log_debug(LD_BTRACK, "ORCONN gid=%"PRIu64" newchan=%"PRIu64,
              bto->gid, key->chan);
    bto->chan = key->chan;
    HT_INSERT(bto_chan_ht, bto_chan_map, bto);
  }
  return bto;
}

/** Helper for bto_find_or_new() */
static bt_orconn_t *
bto_new(const bt_orconn_t *key)
{
  struct bt_orconn_t *bto = tor_malloc(sizeof(*bto));

  bto->gid = key->gid;
  bto->chan = key->chan;
  bto->state = 0;
  bto->proxy_type = 0;
  bto->is_orig = false;
  bto->is_onehop = true;

  if (bto->gid)
    HT_INSERT(bto_gid_ht, bto_gid_map, bto);
  if (bto->chan)
    HT_INSERT(bto_chan_ht, bto_chan_map, bto);

  return bto;
}

/**
 * Insert a new bt_orconn with the given GID and chan ID, or update
 * the GID and chan ID if one already exists.
 *
 * Return the found or allocated bt_orconn.
 **/
bt_orconn_t *
bto_find_or_new(uint64_t gid, uint64_t chan)
{
  bt_orconn_t key, *bto = NULL;

  tor_assert(gid || chan);
  key.gid = gid;
  key.chan = chan;
  if (key.gid)
    bto = HT_FIND(bto_gid_ht, bto_gid_map, &key);
  if (!bto && key.chan) {
    /* Not found by GID; look up by chan ID */
    bto = HT_FIND(bto_chan_ht, bto_chan_map, &key);
  }
  if (bto)
    return bto_update(bto, &key);
  else
    return bto_new(&key);
}

/** Initialize the hash maps  */
void
bto_init_maps(void)
{
  bto_gid_map = tor_malloc(sizeof(*bto_gid_map));
  HT_INIT(bto_gid_ht, bto_gid_map);
  bto_chan_map = tor_malloc(sizeof(*bto_chan_map));
  HT_INIT(bto_chan_ht, bto_chan_map);
}

/** Clear the hash maps, freeing all associated storage */
void
bto_clear_maps(void)
{
  bto_gid_clear_map();
  bto_chan_clear_map();
}
