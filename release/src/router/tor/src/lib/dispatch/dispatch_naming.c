/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dispatch_naming.c
 * @brief Name-to-ID maps for our message dispatch system.
 **/

#include "orconfig.h"

#include "lib/cc/compat_compiler.h"

#include "lib/dispatch/dispatch_naming.h"
#include "lib/dispatch/msgtypes.h"

#include "lib/container/namemap.h"
#include "lib/container/namemap_st.h"

#include "lib/log/util_bug.h"
#include "lib/log/log.h"

#include <stdlib.h>

/** Global namemap for message IDs. */
static namemap_t message_id_map = NAMEMAP_INIT();
/** Global namemap for subsystem IDs. */
static namemap_t subsys_id_map = NAMEMAP_INIT();
/** Global namemap for channel IDs. */
static namemap_t channel_id_map = NAMEMAP_INIT();
/** Global namemap for message type IDs. */
static namemap_t msg_type_id_map = NAMEMAP_INIT();

void
dispatch_naming_init(void)
{
}

#ifndef COCCI
/* Helper macro: declare functions to map IDs to and from names for a given
 * type in a namemap_t.
 */
#define DECLARE_ID_MAP_FNS(type)                                        \
  type##_id_t                                                           \
  get_##type##_id(const char *name)                                     \
  {                                                                     \
    unsigned u = namemap_get_or_create_id(&type##_id_map, name);        \
    tor_assert(u != NAMEMAP_ERR);                                       \
    tor_assert(u != ERROR_ID);                                          \
    return (type##_id_t) u;                                             \
  }                                                                     \
  const char *                                                          \
  get_##type##_id_name(type##_id_t id)                                  \
  {                                                                     \
    return namemap_fmt_name(&type##_id_map, id);                        \
  }                                                                     \
  size_t                                                                \
  get_num_##type##_ids(void)                                            \
  {                                                                     \
    return namemap_get_size(&type##_id_map);                            \
  }                                                                     \
  EAT_SEMICOLON
#endif /* !defined(COCCI) */

DECLARE_ID_MAP_FNS(message);
DECLARE_ID_MAP_FNS(channel);
DECLARE_ID_MAP_FNS(subsys);
DECLARE_ID_MAP_FNS(msg_type);
