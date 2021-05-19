/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file mainloop_sys.c
 * @brief Declare the "mainloop" subsystem.
 **/

#include "core/or/or.h"
#include "core/mainloop/mainloop_sys.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/mainloop_state_st.h"
#include "core/mainloop/netstatus.h"
#include "lib/conf/conftypes.h"
#include "lib/conf/confdecl.h"

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

/** Declare a list of state variables for mainloop state. */
#define CONF_CONTEXT TABLE
#include "core/mainloop/mainloop_state.inc"
#undef CONF_CONTEXT

/** Magic number for mainloop state objects */
#define MAINLOOP_STATE_MAGIC 0x59455449

/**
 * Format object for mainloop state.
 **/
static config_format_t mainloop_state_fmt = {
  .size = sizeof(mainloop_state_t),
  .magic = { "mainloop_state",
             MAINLOOP_STATE_MAGIC,
             offsetof(mainloop_state_t, magic)
            },
  .vars = mainloop_state_t_vars,
};

/**
 */
static int
mainloop_set_state(void *arg)
{
  const mainloop_state_t *state = arg;
  tor_assert(state->magic == MAINLOOP_STATE_MAGIC);

  netstatus_load_from_state(state, approx_time());

  return 0;
}

static int
mainloop_flush_state(void *arg)
{
  mainloop_state_t *state = arg;
  tor_assert(state->magic == MAINLOOP_STATE_MAGIC);

  netstatus_flush_to_state(state, approx_time());

  return 0;
}

const struct subsys_fns_t sys_mainloop = {
  .name = "mainloop",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  .level = 5,
  .initialize = subsys_mainloop_initialize,
  .shutdown = subsys_mainloop_shutdown,

  .state_format = &mainloop_state_fmt,
  .set_state = mainloop_set_state,
  .flush_state = mainloop_flush_state,
};
