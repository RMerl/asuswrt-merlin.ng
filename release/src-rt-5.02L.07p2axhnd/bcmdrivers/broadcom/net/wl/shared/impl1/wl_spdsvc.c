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

static bcmFun_t *wl_spdsvc_transmit = NULL;
static bcmFun_t *wl_spdsvc_receive = NULL;

void wl_spdsvc_init(void)
{
	wl_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
	wl_spdsvc_receive = bcmFun_get(BCM_FUN_ID_SPDSVC_RECEIVE);

	return;
}

int wl_spdsvc_tx(struct sk_buff *skb, struct net_device *dev)
{
	int ret = -1;

	if (wl_spdsvc_transmit != NULL)
	{
		spdsvcHook_transmit_t spdsvc_transmit;

		spdsvc_transmit.pNBuff = SKBUFF_2_PNBUFF(skb);
		spdsvc_transmit.dev = dev;
		spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
		spdsvc_transmit.phy_overhead = WL_SPDSVC_OVERHEAD;

		if (wl_spdsvc_transmit(&spdsvc_transmit))
			ret = 0;
	}

	return ret;
}


int wl_spdsvc_rx(struct sk_buff *skb)
{
	int ret = 1; /* init as positive value, as BCME_OK is 0 and BCME_XXX are all negative */

	if (wl_spdsvc_receive != NULL)
	{
		spdsvcHook_receive_t spdsvc_receive;

		spdsvc_receive.pNBuff = SKBUFF_2_PNBUFF(skb);
		spdsvc_receive.header_type = SPDSVC_HEADER_TYPE_ETH;
		spdsvc_receive.phy_overhead = WL_SPDSVC_OVERHEAD;

		if (wl_spdsvc_receive(&spdsvc_receive))
			ret = BCME_OK;
	}
	return ret;
}
