/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file pubsub_check.c
 * @brief Enforce various requirements on a pubsub_builder.
 **/

/** @{ */
#define PUBSUB_PRIVATE
/** @} */

#include "lib/dispatch/dispatch_naming.h"
#include "lib/dispatch/msgtypes.h"
#include "lib/pubsub/pubsub_flags.h"
#include "lib/pubsub/pubsub_builder_st.h"
#include "lib/pubsub/pubsub_build.h"

#include "lib/container/bitarray.h"
#include "lib/container/smartlist.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/compat_string.h"

#include <string.h>

static void pubsub_adjmap_add(pubsub_adjmap_t *map,
                                const pubsub_cfg_t *item);

/**
 * Helper: construct and return a new pubsub_adjacency_map from <b>cfg</b>.
 * Return NULL on error.
 **/
static pubsub_adjmap_t *
pubsub_build_adjacency_map(const pubsub_items_t *cfg)
{
  pubsub_adjmap_t *map = tor_malloc_zero(sizeof(*map));
  const size_t n_subsystems = get_num_subsys_ids();
  const size_t n_msgs = get_num_message_ids();

  map->n_subsystems = n_subsystems;
  map->n_msgs = n_msgs;

  map->pub_by_subsys = tor_calloc(n_subsystems, sizeof(smartlist_t*));
  map->sub_by_subsys = tor_calloc(n_subsystems, sizeof(smartlist_t*));
  map->pub_by_msg = tor_calloc(n_msgs, sizeof(smartlist_t*));
  map->sub_by_msg = tor_calloc(n_msgs, sizeof(smartlist_t*));

  SMARTLIST_FOREACH_BEGIN(cfg->items, const pubsub_cfg_t *, item) {
    pubsub_adjmap_add(map, item);
  } SMARTLIST_FOREACH_END(item);

  return map;
}

/**
 * Helper: add a single pubsub_cfg_t to an adjacency map.
 **/
static void
pubsub_adjmap_add(pubsub_adjmap_t *map,
                  const pubsub_cfg_t *item)
{
  smartlist_t **by_subsys;
  smartlist_t **by_msg;

  tor_assert(item->subsys < map->n_subsystems);
  tor_assert(item->msg < map->n_msgs);

  if (item->is_publish) {
    by_subsys = &map->pub_by_subsys[item->subsys];
    by_msg = &map->pub_by_msg[item->msg];
  } else {
    by_subsys = &map->sub_by_subsys[item->subsys];
    by_msg = &map->sub_by_msg[item->msg];
  }

  if (! *by_subsys)
    *by_subsys = smartlist_new();
  if (! *by_msg)
    *by_msg = smartlist_new();
  smartlist_add(*by_subsys, (void*) item);
  smartlist_add(*by_msg, (void *) item);
}

/**
 * Release all storage held by m and set m to NULL.
 **/
#define pubsub_adjmap_free(m) \
  FREE_AND_NULL(pubsub_adjmap_t, pubsub_adjmap_free_, m)

/**
 * Free every element of an <b>n</b>-element array of smartlists, then
 * free the array itself.
 **/
static void
pubsub_adjmap_free_helper(smartlist_t **lsts, size_t n)
{
  if (!lsts)
    return;

  for (unsigned i = 0; i < n; ++i) {
    smartlist_free(lsts[i]);
  }
  tor_free(lsts);
}

/**
 * Release all storage held by <b>map</b>.
 **/
static void
pubsub_adjmap_free_(pubsub_adjmap_t *map)
{
  if (!map)
    return;
  pubsub_adjmap_free_helper(map->pub_by_subsys, map->n_subsystems);
  pubsub_adjmap_free_helper(map->sub_by_subsys, map->n_subsystems);
  pubsub_adjmap_free_helper(map->pub_by_msg, map->n_msgs);
  pubsub_adjmap_free_helper(map->sub_by_msg, map->n_msgs);
  tor_free(map);
}

/**
 * Helper: return the length of <b>sl</b>, or 0 if sl is NULL.
 **/
static int
smartlist_len_opt(const smartlist_t *sl)
{
  if (sl)
    return smartlist_len(sl);
  else
    return 0;
}

/** Return a pointer to a statically allocated string encoding the
 * dispatcher flags in <b>flags</b>. */
static const char *
format_flags(unsigned flags)
{
  static char buf[32];
  buf[0] = 0;
  if (flags & DISP_FLAG_EXCL) {
    strlcat(buf, " EXCL", sizeof(buf));
  }
  if (flags & DISP_FLAG_STUB) {
    strlcat(buf, " STUB", sizeof(buf));
  }
  return buf[0] ? buf+1 : buf;
}

/**
 * Log a message containing a description of <b>cfg</b> at severity, prefixed
 * by the string <b>prefix</b>.
 */
static void
pubsub_cfg_dump(const pubsub_cfg_t *cfg, int severity, const char *prefix)
{
  tor_assert(prefix);

  tor_log(severity, LD_MESG,
          "%s%s %s: %s{%s} on %s (%s) <%u %u %u %u %x> [%s:%d]",
          prefix,
          get_subsys_id_name(cfg->subsys),
          cfg->is_publish ? "PUB" : "SUB",
          get_message_id_name(cfg->msg),
          get_msg_type_id_name(cfg->type),
          get_channel_id_name(cfg->channel),
          format_flags(cfg->flags),
          cfg->subsys, cfg->msg, cfg->type, cfg->channel, cfg->flags,
          cfg->added_by_file, cfg->added_by_line);
}

/**
 * Helper: fill a bitarray <b>out</b> with entries corresponding to the
 * subsystems listed in <b>items</b>.
 **/
static void
get_message_bitarray(const pubsub_adjmap_t *map,
                     const smartlist_t *items,
                     bitarray_t **out)
{
  *out = bitarray_init_zero((unsigned)map->n_subsystems);
  if (! items)
    return;

  SMARTLIST_FOREACH_BEGIN(items, const pubsub_cfg_t *, cfg) {
    bitarray_set(*out, cfg->subsys);
  } SMARTLIST_FOREACH_END(cfg);
}

/**
 * Helper for lint_message: check that all the pubsub_cfg_t items in the two
 * respective smartlists obey our local graph topology rules.
 *
 * (Right now this is just a matter of "each subsystem only
 * publishes/subscribes once; no subsystem is a publisher and subscriber for
 * the same message.")
 *
 * Return 0 on success, -1 on failure.
 **/
static int
lint_message_graph(const pubsub_adjmap_t *map,
                   message_id_t msg,
                   const smartlist_t *pub,
                   const smartlist_t *sub)
{
  bitarray_t *published_by = NULL;
  bitarray_t *subscribed_by = NULL;
  bool ok = true;

  get_message_bitarray(map, pub, &published_by);
  get_message_bitarray(map, sub, &subscribed_by);

  /* Check whether any subsystem is publishing and subscribing the same
   * message. [??]
   */
  for (unsigned i = 0; i < map->n_subsystems; ++i) {
    if (bitarray_is_set(published_by, i) &&
        bitarray_is_set(subscribed_by, i)) {
      log_warn(LD_MESG|LD_BUG,
               "Message \"%s\" is published and subscribed by the same "
               "subsystem \"%s\".",
               get_message_id_name(msg),
               get_subsys_id_name(i));
      ok = false;
    }
  }

  bitarray_free(published_by);
  bitarray_free(subscribed_by);

  return ok ? 0 : -1;
}

/**
 * Helper for lint_message: check that all the pubsub_cfg_t items in the two
 * respective smartlists have compatible flags, channels, and types.
 **/
static int
lint_message_consistency(message_id_t msg,
                         const smartlist_t *pub,
                         const smartlist_t *sub)
{
  if (!smartlist_len_opt(pub) && !smartlist_len_opt(sub))
    return 0; // LCOV_EXCL_LINE -- this was already checked.

  /* The 'all' list has the publishers and the subscribers. */
  smartlist_t *all = smartlist_new();
  if (pub)
    smartlist_add_all(all, pub);
  if (sub)
    smartlist_add_all(all, sub);

  const pubsub_cfg_t *item0 = smartlist_get(all, 0);

  /* Indicates which subsystems we've found publishing/subscribing here. */
  bool pub_excl = false, sub_excl = false, chan_same = true, type_same = true;

  /* Simple message consistency properties across messages.
   */
  SMARTLIST_FOREACH_BEGIN(all, const pubsub_cfg_t *, cfg) {
    chan_same &= (cfg->channel == item0->channel);
    type_same &= (cfg->type == item0->type);
    if (cfg->is_publish)
      pub_excl |= (cfg->flags & DISP_FLAG_EXCL) != 0;
    else
      sub_excl |= (cfg->flags & DISP_FLAG_EXCL) != 0;
  } SMARTLIST_FOREACH_END(cfg);

  bool ok = true;

  if (! chan_same) {
    log_warn(LD_MESG|LD_BUG,
             "Message \"%s\" is associated with multiple inconsistent "
             "channels.",
             get_message_id_name(msg));
    ok = false;
  }
  if (! type_same) {
    log_warn(LD_MESG|LD_BUG,
             "Message \"%s\" is associated with multiple inconsistent "
             "message types.",
             get_message_id_name(msg));
    ok = false;
  }

  /* Enforce exclusive-ness for publishers and subscribers that have asked for
   * it.
   */
  if (pub_excl && smartlist_len_opt(pub) > 1) {
    log_warn(LD_MESG|LD_BUG,
             "Message \"%s\" has multiple publishers, but at least one is "
             "marked as exclusive.",
             get_message_id_name(msg));
    ok = false;
  }
  if (sub_excl && smartlist_len_opt(sub) > 1) {
    log_warn(LD_MESG|LD_BUG,
             "Message \"%s\" has multiple subscribers, but at least one is "
             "marked as exclusive.",
             get_message_id_name(msg));
    ok = false;
  }

  smartlist_free(all);

  return ok ? 0 : -1;
}

/**
 * Check whether there are any errors or inconsistencies for the message
 * described by <b>msg</b> in <b>map</b>.  If there are problems, log about
 * them, and return -1.  Otherwise return 0.
 **/
static int
lint_message(const pubsub_adjmap_t *map, message_id_t msg)
{
  /* NOTE: Some of the checks in this function are maybe over-zealous, and we
   * might not want to have them forever.  I've marked them with [?] below.
   */
  if (BUG(msg >= map->n_msgs))
    return 0; // LCOV_EXCL_LINE

  const smartlist_t *pub = map->pub_by_msg[msg];
  const smartlist_t *sub = map->sub_by_msg[msg];

  const size_t n_pub = smartlist_len_opt(pub);
  const size_t n_sub = smartlist_len_opt(sub);

  if (n_pub == 0 && n_sub == 0) {
    log_info(LD_MESG, "Nobody is publishing or subscribing to message "
             "\"%s\".",
             get_message_id_name(msg));
    return 0; // No publishers or subscribers: nothing to do.
  }
  /* We'll set this to false if there are any problems. */
  bool ok = true;

  /* First make sure that if there are publishers, there are subscribers. */
  if (n_pub == 0) {
    log_warn(LD_MESG|LD_BUG,
             "Message \"%s\" has subscribers, but no publishers.",
             get_message_id_name(msg));
    ok = false;
  } else if (n_sub == 0) {
    log_warn(LD_MESG|LD_BUG,
             "Message \"%s\" has publishers, but no subscribers.",
             get_message_id_name(msg));
    ok = false;
  }

  /* Check the message graph topology. */
  if (lint_message_graph(map, msg, pub, sub) < 0)
    ok = false;

  /* Check whether the messages have the same fields set on them. */
  if (lint_message_consistency(msg, pub, sub) < 0)
    ok = false;

  if (!ok) {
    /* There was a problem -- let's log all the publishers and subscribers on
     * this message */
    if (pub) {
      SMARTLIST_FOREACH(pub, pubsub_cfg_t *, cfg,
                        pubsub_cfg_dump(cfg, LOG_WARN, "   "));
    }
    if (sub) {
      SMARTLIST_FOREACH(sub, pubsub_cfg_t *, cfg,
                        pubsub_cfg_dump(cfg, LOG_WARN, "   "));
    }
  }

  return ok ? 0 : -1;
}

/**
 * Check all the messages in <b>map</b> for consistency.  Return 0 on success,
 * -1 on problems.
 **/
static int
pubsub_adjmap_check(const pubsub_adjmap_t *map)
{
  bool all_ok = true;
  for (unsigned i = 0; i < map->n_msgs; ++i) {
    if (lint_message(map, i) < 0) {
      all_ok = false;
    }
  }
  return all_ok ? 0 : -1;
}

/**
 * Check builder for consistency and various constraints. Return 0 on success,
 * -1 on failure.
 **/
int
pubsub_builder_check(pubsub_builder_t *builder)
{
  pubsub_adjmap_t *map = pubsub_build_adjacency_map(builder->items);
  int rv = -1;

  if (!map)
    goto err; // should be impossible

  if (pubsub_adjmap_check(map) < 0)
    goto err;

  rv = 0;
 err:
  pubsub_adjmap_free(map);
  return rv;
}
