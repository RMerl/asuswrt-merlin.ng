/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file routermode.c
 * @brief Check if we're running as a relay/cache.
 **/

#include "core/or/or.h"

#include "app/config/config.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"

/** Return 1 if we are configured to accept either relay or directory requests
 * from clients and we aren't at risk of exceeding our bandwidth limits, thus
 * we should be a directory server. If not, return 0.
 */
int
dir_server_mode(const or_options_t *options)
{
  if (!options->DirCache)
    return 0;
  return options->DirPort_set ||
    (server_mode(options) && router_has_bandwidth_to_be_dirserver(options));
}

/** Return true iff we are trying to be a server.
 */
MOCK_IMPL(int,
server_mode,(const or_options_t *options))
{
  if (options->ClientOnly) return 0;
  return (options->ORPort_set);
}

/** Return true iff we are trying to be a non-bridge server.
 */
MOCK_IMPL(int,
public_server_mode,(const or_options_t *options))
{
  if (!server_mode(options)) return 0;
  return (!options->BridgeRelay);
}

/** Remember if we've advertised ourselves to the dirservers. */
static int server_is_advertised=0;

/** Return true iff we have published our descriptor lately.
 */
MOCK_IMPL(int,
advertised_server_mode,(void))
{
  return server_is_advertised;
}

/**
 * Called with a boolean: set whether we have recently published our
 * descriptor.
 */
void
set_server_advertised(int s)
{
  server_is_advertised = s;
}
