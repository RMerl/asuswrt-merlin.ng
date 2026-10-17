/*
 * DHD IFE header file
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_ife.h 847137 2024-10-30 02:11:51Z $
 */

#ifndef __DHD_IFE_H__
#define __DHD_IFE_H__
#include <dhd.h>
#include <dhd_flowring.h>

uint32 dhd_ife_get_timer_interval(dhd_pub_t *dhdp);
void dhd_ife_set_timer_interval(dhd_pub_t *dhdp, uint32 intvl);
uint32 dhd_ife_get_budget(dhd_pub_t *dhdp);
void dhd_ife_set_budget(dhd_pub_t *dhdp, uint32 budget);

int dhd_ife_init(dhd_pub_t *dhdp);
int dhd_ife_deinit(dhd_pub_t *dhdp);
void dhd_ife_evict_timer_start(dhd_pub_t *dhdp);

#ifdef DHD_IFE_RECLAIM
void dhd_ife_cfg_evict_timer(dhd_pub_t *dhdp, uint32 timeout, uint32 budget);
uint32 dhd_get_upper_thr(dhd_pub_t *dhdp);
uint32 dhd_get_lower_thr(dhd_pub_t *dhdp);
void dhd_set_upper_thr(dhd_pub_t *dhdp, uint32 thr);
void dhd_set_lower_thr(dhd_pub_t *dhdp, uint32 thr);
#define	dhd_ife_cons_fr(cons, total)	(((cons) * 100)/(total))
#define	DHD_IFE_DFLT_CONS_HIGH_MARK	75	// % of consumed flowring to expedite eviction
#define	DHD_IFE_DFLT_CONS_LOW_MARK	25	// % of consumed flowring for regular operation
#define DHD_IFE_ENH_IDLE_CNT		0
#define DHD_IFE_ENH_EVICTION		1
#define DHD_IFE_ENH_FLRING_BUDGET(cons)	((cons) >> 2)	// 25% of consumed flowrings
#define DHD_IFE_ENH_TIMEOUT		5000	// 5 sec
#define DHD_IFE_ENH_LOOP_TIMEOUT_US	30000	// 30 ms
#endif /* DHD_IFE_RECLAIM */
#define DHD_IFE_DEFAULT_TIMEOUT_CNT	3

#ifdef CMWIFI_RDKB
#define DHD_IFE_DEFAULT_TIMEOUT         200000  // 10 min
#else
#define DHD_IFE_DEFAULT_TIMEOUT		25000	// 25 sec
#endif
#define DHD_IFE_DEFAULT_FLRING_BUDGET	10	// Max no of flowrings to iterate
						// on each timer invocation

void dhd_ife_flowring_create(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node);
void dhd_ife_flowring_delete(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node);

#endif /* __DHD_IFE_H__ */
