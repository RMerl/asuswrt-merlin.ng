/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file describe.h
 * \brief Header file for describe.c.
 **/

#ifndef TOR_DESCRIBE_H
#define TOR_DESCRIBE_H

struct extend_info_t;
struct node_t;
struct routerinfo_t;
struct routerstatus_t;

const char *extend_info_describe(const struct extend_info_t *ei);
const char *node_describe(const struct node_t *node);
const char *router_describe(const struct routerinfo_t *ri);
const char *routerstatus_describe(const struct routerstatus_t *ri);

#endif
