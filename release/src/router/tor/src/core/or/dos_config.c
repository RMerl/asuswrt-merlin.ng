/* Copyright (c) 2021-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dos_config.c
 * @brief Code to interpret the user's configuration of DoS module.
 **/

#include "core/or/dos_config.h"
#include "core/or/dos_options_st.h"

/* Declare the options field table for dos_options */
#define CONF_CONTEXT TABLE
#include "core/or/dos_options.inc"
#undef CONF_CONTEXT

/** Magic number for dos_options_t. */
#define DOS_OPTIONS_MAGIC 0x91716151

/**
 * Declare the configuration options for the dos module.
 **/
const config_format_t dos_options_fmt = {
  .size = sizeof(dos_options_t),
  .magic = { "dos_options_t",
             DOS_OPTIONS_MAGIC,
             offsetof(dos_options_t, magic) },
  .vars = dos_options_t_vars,
};
