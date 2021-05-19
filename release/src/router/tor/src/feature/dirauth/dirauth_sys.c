/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dirauth_sys.c
 * @brief Directory authority subsystem declarations
 **/

#include "core/or/or.h"

#define DIRAUTH_SYS_PRIVATE
#include "feature/dirauth/bwauth.h"
#include "feature/dirauth/dirauth_sys.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirauth/dirauth_periodic.h"
#include "feature/dirauth/keypin.h"
#include "feature/dirauth/process_descs.h"
#include "feature/dirauth/dirauth_config.h"

#include "feature/dirauth/dirauth_options_st.h"

#include "lib/subsys/subsys.h"

static const dirauth_options_t *global_dirauth_options;

static int
subsys_dirauth_initialize(void)
{
  dirauth_register_periodic_events();
  return 0;
}

static void
subsys_dirauth_shutdown(void)
{
  dirserv_free_fingerprint_list();
  dirvote_free_all();
  dirserv_clear_measured_bw_cache();
  keypin_close_journal();
  global_dirauth_options = NULL;
}

const dirauth_options_t *
dirauth_get_options(void)
{
  tor_assert(global_dirauth_options);
  return global_dirauth_options;
}

STATIC int
dirauth_set_options(void *arg)
{
  dirauth_options_t *opts = arg;
  global_dirauth_options = opts;
  return 0;
}

const struct subsys_fns_t sys_dirauth = {
  .name = "dirauth",
  SUBSYS_DECLARE_LOCATION(),
  .supported = true,
  .level = DIRAUTH_SUBSYS_LEVEL,
  .initialize = subsys_dirauth_initialize,
  .shutdown = subsys_dirauth_shutdown,

  .options_format = &dirauth_options_fmt,
  .set_options = dirauth_set_options,
};
