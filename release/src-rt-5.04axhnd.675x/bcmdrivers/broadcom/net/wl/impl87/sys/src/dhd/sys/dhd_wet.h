/*
 * Wireless Ethernet (WET) interface
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
 * $Id: dhd_wet.h 764683 2018-05-29 10:30:59Z $
 */

#ifndef _dhd_wet_h_
#define _dhd_wet_h_

#include <ethernet.h>
#include <dngl_stats.h>
#include <dhd.h>

/* forward declaration */
typedef struct dhd_wet_info dhd_wet_info_t;

extern dhd_wet_info_t *dhd_get_wet_info(dhd_pub_t *pub);
extern void dhd_free_wet_info(dhd_pub_t *pub, void *wet);

/* Process frames in transmit direction */
extern int dhd_wet_send_proc(void *weth, int ifidx, void *sdu, void **new);
extern int dhd_set_wet_host_ipv4(dhd_pub_t *pub, void *parms, uint32 len);
extern int dhd_set_wet_host_mac(dhd_pub_t *pub, void *parms, uint32 len);
/* Process frames in receive direction */
extern int dhd_wet_recv_proc(void *weth, int ifidx, void *sdu);
extern void dhd_wet_sta_delete_list(dhd_pub_t *dhd_pub);

#ifdef PLC_WET
extern void dhd_wet_bssid_upd(dhd_wet_info_t *weth, dhd_bsscfg_t *cfg);
#endif /* PLC_WET */

int dhd_set_wet_mode(dhd_pub_t *dhdp, uint32 val);
int dhd_get_wet_mode(dhd_pub_t *dhdp);

extern void dhd_wet_dump(dhd_pub_t *dhdp, struct bcmstrbuf *b);
#ifdef DHD_DPSTA
extern void dhd_dpsta_wet_register(dhd_pub_t *dhd);
#endif /* DHD_DPSTA */
#endif	/* _dhd_wet_h_ */
