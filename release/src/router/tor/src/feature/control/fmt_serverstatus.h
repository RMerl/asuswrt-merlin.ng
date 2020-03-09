/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file fmt_serverstatus.h
 * \brief Header file for fmt_serverstatus.c.
 **/

#ifndef TOR_FMT_SERVERSTATUS_H
#define TOR_FMT_SERVERSTATUS_H

int list_server_status_v1(smartlist_t *routers, char **router_status_out,
                          int for_controller);

#endif /* !defined(TOR_FMT_SERVERSTATUS_H) */
