/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file authmode.c
 * \brief What kind of directory authority are we?
 *
 * If we're not an authority, these functions are all replaced with 0 in
 * authmode.h.
 **/

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/dirauth/authmode.h"

#include "feature/nodelist/routerinfo_st.h"

/** Return true iff we believe ourselves to be an authoritative
 * directory server.
 */
int
authdir_mode(const or_options_t *options)
{
  return options->AuthoritativeDir != 0;
}

/* Return true iff we believe ourselves to be a v3 authoritative directory
 * server. */
int
authdir_mode_v3(const or_options_t *options)
{
  return authdir_mode(options) && options->V3AuthoritativeDir != 0;
}

/** Return true iff we are an authoritative directory server that is
 * authoritative about receiving and serving descriptors of type
 * <b>purpose</b> on its dirport.
 */
int
authdir_mode_handles_descs(const or_options_t *options, int purpose)
{
  if (BUG(purpose < 0)) /* Deprecated. */
    return authdir_mode(options);
  else if (purpose == ROUTER_PURPOSE_GENERAL)
    return authdir_mode_v3(options);
  else if (purpose == ROUTER_PURPOSE_BRIDGE)
    return authdir_mode_bridge(options);
  else
    return 0;
}
/** Return true iff we are an authoritative directory server that
 * publishes its own network statuses.
 */
int
authdir_mode_publishes_statuses(const or_options_t *options)
{
  if (authdir_mode_bridge(options))
    return 0;
  return authdir_mode(options);
}
/** Return true iff we are an authoritative directory server that
 * tests reachability of the descriptors it learns about.
 */
int
authdir_mode_tests_reachability(const or_options_t *options)
{
  return authdir_mode(options);
}
/** Return true iff we believe ourselves to be a bridge authoritative
 * directory server.
 */
int
authdir_mode_bridge(const or_options_t *options)
{
  return authdir_mode(options) && options->BridgeAuthoritativeDir != 0;
}
