/*
 * Neighbor Advertisement Offload
 *
 * @file
 * @brief
 * The dongle should be able to handle Neighbor Solicitation request and reply with Neighbor
 * Advertisement without having to wake up the host.
 *
 * The code below implements the Neighbor Advertisement Offload.
 * It supports multihoming hosts and link-local addressing.
 *
 * Supported protocol families: IPV6.
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
 * $Id: wl_ndoe.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * IPv6 implements Neighbor Solicitation and Neighbor Advertisement to determine/advertise the link
 * layer address of a node (similar to ARP in IPv4). NS packets are multicast packets and are
 * received by multiple hosts on the same subnet, each demands processing by the host requiring it
 * to wake up. This features implements the neighbor advertisement in the wl to save power by
 * avoiding some of host wake-ups.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [NeighborSolicitationOffload]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <proto/ethernet.h>
#include <proto/802.3.h>
#include <proto/bcmip.h>
#include <proto/bcmipv6.h>
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#endif  /* WLBTAMP */
#include <bcmendian.h>
#include <d11.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wl_export.h>
#ifdef BCM_OL_DEV
#include <bcm_ol_msg.h>
#include <wlc_dngl_ol.h>
#else
#include <wlc_bsscfg.h>
#endif // endif
#include <wl_ndoe.h>

#if defined(BCMDBG) || defined(WLMSG_INFORM)
#define WL_MSGBUF
#endif // endif

#ifdef BCM_OL_DEV
#define ND_OL_IDX 0
#endif // endif

#ifdef WLNDOE_RA
#define WL_ND_MAX_RA_NODES 10
typedef struct wl_nd_ra_node {
	void *cached_packet;
	uint32 cached_packet_len;
	uint32 count;
	uint8 sa_ip[IPV6_ADDR_LEN];
	uint32	time_stamp;
} wl_nda_node_t;

typedef struct wl_nd_ra_info {
	struct wl_nd_ra_node nodes[WL_ND_MAX_RA_NODES];
	uint8 ra_filter_enable;
	uint8 ra_filter_ucast;
	uint8 ring_index;
} wl_nd_ra_info_t;
#endif /* WLNDOE_RA */

/* Neighbor Discovery private info structure */
struct wl_nd_info {
	wlc_info_t			*wlc;		/* Pointer back to wlc structure */
	struct ipv6_addr	host_ip[ND_MULTIHOMING_MAX];
	struct ipv6_addr	solicit_ip;	/* local Solicit Ip Address */
	uint8				host_mac[ETHER_ADDR_LEN];
	struct wlc_if 		*wlcif;
	struct ipv6_addr	remote_ip;	/* Win8 specific */
	struct nd_ol_stats_t	nd_ol_stats;
	bool pkt_snooping;
#ifdef BCM_OL_DEV
	olmsg_nd_stats stats;
	bool	nd_enabled;
#endif // endif
	nd_param_t				param[ND_REQUEST_MAX];
	uint8					req_count;
#ifdef WLNDOE_RA
	struct wl_nd_ra_info *ra_info;
#endif // endif
};

#ifdef WLNDOE_RA
static int
wl_nd_ra_filter_init(struct wl_nd_info *ndi);
static int
wl_nd_ra_filter_deinit(struct wl_nd_info *ndi);
static int
wl_nd_ra_intercept_packet(struct wl_nd_info *ndi, void *sdu, uint8 *sa, uint8 *da);
#endif /* WLNDOE_RA */

/* forward declarations */
#ifndef BCM_OL_DEV
static int nd_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
                       const char *name, void *p, uint plen, void *a, int alen,
                       int vsize, struct wlc_if *wlcif);
#endif // endif
static int wl_ns_parse(wl_nd_info_t *ndi, void *sdu, bool sentby_host);

static int na_reply_peer(wl_nd_info_t *ndi, struct bcm_nd_msg *ns_req,
	struct nd_msg_opt *options, struct ether_header *eth_hdr,
	struct ipv6_hdr *ip_hdr, bool snap, uint8 req_index);

/* wlc_pub_t struct access macros */
#define WLCUNIT(ndi)	((ndi)->wlc->pub->unit)
#define WLCOSH(ndi)		((ndi)->wlc->osh)

/* special values */
/* 802.3 llc/snap header */
static const uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
static uint16 csum_ipv6(wl_nd_info_t *ndi, uint8 *saddr, uint8 *daddr,
	uint8 proto, uint8 *buf, uint32 buf_len);
static uint32 csum_partial_16(uint8 *nptr, int nlen, uint32 x);

/* IOVar table */
enum {
	IOV_ND_HOSTIP,			/* Add local ip address to host_ip[] table */
	IOV_ND_HOSTIP_CLEAR,	/* Clear all entries in the host_ip[] table */
	IOV_ND_MAC_ADDRESS,		/* Set MAC address */
	IOV_ND_REMOTEIP,		/* Remote IP Win8 Req */
	IOV_ND_STATUS,
	IOV_ND_SNOOP,			/* Enable/Dis NA packet SNOOPING */
	IOV_ND_SOLICITIP,
	IOV_ND_STATUS_CLEAR,		/* clears all the ND status counters */
	IOV_ND_SET_PARAM,
	IOV_ND_CLEAR_PARAM,
	IOV_ND_RA_FILTER,
	IOV_ND_RA_FILTER_UCAST,
	IOV_ND_RA_FILTER_CACHE_CLEAR
};

static const bcm_iovar_t nd_iovars[] = {
	{"nd_hostip", IOV_ND_HOSTIP,
	(0), IOVT_BUFFER, IPV6_ADDR_LEN
	},
	{"nd_hostip_clear", IOV_ND_HOSTIP_CLEAR,
	(0), IOVT_VOID, 0
	},
	{"nd_macaddr", IOV_ND_MAC_ADDRESS,
	(0), ETHER_ADDR_LEN, 0
	},
	{"nd_status", IOV_ND_STATUS,
	(0), IOVT_BUFFER, sizeof(struct nd_ol_stats_t)
	},
	{"nd_snoop", IOV_ND_SNOOP,
	(0), IOVT_BOOL, 0
	},
	{"nd_status_clear", IOV_ND_STATUS_CLEAR,
	(0), IOVT_VOID, 0
	},
	{"nd_set_param", IOV_ND_SET_PARAM,
	(0), IOVT_BUFFER, sizeof(nd_param_t)
	},
	{"nd_clear_param", IOV_ND_CLEAR_PARAM,
	(0), IOVT_UINT32, sizeof(uint32)
	},
	{"nd_ra_filter_enable", IOV_ND_RA_FILTER,
	(0), IOVT_UINT32, sizeof(uint32)
	},
	{"nd_ra_filter_ucast", IOV_ND_RA_FILTER_UCAST,
	(0), IOVT_UINT32, sizeof(uint32)
	},
	{"nd_ra_filter_cache_clear", IOV_ND_RA_FILTER_CACHE_CLEAR,
	(0), IOVT_VOID, 0
	},
	{NULL, 0, 0, 0, 0 }
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/*
 * Initialize ND private context.
 * Returns a pointer to the ND private context, NULL on failure.
 */
wl_nd_info_t *
BCMATTACHFN(wl_nd_attach)(wlc_info_t *wlc)
{
	wl_nd_info_t *ndi;

	/* allocate ND private info struct */
	ndi = MALLOCZ(wlc->osh, sizeof(wl_nd_info_t));
	if (!ndi) {
		WL_ERROR(("wl%d: wl_nd_attach: MALLOC failed; total mallocs %d bytes\n",
		          wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init ND private info struct */
	ndi->wlc = wlc;

#ifndef BCM_OL_DEV
	/* register module */
	if (wlc_module_register(wlc->pub, nd_iovars, "nd", ndi, nd_doiovar,
		NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		MFREE(WLCOSH(ndi), ndi, sizeof(wl_nd_info_t));
		return NULL;
	}
#endif // endif

#ifdef WLNDOE_RA
	wl_nd_ra_filter_init(ndi);
#endif // endif

	return ndi;
}

void
BCMATTACHFN(wl_nd_detach)(wl_nd_info_t *ndi)
{
	WL_INFORM(("wl%d: nd_detach()\n", WLCUNIT(ndi)));

	if (!ndi)
		return;

#ifdef WLNDOE_RA
	wl_nd_ra_filter_deinit(ndi);
#endif // endif

#ifndef BCM_OL_DEV
	wlc_module_unregister(ndi->wlc->pub, "nd", ndi);
#endif // endif
	MFREE(WLCOSH(ndi), ndi, sizeof(wl_nd_info_t));
}

#ifndef BCM_OL_DEV
/* Handling ND-related iovars */
static int
nd_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wl_nd_info_t *ndi = hdl;
	int err = 0;
	int i;
	struct ipv6_addr *ipaddr;
	uint32 *ret_int_ptr = (uint32 *)a;
	int32 int_val = 0;

#ifndef BCMROMOFFLOAD
	WL_INFORM(("wl%d: nd_doiovar()\n", WLCUNIT(ndi)));
#endif /* !BCMROMOFFLOAD */

	/* change ndi if wlcif is corr to a virtualIF */
	if (wlcif != NULL) {
		if (wlcif->wlif != NULL) {
			ndi = (wl_nd_info_t *)wl_get_ifctx(ndi->wlc->wl, IFCTX_NDI,
			                                   wlcif->wlif);
		}
	}

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (actionid) {
		case IOV_SVAL(IOV_ND_HOSTIP):
		{
			/* Add one IP address to the host IP table. */
			if (plen < sizeof(struct ipv6_addr))
				return BCME_BUFTOOSHORT;

			if (ETHER_ISNULLADDR(&ndi->host_mac)) {
				bcopy(&ndi->wlc->pub->cur_etheraddr,
					&ndi->host_mac, ETHER_ADDR_LEN);
			}

			ipaddr = (struct ipv6_addr *)p;
			if (IPV6_ADDR_NULL(ipaddr->addr)) {
				WL_INFORM(("wl%d: nd_doiovar(), NULL IP address\n",
					WLCUNIT(ndi)));
				break;
			}

			/* Check if the ip is already in the table */
			for (i = 0; i < ND_MULTIHOMING_MAX; i++) {
				if (!bcmp(ipaddr->addr, ndi->host_ip[i].addr, IPV6_ADDR_LEN)) {
					err = 0;
					return err;
				}
			}

			/*
			 * Requested ip-addr not found in the host_ip[] table; add it.
			 */
			/* copy the host-requested ip-addr into an empty entry of host_ip[] */
			err = BCME_NORESOURCE;

			for (i = 0; i < ND_MULTIHOMING_MAX; i++) {
				if (IPV6_ADDR_NULL(ndi->host_ip[i].addr)) {
					bcopy(p, ndi->host_ip[i].addr, IPV6_ADDR_LEN);
					err = 0;
					ndi->nd_ol_stats.host_ip_entries++;
					break;
				}
			}

			if (i == ND_MULTIHOMING_MAX) {
				ndi->nd_ol_stats.host_ip_overflow++;
			}
			break;
		}

		case IOV_GVAL(IOV_ND_HOSTIP):
		{
			uint8 *hst_ip = (uint8 *)a;
			/*
			 * Return all IP addresses from host table.
			 * The return buffer is a list of valid IP addresses
			 * terminated by an address of all zeroes.
			 */
			for (i = 0; i < ND_MULTIHOMING_MAX; i++) {
				if (!IPV6_ADDR_NULL(ndi->host_ip[i].addr)) {
					if (alen < sizeof(struct ipv6_addr))
						return BCME_BUFTOOSHORT;
					bcopy(ndi->host_ip[i].addr, hst_ip, IPV6_ADDR_LEN);
					hst_ip += IPV6_ADDR_LEN;
					alen -= sizeof(struct ipv6_addr);
				}
			}

			if (alen < sizeof(struct ipv6_addr))
				return BCME_BUFTOOSHORT;

			bzero(hst_ip, IPV6_ADDR_LEN);
			break;
		}

		case IOV_SVAL(IOV_ND_HOSTIP_CLEAR):
		{
			for (i = 0; i < ND_MULTIHOMING_MAX; i++) {
				bzero(ndi->host_ip[i].addr, IPV6_ADDR_LEN);
			}

			ndi->nd_ol_stats.host_ip_entries = 0;
			break;
		}

		case IOV_GVAL(IOV_ND_MAC_ADDRESS):
		{
			if (alen < ETHER_ADDR_LEN)
				return BCME_BUFTOOSHORT;

			bcopy(&ndi->host_mac, a, ETHER_ADDR_LEN);
			break;
		}

		case IOV_SVAL(IOV_ND_MAC_ADDRESS):
		{
			if (plen < ETHER_ADDR_LEN)
				return BCME_BUFTOOSHORT;

			if (!ETHER_ISNULLADDR(p)) {
				bcopy(p, &ndi->host_mac, ETHER_ADDR_LEN);
			}
			break;
		}

		case IOV_GVAL(IOV_ND_STATUS):
		{
			if (alen < sizeof(struct nd_ol_stats_t))
				return BCME_BUFTOOSHORT;

			bcopy((uint8*)&ndi->nd_ol_stats, a, sizeof(struct nd_ol_stats_t));
			break;
		}

		case IOV_GVAL(IOV_ND_SNOOP):
			{
			if (alen < sizeof(uint32))
				return BCME_BUFTOOSHORT;

			*ret_int_ptr = (int32)ndi->pkt_snooping;
			}
			break;

		case IOV_SVAL(IOV_ND_SNOOP):
			ndi->pkt_snooping = (int_val != 0);
			break;

		case IOV_SVAL(IOV_ND_STATUS_CLEAR):
		{
			ndi->nd_ol_stats.host_ip_overflow = 0;
			ndi->nd_ol_stats.peer_reply_drop = 0;
			ndi->nd_ol_stats.peer_request = 0;
			ndi->nd_ol_stats.peer_request_drop = 0;
			ndi->nd_ol_stats.peer_service = 0;
			break;
		}

		case IOV_SVAL(IOV_ND_SET_PARAM):
		{
			if (ndi->req_count == ND_REQUEST_MAX) {
				WL_ERROR(("wl%d: IOV_ND_SET_PARAM: no room for new req [%d]\n",
					WLCUNIT(ndi), ndi->req_count));
				err = BCME_ERROR;
				break;
			}

			bcopy(p, &ndi->param[ndi->req_count], sizeof(nd_param_t));

			if (ETHER_ISNULLADDR(&ndi->param[ndi->req_count].host_mac)) {
				bcopy(&ndi->wlc->pub->cur_etheraddr,
					&ndi->param[ndi->req_count].host_mac, ETHER_ADDR_LEN);
			}

			ndi->req_count++;
			break;
		}

		case IOV_SVAL(IOV_ND_CLEAR_PARAM):
		{
			uint8 to_copy;

			if (ndi->req_count == 0) {
				err = BCME_ERROR;
				break;
			}

			for (i = 0; i < ndi->req_count; i++) {
				if (ndi->param[i].offload_id == int_val) {
					to_copy = (ndi->req_count - i - 1);
					/* shift the data 1 level up */
					if (to_copy) {
						bcopy(&ndi->param[i], &ndi->param[i+1],
						(sizeof(nd_param_t) * (ndi->req_count - i - 1)));
					}
					else {
						bzero(&ndi->param[i], sizeof(nd_param_t));
					}
					ndi->req_count--;
				}
			}
			break;
		}

		case IOV_SVAL(IOV_ND_RA_FILTER):
		{
#ifdef WLNDOE_RA
			if (ndi && ndi->ra_info) {
				ndi->ra_info->ra_filter_enable =  int_val;
			}
#else
			err = BCME_UNSUPPORTED;
#endif // endif

			break;
		}

		case IOV_GVAL(IOV_ND_RA_FILTER):
		{
#ifdef WLNDOE_RA
			if (ndi && ndi->ra_info)
				*ret_int_ptr = (int32)ndi->ra_info->ra_filter_enable;
			else
				*ret_int_ptr = 0;
#else
			err = BCME_UNSUPPORTED;
#endif // endif
			break;
		}
		case IOV_SVAL(IOV_ND_RA_FILTER_UCAST):
		{
#ifdef WLNDOE_RA
			if (ndi && ndi->ra_info) {
				ndi->ra_info->ra_filter_ucast =  (int_val != 0);
			}
#else
			err = BCME_UNSUPPORTED;
#endif // endif

			break;
		}

		case IOV_GVAL(IOV_ND_RA_FILTER_UCAST):
		{
#ifdef WLNDOE_RA
			if (ndi && ndi->ra_info)
				*ret_int_ptr = (int32)ndi->ra_info->ra_filter_ucast;
			else
				*ret_int_ptr = 0;
#else
			err = BCME_UNSUPPORTED;
#endif // endif
			break;
		}
		case IOV_SVAL(IOV_ND_RA_FILTER_CACHE_CLEAR):
		{
#ifdef WLNDOE_RA
			wl_nd_ra_filter_clear_cache(ndi);
#else
			err = BCME_UNSUPPORTED;
#endif // endif
			break;
		}

		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}
#endif /* BCM_OL_DEV */

/*
 * Process ND frames in receive direction.
 *
 * Return value:
 *	-1					Packet parsing error
 *	ND_REQ_SINK			NS/NA packet not for local host
 *  ND_REPLY_PEER		Sent NA resp from firmware
 *  ND_FORCE_FORWARD	Fw the packet to host
 */
int
wl_nd_recv_proc(wl_nd_info_t *ndi, void *sdu)
{
	int ret;

	WL_INFORM(("wl%d: wl_nd_recv_proc()\n", WLCUNIT(ndi)));

#ifdef BCM_OL_DEV
	if (!ndi->nd_enabled)
		return -1;
#endif // endif

	/* Parse NS packet and do table lookups */
	ret = wl_ns_parse(ndi, sdu, FALSE);

	switch (ret)
	{
		case ND_REPLY_PEER:
			ndi->nd_ol_stats.peer_service++;
			break;

		case ND_REQ_SINK:
			ndi->nd_ol_stats.peer_request_drop++;
			break;

		default:
			break;
	}

	return ret;
}

/*
 *  The received packet formats are different when EXT_STA is enabled. In case
 *  of EXT_STA the received packets are in 802.11 format, where as in other
 *  case the received packets have Ethernet II format
 *
 *  1. 802.11 frames
 *  -------------------------------------------------------------------------------------------
 *  | FC(2) | DID(2) |  A1(6) |  A2(6) | A3(6) | SID(2) | SNAP(6) | type(2) | data(46 - 1500) |
 *  -------------------------------------------------------------------------------------------
 *
 *  2. Ethernet II frames
 *  ---------------------------------------------
 *  | DA(6) | SA(6) | type(2) | data(46 - 1500) |
 *  ---------------------------------------------
 */
/* Returns -1 if frame is not IP; otherwise, returns pointer/length of IP portion */
static int
wl_ns_parse(wl_nd_info_t *ndi, void *sdu, bool sentby_host)
{
	uint8 *frame = PKTDATA(WLCOSH(ndi), sdu);
	int length = PKTLEN(WLCOSH(ndi), sdu);
	uint8 *pt;
#if defined(EXT_STA)
	struct ether_header eth_pkt;
#endif // endif
	struct ether_header *eth = NULL;
	struct ipv6_hdr *ip = NULL;
	struct bcm_nd_msg *ns_req = NULL;
	struct nd_msg_opt *ns_req_opt = NULL;
	int i;
	int ret = -1;
	uint16 ns_pktlen;
#ifdef BCM_OL_DEV
	olmsg_nd_stats *pstats = &ndi->stats;
#endif // endif

#if defined(EXT_STA)
	if (WLEXTSTA_ENAB(ndi->wlc->pub))
		ns_pktlen = (DOT11_MAC_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN +
		            sizeof(struct bcm_nd_msg)+ sizeof(struct ipv6_hdr));
	else
#endif /* EXT_STA */
		ns_pktlen = (ETHER_HDR_LEN + sizeof(struct bcm_nd_msg)+ sizeof(struct ipv6_hdr));
#if defined(EXT_STA)
	if (WLEXTSTA_ENAB(ndi->wlc->pub)) {
		if (ndi->req_count == 0) {
			return ret;
		}
	}
	else
#endif // endif
	{
		if (IPV6_ADDR_NULL(ndi->host_ip[0].addr)) {
			return ret;
		}
	}

	/* Check if the pkt lenght is atleast the NS pkt req size */
	if (length < ns_pktlen) {
		WL_INFORM(("wl%d: wl_nd_parse: short eth frame (%d)\n",
		      WLCUNIT(ndi), length));
		return -1;
	}

#if defined(EXT_STA)
	if (WLEXTSTA_ENAB(ndi->wlc->pub) && !PKT_DOT3(ndi->wlc->osh, sdu)) {
		struct dot11_header *h = (struct dot11_header *)frame;
		/* Create a ether Header from 802.11 header */
		eth = &eth_pkt;

		/* No need of eth header since the tx is used only for SNOOPING */
		if (sentby_host) {
			uint16 fc = ntoh16(h->fc);
			bcopy(h->a1.octet, &eth->ether_dhost, ETHER_ADDR_LEN);

			if (fc & FC_FROMDS) {
				bcopy(h->a3.octet, eth->ether_shost, ETHER_ADDR_LEN);
			}
			else {
				bcopy(h->a2.octet, eth->ether_shost, ETHER_ADDR_LEN);
			}
		}

		pt = frame + DOT11_MAC_HDR_LEN + SNAP_HDR_LEN;
		eth->ether_type = *((uint16*)(pt));
		pt += 2;
	}
	else
#endif /* EXT_STA */
	{
		/* Process Ethernet II or SNAP-encapsulated 802.3 frames */
		if (ntoh16_ua((const void *)(frame + ETHER_TYPE_OFFSET)) >= ETHER_TYPE_MIN) {
			/* Frame is Ethernet II */
			eth  = (struct ether_header *)frame;
			pt = frame + ETHER_HDR_LEN;
		} else if (length >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN &&
		           !bcmp(llc_snap_hdr, frame + ETHER_HDR_LEN, SNAP_HDR_LEN)) {
			WL_INFORM(("wl%d: wl_ns_parse: 802.3 LLC/SNAP\n", WLCUNIT(ndi)));
			eth  = (struct ether_header *)frame;
			pt = frame + ETHER_HDR_LEN + SNAP_HDR_LEN;
#ifdef WLBTAMP
		} else if (length >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN &&
		           !bcmp(BT_SIG_SNAP_MPROT, frame + ETHER_HDR_LEN,
		                 DOT11_LLC_SNAP_HDR_LEN - 2)) {
			/* ignore BTAMP frame */
			return -1;
#endif /* WLBTAMP */
		} else {
			WL_ERROR(("wl%d: wl_ns_parse: non-SNAP 802.3 frame\n",
			          WLCUNIT(ndi)));
			return -1;
		}
	}

	ip = (struct ipv6_hdr *)pt;
	pt +=  sizeof(struct ipv6_hdr);
	ns_req = (struct bcm_nd_msg *)pt;

	if ((ntoh16(eth->ether_type) != ETHER_TYPE_IPV6) ||
		(ip->nexthdr != ICMPV6_HEADER_TYPE)) {
		return ret;
	}

#ifdef BCM_OL_DEV
			/* Total number of ICMPV6 packets received */
			RXOEINC(ndi->wlc->wlc_dngl_ol, rxoe_totalndcnt);
#endif // endif

	if (length >= (ns_pktlen + sizeof(struct nd_msg_opt))) {
		ns_req_opt = (struct nd_msg_opt *)(pt + sizeof(struct bcm_nd_msg));
	}

#ifdef WLNDOE_RA
	if (ns_req->icmph.icmp6_type == ICMPV6_PKT_TYPE_RA) {
		return wl_nd_ra_intercept_packet(ndi, (void *)ip,
			eth->ether_shost, eth->ether_dhost);
	}
#endif // endif

	if (sentby_host) {
		/* SNOOP host ip from the NA host response */
		if (ns_req->icmph.icmp6_type == ICMPV6_PKT_TYPE_NA)  {
			bcopy(ip->saddr.addr, ndi->host_ip[0].addr, IPV6_ADDR_LEN);

			if (ETHER_ISNULLADDR(&ndi->host_mac)) {
				bcopy(&ndi->wlc->pub->cur_etheraddr, &ndi->host_mac,
					ETHER_ADDR_LEN);
			}

			ndi->nd_ol_stats.host_ip_entries++;
		}
		return -1;
	}

	if (ns_req->icmph.icmp6_type == ICMPV6_PKT_TYPE_NS &&
		!IPV6_ADDR_NULL(ns_req->target.addr)) {
		ndi->nd_ol_stats.peer_request++;

#ifdef BCM_OL_DEV
		/* Total number of NS packets received */
		RXOEINC(ndi->wlc->wlc_dngl_ol, rxoe_nscnt);
		bzero(pstats, sizeof(olmsg_nd_stats));
		bcopy(&(ns_req->target), &(pstats->dest_ip), IPV6_ADDR_LEN);
		pstats->is_request = TRUE;
		pstats->armtx = arm_dotx(ndi->wlc);

		if (IPV6_ADDR_NULL(ip->saddr.addr))
			return 0;
#endif /* BCM_OL_DEV */

		ret = ND_REQ_SINK;

#if defined(EXT_STA)
		if (WLEXTSTA_ENAB(ndi->wlc->pub)) {
			/* Check if pkt dst m/ucast addr matches offload req */
			for (i = 0; i < ndi->req_count; i++) {
				nd_param_t *req = &ndi->param[i];
				if  (IPV6_ADDR_NULL(req->solicit_ip.addr) ||
					!bcmp(ip->daddr.addr, req->solicit_ip.addr,
					IPV6_ADDR_LEN) ||
					(!IPV6_ADDR_NULL(req->host_ip[0].addr) &&
					!bcmp(ip->daddr.addr, req->host_ip[0].addr,
					IPV6_ADDR_LEN)) ||
					(!IPV6_ADDR_NULL(req->host_ip[1].addr) &&
					!bcmp(ip->daddr.addr, req->host_ip[1].addr,
					IPV6_ADDR_LEN))) {

					/* respond only to remote_ip if set */
					if (!IPV6_ADDR_NULL(req->remote_ip.addr) &&
						bcmp(ip->saddr.addr, req->remote_ip.addr,
						IPV6_ADDR_LEN)) {
						continue;
					}

					/* Verify the target ipv6 addr */
					if (!bcmp(ns_req->target.addr,
					   req->host_ip[0].addr, IPV6_ADDR_LEN) ||
					   !bcmp(ns_req->target.addr,
					   req->host_ip[1].addr, IPV6_ADDR_LEN)) {
						ret = na_reply_peer(ndi, ns_req, ns_req_opt,
							eth, ip, FALSE, i);
						break;
					}
				}
			}
		}
		else
#endif /* EXT_STA */
		{

			/* Check if the ipv6 target addess is for the local
			 * host
			 */
			for (i = 0; i < ND_MULTIHOMING_MAX; i++) {
				if (!bcmp(ns_req->target.addr, ndi->host_ip[i].addr,
					IPV6_ADDR_LEN)) {
#ifdef BCM_OL_DEV
					if (arm_dotx(ndi->wlc)) {
						RXOEINC(ndi->wlc->wlc_dngl_ol, rxoe_nsresponsecnt);
						pstats->resp_sent = TRUE;
						ret = na_reply_peer(ndi, ns_req, ns_req_opt,
							eth, ip, FALSE, i);
					}
					else
						ret = 0;
#else
					ret = na_reply_peer(ndi, ns_req, ns_req_opt,
						eth, ip, FALSE, i);
#endif // endif
					break;
				}
			}
		}
	}
	return ret;
}

static int
na_reply_peer(wl_nd_info_t *ndi, struct bcm_nd_msg *ns_req, struct nd_msg_opt *options,
	struct ether_header *eth_hdr, struct ipv6_hdr *ip_hdr, bool snap, uint8 req_index)
{
	void *pkt;
	uint8 *frame;
	uint16 ns_pktlen = (ETHER_HDR_LEN + sizeof(struct bcm_nd_msg)+ sizeof(struct nd_msg_opt) +
		sizeof(struct ipv6_hdr) + ((snap == TRUE) ? (SNAP_HDR_LEN + ETHER_TYPE_LEN) : 0));
	struct ether_header *na_eth_hdr = NULL;
	struct ipv6_hdr *na_ip_hdr = NULL;
	struct bcm_nd_msg *na_res = NULL;
	struct nd_msg_opt *na_res_opt = NULL;

	WL_INFORM(("wl%d: na_reply_peer()\n", WLCUNIT(ndi)));

	if (!(pkt = PKTGET(WLCOSH(ndi), ns_pktlen, TRUE))) {
		WL_ERROR(("wl%d: nd_reply_peer: alloc failed; dropped\n",
		          WLCUNIT(ndi)));
		WLCNTINCR(ndi->wlc->pub->_cnt->rxnobuf);
		return -1;
	}

	WL_INFORM(("wl%d: na_reply_peer: servicing request from peer\n", WLCUNIT(ndi)));

	frame = PKTDATA(WLCOSH(ndi), pkt);
	bzero(frame, ns_pktlen);

	na_eth_hdr = (struct ether_header *)frame;
	frame += ETHER_HDR_LEN;

	if (snap) {
		bcopy(llc_snap_hdr, frame + ETHER_HDR_LEN, SNAP_HDR_LEN);
		hton16_ua_store(ETHER_TYPE_IPV6, frame + ETHER_HDR_LEN + SNAP_HDR_LEN);
		frame += (SNAP_HDR_LEN + ETHER_TYPE_LEN);
	}

	na_ip_hdr = (struct ipv6_hdr *)frame;
	na_res = (struct bcm_nd_msg *)(((uint8*)na_ip_hdr) + sizeof(struct ipv6_hdr));
	na_res_opt = (struct nd_msg_opt *)((uint8 *)na_res + sizeof(struct bcm_nd_msg));

	/* Create 14-byte eth header, plus snap header if applicable */
#if defined(EXT_STA)
	if (WLEXTSTA_ENAB(ndi->wlc->pub))
		bcopy(ndi->param[req_index].host_mac, na_eth_hdr->ether_shost, ETHER_ADDR_LEN);
	else
#endif // endif
		bcopy(ndi->host_mac, na_eth_hdr->ether_shost, ETHER_ADDR_LEN);

	/* Get the Dst Mac from the options field if available */
	if (options != NULL && options->type == ICMPV6_ND_OPT_TYPE_SRC_MAC) {
		bcopy(options->mac_addr, na_eth_hdr->ether_dhost, ETHER_ADDR_LEN);
	}
	else {
		bcopy(eth_hdr->ether_shost, na_eth_hdr->ether_dhost, ETHER_ADDR_LEN);
	}

	na_eth_hdr->ether_type = hton16(ETHER_TYPE_IPV6);

	/* Create IPv6 Header */
	if (IPV6_ADDR_NULL(ip_hdr->saddr.addr)) {
		bcopy(all_node_ipv6_maddr.addr, na_ip_hdr->daddr.addr, IPV6_ADDR_LEN);
		na_res->icmph.opt.nd_advt.solicited = 0;

	}
	else {
		bcopy(ip_hdr->saddr.addr, na_ip_hdr->daddr.addr, IPV6_ADDR_LEN);
		na_res->icmph.opt.nd_advt.solicited = 1;
	}

	bcopy(ns_req->target.addr, na_ip_hdr->saddr.addr, IPV6_ADDR_LEN);
	na_ip_hdr->payload_len = hton16(sizeof(struct bcm_nd_msg) + sizeof(struct nd_msg_opt));
	na_ip_hdr->nexthdr = ICMPV6_HEADER_TYPE;
	na_ip_hdr->hop_limit = IPV6_HOP_LIMIT;
	na_ip_hdr->version = IPV6_VERSION;

	/* Create Neighbor Advertisement Msg (ICMPv6) */
	na_res->icmph.icmp6_type = ICMPV6_PKT_TYPE_NA;
	na_res->icmph.icmp6_code = 0;
	na_res->icmph.opt.nd_advt.override = 1;
	bcopy(ns_req->target.addr, na_res->target.addr, IPV6_ADDR_LEN);

	/* Create Neighbor Advertisement Opt Header (ICMPv6) */
	na_res_opt->type = ICMPV6_ND_OPT_TYPE_TARGET_MAC;
	na_res_opt->len = 1;
#if defined(EXT_STA)
	if (WLEXTSTA_ENAB(ndi->wlc->pub))
		bcopy(ndi->param[req_index].host_mac, na_res_opt->mac_addr, ETHER_ADDR_LEN);
	else
#endif // endif
		bcopy(ndi->host_mac, na_res_opt->mac_addr, ETHER_ADDR_LEN);

	/* Calculate Checksum */
	na_res->icmph.icmp6_cksum = csum_ipv6(ndi, na_ip_hdr->saddr.addr, na_ip_hdr->daddr.addr,
		ICMPV6_HEADER_TYPE, (uint8*)na_res,
		(sizeof(struct bcm_nd_msg) + sizeof(struct nd_msg_opt)));

	wlc_sendpkt(ndi->wlc, pkt, ndi->wlcif);

	return ND_REPLY_PEER;
}

/*
 * Process ND (NS/NA) frames in transmit direction.
 *
 * Return value:
 *	0		ND processing not enabled
 *	-1		Packet parsing error
 */
int
wl_nd_send_proc(wl_nd_info_t *ndi, void *sdu)
{
	WL_INFORM(("wl%d: wl_nd_send_proc()\n", WLCUNIT(ndi)));
	if (ndi->pkt_snooping && IPV6_ADDR_NULL(ndi->host_ip[0].addr)) {
		wl_ns_parse(ndi, sdu, TRUE);
	}

	return 0;
}

/* called when a new virtual IF is created.
 *	i/p: primary NDIIF [ndi_p] and the new wlcif,
 *	o/p: new ndi structure populated with inputs and
 *		the global parameters duplicated from ndi_p
 *	side-effects: ndi for a new IF will inherit properties of ndi_p till
 *		the point new ndi is created. After that, for any change in
 *		ndi_p will NOT change the ndi corr to new IF. To change property
 *		of new IF, wl -i wl0.x has to be used.
*/
wl_nd_info_t *
wl_nd_alloc_ifndi(wl_nd_info_t *ndi_p, wlc_if_t *wlcif)
{
	wl_nd_info_t *ndi;
	wlc_info_t *wlc = ndi_p->wlc;

	/* allocate ND private info struct */
	ndi = MALLOCZ(wlc->osh, sizeof(wl_nd_info_t));
	if (!ndi) {
		WL_ERROR(("wl%d: wl_nd_alloc_ifndi: MALLOCZ failed; total mallocs %d bytes\n",
		          wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init nd private info struct */
	ndi->wlc = wlc;
	ndi->wlcif = wlcif;

	return ndi;
}

void
wl_nd_free_ifndi(wl_nd_info_t *ndi)
{
	if (ndi != NULL) {
		MFREE(WLCOSH(ndi), ndi, sizeof(wl_nd_info_t));
	}
}

void
wl_nd_clone_ifndi(wl_nd_info_t *from_ndi, wl_nd_info_t *to_ndi)
{
	wlc_if_t *wlcif = to_ndi->wlcif;
	wlc_info_t *wlc = to_ndi->wlc;
	bcopy(from_ndi, to_ndi, sizeof(wl_nd_info_t));
	to_ndi->wlc = wlc;
	to_ndi->wlcif = wlcif;
}

/* Parital ip checksum algorithm */
static uint32
csum_partial_16(uint8 *nptr, int nlen, uint32 x)
{
	uint32 new;

	while (nlen)
	{
		new = (nptr[0] << 8) + nptr[1];
		nptr += 2;
		x += new & 0xffff;
		if (x & 0x10000) {
			x++;
			x &= 0xffff;
		}
		nlen -= 2;
	}

	return x;
}

/*
 * Caclulates the checksum for the pseodu IP hdr + NS req/res
 *
 */
static uint16 csum_ipv6(wl_nd_info_t *ndi, uint8 *saddr, uint8 *daddr,
	uint8 proto, uint8 *buf, uint32 buf_len)
{
	uint16 ret;
	uint32 cksum;
	uint32 len = hton32(buf_len);
	uint8 prot[4] = {0, 0, 0, 0};

	prot[3] = proto;

	cksum = csum_partial_16(saddr, IPV6_ADDR_LEN, 0);
	cksum = csum_partial_16(daddr, IPV6_ADDR_LEN, cksum);
	cksum = csum_partial_16((uint8*)&len, 4, cksum);
	cksum = csum_partial_16(prot, 4, cksum);
	cksum = csum_partial_16(buf, buf_len, cksum);

	cksum = ~cksum & 0xFFFF;
	hton16_ua_store((uint16)cksum, &ret);

	return ret;
}

#ifdef BCM_OL_DEV
/* TBD: Lets keep it for now */

void wl_nd_update_stats(wl_nd_info_t *ndi, bool suppressed)
{
	ndi->stats.suppressed = suppressed;
	RXOEADDNDENTRY(ndi->wlc->wlc_dngl_ol, ndi->stats);
}

void wl_nd_proc_msg(wlc_dngl_ol_info_t *wlc_dngl_ol, wl_nd_info_t *ndi, void *buf)
{
	uchar *pktdata;
	olmsg_header *msg_hdr;
	olmsg_nd_enable *nd_enable;
	olmsg_nd_setip *nd_setip;
	int i;

	pktdata = (uint8 *) buf;
	msg_hdr = (olmsg_header *) pktdata;

	WL_TRACE(("dongle :message type:%d\n", msg_hdr->type));

	switch (msg_hdr->type) {

		case BCM_OL_ND_ENABLE:
			nd_enable = (olmsg_nd_enable *)pktdata;
			bcopy(nd_enable->host_mac.octet,
				ndi->host_mac, ETHER_ADDR_LEN);
			bcopy(&(nd_enable->host_mac), &(wlc_dngl_ol->cur_etheraddr),
				sizeof(struct ether_addr));

			wlc_dngl_ol_sec_info_from_host(wlc_dngl_ol, &nd_enable->host_mac,
				&nd_enable->BSSID, &nd_enable->sec_info);

			ndi->nd_enabled = TRUE;
			break;

		case BCM_OL_ND_DISABLE:
			bzero(ndi->host_ip, (ND_MULTIHOMING_MAX * IPV6_ADDR_LEN));
			bzero(ndi->host_mac, ETHER_ADDR_LEN);
			ndi->nd_enabled = FALSE;
			break;

		case BCM_OL_ND_SETIP:
			nd_setip = (olmsg_nd_setip *)pktdata;

			if (ETHER_ISNULLADDR(&ndi->host_mac)) {

				bcopy(&ndi->wlc->pub->cur_etheraddr,
					&ndi->host_mac, ETHER_ADDR_LEN);
			}

			/* Check if the ip is already in the table */
			for (i = 0; i < ND_MULTIHOMING_MAX; i++) {
				if (!bcmp(nd_setip->host_ip.addr,
					ndi->host_ip[i].addr, IPV6_ADDR_LEN)) {
					WL_ERROR(("%s: IPv6 addr already present in hostip array\n",
						__FUNCTION__));
					break;

				} else {

					/*
					 * Requested ip-addr not found in the
					 * host_ip[] table; add it.
					 */
					 /* copy the host-requested ip-addr
					 * into an empty entry of host_ip[]
					 */

					if (IPV6_ADDR_NULL(ndi->host_ip[i].addr)) {

						bcopy(nd_setip->host_ip.addr,
							ndi->host_ip[i].addr,
							IPV6_ADDR_LEN);
						WL_INFORM(("%s: IPv6 addr added at index %d\n",
							__FUNCTION__, i));
						ndi->nd_ol_stats.host_ip_entries++;
						break;
					}
				}
			}

			if (i == ND_MULTIHOMING_MAX) {
				WL_INFORM(("%s: HostIP overflow !!!\n", __FUNCTION__));
				ndi->nd_ol_stats.host_ip_overflow++;
			}

			break;
		default:
			WL_ERROR(("INVALID message type:%d\n", msg_hdr->type));
	}
}
#endif /* BCM_OL_DEV */

#ifdef WLNDOE_RA
static int
wl_nd_ra_filter_init(wl_nd_info_t *ndi)
{
	ndi->ra_info = MALLOC(WLCOSH(ndi), sizeof(wl_nd_ra_info_t));
	if (!ndi->ra_info) {
		WL_ERROR(("wl: wl_nd_ra_filter_init: MALLOC failed; \n"));
		return NULL;
	}

	bzero(ndi->ra_info, sizeof(wl_nd_ra_info_t));
	/* By default, RA FILTER is disabled */
	ndi->ra_info->ra_filter_enable = 0;
	return 0;
}

int
wl_nd_ra_filter_clear_cache(wl_nd_info_t *ndi)
{
	int i;
	wl_nda_node_t *node;

	if (!ndi || !ndi->ra_info)
		return -1;

	for (i = 0; i < WL_ND_MAX_RA_NODES; i++) {
		node = &ndi->ra_info->nodes[i];

		if (node && node->cached_packet) {
			/* free the cached packet */
			WL_INFORM(("Freeing cached packet_ptr:%p\n", node->cached_packet));
			MFREE(WLCOSH(ndi), node->cached_packet, node->cached_packet_len);
			node->cached_packet_len = 0;
			node->cached_packet = NULL;
			node->count = 0;
			bzero(node->sa_ip,  IPV6_ADDR_LEN);
		}
	}

	return 0;
}

static int
wl_nd_ra_filter_deinit(wl_nd_info_t *ndi)
{
	WL_TRACE(("%s: Enter..\n", __func__));

	if (!ndi || !ndi->ra_info)
		return -1;

	wl_nd_ra_filter_clear_cache(ndi);

	MFREE(WLCOSH(ndi), ndi->ra_info, sizeof(wl_nd_ra_info_t));

	return 0;
}

/* byte offset of lifetime into payload */
#define RA_LIFETIME_OFFSET     6
/* RA rate limit interval in millisecs */
#define RA_RATELIMIT_INTERVAL  60000
/* min. RA lifetime threshold before rate limiting kicks in
 * if RA lifetime is "short" relative to rate limit interval, just cache it
 * "short" is currently defined as 3 times the rate limit interval
 */
#define RA_MIN_LT_THRESHOLD    (3 * RA_RATELIMIT_INTERVAL)
#define SEC_TO_MS              1000
static int
wl_nd_ra_intercept_packet(wl_nd_info_t *ndi, void *ip, uint8 *sa, uint8 *da)
{
	int ret = 0;
	bool filter = FALSE;
	struct ipv6_hdr *iphdr = (struct ipv6_hdr *)ip;
	uint32 pkt_len;
	uint8 count = 0;
	wl_nda_node_t *node;
	bool match = FALSE;
	uint8 index = 0;

	if (!ndi || !ndi->ra_info || !iphdr) {
		WL_INFORM(("%s: ndi not initialized ndi:%p, ra_info:%p iphdr:%p\n",
			__func__, ndi, ndi->ra_info, iphdr));
		return -1;
	}

	pkt_len = ntoh16(iphdr->payload_len);
	/* Filter is enabled by default, if NDOE is enabled. But this
	 * can be explicitly disabled via iovar
	 */
	if (!ndi->ra_info->ra_filter_enable) {
		WL_INFORM(("%s: Filter explicitly disabled! Return. %d\n",
			__func__, ndi->ra_info->ra_filter_enable));
		return 0;
	}

	 if (ETHER_ISMULTI(da) || ndi->ra_info->ra_filter_ucast) {
		/* RA lifetime in sec */
		 uint16 lifetime = ntoh16(*(uint16 *)((uint8 *)ip
					+ (sizeof(struct ipv6_hdr)) + RA_LIFETIME_OFFSET));

		/* Router advertisement with Multicast DA */
		while (count < WL_ND_MAX_RA_NODES) {
			node = &ndi->ra_info->nodes[count];
			if (!bcmp(node->sa_ip, iphdr->saddr.addr, IPV6_ADDR_LEN)) {
				match = TRUE;
				WL_INFORM((" Packet from an already cached SRC IP ADDR \n"));

				if ((node->cached_packet_len == pkt_len) &&
					!bcmp((uint8 *)node->cached_packet,
					((uint8 *)ip + (sizeof(struct ipv6_hdr))), pkt_len)) {
					if (lifetime * SEC_TO_MS <= RA_MIN_LT_THRESHOLD) {
						WL_INFORM(("RA lifetime not much less than "
							"rate limit interval: Send it up\n"));
						index = count;
						break;
					}

					if (OSL_SYSUPTIME() >=
						(node->time_stamp + RA_RATELIMIT_INTERVAL)) {
						WL_INFORM(("Identical RA received outside "
							"rate limit interval: Send it up\n"));
						index = count;
						break;
					}

					/* The cached content matches.
					 * Discard the newly arrived frame
					 */
					node->count++;
					WL_INFORM(("Cached content matches. Filter out"
							" the frame. counter:%d \n", node->count));
					filter = TRUE;
					break;
				} else {
					/* Content doesn't match. Cach it up and send it up */
					WL_INFORM((" Packet content doesn't match with the "
							"cached content. Send it up \n"));
					index = count;
					break;
				}
			}
			count++;
		 }

		if (!match) {
			/* Use ring index. Ring Index would always point to available/oldest slot */
			if (ndi->ra_info->ring_index >= WL_ND_MAX_RA_NODES)
				ndi->ra_info->ring_index = 0;
			 index = ndi->ra_info->ring_index;
			 /* Increment and Point to the oldest entry */
			 ndi->ra_info->ring_index++;
		}

		if (!filter) {
			/* Cache the frame */
			node = &ndi->ra_info->nodes[index];
			bcopy(iphdr->saddr.addr, node->sa_ip, IPV6_ADDR_LEN);
			if (node->cached_packet)
				MFREE(WLCOSH(ndi), node->cached_packet, node->cached_packet_len);
			node->cached_packet = MALLOC(WLCOSH(ndi), pkt_len);
			if (!node->cached_packet) {
				WL_ERROR(("%s: Caching failed. Send the packet up \n", __func__));
				return 0;
			}
			bcopy(((uint8 *)ip + sizeof(struct ipv6_hdr)),
				node->cached_packet, pkt_len);
			node->cached_packet_len = pkt_len;
			node->time_stamp = OSL_SYSUPTIME();
		}

	 } else {
			/* Directed Frame. Give it up */
			WL_INFORM(("%s: Not a multicast packet/unicast filter is enabled(%d) \n",
				__func__, ndi->ra_info->ra_filter_ucast));
	 }

	if (filter) {
		ret = ND_REQ_SINK;
	}

	return ret;
}
#endif /* WLNDOE_RA */
