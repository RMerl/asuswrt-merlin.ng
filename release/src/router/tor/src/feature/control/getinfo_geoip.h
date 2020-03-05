/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_GETINFO_GEOIP_H
#define TOR_GETINFO_GEOIP_H

int getinfo_helper_geoip(control_connection_t *control_conn,
                     const char *question, char **answer,
                     const char **errmsg);

#endif /* !defined(TOR_GETINFO_GEOIP_H) */
