/* Copyright (c) 2016-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_stats.h
 * \brief Header file for hs_stats.c
 **/

#ifndef TOR_HS_STATS_H
#define TOR_HS_STATS_H

void hs_stats_note_introduce2_cell(int is_hsv3);
uint32_t hs_stats_get_n_introduce2_v3_cells(void);
uint32_t hs_stats_get_n_introduce2_v2_cells(void);
void hs_stats_note_service_rendezvous_launch(void);
uint32_t hs_stats_get_n_rendezvous_launches(void);

#endif /* !defined(TOR_HS_STATS_H) */
