/*
 * DHD PSTA Processing for ARP, IPv4, IPv6,DHCP, ICMPv6, DHCP6
 * PSTA acting as Proxy STA Repeater (PSR)
 * PSTA forwarding packets for wired/wireless sta
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
 * $Id: dhd_psta.h 469476 2014-04-15 12:39:01Z $
 */
#ifndef _dhd_psta_h_
#define _dhd_psta_h_

#include <dhd.h>

#define DHD_MODE_PSTA_DISABLED	0x00
#define DHD_MODE_PSTA	0x01
#define DHD_MODE_PSR	0x02

#define	PSR_ENABLED(dhdp)	((dhdp)->info->psta_mode == DHD_MODE_PSR)

/* This macro is used in dongle to get vif MAC. Multi repeater is not supported
 * in DHDAP at this point
 */
#define ETHER_SET_LOCALADDR(ea)	(((uint8 *)(ea))[0] = (((uint8 *)(ea))[0] | 2))

int32 dhd_psta_proc(dhd_pub_t *dhdp, uint32 ifidx, void **pkt, bool tx);
int dhd_set_psta_mode(dhd_pub_t *dhdp, uint32 val);
int dhd_get_psta_mode(dhd_pub_t *dhdp);
#endif /* _dhd_psta_h_ */
