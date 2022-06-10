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

const struct subsys_fns_t sys_relay = {
  .name = "relay",
  SUBSYS_DECLARE_LOCATION(),
  .supported = false,
  .level = RELAY_SUBSYS_LEVEL,
};
