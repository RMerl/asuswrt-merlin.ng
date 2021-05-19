/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file bridges.c
 * \brief Code to manage bridges and bridge selection.
 *
 * Bridges are fixed entry nodes, used for censorship circumvention.
 **/

#define TOR_BRIDGES_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/circuitbuild.h"
#include "core/or/policies.h"
#include "feature/client/bridges.h"
#include "feature/client/entrynodes.h"
#include "feature/client/transports.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirclient/dlstatus.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"

#include "core/or/extend_info_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/microdesc_st.h"

/** Information about a configured bridge. Currently this just matches the
 * ones in the torrc file, but one day we may be able to learn about new
 * bridges on our own, and remember them in the state file. */
struct bridge_info_t {
  /** Address and port of the bridge, as configured by the user.*/
  tor_addr_port_t addrport_configured;
  /** Address of the bridge. */
  tor_addr_t addr;
  /** TLS port for the bridge. */
  uint16_t port;
  /** Boolean: We are re-parsing our bridge list, and we are going to remove
   * this one if we don't find it in the list of configured bridges. */
  unsigned marked_for_removal : 1;
  /** Expected identity digest, or all zero bytes if we don't know what the
   * digest should be. */
  char identity[DIGEST_LEN];

  /** Name of pluggable transport protocol taken from its config line. */
  char *transport_name;

  /** When should we next try to fetch a descriptor for this bridge? */
  download_status_t fetch_status;

  /** A smartlist of k=v values to be passed to the SOCKS proxy, if
      transports are used for this bridge. */
  smartlist_t *socks_args;
};

#define bridge_free(bridge) \
  FREE_AND_NULL(bridge_info_t, bridge_free_, (bridge))

static void bridge_free_(bridge_info_t *bridge);
static void rewrite_node_address_for_bridge(const bridge_info_t *bridge,
                                            node_t *node);

/** A list of configured bridges. Whenever we actually get a descriptor
 * for one, we add it as an entry guard.  Note that the order of bridges
 * in this list does not necessarily correspond to the order of bridges
 * in the torrc. */
static smartlist_t *bridge_list = NULL;

/** Mark every entry of the bridge list to be removed on our next call to
 * sweep_bridge_list unless it has first been un-marked. */
void
mark_bridge_list(void)
{
  if (!bridge_list)
    bridge_list = smartlist_new();
  SMARTLIST_FOREACH(bridge_list, bridge_info_t *, b,
                    b->marked_for_removal = 1);
}

/** Remove every entry of the bridge list that was marked with
 * mark_bridge_list if it has not subsequently been un-marked. */
void
sweep_bridge_list(void)
{
  if (!bridge_list)
    bridge_list = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, b) {
    if (b->marked_for_removal) {
      SMARTLIST_DEL_CURRENT(bridge_list, b);
      bridge_free(b);
    }
  } SMARTLIST_FOREACH_END(b);
}

/** Initialize the bridge list to empty, creating it if needed. */
STATIC void
clear_bridge_list(void)
{
  if (!bridge_list)
    bridge_list = smartlist_new();
  SMARTLIST_FOREACH(bridge_list, bridge_info_t *, b, bridge_free(b));
  smartlist_clear(bridge_list);
}

/** Free the bridge <b>bridge</b>. */
static void
bridge_free_(bridge_info_t *bridge)
{
  if (!bridge)
    return;

  tor_free(bridge->transport_name);
  if (bridge->socks_args) {
    SMARTLIST_FOREACH(bridge->socks_args, char*, s, tor_free(s));
    smartlist_free(bridge->socks_args);
  }

  tor_free(bridge);
}

/** Return a list of all the configured bridges, as bridge_info_t pointers. */
const smartlist_t *
bridge_list_get(void)
{
  if (!bridge_list)
    bridge_list = smartlist_new();
  return bridge_list;
}

/**
 * Given a <b>bridge</b>, return a pointer to its RSA identity digest, or
 * NULL if we don't know one for it.
 */
const uint8_t *
bridge_get_rsa_id_digest(const bridge_info_t *bridge)
{
  tor_assert(bridge);
  if (tor_digest_is_zero(bridge->identity))
    return NULL;
  else
    return (const uint8_t *) bridge->identity;
}

/**
 * Given a <b>bridge</b>, return a pointer to its configured addr:port
 * combination.
 */
const tor_addr_port_t *
bridge_get_addr_port(const bridge_info_t *bridge)
{
  tor_assert(bridge);
  return &bridge->addrport_configured;
}

/**
 * Given a <b>bridge</b>, return the transport name. If none were configured,
 * NULL is returned.
 */
const char *
bridget_get_transport_name(const bridge_info_t *bridge)
{
  tor_assert(bridge);
  return bridge->transport_name;
}

/**
 * Return true if @a bridge has a transport name for which we don't actually
 * know a transport.
 */
bool
bridge_has_invalid_transport(const bridge_info_t *bridge)
{
  const char *tname = bridget_get_transport_name(bridge);
  return tname && transport_get_by_name(tname) == NULL;
}

/** If we have a bridge configured whose digest matches <b>digest</b>, or a
 * bridge with no known digest whose address matches any of the
 * tor_addr_port_t's in <b>orports</b>, return that bridge.  Else return
 * NULL. */
STATIC bridge_info_t *
get_configured_bridge_by_orports_digest(const char *digest,
                                        const smartlist_t *orports)
{
  if (!bridge_list)
    return NULL;
  SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, bridge)
    {
      if (tor_digest_is_zero(bridge->identity)) {
        SMARTLIST_FOREACH_BEGIN(orports, tor_addr_port_t *, ap)
          {
            if (tor_addr_compare(&bridge->addr, &ap->addr, CMP_EXACT) == 0 &&
                bridge->port == ap->port)
              return bridge;
          }
        SMARTLIST_FOREACH_END(ap);
      }
      if (digest && tor_memeq(bridge->identity, digest, DIGEST_LEN))
        return bridge;
    }
  SMARTLIST_FOREACH_END(bridge);
  return NULL;
}

/** If we have a bridge configured whose digest matches <b>digest</b>, or a
 * bridge with no known digest whose address matches <b>addr</b>:<b>port</b>,
 * return that bridge.  Else return NULL. If <b>digest</b> is NULL, check for
 * address/port matches only. */
bridge_info_t *
get_configured_bridge_by_addr_port_digest(const tor_addr_t *addr,
                                          uint16_t port,
                                          const char *digest)
{
  if (!bridge_list)
    return NULL;
  SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, bridge)
    {
      if ((tor_digest_is_zero(bridge->identity) || digest == NULL) &&
          !tor_addr_compare(&bridge->addr, addr, CMP_EXACT) &&
          bridge->port == port)
        return bridge;
      if (digest && tor_memeq(bridge->identity, digest, DIGEST_LEN))
        return bridge;
    }
  SMARTLIST_FOREACH_END(bridge);
  return NULL;
}

/**
 * As get_configured_bridge_by_addr_port, but require that the
 * address match <b>addr</b>:<b>port</b>, and that the ID digest match
 * <b>digest</b>.  (The other function will ignore the address if the
 * digest matches.)
 */
bridge_info_t *
get_configured_bridge_by_exact_addr_port_digest(const tor_addr_t *addr,
                                                uint16_t port,
                                                const char *digest)
{
  if (!bridge_list)
    return NULL;
  SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, bridge) {
    if (!tor_addr_compare(&bridge->addr, addr, CMP_EXACT) &&
        bridge->port == port) {

      if (digest && tor_memeq(bridge->identity, digest, DIGEST_LEN))
        return bridge;
      else if (!digest || tor_digest_is_zero(bridge->identity))
        return bridge;
    }

  } SMARTLIST_FOREACH_END(bridge);
  return NULL;
}

/** If we have a bridge configured whose digest matches <b>digest</b>, or a
 * bridge with no known digest whose address matches <b>addr</b>:<b>port</b>,
 * return 1.  Else return 0. If <b>digest</b> is NULL, check for
 * address/port matches only. */
int
addr_is_a_configured_bridge(const tor_addr_t *addr,
                            uint16_t port,
                            const char *digest)
{
  tor_assert(addr);
  return get_configured_bridge_by_addr_port_digest(addr, port, digest) ? 1 : 0;
}

/** If we have a bridge configured whose digest matches
 * <b>ei->identity_digest</b>, or a bridge with no known digest whose address
 * matches <b>ei->addr</b>:<b>ei->port</b>, return 1.  Else return 0.
 * If <b>ei->onion_key</b> is NULL, check for address/port matches only.
 *
 * Note that if the extend_info_t contains multiple addresses, we return true
 * only if _every_ address is a bridge.
 */
int
extend_info_is_a_configured_bridge(const extend_info_t *ei)
{
  const char *digest = ei->onion_key ? ei->identity_digest : NULL;
  const tor_addr_port_t *ap1 = NULL, *ap2 = NULL;
  if (! tor_addr_is_null(&ei->orports[0].addr))
    ap1 = &ei->orports[0];
  if (! tor_addr_is_null(&ei->orports[1].addr))
    ap2 = &ei->orports[1];
  IF_BUG_ONCE(ap1 == NULL) {
    return 0;
  }
  return addr_is_a_configured_bridge(&ap1->addr, ap1->port, digest) &&
    (ap2 == NULL ||
     addr_is_a_configured_bridge(&ap2->addr, ap2->port, digest));
}

/** Wrapper around get_configured_bridge_by_addr_port_digest() to look
 * it up via router descriptor <b>ri</b>. */
static bridge_info_t *
get_configured_bridge_by_routerinfo(const routerinfo_t *ri)
{
  bridge_info_t *bi = NULL;
  smartlist_t *orports = router_get_all_orports(ri);
  bi = get_configured_bridge_by_orports_digest(ri->cache_info.identity_digest,
                                               orports);
  SMARTLIST_FOREACH(orports, tor_addr_port_t *, p, tor_free(p));
  smartlist_free(orports);
  return bi;
}

/** Return 1 if <b>ri</b> is one of our known bridges, else 0. */
int
routerinfo_is_a_configured_bridge(const routerinfo_t *ri)
{
  return get_configured_bridge_by_routerinfo(ri) ? 1 : 0;
}

/**
 * Return 1 iff <b>bridge_list</b> contains entry matching given
 * <b>addr</b> and <b>port</b> (and no identity digest) OR
 * it contains an  entry whose identity matches <b>digest</b>.
 * Otherwise, return 0.
 */
static int
bridge_exists_with_addr_and_port(const tor_addr_t *addr,
                                 const uint16_t port,
                                 const char *digest)
{
  if (!tor_addr_port_is_valid(addr, port, 0))
    return 0;

  bridge_info_t *bridge =
   get_configured_bridge_by_addr_port_digest(addr, port, digest);

  return (bridge != NULL);
}

/** Return 1 if <b>node</b> is one of our configured bridges, else 0.
 * More specifically, return 1 iff: a bridge_info_t object exists in
 * <b>bridge_list</b> such that: 1) It's identity is equal to node
 * identity OR 2) It's identity digest is zero, but it matches
 * address and port of any ORPort in the node.
 */
int
node_is_a_configured_bridge(const node_t *node)
{
  /* First, let's try searching for a bridge with matching identity. */
  if (BUG(fast_mem_is_zero(node->identity, DIGEST_LEN)))
    return 0;

  if (find_bridge_by_digest(node->identity) != NULL)
    return 1;

  /* At this point, we have established that no bridge exists with
   * matching identity digest. However, we still pass it into
   * bridge_exists_* functions because we want further code to
   * check for absence of identity digest in a bridge.
   */
  if (node->ri) {
    if (bridge_exists_with_addr_and_port(&node->ri->ipv4_addr,
                                         node->ri->ipv4_orport,
                                         node->identity))
      return 1;

    if (bridge_exists_with_addr_and_port(&node->ri->ipv6_addr,
                                         node->ri->ipv6_orport,
                                         node->identity))
      return 1;
  } else if (node->rs) {
    if (bridge_exists_with_addr_and_port(&node->rs->ipv4_addr,
                                         node->rs->ipv4_orport,
                                         node->identity))
      return 1;

    if (bridge_exists_with_addr_and_port(&node->rs->ipv6_addr,
                                         node->rs->ipv6_orport,
                                         node->identity))
      return 1;
  }  else if (node->md) {
    if (bridge_exists_with_addr_and_port(&node->md->ipv6_addr,
                                         node->md->ipv6_orport,
                                         node->identity))
      return 1;
  }

  return 0;
}

/** We made a connection to a router at <b>addr</b>:<b>port</b>
 * without knowing its digest. Its digest turned out to be <b>digest</b>.
 * If it was a bridge, and we still don't know its digest, record it.
 */
void
learned_router_identity(const tor_addr_t *addr, uint16_t port,
                        const char *digest,
                        const ed25519_public_key_t *ed_id)
{
  // XXXX prop220 use ed_id here, once there is some way to specify
  (void)ed_id;
  int learned = 0;
  bridge_info_t *bridge =
    get_configured_bridge_by_exact_addr_port_digest(addr, port, digest);
  if (bridge && tor_digest_is_zero(bridge->identity)) {
    memcpy(bridge->identity, digest, DIGEST_LEN);
    learned = 1;
  }
  /* XXXX prop220 remember bridge ed25519 identities -- add a field */
#if 0
  if (bridge && ed_id &&
      ed25519_public_key_is_zero(&bridge->ed25519_identity) &&
      !ed25519_public_key_is_zero(ed_id)) {
    memcpy(&bridge->ed25519_identity, ed_id, sizeof(*ed_id));
    learned = 1;
  }
#endif /* 0 */
  if (learned) {
    char *transport_info = NULL;
    const char *transport_name =
      find_transport_name_by_bridge_addrport(addr, port);
    if (transport_name)
      tor_asprintf(&transport_info, " (with transport '%s')", transport_name);

    // XXXX prop220 log both fingerprints.
    log_notice(LD_DIR, "Learned fingerprint %s for bridge %s%s.",
               hex_str(digest, DIGEST_LEN), fmt_addrport(addr, port),
               transport_info ? transport_info : "");
    tor_free(transport_info);
    entry_guard_learned_bridge_identity(&bridge->addrport_configured,
                                        (const uint8_t *)digest);
  }
}

/** Return true if <b>bridge</b> has the same identity digest as
 *  <b>digest</b>. If <b>digest</b> is NULL, it matches
 *  bridges with unspecified identity digests. */
static int
bridge_has_digest(const bridge_info_t *bridge, const char *digest)
{
  if (digest)
    return tor_memeq(digest, bridge->identity, DIGEST_LEN);
  else
    return tor_digest_is_zero(bridge->identity);
}

/** We are about to add a new bridge at <b>addr</b>:<b>port</b>, with optional
 * <b>digest</b> and <b>transport_name</b>. Mark for removal any previously
 * existing bridge with the same address and port, and warn the user as
 * appropriate.
 */
STATIC void
bridge_resolve_conflicts(const tor_addr_t *addr, uint16_t port,
                         const char *digest, const char *transport_name)
{
  /* Iterate the already-registered bridge list:

     If you find a bridge with the same address and port, mark it for
     removal. It doesn't make sense to have two active bridges with
     the same IP:PORT. If the bridge in question has a different
     digest or transport than <b>digest</b>/<b>transport_name</b>,
     it's probably a misconfiguration and we should warn the user.
  */
  SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, bridge) {
    if (bridge->marked_for_removal)
      continue;

    if (tor_addr_eq(&bridge->addr, addr) && (bridge->port == port)) {

      bridge->marked_for_removal = 1;

      if (!bridge_has_digest(bridge, digest) ||
          strcmp_opt(bridge->transport_name, transport_name)) {
        /* warn the user */
        char *bridge_description_new, *bridge_description_old;
        tor_asprintf(&bridge_description_new, "%s:%s:%s",
                     fmt_addrport(addr, port),
                     digest ? hex_str(digest, DIGEST_LEN) : "",
                     transport_name ? transport_name : "");
        tor_asprintf(&bridge_description_old, "%s:%s:%s",
                     fmt_addrport(&bridge->addr, bridge->port),
                     tor_digest_is_zero(bridge->identity) ?
                     "" : hex_str(bridge->identity,DIGEST_LEN),
                     bridge->transport_name ? bridge->transport_name : "");

        log_warn(LD_GENERAL,"Tried to add bridge '%s', but we found a conflict"
                 " with the already registered bridge '%s'. We will discard"
                 " the old bridge and keep '%s'. If this is not what you"
                 " wanted, please change your configuration file accordingly.",
                 bridge_description_new, bridge_description_old,
                 bridge_description_new);

        tor_free(bridge_description_new);
        tor_free(bridge_description_old);
      }
    }
  } SMARTLIST_FOREACH_END(bridge);
}

/** Return True if we have a bridge that uses a transport with name
 *  <b>transport_name</b>. */
MOCK_IMPL(int,
transport_is_needed, (const char *transport_name))
{
  if (!bridge_list)
    return 0;

  SMARTLIST_FOREACH_BEGIN(bridge_list, const bridge_info_t *, bridge) {
    if (bridge->transport_name &&
        !strcmp(bridge->transport_name, transport_name))
      return 1;
  } SMARTLIST_FOREACH_END(bridge);

  return 0;
}

/** Register the bridge information in <b>bridge_line</b> to the
 *  bridge subsystem. Steals reference of <b>bridge_line</b>. */
void
bridge_add_from_config(bridge_line_t *bridge_line)
{
  bridge_info_t *b;

  // XXXX prop220 add a way to specify ed25519 ID to bridge_line_t.

  { /* Log the bridge we are about to register: */
    log_debug(LD_GENERAL, "Registering bridge at %s (transport: %s) (%s)",
              fmt_addrport(&bridge_line->addr, bridge_line->port),
              bridge_line->transport_name ?
              bridge_line->transport_name : "no transport",
              tor_digest_is_zero(bridge_line->digest) ?
              "no key listed" : hex_str(bridge_line->digest, DIGEST_LEN));

    if (bridge_line->socks_args) { /* print socks arguments */
      int i = 0;

      tor_assert(smartlist_len(bridge_line->socks_args) > 0);

      log_debug(LD_GENERAL, "Bridge uses %d SOCKS arguments:",
                smartlist_len(bridge_line->socks_args));
      SMARTLIST_FOREACH(bridge_line->socks_args, const char *, arg,
                        log_debug(LD_CONFIG, "%d: %s", ++i, arg));
    }
  }

  bridge_resolve_conflicts(&bridge_line->addr,
                           bridge_line->port,
                           bridge_line->digest,
                           bridge_line->transport_name);

  b = tor_malloc_zero(sizeof(bridge_info_t));
  tor_addr_copy(&b->addrport_configured.addr, &bridge_line->addr);
  b->addrport_configured.port = bridge_line->port;
  tor_addr_copy(&b->addr, &bridge_line->addr);
  b->port = bridge_line->port;
  memcpy(b->identity, bridge_line->digest, DIGEST_LEN);
  if (bridge_line->transport_name)
    b->transport_name = bridge_line->transport_name;
  b->fetch_status.schedule = DL_SCHED_BRIDGE;
  b->fetch_status.increment_on = DL_SCHED_INCREMENT_ATTEMPT;
  /* We can't reset the bridge's download status here, because UseBridges
   * might be 0 now, and it might be changed to 1 much later. */
  b->socks_args = bridge_line->socks_args;
  if (!bridge_list)
    bridge_list = smartlist_new();

  tor_free(bridge_line); /* Deallocate bridge_line now. */

  smartlist_add(bridge_list, b);
}

/** If <b>digest</b> is one of our known bridges, return it. */
STATIC bridge_info_t *
find_bridge_by_digest(const char *digest)
{
  if (! bridge_list)
    return NULL;
  SMARTLIST_FOREACH(bridge_list, bridge_info_t *, bridge,
    {
      if (tor_memeq(bridge->identity, digest, DIGEST_LEN))
        return bridge;
    });
  return NULL;
}

/** Given the <b>addr</b> and <b>port</b> of a bridge, if that bridge
 *  supports a pluggable transport, return its name. Otherwise, return
 *  NULL. */
const char *
find_transport_name_by_bridge_addrport(const tor_addr_t *addr, uint16_t port)
{
  if (!bridge_list)
    return NULL;

  SMARTLIST_FOREACH_BEGIN(bridge_list, const bridge_info_t *, bridge) {
    if (tor_addr_eq(&bridge->addr, addr) &&
        (bridge->port == port))
      return bridge->transport_name;
  } SMARTLIST_FOREACH_END(bridge);

  return NULL;
}

/** If <b>addr</b> and <b>port</b> match the address and port of a
 * bridge of ours that uses pluggable transports, place its transport
 * in <b>transport</b>.
 *
 * Return 0 on success (found a transport, or found a bridge with no
 * transport, or found no bridge); return -1 if we should be using a
 * transport, but the transport could not be found.
 */
int
get_transport_by_bridge_addrport(const tor_addr_t *addr, uint16_t port,
                                 const transport_t **transport)
{
  *transport = NULL;
  if (!bridge_list)
    return 0;

  SMARTLIST_FOREACH_BEGIN(bridge_list, const bridge_info_t *, bridge) {
    if (tor_addr_eq(&bridge->addr, addr) &&
        (bridge->port == port)) { /* bridge matched */
      if (bridge->transport_name) { /* it also uses pluggable transports */
        *transport = transport_get_by_name(bridge->transport_name);
        if (*transport == NULL) { /* it uses pluggable transports, but
                                     the transport could not be found! */
          return -1;
        }
        return 0;
      } else { /* bridge matched, but it doesn't use transports. */
        break;
      }
    }
  } SMARTLIST_FOREACH_END(bridge);

  *transport = NULL;
  return 0;
}

/** Return a smartlist containing all the SOCKS arguments that we
 *  should pass to the SOCKS proxy. */
const smartlist_t *
get_socks_args_by_bridge_addrport(const tor_addr_t *addr, uint16_t port)
{
  bridge_info_t *bridge = get_configured_bridge_by_addr_port_digest(addr,
                                                                    port,
                                                                    NULL);
  return bridge ? bridge->socks_args : NULL;
}

/** We need to ask <b>bridge</b> for its server descriptor. */
static void
launch_direct_bridge_descriptor_fetch(bridge_info_t *bridge)
{
  const or_options_t *options = get_options();
  circuit_guard_state_t *guard_state = NULL;

  if (connection_get_by_type_addr_port_purpose(
      CONN_TYPE_DIR, &bridge->addr, bridge->port,
      DIR_PURPOSE_FETCH_SERVERDESC))
    return; /* it's already on the way */

  if (bridge_has_invalid_transport(bridge)) {
    download_status_mark_impossible(&bridge->fetch_status);
    log_warn(LD_CONFIG, "Can't use bridge at %s: there is no configured "
             "transport called \"%s\".",
             safe_str_client(fmt_and_decorate_addr(&bridge->addr)),
             bridget_get_transport_name(bridge));
    return; /* Can't use this bridge; it has not */
  }

  if (routerset_contains_bridge(options->ExcludeNodes, bridge)) {
    download_status_mark_impossible(&bridge->fetch_status);
    log_warn(LD_APP, "Not using bridge at %s: it is in ExcludeNodes.",
             safe_str_client(fmt_and_decorate_addr(&bridge->addr)));
    return;
  }

  /* Until we get a descriptor for the bridge, we only know one address for
   * it. */
  if (!reachable_addr_allows_addr(&bridge->addr, bridge->port,
                                            FIREWALL_OR_CONNECTION, 0, 0)) {
    log_notice(LD_CONFIG, "Tried to fetch a descriptor directly from a "
               "bridge, but that bridge is not reachable through our "
               "firewall.");
    return;
  }

  /* If we already have a node_t for this bridge, rewrite its address now. */
  node_t *node = node_get_mutable_by_id(bridge->identity);
  if (node) {
    rewrite_node_address_for_bridge(bridge, node);
  }

  tor_addr_port_t bridge_addrport;
  memcpy(&bridge_addrport.addr, &bridge->addr, sizeof(tor_addr_t));
  bridge_addrport.port = bridge->port;

  guard_state = get_guard_state_for_bridge_desc_fetch(bridge->identity);

  directory_request_t *req =
    directory_request_new(DIR_PURPOSE_FETCH_SERVERDESC);
  directory_request_set_or_addr_port(req, &bridge_addrport);
  directory_request_set_directory_id_digest(req, bridge->identity);
  directory_request_set_router_purpose(req, ROUTER_PURPOSE_BRIDGE);
  directory_request_set_resource(req, "authority.z");
  if (guard_state) {
    directory_request_set_guard_state(req, guard_state);
  }
  directory_initiate_request(req);
  directory_request_free(req);
}

/** Fetching the bridge descriptor from the bridge authority returned a
 * "not found". Fall back to trying a direct fetch. */
void
retry_bridge_descriptor_fetch_directly(const char *digest)
{
  bridge_info_t *bridge = find_bridge_by_digest(digest);
  if (!bridge)
    return; /* not found? oh well. */

  launch_direct_bridge_descriptor_fetch(bridge);
}

/** For each bridge in our list for which we don't currently have a
 * descriptor, fetch a new copy of its descriptor -- either directly
 * from the bridge or via a bridge authority. */
void
fetch_bridge_descriptors(const or_options_t *options, time_t now)
{
  int num_bridge_auths = get_n_authorities(BRIDGE_DIRINFO);
  int ask_bridge_directly;
  int can_use_bridge_authority;

  if (!bridge_list)
    return;

  /* If we still have unconfigured managed proxies, don't go and
     connect to a bridge. */
  if (pt_proxies_configuration_pending())
    return;

  SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, bridge)
    {
      /* This resets the download status on first use */
      if (!download_status_is_ready(&bridge->fetch_status, now))
        continue; /* don't bother, no need to retry yet */
      if (routerset_contains_bridge(options->ExcludeNodes, bridge)) {
        download_status_mark_impossible(&bridge->fetch_status);
        log_warn(LD_APP, "Not using bridge at %s: it is in ExcludeNodes.",
                 safe_str_client(fmt_and_decorate_addr(&bridge->addr)));
        continue;
      }

      /* schedule the next attempt
       * we can't increment after a failure, because sometimes we use the
       * bridge authority, and sometimes we use the bridge direct */
      download_status_increment_attempt(
                        &bridge->fetch_status,
                        safe_str_client(fmt_and_decorate_addr(&bridge->addr)),
                        now);

      can_use_bridge_authority = !tor_digest_is_zero(bridge->identity) &&
                                 num_bridge_auths;
      ask_bridge_directly = !can_use_bridge_authority ||
                            !options->UpdateBridgesFromAuthority;
      log_debug(LD_DIR, "ask_bridge_directly=%d (%d, %d, %d)",
                ask_bridge_directly, tor_digest_is_zero(bridge->identity),
                !options->UpdateBridgesFromAuthority, !num_bridge_auths);

      if (ask_bridge_directly &&
          !reachable_addr_allows_addr(&bridge->addr, bridge->port,
                                                FIREWALL_OR_CONNECTION, 0,
                                                0)) {
        log_notice(LD_DIR, "Bridge at '%s' isn't reachable by our "
                   "firewall policy. %s.",
                   fmt_addrport(&bridge->addr, bridge->port),
                   can_use_bridge_authority ?
                     "Asking bridge authority instead" : "Skipping");
        if (can_use_bridge_authority)
          ask_bridge_directly = 0;
        else
          continue;
      }

      if (ask_bridge_directly) {
        /* we need to ask the bridge itself for its descriptor. */
        launch_direct_bridge_descriptor_fetch(bridge);
      } else {
        /* We have a digest and we want to ask an authority. We could
         * combine all the requests into one, but that may give more
         * hints to the bridge authority than we want to give. */
        char resource[10 + HEX_DIGEST_LEN];
        memcpy(resource, "fp/", 3);
        base16_encode(resource+3, HEX_DIGEST_LEN+1,
                      bridge->identity, DIGEST_LEN);
        memcpy(resource+3+HEX_DIGEST_LEN, ".z", 3);
        log_info(LD_DIR, "Fetching bridge info '%s' from bridge authority.",
                 resource);
        directory_get_from_dirserver(DIR_PURPOSE_FETCH_SERVERDESC,
                ROUTER_PURPOSE_BRIDGE, resource, 0, DL_WANT_AUTHORITY);
      }
    }
  SMARTLIST_FOREACH_END(bridge);
}

/** If our <b>bridge</b> is configured to be a different address than
 * the bridge gives in <b>node</b>, rewrite the routerinfo
 * we received to use the address we meant to use. Now we handle
 * multihomed bridges better.
 */
static void
rewrite_node_address_for_bridge(const bridge_info_t *bridge, node_t *node)
{
  /* XXXX move this function. */
  /* XXXX overridden addresses should really live in the node_t, so that the
   *   routerinfo_t and the microdesc_t can be immutable.  But we can only
   *   do that safely if we know that no function that connects to an OR
   *   does so through an address from any source other than node_get_addr().
   */
  const or_options_t *options = get_options();

  if (node->ri) {
    routerinfo_t *ri = node->ri;
    if ((!tor_addr_compare(&bridge->addr, &ri->ipv4_addr, CMP_EXACT) &&
         bridge->port == ri->ipv4_orport) ||
        (!tor_addr_compare(&bridge->addr, &ri->ipv6_addr, CMP_EXACT) &&
         bridge->port == ri->ipv6_orport)) {
      /* they match, so no need to do anything */
    } else {
      if (tor_addr_family(&bridge->addr) == AF_INET) {
        tor_addr_copy(&ri->ipv4_addr, &bridge->addr);
        ri->ipv4_orport = bridge->port;
        log_info(LD_DIR,
                 "Adjusted bridge routerinfo for '%s' to match configured "
                 "address %s:%d.",
                 ri->nickname, fmt_addr(&ri->ipv4_addr), ri->ipv4_orport);
      } else if (tor_addr_family(&bridge->addr) == AF_INET6) {
        tor_addr_copy(&ri->ipv6_addr, &bridge->addr);
        ri->ipv6_orport = bridge->port;
        log_info(LD_DIR,
                 "Adjusted bridge routerinfo for '%s' to match configured "
                 "address %s.",
                 ri->nickname, fmt_addrport(&ri->ipv6_addr, ri->ipv6_orport));
      } else {
        log_err(LD_BUG, "Address family not supported: %d.",
                tor_addr_family(&bridge->addr));
        return;
      }
    }

    if (options->ClientPreferIPv6ORPort == -1) {
      /* Mark which address to use based on which bridge_t we got. */
      node->ipv6_preferred = (tor_addr_family(&bridge->addr) == AF_INET6 &&
                              !tor_addr_is_null(&node->ri->ipv6_addr));
    } else {
      /* Mark which address to use based on user preference */
      node->ipv6_preferred = (reachable_addr_prefer_ipv6_orport(options) &&
                              !tor_addr_is_null(&node->ri->ipv6_addr));
    }

    /* XXXipv6 we lack support for falling back to another address for
       the same relay, warn the user */
    if (!tor_addr_is_null(&ri->ipv6_addr)) {
      tor_addr_port_t ap;
      node_get_pref_orport(node, &ap);
      log_notice(LD_CONFIG,
                 "Bridge '%s' has both an IPv4 and an IPv6 address.  "
                 "Will prefer using its %s address (%s) based on %s.",
                 ri->nickname,
                 node->ipv6_preferred ? "IPv6" : "IPv4",
                 fmt_addrport(&ap.addr, ap.port),
                 options->ClientPreferIPv6ORPort == -1 ?
                 "the configured Bridge address" :
                 "ClientPreferIPv6ORPort");
    }
  }
  if (node->rs) {
    routerstatus_t *rs = node->rs;

    if ((!tor_addr_compare(&bridge->addr, &rs->ipv4_addr, CMP_EXACT) &&
        bridge->port == rs->ipv4_orport) ||
       (!tor_addr_compare(&bridge->addr, &rs->ipv6_addr, CMP_EXACT) &&
        bridge->port == rs->ipv6_orport)) {
      /* they match, so no need to do anything */
    } else {
      if (tor_addr_family(&bridge->addr) == AF_INET) {
        tor_addr_copy(&rs->ipv4_addr, &bridge->addr);
        rs->ipv4_orport = bridge->port;
        log_info(LD_DIR,
                 "Adjusted bridge routerstatus for '%s' to match "
                 "configured address %s.",
                 rs->nickname, fmt_addrport(&bridge->addr, rs->ipv4_orport));
      /* set IPv6 preferences even if there is no ri */
      } else if (tor_addr_family(&bridge->addr) == AF_INET6) {
        tor_addr_copy(&rs->ipv6_addr, &bridge->addr);
        rs->ipv6_orport = bridge->port;
        log_info(LD_DIR,
                 "Adjusted bridge routerstatus for '%s' to match configured"
                 " address %s.",
                 rs->nickname, fmt_addrport(&rs->ipv6_addr, rs->ipv6_orport));
      } else {
        log_err(LD_BUG, "Address family not supported: %d.",
                tor_addr_family(&bridge->addr));
        return;
      }
    }

    if (options->ClientPreferIPv6ORPort == -1) {
      /* Mark which address to use based on which bridge_t we got. */
      node->ipv6_preferred = (tor_addr_family(&bridge->addr) == AF_INET6 &&
                              !tor_addr_is_null(&node->rs->ipv6_addr));
    } else {
      /* Mark which address to use based on user preference */
      node->ipv6_preferred = (reachable_addr_prefer_ipv6_orport(options) &&
                              !tor_addr_is_null(&node->rs->ipv6_addr));
    }

    /* XXXipv6 we lack support for falling back to another address for
    the same relay, warn the user */
    if (!tor_addr_is_null(&rs->ipv6_addr)) {
      tor_addr_port_t ap;
      node_get_pref_orport(node, &ap);
      log_notice(LD_CONFIG,
                 "Bridge '%s' has both an IPv4 and an IPv6 address.  "
                 "Will prefer using its %s address (%s) based on %s.",
                 rs->nickname,
                 node->ipv6_preferred ? "IPv6" : "IPv4",
                 fmt_addrport(&ap.addr, ap.port),
                 options->ClientPreferIPv6ORPort == -1 ?
                 "the configured Bridge address" :
                 "ClientPreferIPv6ORPort");
    }
  }
}

/** We just learned a descriptor for a bridge. See if that
 * digest is in our entry guard list, and add it if not. */
void
learned_bridge_descriptor(routerinfo_t *ri, int from_cache)
{
  tor_assert(ri);
  tor_assert(ri->purpose == ROUTER_PURPOSE_BRIDGE);
  if (get_options()->UseBridges) {
    /* Retry directory downloads whenever we get a bridge descriptor:
     * - when bootstrapping, and
     * - when we aren't sure if any of our bridges are reachable.
     * Keep on retrying until we have at least one reachable bridge. */
    int first = num_bridges_usable(0) < 1;
    bridge_info_t *bridge = get_configured_bridge_by_routerinfo(ri);
    time_t now = time(NULL);
    router_set_status(ri->cache_info.identity_digest, 1);

    if (bridge) { /* if we actually want to use this one */
      node_t *node;
      /* it's here; schedule its re-fetch for a long time from now. */
      if (!from_cache) {
        /* This schedules the re-fetch at a constant interval, which produces
         * a pattern of bridge traffic. But it's better than trying all
         * configured bridges several times in the first few minutes. */
        download_status_reset(&bridge->fetch_status);
      }

      node = node_get_mutable_by_id(ri->cache_info.identity_digest);
      tor_assert(node);
      rewrite_node_address_for_bridge(bridge, node);
      if (tor_digest_is_zero(bridge->identity)) {
        memcpy(bridge->identity,ri->cache_info.identity_digest, DIGEST_LEN);
        log_notice(LD_DIR, "Learned identity %s for bridge at %s:%d",
                   hex_str(bridge->identity, DIGEST_LEN),
                   fmt_and_decorate_addr(&bridge->addr),
                   (int) bridge->port);
      }
      entry_guard_learned_bridge_identity(&bridge->addrport_configured,
                              (const uint8_t*)ri->cache_info.identity_digest);

      log_notice(LD_DIR, "new bridge descriptor '%s' (%s): %s", ri->nickname,
                 from_cache ? "cached" : "fresh", router_describe(ri));
      /* If we didn't have a reachable bridge before this one, try directory
       * documents again. */
      if (first) {
        routerlist_retry_directory_downloads(now);
      }
    }
  }
}

/** Return a smartlist containing all bridge identity digests */
MOCK_IMPL(smartlist_t *,
list_bridge_identities, (void))
{
  smartlist_t *result = NULL;
  char *digest_tmp;

  if (get_options()->UseBridges && bridge_list) {
    result = smartlist_new();

    SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, b) {
      digest_tmp = tor_malloc(DIGEST_LEN);
      memcpy(digest_tmp, b->identity, DIGEST_LEN);
      smartlist_add(result, digest_tmp);
    } SMARTLIST_FOREACH_END(b);
  }

  return result;
}

/** Get the download status for a bridge descriptor given its identity */
MOCK_IMPL(download_status_t *,
get_bridge_dl_status_by_id, (const char *digest))
{
  download_status_t *dl = NULL;

  if (digest && get_options()->UseBridges && bridge_list) {
    SMARTLIST_FOREACH_BEGIN(bridge_list, bridge_info_t *, b) {
      if (tor_memeq(digest, b->identity, DIGEST_LEN)) {
        dl = &(b->fetch_status);
        break;
      }
    } SMARTLIST_FOREACH_END(b);
  }

  return dl;
}

/** Release all storage held in bridges.c */
void
bridges_free_all(void)
{
  clear_bridge_list();
  smartlist_free(bridge_list);
  bridge_list = NULL;
}
