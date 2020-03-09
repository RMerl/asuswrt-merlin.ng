/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file routerinfo.h
 * \brief Header file for routerinfo.c.
 **/

#ifndef TOR_ROUTERINFO_H
#define TOR_ROUTERINFO_H

void router_get_prim_orport(const routerinfo_t *router,
                            tor_addr_port_t *addr_port_out);
int router_has_orport(const routerinfo_t *router,
                      const tor_addr_port_t *orport);

smartlist_t *router_get_all_orports(const routerinfo_t *ri);

const char *router_purpose_to_string(uint8_t p);
uint8_t router_purpose_from_string(const char *s);

#endif /* !defined(TOR_ROUTERINFO_H) */
