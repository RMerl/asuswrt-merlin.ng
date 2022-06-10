/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file trace_stub.c
 * \brief Stub declarations for use when trace library is disabled.
 **/

#include "lib/subsys/subsys.h"

#include "lib/trace/trace_sys.h"

const subsys_fns_t sys_tracing = {
  SUBSYS_DECLARE_LOCATION(),

  .name = "tracing",
  .supported = false,
  .level = TRACE_SUBSYS_LEVEL,
};
