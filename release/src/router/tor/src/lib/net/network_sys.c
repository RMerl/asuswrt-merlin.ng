/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file network_sys.c
 * \brief Subsystem object for networking setup.
 **/

#include "orconfig.h"
#include "lib/subsys/subsys.h"
#include "lib/net/network_sys.h"
#include "lib/net/resolve.h"
#include "lib/net/socket.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

static int
subsys_network_initialize(void)
{
  if (network_init() < 0)
    return -1;

  return 0;
}

static void
subsys_network_shutdown(void)
{
#ifdef _WIN32
  WSACleanup();
#endif
  tor_free_getaddrinfo_cache();
}

const subsys_fns_t sys_network = {
  .name = "network",
  SUBSYS_DECLARE_LOCATION(),
  /* Network depends on logging, and a lot of other modules depend on network.
   */
  .level = -55,
  .supported = true,
  .initialize = subsys_network_initialize,
  .shutdown = subsys_network_shutdown,
};
