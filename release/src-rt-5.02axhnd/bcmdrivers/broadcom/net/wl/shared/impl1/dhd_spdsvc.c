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

#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_runner.h>
#include <dhd_flowring.h>

#include "bcm_spdsvc.h"

static bcmFun_t *dhd_spdsvc_transmit = NULL;
static bcmFun_t *dhd_spdsvc_receive = NULL;

void dhd_spdsvc_init(void)
{
	dhd_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
	dhd_spdsvc_receive = bcmFun_get(BCM_FUN_ID_SPDSVC_RECEIVE);

	return;
}

int dhd_spdsvc_tx(dhd_pub_t *dhdp, void **pktbuf)
{
	int ret = 1; /* init as positive value, as BCME_OK is 0 and BCME_XXX are all negative */

	if (dhd_spdsvc_transmit != NULL)
	{
#if defined(BCM_DHD_RUNNER)
		uint16 flowid = DHD_PKT_GET_FLOWID(*pktbuf);
		flow_ring_node_t *flow_ring_node = dhd_flow_ring_node(dhdp, flowid);

		if (!DHD_FLOWRING_RNR_OFFL(flow_ring_node))
#endif /* BCM_DHD_RUNNER */
		{
			if (IS_SKBUFF_PTR(*pktbuf))
			{
				struct sk_buff *skb;
				spdsvcHook_transmit_t spdsvc_transmit;
				if (DHDHDR_SUPPORT(dhdp) &&
				    !(*pktbuf = nbuff_unshare((pNBuff_t)(*pktbuf)))) {
					ret = BCME_ERROR;
					goto exit;
				}
				skb = PNBUFF_2_SKBUFF(*pktbuf);
				spdsvc_transmit.pNBuff = *pktbuf;
				spdsvc_transmit.dev = skb->dev;
				spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
				spdsvc_transmit.phy_overhead = WL_SPDSVC_OVERHEAD;

				if (dhd_spdsvc_transmit(&spdsvc_transmit))
				{
					ret = BCME_OK;
					goto exit;
				}
			}
		}
	}
exit:
	return ret;
}

int dhd_spdsvc_rx(struct sk_buff *skb)
{
	int ret = 1; /* init as positive value, as BCME_OK is 0 and BCME_XXX are all negative */

	if (dhd_spdsvc_receive != NULL)
	{
		spdsvcHook_receive_t spdsvc_receive;

		spdsvc_receive.pNBuff = SKBUFF_2_PNBUFF(skb);
		spdsvc_receive.header_type = SPDSVC_HEADER_TYPE_ETH;
		spdsvc_receive.phy_overhead = WL_SPDSVC_OVERHEAD;

		if (dhd_spdsvc_receive(&spdsvc_receive))
		{
			ret = BCME_OK;
			goto exit;
		}
	}
exit:
	return ret;
}
