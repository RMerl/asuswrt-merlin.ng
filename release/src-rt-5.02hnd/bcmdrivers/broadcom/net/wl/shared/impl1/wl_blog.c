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
 * $Id: wl_blog.c $
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
		return s;
	}

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
	/* Fix the priority if WME is enabled */
	if (WME_ENAB(wl->pub) && (PKTPRIO(skb) == 0))
		pktsetprio(skb, FALSE);

	if (wl->pub->fcache && (skb->blog_p != NULL)) {
		uint8_t prio4bit = 0;

#if defined(DSLCPE_PLATFORM_WITH_RUNNER) && defined(BCM_WFD)
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
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
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

	if (!(e->addr) || (e->event.status != WLC_E_STATUS_SUCCESS) || (dev == NULL))
		return;

	switch (e->event.event_type) {
		case WLC_E_DEAUTH:
		case WLC_E_DEAUTH_IND:
		case WLC_E_DISASSOC:
		case WLC_E_DISASSOC_IND:
			WL_ERROR(("wl%d: notify system/blog disconnection event.\n",
				wl->pub->unit));
			/* also destroy all fcache flows */
			blog_lock();
			blog_notify(DESTROY_NETDEVICE, dev, 0, 0);
			blog_unlock();
			break;

		default:
			return;
	}
}

#endif /* BCM_BLOG */
