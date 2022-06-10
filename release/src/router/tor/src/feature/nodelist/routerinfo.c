/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file routerinfo.c
 * @brief Manipulate full router descriptors.
 **/

#include "core/or/or.h"

#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/torcert.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"

/** Copy the OR port (IP address and TCP port) for <b>router</b> and
 * <b>family</b> into *<b>ap_out</b>.
 *
 * If the requested ORPort does not exist, sets *<b>ap_out</b> to the null
 * address and port, and returns -1. Otherwise, returns 0. */
int
router_get_orport(const routerinfo_t *router,
                  tor_addr_port_t *ap_out,
                  int family)
{
  tor_assert(ap_out != NULL);
  if (family == AF_INET) {
    tor_addr_copy(&ap_out->addr, &router->ipv4_addr);
    ap_out->port = router->ipv4_orport;
    return 0;
  } else if (family == AF_INET6) {
    /* IPv6 addresses are optional, so check if it is valid. */
    if (tor_addr_port_is_valid(&router->ipv6_addr, router->ipv6_orport, 0)) {
      tor_addr_copy(&ap_out->addr, &router->ipv6_addr);
      ap_out->port = router->ipv6_orport;
      return 0;
    } else {
      tor_addr_port_make_null_ap(ap_out, AF_INET6);
      return -1;
    }
  } else {
    /* Unsupported address family */
    tor_assert_nonfatal_unreached();
    tor_addr_port_make_null_ap(ap_out, AF_UNSPEC);
    return -1;
  }
}

int
router_has_orport(const routerinfo_t *router, const tor_addr_port_t *orport)
{
  return
    (tor_addr_eq(&orport->addr, &router->ipv4_addr) &&
     orport->port == router->ipv4_orport) ||
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

/** Return the Ed25519 identity key for this routerinfo, or NULL if it
 * doesn't have one. */
const ed25519_public_key_t *
routerinfo_get_ed25519_id(const routerinfo_t *ri)
{
  if (BUG(! ri))
    return NULL;

  const tor_cert_t *cert = ri->cache_info.signing_key_cert;
  if (cert && ! ed25519_public_key_is_zero(&cert->signing_key))
    return &cert->signing_key;
  else
    return NULL;
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
