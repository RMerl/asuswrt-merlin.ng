/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dispatch_new.c
 * \brief Code to construct a dispatch_t from a dispatch_cfg_t.
 **/

#define DISPATCH_NEW_PRIVATE
#define DISPATCH_PRIVATE
#include "orconfig.h"

#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_st.h"
#include "lib/dispatch/dispatch_cfg.h"
#include "lib/dispatch/dispatch_cfg_st.h"

#include "lib/cc/ctassert.h"
#include "lib/intmath/cmp.h"
#include "lib/malloc/malloc.h"
#include "lib/log/util_bug.h"

#include <string.h>

/** Given a smartlist full of (possibly NULL) pointers to uint16_t values,
 * return the largest value, or dflt if the list is empty. */
STATIC int
max_in_u16_sl(const smartlist_t *sl, int dflt)
{
  uint16_t *maxptr = NULL;
  SMARTLIST_FOREACH_BEGIN(sl, uint16_t *, u) {
    if (!maxptr)
      maxptr = u;
    else if (u && *u > *maxptr)
      maxptr = u;
  } SMARTLIST_FOREACH_END(u);

  return maxptr ? *maxptr : dflt;
}

/* The above function is only safe to call if we are sure that channel_id_t
 * and msg_type_id_t are really uint16_t.  They should be so defined in
 * msgtypes.h, but let's be extra cautious.
 */
CTASSERT(sizeof(uint16_t) == sizeof(msg_type_id_t));
CTASSERT(sizeof(uint16_t) == sizeof(channel_id_t));

/** Helper: Format an unformattable message auxiliary data item: just return a
* copy of the string <>. */
static char *
type_fmt_nop(msg_aux_data_t arg)
{
  (void)arg;
  return tor_strdup("<>");
}

/** Helper: Free an unfreeable message auxiliary data item: do nothing. */
static void
type_free_nop(msg_aux_data_t arg)
{
  (void)arg;
}

/** Type functions to use when no type functions are provided. */
static dispatch_typefns_t nop_typefns = {
  .free_fn = type_free_nop,
  .fmt_fn = type_fmt_nop
};

/**
 * Alert function to use when none is configured: do nothing.
 **/
static void
alert_fn_nop(dispatch_t *d, channel_id_t ch, void *arg)
{
  (void)d;
  (void)ch;
  (void)arg;
}

/**
 * Given a list of recvfn_t, create and return a new dtbl_entry_t mapping
 * to each of those functions.
 **/
static dtbl_entry_t *
dtbl_entry_from_lst(smartlist_t *receivers)
{
  if (!receivers)
    return NULL;

  size_t n_recv = smartlist_len(receivers);
  dtbl_entry_t *ent;
  ent = tor_malloc_zero(offsetof(dtbl_entry_t, rcv) +
                        sizeof(dispatch_rcv_t) * n_recv);

  ent->n_fns = n_recv;

  SMARTLIST_FOREACH_BEGIN(receivers, const dispatch_rcv_t *, rcv) {
    memcpy(&ent->rcv[rcv_sl_idx], rcv, sizeof(*rcv));
    if (rcv->enabled) {
      ++ent->n_enabled;
    }
  } SMARTLIST_FOREACH_END(rcv);

  return ent;
}

/** Create and return a new dispatcher from a given dispatch_cfg_t. */
dispatch_t *
dispatch_new(const dispatch_cfg_t *cfg)
{
  dispatch_t *d = tor_malloc_zero(sizeof(dispatch_t));

  /* Any message that has a type or a receiver counts towards our messages */
  const size_t n_msgs = MAX(smartlist_len(cfg->type_by_msg),
                            smartlist_len(cfg->recv_by_msg)) + 1;

  /* Any channel that any message has counts towards the number of channels. */
  const size_t n_chans = (size_t)
    MAX(1, max_in_u16_sl(cfg->chan_by_msg,0)) + 1;

  /* Any type that a message has, or that has functions, counts towards
   * the number of types. */
  const size_t n_types = (size_t) MAX(max_in_u16_sl(cfg->type_by_msg,0),
                                      smartlist_len(cfg->fns_by_type)) + 1;

  d->n_msgs = n_msgs;
  d->n_queues = n_chans;
  d->n_types = n_types;

  /* Initialize the array of type-functions. */
  d->typefns = tor_calloc(n_types, sizeof(dispatch_typefns_t));
  for (size_t i = 0; i < n_types; ++i) {
    /* Default to no-op for everything... */
    memcpy(&d->typefns[i], &nop_typefns, sizeof(dispatch_typefns_t));
  }
  SMARTLIST_FOREACH_BEGIN(cfg->fns_by_type, dispatch_typefns_t *, fns) {
    /* Set the functions if they are provided. */
    if (fns) {
      if (fns->free_fn)
        d->typefns[fns_sl_idx].free_fn = fns->free_fn;
      if (fns->fmt_fn)
        d->typefns[fns_sl_idx].fmt_fn = fns->fmt_fn;
    }
  } SMARTLIST_FOREACH_END(fns);

  /* Initialize the message queues: one for each channel. */
  d->queues = tor_calloc(d->n_queues, sizeof(dqueue_t));
  for (size_t i = 0; i < d->n_queues; ++i) {
    TOR_SIMPLEQ_INIT(&d->queues[i].queue);
    d->queues[i].alert_fn = alert_fn_nop;
  }

  /* Build the dispatch tables mapping message IDs to receivers. */
  d->table = tor_calloc(d->n_msgs, sizeof(dtbl_entry_t *));
  SMARTLIST_FOREACH_BEGIN(cfg->recv_by_msg, smartlist_t *, rcv) {
    d->table[rcv_sl_idx] = dtbl_entry_from_lst(rcv);
  } SMARTLIST_FOREACH_END(rcv);

  /* Fill in the empty entries in the dispatch tables:
   * types and channels for each message. */
  SMARTLIST_FOREACH_BEGIN(cfg->type_by_msg, msg_type_id_t *, type) {
    if (d->table[type_sl_idx])
      d->table[type_sl_idx]->type = *type;
  } SMARTLIST_FOREACH_END(type);

  SMARTLIST_FOREACH_BEGIN(cfg->chan_by_msg, channel_id_t *, chan) {
    if (d->table[chan_sl_idx])
      d->table[chan_sl_idx]->channel = *chan;
  } SMARTLIST_FOREACH_END(chan);

  return d;
}
