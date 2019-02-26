/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"

#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"

/** Copy the primary (IPv4) OR port (IP address and TCP port) for
 * <b>router</b> into *<b>ap_out</b>. */
void
router_get_prim_orport(const routerinfo_t *router, tor_addr_port_t *ap_out)
{
  tor_assert(ap_out != NULL);
  tor_addr_from_ipv4h(&ap_out->addr, router->addr);
  ap_out->port = router->or_port;
}

int
router_has_orport(const routerinfo_t *router, const tor_addr_port_t *orport)
{
  return
    (tor_addr_eq_ipv4h(&orport->addr, router->addr) &&
     orport->port == router->or_port) ||
    (tor_addr_eq(&orport->addr, &router->ipv6_addr) &&
     orport->port == router->ipv6_orport);
}

/** Return a smartlist of tor_addr_port_t's with all the OR ports of
    <b>ri</b>. Note that freeing of the items in the list as well as
    the smartlist itself is the callers responsibility. */
smartlist_t *
router_get_all_orports(const routerinfo_t *ri)
{
  tor_assert(ri);
  node_t fake_node;
  memset(&fake_node, 0, sizeof(fake_node));
  /* we don't modify ri, fake_node is passed as a const node_t *
   */
  fake_node.ri = (routerinfo_t *)ri;
  return node_get_all_orports(&fake_node);
}

/** Given a router purpose, convert it to a string.  Don't call this on
 * ROUTER_PURPOSE_UNKNOWN: The whole point of that value is that we don't
 * know its string representation. */
const char *
router_purpose_to_string(uint8_t p)
{
  switch (p)
    {
    case ROUTER_PURPOSE_GENERAL: return "general";
    case ROUTER_PURPOSE_BRIDGE: return "bridge";
    case ROUTER_PURPOSE_CONTROLLER: return "controller";
    default:
      tor_assert(0);
    }
  return NULL;
}

/** Given a string, convert it to a router purpose. */
uint8_t
router_purpose_from_string(const char *s)
{
  if (!strcmp(s, "general"))
    return ROUTER_PURPOSE_GENERAL;
  else if (!strcmp(s, "bridge"))
    return ROUTER_PURPOSE_BRIDGE;
  else if (!strcmp(s, "controller"))
    return ROUTER_PURPOSE_CONTROLLER;
  else
    return ROUTER_PURPOSE_UNKNOWN;
}
