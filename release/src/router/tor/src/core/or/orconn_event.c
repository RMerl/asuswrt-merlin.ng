/* Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file orconn_event.c
 * \brief Publish state change messages for OR connections
 *
 * Implements a basic publish-subscribe framework for messages about
 * the state of OR connections.  The publisher calls the subscriber
 * callback functions synchronously.
 *
 * Although the synchronous calls might not simplify the call graph,
 * this approach improves data isolation because the publisher doesn't
 * need knowledge about the internals of subscribing subsystems.  It
 * also avoids race conditions that might occur in asynchronous
 * frameworks.
 **/

#include "core/or/or.h"
#include "lib/pubsub/pubsub.h"
#include "lib/subsys/subsys.h"

#define ORCONN_EVENT_PRIVATE
#include "core/or/orconn_event.h"
#include "core/or/or_sys.h"

DECLARE_PUBLISH(orconn_state);
DECLARE_PUBLISH(orconn_status);

static void
orconn_event_free(msg_aux_data_t u)
{
  tor_free_(u.ptr);
}

static char *
orconn_state_fmt(msg_aux_data_t u)
{
  orconn_state_msg_t *msg = (orconn_state_msg_t *)u.ptr;
  char *s = NULL;

  tor_asprintf(&s, "<gid=%"PRIu64" chan=%"PRIu64" proxy_type=%d state=%d>",
               msg->gid, msg->chan, msg->proxy_type, msg->state);
  return s;
}

static char *
orconn_status_fmt(msg_aux_data_t u)
{
  orconn_status_msg_t *msg = (orconn_status_msg_t *)u.ptr;
  char *s = NULL;

  tor_asprintf(&s, "<gid=%"PRIu64" status=%d reason=%d>",
               msg->gid, msg->status, msg->reason);
  return s;
}

static dispatch_typefns_t orconn_state_fns = {
  .free_fn = orconn_event_free,
  .fmt_fn = orconn_state_fmt,
};

static dispatch_typefns_t orconn_status_fns = {
  .free_fn = orconn_event_free,
  .fmt_fn = orconn_status_fmt,
};

int
orconn_add_pubsub(struct pubsub_connector_t *connector)
{
  if (DISPATCH_REGISTER_TYPE(connector, orconn_state, &orconn_state_fns))
    return -1;
  if (DISPATCH_REGISTER_TYPE(connector, orconn_status, &orconn_status_fns))
    return -1;
  if (DISPATCH_ADD_PUB(connector, orconn, orconn_state) != 0)
    return -1;
  if (DISPATCH_ADD_PUB(connector, orconn, orconn_status) != 0)
    return -1;
  return 0;
}

void
orconn_state_publish(orconn_state_msg_t *msg)
{
  PUBLISH(orconn_state, msg);
}

void
orconn_status_publish(orconn_status_msg_t *msg)
{
  PUBLISH(orconn_status, msg);
}
