/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file getinfo_geoip.c
 * @brief GEOIP-related controller GETINFO commands.
 **/

#include "core/or/or.h"
#include "core/mainloop/connection.h"
#include "feature/control/control.h"
#include "feature/control/getinfo_geoip.h"
#include "lib/geoip/geoip.h"

/** Helper used to implement GETINFO ip-to-country/... controller command. */
int
getinfo_helper_geoip(control_connection_t *control_conn,
                     const char *question, char **answer,
                     const char **errmsg)
{
  (void)control_conn;
  if (!strcmpstart(question, "ip-to-country/")) {
    int c;
    sa_family_t family;
    tor_addr_t addr;
    question += strlen("ip-to-country/");

    if (!strcmp(question, "ipv4-available") ||
        !strcmp(question, "ipv6-available")) {
      family = !strcmp(question, "ipv4-available") ? AF_INET : AF_INET6;
      const int available = geoip_is_loaded(family);
      tor_asprintf(answer, "%d", !! available);
      return 0;
    }

    family = tor_addr_parse(&addr, question);
    if (family != AF_INET && family != AF_INET6) {
      *errmsg = "Invalid address family";
      return -1;
    }
    if (!geoip_is_loaded(family)) {
      *errmsg = "GeoIP data not loaded";
      return -1;
    }
    c = geoip_get_country_by_addr(&addr);
    *answer = tor_strdup(geoip_get_country_name(c));
  }
  return 0;
}
