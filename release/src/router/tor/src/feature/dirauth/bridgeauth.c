/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file bridgeauth.c
 * @brief Bridge authority code
 **/

#include "core/or/or.h"
#include "feature/dirauth/bridgeauth.h"
#include "feature/dirauth/voteflags.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/relay/router.h"
#include "app/config/config.h"

#include "feature/nodelist/routerinfo_st.h"

/** Write out router status entries for all our bridge descriptors. Here, we
 * also mark routers as running. */
void
bridgeauth_dump_bridge_status_to_file(time_t now)
{
  char *status;
  char *fname = NULL;
  char *thresholds = NULL;
  char *published_thresholds_and_status = NULL;
  char published[ISO_TIME_LEN+1];
  const routerinfo_t *me = router_get_my_routerinfo();
  char fingerprint[FINGERPRINT_LEN+1];
  char *fingerprint_line = NULL;

  dirserv_set_bridges_running(now);
  status = networkstatus_getinfo_by_purpose("bridge", now);

  if (me && crypto_pk_get_fingerprint(me->identity_pkey,
                                      fingerprint, 0) >= 0) {
    tor_asprintf(&fingerprint_line, "fingerprint %s\n", fingerprint);
  } else {
    log_warn(LD_BUG, "Error computing fingerprint for bridge status.");
  }
  format_iso_time(published, now);
  dirserv_compute_bridge_flag_thresholds();
  thresholds = dirserv_get_flag_thresholds_line();
  tor_asprintf(&published_thresholds_and_status,
               "published %s\nflag-thresholds %s\n%s%s",
               published, thresholds, fingerprint_line ? fingerprint_line : "",
               status);
  fname = get_datadir_fname("networkstatus-bridges");
  if (write_str_to_file(fname,published_thresholds_and_status,0)<0) {
    log_warn(LD_DIRSERV, "Unable to write networkstatus-bridges file.");
  }
  tor_free(thresholds);
  tor_free(published_thresholds_and_status);
  tor_free(fname);
  tor_free(status);
  tor_free(fingerprint_line);
}
