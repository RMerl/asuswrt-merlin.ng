/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_stats.c
 * \brief Keeps stats about the activity of our onion service(s).
 **/

#include "core/or/or.h"
#include "feature/hs/hs_stats.h"
#include "feature/hs/hs_service.h"

/** Number of v3 INTRODUCE2 cells received */
static uint32_t n_introduce2_v3 = 0;
/** Number of v2 INTRODUCE2 cells received */
static uint32_t n_introduce2_v2 = 0;
/** Number of attempts to make a circuit to a rendezvous point */
static uint32_t n_rendezvous_launches = 0;

/** Note that we received another INTRODUCE2 cell. */
void
hs_stats_note_introduce2_cell(int is_hsv3)
{
  if (is_hsv3) {
    n_introduce2_v3++;
  } else {
    n_introduce2_v2++;
  }
}

/** Return the number of v3 INTRODUCE2 cells we have received. */
uint32_t
hs_stats_get_n_introduce2_v3_cells(void)
{
  return n_introduce2_v3;
}

/** Return the number of v2 INTRODUCE2 cells we have received. */
uint32_t
hs_stats_get_n_introduce2_v2_cells(void)
{
  return n_introduce2_v2;
}

/** Note that we attempted to launch another circuit to a rendezvous point. */
void
hs_stats_note_service_rendezvous_launch(void)
{
  n_rendezvous_launches++;
}

/** Return the number of rendezvous circuits we have attempted to launch. */
uint32_t
hs_stats_get_n_rendezvous_launches(void)
{
  return n_rendezvous_launches;
}

