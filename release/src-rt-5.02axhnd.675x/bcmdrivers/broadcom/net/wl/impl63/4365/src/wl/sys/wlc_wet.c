/**
 * @file
 * @brief
 * Wireless EThernet (WET) Bridge.
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
 *	struct wet_sta {
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
 * $Id: wlc_wet.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WirelessEthernet]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
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

#include <wlc_wet.h>

/* IP/MAC address mapping entry */
typedef struct wet_sta wet_sta_t;
struct wet_sta {
	/* client */
	uint8 ip[IPV4_ADDR_LEN];	/* client IP addr */
	struct ether_addr mac;	/* client MAC addr */
	uint8 flags[DHCP_FLAGS_LEN];	/* orig. dhcp flags */
	/* internal */
	wet_sta_t *next;	/* free STA link */
	wet_sta_t *next_ip;	/* hash link by IP */
	wet_sta_t *next_mac;	/* hash link by MAC */
};
#define WET_NUMSTAS	(1 << 8)	/* max. # clients, must be multiple of 2 */
#define WET_STA_HASH_SIZE	(WET_NUMSTAS/2)	/* must be <= WET_NUMSTAS */
#define WET_STA_HASH_IP(ip)	((ip)[3]&(WET_STA_HASH_SIZE-1))	/* hash by IP */
#define WET_STA_HASH_MAC(ea)	(((ea)[3]^(ea)[4]^(ea)[5])&(WET_STA_HASH_SIZE-1)) /* hash by MAC */
#define WET_STA_HASH_UNK	-1 /* Unknown hash */

#define IP_ISMULTI(ip)           (((ip) & 0xf0000000) == 0xe0000000) /* Check for multicast by IP */

/* WET private info structure */
struct wlc_wet_info {
	/* pointer to wlc public info struct */
	wlc_pub_t *pub;
	/* Host addresses */
	uint8 ip[IPV4_ADDR_LEN];
	struct ether_addr mac;
	/* STA storage, one entry per eth. client */
	wet_sta_t sta[WET_NUMSTAS];
	/* Free sta list */
	wet_sta_t *stafree;
	/* Used sta hash by IP */
	wet_sta_t *stahash_ip[WET_STA_HASH_SIZE];
	/* Used sta hash by MAC */
	wet_sta_t *stahash_mac[WET_STA_HASH_SIZE];
};

/* forward declarations */
static int wet_eth_proc(wlc_wet_info_t *weth, void *sdu,
	uint8 *frame, int length, int send);
static int wet_vtag_proc(wlc_wet_info_t *weth, void *sdu,
	uint8 * eh, uint8 *vtag, int length, int send);
static int wet_ip_proc(wlc_wet_info_t *weth, void *sdu,
	uint8 * eh, uint8 *iph, int length, int send);
static int wet_arp_proc(wlc_wet_info_t *weth, void *sdu,
	uint8 *eh, uint8 *arph, int length, int send);
static int wet_udp_proc(wlc_wet_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, int length, int send);
static int wet_dhcpc_proc(wlc_wet_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);
static int wet_dhcps_proc(wlc_wet_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);
static int wet_sta_alloc(wlc_wet_info_t *weth, wet_sta_t **saddr);
static int wet_sta_update_all(wlc_wet_info_t *weth,
	uint8 *iaddr, struct ether_addr *eaddr, wet_sta_t **saddr);
static int wet_sta_update_mac(wlc_wet_info_t *weth,
	struct ether_addr *eaddr, wet_sta_t **saddr);
static int wet_sta_remove_mac_entry(wlc_wet_info_t *weth, struct ether_addr *eaddr);
static int wet_sta_find_ip(wlc_wet_info_t *weth,
	uint8 *iaddr, wet_sta_t **saddr);
static int wet_sta_find_mac(wlc_wet_info_t *weth,
	struct ether_addr *eaddr, wet_sta_t **saddr);

static void csum_fixup_16(uint8 *chksum,
	uint8 *optr, int olen, uint8 *nptr, int nlen);

static int wet_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);

/*
 * Protocol handler. 'ph' points to protocol specific header,
 * for example, it points to IP header if it is IP packet.
 */
typedef int (*prot_proc_t)(wlc_wet_info_t *weth, void *sdu, uint8 *eh,
	uint8 *ph, int length, int send);
/* Protocol handlers hash table - hash by ether type */
typedef struct prot_hdlr prot_hdlr_t;
struct prot_hdlr {
	uint16 type;	/* ether type */
	prot_proc_t prot_proc;
	prot_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_PROT_HASH_SIZE	(1 << 3)
#define WET_PROT_HASH(t)	((t)[1]&(WET_PROT_HASH_SIZE-1))
static prot_hdlr_t ept_tbl[] = {
	/* 0 */ {HTON16(ETHER_TYPE_8021Q), wet_vtag_proc, NULL}, /* 0x8100 */
};
static prot_hdlr_t prot_hash[WET_PROT_HASH_SIZE] = {
	/* 0 */ {HTON16(ETHER_TYPE_IP), wet_ip_proc, &ept_tbl[0]}, /* 0x0800 */
	/* 1 */ {0, NULL, NULL},	/* unused   */
	/* 2 */ {0, NULL, NULL},	/* unused   */
	/* 3 */ {0, NULL, NULL},	/* unused   */
	/* 4 */ {0, NULL, NULL},	/* unused   */
	/* 5 */ {0, NULL, NULL},	/* unused   */
	/* 6 */ {HTON16(ETHER_TYPE_ARP), wet_arp_proc, NULL},	/* 0x0806 */
	/* 7 */ {0, NULL, NULL},	/* unused   */
};

/*
 * IPv4 handler. 'ph' points to protocol specific header,
 * for example, it points to UDP header if it is UDP packet.
 */
typedef int (*ipv4_proc_t)(wlc_wet_info_t *weth, uint8 *eh,
	uint8 *iph, uint8 *ph, int length, int send);
/* IPv4 handlers hash table - hash by protocol type */
typedef struct ipv4_hdlr ipv4_hdlr_t;
struct ipv4_hdlr {
	uint8 type;	/* protocol type */
	ipv4_proc_t ipv4_proc;
	ipv4_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_IPV4_HASH_SIZE	(1 << 1)
#define WET_IPV4_HASH(p)	((p)&(WET_IPV4_HASH_SIZE-1))
static ipv4_hdlr_t ipv4_hash[WET_IPV4_HASH_SIZE] = {
	/* 0 */ {0, NULL, NULL},	/* unused   */
	/* 1 */ {IP_PROT_UDP, wet_udp_proc, NULL},	/* 0x11 */
};

/*
 * UDP handler. 'ph' points to protocol specific header,
 * for example, it points to DHCP header if it is DHCP packet.
 */
typedef int (*udp_proc_t)(wlc_wet_info_t *weth, uint8 *eh,
	uint8 *iph, uint8 *udph, uint8 *ph, int length, int send);
/* UDP handlers hash table - hash by port number */
typedef struct udp_hdlr udp_hdlr_t;
struct udp_hdlr {
	uint16 port;	/* udp dest. port */
	udp_proc_t udp_proc;
	udp_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_UDP_HASH_SIZE	(1 << 3)
#define WET_UDP_HASH(p)	((p)[1]&(WET_UDP_HASH_SIZE-1))
static udp_hdlr_t udp_hash[WET_UDP_HASH_SIZE] = {
	/* 0 */ {0, NULL, NULL},	/* unused   */
	/* 1 */ {0, NULL, NULL},	/* unused   */
	/* 2 */ {0, NULL, NULL},	/* unused   */
	/* 3 */ {HTON16(DHCP_PORT_SERVER), wet_dhcpc_proc, NULL}, /* 0x43 */
	/* 4 */ {HTON16(DHCP_PORT_CLIENT), wet_dhcps_proc, NULL}, /* 0x44 */
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
static uint8 ipv4_bcast[IPV4_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff}; /* IP v4 broadcast address */
static uint8 ipv4_null[IPV4_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00}; /* IP v4 NULL address */

/* IOVar table */
enum {
	IOV_WET_HOST_MAC,
	IOV_WET_HOST_IPV4
};

static const bcm_iovar_t wet_iovars[] = {
	{"wet_host_ipv4", IOV_WET_HOST_IPV4,
	(0), IOVT_BUFFER, IPV4_ADDR_LEN
	},
	{"wet_host_mac", IOV_WET_HOST_MAC,
	(0), IOVT_BUFFER, ETHER_ADDR_LEN
	},
	{NULL, 0, 0, 0, 0 }
};

/*
 * Initialize wet private context. It returns a pointer to the
 * wet private context if succeeded. Otherwise it returns NULL.
 */
wlc_wet_info_t *
BCMATTACHFN(wlc_wet_attach)(wlc_info_t *wlc)
{
	wlc_wet_info_t *weth;
	wlc_pub_t *pub = wlc->pub;
	int i;

	/* allocate wet private info struct */
	weth = MALLOCZ(pub->osh, sizeof(wlc_wet_info_t));
	if (!weth) {
		WL_ERROR(("wl%d: wlc_wet_attach: MALLOCZ failed, malloced %d bytes\n",
			pub->unit, MALLOCED(pub->osh)));
		return NULL;
	}

	/* init wet private info struct */
	for (i = 0; i < WET_NUMSTAS - 1; i ++)
		weth->sta[i].next = &weth->sta[i + 1];
	weth->stafree = &weth->sta[0];
	weth->pub = pub;

	/* register module */
	if (wlc_module_register(pub, wet_iovars, "wet", weth, wet_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          pub->unit, __FUNCTION__));
		return NULL;
	}

#ifdef BCMDBG
	wlc_dump_register(pub, "wet", (dump_fn_t)wlc_wet_dump, (void *)weth);
#endif // endif

	return weth;
}

/* Cleanup wet private context */
void
BCMATTACHFN(wlc_wet_detach)(wlc_wet_info_t *weth)
{
	if (!weth)
		return;
	wlc_module_unregister(weth->pub, "wet", weth);
	MFREE(weth->pub->osh, weth, sizeof(wlc_wet_info_t));
}

/* Handling WET related iovars */
static int
wet_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_wet_info_t *weth = hdl;
	int err = 0;

	switch (actionid) {
	case IOV_SVAL(IOV_WET_HOST_IPV4):
		bcopy(a, weth->ip, alen);
		break;

	case IOV_SVAL(IOV_WET_HOST_MAC):
		bcopy(a, &weth->mac, alen);
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
wet_eth_proc(wlc_wet_info_t *weth, void *sdu, uint8 *frame, int length, int send)
{
	uint8 *pt = frame + ETHER_TYPE_OFFSET;
	uint16 type;
	uint8 *ph;
	prot_hdlr_t *phdlr;

	/* intercept Ethernet II frame (type > 1500) */
	if (length >= ETHER_HDR_LEN && (pt[0] > (ETHER_MAX_DATA >> 8) ||
	    (pt[0] == (ETHER_MAX_DATA >> 8) && pt[1] > (ETHER_MAX_DATA & 0xff))))
		;
	/* intercept 802.3 LLC/SNAP frame (type <= 1500) */
	else if (length >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN) {
		uint8 *llc = frame + ETHER_HDR_LEN;
		if (bcmp(llc_snap_hdr, llc, SNAP_HDR_LEN))
			return 0;
		pt = llc + SNAP_HDR_LEN;
	}
	/* frame too short bail out */
	else {
		WL_ERROR(("wl%d: wet_eth_proc: %s short eth frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}
	ph = pt + ETHER_TYPE_LEN;
	length -= ph - frame;

	/* Call protocol specific handler to process frame. */
	type = *(uint16 *)pt;
	for (phdlr = &prot_hash[WET_PROT_HASH(pt)];
	     phdlr != NULL; phdlr = phdlr->next) {
		if (phdlr->type != type || !phdlr->prot_proc)
			continue;
		return (phdlr->prot_proc)(weth, sdu, frame, ph, length, send);
	}

	if (!bcmp(WLCHWADDR(weth), frame + ETHER_SRC_OFFSET, ETHER_ADDR_LEN)) {
		return 0;
	}
	else {
		WL_TRACE(("wl%d: %s: %s unknown type (0x%X), ignored %s\n",
			WLCUNIT(weth), __FUNCTION__, send ? "send" : "recv", type,
			(type == 0xDD86) ? "IPv6":""));
		/* ignore unsupported protocol from different mac addr than us */
		return BCME_UNSUPPORTED;
	}
}

/* process 8021p/Q tagged frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_vtag_proc(wlc_wet_info_t *weth, void *sdu,
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
	type = *(uint16 *)(pt = vtag + VLAN_TAG_LEN);

	for (phdlr = &prot_hash[WET_PROT_HASH(pt)];
	     phdlr != NULL; phdlr = phdlr->next) {
		if (phdlr->type != type || !phdlr->prot_proc)
			continue;
		return (phdlr->prot_proc)(weth, sdu, eh,
			pt + ETHER_TYPE_LEN, length, send);
	}

	return 0;
}

/* process IP frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_ip_proc(wlc_wet_info_t *weth, void *sdu,
	uint8 *eh, uint8 *iph, int length, int send)
{
	uint8 type;
	int ihl;
	wet_sta_t *sta;
	ipv4_hdlr_t *iphdlr;
	uint8 *iaddr;
	struct ether_addr *ea = NULL;
	int ret, ea_off = 0;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/* IPv4 only */
	if (length < 1 || (IP_VER(iph) != IP_VER_4)) {
		WL_ERROR(("wl%d: wet_ip_proc: %s non IPv4 frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}

	ihl = IPV4_HLEN(iph);

	/* minimum length */
	if (length < ihl) {
		WL_ERROR(("wl%d: wet_ip_proc: %s short IPv4 frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}

	/* protocol specific handling */
	type = IPV4_PROT(iph);
	for (iphdlr = &ipv4_hash[WET_IPV4_HASH(type)];
	     iphdlr; iphdlr = iphdlr->next) {
		if (iphdlr->type != type || !iphdlr->ipv4_proc)
			continue;
		if ((ret = (iphdlr->ipv4_proc)(weth, eh,
			iph, iph + ihl, length - ihl, send)))
			return ret;
	}

	/* generic IP packet handling */
	/*
	 * Replace source MAC in Ethernet header with wireless's and
	 * keep track of IP MAC mapping when sending frame.
	 */
	if (send) {
		uint32 iaddr_dest, iaddr_src;
		bool wet_table_upd = TRUE;

		iaddr = iph + IPV4_SRC_IP_OFFSET;
		iaddr_dest = ntoh32(*((uint32 *)(iph + IPV4_DEST_IP_OFFSET)));
		iaddr_src = ntoh32(*(uint32 *)(iaddr));

		/* Do not process and update knowledge base on receipt of a local IP
		* multicast frame
		*/
		if (IP_ISMULTI(iaddr_dest) && !iaddr_src) {
			WL_INFORM(("wl%d:recv multicast frame from %s.Don't update hash table\n",
					WLCUNIT(weth), bcm_ether_ntoa((struct ether_addr*)
					(eh + ETHER_SRC_OFFSET), eabuf)));
			wet_table_upd = FALSE;
		}

		if (wet_table_upd && wet_sta_update_all(weth, iaddr,
		(struct ether_addr*)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
			WL_ERROR(("wl%d: wet_ip_proc: unable to update STA %u.%u.%u.%u %s\n",
				WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2],
				iaddr[3], bcm_ether_ntoa((struct ether_addr*)(eh +
				ETHER_SRC_OFFSET), eabuf)));
			return -1;
		}
		ea = (struct ether_addr *)WLCHWADDR(weth);
		ea_off = ETHER_SRC_OFFSET;
		eacopy(ea, eh + ea_off);
	}
	/*
	 * Replace dest MAC in Ethernet header using the found one
	 * when receiving frame.
	 */
	/* no action for received bcast/mcast ethernet frame */
	else if (!ETHER_ISMULTI(eh)) {
		iaddr = iph + IPV4_DEST_IP_OFFSET;
		if (wet_sta_find_ip(weth, iaddr, &sta) < 0) {
			WL_ERROR(("wl%d: wet_ip_proc: unable to find STA %u.%u.%u.%u\n",
				WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2],
				iaddr[3]));
			return -1;
		}
		ea = &sta->mac;
		ea_off = ETHER_DEST_OFFSET;
		eacopy(ea, eh + ea_off);
	}

#ifdef PKTC
	if (PKTISCHAINED(sdu)) {
		void *t;
		for (t = PKTCLINK(sdu); t != NULL; t = PKTCLINK(t)) {
			eh = PKTDATA(WLCOSH(weth), t);
			eacopy(ea, eh + ea_off);
		}
	}
#endif // endif

	return 0;
}

/* process ARP frame - ARP proxy */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_arp_proc(wlc_wet_info_t *weth, void *sdu,
	uint8 *eh, uint8 *arph, int length, int send)
{
	wet_sta_t *sta;
	uint8 *iaddr;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/*
	 * FIXME: validate ARP header:
	 *  h/w Ethernet 2, proto IP x800, h/w addr size 6, proto addr size 4.
	 */

	/*
	 * Replace source MAC in Ethernet header as well as source MAC in
	 * ARP protocol header when processing frame sent.
	 */
	if (send) {
		iaddr = arph + ARP_SRC_IP_OFFSET;
		if (wet_sta_update_all(weth, iaddr,
		(struct ether_addr*)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
			WL_ERROR(("wl%d: wet_arp_proc: unable to update STA %u.%u.%u.%u %s\n",
				WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2], iaddr[3],
				bcm_ether_ntoa(
					(struct ether_addr*)(eh + ETHER_SRC_OFFSET), eabuf)));
			return -1;
		}
		bcopy(WLCHWADDR(weth), eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
		bcopy(WLCHWADDR(weth), arph+ARP_SRC_ETH_OFFSET, ETHER_ADDR_LEN);
	}
	/*
	 * Replace dest MAC in Ethernet header as well as dest MAC in
	 * ARP protocol header when processing frame recv'd. Process ARP
	 * replies and Unicast ARP requests
	 */
	else if ((*(uint16 *)(arph + ARP_OPC_OFFSET) == HTON16(ARP_OPC_REPLY)) ||
		((*(uint16 *)(arph + ARP_OPC_OFFSET) == HTON16(ARP_OPC_REQUEST)) &&
		(!ETHER_ISMULTI(eh)))) {
		iaddr = arph + ARP_TGT_IP_OFFSET;
		if (wet_sta_find_ip(weth, iaddr, &sta) < 0) {
			WL_ERROR(("wl%d: wet_arp_proc: unable to find STA %u.%u.%u.%u\n",
				WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2], iaddr[3]));
			return -1;
		}
		bcopy(&sta->mac, arph + ARP_TGT_ETH_OFFSET, ETHER_ADDR_LEN);
		bcopy(&sta->mac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	}

	return 0;
}

/* process UDP frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_udp_proc(wlc_wet_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, int length, int send)
{
	udp_hdlr_t *udphdlr;
	uint16 port;

	/* check frame length, at least UDP_HDR_LEN */
	if ((length -= UDP_HDR_LEN) < 0) {
		WL_ERROR(("wl%d: wet_udp_proc: %s short UDP frame, ignored\n",
			WLCUNIT(weth), send ? "send" : "recv"));
		return -1;
	}

	/*
	 * Unfortunately we must spend some time here to deal with
	 * some higher layer protocol special processings.
	 * See individual handlers for protocol specific details.
	 */
	port = *(uint16 *)(udph + UDP_DEST_PORT_OFFSET);
	for (udphdlr = &udp_hash[WET_UDP_HASH((uint8 *)&port)];
	     udphdlr; udphdlr = udphdlr->next) {
		if (udphdlr->port != port || !udphdlr->udp_proc)
			continue;
		return (udphdlr->udp_proc)(weth, eh, iph, udph,
			udph + UDP_HDR_LEN, length, send);
	}

	return 0;
}

/*
* DHCP is a 'complex' protocol for WET, mainly because it
* uses its protocol body to convey IP/MAC info. It is impossible
* to forward frames correctly back and forth without looking
* into the DHCP's body and interpreting it. See RFC2131 sect.
* 4.1 'Constructing and sending DHCP messages' for details
* of using/parsing various fields in the body.
*
* DHCP pass through:
*
*       Must alter DHCP flag to broadcast so that the server
*       can reply with the broadcast address before we can
*	provide DHCP relay functionality. Otherwise the DHCP
*       server will send DHCP replies using the DHCP client's
*       MAC address. Such replies will not be delivered simply
*       because:
*
*         1. The AP's bridge will not forward the replies back to
*         this device through the wireless link because it does not
*         know such node exists on this link. The bridge's forwarding
*         table on the AP will have this device's MAC address only.
*         It does not know anything else behind this device.
*
*         2. The AP's wireless driver won't allow such frames out
*         either even if they made their way out the AP's bridge
*         through the bridge's DLF broadcasting because there is
*         no such STA associated with the AP.
*
*         3. This device's MAC won't allow such frames pass
*         through in non-promiscuous mode even when they made
*         their way out of the AP's wireless interface somehow.
*
* DHCP relay:
*
*       Once the WET is configured with the host MAC address it can
*       relay the host request as if it were sent from WET itself.
*
*       Once the WET is configured with the host IP address it can
*       pretend to be the host and act as a relay agent.
*/
/* process DHCP client frame (client to server, or server to relay agent) */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_dhcpc_proc(wlc_wet_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_sta_t *sta;
	uint16 flags;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	uint16 port;
	uint8 *ipv4;
	const struct ether_addr *ether;

	/*
	 * FIXME: validate DHCP body:
	 *  htype Ethernet 1, hlen Ethernet 6, frame length at least 242.
	 */

	/* only interested in requests when sending to server */
	if (send && *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REQUEST)
		return 0;
	/* only interested in replies when receiving from server as a relay agent */
	if (!send && *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return 0;

	/* send request */
	if (send) {
		/* find existing or alloc new IP/MAC mapping entry */
		if (wet_sta_update_mac(weth,
		(struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), &sta) < 0) {
			WL_ERROR(("wl%d: wet_dhcpc_proc: unable to update STA %s\n",
				WLCUNIT(weth),
				bcm_ether_ntoa(
					(struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
			return -1;
		}
		bcopy(dhcp + DHCP_FLAGS_OFFSET, &flags, DHCP_FLAGS_LEN);
		/* We can always relay the host's request when we know its MAC addr. */
		if (!ETHER_ISNULLADDR(weth->mac.octet) &&
		    !bcmp(dhcp + DHCP_CHADDR_OFFSET, &weth->mac, ETHER_ADDR_LEN)) {
			/* replace chaddr with host's MAC */
			csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
				dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN,
				WLCHWADDR(weth), ETHER_ADDR_LEN);
			bcopy(WLCHWADDR(weth), dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN);
			/* force reply to be unicast */
			flags &= ~HTON16(DHCP_FLAG_BCAST);
		}
		/* We can relay other clients' requests when we know the host's IP addr. */
		else if (!IPV4_ADDR_NULL(weth->ip)) {
			/* we can only handle the first hop otherwise drop it */
			if (!IPV4_ADDR_NULL(dhcp + DHCP_GIADDR_OFFSET)) {
				WL_ERROR(("wl%d: wet_dhcpc_proc: not first hop, ignored\n",
					WLCUNIT(weth)));
				return -1;
			}
			/* replace giaddr with host's IP */
			csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
				dhcp + DHCP_GIADDR_OFFSET, IPV4_ADDR_LEN,
				weth->ip, IPV4_ADDR_LEN);
			bcopy(weth->ip, dhcp + DHCP_GIADDR_OFFSET, IPV4_ADDR_LEN);
			/* force reply to be unicast */
			flags &= ~HTON16(DHCP_FLAG_BCAST);
		}
		/*
		 * Request comes in when we don't know the host's MAC and/or IP
		 * addresses hence we can't relay the request. We must notify the
		 * server of our addressing limitation by turning on the broadcast
		 * bit at this point as what the function comments point out.
		 */
		else
			flags |= HTON16(DHCP_FLAG_BCAST);
		/* update flags */
		bcopy(dhcp + DHCP_FLAGS_OFFSET, sta->flags, DHCP_FLAGS_LEN);
		if (flags != *(uint16 *)sta->flags) {
			csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
				dhcp + DHCP_FLAGS_OFFSET, DHCP_FLAGS_LEN,
				(uint8 *)&flags, DHCP_FLAGS_LEN);
			bcopy((uint8 *)&flags, dhcp + DHCP_FLAGS_OFFSET,
				DHCP_FLAGS_LEN);
		}
		/* replace the Ethernet source MAC with ours */
		bcopy(WLCHWADDR(weth), eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
	}
	/* relay recv'd reply to its destiny */
	else if (!IPV4_ADDR_NULL(weth->ip) &&
	    !bcmp(dhcp + DHCP_GIADDR_OFFSET, weth->ip, IPV4_ADDR_LEN)) {
		/* find IP/MAC mapping entry */
		if (wet_sta_find_mac(weth,
		(struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), &sta) < 0) {
			WL_ERROR(("wl%d: wet_dhcpc_proc: unable to find STA %s\n",
				WLCUNIT(weth),
				bcm_ether_ntoa(
					(struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
			return -1;
		}
		/*
		 * XXX the following code works for the first hop only
		 */
		/* restore the DHCP giaddr with its original */
		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
			dhcp + DHCP_GIADDR_OFFSET, IPV4_ADDR_LEN,
			ipv4_null, IPV4_ADDR_LEN);
		bcopy(ipv4_null, dhcp + DHCP_GIADDR_OFFSET, IPV4_ADDR_LEN);
		/* restore the original client's dhcp flags */
		if (bcmp(dhcp + DHCP_FLAGS_OFFSET, sta->flags, DHCP_FLAGS_LEN)) {
			csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
				dhcp + DHCP_FLAGS_OFFSET, DHCP_FLAGS_LEN,
				sta->flags, DHCP_FLAGS_LEN);
			bcopy(sta->flags, dhcp + DHCP_FLAGS_OFFSET, DHCP_FLAGS_LEN);
		}
		/* replace the dest UDP port with DHCP client port */
		port = HTON16(DHCP_PORT_CLIENT);
		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
			udph + UDP_DEST_PORT_OFFSET, UDP_PORT_LEN,
			(uint8 *)&port, UDP_PORT_LEN);
		bcopy((uint8 *)&port, udph + UDP_DEST_PORT_OFFSET, UDP_PORT_LEN);
		/* replace the dest MAC & IP addr with the client's */
		if (*(uint16 *)sta->flags & HTON16(DHCP_FLAG_BCAST)) {
			ipv4 = ipv4_bcast;
			ether = &ether_bcast;
		}
		else {
			ipv4 = dhcp + DHCP_YIADDR_OFFSET;
			ether = &sta->mac;
		}
		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
			iph + IPV4_DEST_IP_OFFSET, IPV4_ADDR_LEN,
			ipv4, IPV4_ADDR_LEN);
		csum_fixup_16(iph + IPV4_CHKSUM_OFFSET,
			iph + IPV4_DEST_IP_OFFSET, IPV4_ADDR_LEN,
			ipv4, IPV4_ADDR_LEN);
		bcopy(ipv4, iph + IPV4_DEST_IP_OFFSET, IPV4_ADDR_LEN);
		bcopy(ether, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	}
	/* it should not recv non-relay reply at all, but just in case */
	else {
		WL_ERROR(("wl%d: wet_dhcpc_proc: ignore recv'd frame from %s\n",
			WLCUNIT(weth),
			bcm_ether_ntoa((struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
		return -1;
	}

	/* no further processing! */
	return 1;
}

/* process DHCP server frame (server to client) */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_dhcps_proc(wlc_wet_info_t *weth,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_sta_t *sta;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/*
	 * FIXME: validate DHCP body:
	 *  htype Ethernet 1, hlen Ethernet 6, frame length at least 242.
	 */

	/* only interested in replies when receiving from server */
	if (send || *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return 0;

	/* find IP/MAC mapping entry */
	if (wet_sta_find_mac(weth, (struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), &sta) < 0) {
		WL_ERROR(("wl%d: wet_dhcps_proc: unable to find STA %s\n",
			WLCUNIT(weth),
			bcm_ether_ntoa((struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
		return -1;
	}
	/* relay the reply to the host when we know the host's MAC addr */
	if (!ETHER_ISNULLADDR(weth->mac.octet) &&
	    !bcmp(dhcp + DHCP_CHADDR_OFFSET, WLCHWADDR(weth), ETHER_ADDR_LEN)) {
		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
			dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN,
			weth->mac.octet, ETHER_ADDR_LEN);
		bcopy(&weth->mac, dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN);
	}
	/* restore the original client's dhcp flags if necessary */
	if (bcmp(dhcp + DHCP_FLAGS_OFFSET, sta->flags, DHCP_FLAGS_LEN)) {
		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
			dhcp + DHCP_FLAGS_OFFSET, DHCP_FLAGS_LEN,
			sta->flags, DHCP_FLAGS_LEN);
		bcopy(sta->flags, dhcp + DHCP_FLAGS_OFFSET, DHCP_FLAGS_LEN);
	}
	/* replace the dest MAC with that of client's */
	if (*(uint16 *)sta->flags & HTON16(DHCP_FLAG_BCAST))
		bcopy((const uint8 *)&ether_bcast, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	else
		bcopy(&sta->mac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);

	/* no further processing! */
	return 1;
}

/* alloc IP/MAC mapping entry */
/* Returns 0 if succeeded; < 0 otherwise. */
static int
wet_sta_alloc(wlc_wet_info_t *weth, wet_sta_t **saddr)
{
	wet_sta_t *sta;

	/* allocate a new one */
	if (!weth->stafree) {
		WL_ERROR(("wl%d: wet_sta_alloc: no room for another STA\n", WLCUNIT(weth)));
		return -1;
	}
	sta = weth->stafree;
	weth->stafree = sta->next;

	/* init them just in case */
	sta->next = NULL;
	sta->next_ip = NULL;
	sta->next_mac = NULL;

	*saddr = sta;
	return 0;
}

/* update IP/MAC mapping entry and hash */
/* Returns 0 if succeeded; < 0 otherwise. */
static int
wet_sta_update_all(wlc_wet_info_t *weth, uint8 *iaddr, struct ether_addr *eaddr,
	wet_sta_t **saddr)
{
	wet_sta_t *sta;
	int i;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	/* find the existing one and remove it from the old IP hash link */
	if (!wet_sta_find_mac(weth, eaddr, &sta)) {
		i = WET_STA_HASH_IP(sta->ip);
		if (bcmp(sta->ip, iaddr, IPV4_ADDR_LEN)) {
			wet_sta_t *sta2, **next;
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
			i = WET_STA_HASH_UNK;
		}
	}
	/* allocate a new one and hash it by MAC */
	else if (!wet_sta_alloc(weth, &sta)) {
		i = WET_STA_HASH_MAC(eaddr->octet);
		bcopy(eaddr, &sta->mac, ETHER_ADDR_LEN);
		sta->next_mac = weth->stahash_mac[i];
		weth->stahash_mac[i] = sta;
		i = WET_STA_HASH_UNK;
	}
	/* bail out if we can't find nor create any */
	else {
		WL_ERROR(("wl%d: wet_sta_update_all: unable to alloc STA %u.%u.%u.%u %s\n",
			WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2], iaddr[3],
			bcm_ether_ntoa(eaddr, eabuf)));
		return -1;
	}

	/* update IP and hash by new IP */
	if (i == WET_STA_HASH_UNK) {
		i = WET_STA_HASH_IP(iaddr);
		bcopy(iaddr, sta->ip, IPV4_ADDR_LEN);
		sta->next_ip = weth->stahash_ip[i];
		weth->stahash_ip[i] = sta;

		/* start here and look for other entries with same IP address */
		{
			wet_sta_t *sta2, *prev;
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
					wet_sta_remove_mac_entry(weth, &sta2->mac);
					/* entry should be completely out of the table now,
					   add it to the free list
					*/
					memset(sta2, 0, sizeof(wet_sta_t));
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

/* update IP/MAC mapping entry and hash */
static int
wet_sta_update_mac(wlc_wet_info_t *weth, struct ether_addr *eaddr, wet_sta_t **saddr)
{
	wet_sta_t *sta;
	int i;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/* find the existing one */
	if (!wet_sta_find_mac(weth, eaddr, &sta))
		;
	/* allocate a new one and hash it */
	else if (!wet_sta_alloc(weth, &sta)) {
		i = WET_STA_HASH_MAC(eaddr->octet);
		bcopy(eaddr, &sta->mac, ETHER_ADDR_LEN);
		sta->next_mac = weth->stahash_mac[i];
		weth->stahash_mac[i] = sta;
	}
	/* bail out if we can't find nor create any */
	else {
		WL_ERROR(("wl%d: wet_sta_update_mac: unable to alloc STA %s\n",
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
wet_sta_remove_mac_entry(wlc_wet_info_t *weth, struct ether_addr *eaddr)
{
	wet_sta_t *sta, *prev;
	int i = WET_STA_HASH_MAC(eaddr->octet);
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
		WL_ERROR(("wl%d: wet_sta_remove_mac_entry: unable to find STA %s entry\n",
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
wet_sta_find_ip(wlc_wet_info_t *weth, uint8 *iaddr, wet_sta_t **saddr)
{
	int i = WET_STA_HASH_IP(iaddr);
	wet_sta_t *sta;

	/* find the existing one by IP */
	for (sta = weth->stahash_ip[i]; sta; sta = sta->next_ip) {
		if (bcmp(sta->ip, iaddr, IPV4_ADDR_LEN))
			continue;
		*saddr = sta;
		return 0;
	}

	/* sta has not been learned */
	WL_ERROR(("wl%d: wet_sta_find_ip: unable to find STA %u.%u.%u.%u\n",
		WLCUNIT(weth), iaddr[0], iaddr[1], iaddr[2], iaddr[3]));
	return -1;
}

/* find IP/MAC mapping entry by MAC address */
/* Returns 0 if succeeded; < 0 otherwise. */
static int
wet_sta_find_mac(wlc_wet_info_t *weth, struct ether_addr *eaddr, wet_sta_t **saddr)
{
	int i = WET_STA_HASH_MAC(eaddr->octet);
	wet_sta_t *sta;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	/* find the existing one by MAC */
	for (sta = weth->stahash_mac[i]; sta; sta = sta->next_mac) {
		if (bcmp(&sta->mac, eaddr, ETHER_ADDR_LEN))
			continue;
		*saddr = sta;
		return 0;
	}

	/* sta has not been learnt */
	WL_ERROR(("wl%d: wet_sta_find_mac: unable to find STA %s\n",
		WLCUNIT(weth), bcm_ether_ntoa(eaddr, eabuf)));
	return -1;
}

/*
* Adjust 16 bit checksum - taken from RFC 3022.
*
*   The algorithm below is applicable only for even offsets (i.e., optr
*   below must be at an even offset from start of header) and even lengths
*   (i.e., olen and nlen below must be even).
*/
static void
csum_fixup_16(uint8 *chksum, uint8 *optr, int olen, uint8 *nptr, int nlen)
{
	long x, old, new;

	ASSERT(!((long int)optr&1) && !(olen&1));
	ASSERT(!((long int)nptr&1) && !(nlen&1));

	x = (chksum[0]<< 8)+chksum[1];
	if (!x)
		return;
	x = ~x & 0xFFFF;
	while (olen)
	{
		old = (optr[0]<< 8)+optr[1]; optr += 2;
		x -= old & 0xffff;
		if (x <= 0) { x--; x &= 0xffff; }
		olen -= 2;
	}
	while (nlen)
	{
		new = (nptr[0]<< 8)+nptr[1]; nptr += 2;
		x += new & 0xffff;
		if (x & 0x10000) { x++; x &= 0xffff; }
		nlen -= 2;
	}
	x = ~x & 0xFFFF;
	chksum[0] = (uint8)(x >> 8); chksum[1] = (uint8)x;
}

/*
* Process frames in transmit direction by replacing source MAC with
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
wlc_wet_send_proc(wlc_wet_info_t *weth, void *sdu, void **new)
{
	uint8 *frame = PKTDATA(WLCOSH(weth), sdu);
	int length = PKTLEN(WLCOSH(weth), sdu);
	void *pkt = sdu;
	wlc_info_t *wlc = (wlc_info_t *)weth->pub->wlc;

	/*
	 * FIXME: need to tell if buffer is shared and only
	 * do copy on shared buffer.
	 */
	/*
	 * copy broadcast/multicast frame to our own packet
	 * otherwise we will screw up others because we alter
	 * the frame content.
	 */
	if (length < ETHER_HDR_LEN) {
		WL_ERROR(("wl%d: wlc_wet_send_proc: unable to process short frame\n",
			WLCUNIT(weth)));
		return -1;
	}
	if (ETHER_ISMULTI(frame)) {
		length = pkttotlen(WLCOSH(weth), sdu);
		if (!(pkt = PKTGET(WLCOSH(weth), TXOFF + length, TRUE))) {
			WL_ERROR(("wl%d: wlc_wet_send_proc: unable to alloc, dropped\n",
				WLCUNIT(weth)));
			WLCNTINCR(weth->pub->_cnt->txnobuf);
			return -1;
		}
		PKTPULL(WLCOSH(weth), pkt, TXOFF);
		frame = PKTDATA(WLCOSH(weth), pkt);
		pktcopy(WLCOSH(weth), sdu, 0, length, frame);
		/* Move pkttag info from sdu to pkt */

		wlc_pkttag_info_move(wlc, sdu, pkt);
		/* Transfer priority */
		PKTSETPRIO(pkt, PKTPRIO(sdu));

		PKTFREE(WLCOSH(weth), sdu, TRUE);
		PKTSETLEN(WLCOSH(weth), pkt, length);
	}
	*new = pkt;

	/* process frame */
	return wet_eth_proc(weth, sdu, frame, length, 1) < 0 ? -1 : 0;
}

/*
* Process frames in receive direction by replacing destination MAC with
* the one found in IP MAC address mapping table.
* Return:
*	= 0 if frame is done ok;
*	< 0 if unable to handle the frame;
*/
int
wlc_wet_recv_proc(wlc_wet_info_t *weth, void *sdu)
{
	/* process frame */
	return wet_eth_proc(weth, sdu, PKTDATA(WLCOSH(weth), sdu),
		PKTLEN(WLCOSH(weth), sdu), 0) < 0 ? -1 : 0;
}

#ifdef BCMDBG
#define WET_STA_TEST	0 /* Control for the debug code for STA_TEST */
#if WET_STA_TEST
void
wet_sta_dump(wlc_wet_info_t *weth, struct bcmstrbuf *b)
{
	wet_sta_t *sta;
	int i;

	/* dump MAC hash */
	for (i = 0; i < WET_STA_HASH_SIZE; i ++) {
		for (sta = weth->stahash_mac[i]; sta; sta = sta->next_mac) {
			bcm_bprintf(b, "sta %x mh %d next_mac %x\n",
				(uint)sta, i, (uint)sta->next_mac);
		}
	}

	/* dump IP hash */
	for (i = 0; i < WET_STA_HASH_SIZE; i ++) {
		for (sta = weth->stahash_ip[i]; sta; sta = sta->next_ip) {
			bcm_bprintf(b, "sta %x ih %d next_ip %x\n",
				(uint)sta, i, (uint)sta->next_ip);
		}
	}
}

void
wet_sta_test(wlc_wet_info_t *weth, struct bcmstrbuf *b)
{
	char mac1[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5};
	char mac2[] = {0x0, 0x2, 0x2, 0x3, 0x4, 0x5};
	char mac3[] = {0x0, 0x3, 0x2, 0x3, 0x4, 0x5};
	char ip1[] = {0x0, 0x1, 0x2, 0x1};
	char ip2[] = {0x0, 0x2, 0x2, 0x1};
	char ip3[] = {0x0, 0x3, 0x2, 0x1};
	char IP1[] = {0x0, 0x1, 0x2, 0xb};
	char IP2[] = {0x0, 0x2, 0x2, 0xb};
	char IP3[] = {0x0, 0x3, 0x2, 0xb};
	wet_sta_t *sta;

	wet_sta_update_all(weth, ip1, mac1, &sta);
	wet_sta_dump(weth, b);
	wet_sta_update_all(weth, ip2, mac2, &sta);
	wet_sta_dump(weth, b);
	wet_sta_update_all(weth, ip3, mac3, &sta);
	wet_sta_dump(weth, b);
	wet_sta_update_all(weth, IP2, mac2, &sta);
	wet_sta_dump(weth, b);
	wet_sta_update_all(weth, IP1, mac1, &sta);
	wet_sta_dump(weth, b);
	wet_sta_update_all(weth, IP3, mac3, &sta);
	wet_sta_dump(weth, b);

}
#endif	/* #if WET_STA_TEST */

int
wlc_wet_dump(wlc_wet_info_t *weth, struct bcmstrbuf *b)
{
	char eabuf[ETHER_ADDR_STR_LEN];
	wet_sta_t *sta;
	int i;

	bcm_bprintf(b, "Host MAC: %s\n", bcm_ether_ntoa(&weth->mac, eabuf));
	bcm_bprintf(b, "Host IP: %u.%u.%u.%u\n",
		weth->ip[0], weth->ip[1], weth->ip[2], weth->ip[3]);
	bcm_bprintf(b, "Entry\tEnetAddr\t\tInetAddr\n");
	for (i = 0; i < WET_NUMSTAS; i ++) {
		if (weth->sta[i].next)
			continue;
		/* format the entry dump */
		sta = &weth->sta[i];
		bcm_bprintf(b, "%u\t%s\t%u.%u.%u.%u\n",
			i, bcm_ether_ntoa(&sta->mac, eabuf),
			sta->ip[0], sta->ip[1], sta->ip[2], sta->ip[3]);
	}
#if WET_STA_TEST
	wet_sta_test(weth, b);
#endif /* WET_STA_TEST */

	return 0;
}
#endif	/* #ifdef BCMDBG */
