/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dispatch_core.c
 * \brief Core module for sending and receiving messages.
 */

#define DISPATCH_PRIVATE
#include "orconfig.h"

#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_st.h"
#include "lib/dispatch/dispatch_naming.h"

#include "lib/malloc/malloc.h"
#include "lib/log/util_bug.h"

#include <string.h>

/**
 * Use <b>d</b> to drop all storage held for <b>msg</b>.
 *
 * (We need the dispatcher so we know how to free the auxiliary data.)
 **/
void
dispatch_free_msg_(const dispatch_t *d, msg_t *msg)
{
  if (!msg)
    return;

  d->typefns[msg->type].free_fn(msg->aux_data__);
  tor_free(msg);
}

/**
 * Format the auxiliary data held by msg.
 **/
char *
dispatch_fmt_msg_data(const dispatch_t *d, const msg_t *msg)
{
  if (!msg)
    return NULL;

  return d->typefns[msg->type].fmt_fn(msg->aux_data__);
}

/**
 * Release all storage held by <b>d</b>.
 **/
void
dispatch_free_(dispatch_t *d)
{
  if (d == NULL)
    return;

  size_t n_queues = d->n_queues;
  for (size_t i = 0; i < n_queues; ++i) {
    msg_t *m, *mtmp;
    TOR_SIMPLEQ_FOREACH_SAFE(m, &d->queues[i].queue, next, mtmp) {
      dispatch_free_msg(d, m);
    }
  }

  size_t n_msgs = d->n_msgs;

  for (size_t i = 0; i < n_msgs; ++i) {
    tor_free(d->table[i]);
  }
  tor_free(d->table);
  tor_free(d->typefns);
  tor_free(d->queues);

  // This is the only time we will treat d->cfg as non-const.
  //dispatch_cfg_free_((dispatch_items_t *) d->cfg);

  tor_free(d);
}

/**
 * Tell the dispatcher to call <b>fn</b> with <b>userdata</b> whenever
 * <b>chan</b> becomes nonempty.  Return 0 on success, -1 on error.
 **/
int
dispatch_set_alert_fn(dispatch_t *d, channel_id_t chan,
                      dispatch_alertfn_t fn, void *userdata)
{
  if (BUG(chan >= d->n_queues))
    return -1;

  dqueue_t *q = &d->queues[chan];
  q->alert_fn = fn;
  q->alert_fn_arg = userdata;
  return 0;
}

/**
 * Send a message on the appropriate channel notifying that channel if
 * necessary.
 *
 * This function takes ownership of the auxiliary data; it can't be static or
 * stack-allocated, and the caller is not allowed to use it afterwards.
 *
 * This function does not check the various vields of the message object for
 * consistency.
 **/
int
dispatch_send(dispatch_t *d,
              subsys_id_t sender,
              channel_id_t channel,
              message_id_t msg,
              msg_type_id_t type,
              msg_aux_data_t auxdata)
{
  if (!d->table[msg]) {
    /* Fast path: nobody wants this data. */

    d->typefns[type].free_fn(auxdata);
    return 0;
  }

  msg_t *m = tor_malloc(sizeof(msg_t));

  m->sender = sender;
  m->channel = channel;
  m->msg = msg;
  m->type = type;
  memcpy(&m->aux_data__, &auxdata, sizeof(msg_aux_data_t));

  return dispatch_send_msg(d, m);
}

int
dispatch_send_msg(dispatch_t *d, msg_t *m)
{
  if (BUG(!d))
    goto err;
  if (BUG(!m))
    goto err;
  if (BUG(m->channel >= d->n_queues))
    goto err;
  if (BUG(m->msg >= d->n_msgs))
    goto err;

  dtbl_entry_t *ent = d->table[m->msg];
  if (ent) {
    if (BUG(m->type != ent->type))
      goto err;
    if (BUG(m->channel != ent->channel))
      goto err;
  }

  return dispatch_send_msg_unchecked(d, m);
 err:
  /* Probably it isn't safe to free m, since type could be wrong. */
  return -1;
}

/**
 * Send a message on the appropriate queue, notifying that queue if necessary.
 *
 * This function takes ownership of the message object and its auxiliary data;
 * it can't be static or stack-allocated, and the caller isn't allowed to use
 * it afterwards.
 *
 * This function does not check the various fields of the message object for
 * consistency, and can crash if they are out of range.  Only functions that
 * have already constructed the message in a safe way, or checked it for
 * correctness themselves, should call this function.
 **/
int
dispatch_send_msg_unchecked(dispatch_t *d, msg_t *m)
{
  /* Find the right queue. */
  dqueue_t *q = &d->queues[m->channel];
  bool was_empty = TOR_SIMPLEQ_EMPTY(&q->queue);

  /* Append the message. */
  TOR_SIMPLEQ_INSERT_TAIL(&q->queue, m, next);

  if (debug_logging_enabled()) {
    char *arg = dispatch_fmt_msg_data(d, m);
    log_debug(LD_MESG,
              "Queued: %s (%s) from %s, on %s.",
              get_message_id_name(m->msg),
              arg,
              get_subsys_id_name(m->sender),
              get_channel_id_name(m->channel));
    tor_free(arg);
  }

  /* If we just made the queue nonempty for the first time, call the alert
   * function. */
  if (was_empty) {
    q->alert_fn(d, m->channel, q->alert_fn_arg);
  }

  return 0;
}

/**
 * Run all of the callbacks on <b>d</b> associated with <b>m</b>.
 **/
static void
dispatcher_run_msg_cbs(const dispatch_t *d, msg_t *m)
{
  tor_assert(m->msg <= d->n_msgs);
  dtbl_entry_t *ent = d->table[m->msg];
  int n_fns = ent->n_fns;

  if (debug_logging_enabled()) {
    char *arg = dispatch_fmt_msg_data(d, m);
    log_debug(LD_MESG,
              "Delivering: %s (%s) from %s, on %s:",
              get_message_id_name(m->msg),
              arg,
              get_subsys_id_name(m->sender),
              get_channel_id_name(m->channel));
    tor_free(arg);
  }

  int i;
  for (i=0; i < n_fns; ++i) {
    if (ent->rcv[i].enabled) {
      log_debug(LD_MESG, "  Delivering to %s.",
                get_subsys_id_name(ent->rcv[i].sys));
      ent->rcv[i].fn(m);
    }
  }
}

/**
 * Run up to <b>max_msgs</b> callbacks for messages on the channel <b>ch</b>
 * on the given dispatcher.  Return 0 on success or recoverable failure,
 * -1 on unrecoverable error.
 **/
int
dispatch_flush(dispatch_t *d, channel_id_t ch, int max_msgs)
{
  if (BUG(ch >= d->n_queues))
    return 0;

  int n_flushed = 0;
  dqueue_t *q = &d->queues[ch];

  while (n_flushed < max_msgs) {
    msg_t *m = TOR_SIMPLEQ_FIRST(&q->queue);
    if (!m)
      break;
    TOR_SIMPLEQ_REMOVE_HEAD(&q->queue, next);
    dispatcher_run_msg_cbs(d, m);
    dispatch_free_msg(d, m);
    ++n_flushed;
  }

  return 0;
}
