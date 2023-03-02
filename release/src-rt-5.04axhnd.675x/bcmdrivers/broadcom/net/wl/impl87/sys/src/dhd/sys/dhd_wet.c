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
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_wet.c 806021 2021-12-10 18:07:13Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WirelessEthernet]
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <ethernet.h>
#include <vlan.h>
#include <802.3.h>
#include <bcmip.h>
#include <bcmipv6.h>
#include <bcmarp.h>
#include <bcmudp.h>
#include <bcmdhcp.h>
#include <bcmicmp.h>
#include <bcmendian.h>
#include <dhd_dbg.h>

#include <dhd_wet.h>
#include <dhd_linux.h>

#ifdef DHD_DPSTA
#include <dpsta.h>
#endif /* DHD_DPSTA */

#define BSSCFG_IDX_TO_WET_NWKID(idx)	((uint8)(idx))

/* IP/MAC address mapping entry
 * One MAC can map to IP and IPv6
 */

enum {
	IPVER_4 = 0, /* must start with 0 as array index */
	IPVER_6,
	IPTYPE  /* indicates how many IPVERs */
};

typedef struct wet_sta wet_sta_t;
struct wet_sta {
	/* client */
	uint8 ip[IPTYPE][IPV6_ADDR_LEN];	/* client IP addr */
	struct ether_addr mac;	/* client MAC addr */
	uint8 flags[DHCP_FLAGS_LEN];	/* orig. dhcp flags */
	/* internal */
	wet_sta_t *next;	/* free STA link */
	wet_sta_t *next_ip[IPTYPE];	/* hash link by IP */
	wet_sta_t *next_mac;	/* hash link by MAC */
	uint8 bss;
};

const uint8 iphashoff[IPTYPE] = {
	3 /* IPV4_ADDR_LEN-1 */,
	15 /* IPV6_ADDR_LEN -1 */
};

const uint8 IP_ADDR_LEN[IPTYPE] = {IPV4_ADDR_LEN, IPV6_ADDR_LEN};

#define WET_NUMSTAS		(1 << 8)	/* max. # clients, must be multiple of 2 */
#define WET_STA_HASH_SIZE	(WET_NUMSTAS/2)	/* must be <= WET_NUMSTAS */
#define WET_STA_HASH_IP(ip, ipver)	\
			((ip)[iphashoff[ipver]]&(WET_STA_HASH_SIZE-1))	/* hash by IP */
#define WET_STA_HASH_MAC(ea)	\
			(((ea)[3]^(ea)[4]^(ea)[5])&(WET_STA_HASH_SIZE-1)) /* hash by MAC */
#define WET_STA_HASH_UNK	-1 /* Unknown hash */
#define WET_STA_HASH_OK	    0  /* hash exists */

#define IP_ISMULTI(ip)           (((ip) & 0xf0000000) == 0xe0000000) /* Check for multicast by IP */

/* WET private info structure */
struct dhd_wet_info {
	/* pointer to dhdpublic info struct */
	dhd_pub_t *pub;
	/* Host addresses */
	uint8 ip[DHD_MAX_URE_STA][IPV4_ADDR_LEN];
	struct ether_addr mac[DHD_MAX_URE_STA];
	/* STA storage, one entry per eth. client */
	wet_sta_t sta[WET_NUMSTAS];
	/* Free sta list */
	wet_sta_t *stafree;
	/* Used sta hash by IP */
	wet_sta_t *stahash_ip[IPTYPE][WET_STA_HASH_SIZE];
	/* Used sta hash by MAC */
	wet_sta_t *stahash_mac[WET_STA_HASH_SIZE];
};

/* forward declarations */
static int wet_eth_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
	uint8 *frame, int length, int send);
static int wet_vtag_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
	uint8 * eh, uint8 *vtag, int length, int send);
static int wet_ip_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
	uint8 * eh, uint8 *iph, int length, int send);
static int wet_ipv6_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
	uint8 * eh, uint8 *iph, int length, int send);
static int wet_arp_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
	uint8 *eh, uint8 *arph, int length, int send);
static int wet_udp_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, int length, int send);
static int wet_dhcpc_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);
static int wet_dhcps_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);

static int wet_dhcp6c_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);
static int wet_dhcp6s_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send);
static int wet_icmpv6_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int length, int send);
static int wet_na_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int length, int send);
static int wet_ns_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int length, int send);
static int wet_ra_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int length, int send);
static int wet_rs_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int length, int send);
static int wet_rm_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int length, int send);

static int wet_sta_alloc(dhd_wet_info_t *weth, int ifidx, wet_sta_t **saddr);
static int wet_sta_update_all(dhd_wet_info_t *weth, int ifidx, uint8 ipver,
	uint8 *iaddr, struct ether_addr *eaddr, wet_sta_t **saddr);
static int wet_sta_update_mac(dhd_wet_info_t *weth, int ifidx,
	struct ether_addr *eaddr, wet_sta_t **saddr);
static int wet_sta_remove_mac_entry(dhd_wet_info_t *weth, struct ether_addr *eaddr);
static int wet_sta_find_ip(dhd_wet_info_t *weth, uint8 ipver,
	uint8 *iaddr, wet_sta_t **saddr);
static int wet_sta_find_mac(dhd_wet_info_t *weth,
	struct ether_addr *eaddr, wet_sta_t **saddr);
static void csum_fixup_16(uint8 *chksum,
	uint8 *optr, int olen, uint8 *nptr, int nlen);

/*
 * Protocol handler. 'ph' points to protocol specific header,
 * for example, it points to IP header if it is IP packet.
 */
typedef int (*prot_proc_t)(dhd_wet_info_t *weth, int ifidx, void *sdu, uint8 *eh,
	uint8 *ph, int length, int send);
/* Protocol handlers hash table - hash by ether type */
typedef struct prot_hdlr prot_hdlr_t;
struct prot_hdlr {
	uint16 type;		/* ether type */
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
	/* 5 */ {HTON16(ETHER_TYPE_IPV6), wet_ipv6_proc, NULL},	/* 0x86dd */
	/* 6 */ {HTON16(ETHER_TYPE_ARP), wet_arp_proc, NULL},	/* 0x0806 */
	/* 7 */ {0, NULL, NULL},	/* unused   */
};

/*
 * IPv4 handler. 'ph' points to protocol specific header,
 * for example, it points to UDP header if it is UDP packet.
 */
typedef int (*ipv4_proc_t)(dhd_wet_info_t *weth, int ifidx, uint8 *eh,
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
typedef int (*udp_proc_t)(dhd_wet_info_t *weth, int ifidx, uint8 *eh,
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

static udp_hdlr_t dv6_srvpt[] = {
	/* 0 */ {HTON16(DHCP6_PORT_SERVER), wet_dhcp6c_proc, NULL}, /* 0x0223 */
};

static udp_hdlr_t udp_hash[WET_UDP_HASH_SIZE] = {
	/* 0 */ {0, NULL, NULL},	/* unused   */
	/* 1 */ {0, NULL, NULL},	/* unused   */
	/* 2 */ {HTON16(DHCP6_PORT_CLIENT), wet_dhcp6s_proc, NULL}, /* 0x0222 */
	/* 3 */ {HTON16(DHCP_PORT_SERVER), wet_dhcpc_proc, &dv6_srvpt[0]}, /* 0x43 */
	/* 4 */ {HTON16(DHCP_PORT_CLIENT), wet_dhcps_proc, NULL}, /* 0x44 */
	/* 5 */ {0, NULL, NULL},	/* unused   */
	/* 6 */ {0, NULL, NULL},	/* unused   */
	/* 7 */ {0, NULL, NULL},	/* unused   */
};

/*
 * IPv6 handler. 'ph' points to protocol specific header,
 * for example, it points to ICMPv6 header if it is ICMP packet.
 */
typedef int (*ipv6_proc_t)(dhd_wet_info_t *weth, int ifidx, uint8 *eh,
	uint8 *iph, uint8 *ph, int length, int send);
/* IPv4 handlers hash table - hash by protocol type */
typedef struct ipv6_hdlr ipv6_hdlr_t;
struct ipv6_hdlr {
	uint8 type;	/* protocol type */
	ipv6_proc_t ipv6_proc;
	ipv6_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_IPV6_HASH_SIZE	(1 << 1)
#define WET_IPV6_HASH(p)	((p)&(WET_IPV6_HASH_SIZE-1))
static ipv6_hdlr_t ipv6_hash[WET_IPV6_HASH_SIZE] = {
	/* 0 */ {IP_PROT_ICMP6, wet_icmpv6_proc, NULL},	/* 0x3a   */
	/* 1 */ {IP_PROT_UDP, wet_udp_proc, NULL},	/* 0x11 */
};

/*
 * ICMPv6 handler. 'ph' points to type specific header,
 * for example, it points to Neighbor Solicitation header if it is Neighbor Discovery packet.
 */
typedef int (*icmpv6_proc_t)(dhd_wet_info_t *weth, int ifidx, uint8 *eh,
	uint8 *iph, uint8 *ph, int length, int send);
/* UDP handlers hash table - hash by port number */
typedef struct icmpv6_hdlr icmpv6_hdlr_t;
struct icmpv6_hdlr {
	uint8 type;	/* icmpv6 type */
	icmpv6_proc_t icmpv6_proc;
	icmpv6_hdlr_t *next;	/* next proto handler that has the same hash */
};
#define WET_ICMPV6_HASH_SIZE	(1 << 3)
#define WET_ICMPV6_HASH(p)	((p)&(WET_ICMPV6_HASH_SIZE-1))
static icmpv6_hdlr_t icmpv6_hash[WET_ICMPV6_HASH_SIZE] = {
	/* 0 */ {ICMP6_NEIGH_ADVERTISEMENT, wet_na_proc, NULL},	/* 0x88 */
	/* 1 */ {ICMP6_REDIRECT, wet_rm_proc, NULL},	/* 0x89 */
	/* 2 */ {0, NULL, NULL},	/* unused */
	/* 3 */ {0, NULL, NULL},	/* unused */
	/* 4 */ {0, NULL, NULL},	/* unused */
	/* 5 */ {ICMP6_RTR_SOLICITATION, wet_rs_proc, NULL},	/* 0x85 */
	/* 6 */ {ICMP6_RTR_ADVERTISEMENT, wet_ra_proc, NULL},	/* 0x86 */
	/* 7 */ {ICMP6_NEIGH_SOLICITATION, wet_ns_proc, NULL},	/* 0x87 */
};

#define WETHWADDR(weth)	((weth)->pub->mac.octet)
#define WETBSSADDR(weth, idx)	((idx == 0) ? \
		WETHWADDR(weth) : dhd_if_get_macaddr((weth)->pub, idx))
#define WETOSH(weth)	((weth)->pub->osh)

/* special values */
/* 802.3 llc/snap header */
static uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
static uint8 ipv4_bcast[IPV4_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff}; /* IP v4 broadcast address */
static uint8 ipv4_null[IPV4_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00}; /* IP v4 NULL address */

dhd_wet_info_t *
dhd_get_wet_info(dhd_pub_t *pub)
{
	dhd_wet_info_t *p;
	int i;
	p = (dhd_wet_info_t *)MALLOCZ(pub->osh, sizeof(dhd_wet_info_t));
	if (p == NULL) {
		return 0;
	}
	for (i = 0; i < WET_NUMSTAS - 1; i ++)
		p->sta[i].next = &p->sta[i + 1];
	p->stafree = &p->sta[0];
	p->pub = pub;
	return p;
}

void
dhd_free_wet_info(dhd_pub_t *pub, void *wet)
{
	if (wet) {
		MFREE(pub->osh, wet, sizeof(dhd_wet_info_t));
	}
}

int dhd_set_wet_host_ipv4(dhd_pub_t *pub, void *parms, uint32 len)
{
	dhd_wet_info_t *p;
	wet_host_t *wh = (wet_host_t *)parms;
	int ifidx = dhd_bssidx2idx(pub, wh->bssidx);

	if (ifidx >= DHD_MAX_URE_STA) {
		return BCME_RANGE;
	}
	p = (dhd_wet_info_t *)pub->wet_info;
	bcopy(wh->buf, p->ip[ifidx], len);
	return BCME_OK;
}

int dhd_set_wet_host_mac(dhd_pub_t *pub, void *parms, uint32 len)
{
	dhd_wet_info_t *p;
	wet_host_t *wh = (wet_host_t *)parms;
	int ifidx = dhd_bssidx2idx(pub, wh->bssidx);

	if (ifidx >= DHD_MAX_URE_STA) {
		return BCME_RANGE;
	}
	p = (dhd_wet_info_t *)pub->wet_info;
	bcopy(wh->buf, &p->mac[ifidx], len);
	return BCME_OK;
}
/* process Ethernet frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*	> 0 if no further process
*/
static int BCMFASTPATH
wet_eth_proc(dhd_wet_info_t *weth, int ifidx,
	void *sdu, uint8 *frame, int length, int send)
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
		DHD_ERROR(("wet_eth_proc: %s short eth frame, ignored\n",
			send ? "send" : "recv"));
		return -1;
	}

	if (!bcmp(WETBSSADDR(weth, ifidx), frame + ETHER_SRC_OFFSET, ETHER_ADDR_LEN)) {
		return 0;
	}

	if ((*(uint16 *)pt) == HTON16(ETHER_TYPE_8021Q))
		pt += VLAN_TAG_LEN;

	ph = pt + ETHER_TYPE_LEN;
	length -= ph - frame;

	/* Call protocol specific handler to process frame. */
	type = *(uint16 *)pt;

	for (phdlr = &prot_hash[WET_PROT_HASH(pt)];
	     phdlr != NULL; phdlr = phdlr->next) {
		if (phdlr->type != type || !phdlr->prot_proc)
			continue;
		return (phdlr->prot_proc)(weth, ifidx, sdu, frame, ph, length, send);
	}

	DHD_INFO(("%s: %s unknown type (0x%X), ignored %s\n",
		__FUNCTION__, send ? "send" : "recv", type,
		(type == 0xDD86) ? "IPv6":""));
	/* ignore unsupported protocol from different mac addr than us */
	return BCME_UNSUPPORTED;
}

/* process 8021p/Q tagged frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*	> 0 if no further process
*/
static int BCMFASTPATH
wet_vtag_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
	uint8 * eh, uint8 *vtag, int length, int send)
{
	uint16 type;
	uint8 *pt;
	prot_hdlr_t *phdlr;

	/* check minimum length */
	if (length < ETHERVLAN_HDR_LEN) {
		DHD_ERROR(("wet_vtag_proc: %s short VLAN frame, ignored\n",
			send ? "send" : "recv"));
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
		return (phdlr->prot_proc)(weth, ifidx, sdu, eh,
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
static int BCMFASTPATH
wet_ip_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
		uint8 *eh, uint8 *iph, int length, int send)
{
	uint8 type;
	int ihl;
	wet_sta_t *sta;
	ipv4_hdlr_t *iphdlr;
	uint8 *iaddr;
	struct ether_addr *ea = NULL;
	struct ether_addr mcast_ea;
	int ret, ea_off = 0;
	uint32 iaddr_dest;
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

	/* IPv4 only */
	if (length < 1 || (IP_VER(iph) != IP_VER_4)) {
		DHD_INFO(("wet_ip_proc: %s non IPv4 frame, ignored\n",
			send ? "send" : "recv"));
		return -1;
	}

	ihl = IPV4_HLEN(iph);

	/* minimum length */
	if (length < ihl) {
		DHD_ERROR(("wet_ip_proc: %s short IPv4 frame, ignored\n",
		send ? "send" : "recv"));
		return -1;
	}

	/* protocol specific handling */
	type = IPV4_PROT(iph);
	for (iphdlr = &ipv4_hash[WET_IPV4_HASH(type)];
			iphdlr; iphdlr = iphdlr->next) {
		if (iphdlr->type != type || !iphdlr->ipv4_proc)
			continue;
		if ((ret = (iphdlr->ipv4_proc)(weth, ifidx, eh,
			iph, iph + ihl, length - ihl, send)))
			return ret;
	}

	/* generic IP packet handling
	 * Replace source MAC in Ethernet header with wireless's and
	 * keep track of IP MAC mapping when sending frame.
	 */
	if (send) {
		uint32 iaddr_src;
		bool wet_table_upd = TRUE;
		iaddr = iph + IPV4_SRC_IP_OFFSET;
		iaddr_dest = ntoh32(*((uint32 *)(iph + IPV4_DEST_IP_OFFSET)));
		iaddr_src = ntoh32(*(uint32 *)(iaddr));

		/* Do not process and update knowledge base on receipt of a local IP
		 * multicast frame
		 */
		if (IP_ISMULTI(iaddr_dest) && !iaddr_src) {
			DHD_INFO(("recv multicast frame from %s.Don't update hash table\n",
				bcm_ether_ntoa((struct ether_addr*)
				(eh + ETHER_SRC_OFFSET), eabuf)));
			wet_table_upd = FALSE;
		}
		if (wet_table_upd && wet_sta_update_all(weth, ifidx, IPVER_4, iaddr,
				(struct ether_addr*)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
			DHD_INFO(("wet_ip_proc: unable to update STA %u.%u.%u.%u %s\n",
				iaddr[0], iaddr[1], iaddr[2], iaddr[3],
				bcm_ether_ntoa(
				(struct ether_addr*)(eh + ETHER_SRC_OFFSET), eabuf)));
			return -1;
		}
		ea = (struct ether_addr *)WETBSSADDR(weth, ifidx);
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
		iaddr_dest = ntoh32(*((uint32 *)(iaddr)));

		/* Receive unicast frame with multicast ip, covert dest MAC
		 * to ethernet multicast.
		 */
		if (IP_ISMULTI(iaddr_dest)) {
			IPV4_MCAST_TO_ETHER_MCAST(iaddr_dest, mcast_ea.octet);
			ea = &mcast_ea;
		} else {
			if (wet_sta_find_ip(weth, IPVER_4, iaddr, &sta) < 0) {
				DHD_ERROR(("wet_ip_proc: unable to find STA %u.%u.%u.%u\n",
					iaddr[0], iaddr[1], iaddr[2], iaddr[3]));
				return -1;
			} else {
				ea = &sta->mac;
			}
		}
		ea_off = ETHER_DEST_OFFSET;
		eacopy(ea, eh + ea_off);
	}

	return 0;
}

/* process ARP frame - ARP proxy */
/*
 * Return:
 *	= 0 if frame is done ok
 *	< 0 if unable to handle the frame
 *       > 0 if no further process
 */
static int BCMFASTPATH
wet_arp_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
		uint8 *eh, uint8 *arph, int length, int send)
{
	wet_sta_t *sta;
	uint8 *iaddr;
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

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
		if (wet_sta_update_all(weth, ifidx, IPVER_4, iaddr,
				(struct ether_addr*)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
			DHD_INFO(("wet_arp_proc: unable to update STA %u.%u.%u.%u %s\n",
					iaddr[0], iaddr[1], iaddr[2], iaddr[3],
					bcm_ether_ntoa(
					(struct ether_addr*)(eh + ETHER_SRC_OFFSET), eabuf)));
			return -1;
		}
		bcopy(WETBSSADDR(weth, ifidx), eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
		bcopy(WETBSSADDR(weth, ifidx), arph+ARP_SRC_ETH_OFFSET, ETHER_ADDR_LEN);
	}
	/*
	 * Replace dest MAC in Ethernet header as well as dest MAC in
	 * ARP protocol header when processing frame recv'd. Process ARP
	 * replies and Unicast ARP requests.
	 */
	else if ((*(uint16 *)(arph + ARP_OPC_OFFSET) == HTON16(ARP_OPC_REPLY)) ||
		((*(uint16 *)(arph + ARP_OPC_OFFSET) == HTON16(ARP_OPC_REQUEST)) &&
		(!ETHER_ISMULTI(eh)))) {
		iaddr = arph + ARP_TGT_IP_OFFSET;
		if (wet_sta_find_ip(weth, IPVER_4, iaddr, &sta) < 0) {
			DHD_INFO(("wet_arp_proc: unable to find STA %u.%u.%u.%u\n",
				iaddr[0], iaddr[1], iaddr[2], iaddr[3]));
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
static int BCMFASTPATH
wet_udp_proc(dhd_wet_info_t *weth, int ifidx,
		uint8 *eh, uint8 *iph, uint8 *udph, int length, int send)
{
	udp_hdlr_t *udphdlr;
	uint16 port;

	/* check frame length, at least UDP_HDR_LEN */
	if ((length -= UDP_HDR_LEN) < 0) {
		DHD_ERROR(("wet_udp_proc: %s short UDP frame, ignored\n",
			send ? "send" : "recv"));
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
		return (udphdlr->udp_proc)(weth, ifidx, eh, iph, udph,
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
 *	 provide DHCP relay functionality. Otherwise the DHCP
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
 *
 * process DHCP client frame (client to server, or server to relay agent)
 * Return:
 *	= 0 if frame is done ok
 *	< 0 if unable to handle the frame
 *      > 0 if no further process
 */
static int BCMFASTPATH
wet_dhcpc_proc(dhd_wet_info_t *weth, int ifidx,
		uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_sta_t *sta;
	uint16 flags;
	char eabuf[ETHER_ADDR_STR_LEN];
	uint16 port;
	uint8 *ipv4;
	const struct ether_addr *ether;
	uint8 bss = BSSCFG_IDX_TO_WET_NWKID(ifidx);
	BCM_REFERENCE(eabuf);

	/*
	 * FIXME: validate DHCP body:
	 * htype Ethernet 1, hlen Ethernet 6, frame length at least 242.
	 */
	if (length < DHCP_OPT_OFFSET)
		return 0;
	/* only interested in requests when sending to server */
	if (send && *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REQUEST)
		return 0;
	/* only interested in replies when receiving from server as a relay agent */
	if (!send && *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return 0;

	/* send request */
	if (send) {
		/* find existing or alloc new IP/MAC mapping entry */
		if (wet_sta_update_mac(weth, ifidx,
				(struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), &sta) < 0) {
			DHD_INFO(("wet_dhcpc_proc: unable to update STA %s\n",
				bcm_ether_ntoa(
				(struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
			return -1;
		}
		bcopy(dhcp + DHCP_FLAGS_OFFSET, &flags, DHCP_FLAGS_LEN);
		/* We can always relay the host's request when we know its MAC addr. */
		if (!ETHER_ISNULLADDR(weth->mac[bss].octet) &&
				!bcmp(dhcp + DHCP_CHADDR_OFFSET, &weth->mac[bss], ETHER_ADDR_LEN)) {
			/* replace chaddr with host's MAC */
			csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
					dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN,
					WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN);
			bcopy(WETBSSADDR(weth, ifidx), dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN);
			/* force reply to be unicast */
			flags &= ~HTON16(DHCP_FLAG_BCAST);
		}
		/* We can relay other clients' requests when we know the host's IP addr. */
		else if (!IPV4_ADDR_NULL(weth->ip[bss])) {
			/* we can only handle the first hop otherwise drop it */
			if (!IPV4_ADDR_NULL(dhcp + DHCP_GIADDR_OFFSET)) {
				DHD_INFO(("wet_dhcpc_proc: not first hop, ignored\n"));
				return -1;
			}
			/* replace giaddr with host's IP */
			csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
					dhcp + DHCP_GIADDR_OFFSET, IPV4_ADDR_LEN,
					weth->ip[bss], IPV4_ADDR_LEN);
			bcopy(weth->ip[bss], dhcp + DHCP_GIADDR_OFFSET, IPV4_ADDR_LEN);
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
		bcopy(WETBSSADDR(weth, ifidx), eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
	}
	/* relay recv'd reply to its destiny */
	else if (!IPV4_ADDR_NULL(weth->ip[bss]) &&
			!bcmp(dhcp + DHCP_GIADDR_OFFSET, weth->ip[bss], IPV4_ADDR_LEN)) {
		/* find IP/MAC mapping entry */
		if (wet_sta_find_mac(weth,
		(struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), &sta) < 0) {
			DHD_INFO(("wet_dhcpc_proc: unable to find STA %s\n",
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
		DHD_INFO(("wet_dhcpc_proc: ignore recv'd frame from %s\n",
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
 *      > 0 if no further process
 */
static int BCMFASTPATH
wet_dhcps_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_sta_t *sta;
	char eabuf[ETHER_ADDR_STR_LEN];
	uint8 bss = BSSCFG_IDX_TO_WET_NWKID(ifidx);
	BCM_REFERENCE(eabuf);

	/*
	 * FIXME: validate DHCP body:
	 *  htype Ethernet 1, hlen Ethernet 6, frame length at least 242.
	 */
	if (length < DHCP_OPT_OFFSET)
		return 0;
	/* only interested in replies when receiving from server */
	if (send || *(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return 0;

	/* find IP/MAC mapping entry */
	if (wet_sta_find_mac(weth, (struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), &sta) < 0) {
		DHD_INFO(("wet_dhcps_proc: unable to find STA %s\n",
		bcm_ether_ntoa((struct ether_addr*)(dhcp + DHCP_CHADDR_OFFSET), eabuf)));
		return -1;
	}
	/* relay the reply to the host when we know the host's MAC addr */
	if (!ETHER_ISNULLADDR(weth->mac[bss].octet) &&
			!bcmp(dhcp + DHCP_CHADDR_OFFSET, WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN)) {
		csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
				dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN,
				weth->mac[bss].octet, ETHER_ADDR_LEN);
		bcopy(&weth->mac[bss], dhcp + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN);
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

/* process IPv6 frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_ipv6_proc(dhd_wet_info_t *weth, int ifidx, void *sdu,
	uint8 *eh, uint8 *iph, int length, int send)
{
	uint8 type;
	wet_sta_t *sta;
	ipv6_hdlr_t *ip6hdlr;
	uint8 *iaddr;
	int ret;
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

#define IP6_HDRLEN (40)

	/* IPv6 only */
	if ((IP_VER(iph) != IP_VER_6) ||
		(length < IP6_HDRLEN)) {
		DHD_INFO((
		"wet_ipv6_proc: %s non IPv6 frame or too short, ignored\n",
			send ? "send" : "recv"));
		return -1;
	}

	/* protocol specific handling */
	type = IPV6_PROT(iph);

	if (send && IPV6_ADDR_NULL((iph + IPV6_SRC_IP_OFFSET))) {
		/* Ipv6 Duplicate Address Detection */
		goto replace_smac;
	}

	for (ip6hdlr = &ipv6_hash[WET_IPV6_HASH(type)];
	     ip6hdlr; ip6hdlr = ip6hdlr->next) {
		if (ip6hdlr->type != type || !ip6hdlr->ipv6_proc)
			continue;
		if ((ret = (ip6hdlr->ipv6_proc)(weth, ifidx, eh,
			iph, iph + IP6_HDRLEN,
				length - IP6_HDRLEN, send)))
			return ret;
	}

	/* generic IP packet handling */
	if (send) {
		iaddr = iph + IPV6_SRC_IP_OFFSET;
		if (wet_sta_update_all(weth, ifidx, IPVER_6, iaddr,
				(struct ether_addr*)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
			DHD_ERROR(("wet_ipv6_proc: unable to update STA "
			"%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x %s\n",
				iaddr[0], iaddr[1], iaddr[2], iaddr[3],
				iaddr[4], iaddr[5], iaddr[6], iaddr[7],
				iaddr[8], iaddr[9], iaddr[10], iaddr[11],
				iaddr[12], iaddr[13], iaddr[14], iaddr[15],
				bcm_ether_ntoa(
					(struct ether_addr*)(eh + ETHER_SRC_OFFSET), eabuf)));
			return -1;
		}
replace_smac:
		bcopy(WETBSSADDR(weth, ifidx), eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
	}
	/* no action for received bcast/mcast ethernet frame */
	else if (!ETHER_ISMULTI(eh)) {
		iaddr = iph + IPV6_DEST_IP_OFFSET;
		if (IPV6_ISMULTI(iaddr)) {
			struct ether_addr ea;
			IPV6_MCAST_TO_ETHER_MCAST(iaddr, ea.octet);
			bcopy(&ea, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
		}
		else {
			if (wet_sta_find_ip(weth, IPVER_6, iaddr, &sta) < 0) {
				DHD_ERROR(("wet_ipv6_proc: unable to find STA "
				"%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x %s\n",
					iaddr[0], iaddr[1], iaddr[2], iaddr[3],
					iaddr[4], iaddr[5], iaddr[6], iaddr[7],
					iaddr[8], iaddr[9], iaddr[10], iaddr[11],
					iaddr[12], iaddr[13], iaddr[14], iaddr[15],
					bcm_ether_ntoa(
					(struct ether_addr*)(eh + ETHER_SRC_OFFSET), eabuf)));
				return -1;
			}
			bcopy(&sta->mac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
		}
	}

	return 0;
}

/* process ICMPv6 frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_icmpv6_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int icmp6len, int send)
{
	icmpv6_hdlr_t *icmp6hdlr;
	uint8 type;
	uint8 *iaddr;
	wet_sta_t *sta;
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

	if (icmp6len < sizeof(icmpv6_hdlr_t)) {
		DHD_INFO(("%s: length too small. ignore icmp6len %d !!\n",
			__FUNCTION__, icmp6len));
		return -1;
	}
	if (send) {
		type = *(uint8 *)(icmp6h);
		for (icmp6hdlr = &icmpv6_hash[WET_ICMPV6_HASH(type)];
		     icmp6hdlr; icmp6hdlr = icmp6hdlr->next) {
			if (icmp6hdlr->type != type || !icmp6hdlr->icmpv6_proc)
				continue;

			/* Replace source MAC in Ethernet header first
			 * and update wet_sta as well
			 */
			iaddr = iph + IPV6_SRC_IP_OFFSET;
			if (wet_sta_update_all(weth, ifidx, IPVER_6, iaddr,
				(struct ether_addr*)(eh + ETHER_SRC_OFFSET), &sta) < 0) {
				DHD_ERROR(("wet_icmpv6_proc: unable to update STA "
				"%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x %s\n",
					iaddr[0], iaddr[1], iaddr[2], iaddr[3],
					iaddr[4], iaddr[5], iaddr[6], iaddr[7],
					iaddr[8], iaddr[9], iaddr[10], iaddr[11],
					iaddr[12], iaddr[13], iaddr[14], iaddr[15],
				bcm_ether_ntoa(
					(struct ether_addr*)(eh + ETHER_SRC_OFFSET), eabuf)));
				return -1;
			}
			bcopy(WETBSSADDR(weth, ifidx), eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);

			return (icmp6hdlr->icmpv6_proc)(
					weth, ifidx, eh, iph, icmp6h, icmp6len, send);
		}
	}

	return 0;
}

/* process Neighbor solicitation frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_ns_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int icmplen, int send)
{
	struct icmp6_opt *opt;
	int general_hdrlen = ICMP6_NEIGHSOL_OPT_OFFSET;
	int optlen;
	icmplen -= general_hdrlen;
	opt = (struct icmp6_opt *)(icmp6h + general_hdrlen);
	optlen = opt->length << 3; /* length in unit of 8 octets */

	/*
	 * Replace source MAC in source link-layer address
	 * of Neighbor solicitation frame when processing frame sent.
	 */
	while (send && (icmplen > 0) && (icmplen >= optlen)) {
		if (opt->type == ICMP6_OPT_TYPE_SRC_LINK_LAYER) {
			csum_fixup_16(icmp6h + ICMP_CHKSUM_OFFSET, opt->data,
			ETHER_ADDR_LEN, WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN);
			bcopy(WETBSSADDR(weth, ifidx), opt->data, ETHER_ADDR_LEN);
		}
		icmplen -= optlen;
		opt = (struct icmp6_opt *)((uint8*)opt + optlen);
		optlen = opt->length << 3; /* length in unit of 8 octets */
	}
	return 1;
}

/* process Neighbor advertisement frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_na_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int icmplen, int send)
{
	struct icmp6_opt *opt;
	int general_hdrlen = ICMP6_NEIGHADV_OPT_OFFSET;
	int optlen;
	icmplen -= general_hdrlen;
	opt = (struct icmp6_opt *)(icmp6h + general_hdrlen);
	optlen = opt->length << 3; /* length in unit of 8 octets */

	/*
	 * Replace source MAC in Target link-layer address of
	 * Neighbor advertisement frame when processing frame sent.
	 */
	while (send && (icmplen > 0) && (icmplen >= optlen)) {
		if (opt->type == ICMP6_OPT_TYPE_TGT_LINK_LAYER) {
			csum_fixup_16(icmp6h + ICMP_CHKSUM_OFFSET, opt->data,
			ETHER_ADDR_LEN, WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN);
			bcopy(WETBSSADDR(weth, ifidx), opt->data, ETHER_ADDR_LEN);
		}
		icmplen -= optlen;
		opt = (struct icmp6_opt *)((uint8*)opt + optlen);
		optlen = opt->length << 3; /* length in unit of 8 octets */
	}
	return 1;
}

/* process Router solicitation frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_rs_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int icmplen, int send)
{
	struct icmp6_opt *opt;
	int general_hdrlen = ICMP6_RTRSOL_OPT_OFFSET;
	int optlen;
	icmplen -= general_hdrlen;
	opt = (struct icmp6_opt *)(icmp6h + general_hdrlen);
	optlen = opt->length << 3; /* length in unit of 8 octets */

	/*
	 * Replace source MAC in source link-layer address of
	 * Router solicitation frame when processing frame sent.
	 */
	while (send && (icmplen > 0) && (icmplen >= optlen)) {
		if (opt->type == ICMP6_OPT_TYPE_SRC_LINK_LAYER) {
			csum_fixup_16(icmp6h + ICMP_CHKSUM_OFFSET, opt->data,
			ETHER_ADDR_LEN, WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN);
			bcopy(WETBSSADDR(weth, ifidx), opt->data, ETHER_ADDR_LEN);
		}
		icmplen -= optlen;
		opt = (struct icmp6_opt *)((uint8*)opt + optlen);
		optlen = opt->length << 3; /* length in unit of 8 octets */
	}
	return 1;
}

/* process Router advertisement frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_ra_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int icmplen, int send)
{
	struct icmp6_opt *opt;
	int general_hdrlen = ICMP6_RTRADV_OPT_OFFSET;
	int optlen;
	icmplen -= general_hdrlen;
	opt = (struct icmp6_opt *)(icmp6h + general_hdrlen);
	optlen = opt->length << 3; /* length in unit of 8 octets */

	/*
	 * Replace source MAC in source link-layer address of
	 * Router advertisement frame when processing frame sent.
	 */
	while (send && (icmplen > 0) && (icmplen >= optlen)) {
		if (opt->type == ICMP6_OPT_TYPE_SRC_LINK_LAYER) {
				csum_fixup_16(icmp6h + ICMP_CHKSUM_OFFSET, opt->data,
				ETHER_ADDR_LEN, WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN);
				bcopy(WETBSSADDR(weth, ifidx), opt->data, ETHER_ADDR_LEN);
		}
		icmplen -= optlen;
		opt = (struct icmp6_opt *)((uint8*)opt + optlen);
		optlen = opt->length << 3; /* length in unit of 8 octets */
	}

	/* no further processing! */
	return 1;
}

/* process Redirect Message frame */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_rm_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *icmp6h, int icmplen, int send)
{
	struct icmp6_opt *opt;
	int general_hdrlen = ICMP6_REDIRECT_OPT_OFFSET;
	int optlen;
	icmplen -= general_hdrlen;
	opt = (struct icmp6_opt *)(icmp6h + general_hdrlen);
	optlen = opt->length << 3; /* length in unit of 8 octets */

	/*
	 * Replace source MAC in source link-layer address of
	 * Redirect Message frame when processing frame sent.
	 */
	while (send && (icmplen > 0) && (icmplen >= optlen)) {
		if (opt->type == ICMP6_OPT_TYPE_TGT_LINK_LAYER) {
			csum_fixup_16(icmp6h + ICMP_CHKSUM_OFFSET, opt->data,
			ETHER_ADDR_LEN, WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN);
			bcopy(WETBSSADDR(weth, ifidx), opt->data, ETHER_ADDR_LEN);
		}
		icmplen -= optlen;
		opt = (struct icmp6_opt *)((uint8*)opt + optlen);
		optlen = opt->length << 3; /* length in unit of 8 octets */
	}
	return 1;
}

/* Search the specified dhcp6 option and return the offset of it
	dhcp6 option : code (2 bytes) + data length (2 bytes) + data (variable length)
*/
static int32
wlc_dhcp6_option_find(uint8 *dhcp, uint16 dhcplen, uint16 option_code)
{
	bool found = FALSE;
	uint8* ptr;
	uint16 code, optlen, offset;
	uint8 msg_type = *(dhcp + DHCP6_TYPE_OFFSET);

	/* Get the pointer to options */
	if (msg_type == DHCP6_TYPE_RELAYFWD)
		offset = DHCP6_RELAY_OPT_OFFSET;
	else
		offset = DHCP6_MSG_OPT_OFFSET;

	while (offset < dhcplen) {
		ptr = dhcp + offset;
		code = NTOH16(*(ptr + DHCP6_OPT_CODE_OFFSET));
		optlen =  NTOH16(*(ptr + DHCP6_OPT_LEN_OFFSET));
		if ((code == option_code) && (offset + optlen + 4) <= dhcplen) {
			found = TRUE;
			break;
		}
		offset = offset + 4 /* code, len */ + optlen;
	}

	return found ? offset : -1;
}

/* process DHCPv6 client frame (client to server, or server to relay agent) */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_dhcp6c_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_sta_t *sta;
	int32 opt_offset, duid_offset;
	uint8 msg_type;
	/* Value of type_bypass for interested type is 0 */
	const bool type_bypass[14] =
		{1 /* No such type */, 0 /* DHCP6_TYPE_SOLICIT */, 1 /* DHCP6_TYPE_ADVERTISE */,
		 0 /* DHCP6_TYPE_REQUEST */, 0 /* DHCP6_TYPE_CONFIRM */, 0 /* DHCP6_TYPE_RENEW */,
		 0 /* DHCP6_TYPE_REBIND */, 1 /* DHCP6_TYPE_REPLY */, 0 /* DHCP6_TYPE_RELEASE */,
		 0 /* DHCP6_TYPE_DECLINE */, 1 /* DHCP6_TYPE_RECONFIGURE */,
		 0 /* DHCP6_TYPE_INFOREQ */, 0 /* DHCP6_TYPE_RELAYFWD */,
		 1 /* DHCP6_TYPE_RELAYREPLY */};
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

	if (!send)
		return 0;

	msg_type = *(dhcp + DHCP6_TYPE_OFFSET);

	/* only interested in solict or request when sending to server */
	if (send && type_bypass[msg_type])
		return 0;

	/* If client identifier option is not present then
	 * no processing required.
	 */
	opt_offset = wlc_dhcp6_option_find(dhcp, length,
		DHCP6_OPT_CODE_CLIENTID);
	if (opt_offset == -1) {
		DHD_ERROR(("%s: dhcp6 clientid option not found\n",
			__FUNCTION__));
		return 0;
	}
	duid_offset = (opt_offset + DHCP6_OPT_DATA_OFFSET);

	/* Look for DUID-LLT or DUID-LL */
	if (*(uint16 *)(dhcp + duid_offset) == HTON16(1))
		duid_offset += 8;
	else if (*(uint16 *)(dhcp + duid_offset) == HTON16(3))
		duid_offset += 4;
	else
		return 0;

	if (length < (duid_offset + ETHER_ADDR_LEN))
		return 0;
	/* find existing or alloc new IP/MAC mapping entry */
	if (wet_sta_update_mac(weth, ifidx,
		(struct ether_addr*)(dhcp + duid_offset), &sta) < 0) {
		DHD_INFO(("wet_dhcp6c_proc: unable to update STA %s\n",
			bcm_ether_ntoa(
				(struct ether_addr*)(dhcp + duid_offset), eabuf)));
		return -1;
	}

	/* replace chaddr with our MAC */
	csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
		dhcp + duid_offset, ETHER_ADDR_LEN,
		WETBSSADDR(weth, ifidx), ETHER_ADDR_LEN);
	bcopy(WETBSSADDR(weth, ifidx), dhcp + duid_offset, ETHER_ADDR_LEN);

	/* replace the Ethernet source MAC with ours */
	bcopy(WETBSSADDR(weth, ifidx), eh + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);

	/* no further processing! */
	return 1;
}

/* process DHCPv6 server frame (server to client) */
/*
* Return:
*	= 0 if frame is done ok
*	< 0 if unable to handle the frame
*       > 0 if no further process
*/
static int
wet_dhcp6s_proc(dhd_wet_info_t *weth, int ifidx,
	uint8 *eh, uint8 *iph, uint8 *udph, uint8 *dhcp, int length, int send)
{
	wet_sta_t *sta;
	int32 opt_offset, duid_offset;
	uint8 *iaddr;
	uint8 msg_type;
	/* Value of type_bypass for interested type is 0 */
	const bool type_bypass[14] =
		{1 /* No such type */, 1 /* DHCP6_TYPE_SOLICIT */, 0 /* DHCP6_TYPE_ADVERTISE */,
		 1 /* DHCP6_TYPE_REQUEST */, 1 /* DHCP6_TYPE_CONFIRM */, 1 /* DHCP6_TYPE_RENEW */,
		 1 /* DHCP6_TYPE_REBIND */, 0 /* DHCP6_TYPE_REPLY */, 1 /* DHCP6_TYPE_RELEASE */,
		 1 /* DHCP6_TYPE_DECLINE */, 0 /* DHCP6_TYPE_RECONFIGURE */,
		 1 /* DHCP6_TYPE_INFOREQ */, 1 /* DHCP6_TYPE_RELAYFWD */,
		 0 /* DHCP6_TYPE_RELAYREPLY */};

	if (send)
		return 0;

	msg_type = *(dhcp + DHCP6_TYPE_OFFSET);

	if (!send && type_bypass[msg_type])
		return 0;
	/* If server identifier option is not present then
	 * no processing required.
	 */
	opt_offset =
		wlc_dhcp6_option_find(dhcp, length,	DHCP6_OPT_CODE_CLIENTID);
	if (opt_offset == -1) {
		DHD_ERROR(("%s: dhcp6 clientid option not found\n",
			__FUNCTION__));
		return 0;
	}

	duid_offset = (opt_offset + DHCP6_OPT_DATA_OFFSET);

	/* Look for DUID-LLT or DUID-LL */
	if (*(uint16 *)(dhcp + duid_offset) == HTON16(1))
		duid_offset += 8;
	else if (*(uint16 *)(dhcp + duid_offset) == HTON16(3))
		duid_offset += 4;
	else
		return 0;

	/* find IP/MAC mapping entry */
	iaddr = iph + IPV6_DEST_IP_OFFSET;

	if (length < (duid_offset + ETHER_ADDR_LEN))
		return 0;
	if (wet_sta_find_ip(weth, IPVER_6, iaddr, &sta) < 0) {
		DHD_ERROR(("wet_dhcp6s_proc: unable to find STA "
		"%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",
			iaddr[0], iaddr[1], iaddr[2], iaddr[3],
			iaddr[4], iaddr[5], iaddr[6], iaddr[7],
			iaddr[8], iaddr[9], iaddr[10], iaddr[11],
			iaddr[12], iaddr[13], iaddr[14], iaddr[15]));
		return -1;
	}
	bcopy(&sta->mac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);

	csum_fixup_16(udph + UDP_CHKSUM_OFFSET,
		dhcp + duid_offset, ETHER_ADDR_LEN,
		sta->mac.octet, ETHER_ADDR_LEN);
	bcopy(&sta->mac, dhcp + duid_offset, ETHER_ADDR_LEN);
	bcopy(&sta->mac, eh + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);

	return 1;
}

/* alloc IP/MAC mapping entry
 * Returns 0 if succeeded; < 0 otherwise.
 */
static int
wet_sta_alloc(dhd_wet_info_t *weth, int ifidx, wet_sta_t **saddr)
{
	wet_sta_t *sta;

	/* allocate a new one */
	if (!weth->stafree) {
		DHD_INFO(("wet_sta_alloc: no room for another STA\n"));
		return -1;
	}
	sta = weth->stafree;
	weth->stafree = sta->next;

	/* init them just in case */
	sta->next = NULL;
	sta->next_ip[IPVER_4] = NULL;
	sta->next_ip[IPVER_6] = NULL;
	sta->next_mac = NULL;
	sta->bss = BSSCFG_IDX_TO_WET_NWKID(ifidx);
	*saddr = sta;
	return 0;
}

/* update IP/MAC mapping entry and hash
 * Returns 0 if succeeded; < 0 otherwise.
 */
static int BCMFASTPATH
wet_sta_update_all(dhd_wet_info_t *weth, int ifidx, uint8 ipver,
		uint8 *iaddr, struct ether_addr *eaddr, wet_sta_t **saddr)
{
	wet_sta_t *sta;
	int i = WET_STA_HASH_OK;
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

	/* For ipv6, check addr null in wet_ipv6_proc */
	if ((ipver == IPVER_4) && IPV4_ADDR_NULL(iaddr))
		return 0;

	/* find the existing one and remove it from the old mac hash link */
	if (!wet_sta_find_ip(weth, ipver, iaddr, &sta)) {
		i = WET_STA_HASH_MAC(eaddr->octet);
		if (bcmp(&sta->mac, eaddr, ETHER_ADDR_LEN)) {
			wet_sta_t *sta2, **next;
			for (next = &weth->stahash_mac[i], sta2 = *next;
				sta2; sta2 = sta2->next_mac) {
				if (sta2 == sta)
					break;
				next = &sta2->next_mac;
			}
			if (sta2) {
				*next = sta2->next_mac;
				sta2->next_mac = NULL;
			}
			i = WET_STA_HASH_UNK;
		}
	}
	/* allocate a new one and hash it by IP */
	else if (!wet_sta_alloc(weth, ifidx, &sta)) {
		i = WET_STA_HASH_IP(iaddr, ipver);
		bcopy(iaddr, sta->ip[ipver], IP_ADDR_LEN[ipver]);
		sta->next_ip[ipver] = weth->stahash_ip[ipver][i];
		weth->stahash_ip[ipver][i] = sta;
		i = WET_STA_HASH_UNK;
	}
	/* bail out if we can't find nor create any */
	else {
		DHD_INFO(("wet_sta_update_all: %s, unable to alloc STA ipv%d, %u.%u.%u.%u %s\n",
		dhd_ifname(weth->pub, ifidx),
		((ipver == IPVER_4) ? 4: 6), iaddr[0], iaddr[1], iaddr[2], iaddr[3],
		bcm_ether_ntoa(eaddr, eabuf)));
		return -1;
	}

	/* update MAC and hash by new MAC */
	if (i == WET_STA_HASH_UNK) {
		i = WET_STA_HASH_MAC(eaddr->octet);
		bcopy(eaddr, &sta->mac, ETHER_ADDR_LEN);
		sta->next_mac = weth->stahash_mac[i];
		sta->bss = BSSCFG_IDX_TO_WET_NWKID(ifidx);
		weth->stahash_mac[i] = sta;
	}
	*saddr = sta;
	return 0;
}

/* update IP/MAC mapping entry and hash */
static int BCMFASTPATH
wet_sta_update_mac(dhd_wet_info_t *weth, int ifidx,
	struct ether_addr *eaddr, wet_sta_t **saddr)
{
	wet_sta_t *sta;
	int i;
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

	/* find the existing one */
	if (!wet_sta_find_mac(weth, eaddr, &sta))
		;
	/* allocate a new one and hash it */
	else if (!wet_sta_alloc(weth, ifidx, &sta)) {
		i = WET_STA_HASH_MAC(eaddr->octet);
		bcopy(eaddr, &sta->mac, ETHER_ADDR_LEN);
		sta->next_mac = weth->stahash_mac[i];
		weth->stahash_mac[i] = sta;
	}
	/* bail out if we can't find nor create any */
	else {
		DHD_INFO(("wet_sta_update_mac: unable to alloc STA %s\n",
		bcm_ether_ntoa(eaddr, eabuf)));
		return -1;
	}

	*saddr = sta;
	return 0;
}

/*  Remove MAC entry from hash list
 *  NOTE:  This only removes the entry matching "eaddr" from the MAC
 *  list.  The caller needs to remove from the IP list and
 *  put back onto the free list to completely remove the entry
 *  from the WET table.
 */
static int BCMFASTPATH
wet_sta_remove_mac_entry(dhd_wet_info_t *weth, struct ether_addr *eaddr)
{
	wet_sta_t *sta, *prev;
	int i = WET_STA_HASH_MAC(eaddr->octet);
	char eabuf[ETHER_ADDR_STR_LEN];
	int found = 0;
	BCM_REFERENCE(eabuf);

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
		DHD_INFO(("wet_sta_remove_mac_entry: unable to find STA %s entry\n",
		bcm_ether_ntoa(eaddr, eabuf)));
		return -1;
	}

	/* fix the list */
	if (prev == sta)
		weth->stahash_mac[i] = sta->next_mac; /* removing first entry in this bucket */
	else
		prev->next_mac = sta->next_mac;

	return 0;
}

/* find IP/MAC mapping entry by IP address
 * Returns 0 if succeeded; < 0 otherwise.
 */
static int BCMFASTPATH
wet_sta_find_ip(dhd_wet_info_t *weth, uint8 ipver,
		uint8 *iaddr, wet_sta_t **saddr)
{
	int i = WET_STA_HASH_IP(iaddr, ipver);
	wet_sta_t *sta;

	/* find the existing one by IP */
	for (sta = weth->stahash_ip[ipver][i]; sta; sta = sta->next_ip[ipver]) {
		if (bcmp(sta->ip[ipver], iaddr, IP_ADDR_LEN[ipver]))
			continue;
		*saddr = sta;
		return 0;
	}

	/* sta has not been learned */
	DHD_INFO(("wet_sta_find_ip: unable to find STA (ipv%d) %u.%u.%u.%u\n",
		((ipver == IPVER_4) ? 4 : 6), iaddr[0], iaddr[1], iaddr[2], iaddr[3]));
	return -1;
}

/* find IP/MAC mapping entry by MAC address
 * Returns 0 if succeeded; < 0 otherwise.
 */
static int BCMFASTPATH
wet_sta_find_mac(dhd_wet_info_t *weth, struct ether_addr *eaddr, wet_sta_t **saddr)
{
	int i = WET_STA_HASH_MAC(eaddr->octet);
	wet_sta_t *sta;
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);

	/* find the existing one by MAC */
	for (sta = weth->stahash_mac[i]; sta; sta = sta->next_mac) {
		if (bcmp(&sta->mac, eaddr, ETHER_ADDR_LEN))
			continue;
		*saddr = sta;
		return 0;
	}

	/* sta has not been learnt */
	DHD_INFO(("wet_sta_find_mac: unable to find STA %s\n",
		bcm_ether_ntoa(eaddr, eabuf)));
	return -1;
}

/* Adjust 16 bit checksum - taken from RFC 3022.
 *
 *   The algorithm below is applicable only for even offsets (i.e., optr
 *   below must be at an even offset from start of header) and even lengths
 *   (i.e., olen and nlen below must be even).
 */
static void BCMFASTPATH
csum_fixup_16(uint8 *chksum, uint8 *optr, int olen, uint8 *nptr, int nlen)
{
	long x, old, new;

	ASSERT(!((uintptr)optr&1) && !(olen&1));
	ASSERT(!((uintptr)nptr&1) && !(nlen&1));

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

/* Process frames in transmit direction by replacing source MAC with
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
int BCMFASTPATH
dhd_wet_send_proc(void *wet, int ifidx, void *sdu, void **new)
{
	dhd_wet_info_t *weth = (dhd_wet_info_t *)wet;
	uint8 *frame = PKTDATA(WETOSH(weth), sdu);
	int length = PKTLEN(WETOSH(weth), sdu);
	void *pkt = sdu;

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
		DHD_ERROR(("dhd_wet_send_proc: unable to process short frame\n"));
		return -1;
	}
	if (ETHER_ISMULTI(frame)) {
		length = pkttotlen(WETOSH(weth), sdu);
		if (!(pkt = PKTGET(WETOSH(weth), length, TRUE))) {
			DHD_ERROR(("dhd_wet_send_proc: unable to alloc, dropped\n"));
			return -1;
		}
		frame = PKTDATA(WETOSH(weth), pkt);
		pktcopy(WETOSH(weth), sdu, 0, length, frame);
		/* Transfer priority */
		PKTSETPRIO(pkt, PKTPRIO(sdu));
#if defined(BCA_CPEROUTER)
		if (IS_SKBUFF_PTR(sdu) && ((struct sk_buff*)sdu)->blog_p) {
			blog_skb((struct sk_buff *)pkt);
			blog_copy(((struct sk_buff *)pkt)->blog_p, ((struct sk_buff *)sdu)->blog_p);
			((struct sk_buff *)pkt)->dev = ((struct sk_buff *)sdu)->dev;
		}
#endif
		PKTFREE(WETOSH(weth), sdu, TRUE);
		PKTSETLEN(WETOSH(weth), pkt, length);
	}
	*new = pkt;

	/* process frame */
	return wet_eth_proc(weth, ifidx, pkt, frame, length, 1) < 0 ? -1 : 0;
}

/*
 * Process frames in receive direction by replacing destination MAC with
 * the one found in IP MAC address mapping table.
 * Return:
 *	= 0 if frame is done ok;
 *	< 0 if unable to handle the frame;
 */
int BCMFASTPATH
dhd_wet_recv_proc(void *wet, int ifidx, void *sdu)
{
	dhd_wet_info_t *weth = (dhd_wet_info_t *)wet;
	/* process frame */
	return wet_eth_proc(weth, ifidx, sdu, PKTDATA(WETOSH(weth), sdu),
			PKTLEN(WETOSH(weth), sdu), 0) < 0 ? -1 : 0;
}

/* Delete WET Database */
void
dhd_wet_sta_delete_list(dhd_pub_t *dhd_pub)
{
	wet_sta_t *sta;
	int i, j, ipver;
	dhd_wet_info_t *weth = dhd_pub->wet_info;

	for (i = 0; i < WET_STA_HASH_SIZE; i ++) {
		for (sta = weth->stahash_mac[i]; sta; sta = sta->next_mac) {
			wet_sta_t *sta2, **next;
			for (ipver = IPVER_4; ipver <= IPVER_6; ipver ++) {
				j = WET_STA_HASH_IP(sta->ip[ipver], ipver);
				for (next = &weth->stahash_ip[ipver][j], sta2 = *next;
						sta2; sta2 = sta2->next_ip[ipver]) {
					if (sta2 == sta)
						break;
					next = &sta2->next_ip[ipver];
				}
				if (sta2) {
					*next = sta2->next_ip[ipver];
					sta2->next_ip[ipver] = NULL;
				}
				j = WET_STA_HASH_UNK;
			}

			wet_sta_remove_mac_entry(weth, &sta->mac);
			memset(sta, 0, sizeof(wet_sta_t));
		}
	}
}
void
dhd_wet_dump(dhd_pub_t *dhdp, struct bcmstrbuf *b)
{
	char eabuf[ETHER_ADDR_STR_LEN];
	wet_sta_t *sta;
	int i;

	dhd_wet_info_t *weth = dhdp->wet_info;

	for (i = 0; i < DHD_MAX_URE_STA; i ++) {
		if (!ETHER_ISNULLADDR(&weth->mac[i]) || (uint32)(weth->ip[i][0])) {
			bcm_bprintf(b, "Host MAC %u: %s\n", i,
				bcm_ether_ntoa(&weth->mac[i], eabuf));
			bcm_bprintf(b, "Host IP %u: %u.%u.%u.%u\n", i,
				weth->ip[i][0], weth->ip[i][1], weth->ip[i][2], weth->ip[i][3]);
		}
	}
	bcm_bprintf(b, "Entry\tBSS\tEnetAddr\t\tInetAddr\n");
	for (i = 0; i < WET_NUMSTAS; i ++) {
		if (weth->sta[i].next)
			continue;
		/* format the entry dump */
		sta = &weth->sta[i];
			if (!IPV4_ADDR_NULL(&sta->ip[IPVER_4])) {
				bcm_bprintf(b, "%u\t%u\t%s\t%u.%u.%u.%u\n",
						i, sta->bss, bcm_ether_ntoa(&sta->mac, eabuf),
						sta->ip[IPVER_4][0], sta->ip[IPVER_4][1],
						sta->ip[IPVER_4][2], sta->ip[IPVER_4][3]);
			}
			if (!IPV6_ADDR_NULL(sta->ip[IPVER_6])) {
				bcm_bprintf(b,
					"%u\t%u\t%s\t%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",
					i, sta->bss, bcm_ether_ntoa(&sta->mac, eabuf),
					sta->ip[IPVER_6][0], sta->ip[IPVER_6][1],
					sta->ip[IPVER_6][2], sta->ip[IPVER_6][3],
					sta->ip[IPVER_6][4], sta->ip[IPVER_6][5],
					sta->ip[IPVER_6][6], sta->ip[IPVER_6][7],
					sta->ip[IPVER_6][8], sta->ip[IPVER_6][9],
					sta->ip[IPVER_6][10], sta->ip[IPVER_6][11],
					sta->ip[IPVER_6][12], sta->ip[IPVER_6][13],
					sta->ip[IPVER_6][14], sta->ip[IPVER_6][15]);
			}
	}
}

#ifdef DHD_DPSTA
void
dhd_dpsta_wet_register(dhd_pub_t *dhdp)
{
	psta_if_api_t api;

	DHD_ERROR(("dhd_dpsta_wet_register()\n"));

	/* Register proxy sta APIs with DPSTA module */
	api.is_ds_sta = (bool (*)(void *, void *, struct ether_addr *))dhd_psta_is_ds_sta;
	api.psta_find = (void *(*)(void *, void *, uint8 *)) NULL;
	api.bss_auth = (bool (*)(void *, void *))dhd_psta_authorized;
	api.wl = dhdp->info;
	api.psta = api.bsscfg = dhdp;
	api.mode = DPSTA_MODE_WET;
	(void) dpsta_register(dhd_get_instance(dhdp), &api);

	return;
}
#endif /* DHD_DPSTA */
