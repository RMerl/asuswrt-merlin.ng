/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file opts_testing_helpers.c
 * @brief Helper functions to access module-specific config options.
 **/

#include "orconfig.h"
#include "test/opts_test_helpers.h"

#define CONFIG_PRIVATE
#include "core/or/or.h"
#include "lib/confmgt/confmgt.h"
#include "app/main/subsysmgr.h"
#include "app/config/config.h"

#include "lib/crypt_ops/crypto_sys.h"
#include "feature/dirauth/dirauth_sys.h"

struct dirauth_options_t *
get_dirauth_options(struct or_options_t *opt)
{
  int idx = subsystems_get_options_idx(&sys_dirauth);
  tor_assert(idx >= 0);
  return config_mgr_get_obj_mutable(get_options_mgr(), opt, idx);
}

struct crypto_options_t *
get_crypto_options(struct or_options_t *opt)
{
  int idx = subsystems_get_options_idx(&sys_crypto);
  tor_assert(idx >= 0);
  return config_mgr_get_obj_mutable(get_options_mgr(), opt, idx);
}
