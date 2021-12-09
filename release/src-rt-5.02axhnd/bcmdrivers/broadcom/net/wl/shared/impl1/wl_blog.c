/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#if defined(BCM_BLOG)

#include <linux/bcm_skb_defines.h>

#include <wlc_cfg.h>
#include <wlc_channel.h>

#include <wlioctl.h>
#include <wl_dbg.h>

#include <wlc_pub.h>
#include <wl_linux.h>
#include <wl_pktc.h>

#include <wl_blog.h>

struct sk_buff *wl_xlate_to_skb(struct wl_info *wl, struct sk_buff *s)
{
	struct sk_buff *orig_s, *xlated_s;

	if (IS_SKBUFF_PTR(s)) {
		/* reset skb->cb field as initializing WLPKTTAG for all incoming skbs */
		memset(&s->cb[0], 0, sizeof(s->cb));
#ifdef DSLCPE_CACHE_SMARTFLUSH
		PKTSETDIRTYP(wl->osh, s, NULL);
#endif
#if defined(BCM_PKTFWD)
		wl_pktfwd_stats_gp->txf_fkb_pkts++;
#elif defined(PKTC_TBL)
		if (wl->pub->pktc_tbl && wl->pub->pktc_tbl->g_stats)
			WLCNTINCR(wl->pub->pktc_tbl->g_stats->tx_slowpath_skb);
#endif
		return s;
	}
#if defined(BCM_PKTFWD)
	wl_pktfwd_stats_gp->txf_fkb_pkts++;
#elif defined(PKTC_TBL)
	if (wl->pub->pktc_tbl && wl->pub->pktc_tbl->g_stats)
		WLCNTINCR(wl->pub->pktc_tbl->g_stats->tx_slowpath_fkb);
#endif
	orig_s = s;
	xlated_s = nbuff_xlate((pNBuff_t)s);
	if (xlated_s == NULL) {
		nbuff_free((pNBuff_t) orig_s);
		return NULL;
	}
	return xlated_s;
}

#if defined(BCM_WFD) && defined(CONFIG_BCM_FC_BASED_WFD)
typedef int (*FC_WFD_ENQUEUE_HOOK)(void * nbuff_p,const Blog_t * const blog_p); /* Xmit with blog */
extern FC_WFD_ENQUEUE_HOOK fc_wfd_enqueue_cb;
static int wl_fc_wfd_enqueue(void * nbuff_p,const Blog_t * const blog_p)
{
    if(fc_wfd_enqueue_cb)
        return (fc_wfd_enqueue_cb)(nbuff_p,blog_p);
         
    return 0;
}
#endif

int wl_handle_blog_emit(struct wl_info *wl, struct wl_if *wlif, struct sk_buff *skb,
	struct net_device *dev)
{
	/* Fix the priority if WME is enabled */
	if (WME_ENAB(wl->pub) && (PKTPRIO(skb) == 0))
		pktsetprio(skb, FALSE);

	if (wl->pub->fcache && (skb->blog_p != NULL)) {
		uint8_t prio4bit = 0;

#if defined(BCM_WFD)
		struct ether_header *eh = (struct ether_header*) PKTDATA(wl->osh, skb);
#endif

		ENCODE_WLAN_PRIORITY_MARK(prio4bit, skb->mark);
		skb->blog_p->wfd.nic_ucast.priority = (prio4bit & 0x0f);

#if defined(DSLCPE_PLATFORM_WITH_RUNNER) && defined(BCM_WFD)
		if (ETHER_ISMULTI(eh->ether_dhost)) {
			skb->blog_p->wfd.mcast.is_tx_hw_acc_en = 1;
			skb->blog_p->wfd.mcast.is_wfd = 1;
			skb->blog_p->wfd.mcast.is_chain = 0;
			skb->blog_p->wfd.mcast.wfd_idx = wl->wfd_idx;
			skb->blog_p->wfd.mcast.wfd_prio = 0 ; /* put mcast in high prio queue */
			skb->blog_p->wfd.mcast.ssid = wlif->subunit;
		}
#endif

#if defined(BCM_WFD)
		if (!ETHER_ISMULTI(eh->ether_dhost))
		{
			skb->blog_p->wfd.nic_ucast.is_wfd = 1;
#if defined(BCM_WFD) && defined(CONFIG_BCM_FC_BASED_WFD)
			if(skb->blog_p->wfd.nic_ucast.is_chain)
			{
				skb->blog_p->dev_xmit_blog = wl_fc_wfd_enqueue;
			}
#else
			skb->blog_p->dev_xmit_blog = NULL;
#endif
		}
#endif

		blog_emit(skb, dev, TYPE_ETH, 0, BLOG_WLANPHY);
	}
	skb->dev = dev;

	return 0;
}

int wl_handle_blog_sinit(struct wl_info *wl, struct sk_buff *skb)
{
	/* If Linux TCP/IP stack is bypassed by injecting this packet directly to fastlath,
	 * then avoid software FC - it will probably be slower than fastpath.
	 */
	if (wl->pub->fcache) {
		BlogAction_t blog_ret;

		blog_ret = blog_sinit(skb, skb->dev, TYPE_ETH, 0, BLOG_WLANPHY);
		if (PKT_DONE == blog_ret) {
			/* Doesnot need go to IP stack */
			return 0;
		} else if (PKT_BLOG == blog_ret) {
#ifdef CONFIG_BCM_PON
			/* PON Platforms support WLAN_RX_ACCELERATION through loopback model */
			skb->blog_p->rnr.is_rx_hw_acc_en = 1;
#else
			/* For NIC mode -- RX is always on host, so HW can't accelerate */
			skb->blog_p->rnr.is_rx_hw_acc_en = 0;
#endif
		}
	}

	return -1;
}

void wl_handle_blog_event(wl_info_t *wl, wlc_event_t *e)
{
	struct net_device *dev = wl->dev;
	BlogFlushParams_t params = {};

	if (!(e->addr) || (e->event.status != WLC_E_STATUS_SUCCESS) || (dev == NULL))
		return;

	switch (e->event.event_type) {
		case WLC_E_DEAUTH:
		case WLC_E_DEAUTH_IND:
		case WLC_E_DISASSOC:
		case WLC_E_DISASSOC_IND:
			WL_ERROR(("wl%d: notify system/blog disconnection event.\n",
				wl->pub->unit));
			/* also destroy the fcache flow */
			params.flush_dstmac = 1;
			params.flush_srcmac = 1;
			memcpy(&params.mac[0], &e->event.addr.octet[0], sizeof(e->event.addr.octet));
			blog_lock();
			blog_notify(FLUSH, dev, (unsigned long)&params, 0);
			blog_unlock();

#if defined(PKTC_TBL)
			/* mark as STA disassoc */
			WL_ERROR(("%s: mark as DIS-associated. addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__,
				e->event.addr.octet[0], e->event.addr.octet[1],
				e->event.addr.octet[2], e->event.addr.octet[3],
				e->event.addr.octet[4], e->event.addr.octet[5]));
			wl_pktc_req(PKTC_TBL_SET_STA_ASSOC, (unsigned long)e->event.addr.octet, 0, e->event.event_type);
			wl_pktc_del((unsigned long)e->event.addr.octet);
#endif /* PKTC_TBL */
			break;

		case WLC_E_ASSOC:
		case WLC_E_ASSOC_IND:
		case WLC_E_REASSOC_IND:
#if defined(PKTC_TBL)
			/* mark as STA assoc */
			WL_ERROR(("%s: mark as associated. addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__,
				e->event.addr.octet[0], e->event.addr.octet[1],
				e->event.addr.octet[2], e->event.addr.octet[3],
				e->event.addr.octet[4], e->event.addr.octet[5]));
			wl_pktc_req(PKTC_TBL_UPDATE, (unsigned long)e->event.addr.octet, (unsigned long)dev, 0);
			wl_pktc_req(PKTC_TBL_SET_STA_ASSOC, (unsigned long)e->event.addr.octet, 1, e->event.event_type);
#endif /* PKTC_TBL */
			break;

		default:
			return;
	}
}

#endif /* BCM_BLOG */
