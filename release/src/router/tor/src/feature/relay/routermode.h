/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file routermode.h
 * \brief Header file for routermode.c.
 **/

#ifndef TOR_ROUTERMODE_H
#define TOR_ROUTERMODE_H

int dir_server_mode(const or_options_t *options);

MOCK_DECL(int, server_mode, (const or_options_t *options));
MOCK_DECL(int, public_server_mode, (const or_options_t *options));
MOCK_DECL(int, advertised_server_mode, (void));
int proxy_mode(const or_options_t *options);

void set_server_advertised(int s);

#endif /* !defined(TOR_ROUTERMODE_H) */
