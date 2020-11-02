/**
 * @file
 * @brief
 * Wireless EThernet (WET) Tunnel.
 *
 * WET STA and WET client are inter-exchangable in this file and refer to
 * addressable entities whose traffic are sent and received through this
 * bridge, including the hosting device.
 *
 * Supported protocol families: IP v4.
 *
 * Tx: replace frames' source MAC address with wireless interface's;
 * update the IP-MAC address mapping table entry.
 *
 * Rx: replace frames' the destination MAC address with what found in
 * the IP-MAC address mapping table.
 *
 * All data structures defined in this file are optimized for IP v4. To
 * support other protocol families, write protocol specific handlers.
 * Doing so may require data structures changes to expand various address
 * storages to fit the protocol specific needs, for example, IPX needs 10
 * octets for its network address. Also one may need to define the data
 * structures in a more generic way so that they work with all supported
 * protocol families, for example, the wet_sta strcture may be defined
 * as follow:
 *
 *	struct wet_tunnel_sta {
 *		uint8 nal;		network address length
 *		uint8 na[NETA_MAX_LEN];	network address
 *		uint8 mac[ETHER_ADDR_LEN];
 *		...
 *	};
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
 * $Id: wlc_wet_tunnel.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * Goal: allows connection of multiple ethernet devices to a wireless->ethernet bridging device.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlDriverWetTunneling]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <proto/802.11.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/802.3.h>
#include <proto/bcmip.h>
#include <proto/bcmarp.h>
#include <proto/bcmudp.h>
#include <proto/bcmdhcp.h>
#include <bcmendian.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wl_export.h>

#include <wlc_wet_tunnel.h>

/* IP/MAC address mapping entry */
typedef struct wet_tunnel_sta wet_tunnel_sta_t;
struct wet_tunnel_sta {
	/* client */
	uint8 ip[IPV4_ADDR_LEN];		/* client IP addr */
	struct ether_addr mac;			/* client MAC addr */
	uint8 flags[DHCP_FLAGS_LEN];		/* orig. dhcp flags */
	/* internal */
	wet_tunnel_sta_t *next;			/* free STA link */
	wet_tunnel_sta_t *next_ip;		/* hash link by IP */
	wet_tunnel_sta_t *next_mac;		/* hash link by MAC */

	struct ether_addr wetmac;		/* wet MAC addr */
	uint used;				/* time of last use */
};
#define WET_TUNNEL_NUMSTAS			(1 << 8) /* max. # clients, must be multiple of 2 */
#define WET_TUNNEL_STA_HASH_SIZE	(WET_TUNNEL_NUMSTAS/2)	 /* must be <= WET_TUNNEL_NUMSTAS */
#define WET_TUNNEL_STA_HASH_IP(ip)  \
	((ip)[3]&(WET_TUNNEL_STA_HASH_SIZE-1))			 /* hash by IP */
#define WET_TUNNEL_STA_HASH_MAC(ea)	\
	(((ea)[3]^(ea)[4]^(ea)[5])&(WET_TUNNEL_STA_HASH_SIZE-1)) /* hash by MAC */
#define WET_TUNNEL_STA_HASH_UNK		-1 /* Unknown hash */

/* WET tunnel private info structure */
struct wlc_wet_tunnel_info {
	/* pointer to wlc public info struct */
	wlc_pub_t *pub;
	/* pointer to wlc info struct */
	wlc_info_t *wlc;
	/* STA storage, one entry per eth. client */
	wet_tunnel_sta_t sta[WET_TUNNEL_NUMSTAS];
	/* Free sta list */
	wet_tunnel_sta_t *stafree;
	/* Used sta hash by IP */
	wet_tunnel_sta_t *stahash_ip[WET_TUNNEL_STA_HASH_SIZE];
	/* Used sta hash by MAC */
	wet_tunnel_sta_t *stahash_mac[WET_TUNNEL_STA_HASH_SIZE];
	/* inactivity timeout for WET tunnel STA */
	uint sta_timeout;
};

/* forward declarations */
static int wet_tunnel_eth_proc(wlc_wet_tunnel_info_t *weth, void *sdu, int send);
static int wet_tunnel_vtag_proc(wlc_wet_tunnel_info_t *weth,
	uint8 * eh, uint8 *vtag, int length, int send);
static int wet_tunnel_ip_proc(wlc_wet_tunnel_info_t *weth,
	uint8 * eh, uint8 *iph, int length, int send);
static int wet_tunnel_arp_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *arph, int length, int send);
static int wet_tunnel_udp_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, int length, int send);
static int wet_tunnel_dhcpc_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);
static int wet_tunnel_dhcps_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);

static int wet_tunnel_sta_alloc(wlc_wet_tunnel_info_t *weth, wet_tunnel_sta_t **saddr);
static int wet_tunnel_sta_update_all(wlc_wet_tunnel_info_t *weth,
	uint8 *iaddr, struct ether_addr *eaddr, struct ether_addr *waddr, wet_tunnel_sta_t **saddr);
static int wet_tunnel_sta_update_mac(wlc_wet_tunnel_info_t *weth,
	struct ether_addr *eaddr, struct ether_addr *waddr, wet_tunnel_sta_t **saddr);
static int wet_tunnel_sta_remove_mac_entry(wlc_wet_tunnel_info_t *weth, struct ether_addr *eaddr);
static int wet_tunnel_sta_find_ip(wlc_wet_tunnel_info_t *weth,
	uint8 *iaddr, wet_tunnel_sta_t **saddr);
static int wet_tunnel_sta_find_mac(wlc_wet_tunnel_info_t *weth,
	struct ether_addr *eaddr, wet_tunnel_sta_t **saddr);
static void wet_tunnel_sta_clean_all(wlc_wet_tunnel_info_t *weth);

static int wet_tunnel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);

/*
 * Protocol handler. 'ph' points to protocol specific header,
 * for example, it points to IP header if it is IP packet.
 */
typedef int (*prot_proc_t)(wlc_wet_tunnel_info_t *weth, uint8 *eh,
	uint8 *ph, int length, int send);
/* Protocol handlers hash table - hash by ether type */
typedef struct prot_hdlr prot_hdlr_t;
struct prot_hdlr {
	uint16 type;		/* ether type */
	prot_proc_t prot_proc;
	prot_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_TUNNEL_PROT_HASH_SIZE	(1 << 3)
#define WET_TUNNEL_PROT_HASH(t)		((t)[1]&(WET_TUNNEL_PROT_HASH_SIZE-1))
static prot_hdlr_t ept_tunnel_tbl[] = {
	/* 0 */ {HTON16(ETHER_TYPE_8021Q), wet_tunnel_vtag_proc, NULL}, /* 0x8100 */
};
static prot_hdlr_t prot_tunnel_hash[WET_TUNNEL_PROT_HASH_SIZE] = {
	/* 0 */ {HTON16(ETHER_TYPE_IP), wet_tunnel_ip_proc, &ept_tunnel_tbl[0]},	/* 0x0800 */
	/* 1 */ {0, NULL, NULL},	/* unused   */
	/* 2 */ {0, NULL, NULL},	/* unused   */
	/* 3 */ {0, NULL, NULL},	/* unused   */
	/* 4 */ {0, NULL, NULL},	/* unused   */
	/* 5 */ {0, NULL, NULL},	/* unused   */
	/* 6 */ {HTON16(ETHER_TYPE_ARP), wet_tunnel_arp_proc, &ept_tunnel_tbl[0]},	/* 0x0806 */
	/* 7 */ {0, NULL, NULL},	/* unused   */
};

/*
 * IPv4 handler. 'ph' points to protocol specific header,
 * for example, it points to UDP header if it is UDP packet.
 */
typedef int (*ipv4_proc_t)(wlc_wet_tunnel_info_t *weth, uint8 *eh,
	uint8 *iph, uint8 *ph, int length, int send);
/* IPv4 handlers hash table - hash by protocol type */
typedef struct ipv4_hdlr ipv4_hdlr_t;
struct ipv4_hdlr {
	uint8 type;		/* protocol type */
	ipv4_proc_t ipv4_proc;
	ipv4_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_TUNNEL_IPV4_HASH_SIZE	(1 << 1)
#define WET_TUNNEL_IPV4_HASH(p)		((p)&(WET_TUNNEL_IPV4_HASH_SIZE-1))
static ipv4_hdlr_t ipv4_tunnel_hash[WET_TUNNEL_IPV4_HASH_SIZE] = {
	/* 0 */ {0, NULL, NULL},				/* unused */
	/* 1 */ {IP_PROT_UDP, wet_tunnel_udp_proc, NULL},	/* 0x11 */
};

/*
 * UDP handler. 'ph' points to protocol specific header,
 * for example, it points to DHCP header if it is DHCP packet.
 */
typedef int (*udp_proc_t)(wlc_wet_tunnel_info_t *weth, uint8 *eh,
	uint8 *iph, uint8 *udph, uint8 *ph, int length, int send);
/* UDP handlers hash table - hash by port number */
typedef struct udp_hdlr udp_hdlr_t;
struct udp_hdlr {
	uint16 port;		/* udp dest. port */
	udp_proc_t udp_proc;
	udp_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_TUNNEL_UDP_HASH_SIZE	(1 << 3)
#define WET_TUNNEL_UDP_HASH(p)		((p)[1]&(WET_TUNNEL_UDP_HASH_SIZE-1))
static udp_hdlr_t udp_tunnel_hash[WET_TUNNEL_UDP_HASH_SIZE] = {
	/* 0 */ {0, NULL, NULL},	/* unused   */
	/* 1 */ {0, NULL, NULL},	/* unused   */
	/* 2 */ {0, NULL, NULL},	/* unused   */
	/* 3 */ {HTON16(DHCP_PORT_SERVER), wet_tunnel_dhcpc_proc, NULL}, /* 0x43 */
	/* 4 */ {HTON16(DHCP_PORT_CLIENT), wet_tunnel_dhcps_proc, NULL}, /* 0x44 */
	/* 5 */ {0, NULL, NULL},	/* unused   */
	/* 6 */ {0, NULL, NULL},	/* unused   */
	/* 7 */ {0, NULL, NULL},	/* unused   */
};

/* wlc_pub_t struct access macros */
#define WLCUNIT(weth)	((weth)->pub->unit)
#define WLCHWADDR(weth)	((weth)->pub->cur_etheraddr.octet)
#define WLCOSH(weth)	((weth)->pub->osh)

/* special values */
/* 802.3 llc/snap header */
static uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

/* IOVar table */
enum {
	IOV_WET_TUNNEL
};

static const bcm_iovar_t wet_tunnel_iovars[] = {
	{"wet_tunnel", IOV_WET_TUNNEL,
	(IOVF_SET_DOWN), IOVT_BOOL, 0
	},
	{NULL, 0, 0, 0, 0 }
};

/*
 * Initialize wet tunnel private context. It returns a pointer to the
 * wet tunnel private context if succeeded. Otherwise it returns NULL.
 */
wlc_wet_tunnel_info_t *
BCMATTACHFN(wlc_wet_tunnel_attach)(wlc_info_t *wlc)
{
	wlc_wet_tunnel_info_t *weth;
	int i;

	/* allocate wet private info struct */
	weth = MALLOC(wlc->pub->osh, sizeof(wlc_wet_tunnel_info_t));
	if (!weth) {
		WL_ERROR(("wl%d: wlc_wet_tunnel_attach: MALLOC failed, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->pub->osh)));
		return NULL;
	}

	/* init wet private info struct */
	bzero(weth, sizeof(wlc_wet_tunnel_info_t));
	for (i = 0; i < WET_TUNNEL_NUMSTAS - 1; i ++)
		weth->sta[i].next = &weth->sta[i + 1];
	weth->stafree = &weth->sta[0];
	weth->wlc = wlc;
	weth->pub = wlc->pub;

	/* inactivity timeout for WET STA */
	weth->sta_timeout = 180;		/* 180 seconds */

	/* register module */
	if (wlc_module_register(wlc->pub, wet_tunnel_iovars, "wet_tunnel", weth,
		wet_tunnel_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "wet_tunnel", (dump_fn_t)wlc_wet_tunnel_dump, (void *)weth);
#endif // endif

	return weth;

fail:
	wlc_wet_tunnel_detach(weth);
	return NULL;
}

/* Cleanup wet tunnel private context */
void
BCMATTACHFN(wlc_wet_tunnel_detach)(wlc_wet_tunnel_info_t *weth)
{
	if (!weth)
		return;
	wlc_module_unregister(weth->pub, "wet_tunnel", weth);
	MFREE(weth->pub->osh, weth, sizeof(wlc_wet_tunnel_info_t));
}

/* Handling WET related iovars */
static int
wet_tunnel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_wet_tunnel_info_t *weth = hdl;
	wlc_info_t *wlc = (wlc_info_t *)weth->pub->wlc;
	int32 int_val = 0;
	bool bool_val;
	int32 *ret_int_ptr;
	int err = 0;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)a;

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_GVAL(IOV_WET_TUNNEL):
		*ret_int_ptr = weth->pub->wet_tunnel;
		break;

	case IOV_SVAL(IOV_WET_TUNNEL):
		if (AP_ENAB(wlc->pub)) {
			if (weth->pub->wet_tunnel != bool_val) {
				wet_tunnel_sta_clean_all(weth);
				weth->pub->wet_tunnel = bool_val;
			}
		}
		else
			err = BCME_UNSUPPORTED;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* process Ethernet frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*	> 0 if no further process
*/
static int
wet_tunnel_eth_proc(wlc_wet_tunnel_info_t *weth, void *sdu, int send)
{
	osl_t *osh = WLCOSH(weth);
	uint8 *frame = PKTDATA(osh, sdu);
	int length = pkttotlen(osh, sdu);
	uint8 *pt = frame + ETHER_TYPE_OFFSET;
	uint16 type;
	uint8 *ph;
	prot_hdlr_t *phdlr;

	/* intercept Ethernet II frame (type > 1500) */
	if (length >= ETHER_HDR_LEN && (pt[0] > (ETHER_MAX_DATA >> 8) ||
	    (pt[0] == (ETHER_MAX_DATA >> 8) && pt[1] > (ETHER_MAX_DATA & 0xff))))
		;
	/* intercept 802.3 LLC/SNAP frame (type <= 1500) */
	else if (!send && (length >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN)) {
		uint8 *llc = frame + ETHER_HDR_LEN;
		if (bcmp(llc_snap_hdr, llc, SNAP_HDR_LEN))
			return 0;
		pt = llc + SNAP_HDR_LEN;
	}

	ph = PKTNEXT(osh, sdu) ? PKTDATA(osh, PKTNEXT(osh, sdu)) : (pt + ETHER_TYPE_LEN);
	length -= ((pt + ETHER_TYPE_LEN) - frame);

	ASSERT((PKTNEXT(osh, sdu) == NULL) ||
	       (PKTLEN(osh, sdu) == ((pt + ETHER_TYPE_LEN) - frame)));

	/* Call protocol specific handler to process frame. */
	type = *(uint16 *)pt;
	for (phdlr = &prot_tunnel_hash[WET_TUNNEL_PROT_HASH(pt)];
	     phdlr != NULL; phdlr = phdlr->next) {
		if (phdlr->type != type || !phdlr->prot_proc)
			continue;
		return (phdlr->prot_proc)(weth, frame, ph, length, send);
	}
	if (!bcmp(WLCHWADDR(weth), frame + ETHER_SRC_OFFSET, ETHER_ADDR_LEN)) {
		return 0;
	}
	else {
		WL_INFORM(("wl%d: %s: %s unknown type (0x%X), ignored %s\n",
			WLCUNIT(weth), __FUNCTION__, send ? "send" : "recv", type,
			(type == 0xDD86) ? "IPv6":""));
		/* ignore unsupported protocol from different mac addr than us */
		return BCME_UNSUPPORTED;
	}
}

/* process ARP frame - ARP proxy */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*   > 0 if no further process
*/
static int
wet_tunnel_arp_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *arph, int length, int send)
{
	wet_tunnel_sta_t *sta;
	uint8 *iaddr;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/*
	 * Replace source MAC in Ethernet header as well as source MAC in
	 * ARP protocol header when processing frame recv'd.
	 */
	if (!send) {
		/* this comes from other normal wireless station */
		if (!bcmp(eh + ETHER_SRC_OFFSET, arph + ARP_SRC_ETH_OFFSET, ETHER_ADDR_LEN) &&
			(*(uint16 *)(arph + ARP_OPC_OFFSET) == HTON16(ARP_OPC_REPLY))) {
			if (wet_tunnel_sta_find_mac(weth,
				(struct ether_addr *)(arph + ARP_TGT_ETH_OFFSET), &sta) < 0) {
				WL_ERROR(("wl%d: wet_tunnel_arp_proc: unable to find STA %s\n",
					WLCUNIT(weth),
					bcm_ether_ntoa(
						(struct ether_addr *)
						(arph + ARP_TGT_ETH_OFFSET), eabuf)));
				return 1;
			}
			bcopy(&sta->wetmac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
		} else {
			/* it's not from wet station */
			if (!bcmp(eh + ETHER_SRC_OFFSET, arph + ARP_SRC_ETH_OFFSET, ETHER_ADDR_LEN))
				return 1;
			iaddr = arph + ARP_SRC_IP_OFFSET;
			/* update wet host info */
			if (wet_tunnel_sta_update_all(weth, iaddr,
				(struct ether_addr *)(arph + ARP_SRC_ETH_OFFSET),
				(struct ether_addr *)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
				WL_ERROR(("wl%d: wet_tunnel_arp_proc: unable to update STA "
					"%u.%u.%u.%u %s\n",
					WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2], iaddr[3],
					bcm_ether_ntoa(
						(struct ether_addr *)
						(arph + ARP_SRC_ETH_OFFSET), eabuf)));
				return -1;
			}
			bcopy(&sta->mac, eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
			/* reply from other wet station */
			if (wet_tunnel_sta_find_mac(weth,
				(struct ether_addr *)(arph + ARP_TGT_ETH_OFFSET), &sta) == 0) {
				bcopy(&sta->wetmac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
			}
		}
	}
	/*
	 * Replace dest MAC in Ethernet header as well as dest MAC in
	 * ARP protocol header when processing frame send. It only
	 * needs to be done for ARP reply. ARP request received is
	 * expected to be a broadcast address.
	 */
	else if (*(uint16 *)(arph + ARP_OPC_OFFSET) == HTON16(ARP_OPC_REPLY)) {
		iaddr = arph + ARP_TGT_IP_OFFSET;
		if (wet_tunnel_sta_find_mac(weth,
			(struct ether_addr *)(arph + ARP_TGT_ETH_OFFSET), &sta) < 0) {
			return 1;
		}
		bcopy(&sta->wetmac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	}

	return 0;
}

/* process 8021p/Q tagged frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_tunnel_vtag_proc(wlc_wet_tunnel_info_t *weth,
	uint8 * eh, uint8 *vtag, int length, int send)
{
	uint16 type;
	uint8 *pt;
	prot_hdlr_t *phdlr;

	/* check minimum length */
	if (length < ETHERVLAN_HDR_LEN) {
		WL_ERROR(("wl%d: wet_vtag_proc: %s short VLAN frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}

	/*
	 * FIXME: check recursiveness to prevent stack from overflow
	 * in case someone sent frames 8100xxxxxxxx8100xxxxxxxx...
	 */

	/* Call protocol specific handler to process frame. */
	type = *(uint16 *)(pt = vtag + VLAN_TAG_LEN - 2);
	length -= VLAN_TAG_LEN;

	for (phdlr = &prot_tunnel_hash[WET_TUNNEL_PROT_HASH(pt)];
	     phdlr != NULL; phdlr = phdlr->next) {
		if (phdlr->type != type || !phdlr->prot_proc)
			continue;
		return (phdlr->prot_proc)(weth, eh, pt + ETHER_TYPE_LEN, length, send);
	}

	return 0;
}

/* process IP frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*	> 0 if no further process
*/
static int
wet_tunnel_ip_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *iph, int length, int send)
{
	uint8 type;
	int ihl;
	wet_tunnel_sta_t *sta = NULL;
	ipv4_hdlr_t *iphdlr;
	uint8 *iaddr;
	int ret;

	/* IPv4 only */
	if (length < 1 || (IP_VER(iph) != IP_VER_4)) {
		WL_ERROR(("wl%d: wet_tunnel_ip_proc: %s non IPv4 frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}

	ihl = IPV4_HLEN(iph);

	/* minimum length */
	if (length < ihl) {
		WL_ERROR(("wl%d: wet_tunnel_ip_proc: %s short IPv4 frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}

	/* protocol specific handling */
	type = IPV4_PROT(iph);
	for (iphdlr = &ipv4_tunnel_hash[WET_TUNNEL_IPV4_HASH(type)];
	     iphdlr; iphdlr = iphdlr->next) {
		if (iphdlr->type != type || !iphdlr->ipv4_proc)
			continue;
		if ((ret = (iphdlr->ipv4_proc)(weth, eh,
			iph, iph + ihl, length - ihl, send)))
			return ret;
	}

	/* generic IP packet handling */
	/*
	 * Replace src MAC in Ethernet header with host's.
	 */
	if (!send) {
		/* check if this comes from wet station */
		iaddr = iph + IPV4_SRC_IP_OFFSET;
		if (wet_tunnel_sta_find_ip(weth, iaddr, &sta) == 0) {
			bcopy(&sta->mac, eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
		}
		/* check if this comes to other wet station */
		if (!ETHER_ISMULTI(eh)) {
			wet_tunnel_sta_t *dsta;
			iaddr = iph + IPV4_DEST_IP_OFFSET;
			if (wet_tunnel_sta_find_ip(weth, iaddr, &dsta) == 0) {
				/* this should not be same as source wetmac */
				if (!sta || bcmp(&dsta->wetmac, &sta->wetmac, ETHER_ADDR_LEN)) {
					bcopy(&dsta->wetmac,
					      eh + ETHER_DEST_OFFSET,
					      ETHER_ADDR_LEN);
				}
			}
		}
	}
	/*
	 * Replace dest MAC in Ethernet header using the found one
	 * when sending frame.
	 */
	/* no action for sent bcast/mcast ethernet frame */
	else if (!ETHER_ISMULTI(eh)) {
		if (wet_tunnel_sta_find_mac(weth,
			(struct ether_addr *)(eh + ETHER_DEST_OFFSET), &sta) < 0) {
			return 1;
		}
		bcopy(&sta->wetmac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	}

	return 0;
}

/* process UDP frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*   > 0 if no further process
*/
static int
wet_tunnel_udp_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, int length, int send)
{
	udp_hdlr_t *udphdlr;
	uint16 port;

	/* check frame length, at least UDP_HDR_LEN */
	if ((length -= UDP_HDR_LEN) < 0) {
		WL_ERROR(("wl%d: wet_tunnel_udp_proc: %s short UDP frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}

	/*
	 * Unfortunately we must spend some time here to deal with
	 * some higher layer protocol special processings.
	 * See individual handlers for protocol specific details.
	 */
	port = *(uint16 *)(udph + UDP_DEST_PORT_OFFSET);
	for (udphdlr = &udp_tunnel_hash[WET_TUNNEL_UDP_HASH((uint8 *)&port)];
	     udphdlr; udphdlr = udphdlr->next) {
		if (udphdlr->port != port || !udphdlr->udp_proc)
			continue;
		return (udphdlr->udp_proc)(weth, eh, iph, udph,
			udph + UDP_HDR_LEN, length, send);
	}

	return 0;
}

/* process DHCP client frame (client to server, or server to relay agent) */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*   > 0 if no further process
*/
static int
wet_tunnel_dhcpc_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_tunnel_sta_t *sta;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/* only interested in requests when sending to server */
	if (!send && *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REQUEST)
		return 0;
	/* only interested in replies when receiving from server as a relay agent */
	if (send && *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return 0;
	/* it's not wet station */
	if (!bcmp(eh + ETHER_SRC_OFFSET, dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN))
		return 0;

	/*
	 * FIXME: validate DHCP body:
	 *  htype Ethernet 1, hlen Ethernet 6, frame length at least 242.
	 */

	/* received request */
	if (!send) {
		/* find existing or alloc new IP/MAC mapping entry */
		if (wet_tunnel_sta_update_mac(weth,
			(struct ether_addr *)(dhcp + DHCP_CHADDR_OFFSET),
			(struct ether_addr *)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
			WL_ERROR(("wl%d: wet_tunnel_dhcpc_proc: unable to update STA %s\n",
				WLCUNIT(weth),
				bcm_ether_ntoa(
					(struct ether_addr *)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
			return -1;
		}
		/* replace the Ethernet source MAC with host's */
		bcopy(dhcp + DHCP_CHADDR_OFFSET, eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
	}

	/* no further processing! */
	return 1;
}

/* process DHCP server frame (server to client) */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*   > 0 if no further process
*/
static int
wet_tunnel_dhcps_proc(wlc_wet_tunnel_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_tunnel_sta_t *sta;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/* only interested in replies when receiving from server */
	if (!send || *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return 0;

	/* find MAC mapping entry */
	if (wet_tunnel_sta_find_mac(weth,
		(struct ether_addr *)(dhcp + DHCP_CHADDR_OFFSET),
		&sta) < 0) {
		WL_ERROR(("wl%d: wet_tunnel_dhcps_proc: unable to find STA %s\n",
			WLCUNIT(weth),
			bcm_ether_ntoa((struct ether_addr *)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
		return 0;
	}
	/* replace the Ethernet destination MAC with broadcast */
	bcopy(&ether_bcast, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);

	/* no further processing! */
	return 1;
}

/* alloc IP/MAC mapping entry */
/* Returns 0 if succeeded; < 0 otherwise. */
static int
wet_tunnel_sta_alloc(wlc_wet_tunnel_info_t *weth, wet_tunnel_sta_t **saddr)
{
	wet_tunnel_sta_t *sta;

	/* allocate a new one */
	if (!weth->stafree) {
		WL_ERROR(("wl%d: wet_tunnel_sta_alloc: no room for another STA\n", WLCUNIT(weth)));
		return -1;
	}
	sta = weth->stafree;
	weth->stafree = sta->next;

	/* init them just in case */
	sta->next = NULL;
	sta->next_ip = NULL;
	sta->next_mac = NULL;

	sta->used = weth->pub->now;

	*saddr = sta;
	return 0;
}

/* update IP/MAC mapping entry and hash */
/* Returns 0 if succeeded; < 0 otherwise. */
static int
wet_tunnel_sta_update_all(wlc_wet_tunnel_info_t *weth, uint8 *iaddr, struct ether_addr *eaddr,
	struct ether_addr *waddr, wet_tunnel_sta_t **saddr)
{
	wet_tunnel_sta_t *sta;
	int i;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	/* find the existing one and remove it from the old IP hash link */
	if (!wet_tunnel_sta_find_mac(weth, eaddr, &sta)) {
		i = WET_TUNNEL_STA_HASH_IP(sta->ip);
		if (bcmp(sta->ip, iaddr, IPV4_ADDR_LEN)) {
			wet_tunnel_sta_t *sta2, **next;
			for (next = &weth->stahash_ip[i], sta2 = *next;
					sta2; sta2 = sta2->next_ip) {
				if (sta2 == sta)
					break;
				next = &sta2->next_ip;
			}
			if (sta2) {
				*next = sta2->next_ip;
				sta2->next_ip = NULL;
			}
			i = WET_TUNNEL_STA_HASH_UNK;
		}
		bcopy(waddr, &sta->wetmac, ETHER_ADDR_LEN);
	}
	/* allocate a new one and hash it by MAC */
	else if (!wet_tunnel_sta_alloc(weth, &sta)) {
		i = WET_TUNNEL_STA_HASH_MAC(eaddr->octet);
		bcopy(eaddr, &sta->mac, ETHER_ADDR_LEN);
		bcopy(waddr, &sta->wetmac, ETHER_ADDR_LEN);
		sta->next_mac = weth->stahash_mac[i];
		weth->stahash_mac[i] = sta;
		i = WET_TUNNEL_STA_HASH_UNK;
	}
	/* bail out if we can't find nor create any */
	else {
		WL_ERROR(("wl%d: wet_tunnel_sta_update_all: unable to alloc STA %u.%u.%u.%u %s\n",
			WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2], iaddr[3],
			bcm_ether_ntoa(eaddr, eabuf)));
		return -1;
	}

	/* update IP and hash by new IP */
	if (i == WET_TUNNEL_STA_HASH_UNK) {
		i = WET_TUNNEL_STA_HASH_IP(iaddr);
		bcopy(iaddr, sta->ip, IPV4_ADDR_LEN);
		sta->next_ip = weth->stahash_ip[i];
		weth->stahash_ip[i] = sta;

		/* start here and look for other entries with same IP address */
		{
			wet_tunnel_sta_t *sta2, *prev;
			prev = sta;
			for (sta2 = sta->next_ip;	sta2; sta2 = sta2->next_ip) {
				/* does this entry have the same IP address? */
				if (!bcmp(sta->ip, sta2->ip, IPV4_ADDR_LEN)) {
					/* sta2 currently points to the entry we need to remove */
					/* fix next pointers */
					prev->next_ip = sta2->next_ip;
					sta2->next_ip = NULL;
					/* now we need to find this guy in the MAC list and
					   remove it from that list too.
					*/
					wet_tunnel_sta_remove_mac_entry(weth, &sta2->mac);
					/* entry should be completely out of the table now,
					   add it to the free list
					*/
					memset(sta2, 0, sizeof(wet_tunnel_sta_t));
					sta2->next = weth->stafree;
					weth->stafree = sta2;

					sta2 = prev;
				}
				prev = sta2;
			}
		}
	}

	*saddr = sta;
	return 0;
}

/* clear IP/MAC mapping entry and hash */
static void
wet_tunnel_sta_clean_all(wlc_wet_tunnel_info_t *weth)
{
	int i;

	bzero(weth->sta, sizeof(wet_tunnel_sta_t)*WET_TUNNEL_NUMSTAS);
	bzero(weth->stahash_mac, sizeof(wet_tunnel_sta_t *)*WET_TUNNEL_STA_HASH_SIZE);
	bzero(weth->stahash_ip, sizeof(wet_tunnel_sta_t *)*WET_TUNNEL_STA_HASH_SIZE);
	for (i = 0; i < WET_TUNNEL_NUMSTAS - 1; i++)
		weth->sta[i].next = &weth->sta[i + 1];
	weth->stafree = &weth->sta[0];
}

/* update IP/MAC mapping entry and hash */
static int
wet_tunnel_sta_update_mac(wlc_wet_tunnel_info_t *weth, struct ether_addr *eaddr,
	struct ether_addr *waddr, wet_tunnel_sta_t **saddr)
{
	wet_tunnel_sta_t *sta;
	int i;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/* find the existing one */
	if (!wet_tunnel_sta_find_mac(weth, eaddr, &sta))
		;
	/* allocate a new one and hash it */
	else if (!wet_tunnel_sta_alloc(weth, &sta)) {
		i = WET_TUNNEL_STA_HASH_MAC(eaddr->octet);
		bcopy(eaddr, &sta->mac, ETHER_ADDR_LEN);
		bcopy(waddr, &sta->wetmac, ETHER_ADDR_LEN);
		sta->next_mac = weth->stahash_mac[i];
		weth->stahash_mac[i] = sta;
	}
	/* bail out if we can't find nor create any */
	else {
		WL_ERROR(("wl%d: wet_tunnel_sta_update_mac: unable to alloc STA %s\n",
			WLCUNIT(weth), bcm_ether_ntoa(eaddr, eabuf)));
		return -1;
	}

	*saddr = sta;
	return 0;
}

/*  Remove MAC entry from hash list
    NOTE:  This only removes the entry matching "eaddr" from the MAC
    list.  The caller needs to remove from the IP list and
    put back onto the free list to completely remove the entry
    from the WET table.
*/
static int
wet_tunnel_sta_remove_mac_entry(wlc_wet_tunnel_info_t *weth, struct ether_addr *eaddr)
{
	wet_tunnel_sta_t *sta, *prev;
	int i = WET_TUNNEL_STA_HASH_MAC(eaddr->octet);
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	int found = 0;

	/* find the existing one */
	for (sta = prev = weth->stahash_mac[i]; sta; sta = sta->next_mac) {
		if (!bcmp(&sta->mac, eaddr, ETHER_ADDR_LEN)) {
			found = 1;
			break;
		}
		prev = sta;
	}

	/* bail out if we can't find */
	if (!found) {
		WL_ERROR(("wl%d: wet_tunnel_sta_remove_mac_entry: unable to find STA %s entry\n",
			WLCUNIT(weth), bcm_ether_ntoa(eaddr, eabuf)));
		return -1;
	}

	/* fix the list */
	if (prev == sta)
		weth->stahash_mac[i] = sta->next_mac; /* removing first entry in this bucket */
	else
		prev->next_mac = sta->next_mac;

	return 0;
}

/* find IP/MAC mapping entry by IP address */
/* Returns 0 if succeeded; < 0 otherwise. */
static int
wet_tunnel_sta_find_ip(wlc_wet_tunnel_info_t *weth, uint8 *iaddr, wet_tunnel_sta_t **saddr)
{
	int i = WET_TUNNEL_STA_HASH_IP(iaddr);
	wet_tunnel_sta_t *sta;

	/* find the existing one by IP */
	for (sta = weth->stahash_ip[i]; sta; sta = sta->next_ip) {
		if (bcmp(sta->ip, iaddr, IPV4_ADDR_LEN))
			continue;
		sta->used = weth->pub->now;
		*saddr = sta;
		return 0;
	}
	return -1;
}

/* find IP/MAC mapping entry by MAC address */
/* Returns 0 if succeeded; < 0 otherwise. */
static int
wet_tunnel_sta_find_mac(wlc_wet_tunnel_info_t *weth,
	struct ether_addr *eaddr, wet_tunnel_sta_t **saddr)
{
	int i = WET_TUNNEL_STA_HASH_MAC(eaddr->octet);
	wet_tunnel_sta_t *sta;

	/* find the existing one by MAC */
	for (sta = weth->stahash_mac[i]; sta; sta = sta->next_mac) {
		if (bcmp(&sta->mac, eaddr, ETHER_ADDR_LEN))
			continue;
		sta->used = weth->pub->now;
		*saddr = sta;
		return 0;
	}
	return -1;
}

/*
* Process frames in transmit direction by replacing destination MAC with
* wireless's and keep track of IP MAC address mapping table.
* Return:
*	= 0 if frame is done ok;
*	< 0 if unable to handle the frame;
*
* To avoid other interfaces to see our changes specially
* changes to broadcast frame which definitely will be seen by
* other bridged interfaces we must copy the frame to our own
* buffer, modify it, and then sent it.
* Return the new sdu in 'new'.
*/
int
wlc_wet_tunnel_send_proc(wlc_wet_tunnel_info_t *weth, void *sdu)
{
	/* process frame */
	return wet_tunnel_eth_proc(weth, sdu, 1) < 0 ? -1 : 0;
}

/*
* Process frames in receive direction by replacing destination MAC with
* the one found in IP MAC address mapping table.
* Return:
*	= 0 if frame is done ok;
*	< 0 if unable to handle the frame;
*/
int
wlc_wet_tunnel_recv_proc(wlc_wet_tunnel_info_t *weth, void *sdu)
{
	/* process frame */
	return wet_tunnel_eth_proc(weth, sdu, 0) < 0 ? -1 : 0;
}

/*
 * Process multicast frames in receive direction by making a packet copy,
 * writing back the sender address and then sending onto BSS.
 * Return:
 *	= 0 if frame is forwarded ok;
 *	< 0 if fail to forward the frame;
 */
int
wlc_wet_tunnel_multi_packet_forward(wlc_info_t *wlc, osl_t *osh,
	struct scb *scb, struct wlc_if *wlcif, void *sdu)
{
	void *tmp;
	uint32 totlen = pkttotlen(osh, sdu);

	/* Since wet tunnel is going to modify the packet
	 * make a copy before sending on to BSS.
	 */
	if ((tmp = PKTGET(osh, TXOFF + totlen, TRUE)) != NULL) {
		PKTPULL(osh, tmp, TXOFF);
		wlc_pkttag_info_move(wlc, sdu, tmp);
		PKTSETPRIO(tmp, PKTPRIO(sdu));
		pktcopy(osh, sdu, 0, totlen, PKTDATA(osh, tmp));
		/* Write back the sender address */
		bcopy(&scb->ea, PKTDATA(osh, tmp) + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
		wlc_sendpkt(wlc, tmp, wlcif);

		return 0;
	}

	return -1;
}

#ifdef BCMDBG
int
wlc_wet_tunnel_dump(wlc_wet_tunnel_info_t *weth, struct bcmstrbuf *b)
{
	char eabuf1[ETHER_ADDR_STR_LEN];
	char eabuf2[ETHER_ADDR_STR_LEN];
	wet_tunnel_sta_t *sta;
	int i;

	bcm_bprintf(b, "Entry\tEnetAddr\t\tWetAddr\t\t\tInetAddr\tIdle (seconds)\n");
	for (i = 0; i < WET_TUNNEL_NUMSTAS; i++) {
		if (i == (WET_TUNNEL_NUMSTAS - 1))
			continue;
		if (weth->sta[i].next)
			continue;
		/* format the entry dump */
		sta = &weth->sta[i];
		bcm_bprintf(b, "%u\t%s\t%s\t%u.%u.%u.%u\t%d\n",
			i,
			bcm_ether_ntoa(&sta->mac, eabuf1),
			bcm_ether_ntoa(&sta->wetmac, eabuf2),
			sta->ip[0], sta->ip[1], sta->ip[2], sta->ip[3],
			weth->pub->now - sta->used);
	}

	return 0;
}
#endif	/* #ifdef BCMDBG */
