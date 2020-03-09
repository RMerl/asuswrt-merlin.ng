/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "core/mainloop/mainloop_sys.h"
#include "core/mainloop/mainloop.h"

#include "lib/subsys/subsys.h"

static int
subsys_mainloop_initialize(void)
{
  initialize_periodic_events();
  return 0;
}

static void
subsys_mainloop_shutdown(void)
{
  tor_mainloop_free_all();
}

const struct subsys_fns_t sys_mainloop = {
  .name = "mainloop",
  .supported = true,
  .level = 5,
  .initialize = subsys_mainloop_initialize,
  .shutdown = subsys_mainloop_shutdown,
};
