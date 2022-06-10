/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file pubsub_publish.c
 * @brief Header for functions to publish using a pub_binding_t.
 **/

#define PUBSUB_PRIVATE
#define DISPATCH_PRIVATE
#include "orconfig.h"

#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_st.h"

#include "lib/pubsub/pub_binding_st.h"
#include "lib/pubsub/pubsub_publish.h"

#include "lib/malloc/malloc.h"
#include "lib/log/util_bug.h"

#include <string.h>

/**
 * Publish a message from the publication binding <b>pub</b> using the
 * auxiliary data <b>auxdata</b>.
 *
 * Return 0 on success, -1 on failure.
 **/
int
pubsub_pub_(const pub_binding_t *pub, msg_aux_data_t auxdata)
{
  dispatch_t *d = pub->dispatch_ptr;
  if (BUG(! d)) {
    /* Tried to publish a message before the dispatcher was configured. */
    /* (Without a dispatcher, we don't know how to free auxdata.) */
    return -1;
  }

  if (BUG(pub->msg_template.type >= d->n_types)) {
    /* The type associated with this message is not known to the dispatcher. */
    /* (Without a correct type, we don't know how to free auxdata.) */
    return -1;
  }

  if (BUG(pub->msg_template.msg >= d->n_msgs) ||
      BUG(pub->msg_template.channel >= d->n_queues)) {
    /* The message ID or channel ID was out of bounds. */
    // LCOV_EXCL_START
    d->typefns[pub->msg_template.type].free_fn(auxdata);
    return -1;
    // LCOV_EXCL_STOP
  }

  if (! d->table[pub->msg_template.msg]) {
    /* Fast path: nobody wants this data. */

    // XXXX Faster path: we could store this in the pub_binding_t.
    d->typefns[pub->msg_template.type].free_fn(auxdata);
    return 0;
  }

  /* Construct the message object */
  msg_t *m = tor_malloc(sizeof(msg_t));
  memcpy(m, &pub->msg_template, sizeof(msg_t));
  m->aux_data__ = auxdata;

  return dispatch_send_msg_unchecked(d, m);
}
