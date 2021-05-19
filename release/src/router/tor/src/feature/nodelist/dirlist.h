/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dirlist.h
 * \brief Header file for dirlist.c
 **/

#ifndef TOR_DIRLIST_H
#define TOR_DIRLIST_H

int get_n_authorities(dirinfo_type_t type);
const smartlist_t *router_get_trusted_dir_servers(void);
const smartlist_t *router_get_fallback_dir_servers(void);
smartlist_t *router_get_trusted_dir_servers_mutable(void);
smartlist_t *router_get_fallback_dir_servers_mutable(void);
void mark_all_dirservers_up(smartlist_t *server_list);

dir_server_t *router_get_trusteddirserver_by_digest(const char *d);
dir_server_t *router_get_fallback_dirserver_by_digest(
                                                   const char *digest);
int router_digest_is_fallback_dir(const char *digest);
MOCK_DECL(dir_server_t *, trusteddirserver_get_by_v3_auth_digest,
          (const char *d));

MOCK_DECL(int, router_digest_is_trusted_dir_type,
        (const char *digest, dirinfo_type_t type));

bool router_addr_is_trusted_dir_type(const tor_addr_t *addr,
                                     dirinfo_type_t type);
#define router_addr_is_trusted_dir(d) \
  router_addr_is_trusted_dir_type((d), NO_DIRINFO)

#define router_digest_is_trusted_dir(d) \
  router_digest_is_trusted_dir_type((d), NO_DIRINFO)

dir_server_t *trusted_dir_server_new(const char *nickname, const char *address,
                       uint16_t dir_port, uint16_t or_port,
                       const tor_addr_port_t *addrport_ipv6,
                       const char *digest, const char *v3_auth_digest,
                       dirinfo_type_t type, double weight);
dir_server_t *fallback_dir_server_new(const tor_addr_t *addr,
                                      uint16_t dir_port, uint16_t or_port,
                                      const tor_addr_port_t *addrport_ipv6,
                                      const char *id_digest, double weight);
void dir_server_add(dir_server_t *ent);

void clear_dir_servers(void);
void dirlist_free_all(void);

MOCK_DECL(void, dirlist_add_trusted_dir_addresses, (void));

#endif /* !defined(TOR_DIRLIST_H) */
