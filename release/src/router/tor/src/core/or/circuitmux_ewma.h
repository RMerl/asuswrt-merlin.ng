/* * Copyright (c) 2012-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file circuitmux_ewma.h
 * \brief Header file for circuitmux_ewma.c
 **/

#ifndef TOR_CIRCUITMUX_EWMA_H
#define TOR_CIRCUITMUX_EWMA_H

#include "core/or/or.h"
#include "core/or/circuitmux.h"

/* The public EWMA policy callbacks object. */
extern circuitmux_policy_t ewma_policy;

/* Externally visible EWMA functions */
void cmux_ewma_set_options(const or_options_t *options,
                           const networkstatus_t *consensus);

void circuitmux_ewma_free_all(void);

#ifdef CIRCUITMUX_EWMA_PRIVATE
STATIC unsigned cell_ewma_get_current_tick_and_fraction(double *remainder_out);
STATIC void cell_ewma_initialize_ticks(void);
#endif

#endif /* !defined(TOR_CIRCUITMUX_EWMA_H) */

