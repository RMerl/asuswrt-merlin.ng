
/*
 * Linux-specific portion of
 * Broadcom 802.11 Networking Device Driver
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
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
 * $Id: wl_wfd.h $
 */

#ifndef _wl_wfd_h_
#define _wl_wfd_h_

#include <linux/netdevice.h>
#include <linux/skbuff.h>

#include <wfd_dev.h>

#define WL_NUM_OF_SSID_PER_UNIT 8

extern int wl_start_int(wl_info_t *wl, wl_if_t *wlif, struct sk_buff *skb);
extern void wl_bulk_txchainhandler(unsigned int rx_pktcnt, void **rx_pkts,
	int wl_radio_idx, uint32_t dummy);

extern int wl_wfd_bind(struct net_device *net, unsigned int unit);
extern void wl_wfd_unbind(int wfd_idx);

extern int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev);
extern int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev);

#endif /* _wl_wfd_h_ */
