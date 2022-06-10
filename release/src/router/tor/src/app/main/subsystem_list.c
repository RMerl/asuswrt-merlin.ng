/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file subsystem_list.c
 * @brief List of Tor's subsystems.
 **/

#include "orconfig.h"
#include "app/main/subsysmgr.h"
#include "lib/cc/compat_compiler.h"
#include "lib/cc/torint.h"

#include "core/mainloop/mainloop_sys.h"
#include "core/or/dos_sys.h"
#include "core/or/or_sys.h"
#include "feature/control/btrack_sys.h"
#include "lib/compress/compress_sys.h"
#include "lib/crypt_ops/crypto_sys.h"
#include "lib/err/torerr_sys.h"
#include "lib/log/log_sys.h"
#include "lib/net/network_sys.h"
#include "lib/process/process_sys.h"
#include "lib/llharden/winprocess_sys.h"
#include "lib/thread/thread_sys.h"
#include "lib/time/time_sys.h"
#include "lib/tls/tortls_sys.h"
#include "lib/trace/trace_sys.h"
#include "lib/wallclock/wallclock_sys.h"
#include "lib/evloop/evloop_sys.h"

#include "feature/dirauth/dirauth_sys.h"
#include "feature/hs/hs_sys.h"
#include "feature/metrics/metrics_sys.h"
#include "feature/relay/relay_sys.h"

#include <stddef.h>

/**
 * Global list of the subsystems in Tor, in the order of their initialization.
 * Want to know the exact level numbers?
 * We'll implement a level dump command in #31614.
 **/
const subsys_fns_t *tor_subsystems[] = {
  &sys_winprocess,
  &sys_torerr,

  &sys_wallclock,
  &sys_logging,
  &sys_threads,

  &sys_tracing,

  &sys_time,

  &sys_crypto,
  &sys_compress,
  &sys_network,
  &sys_tortls,

  &sys_evloop,
  &sys_process,

  &sys_mainloop,
  &sys_or,
  &sys_dos,

  &sys_relay,
  &sys_hs,

  &sys_btrack,

  &sys_dirauth,
  &sys_metrics,
};

const unsigned n_tor_subsystems = ARRAY_LENGTH(tor_subsystems);
