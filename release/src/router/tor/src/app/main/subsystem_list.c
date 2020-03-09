/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "app/main/subsysmgr.h"
#include "lib/cc/compat_compiler.h"
#include "lib/cc/torint.h"

#include "core/mainloop/mainloop_sys.h"
#include "core/or/ocirc_event_sys.h"
#include "core/or/or_sys.h"
#include "core/or/orconn_event_sys.h"
#include "feature/control/btrack_sys.h"
#include "feature/relay/relay_sys.h"
#include "lib/compress/compress_sys.h"
#include "lib/crypt_ops/crypto_sys.h"
#include "lib/err/torerr_sys.h"
#include "lib/log/log_sys.h"
#include "lib/net/network_sys.h"
#include "lib/process/process_sys.h"
#include "lib/process/winprocess_sys.h"
#include "lib/thread/thread_sys.h"
#include "lib/time/time_sys.h"
#include "lib/tls/tortls_sys.h"
#include "lib/wallclock/wallclock_sys.h"
#include "lib/evloop/evloop_sys.h"

#include "feature/dirauth/dirauth_sys.h"

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
  &sys_threads,
  &sys_logging,

  &sys_time,
  &sys_network,

  &sys_compress,
  &sys_crypto,
  &sys_tortls,
  &sys_process,

  &sys_orconn_event,
  &sys_ocirc_event,
  &sys_btrack,

  &sys_evloop,

  &sys_mainloop,
  &sys_or,

  &sys_relay,

#ifdef HAVE_MODULE_DIRAUTH
  &sys_dirauth,
#endif
};

const unsigned n_tor_subsystems = ARRAY_LENGTH(tor_subsystems);
