/* Copyright (c) 2021-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dos_sys.c
 * @brief Subsystem definitions for DOS module.
 **/

#include "core/or/or.h"

#include "lib/subsys/subsys.h"

#include "core/or/dos_config.h"
#include "core/or/dos_sys.h"

#include "core/or/dos_options_st.h"

static const dos_options_t *global_dos_options;

static int
subsys_dos_initialize(void)
{
  return 0;
}

static void
subsys_dos_shutdown(void)
{
  global_dos_options = NULL;
}

const dos_options_t *
dos_get_options(void)
{
  tor_assert(global_dos_options);
  return global_dos_options;
}

static int
dos_set_options(void *arg)
{
  dos_options_t *opts = arg;
  global_dos_options = opts;
  return 0;
}

const struct subsys_fns_t sys_dos = {
  SUBSYS_DECLARE_LOCATION(),

  .name = "dos",
  .supported = true,
  .level = DOS_SUBSYS_LEVEL,

  .initialize = subsys_dos_initialize,
  .shutdown = subsys_dos_shutdown,

  /* Configuration Options. */
  .options_format = &dos_options_fmt,
  .set_options = dos_set_options,
};
