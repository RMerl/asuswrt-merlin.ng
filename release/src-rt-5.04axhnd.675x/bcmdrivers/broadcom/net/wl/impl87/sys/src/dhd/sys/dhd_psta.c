/*
 * DHD PSTA processing for ARP,DHCP,ICMPv6,DHCP6
 * PSTA acting as Proxy STA Repeater (PSR)
 * PSTA forwarding packets for wired/wireless sta
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
 * $Id: dhd_psta.c 469476 2014-04-15 12:39:01Z $
 */
#include <bcmutils.h>
#include <bcmendian.h>
#include <dhd_dbg.h>

#include <dngl_stats.h>
#include <dhd.h>
#ifdef PCIE_FULL_DONGLE
#include <dhd_flowring.h>
#endif
#include <dhd_psta.h>
#include <bcm_psta.h>

#include <bcmudp.h>
#include <bcmdhcp.h>

#ifdef BCM_NBUFF
#include <dhd_nbuff.h>
#endif /* BCM_NBUFF */

static void *
dhd_psta_pkt_alloc_copy(dhd_pub_t *dhdp, void *p)
{
	void *n;
	uint32 totlen;

	totlen = pkttotlen(dhdp->osh, p);

	DHD_INFO(("%s: Copying %d bytes\n", __FUNCTION__, totlen));

	if ((n = PKTGET(dhdp->osh, totlen, TRUE)) == NULL) {
		DHD_ERROR(("%s: PKTGET of length %d failed\n", __FUNCTION__, totlen));
		return NULL;
	}

	/* Transfer priority */
	PKTSETPRIO(n, PKTPRIO(p));

	/* Copy packet data to new buffer */
	pktcopy(dhdp->osh, p, 0, totlen, PKTDATA(dhdp->osh, n));

	return n;
}

/* Process ARP request and replies in tx and rx directions */
static int32
dhd_psta_arp_proc(dhd_pub_t *dhdp, void **p, uint8 *ah, uint8 **shost, bool tx, bool is_bcast)
{
	uint8 cli_mac[ETHER_ADDR_LEN];
	uint8 mod_mac[ETHER_ADDR_LEN];

	/* get cli_mac & mod_mac for arp processing */
	memcpy(cli_mac, *shost, ETHER_ADDR_LEN);
	memcpy(mod_mac, cli_mac, ETHER_ADDR_LEN);
	ETHER_SET_LOCALADDR(mod_mac);

	if (tx && is_bcast) {
		/* Since we are going to modify the address in arp header
		 * let's make a copy of the whole packet. Otherwise we will
		 * end up modifying arp header of the frame that is being
		 * broadcast to other bridged interfaces.
		 */
		void *n;
		uint8 *eh;
		uint16 ether_type, pull;
		bool fr_is_1x;

		eh = (uint8 *)PKTDATA(dhdp->osh, *p);
		bcm_psta_ether_type(dhdp->osh, eh, &ether_type, &pull, &fr_is_1x);

		if ((n = dhd_psta_pkt_alloc_copy(dhdp, *p)) == NULL) {
			return BCME_NOMEM;
		}

		ah = PKTDATA(dhdp->osh, n) + pull;
		PKTFREE(dhdp->osh, *p, TRUE);
		*p = n;
		*shost = PKTDATA(dhdp->osh, n) + ETHER_SRC_OFFSET;
	}

	/* Modify the source mac address in ARP header */
	if (bcm_psta_arp_proc(ah, cli_mac, mod_mac, tx) < 0)
		return BCME_ERROR;

#ifdef BCM_BLOG
#ifdef BCM_NBUFF
	if (IS_SKBUFF_PTR(*p))
#endif /* BCM_NBUFF */
	DHD_PKT_SET_SKB_SKIP_BLOG(*p);
#endif /* BCM_BLOG */

	return BCME_OK;
}

/* Process DHCP frames in tx and rx directions */
static int32
dhd_psta_dhcp_proc(void *dhd_pub, void *ifidx, void **p,
	uint8 *uh, uint8 *dhcph, uint16 dhcplen, uint16 port,
	uint8 **shost, bool tx, bool is_bcmc)
{
	dhd_pub_t *dhdp = (dhd_pub_t *)dhd_pub;
	uint8 cli_mac[ETHER_ADDR_LEN];
	uint8 mod_mac[ETHER_ADDR_LEN];

	/* if root-AP reply the DHCP Ack/Offer by broadcast, base on the dhcp h/w
	 * MAC to check if the MAC belongs to PSR and retrieve the cli_mac.
	 */
	if (!tx && dhd_get_psta_mode((dhd_pub_t *)dhdp) == DHD_MODE_PSR &&
		*(dhcph + DHCP_TYPE_OFFSET) == DHCP_TYPE_REPLY &&
		ETHER_ISBCAST(*shost)) {
		uint8 chaddr[ETHER_ADDR_LEN];
		char eabuf[ETHER_ADDR_STR_LEN];

		memcpy(chaddr, dhcph + DHCP_CHADDR_OFFSET, ETHER_ADDR_LEN);

		DHD_ERROR(("%s: recv dhcp-reply w/ bcast addr, dhcp-client h/w addr %s\n",
			__func__, bcm_ether_ntoa((struct ether_addr *)chaddr, eabuf)));

		if (ETHER_IS_LOCALADDR(chaddr)) {
			/* check if chaddr belongs to any virtual interface */
			if (dhd_get_ifp_by_mac(dhdp, chaddr)) {
				ETHER_CLR_LOCALADDR(chaddr);
				memcpy(cli_mac, chaddr, ETHER_ADDR_LEN);
			}
		}
	} else {
		/* directly get cli_mac from *shost */
		memcpy(cli_mac, *shost, ETHER_ADDR_LEN);
	}

	/* get mod_mac for dhcp processing */
	memcpy(mod_mac, cli_mac, ETHER_ADDR_LEN);
	ETHER_SET_LOCALADDR(mod_mac);

	DHD_INFO(("%s:%d port 0x%x\n", __FUNCTION__, __LINE__, port));

	/* Since we are going to modify the address in dhcp header
	 * let's make a copy of the whole packet. Otherwise we will
	 * end up modifying dhcp header of the frame that is being
	 * broadcast to other bridged interfaces.
	 */
	if (tx && is_bcmc) {
		void *n;
		uint8 *ih, proto;
		uint8 *eh;
		int32 hlen = 0;
		uint16 ether_type, pull;
		bool fr_is_1x;

		eh = (uint8 *)PKTDATA(dhdp->osh, *p);
		bcm_psta_ether_type(dhdp->osh, eh, &ether_type, &pull, &fr_is_1x);

		if ((n = dhd_psta_pkt_alloc_copy(dhdp, *p)) == NULL) {
			return BCME_NOMEM;
		}

		ih = PKTDATA(dhdp->osh, n) + pull;

		if (IP_VER(ih) == IP_VER_6) {
			proto = ih[IPV6_NEXT_HDR_OFFSET];
			if (IPV6_EXTHDR(proto)) {
				hlen = ipv6_exthdr_len(ih, &proto);
				if (hlen < 0) {
					PKTFREE(dhdp->osh, n, TRUE);
					return BCME_OK;
				}
			}
			hlen += IPV6_MIN_HLEN;
		} else
			hlen = IPV4_HLEN(ih);

		uh = ih + hlen;
		dhcph = uh + UDP_HDR_LEN;
		PKTFREE(dhdp->osh, *p, TRUE);
		*p = n;
		*shost = PKTDATA(dhdp->osh, n) + ETHER_SRC_OFFSET;
	}

	if (bcm_psta_dhcp_proc(port, uh, dhcph, dhcplen, cli_mac, mod_mac, tx, is_bcmc) < 0)
		return BCME_ERROR;

#ifdef BCM_BLOG
#ifdef BCM_NBUFF
	if (IS_SKBUFF_PTR(*p))
#endif /* BCM_NBUFF */
	DHD_PKT_SET_SKB_SKIP_BLOG(*p);
#endif /* BCM_BLOG */

	return BCME_OK;
}

/* Process ICMPv6 in tx and rx directions */
static int32
dhd_psta_icmp6_proc(dhd_pub_t *dhdp, void **p, uint8 *ih,
                    uint8 *icmph, uint16 icmplen, uint8 **shost, bool tx, bool is_bcmc)
{
	uint8 cli_mac[ETHER_ADDR_LEN];
	uint8 mod_mac[ETHER_ADDR_LEN];

	/* get cli_mac & mod_mac for icmp6 processing */
	memcpy(cli_mac, *shost, ETHER_ADDR_LEN);
	memcpy(mod_mac, cli_mac, ETHER_ADDR_LEN);
	ETHER_SET_LOCALADDR(mod_mac);

	/* Since we are going to modify the address in icmp option
	 * let's make a copy of the whole packet.
	 */
	if (tx && is_bcmc) {
		void *n;
		uint8 *eh;
		uint16 ether_type, pull;
		bool fr_is_1x;
		uint8 proto;
		int32 hlen = 0;

		eh = (uint8 *)PKTDATA(dhdp->osh, *p);
		bcm_psta_ether_type(dhdp->osh, eh, &ether_type, &pull, &fr_is_1x);

		if ((n = dhd_psta_pkt_alloc_copy(dhdp, *p)) == NULL) {
			return BCME_NOMEM;
		}

		ih = PKTDATA(dhdp->osh, n) + pull;

		if (IP_VER(ih) == IP_VER_6) {
			proto = ih[IPV6_NEXT_HDR_OFFSET];
			if (IPV6_EXTHDR(proto)) {
				hlen = ipv6_exthdr_len(ih, &proto);
				if (hlen < 0) {
					PKTFREE(dhdp->osh, n, TRUE);
					return BCME_OK;
				}
			}
			hlen += IPV6_MIN_HLEN;
		} else
			hlen = IPV4_HLEN(ih);

		icmph = ih + hlen;
		PKTFREE(dhdp->osh, *p, TRUE);
		*p = n;
		*shost = PKTDATA(dhdp->osh, n) + ETHER_SRC_OFFSET;
	}

	/* Modify the source mac address in icmp6 header */
	if (bcm_psta_icmp6_proc(ih, icmph, icmplen, cli_mac, mod_mac, tx) < 0)
		return BCME_ERROR;

#ifdef BCM_BLOG
#ifdef BCM_NBUFF
	if (IS_SKBUFF_PTR(*p))
#endif /* BCM_NBUFF */
	DHD_PKT_SET_SKB_SKIP_BLOG(*p);
#endif /* BCM_BLOG */

	return BCME_OK;
}

static int32
dhd_psta_proto_proc(dhd_pub_t *dhdp, uint32 ifidx, void **p, uint8 *eh, uint8 **shost, bool tx)
{
	uint8 *ih;
	uint16 ether_type, pull;
	bool bcmc;
	bool fr_is_1x;

	/* Ignore unknown frames */
	if (bcm_psta_ether_type(dhdp->osh, eh, &ether_type, &pull, &fr_is_1x) != BCME_OK)
		return BCME_OK;

	/* Send 1x frames as is. If primary is not authorized yet then wait for it
	 * before starting wpa handshake for secondary associations.
	 */
	if (fr_is_1x)
		return BCME_OK;

	ih = eh + pull;
	bcmc = ETHER_ISMULTI(eh + ETHER_DEST_OFFSET);

	/* NOTE:: We will not do Ethernet header modification here
	 * Dongle has enough information to handle this. We will process
	 * only process ARP and DHCP packets here since we need to modify
	 * the paylod for those packets.
	 * Rest of the processing will be managed in dongle itself
	 */
	switch (ether_type) {
	case ETHER_TYPE_IP:
		if (IP_VER(ih) == IP_VER_4) {
			if (IPV4_PROT(ih) == IP_PROT_UDP) {
				return bcm_psta_udp_proc(dhdp, &ifidx, p,
					ih, ih + IPV4_HLEN(ih),
					shost, tx, bcmc, dhd_psta_dhcp_proc);
			}
		}
		break;
	case ETHER_TYPE_IPV6:
		if (IP_VER(ih) == IP_VER_6) {
			uint8 proto = ih[IPV6_NEXT_HDR_OFFSET];
			int32 exthlen = 0;

			if (IPV6_EXTHDR(proto)) {
				exthlen = ipv6_exthdr_len(ih, &proto);
					if (exthlen < 0)
						return BCME_OK;
			}
			if (proto == IP_PROT_UDP)
				return bcm_psta_udp_proc(dhdp, &ifidx, p, ih,
					ih + IPV6_MIN_HLEN + exthlen,
					shost, tx, bcmc, dhd_psta_dhcp_proc);
			else if (proto == IP_PROT_ICMP6) {
				return dhd_psta_icmp6_proc(dhdp, p, ih,
						ih + IPV6_MIN_HLEN + exthlen,
						IPV6_PAYLOAD_LEN(ih) - exthlen,
						shost, tx, bcmc);
			}
		}
		break;
	case ETHER_TYPE_ARP:
		return dhd_psta_arp_proc(dhdp, p, ih, shost, tx, bcmc);
	default:
		DHD_INFO(("Unhandled ether type 0x%x\n", ether_type));
		return BCME_OK;
	}
	return BCME_OK;
}

/* Protocol processing functions for PSR operations */
int32
dhd_psta_proc(dhd_pub_t *dhdp, uint32 ifidx, void **pkt, bool tx)
{
	uint8 *shost = NULL;
	uint8 *eh;

	eh = (uint8 *)PKTDATA(dhdp->osh, *pkt);

	if (tx)
		shost = eh + ETHER_SRC_OFFSET;
	else
		shost = eh + ETHER_DEST_OFFSET;

	if (DHD_IF_ROLE_STA(dhdp, ifidx))
		return dhd_psta_proto_proc(dhdp, ifidx, pkt, eh, &shost, tx);

	return BCME_OK;
}
