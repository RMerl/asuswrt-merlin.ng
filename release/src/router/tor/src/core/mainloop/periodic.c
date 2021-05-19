/* Copyright (c) 2015-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file periodic.c
 *
 * \brief Generic backend for handling periodic events.
 *
 * The events in this module are used to track items that need
 * to fire once every N seconds, possibly picking a new interval each time
 * that they fire.  See periodic_events[] in mainloop.c for examples.
 *
 * This module manages a global list of periodic_event_item_t objects,
 * each corresponding to a single event.  To register an event, pass it to
 * periodic_events_register() when initializing your subsystem.
 *
 * Registering an event makes the periodic event subsystem know about it, but
 * doesn't cause the event to get created immediately.  Before the event can
 * be started, periodic_event_connect_all() must be called by mainloop.c to
 * connect all the events to Libevent.
 *
 * We expect that periodic_event_item_t objects will be statically allocated;
 * we set them up and tear them down here, but we don't take ownership of
 * them.
 */

#include "core/or/or.h"
#include "lib/evloop/compat_libevent.h"
#include "app/config/config.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/periodic.h"

/** We disable any interval greater than this number of seconds, on the
 * grounds that it is probably an absolute time mistakenly passed in as a
 * relative time.
 */
static const int MAX_INTERVAL = 10 * 365 * 86400;

/**
 * Global list of periodic events that have been registered with
 * <b>periodic_event_register</b>.
 **/
static smartlist_t *the_periodic_events = NULL;

/** Set the event <b>event</b> to run in <b>next_interval</b> seconds from
 * now. */
static void
periodic_event_set_interval(periodic_event_item_t *event,
                            time_t next_interval)
{
  tor_assert(next_interval < MAX_INTERVAL);
  struct timeval tv;
  tv.tv_sec = next_interval;
  tv.tv_usec = 0;
  mainloop_event_schedule(event->ev, &tv);
}

/** Wraps dispatches for periodic events, <b>data</b> will be a pointer to the
 * event that needs to be called */
static void
periodic_event_dispatch(mainloop_event_t *ev, void *data)
{
  periodic_event_item_t *event = data;
  tor_assert(ev == event->ev);

  time_t now = time(NULL);
  update_current_time(now);
  const or_options_t *options = get_options();
//  log_debug(LD_GENERAL, "Dispatching %s", event->name);
  int r = event->fn(now, options);
  int next_interval = 0;

  if (!periodic_event_is_enabled(event)) {
    /* The event got disabled from inside its callback, or before: no need to
     * reschedule. */
    return;
  }

  /* update the last run time if action was taken */
  if (r==0) {
    log_err(LD_BUG, "Invalid return value for periodic event from %s.",
                      event->name);
    tor_assert(r != 0);
  } else if (r > 0) {
    event->last_action_time = now;
    /* If the event is meant to happen after ten years, that's likely
     * a bug, and somebody gave an absolute time rather than an interval.
     */
    tor_assert(r < MAX_INTERVAL);
    next_interval = r;
  } else {
    /* no action was taken, it is likely a precondition failed,
     * we should reschedule for next second in case the precondition
     * passes then */
    next_interval = 1;
  }

//  log_debug(LD_GENERAL, "Scheduling %s for %d seconds", event->name,
//           next_interval);
  struct timeval tv = { next_interval , 0 };
  mainloop_event_schedule(ev, &tv);
}

/** Schedules <b>event</b> to run as soon as possible from now. */
void
periodic_event_reschedule(periodic_event_item_t *event)
{
  /* Don't reschedule a disabled or uninitialized event. */
  if (event->ev && periodic_event_is_enabled(event)) {
    periodic_event_set_interval(event, 1);
  }
}

/** Connects a periodic event to the Libevent backend.  Does not launch the
 * event immediately. */
void
periodic_event_connect(periodic_event_item_t *event)
{
  if (event->ev) { /* Already setup? This is a bug */
    log_err(LD_BUG, "Initial dispatch should only be done once.");
    tor_assert(0);
  }

  event->ev = mainloop_event_new(periodic_event_dispatch,
                                 event);
  tor_assert(event->ev);
}

/** Handles initial dispatch for periodic events. It should happen 1 second
 * after the events are created to mimic behaviour before #3199's refactor */
void
periodic_event_launch(periodic_event_item_t *event)
{
  if (! event->ev) { /* Not setup? This is a bug */
    log_err(LD_BUG, "periodic_event_launch without periodic_event_connect");
    tor_assert(0);
  }
  /* Event already enabled? This is a bug */
  if (periodic_event_is_enabled(event)) {
    log_err(LD_BUG, "periodic_event_launch on an already enabled event");
    tor_assert(0);
  }

  // Initial dispatch
  event->enabled = 1;
  periodic_event_dispatch(event->ev, event);
}

/** Disconnect and unregister the periodic event in <b>event</b> */
static void
periodic_event_disconnect(periodic_event_item_t *event)
{
  if (!event)
    return;

  /* First disable the event so we first cancel the event and set its enabled
   * flag properly. */
  periodic_event_disable(event);

  mainloop_event_free(event->ev);
  event->last_action_time = 0;
}

/** Enable the given event by setting its "enabled" flag and scheduling it to
 * run immediately in the event loop. This can be called for an event that is
 * already enabled. */
void
periodic_event_enable(periodic_event_item_t *event)
{
  tor_assert(event);
  /* Safely and silently ignore if this event is already enabled. */
  if (periodic_event_is_enabled(event)) {
    return;
  }

  tor_assert(event->ev);
  event->enabled = 1;
  mainloop_event_activate(event->ev);
}

/** Disable the given event which means the event is destroyed and then the
 * event's enabled flag is unset. This can be called for an event that is
 * already disabled. */
void
periodic_event_disable(periodic_event_item_t *event)
{
  tor_assert(event);
  /* Safely and silently ignore if this event is already disabled. */
  if (!periodic_event_is_enabled(event)) {
    return;
  }
  mainloop_event_cancel(event->ev);
  event->enabled = 0;
}

/**
 * Disable an event, then schedule it to run once.
 * Do nothing if the event was already disabled.
 */
void
periodic_event_schedule_and_disable(periodic_event_item_t *event)
{
  tor_assert(event);
  if (!periodic_event_is_enabled(event))
    return;

  periodic_event_disable(event);

  mainloop_event_activate(event->ev);
}

/**
 * Add <b>item</b> to the list of periodic events.
 *
 * Note that <b>item</b> should be statically allocated: we do not
 * take ownership of it.
 **/
void
periodic_events_register(periodic_event_item_t *item)
{
  if (!the_periodic_events)
    the_periodic_events = smartlist_new();

  if (BUG(smartlist_contains(the_periodic_events, item)))
    return;

  smartlist_add(the_periodic_events, item);
}

/**
 * Make all registered periodic events connect to the libevent backend.
 */
void
periodic_events_connect_all(void)
{
  if (! the_periodic_events)
    return;

  SMARTLIST_FOREACH_BEGIN(the_periodic_events, periodic_event_item_t *, item) {
    if (item->ev)
      continue;
    periodic_event_connect(item);
  } SMARTLIST_FOREACH_END(item);
}

/**
 * Reset all the registered periodic events so we'll do all our actions again
 * as if we just started up.
 *
 * Useful if our clock just moved back a long time from the future,
 * so we don't wait until that future arrives again before acting.
 */
void
periodic_events_reset_all(void)
{
  if (! the_periodic_events)
    return;

  SMARTLIST_FOREACH_BEGIN(the_periodic_events, periodic_event_item_t *, item) {
    if (!item->ev)
      continue;

    periodic_event_reschedule(item);
  } SMARTLIST_FOREACH_END(item);
}

/**
 * Return the registered periodic event whose name is <b>name</b>.
 * Return NULL if no such event is found.
 */
periodic_event_item_t *
periodic_events_find(const char *name)
{
  if (! the_periodic_events)
    return NULL;

  SMARTLIST_FOREACH_BEGIN(the_periodic_events, periodic_event_item_t *, item) {
    if (strcmp(name, item->name) == 0)
      return item;
  } SMARTLIST_FOREACH_END(item);
  return NULL;
}

/**
 * Start or stop registered periodic events, depending on our current set of
 * roles.
 *
 * Invoked when our list of roles, or the net_disabled flag has changed.
 **/
void
periodic_events_rescan_by_roles(int roles, bool net_disabled)
{
  if (! the_periodic_events)
    return;

  SMARTLIST_FOREACH_BEGIN(the_periodic_events, periodic_event_item_t *, item) {
    if (!item->ev)
      continue;

    int enable = !!(item->roles & roles);

    /* Handle the event flags. */
    if (net_disabled &&
        (item->flags & PERIODIC_EVENT_FLAG_NEED_NET)) {
      enable = 0;
    }

    /* Enable the event if needed. It is safe to enable an event that was
     * already enabled. Same goes for disabling it. */
    if (enable) {
      log_debug(LD_GENERAL, "Launching periodic event %s", item->name);
      periodic_event_enable(item);
    } else {
      log_debug(LD_GENERAL, "Disabling periodic event %s", item->name);
      if (item->flags & PERIODIC_EVENT_FLAG_RUN_ON_DISABLE) {
        periodic_event_schedule_and_disable(item);
      } else {
        periodic_event_disable(item);
      }
    }
  } SMARTLIST_FOREACH_END(item);
}

/**
 * Invoked at shutdown: disconnect and unregister all periodic events.
 *
 * Does not free the periodic_event_item_t object themselves, because we do
 * not own them.
 */
void
periodic_events_disconnect_all(void)
{
  if (! the_periodic_events)
    return;

  SMARTLIST_FOREACH_BEGIN(the_periodic_events, periodic_event_item_t *, item) {
    periodic_event_disconnect(item);
  } SMARTLIST_FOREACH_END(item);

  smartlist_free(the_periodic_events);
}

#define LONGEST_TIMER_PERIOD (30 * 86400)
/** Helper: Return the number of seconds between <b>now</b> and <b>next</b>,
 * clipped to the range [1 second, LONGEST_TIMER_PERIOD].
 *
 * We use this to answer the question, "how many seconds is it from now until
 * next" in periodic timer callbacks.  Don't use it for other purposes
 **/
int
safe_timer_diff(time_t now, time_t next)
{
  if (next > now) {
    /* There were no computers at signed TIME_MIN (1902 on 32-bit systems),
     * and nothing that could run Tor. It's a bug if 'next' is around then.
     * On 64-bit systems with signed TIME_MIN, TIME_MIN is before the Big
     * Bang. We cannot extrapolate past a singularity, but there was probably
     * nothing that could run Tor then, either.
     **/
    tor_assert(next > TIME_MIN + LONGEST_TIMER_PERIOD);

    if (next - LONGEST_TIMER_PERIOD > now)
      return LONGEST_TIMER_PERIOD;
    return (int)(next - now);
  } else {
    return 1;
  }
}
