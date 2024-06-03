/*
 * Copyright (C) 2018, Broadcom. All Rights Reserved.
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
 */

#include <typedefs.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#if defined(BCM_NBUFF)
#include <linux/nbuff.h>
#endif

#include <bcmutils.h>
#include "bcm_spdsvc.h"
#include "wl_br_d3lut.h"

static bcmFun_t *wl_spdsvc_transmit = NULL;
static bcmFun_t *wl_spdsvc_receive = NULL;

void wl_spdsvc_init(void)
{
	wl_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
	wl_spdsvc_receive = bcmFun_get(BCM_FUN_ID_SPDSVC_RECEIVE);

	return;
}

static int wl_spdsvc_tx_helper(pNBuff_t pNBuff, uint32_t * tag_p)
{
	if (likely(IS_SKBUFF_PTR(pNBuff))) {
		struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

		if (skb->blog_p == BLOG_NULL) {
			uint8_t prio4bit = 0;

			skb->blog_p = blog_get();
			if (skb->blog_p == BLOG_NULL) {
				return -1;
			}

			wl_update_d3lut_and_blog(skb->dev, skb, ETHHDR_DATA);

			ENCODE_WLAN_PRIORITY_MARK(prio4bit, skb->mark);
			skb->blog_p->wfd.nic_ucast.priority = (prio4bit & 0x0f);
		}

		skb->wl.u32 = 0;
		skb->wl.ucast.nic.is_ucast = 1;
		skb->wl.ucast.nic.wl_prio = skb->blog_p->wfd.nic_ucast.priority;
		skb->wl.ucast.nic.wl_chainidx = skb->blog_p->wfd.nic_ucast.chain_idx;

		*tag_p = skb->blog_p->wfd.nic_ucast.wfd_idx;

		return 0;
	}

	return -1;
}

int wl_spdsvc_tx(struct sk_buff *skb, struct net_device *dev)
{
	int ret = -1;

	if (wl_spdsvc_transmit != NULL) {
		spdsvcHook_transmit_t spdsvc_transmit = { };

		spdsvc_transmit.pNBuff = SKBUFF_2_PNBUFF(skb);
		spdsvc_transmit.dev = dev;
		spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
		spdsvc_transmit.phy_overhead = WL_SPDSVC_OVERHEAD;
		spdsvc_transmit.transmit_helper = wl_spdsvc_tx_helper;
		spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_WLAN_NIC;
		spdsvc_transmit.flags = SPDSVC_HOOK_TRANSMIT_SPDT_NO_AUTO_TRIGGER;

		if (wl_spdsvc_transmit(&spdsvc_transmit))
			ret = 0;
	}

	return ret;
}

int wl_spdsvc_rx(struct sk_buff *skb)
{
	int ret = 1;	/* init as positive value, as BCME_OK is 0 and BCME_XXX are all negative */

	if (wl_spdsvc_receive != NULL) {
		spdsvcHook_receive_t spdsvc_receive;

		spdsvc_receive.pNBuff = SKBUFF_2_PNBUFF(skb);
		spdsvc_receive.header_type = SPDSVC_HEADER_TYPE_ETH;
		spdsvc_receive.phy_overhead = WL_SPDSVC_OVERHEAD;

		if (wl_spdsvc_receive(&spdsvc_receive))
			ret = BCME_OK;
	}
	return ret;
}
