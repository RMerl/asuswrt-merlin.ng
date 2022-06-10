/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file or_sys.c
 * @brief Subsystem definitions for OR module.
 **/

#include "orconfig.h"
#include "core/or/or.h"
#include "core/or/or_periodic.h"
#include "core/or/or_sys.h"
#include "core/or/policies.h"
#include "core/or/protover.h"
#include "core/or/versions.h"

#include "lib/subsys/subsys.h"

static int
subsys_or_initialize(void)
{
  or_register_periodic_events();
  return 0;
}

static void
subsys_or_shutdown(void)
{
  protover_free_all();
  protover_summary_cache_free_all();
  policies_free_all();
}

static int
subsys_or_add_pubsub(struct pubsub_connector_t *connector)
{
  int rv = 0;
  if (orconn_add_pubsub(connector) < 0)
    rv = -1;
  if (ocirc_add_pubsub(connector) < 0)
    rv = -1;
  return rv;
}

const struct subsys_fns_t sys_or = {
  .name = "or",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  .level = 20,
  .initialize = subsys_or_initialize,
  .shutdown = subsys_or_shutdown,
  .add_pubsub = subsys_or_add_pubsub,
};
