/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dirauth_stub.c
 * @brief Stub declarations for use when dirauth module is disabled.
 **/

#include "orconfig.h"
#include "feature/dirauth/dirauth_sys.h"
#include "lib/conf/conftypes.h"
#include "lib/conf/confdecl.h"
#include "lib/subsys/subsys.h"

/* Declare the options field table for dirauth_options */
#define CONF_CONTEXT STUB_TABLE
#include "feature/dirauth/dirauth_options.inc"
#undef CONF_CONTEXT

static const config_format_t dirauth_options_stub_fmt = {
  .vars = dirauth_options_t_vars,
};

const struct subsys_fns_t sys_dirauth = {
  .name = "dirauth",
  SUBSYS_DECLARE_LOCATION(),
  .supported = false,
  .level = DIRAUTH_SUBSYS_LEVEL,

  .options_format = &dirauth_options_stub_fmt
};
