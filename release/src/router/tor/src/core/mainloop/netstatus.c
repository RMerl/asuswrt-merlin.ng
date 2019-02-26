/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "core/mainloop/netstatus.h"
#include "app/config/config.h"
#include "feature/hibernate/hibernate.h"

/** Return true iff our network is in some sense disabled or shutting down:
 * either we're hibernating, entering hibernation, or the network is turned
 * off with DisableNetwork. */
int
net_is_disabled(void)
{
  return get_options()->DisableNetwork || we_are_hibernating();
}

/** Return true iff our network is in some sense "completely disabled" either
 * we're fully hibernating or the network is turned off with
 * DisableNetwork. */
int
net_is_completely_disabled(void)
{
  return get_options()->DisableNetwork || we_are_fully_hibernating();
}
