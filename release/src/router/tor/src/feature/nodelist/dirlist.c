/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dirlist.c
 * \brief Code to maintain our lists of directory authorities and
 *    fallback directories.
 *
 * For the directory authorities, we have a list containing the public
 * identity key, and contact points, for each authority.  The
 * authorities receive descriptors from relays, and publish consensuses,
 * descriptors, and microdescriptors.  This list is pre-configured.
 *
 * Fallback directories are well-known, stable, but untrusted directory
 * caches that clients which have not yet bootstrapped can use to get
 * their first networkstatus consensus, in order to find out where the
 * Tor network really is.  This list is pre-configured in
 * fallback_dirs.inc.  Every authority also serves as a fallback.
 *
 * Both fallback directories and directory authorities are are
 * represented by a dir_server_t.
 */

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/or/policies.h"
#include "feature/control/control_events.h"
#include "feature/dirauth/authmode.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/relay/router.h"
#include "lib/net/resolve.h"

#include "feature/dirclient/dir_server_st.h"
#include "feature/nodelist/node_st.h"

/** Global list of a dir_server_t object for each directory
 * authority. */
static smartlist_t *trusted_dir_servers = NULL;
/** Global list of dir_server_t objects for all directory authorities
 * and all fallback directory servers. */
static smartlist_t *fallback_dir_servers = NULL;

/** Return the number of directory authorities whose type matches some bit set
 * in <b>type</b>  */
int
get_n_authorities(dirinfo_type_t type)
{
  int n = 0;
  if (!trusted_dir_servers)
    return 0;
  SMARTLIST_FOREACH(trusted_dir_servers, dir_server_t *, ds,
                    if (ds->type & type)
                      ++n);
  return n;
}

/** Return a smartlist containing a list of dir_server_t * for all
 * known trusted dirservers.  Callers must not modify the list or its
 * contents.
 */
smartlist_t *
router_get_trusted_dir_servers_mutable(void)
{
  if (!trusted_dir_servers)
    trusted_dir_servers = smartlist_new();

  return trusted_dir_servers;
}

smartlist_t *
router_get_fallback_dir_servers_mutable(void)
{
  if (!fallback_dir_servers)
    fallback_dir_servers = smartlist_new();

  return fallback_dir_servers;
}

const smartlist_t *
router_get_trusted_dir_servers(void)
{
  return router_get_trusted_dir_servers_mutable();
}

const smartlist_t *
router_get_fallback_dir_servers(void)
{
  return router_get_fallback_dir_servers_mutable();
}

/** Reset all internal variables used to count failed downloads of network
 * status objects. */
void
router_reset_status_download_failures(void)
{
  mark_all_dirservers_up(fallback_dir_servers);
}

/** Return the dir_server_t for the directory authority whose identity
 * key hashes to <b>digest</b>, or NULL if no such authority is known.
 */
dir_server_t *
router_get_trusteddirserver_by_digest(const char *digest)
{
  if (!trusted_dir_servers)
    return NULL;

  SMARTLIST_FOREACH(trusted_dir_servers, dir_server_t *, ds,
     {
       if (tor_memeq(ds->digest, digest, DIGEST_LEN))
         return ds;
     });

  return NULL;
}

/** Return the dir_server_t for the fallback dirserver whose identity
 * key hashes to <b>digest</b>, or NULL if no such fallback is in the list of
 * fallback_dir_servers. (fallback_dir_servers is affected by the FallbackDir
 * and UseDefaultFallbackDirs torrc options.)
 * The list of fallback directories includes the list of authorities.
 */
dir_server_t *
router_get_fallback_dirserver_by_digest(const char *digest)
{
  if (!fallback_dir_servers)
    return NULL;

  if (!digest)
    return NULL;

  SMARTLIST_FOREACH(fallback_dir_servers, dir_server_t *, ds,
     {
       if (tor_memeq(ds->digest, digest, DIGEST_LEN))
         return ds;
     });

  return NULL;
}

/** Return 1 if any fallback dirserver's identity key hashes to <b>digest</b>,
 * or 0 if no such fallback is in the list of fallback_dir_servers.
 * (fallback_dir_servers is affected by the FallbackDir and
 * UseDefaultFallbackDirs torrc options.)
 * The list of fallback directories includes the list of authorities.
 */
int
router_digest_is_fallback_dir(const char *digest)
{
  return (router_get_fallback_dirserver_by_digest(digest) != NULL);
}

/** Return the dir_server_t for the directory authority whose
 * v3 identity key hashes to <b>digest</b>, or NULL if no such authority
 * is known.
 */
MOCK_IMPL(dir_server_t *,
trusteddirserver_get_by_v3_auth_digest, (const char *digest))
{
  if (!trusted_dir_servers)
    return NULL;

  SMARTLIST_FOREACH(trusted_dir_servers, dir_server_t *, ds,
     {
       if (tor_memeq(ds->v3_identity_digest, digest, DIGEST_LEN) &&
           (ds->type & V3_DIRINFO))
         return ds;
     });

  return NULL;
}

/** Mark as running every dir_server_t in <b>server_list</b>. */
void
mark_all_dirservers_up(smartlist_t *server_list)
{
  if (server_list) {
    SMARTLIST_FOREACH_BEGIN(server_list, dir_server_t *, dir) {
      routerstatus_t *rs;
      node_t *node;
      dir->is_running = 1;
      node = node_get_mutable_by_id(dir->digest);
      if (node)
        node->is_running = 1;
      rs = router_get_mutable_consensus_status_by_id(dir->digest);
      if (rs) {
        rs->last_dir_503_at = 0;
        control_event_networkstatus_changed_single(rs);
      }
    } SMARTLIST_FOREACH_END(dir);
  }
  router_dir_info_changed();
}

/** Return true iff <b>digest</b> is the digest of the identity key of a
 * trusted directory matching at least one bit of <b>type</b>.  If <b>type</b>
 * is zero (NO_DIRINFO), or ALL_DIRINFO, any authority is okay. */
int
router_digest_is_trusted_dir_type(const char *digest, dirinfo_type_t type)
{
  if (!trusted_dir_servers)
    return 0;
  if (authdir_mode(get_options()) && router_digest_is_me(digest))
    return 1;
  SMARTLIST_FOREACH(trusted_dir_servers, dir_server_t *, ent,
    if (tor_memeq(digest, ent->digest, DIGEST_LEN)) {
      return (!type) || ((type & ent->type) != 0);
    });
  return 0;
}

/** Create a directory server at <b>address</b>:<b>port</b>, with OR identity
 * key <b>digest</b> which has DIGEST_LEN bytes.  If <b>address</b> is NULL,
 * add ourself.  If <b>is_authority</b>, this is a directory authority.  Return
 * the new directory server entry on success or NULL on failure. */
static dir_server_t *
dir_server_new(int is_authority,
               const char *nickname,
               const tor_addr_t *addr,
               const char *hostname,
               uint16_t dir_port, uint16_t or_port,
               const tor_addr_port_t *addrport_ipv6,
               const char *digest, const char *v3_auth_digest,
               dirinfo_type_t type,
               double weight)
{
  dir_server_t *ent;
  uint32_t a;
  char *hostname_ = NULL;

  tor_assert(digest);

  if (weight < 0)
    return NULL;

  if (tor_addr_family(addr) == AF_INET)
    a = tor_addr_to_ipv4h(addr);
  else
    return NULL;

  if (!hostname)
    hostname_ = tor_addr_to_str_dup(addr);
  else
    hostname_ = tor_strdup(hostname);

  ent = tor_malloc_zero(sizeof(dir_server_t));
  ent->nickname = nickname ? tor_strdup(nickname) : NULL;
  ent->address = hostname_;
  ent->addr = a;
  ent->dir_port = dir_port;
  ent->or_port = or_port;
  ent->is_running = 1;
  ent->is_authority = is_authority;
  ent->type = type;
  ent->weight = weight;
  if (addrport_ipv6) {
    if (tor_addr_family(&addrport_ipv6->addr) != AF_INET6) {
      log_warn(LD_BUG, "Hey, I got a non-ipv6 addr as addrport_ipv6.");
      tor_addr_make_unspec(&ent->ipv6_addr);
    } else {
      tor_addr_copy(&ent->ipv6_addr, &addrport_ipv6->addr);
      ent->ipv6_orport = addrport_ipv6->port;
    }
  } else {
    tor_addr_make_unspec(&ent->ipv6_addr);
  }

  memcpy(ent->digest, digest, DIGEST_LEN);
  if (v3_auth_digest && (type & V3_DIRINFO))
    memcpy(ent->v3_identity_digest, v3_auth_digest, DIGEST_LEN);

  if (nickname)
    tor_asprintf(&ent->description, "directory server \"%s\" at %s:%d",
                 nickname, hostname_, (int)dir_port);
  else
    tor_asprintf(&ent->description, "directory server at %s:%d",
                 hostname_, (int)dir_port);

  ent->fake_status.addr = ent->addr;
  tor_addr_copy(&ent->fake_status.ipv6_addr, &ent->ipv6_addr);
  memcpy(ent->fake_status.identity_digest, digest, DIGEST_LEN);
  if (nickname)
    strlcpy(ent->fake_status.nickname, nickname,
            sizeof(ent->fake_status.nickname));
  else
    ent->fake_status.nickname[0] = '\0';
  ent->fake_status.dir_port = ent->dir_port;
  ent->fake_status.or_port = ent->or_port;
  ent->fake_status.ipv6_orport = ent->ipv6_orport;

  return ent;
}

/** Create an authoritative directory server at
 * <b>address</b>:<b>port</b>, with identity key <b>digest</b>.  If
 * <b>address</b> is NULL, add ourself.  Return the new trusted directory
 * server entry on success or NULL if we couldn't add it. */
dir_server_t *
trusted_dir_server_new(const char *nickname, const char *address,
                       uint16_t dir_port, uint16_t or_port,
                       const tor_addr_port_t *ipv6_addrport,
                       const char *digest, const char *v3_auth_digest,
                       dirinfo_type_t type, double weight)
{
  uint32_t a;
  tor_addr_t addr;
  char *hostname=NULL;
  dir_server_t *result;

  if (!address) { /* The address is us; we should guess. */
    if (resolve_my_address(LOG_WARN, get_options(),
                           &a, NULL, &hostname) < 0) {
      log_warn(LD_CONFIG,
               "Couldn't find a suitable address when adding ourself as a "
               "trusted directory server.");
      return NULL;
    }
    if (!hostname)
      hostname = tor_dup_ip(a);
  } else {
    if (tor_lookup_hostname(address, &a)) {
      log_warn(LD_CONFIG,
               "Unable to lookup address for directory server at '%s'",
               address);
      return NULL;
    }
    hostname = tor_strdup(address);
  }
  tor_addr_from_ipv4h(&addr, a);

  result = dir_server_new(1, nickname, &addr, hostname,
                          dir_port, or_port,
                          ipv6_addrport,
                          digest,
                          v3_auth_digest, type, weight);
  tor_free(hostname);
  return result;
}

/** Return a new dir_server_t for a fallback directory server at
 * <b>addr</b>:<b>or_port</b>/<b>dir_port</b>, with identity key digest
 * <b>id_digest</b> */
dir_server_t *
fallback_dir_server_new(const tor_addr_t *addr,
                        uint16_t dir_port, uint16_t or_port,
                        const tor_addr_port_t *addrport_ipv6,
                        const char *id_digest, double weight)
{
  return dir_server_new(0, NULL, addr, NULL, dir_port, or_port,
                        addrport_ipv6,
                        id_digest,
                        NULL, ALL_DIRINFO, weight);
}

/** Add a directory server to the global list(s). */
void
dir_server_add(dir_server_t *ent)
{
  if (!trusted_dir_servers)
    trusted_dir_servers = smartlist_new();
  if (!fallback_dir_servers)
    fallback_dir_servers = smartlist_new();

  if (ent->is_authority)
    smartlist_add(trusted_dir_servers, ent);

  smartlist_add(fallback_dir_servers, ent);
  router_dir_info_changed();
}

#define dir_server_free(val) \
  FREE_AND_NULL(dir_server_t, dir_server_free_, (val))

/** Free storage held in <b>ds</b>. */
static void
dir_server_free_(dir_server_t *ds)
{
  if (!ds)
    return;

  tor_free(ds->nickname);
  tor_free(ds->description);
  tor_free(ds->address);
  tor_free(ds);
}

/** Remove all members from the list of dir servers. */
void
clear_dir_servers(void)
{
  if (fallback_dir_servers) {
    SMARTLIST_FOREACH(fallback_dir_servers, dir_server_t *, ent,
                      dir_server_free(ent));
    smartlist_clear(fallback_dir_servers);
  } else {
    fallback_dir_servers = smartlist_new();
  }
  if (trusted_dir_servers) {
    smartlist_clear(trusted_dir_servers);
  } else {
    trusted_dir_servers = smartlist_new();
  }
  router_dir_info_changed();
}

void
dirlist_free_all(void)
{
  clear_dir_servers();
  smartlist_free(trusted_dir_servers);
  smartlist_free(fallback_dir_servers);
  trusted_dir_servers = fallback_dir_servers = NULL;
}
