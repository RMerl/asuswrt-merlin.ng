/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file pubsub_build.c
 * @brief Construct a dispatch_t in safer, more OO way.
 **/

#define PUBSUB_PRIVATE

#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_cfg.h"
#include "lib/dispatch/dispatch_naming.h"
#include "lib/dispatch/msgtypes.h"
#include "lib/pubsub/pubsub_flags.h"
#include "lib/pubsub/pub_binding_st.h"
#include "lib/pubsub/pubsub_build.h"
#include "lib/pubsub/pubsub_builder_st.h"
#include "lib/pubsub/pubsub_connect.h"

#include "lib/container/smartlist.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

 #include <string.h>

/** Construct and return a new empty pubsub_items_t. */
static pubsub_items_t *
pubsub_items_new(void)
{
  pubsub_items_t *cfg = tor_malloc_zero(sizeof(*cfg));
  cfg->items = smartlist_new();
  cfg->type_items = smartlist_new();
  return cfg;
}

/** Release all storage held in a pubsub_items_t. */
void
pubsub_items_free_(pubsub_items_t *cfg)
{
  if (! cfg)
    return;
  SMARTLIST_FOREACH(cfg->items, pubsub_cfg_t *, item, tor_free(item));
  SMARTLIST_FOREACH(cfg->type_items,
                    pubsub_type_cfg_t *, item, tor_free(item));
  smartlist_free(cfg->items);
  smartlist_free(cfg->type_items);
  tor_free(cfg);
}

/** Construct and return a new pubsub_builder_t. */
pubsub_builder_t *
pubsub_builder_new(void)
{
  dispatch_naming_init();

  pubsub_builder_t *pb = tor_malloc_zero(sizeof(*pb));
  pb->cfg = dcfg_new();
  pb->items = pubsub_items_new();
  return pb;
}

/**
 * Release all storage held by a pubsub_builder_t.
 *
 * You'll (mostly) only want to call this function on an error case: if you're
 * constructing a dispatch_t instead, you should call
 * pubsub_builder_finalize() to consume the pubsub_builder_t.
 */
void
pubsub_builder_free_(pubsub_builder_t *pb)
{
  if (pb == NULL)
    return;
  pubsub_items_free(pb->items);
  dcfg_free(pb->cfg);
  tor_free(pb);
}

/**
 * Create and return a pubsub_connector_t for the subsystem with ID
 * <b>subsys</b> to use in adding publications, subscriptions, and types to
 * <b>builder</b>.
 **/
pubsub_connector_t *
pubsub_connector_for_subsystem(pubsub_builder_t *builder,
                               subsys_id_t subsys)
{
  tor_assert(builder);
  ++builder->n_connectors;

  pubsub_connector_t *con = tor_malloc_zero(sizeof(*con));

  con->builder = builder;
  con->subsys_id = subsys;

  return con;
}

/**
 * Release all storage held by a pubsub_connector_t.
 **/
void
pubsub_connector_free_(pubsub_connector_t *con)
{
  if (!con)
    return;

  if (con->builder) {
    --con->builder->n_connectors;
    tor_assert(con->builder->n_connectors >= 0);
  }
  tor_free(con);
}

/**
 * Use <b>con</b> to add a request for being able to publish messages of type
 * <b>msg</b> with auxiliary data of <b>type</b> on <b>channel</b>.
 **/
int
pubsub_add_pub_(pubsub_connector_t *con,
                pub_binding_t *out,
                channel_id_t channel,
                message_id_t msg,
                msg_type_id_t type,
                unsigned flags,
                const char *file,
                unsigned line)
{
  pubsub_cfg_t *cfg = tor_malloc_zero(sizeof(*cfg));

  memset(out, 0, sizeof(*out));
  cfg->is_publish = true;

  out->msg_template.sender = cfg->subsys = con->subsys_id;
  out->msg_template.channel = cfg->channel = channel;
  out->msg_template.msg = cfg->msg = msg;
  out->msg_template.type = cfg->type = type;

  cfg->flags = flags;
  cfg->added_by_file = file;
  cfg->added_by_line = line;

  /* We're grabbing a pointer to the pub_binding_t so we can tell it about
   * the dispatcher later on.
   */
  cfg->pub_binding = out;

  smartlist_add(con->builder->items->items, cfg);

  if (dcfg_msg_set_type(con->builder->cfg, msg, type) < 0)
    goto err;
  if (dcfg_msg_set_chan(con->builder->cfg, msg, channel) < 0)
    goto err;

  return 0;
 err:
  ++con->builder->n_errors;
  return -1;
}

/**
 * Use <b>con</b> to add a request for being able to publish messages of type
 * <b>msg</b> with auxiliary data of <b>type</b> on <b>channel</b>,
 * passing them to the callback in <b>recv_fn</b>.
 **/
int
pubsub_add_sub_(pubsub_connector_t *con,
                recv_fn_t recv_fn,
                channel_id_t channel,
                message_id_t msg,
                msg_type_id_t type,
                unsigned flags,
                const char *file,
                unsigned line)
{
  pubsub_cfg_t *cfg = tor_malloc_zero(sizeof(*cfg));

  cfg->is_publish = false;
  cfg->subsys = con->subsys_id;
  cfg->channel = channel;
  cfg->msg = msg;
  cfg->type = type;
  cfg->flags = flags;
  cfg->added_by_file = file;
  cfg->added_by_line = line;

  cfg->recv_fn = recv_fn;

  smartlist_add(con->builder->items->items, cfg);

  if (dcfg_msg_set_type(con->builder->cfg, msg, type) < 0)
    goto err;
  if (dcfg_msg_set_chan(con->builder->cfg, msg, channel) < 0)
    goto err;
  if (! (flags & DISP_FLAG_STUB)) {
    if (dcfg_add_recv(con->builder->cfg, msg, cfg->subsys, recv_fn) < 0)
      goto err;
  }

  return 0;
 err:
  ++con->builder->n_errors;
  return -1;
}

/**
 * Use <b>con</b> to define the functions to use for manipulating the type
 * <b>type</b>.  Any function pointers left as NULL will be implemented as
 * no-ops.
 **/
int
pubsub_connector_register_type_(pubsub_connector_t *con,
                                msg_type_id_t type,
                                dispatch_typefns_t *fns,
                                const char *file,
                                unsigned line)
{
  pubsub_type_cfg_t *cfg = tor_malloc_zero(sizeof(*cfg));
  cfg->type = type;
  memcpy(&cfg->fns, fns, sizeof(*fns));
  cfg->subsys = con->subsys_id;
  cfg->added_by_file = file;
  cfg->added_by_line = line;

  smartlist_add(con->builder->items->type_items, cfg);

  if (dcfg_type_set_fns(con->builder->cfg, type, fns) < 0)
    goto err;

  return 0;
 err:
  ++con->builder->n_errors;
  return -1;
}

/**
 * Initialize the dispatch_ptr field in every relevant publish binding
 * for <b>d</b>.
 */
static void
pubsub_items_install_bindings(pubsub_items_t *items,
                              dispatch_t *d)
{
  SMARTLIST_FOREACH_BEGIN(items->items, pubsub_cfg_t *, cfg) {
    if (cfg->pub_binding) {
      // XXXX we could skip this for STUB publishers, and for any publishers
      // XXXX where all subscribers are STUB.
      cfg->pub_binding->dispatch_ptr = d;
    }
  } SMARTLIST_FOREACH_END(cfg);
}

/**
 * Remove the dispatch_ptr fields for all the relevant publish bindings
 * in <b>items</b>.  The prevents subsequent dispatch_pub_() calls from
 * sending messages to a dispatcher that has been freed.
 **/
void
pubsub_items_clear_bindings(pubsub_items_t *items)
{
  SMARTLIST_FOREACH_BEGIN(items->items, pubsub_cfg_t *, cfg) {
    if (cfg->pub_binding) {
      cfg->pub_binding->dispatch_ptr = NULL;
    }
  } SMARTLIST_FOREACH_END(cfg);
}

/**
 * Create a new dispatcher as configured in a pubsub_builder_t.
 *
 * Consumes and frees its input.
 **/
dispatch_t *
pubsub_builder_finalize(pubsub_builder_t *builder,
                        pubsub_items_t **items_out)
{
  dispatch_t *dispatcher = NULL;
  tor_assert_nonfatal(builder->n_connectors == 0);

  if (pubsub_builder_check(builder) < 0)
    goto err;

  if (builder->n_errors) {
    log_warn(LD_GENERAL, "At least one error occurred previously when "
             "configuring the dispatcher.");
    goto err;
  }

  dispatcher = dispatch_new(builder->cfg);

  if (!dispatcher)
    goto err;

  pubsub_items_install_bindings(builder->items, dispatcher);
  if (items_out) {
    *items_out = builder->items;
    builder->items = NULL; /* Prevent free */
  }

 err:
  pubsub_builder_free(builder);
  return dispatcher;
}
