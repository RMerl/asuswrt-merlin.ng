/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_stub.c
 * @brief Stub declarations for use when relay module is disabled.
 **/

#include "orconfig.h"
#include "feature/relay/relay_sys.h"
#include "lib/subsys/subsys.h"
#include "feature/relay/relay_metrics.h"

const struct subsys_fns_t sys_relay = {
  .name = "relay",
  SUBSYS_DECLARE_LOCATION(),
  .supported = false,
  .level = RELAY_SUBSYS_LEVEL,
};

void
relay_increment_est_intro_action(est_intro_action_t action)
{
  (void)action;
}

void
relay_increment_est_rend_action(est_rend_action_t action)
{
  (void)action;
}

void
relay_increment_intro1_action(intro1_action_t action)
{
  (void)action;
}

void
relay_increment_rend1_action(rend1_action_t action)
{
  (void)action;
}
