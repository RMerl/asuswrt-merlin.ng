/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dispatch_cfg.c
 * \brief Create and configure a dispatch_cfg_t.
 *
 * A dispatch_cfg_t object is used to configure a set of messages and
 * associated information before creating a dispatch_t.
 */

#define DISPATCH_PRIVATE

#include "orconfig.h"
#include "lib/dispatch/dispatch_cfg.h"
#include "lib/dispatch/dispatch_cfg_st.h"
#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_st.h"

#include "lib/container/smartlist.h"
#include "lib/malloc/malloc.h"

/**
 * Create and return a new dispatch_cfg_t.
 **/
dispatch_cfg_t *
dcfg_new(void)
{
  dispatch_cfg_t *cfg = tor_malloc(sizeof(dispatch_cfg_t));
  cfg->type_by_msg = smartlist_new();
  cfg->chan_by_msg = smartlist_new();
  cfg->fns_by_type = smartlist_new();
  cfg->recv_by_msg = smartlist_new();
  return cfg;
}

/**
 * Associate a message with a datatype.  Return 0 on success, -1 if a
 * different type was previously associated with the message ID.
 **/
int
dcfg_msg_set_type(dispatch_cfg_t *cfg, message_id_t msg,
                  msg_type_id_t type)
{
  smartlist_grow(cfg->type_by_msg, msg+1);
  msg_type_id_t *oldval = smartlist_get(cfg->type_by_msg, msg);
  if (oldval != NULL && *oldval != type) {
    return -1;
  }
  if (!oldval)
    smartlist_set(cfg->type_by_msg, msg, tor_memdup(&type, sizeof(type)));
  return 0;
}

/**
 * Associate a message with a channel.  Return 0 on success, -1 if a
 * different channel was previously associated with the message ID.
 **/
int
dcfg_msg_set_chan(dispatch_cfg_t *cfg, message_id_t msg,
                  channel_id_t chan)
{
  smartlist_grow(cfg->chan_by_msg, msg+1);
  channel_id_t *oldval = smartlist_get(cfg->chan_by_msg, msg);
  if (oldval != NULL && *oldval != chan) {
    return -1;
  }
  if (!oldval)
    smartlist_set(cfg->chan_by_msg, msg, tor_memdup(&chan, sizeof(chan)));
  return 0;
}

/**
 * Associate a set of functions with a datatype. Return 0 on success, -1 if
 * different functions were previously associated with the type.
 **/
int
dcfg_type_set_fns(dispatch_cfg_t *cfg, msg_type_id_t type,
                  const dispatch_typefns_t *fns)
{
  smartlist_grow(cfg->fns_by_type, type+1);
  dispatch_typefns_t *oldfns = smartlist_get(cfg->fns_by_type, type);
  if (oldfns && (oldfns->free_fn != fns->free_fn ||
                 oldfns->fmt_fn != fns->fmt_fn))
    return -1;
  if (!oldfns)
    smartlist_set(cfg->fns_by_type, type, tor_memdup(fns, sizeof(*fns)));
  return 0;
}

/**
 * Associate a receiver with a message ID.  Multiple receivers may be
 * associated with a single messasge ID.
 *
 * Return 0 on success, on failure.
 **/
int
dcfg_add_recv(dispatch_cfg_t *cfg, message_id_t msg,
              subsys_id_t sys, recv_fn_t fn)
{
  smartlist_grow(cfg->recv_by_msg, msg+1);
  smartlist_t *receivers = smartlist_get(cfg->recv_by_msg, msg);
  if (!receivers) {
    receivers = smartlist_new();
    smartlist_set(cfg->recv_by_msg, msg, receivers);
  }

  dispatch_rcv_t *rcv = tor_malloc(sizeof(dispatch_rcv_t));
  rcv->sys = sys;
  rcv->enabled = true;
  rcv->fn = fn;
  smartlist_add(receivers, (void*)rcv);
  return 0;
}

/** Helper: release all storage held by <b>cfg</b>. */
void
dcfg_free_(dispatch_cfg_t *cfg)
{
  if (!cfg)
    return;

  SMARTLIST_FOREACH(cfg->type_by_msg, msg_type_id_t *, id, tor_free(id));
  SMARTLIST_FOREACH(cfg->chan_by_msg, channel_id_t *, id, tor_free(id));
  SMARTLIST_FOREACH(cfg->fns_by_type, dispatch_typefns_t *, f, tor_free(f));
  smartlist_free(cfg->type_by_msg);
  smartlist_free(cfg->chan_by_msg);
  smartlist_free(cfg->fns_by_type);
  SMARTLIST_FOREACH_BEGIN(cfg->recv_by_msg, smartlist_t *, receivers) {
    if (!receivers)
      continue;
    SMARTLIST_FOREACH(receivers, dispatch_rcv_t *, rcv, tor_free(rcv));
    smartlist_free(receivers);
  } SMARTLIST_FOREACH_END(receivers);
  smartlist_free(cfg->recv_by_msg);

  tor_free(cfg);
}
