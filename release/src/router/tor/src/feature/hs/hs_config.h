/* Copyright (c) 2016-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_config.h
 * \brief Header file containing configuration ABI/API for the HS subsytem.
 **/

#ifndef TOR_HS_CONFIG_H
#define TOR_HS_CONFIG_H

#include "core/or/or.h"

/* Max value for HiddenServiceMaxStreams */
#define HS_CONFIG_MAX_STREAMS_PER_RDV_CIRCUIT 65535
/* Maximum number of intro points per version 3 services. */
#define HS_CONFIG_V3_MAX_INTRO_POINTS 20

/* API */

int hs_config_service_all(const or_options_t *options, int validate_only);
int hs_config_client_auth_all(const or_options_t *options, int validate_only);

#endif /* !defined(TOR_HS_CONFIG_H) */

