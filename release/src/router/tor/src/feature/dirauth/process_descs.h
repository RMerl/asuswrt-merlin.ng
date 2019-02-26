/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file process_descs.h
 * \brief Header file for process_descs.c.
 **/

#ifndef TOR_RECV_UPLOADS_H
#define TOR_RECV_UPLOADS_H

int dirserv_load_fingerprint_file(void);
void dirserv_free_fingerprint_list(void);
int dirserv_add_own_fingerprint(crypto_pk_t *pk);

enum was_router_added_t dirserv_add_multiple_descriptors(
                                     const char *desc, uint8_t purpose,
                                     const char *source,
                                     const char **msg);
enum was_router_added_t dirserv_add_descriptor(routerinfo_t *ri,
                                               const char **msg,
                                               const char *source);

int authdir_wants_to_reject_router(routerinfo_t *ri, const char **msg,
                                   int complain,
                                   int *valid_out);
uint32_t dirserv_router_get_status(const routerinfo_t *router,
                                   const char **msg,
                                   int severity);
void dirserv_set_node_flags_from_authoritative_status(node_t *node,
                                                      uint32_t authstatus);

int dirserv_would_reject_router(const routerstatus_t *rs);

#endif
