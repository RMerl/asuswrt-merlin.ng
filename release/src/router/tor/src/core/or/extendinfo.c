/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file extendinfo.c
 * @brief Functions for creating and using extend_info_t objects.
 *
 * An extend_info_t is the information we hold about a relay in order to
 * extend a circuit to it.
 **/

#include "core/or/or.h"
#include "core/or/extendinfo.h"

#include "app/config/config.h"
#include "core/or/policies.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/nodelist.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "lib/crypt_ops/crypto_rand.h"

#include "core/or/extend_info_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

/** Allocate a new extend_info object based on the various arguments. */
extend_info_t *
extend_info_new(const char *nickname,
                const char *rsa_id_digest,
                const ed25519_public_key_t *ed_id,
                crypto_pk_t *onion_key,
                const curve25519_public_key_t *ntor_key,
                const tor_addr_t *addr, uint16_t port,
                const protover_summary_flags_t *pv,
                bool for_exit_use)
{
  extend_info_t *info = tor_malloc_zero(sizeof(extend_info_t));
  if (rsa_id_digest)
    memcpy(info->identity_digest, rsa_id_digest, DIGEST_LEN);
  if (ed_id && !ed25519_public_key_is_zero(ed_id))
    memcpy(&info->ed_identity, ed_id, sizeof(ed25519_public_key_t));
  if (nickname)
    strlcpy(info->nickname, nickname, sizeof(info->nickname));
  if (onion_key)
    info->onion_key = crypto_pk_dup_key(onion_key);
  if (ntor_key)
    memcpy(&info->curve25519_onion_key, ntor_key,
           sizeof(curve25519_public_key_t));
  for (int i = 0; i < EXTEND_INFO_MAX_ADDRS; ++i) {
    tor_addr_make_unspec(&info->orports[i].addr);
  }

  if (addr) {
    extend_info_add_orport(info, addr, port);
  }

  if (pv && for_exit_use) {
    info->exit_supports_congestion_control =
      pv->supports_congestion_control;
  }

  return info;
}

/**
 * Add another address:port pair to a given extend_info_t, if there is
 * room.  Return 0 on success, -1 on failure.
 **/
int
extend_info_add_orport(extend_info_t *ei,
                       const tor_addr_t *addr,
                       uint16_t port)
{
  for (int i = 0; i < EXTEND_INFO_MAX_ADDRS; ++i) {
    if (tor_addr_is_unspec(&ei->orports[i].addr)) {
      tor_addr_copy(&ei->orports[i].addr, addr);
      ei->orports[i].port = port;
      return 0;
    }
  }
  return -1;
}

/** Allocate and return a new extend_info that can be used to build a
 * circuit to or through the node <b>node</b>. Use the primary address
 * of the node (i.e. its IPv4 address) unless
 * <b>for_direct_connect</b> is true, in which case the preferred
 * address is used instead. May return NULL if there is not enough
 * info about <b>node</b> to extend to it--for example, if the preferred
 * routerinfo_t or microdesc_t is missing, or if for_direct_connect is
 * true and none of the node's addresses is allowed by tor's firewall
 * and IP version config.
 **/
extend_info_t *
extend_info_from_node(const node_t *node, int for_direct_connect,
                      bool for_exit)
{
  crypto_pk_t *rsa_pubkey = NULL;
  extend_info_t *info = NULL;
  tor_addr_port_t ap;
  int valid_addr = 0;

  if (!node_has_preferred_descriptor(node, for_direct_connect)) {
    return NULL;
  }

  /* Choose a preferred address first, but fall back to an allowed address. */
  if (for_direct_connect)
    reachable_addr_choose_from_node(node, FIREWALL_OR_CONNECTION, 0, &ap);
  else {
    node_get_prim_orport(node, &ap);
  }
  valid_addr = tor_addr_port_is_valid_ap(&ap, 0);

  if (valid_addr)
    log_debug(LD_CIRC, "using %s for %s",
              fmt_addrport(&ap.addr, ap.port),
              node->ri ? node->ri->nickname : node->rs->nickname);
  else
    log_warn(LD_CIRC, "Could not choose valid address for %s",
              node->ri ? node->ri->nickname : node->rs->nickname);

  /* Every node we connect or extend to must support ntor */
  if (!node_has_curve25519_onion_key(node)) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Attempted to create extend_info for a node that does not support "
           "ntor: %s", node_describe(node));
    return NULL;
  }

  const ed25519_public_key_t *ed_pubkey = NULL;

  /* Don't send the ed25519 pubkey unless the target node actually supports
   * authenticating with it. */
  if (node_supports_ed25519_link_authentication(node, 0)) {
    log_info(LD_CIRC, "Including Ed25519 ID for %s", node_describe(node));
    ed_pubkey = node_get_ed25519_id(node);
  } else if (node_get_ed25519_id(node)) {
    log_info(LD_CIRC, "Not including the ed25519 ID for %s, since it won't "
             "be able to authenticate it.",
             node_describe(node));
  }

  /* Retrieve the curve25519 pubkey. */
  const curve25519_public_key_t *curve_pubkey =
    node_get_curve25519_onion_key(node);
  rsa_pubkey = node_get_rsa_onion_key(node);

  if (valid_addr && node->ri) {
    info = extend_info_new(node->ri->nickname,
                           node->identity,
                           ed_pubkey,
                           rsa_pubkey,
                           curve_pubkey,
                           &ap.addr,
                           ap.port,
                           &node->ri->pv,
                           for_exit);
  } else if (valid_addr && node->rs && node->md) {
    info = extend_info_new(node->rs->nickname,
                           node->identity,
                           ed_pubkey,
                           rsa_pubkey,
                           curve_pubkey,
                           &ap.addr,
                           ap.port,
                           &node->rs->pv,
                           for_exit);
  }

  crypto_pk_free(rsa_pubkey);
  return info;
}

/** Release storage held by an extend_info_t struct. */
void
extend_info_free_(extend_info_t *info)
{
  if (!info)
    return;
  crypto_pk_free(info->onion_key);
  tor_free(info);
}

/** Allocate and return a new extend_info_t with the same contents as
 * <b>info</b>. */
extend_info_t *
extend_info_dup(extend_info_t *info)
{
  extend_info_t *newinfo;
  tor_assert(info);
  newinfo = tor_malloc(sizeof(extend_info_t));
  memcpy(newinfo, info, sizeof(extend_info_t));
  if (info->onion_key)
    newinfo->onion_key = crypto_pk_dup_key(info->onion_key);
  else
    newinfo->onion_key = NULL;
  return newinfo;
}

/* Does ei have a valid TAP key? */
int
extend_info_supports_tap(const extend_info_t* ei)
{
  tor_assert(ei);
  /* Valid TAP keys are not NULL */
  return ei->onion_key != NULL;
}

/* Does ei have a valid ntor key? */
int
extend_info_supports_ntor(const extend_info_t* ei)
{
  tor_assert(ei);
  /* Valid ntor keys have at least one non-zero byte */
  return !fast_mem_is_zero(
                          (const char*)ei->curve25519_onion_key.public_key,
                          CURVE25519_PUBKEY_LEN);
}

/** Return true if we can use the Ntor v3 handshake with `ei` */
int
extend_info_supports_ntor_v3(const extend_info_t *ei)
{
  tor_assert(ei);
  return extend_info_supports_ntor(ei) &&
    ei->exit_supports_congestion_control;
}

/* Does ei have an onion key which it would prefer to use?
 * Currently, we prefer ntor keys*/
int
extend_info_has_preferred_onion_key(const extend_info_t* ei)
{
  tor_assert(ei);
  return extend_info_supports_ntor(ei);
}

/** Return true iff the given address can be used to extend to. */
int
extend_info_addr_is_allowed(const tor_addr_t *addr)
{
  tor_assert(addr);

  /* Check if we have a private address and if we can extend to it. */
  if ((tor_addr_is_internal(addr, 0) || tor_addr_is_multicast(addr)) &&
      !get_options()->ExtendAllowPrivateAddresses) {
    goto disallow;
  }
  /* Allowed! */
  return 1;
 disallow:
  return 0;
}

/**
 * Return true if @a addr : @a port is a listed ORPort in @a ei.
 **/
bool
extend_info_has_orport(const extend_info_t *ei,
                       const tor_addr_t *addr, uint16_t port)
{
  IF_BUG_ONCE(ei == NULL) {
    return false;
  }

  for (int i = 0; i < EXTEND_INFO_MAX_ADDRS; ++i) {
    const tor_addr_port_t *ei_ap = &ei->orports[i];
    if (tor_addr_eq(&ei_ap->addr, addr) && ei_ap->port == port)
      return true;
  }
  return false;
}

/**
 * If the extend_info @a ei has an orport of the chosen family, then return
 * that orport.  Otherwise, return NULL.
 **/
const tor_addr_port_t *
extend_info_get_orport(const extend_info_t *ei, int family)
{
  for (int i = 0; i < EXTEND_INFO_MAX_ADDRS; ++i) {
    if (tor_addr_is_unspec(&ei->orports[i].addr))
      continue;
    if (tor_addr_family(&ei->orports[i].addr) == family)
      return &ei->orports[i];
  }
  return NULL;
}

/**
 * Chose an addr_port_t within @a ei to connect to.
 **/
const tor_addr_port_t *
extend_info_pick_orport(const extend_info_t *ei)
{
  IF_BUG_ONCE(!ei) {
    return NULL;
  }
  const or_options_t *options = get_options();
  if (!server_mode(options)) {
    // If we aren't a server, just pick the first address we built into
    // this extendinfo.
    return &ei->orports[0];
  }

  const bool ipv6_ok = router_can_extend_over_ipv6(options);

  // Use 'usable' to collect the usable orports, then pick one.
  const tor_addr_port_t *usable[EXTEND_INFO_MAX_ADDRS];
  int n_usable = 0;
  for (int i = 0; i < EXTEND_INFO_MAX_ADDRS; ++i) {
    const tor_addr_port_t *a = &ei->orports[i];
    const int family = tor_addr_family(&a->addr);
    if (family == AF_INET || (ipv6_ok && family == AF_INET6)) {
      usable[n_usable++] = a;
    }
  }

  if (n_usable == 0) {
    // Need to bail out early, since nothing will work.
    return NULL;
  }

  crypto_fast_rng_t *rng = get_thread_fast_rng();
  const int idx = crypto_fast_rng_get_uint(rng, n_usable);

  return usable[idx];
}

/**
 * Return true if any orport address in @a ei is an internal address.
 **/
bool
extend_info_any_orport_addr_is_internal(const extend_info_t *ei)
{
  IF_BUG_ONCE(ei == NULL) {
    return false;
  }

  for (int i = 0; i < EXTEND_INFO_MAX_ADDRS; ++i) {
    if (! tor_addr_is_unspec(&ei->orports[i].addr) &&
        tor_addr_is_internal(&ei->orports[i].addr, 0))
      return true;
  }
  return false;
}
