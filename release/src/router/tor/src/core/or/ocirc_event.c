/* Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file ocirc_event.c
 * \brief Publish state change messages for origin circuits
 *
 * Implements a basic publish-subscribe framework for messages about
 * the state of origin circuits.  The publisher calls the subscriber
 * callback functions synchronously.
 *
 * Although the synchronous calls might not simplify the call graph,
 * this approach improves data isolation because the publisher doesn't
 * need knowledge about the internals of subscribing subsystems.  It
 * also avoids race conditions that might occur in asynchronous
 * frameworks.
 **/

#include "core/or/or.h"

#define OCIRC_EVENT_PRIVATE

#include "core/or/cpath_build_state_st.h"
#include "core/or/ocirc_event.h"
#include "core/or/or_sys.h"
#include "core/or/origin_circuit_st.h"
#include "lib/subsys/subsys.h"

DECLARE_PUBLISH(ocirc_state);
DECLARE_PUBLISH(ocirc_chan);
DECLARE_PUBLISH(ocirc_cevent);

static void
ocirc_event_free(msg_aux_data_t u)
{
  tor_free_(u.ptr);
}

static char *
ocirc_state_fmt(msg_aux_data_t u)
{
  ocirc_state_msg_t *msg = (ocirc_state_msg_t *)u.ptr;
  char *s = NULL;

  tor_asprintf(&s, "<gid=%"PRIu32" state=%d onehop=%d>",
               msg->gid, msg->state, msg->onehop);
  return s;
}

static char *
ocirc_chan_fmt(msg_aux_data_t u)
{
  ocirc_chan_msg_t *msg = (ocirc_chan_msg_t *)u.ptr;
  char *s = NULL;

  tor_asprintf(&s, "<gid=%"PRIu32" chan=%"PRIu64" onehop=%d>",
               msg->gid, msg->chan, msg->onehop);
  return s;
}

static char *
ocirc_cevent_fmt(msg_aux_data_t u)
{
  ocirc_cevent_msg_t *msg = (ocirc_cevent_msg_t *)u.ptr;
  char *s = NULL;

  tor_asprintf(&s, "<gid=%"PRIu32" evtype=%d reason=%d onehop=%d>",
               msg->gid, msg->evtype, msg->reason, msg->onehop);
  return s;
}

static dispatch_typefns_t ocirc_state_fns = {
  .free_fn = ocirc_event_free,
  .fmt_fn = ocirc_state_fmt,
};

static dispatch_typefns_t ocirc_chan_fns = {
  .free_fn = ocirc_event_free,
  .fmt_fn = ocirc_chan_fmt,
};

static dispatch_typefns_t ocirc_cevent_fns = {
  .free_fn = ocirc_event_free,
  .fmt_fn = ocirc_cevent_fmt,
};

int
ocirc_add_pubsub(struct pubsub_connector_t *connector)
{
  if (DISPATCH_REGISTER_TYPE(connector, ocirc_state, &ocirc_state_fns))
    return -1;
  if (DISPATCH_REGISTER_TYPE(connector, ocirc_chan, &ocirc_chan_fns))
    return -1;
  if (DISPATCH_REGISTER_TYPE(connector, ocirc_cevent, &ocirc_cevent_fns))
    return -1;
  if (DISPATCH_ADD_PUB(connector, ocirc, ocirc_state))
    return -1;
  if (DISPATCH_ADD_PUB(connector, ocirc, ocirc_chan))
    return -1;
  if (DISPATCH_ADD_PUB(connector, ocirc, ocirc_cevent))
    return -1;
  return 0;
}

void
ocirc_state_publish(ocirc_state_msg_t *msg)
{
  PUBLISH(ocirc_state, msg);
}

void
ocirc_chan_publish(ocirc_chan_msg_t *msg)
{
  PUBLISH(ocirc_chan, msg);
}

void
ocirc_cevent_publish(ocirc_cevent_msg_t *msg)
{
  PUBLISH(ocirc_cevent, msg);
}
