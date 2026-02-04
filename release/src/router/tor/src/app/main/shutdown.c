/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2024, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file shutdown.c
 * @brief Code to free global resources used by Tor.
 *
 * In the future, this should all be handled by the subsystem manager. */

#include "core/or/or.h"

#include "app/config/config.h"
#include "app/config/statefile.h"
#include "app/main/main.h"
#include "app/main/shutdown.h"
#include "app/main/subsysmgr.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop_pubsub.h"
#include "core/mainloop/cpuworker.h"
#include "core/or/channeltls.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitmux_ewma.h"
#include "core/or/circuitpadding.h"
#include "core/or/conflux_pool.h"
#include "core/or/connection_edge.h"
#include "core/or/dos.h"
#include "core/or/scheduler.h"
#include "feature/client/addressmap.h"
#include "feature/client/bridges.h"
#include "feature/client/entrynodes.h"
#include "feature/client/transports.h"
#include "feature/control/control.h"
#include "feature/control/control_auth.h"
#include "feature/dirauth/authmode.h"
#include "feature/dirauth/shared_random.h"
#include "feature/dircache/consdiffmgr.h"
#include "feature/dircache/dirserv.h"
#include "feature/dirparse/routerparse.h"
#include "feature/hibernate/hibernate.h"
#include "feature/hs/hs_common.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/ext_orport.h"
#include "feature/relay/relay_config.h"
#include "feature/stats/bwhist.h"
#include "feature/stats/geoip_stats.h"
#include "feature/stats/rephist.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/geoip/geoip.h"

void evdns_shutdown(int);

/** Do whatever cleanup is necessary before shutting Tor down. */
void
tor_cleanup(void)
{
  const or_options_t *options = get_options();
  if (options->command == CMD_RUN_TOR) {
    time_t now = time(NULL);
    /* Remove our pid file. We don't care if there was an error when we
     * unlink, nothing we could do about it anyways. */
    tor_remove_file(options->PidFile);
    /* Remove control port file */
    tor_remove_file(options->ControlPortWriteToFile);
    /* Remove cookie authentication file */
    {
      char *cookie_fname = get_controller_cookie_file_name();
      tor_remove_file(cookie_fname);
      tor_free(cookie_fname);
    }
    /* Remove Extended ORPort cookie authentication file */
    {
      char *cookie_fname = get_ext_or_auth_cookie_file_name();
      if (cookie_fname)
        tor_remove_file(cookie_fname);
      tor_free(cookie_fname);
    }
    if (accounting_is_enabled(options))
      accounting_record_bandwidth_usage(now, get_or_state());
    or_state_mark_dirty(get_or_state(), 0); /* force an immediate save. */
    or_state_save(now);
    if (authdir_mode(options)) {
      sr_save_and_cleanup();
    }
    if (authdir_mode_tests_reachability(options))
      rep_hist_record_mtbf_data(now, 0);
  }

  timers_shutdown();

  tor_free_all(0); /* We could move tor_free_all back into the ifdef below
                      later, if it makes shutdown unacceptably slow.  But for
                      now, leave it here: it's helped us catch bugs in the
                      past. */
}

/** Free all memory that we might have allocated somewhere.
 * If <b>postfork</b>, we are a worker process and we want to free
 * only the parts of memory that we won't touch. If !<b>postfork</b>,
 * Tor is shutting down and we should free everything.
 *
 * Helps us find the real leaks with sanitizers and the like. Also valgrind
 * should then report 0 reachable in its leak report (in an ideal world --
 * in practice libevent, SSL, libc etc never quite free everything). */
void
tor_free_all(int postfork)
{
  if (!postfork) {
    evdns_shutdown(1);
  }
  cpuworker_free_all();
  geoip_free_all();
  geoip_stats_free_all();
  routerlist_free_all();
  networkstatus_free_all();
  addressmap_free_all();
  dirserv_free_all();
  rep_hist_free_all();
  bwhist_free_all();
  circuit_free_all();
  conflux_pool_free_all();
  circpad_machines_free();
  entry_guards_free_all();
  pt_free_all();
  channel_tls_free_all();
  channel_free_all();
  connection_free_all();
  connection_edge_free_all();
  scheduler_free_all();
  nodelist_free_all();
  microdesc_free_all();
  routerparse_free_all();
  control_free_all();
  bridges_free_all();
  consdiffmgr_free_all();
  hs_free_all();
  dos_free_all();
  circuitmux_ewma_free_all();
  accounting_free_all();
  circpad_free_all();

  if (!postfork) {
    config_free_all();
    relay_config_free_all();
    or_state_free_all();
  }
  if (!postfork) {
#ifndef _WIN32
    tor_getpwnam(NULL);
#endif
  }
  /* stuff in main.c */

  tor_mainloop_disconnect_pubsub();

  if (!postfork) {
    release_lockfile();
  }

  subsystems_shutdown();

  /* Stuff in util.c and address.c*/
  if (!postfork) {
    esc_router_info(NULL);
  }
}
