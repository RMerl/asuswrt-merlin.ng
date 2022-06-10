/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dirclient_modes.c
 * @brief Functions to answer questions about how we'd like to behave
 *     as a directory client
 **/

#include "orconfig.h"

#include "core/or/or.h"

#include "feature/dirclient/dirclient_modes.h"
#include "feature/dircache/dirserv.h"
#include "feature/relay/relay_find_addr.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "feature/stats/predict_ports.h"

#include "app/config/or_options_st.h"
#include "feature/nodelist/routerinfo_st.h"

/* Should this tor instance only use begindir for all its directory requests?
 */
int
dirclient_must_use_begindir(const or_options_t *options)
{
  /* Clients, onion services, and bridges must use begindir,
   * relays and authorities do not have to */
  return !public_server_mode(options);
}

/** Return 1 if we fetch our directory material directly from the
 * authorities, rather than from a mirror. */
int
dirclient_fetches_from_authorities(const or_options_t *options)
{
  const routerinfo_t *me;
  int refuseunknown;
  if (options->FetchDirInfoEarly)
    return 1;
  if (options->BridgeRelay == 1)
    return 0;
  refuseunknown = ! router_my_exit_policy_is_reject_star() &&
    should_refuse_unknown_exits(options);
  if (!dir_server_mode(options) && !refuseunknown)
    return 0;
  if (!server_mode(options) || !advertised_server_mode())
    return 0;
  me = router_get_my_routerinfo();
  if (!me || (!me->supports_tunnelled_dir_requests && !refuseunknown))
    return 0; /* if we don't service directory requests, return 0 too */
  return 1;
}

/** Return 1 if we should fetch new networkstatuses, descriptors, etc
 * on the "mirror" schedule rather than the "client" schedule.
 */
int
dirclient_fetches_dir_info_early(const or_options_t *options)
{
  return dirclient_fetches_from_authorities(options);
}

/** Return 1 if we should fetch new networkstatuses, descriptors, etc
 * on a very passive schedule -- waiting long enough for ordinary clients
 * to probably have the info we want. These would include bridge users,
 * and maybe others in the future e.g. if a Tor client uses another Tor
 * client as a directory guard.
 */
int
dirclient_fetches_dir_info_later(const or_options_t *options)
{
  return options->UseBridges != 0;
}

/** Return 1 if we have no need to fetch new descriptors. This generally
 * happens when we're not a dir cache and we haven't built any circuits
 * lately.
 */
int
dirclient_too_idle_to_fetch_descriptors(const or_options_t *options,
                                        time_t now)
{
  return !directory_caches_dir_info(options) &&
         !options->FetchUselessDescriptors &&
         rep_hist_circbuilding_dormant(now);
}
