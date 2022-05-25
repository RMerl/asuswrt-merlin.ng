/*
 * DHD IFE header file
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: dhd_ife.c sg944736 $
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

void dhd_ife_flowring_create(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node);
void dhd_ife_flowring_delete(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node);
#endif /* __DHD_IFE_H__ */
