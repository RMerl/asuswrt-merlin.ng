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

#include <linux/version.h>
#include <linux/bcm_skb_defines.h>

#include <wlc_cfg.h>
#include <wlc_channel.h>

#include <wlioctl.h>
#include <wl_dbg.h>

#include <wlc_pub.h>
#include <wl_linux.h>
#include <wl_pktc.h>

#include <wl_blog.h>
#include "wl_br_d3lut.h"

#if defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING) || defined(BCM_PKTFWD)
#include <wlc.h>
#include <wlc_ap.h>
#include <wlc_scb.h>
#include <wlc_wmf.h>
#endif

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
		if (wl->pub->pktc_tbl && WLPKTCTBL(wl->pub->pktc_tbl)->g_stats)
			WLCNTINCR(WLPKTCTBL(wl->pub->pktc_tbl)->g_stats->tx_slowpath_skb);
#endif
		return s;
	}
#if defined(BCM_PKTFWD)
	wl_pktfwd_stats_gp->txf_fkb_pkts++;
#elif defined(PKTC_TBL)
	if (wl->pub->pktc_tbl && WLPKTCTBL(wl->pub->pktc_tbl)->g_stats)
		WLCNTINCR(WLPKTCTBL(wl->pub->pktc_tbl)->g_stats->tx_slowpath_fkb);
#endif
	orig_s = s;
	xlated_s = nbuff_xlate((pNBuff_t)s);
	if (xlated_s == NULL) {
		nbuff_free((pNBuff_t) orig_s);
		return NULL;
	}
	return xlated_s;
}

int wl_handle_blog_emit(struct wl_info *wl, struct wl_if *wlif, struct sk_buff *skb,
	struct net_device *dev)
{
#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
	if (!PKTISBLOG_TOHANDLE(skb)) return 0;
#endif

	if (skb->dev == NULL)
		skb->dev = dev ? dev : wlif->dev;
	
	/* Fix the priority if WME is enabled */
	if (WME_ENAB(wl->pub) && (PKTPRIO(skb) == 0))
		pktsetprio(skb, FALSE);

	if (wl->pub->fcache && (skb->blog_p != NULL)) {
		uint8_t prio4bit = 0;
		uint32_t hw_port;

#if defined(BCM_WFD) || (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
		struct ether_header *eh = (struct ether_header*) PKTDATA(wl->osh, skb);
#endif

		ENCODE_WLAN_PRIORITY_MARK(prio4bit, skb->mark);
		skb->blog_p->wfd.nic_ucast.priority = (prio4bit & 0x0f);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
		if (!ETHER_ISMULTI(eh->ether_dhost)) {
			/* Handle WAN as well as OVS case (including EAP case) */
			wl_update_d3lut_and_blog(skb->dev, skb);
		}
#endif

#if defined(BCM_WFD)
		if (ETHER_ISMULTI(eh->ether_dhost)) {
			skb->blog_p->wfd.mcast.is_tx_hw_acc_en = 1;
			skb->blog_p->wfd.mcast.is_wfd = 1;
			skb->blog_p->wfd.mcast.is_chain = 0;
			skb->blog_p->wfd.mcast.wfd_idx = wl->wfd_idx;
			skb->blog_p->wfd.mcast.wfd_prio = 0 ; /* put mcast in high prio queue */
			skb->blog_p->wfd.mcast.ssid = ((struct wl_if *)WL_DEV_IF(skb->dev))->subunit;
		} else {
			skb->blog_p->wfd.nic_ucast.is_wfd = 1;
			skb->blog_p->dev_xmit_blog = NULL;
#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
			if (wlif->wlcif->type != WLC_IFTYPE_WDS && PKTISWMF_HANDLED(skb))
			{
				struct scb * scb = wlc_scbfind_from_wlcif(wl->wlc, wlif->wlcif, eh->ether_dhost);
				wlc_bsscfg_t *bsscfg = wl_bsscfg_find(wlif);
				skb->blog_p->wfd.mcast.is_wmf_enabled = WMF_ENAB(bsscfg);

				if (scb) {
					struct scb *psta_prim = NULL;
					if ( !wl_wmf_psta_disable(bsscfg) &&
							(psta_prim = wlc_ap_get_psta_prim(wl->wlc->ap, scb))) {
						scb=psta_prim;
					}
					skb->blog_p->wlsta_id=(scb) ? scb->aid : 0;
				}
			}
#endif
		}
#endif

		hw_port = netdev_path_get_hw_port((struct net_device *)(skb->dev));
#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
		if(blog_emit(skb, skb->dev, TYPE_ETH, hw_port, BLOG_WLANPHY)==PKT_DROP) {
			return BCME_ERROR;
		}
#else
		blog_emit(skb, skb->dev, TYPE_ETH, hw_port, BLOG_WLANPHY);
#endif
	}

	return BCME_OK;
}

int wl_handle_blog_sinit(struct wl_info *wl, struct sk_buff *skb)
{
	/* If Linux TCP/IP stack is bypassed by injecting this packet directly to fastlath,
	 * then avoid software FC - it will probably be slower than fastpath.
	 */
	if (wl->pub->fcache && !skb->blog_p) {
		BlogAction_t blog_ret;
		uint32_t hw_port;

		/* Clear skb->mark (WLAN internal for priority) field as it will be used as skb->fc_ctxt in blog for ingress */
		skb->mark = 0;

		hw_port = netdev_path_get_hw_port((struct net_device *)(skb->dev));
		PKTSETFCDONE(skb);
		blog_ret = blog_sinit(skb, skb->dev, TYPE_ETH, hw_port, BLOG_WLANPHY);
		if (PKT_DONE == blog_ret) {
			/* Doesnot need go to IP stack */
			return 0;
		} else if (PKT_BLOG == blog_ret) {
#if defined(CONFIG_BCM_PON)
			/* PON Platforms support WLAN_RX_ACCELERATION through loopback model */
			skb->blog_p->rnr.is_rx_hw_acc_en = 1;
#elif defined(BCM_AWL)
			/* Archer platforms support acceleration through Archer driver */
			skb->blog_p->wl_hw_support.is_rx_hw_acc_en = 1;
#else
			/* For NIC mode -- RX is always on host, so HW can't accelerate */
			skb->blog_p->rnr.is_rx_hw_acc_en = 0;
#endif
		} 
		else if (PKT_DROP == blog_ret) {
			PKTFREE(wl->osh, skb, TRUE);
			return 0;
		}
		PKTCLRFCDONE(skb);
	}

	return -1;
}

void wl_handle_blog_event(wl_info_t *wl, wlc_event_t *e)
{
	struct net_device *dev;
	BlogFlushParams_t params = {};

	if (!(e->addr) || (e->event.status != WLC_E_STATUS_SUCCESS))
		return;

    dev = dev_get_by_name(&init_net, e->event.ifname);
    if (dev == NULL) {
        WL_ERROR(("wl%d: wl_handle_blog_event - Invalid interface\n",
                 wl->pub->unit));
        return;
    }

	switch (e->event.event_type) {
#if defined(BCM_PKTFWD)
		case WLC_E_LINK:
		{
			wl_if_t *wlif = WL_DEV_IF(dev);
			if (e->event.flags & WLC_EVENT_MSG_LINK) {
				/* wl link up */
				WL_INFORM(("%s: %s, link up\n", __FUNCTION__, dev->name));
				netdev_wlan_set_if_up(wlif->d3fwd_wlif); /* mark this interface is up */
			} else {
				/* wl link down */
				WL_INFORM(("%s: %s, link down\n", __FUNCTION__, dev->name));
				netdev_wlan_unset_if_up(wlif->d3fwd_wlif); /* unset up flag */
			}
			break;
		}
#endif

		case WLC_E_DEAUTH:
		case WLC_E_DEAUTH_IND:
		case WLC_E_DISASSOC:
		case WLC_E_DISASSOC_IND:
			WL_ASSOC(("wl%d: notify system/blog disconnection event.\n",
				wl->pub->unit));
			/* also destroy the fcache flow */
			params.flush_dstmac = 1;
			params.flush_srcmac = 1;
			memcpy(&params.mac[0], &e->event.addr.octet[0], sizeof(e->event.addr.octet));
			blog_notify_async_wait(FLUSH, dev, (unsigned long)&params, 0);

#if defined(PKTC_TBL)
			/* mark as STA disassoc */
			WL_ASSOC(("%s: mark as DIS-associated. addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__,
				e->event.addr.octet[0], e->event.addr.octet[1],
				e->event.addr.octet[2], e->event.addr.octet[3],
				e->event.addr.octet[4], e->event.addr.octet[5]));

            wl_pktc_req(PKTC_TBL_SET_STA_ASSOC, (unsigned long)e->event.addr.octet,
                        0, e->event.event_type);

            if (wl_pktc_del_hook != NULL)
			    wl_pktc_del_hook((unsigned long)e->event.addr.octet, dev);
#endif /* PKTC_TBL */
			break;

		case WLC_E_ASSOC:
		case WLC_E_ASSOC_IND:
		case WLC_E_REASSOC_IND:
			WL_ASSOC(("wl%d: notify system/blog association event.\n",
				wl->pub->unit));
                        /* BCM_PKTFWD: flowid and incarn will be updated for this STA
                         * Destroy stale entries in fcache
                         */
			params.flush_dstmac = 1;
			params.flush_srcmac = 1;
			memcpy(&params.mac[0], &e->event.addr.octet[0], sizeof(e->event.addr.octet));
			blog_notify_async_wait(FLUSH, dev, (unsigned long)&params, 0);
#if defined(PKTC_TBL)
			/* mark as STA assoc */
			WL_ASSOC(("%s: mark as associated. addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__,
				e->event.addr.octet[0], e->event.addr.octet[1],
				e->event.addr.octet[2], e->event.addr.octet[3],
				e->event.addr.octet[4], e->event.addr.octet[5]));
#if defined(BCM_PKTFWD)
			/* setting dwds client properly before d3lut_elem insertion */
			{
				wl_if_t *wlif = WL_DEV_IF(dev);
				struct scb *scb = wlc_scbfind_from_wlcif(wl->wlc, wlif->wlcif, e->event.addr.octet);
				wlc_bsscfg_t *bsscfg = wl_bsscfg_find(wlif);
				netdev_wlan_unset_dwds_client(wlif->d3fwd_wlif); /* reset first */
				if (BSSCFG_STA(bsscfg) && scb && SCB_DWDS(scb)) {
					netdev_wlan_set_dwds_client(wlif->d3fwd_wlif);
				}
			}
#endif /* BCM_PKTFWD */
			wl_pktc_req(PKTC_TBL_SET_STA_ASSOC, (unsigned long)e->event.addr.octet,
				1, e->event.event_type);
			wl_pktc_req(PKTC_TBL_UPDATE, (unsigned long)e->event.addr.octet,
				(unsigned long)dev, 0);

#endif /* PKTC_TBL */
			break;

		default:
			break;
	}
    /* Release reference to device */
    dev_put(dev);
}

#endif /* BCM_BLOG */
