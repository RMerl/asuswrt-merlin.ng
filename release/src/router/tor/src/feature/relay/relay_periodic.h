/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_periodic.h
 * @brief Header for feature/relay/relay_periodic.c
 **/

#ifndef TOR_FEATURE_RELAY_RELAY_PERIODIC_H
#define TOR_FEATURE_RELAY_RELAY_PERIODIC_H

void relay_register_periodic_events(void);
void reschedule_descriptor_update_check(void);

#endif /* !defined(TOR_FEATURE_RELAY_RELAY_PERIODIC_H) */
