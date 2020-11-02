/**
 * @file
 * @brief
 * Dongle D0 packet filter.
 *
 * This feature implements a IP packet filter engine based on MS D0 filter
 * doc.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wl_d0_filter.c 708017 2017-06-29 14:11:45Z $
 */

/* ---- Include Files ---------------------------------------------------- */

#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <proto/ethernet.h>
#include <proto/802.3.h>
#include <proto/bcmip.h>
#include <proto/bcmipv6.h>
#include <proto/bcmarp.h>
#include <proto/bcmudp.h>
#include <bcmendian.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_pio.h>
#include <wlc.h>

#include <wl_export.h>
#include <wl_d0_filter.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#ifdef WLLMAC_ONLY
	#error "LMAC not supported due to variable length 802.11 headers."
#endif // endif

/* This is useful during development to avoid compiler warnings/errors about
 * un-used static functions.
 */
/* #define STATIC */
#ifndef STATIC
#define STATIC	static
#endif // endif

/* wlc_pub_t struct access macros */
#define WLCUNIT(info)	((info)->wlc->pub->unit)
#define WLCOSH(info)	((info)->wlc->osh)

#define 	NUM_PKTS_MAX		16 	/* max num of pkts buffered */
#define 	NUM_PKTS_WATERMARK	8	/* water mark of buffered q */

/* IOVar table */
enum {
	/* Enable/disable packet filter. */
	IOV_D0_FILTER_ENABLE,

	/* Set global packet filter engine match action, e.g.
	 *  - coalescing on match.
	 */
	IOV_D0_FILTER_MODE,

	/* Retrieve debug filter stats. */
	IOV_D0_FILTER_STATS,

	/* Clear debug filter stats. */
	IOV_D0_FILTER_CLEAR_STATS,

	IOV_D0_FILTER_ARP,

	IOV_D0_FILTER_IPV4,

	IOV_D0_FILTER_ARP_CLEAR,

	IOV_D0_FILTER_IPV4_CLEAR,

	IOV_D0_FILTER_MAX_DELAY
};

/* special values */
/* 802.3 llc/snap header */
static const uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
#define IPV4_FILTER		(IPV4_NETBT_FILTER | IPV4_LLMNR_FILTER | \
				IPV4_SSDP_FILTER | IPV4_WSD_FILTER)
#define IPV6_FILTER		(IPV6_NETBT_FILTER | IPV6_LLMNR_FILTER | \
				IPV6_SSDP_FILTER | IPV6_WSD_FILTER)

/* Packet filter private info structure. Container for the set of
 * installed filters.
 */
struct d0_filter_info {

	/* Pointer back to wlc structure */
	wlc_info_t		*wlc;

	uint32 			max_delay;
	uint32			filters;

	/* Statistics counters for debug. */
	unsigned int 		num_pkts_forwarded;
	unsigned int 		num_pkts_discarded;
	unsigned int		num_pkts_matched;

	struct wl_timer *timer; /* timer */

	struct ipv4_addr	host_ip;
	bool   			timer_on;

	bool 			enable;

	struct pktq 		q;		/* buffered pkt queue pending up */
};

/* ---- Private Variables ------------------------------------------------ */

static const bcm_iovar_t pkt_filter_iovars[] = {
	{
		"d0_filter_enable",
		IOV_D0_FILTER_ENABLE,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"d0_filter_stats",
		IOV_D0_FILTER_STATS,
		(0),
		IOVT_BUFFER,
		sizeof(wl_pkt_filter_stats_t)
	},

	{
		"d0_filter_clear_stats",
		IOV_D0_FILTER_CLEAR_STATS,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"d0_filter_arp",
		IOV_D0_FILTER_ARP,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"d0_filter_ipv4",
		IOV_D0_FILTER_IPV4,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"d0_filter_arp_clear",
		IOV_D0_FILTER_ARP_CLEAR,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"d0_filter_ipv4_clear",
		IOV_D0_FILTER_IPV4_CLEAR,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"d0_filter_max_delay",
		IOV_D0_FILTER_MAX_DELAY,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{NULL, 0, 0, 0, 0 }
};

/* ---- Private Function Prototypes -------------------------------------- */

STATIC int
d0_filter_doiovar
(
	void 			*hdl,
	const bcm_iovar_t	*vi,
	uint32 			actionid,
	const char 		*name,
	void 			*p,
	uint			plen,
	void 			*a,
	int 			alen,
	int 			vsize,
	struct wlc_if 		*wlcif
);

static void wlc_d0_filter_send_up(wlc_d0_filter_info_t *info);

STATIC bool
run_ip_filter
(
	wlc_d0_filter_info_t	*info,
	const void 		*sdu
);

STATIC bool
run_arp_filter
(
	wlc_d0_filter_info_t	*info,
	struct  ether_addr	*daddr,
	uint8			*pt
);

STATIC bool
run_ipv4_filter
(
wlc_d0_filter_info_t    *info,
	struct  ether_addr	*daddr,
	uint8			*pt,
	uint			len
);

STATIC bool
run_ipv6_filter
(
	wlc_d0_filter_info_t	*info,
	struct  ether_addr	*daddr,
	uint8			*pt,
	uint			len
);

/* ---- Functions -------------------------------------------------------- */
static void
d0_timeout(void *arg)
{
	wlc_d0_filter_info_t *info = (wlc_d0_filter_info_t *)arg;
	/* send buffered pkts up first */
	wlc_d0_filter_send_up(info);
	info->timer_on = FALSE;
}

/* ----------------------------------------------------------------------- */
wlc_d0_filter_info_t *
BCMATTACHFN(wlc_d0_filter_attach)(wlc_info_t *wlc)
{
	wlc_d0_filter_info_t *info;

	/* Allocate packet filter private info struct. */
	info = MALLOCZ(wlc->osh, sizeof(wlc_d0_filter_info_t));
	if (info == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* Init packet filter private info struct. */
	info->wlc  = wlc;

	/* Register this module. */
	wlc_module_register(wlc->pub,
	                    pkt_filter_iovars,
	                    "d0_filter",
	                    info,
	                    d0_filter_doiovar,
	                    NULL, NULL,
	                    NULL);

	wlc->pub->_d0_filter = TRUE;
	return (info);

fail:
	if (NULL != info)
		MFREE(WLCOSH(info), info, sizeof(wlc_d0_filter_info_t));

	return (NULL);
}

/* ----------------------------------------------------------------------- */
void
BCMATTACHFN(wlc_d0_filter_detach)(wlc_d0_filter_info_t *info)
{

	if (info == NULL)
		return;

	if (info->timer) {
		if (info->timer_on) {
			wl_del_timer(info->wlc->wl, info->timer);
			info->timer_on = FALSE;
		}
		wl_free_timer(info->wlc->wl, info->timer);
		info->timer = NULL;
	}

	/* De-register this module. */
	wlc_module_unregister(info->wlc->pub, "d0_filter", info);

	/* Free allocated packet filter engine state. */
	if (NULL != info)
		MFREE(WLCOSH(info), info, sizeof(wlc_d0_filter_info_t));
}

static void wlc_d0_filter_send_up(wlc_d0_filter_info_t *info)
{
	void *p;
	osl_t *osh;
	struct dot11_header *h;
	uint16 fc;
	wlc_bsscfg_t *bsscfg;
	struct ether_addr *bssid;
	struct wl_if *wlif;

	osh = WLCOSH(info);

	while ((p = pktdeq(&info->q)) != NULL) {
		h = (struct dot11_header *)PKTDATA(osh, p);
		fc = ltoh16(h->fc);
		if (fc & FC_TODS)
			bssid = &h->a1;
		else if (fc & FC_FROMDS)
			bssid = &h->a2;
		else
			bssid = &h->a3;
		bsscfg = wlc_bsscfg_find_by_bssid(info->wlc, bssid);

		if (bsscfg)
			wlif = bsscfg->wlcif->wlif;
		else
			wlif = NULL;

		wl_sendup_no_filter(info->wlc->wl, wlif, p, 1);
	}

	return;
}

/* ----------------------------------------------------------------------- */
bool wlc_d0_filter_recv_proc(wlc_d0_filter_info_t *info, void *sdu)
{
	bool		match;

	if (!info->enable)
		return TRUE;

	if (pktq_len(&info->q) == NUM_PKTS_MAX) {
		goto send_pkt_up;
	}

	match = run_ip_filter(info, sdu);
	if (!match) {
		goto send_pkt_up;
	}

	/* Update debug stats, and then bail... */
	info->num_pkts_matched++;

		pktenq(&info->q, sdu);
	if (!info->timer_on) {
		info->timer_on = TRUE;
		wl_add_timer(info->wlc->wl, info->timer, info->max_delay, 0);
	}
	if (pktq_len(&info->q) < NUM_PKTS_WATERMARK) {
		return (FALSE);
	}
	else {
		/* Q reached watermark, send packets immediately */
		wl_del_timer(info->wlc->wl, info->timer);
		wl_add_timer(info->wlc->wl, info->timer, 0, 0);
	}
	/* Don't send the pkt up */
	return FALSE;

send_pkt_up:
	/* return TRUE so that the pkt is sent up */
	return TRUE;
}

/*
*****************************************************************************
* Function:   d0_filter_doiovar
*
* Purpose:    Handles packet filtering related IOVars.
*
* Parameters:
*
* Returns:    0 on success.
*****************************************************************************
*/
STATIC int
d0_filter_doiovar
(
	void 			*hdl,
	const bcm_iovar_t	*vi,
	uint32 			actionid,
	const char 		*name,
	void 			*p,
	uint 			plen,
	void 			*a,
	int 			alen,
	int 			vsize,
	struct wlc_if 		*wlcif
)
{
	wlc_d0_filter_info_t	*info;
	int err;

	info = hdl;

	if ((err = wlc_iovar_check(info->wlc, vi, a, alen, IOV_ISSET(actionid), wlcif)) != 0)
		return err;

	switch (actionid) {

		case IOV_SVAL(IOV_D0_FILTER_ENABLE):
		{
			/* Make a local copy of the received buffer arg. Can't simply cast
			 * arg to a 'wl_pkt_filter_enable_t' pointer because the arg may not be
			 * aligned correctly.
			 */
			bcopy(a, &info->enable, sizeof(info->enable));
			if (info->enable) {
				if (!info->timer) {
					if (!(info->timer = wl_init_timer(info->wlc->wl,
					d0_timeout, info, "d0_coal"))) {
						WL_ERROR(("wl%d: %s: timer init failed\n",
						info->wlc->pub->unit, __FUNCTION__));
					}
					pktqinit(&info->q, NUM_PKTS_MAX);
				}
			} else {
				if (info->timer) {
					if (info->timer_on) {
						wl_del_timer(info->wlc->wl, info->timer);
						info->timer_on = FALSE;
					}

					/* send buffered pkts up */
					wlc_d0_filter_send_up(info);

					wl_free_timer(info->wlc->wl, info->timer);
					info->timer = NULL;
				}
			}
		}
		break;

		case IOV_GVAL(IOV_D0_FILTER_STATS):
		{
			wl_pkt_filter_stats_t	stats;

			/* Return current stats. */
			stats.num_pkts_matched		= info->num_pkts_matched;
			stats.num_pkts_discarded	= info->num_pkts_discarded;
			stats.num_pkts_forwarded	= info->num_pkts_forwarded;
			bcopy(&stats, a, sizeof(stats));
		}
		break;

		case IOV_SVAL(IOV_D0_FILTER_CLEAR_STATS):
		{

			/* Clear debug stats. */
			info->num_pkts_matched		= 0;
			info->num_pkts_discarded	= 0;
			info->num_pkts_forwarded	= 0;
		}
		break;

		case IOV_SVAL(IOV_D0_FILTER_ARP):
		{
			/* Add one IP address to the host IP table. */
			if (plen < sizeof(struct ipv4_addr))
				return BCME_BUFTOOSHORT;

			/*
			* Requested ip-addr not found in the host_ip; add it.
			* Use link-local static address setting ONLY for testing
			* and warn.  Link-local is used for dynamic addr configuration.
			*/
			bcopy(a, &info->host_ip, IPV4_ADDR_LEN);
			info->filters |= IPV4_ARP_FILTER;
		}
		break;

		case IOV_SVAL(IOV_D0_FILTER_IPV4):
		{
			uint32	id;
			/* Add one IP address to the host IP table. */
			if (plen < sizeof(uint32))
				return BCME_BUFTOOSHORT;

			bcopy(a, &id, sizeof(id));
			ASSERT(id & (IPV6_FILTER | IPV4_FILTER));
			info->filters |= id;
		}
		break;

		case IOV_SVAL(IOV_D0_FILTER_MAX_DELAY):
		{
			uint32	delay;
			/* Add one IP address to the host IP table. */
			if (plen < sizeof(uint32))
				return BCME_BUFTOOSHORT;

			bcopy(a, &delay, sizeof(delay));
			info->max_delay = delay;
		}
		break;

		case IOV_SVAL(IOV_D0_FILTER_ARP_CLEAR):
		{
			/* Add one IP address to the host IP table. */
			if (plen < sizeof(struct ipv4_addr))
				return BCME_BUFTOOSHORT;

			/*
			* Requested ip-addr not found in the host_ip; add it.
			* Use link-local static address setting ONLY for testing
			* and warn.  Link-local is used for dynamic addr configuration.
			*/
			bzero(&info->host_ip, IPV4_ADDR_LEN);
			info->filters &= ~IPV4_ARP_FILTER;
		}
		break;

		case IOV_SVAL(IOV_D0_FILTER_IPV4_CLEAR):
		{
			uint32	id;

			/* Add one IP address to the host IP table. */
			if (plen < sizeof(uint32))
				return BCME_BUFTOOSHORT;

			bcopy(a, &id, sizeof(id));
			ASSERT(id & (IPV6_FILTER | IPV4_FILTER));
			info->filters &= ~id;
		}
		break;

		default:
		{
			err = BCME_UNSUPPORTED;
		}
		break;
	}

	return (err);
}

STATIC bool
run_arp_filter
(
	wlc_d0_filter_info_t	*info,
	struct	ether_addr	*daddr,
	uint8			*pt
)
{
	struct bcmarp *arp;

	arp = (struct bcmarp *)(pt + ETHER_TYPE_LEN);

	if (arp->oper != HTON16(ARP_OPC_REQUEST)) {
		WL_ERROR(("wl%d: run_arp_filter: oper not requrest, ignore\n",
		          WLCUNIT(info)));
		return FALSE;
	}

	if (!ether_isbcast(daddr->octet))
		return FALSE;
	if (IPV4_ADDR_NULL((struct ipv4_addr *)arp->src_ip)) {
		return FALSE;
	}

	if (bcmp(arp->dst_ip, (void *)&info->host_ip, IPV4_ADDR_LEN) == 0) {
		return FALSE;
	}

	return (TRUE);
}

STATIC bool
run_ipv4_filter
(
	wlc_d0_filter_info_t	*info,
	struct	ether_addr	*daddr,
	uint8			*pt,
	uint			len
)
{
	uint8			prot;
	struct ipv4_hdr *	iph;
	int			ihl;
	uint8 *			udph;
	uint16			port;

	iph = (struct ipv4_hdr *)(pt + ETHER_TYPE_LEN);
	ihl = IPV4_HLEN(iph);
	len -= (ETHER_TYPE_LEN + ihl);

	if (IP_VER(iph) != IP_VER_4) {
		WL_ERROR(("wl%d: run_ipv4_filter: not ipv4 pkt, ignored\n",
			WLCUNIT(info)));
		return FALSE;
	}

	/* check frame length, at least UDP_HDR_LEN */
	if (len <= UDP_HDR_LEN) {
		WL_ERROR(("wl%d: run_ipv4_filter: short UDP frame, ignored\n",
			WLCUNIT(info)));
		return FALSE;
	}

	prot = IPV4_PROT(iph);
	if (prot != IP_PROT_UDP) {
		return FALSE;
	}

	udph = (uint8 *)iph + ihl;
	port = ntoh16(*(uint16 *)(udph + UDP_DEST_PORT_OFFSET));
	if ((port == 137) && (info->filters & IPV4_NETBT_FILTER)) {
		WL_INFORM(("wl%d: run_ipv4_filter: found NetBT match\n", WLCUNIT(info)));
		return (ether_isbcast(daddr->octet));
	}
	if ((port == 1900) && (info->filters & IPV4_SSDP_FILTER)) {
		WL_INFORM(("wl%d: run_ipv4_filter: found ssdp match\n", WLCUNIT(info)));
		return (ETHER_ISMULTI(daddr->octet));
	}
	if ((port == 3702) && (info->filters & IPV4_WSD_FILTER)) {
		WL_INFORM(("wl%d: run_ipv4_filter: found WSDiscovery match\n", WLCUNIT(info)));
		return (ETHER_ISMULTI(daddr->octet));
	}
	if ((port == 5355) && (info->filters & IPV4_LLMNR_FILTER)) {
		WL_INFORM(("wl%d: run_ipv4_filter: found LLMNR match\n", WLCUNIT(info)));
		return (ETHER_ISMULTI(daddr->octet));
	}

	return (FALSE);
}

STATIC bool
run_ipv6_filter
(
	wlc_d0_filter_info_t	*info,
	struct	ether_addr	*daddr,
	uint8			*pt,
	uint			len
)
{
	uint8			prot;
	struct ipv6_hdr *	iph;
	int			ihl;
	uint8 *			udph;
	uint16			port;

	iph = (struct ipv6_hdr *)(pt + ETHER_TYPE_LEN);
	ihl = sizeof(struct ipv6_hdr);

	len -= (ihl + ETHER_TYPE_LEN);

	if (IP_VER(iph) != IP_VER_6) {
		WL_ERROR(("wl%d: run_ipv6_filter:  not ipv4 pkt, ignored\n",
			WLCUNIT(info)));
		return FALSE;
	}

	/* check frame length, at least UDP_HDR_LEN */
	if (len <= UDP_HDR_LEN) {
		WL_ERROR(("wl%d: run_ipv6_filter:  short UDP frame, ignored\n",
			WLCUNIT(info)));
		return FALSE;
	}

	prot = IPV6_PROT(iph);
	if (prot != IP_PROT_UDP) {
		return FALSE;
	}

	udph = (uint8 *)iph + ihl;
	port = ntoh16(*(uint16 *)(udph + UDP_DEST_PORT_OFFSET));
	if ((port == 1900) && (info->filters & IPV6_SSDP_FILTER)) {
		WL_INFORM(("wl%d: run_ipv6_filter: found ssdp match\n", WLCUNIT(info)));
		return (ETHER_ISMULTI(daddr->octet));
	}
	if ((port == 3702) && (info->filters & IPV6_WSD_FILTER)) {
		WL_INFORM(("wl%d: run_ipv6_filter: found WSDiscovery match\n", WLCUNIT(info)));
		return (ETHER_ISMULTI(daddr->octet));
	}
	if ((port == 5355) && (info->filters & IPV6_LLMNR_FILTER)) {
		WL_INFORM(("wl%d: run_ipv6_filter: found LLMNR match\n", WLCUNIT(info)));
		return (ETHER_ISMULTI(daddr->octet));
	}

	return (FALSE);
}

/*
*****************************************************************************
* Function:   run_ip_filter
*
* Purpose:    Run the specified filter against a received packet. Determine
*             if the received packet matches the filter specification.
*
* Parameters: info      (mod) Packet filter engine context state.
*             filter   (mod)   Filter to run.
*             sdu      (in)   Received packet.
*
* Returns:    TRUE for filter match, else FALSE.
*****************************************************************************
*/
STATIC bool
run_ip_filter
(
	wlc_d0_filter_info_t	*info,
	const void		*sdu
)
{
	uint8			*pkt;
	int			pkt_len;
	osl_t			*osh;
	uint16			ethertype;
	struct	ether_addr dst_addr;
	struct	ether_header *eth;
	osh = WLCOSH(info);

	pkt		= PKTDATA(osh, sdu);
	pkt_len		= PKTLEN(osh, sdu);

	/* Process Ethernet II or SNAP-encapsulated 802.3 frames */
	if (pkt_len < ETHER_HDR_LEN) {
		WL_ERROR(("wl%d: run_arp_filter: short eth frame (%d)\n",
		          WLCUNIT(info), pkt_len));
		return FALSE;
	}

#if defined(EXT_STA)
	if (WLEXTSTA_ENAB(info->wlc->pub) && !PKT_DOT3(osh, sdu)) {
		struct dot11_header *h = (struct dot11_header *)pkt;
		bcopy(h->a1.octet, dst_addr.octet, ETHER_ADDR_LEN);

		pkt += DOT11_MAC_HDR_LEN + SNAP_HDR_LEN;
		pkt_len -= (DOT11_MAC_HDR_LEN + SNAP_HDR_LEN);
	}
	else
#endif /* EXT_STA */
	if (ntoh16_ua((const void *)(pkt + ETHER_TYPE_OFFSET)) >= ETHER_TYPE_MIN) {
		/* Frame is Ethernet II */
		eth  = (struct ether_header *)pkt;
		bcopy(eth->ether_dhost, dst_addr.octet, ETHER_ADDR_LEN);

		pkt += ETHER_TYPE_OFFSET;
		pkt_len -= ETHER_TYPE_OFFSET;
		/* todo dst_addr */
	} else if (pkt_len >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN &&
	           !bcmp(llc_snap_hdr, pkt + ETHER_HDR_LEN, SNAP_HDR_LEN)) {
		WL_INFORM(("wl%d: run_arp_filter: 802.3 LLC/SNAP\n", WLCUNIT(info)));
		eth  = (struct ether_header *)pkt;
		bcopy(eth->ether_dhost, dst_addr.octet, ETHER_ADDR_LEN);
		pkt += ETHER_HDR_LEN + SNAP_HDR_LEN;
		pkt_len -= (ETHER_HDR_LEN + SNAP_HDR_LEN);
	} else {
		WL_ERROR(("wl%d: run_arp_filter: non-SNAP 802.3 frame\n",
		          WLCUNIT(info)));
		return FALSE;
	}

	ethertype = ntoh16_ua((const void *)pkt);

	if ((ethertype == ETHER_TYPE_ARP) && (info->filters & IPV4_ARP_FILTER)) {
		return run_arp_filter(info, &dst_addr, pkt);
	}
	if ((ethertype == ETHER_TYPE_IP) && (info->filters & IPV4_FILTER)) {
		return run_ipv4_filter(info, &dst_addr, pkt, pkt_len);
	}
	if ((ethertype == ETHER_TYPE_IPV6) && (info->filters & IPV6_FILTER)) {
		return run_ipv6_filter(info, &dst_addr, pkt, pkt_len);
	}

	return FALSE;
}
