/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file mainloop_pubsub.c
 * @brief Connect the publish-subscribe code to the main-loop.
 *
 * This module is responsible for instantiating all the channels used by the
 * publish-subscribe code, and making sure that each one's messages are
 * processed when appropriate.
 **/

#include "orconfig.h"

#include "core/or/or.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/mainloop_pubsub.h"

#include "lib/container/smartlist.h"
#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_naming.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/pubsub/pubsub.h"
#include "lib/pubsub/pubsub_build.h"

/**
 * Dispatcher to use for delivering messages.
 **/
static dispatch_t *the_dispatcher = NULL;
static pubsub_items_t *the_pubsub_items = NULL;
/**
 * A list of mainloop_event_t, indexed by channel ID, to flush the messages
 * on a channel.
 **/
static smartlist_t *alert_events = NULL;

/**
 * Mainloop event callback: flush all the messages in a channel.
 *
 * The channel is encoded as a pointer, and passed via arg.
 **/
static void
flush_channel_event(mainloop_event_t *ev, void *arg)
{
  (void)ev;
  if (!the_dispatcher)
    return;

  channel_id_t chan = (channel_id_t)(uintptr_t)(arg);
  dispatch_flush(the_dispatcher, chan, INT_MAX);
}

/**
 * Construct our global pubsub object from <b>builder</b>. Return 0 on
 * success, -1 on failure. */
int
tor_mainloop_connect_pubsub(struct pubsub_builder_t *builder)
{
  int rv = -1;
  tor_mainloop_disconnect_pubsub();

  the_dispatcher = pubsub_builder_finalize(builder, &the_pubsub_items);
  if (! the_dispatcher)
    goto err;

  rv = 0;
  goto done;
 err:
  tor_mainloop_disconnect_pubsub();
 done:
  return rv;
}

/**
 * Install libevent events for all of the pubsub channels.
 *
 * Invoke this after tor_mainloop_connect_pubsub, and after libevent has been
 * initialized.
 */
void
tor_mainloop_connect_pubsub_events(void)
{
  tor_assert(the_dispatcher);
  tor_assert(! alert_events);

  const size_t num_channels = get_num_channel_ids();
  alert_events = smartlist_new();
  for (size_t i = 0; i < num_channels; ++i) {
    smartlist_add(alert_events,
                  mainloop_event_postloop_new(flush_channel_event,
                                              (void*)(uintptr_t)(i)));
  }
}

/**
 * Dispatch alertfn callback: do nothing. Implements DELIV_NEVER.
 **/
static void
alertfn_never(dispatch_t *d, channel_id_t chan, void *arg)
{
  (void)d;
  (void)chan;
  (void)arg;
}

/**
 * Dispatch alertfn callback: activate a mainloop event. Implements
 * DELIV_PROMPT.
 **/
static void
alertfn_prompt(dispatch_t *d, channel_id_t chan, void *arg)
{
  (void)d;
  (void)chan;
  mainloop_event_t *event = arg;
  mainloop_event_activate(event);
}

/**
 * Dispatch alertfn callback: flush all messages right now. Implements
 * DELIV_IMMEDIATE.
 **/
static void
alertfn_immediate(dispatch_t *d, channel_id_t chan, void *arg)
{
  (void) arg;
  dispatch_flush(d, chan, INT_MAX);
}

/**
 * Set the strategy to be used for delivering messages on the named channel.
 *
 * This function needs to be called once globally for each channel, to
 * set up how messages are delivered.
 **/
int
tor_mainloop_set_delivery_strategy(const char *msg_channel_name,
                                   deliv_strategy_t strategy)
{
  channel_id_t chan = get_channel_id(msg_channel_name);
  if (BUG(chan == ERROR_ID) ||
      BUG(chan >= smartlist_len(alert_events)))
    return -1;

  switch (strategy) {
    case DELIV_NEVER:
      dispatch_set_alert_fn(the_dispatcher, chan, alertfn_never, NULL);
      break;
    case DELIV_PROMPT:
      dispatch_set_alert_fn(the_dispatcher, chan, alertfn_prompt,
                            smartlist_get(alert_events, chan));
      break;
    case DELIV_IMMEDIATE:
      dispatch_set_alert_fn(the_dispatcher, chan, alertfn_immediate, NULL);
      break;
  }
  return 0;
}

/**
 * Remove all pubsub dispatchers and events from the mainloop.
 **/
void
tor_mainloop_disconnect_pubsub(void)
{
  if (the_pubsub_items) {
    pubsub_items_clear_bindings(the_pubsub_items);
    pubsub_items_free(the_pubsub_items);
  }
  if (alert_events) {
    SMARTLIST_FOREACH(alert_events, mainloop_event_t *, ev,
                      mainloop_event_free(ev));
    smartlist_free(alert_events);
  }
  dispatch_free(the_dispatcher);
}
