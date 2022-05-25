/*
 * L2 Filter handling functions
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
 * $Id: dhd_l2_filter.c 793103 2020-11-13 06:08:06Z $
 *
 */
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <dngl_stats.h>
#include <dhd_dbg.h>
#include <dhd.h>

#include <bcmip.h>
#include <bcmipv6.h>
#include <bcmudp.h>
#include <bcmarp.h>
#include <bcmicmp.h>
#include <bcmproto.h>
#include <bcmdhcp.h>

#include <bcm_l2_filter.h>
#include <dhd_l2_filter.h>

static uint8 dhd_handle_proxyarp(dhd_pub_t *pub, int ifidx, frame_proto_t *fp, void **reply,
	uint8 *reply_to_bss, void *pktbuf, bool istx);
static uint8 dhd_handle_proxyarp_icmp6(dhd_pub_t *pub, int ifidx, frame_proto_t *fp, void **reply,
	uint8 *reply_to_bss, void *pktbuf, bool istx);
static uint8 dhd_handle_proxyarp_dhcp4(dhd_pub_t *pub, int ifidx, frame_proto_t *fp, bool istx);
extern int dhd_sendup(dhd_pub_t *dhdp, int ifidx, void *p);
extern bool dhd_sta_associated(dhd_pub_t *dhdp, uint32 bssidx, uint8 *mac);
extern void *dhd_get_ifp_arp_table_handle(dhd_pub_t *dhdp, uint32 bssidx);
extern bool dhd_parp_discard_is_enabled(dhd_pub_t *dhdp, uint32 ifidx);
extern bool dhd_parp_allnode_is_enabled(dhd_pub_t *dhdp, uint32 ifidx);

/* process ARP packets at recieving and transmitting time.
 * return values:
 *	PARP_TAKEN: ARP request has been converted to ARP response
 *	PARP_NOP:   No operation, pass the packet to corresponding next layer
 *	PARP_DROP:  free the packet resouce
 */
int
dhd_l2_filter_pkt_handle(dhd_pub_t *pub, int ifidx, void *pktbuf, bool istx)
{
	void *reply = NULL;
	uint8 reply_to_bss = 0;
	uint8 result = PARP_NOP;
	frame_proto_t fp;

	/* get frame type */
	if (hnd_frame_proto(PKTDATA(pub->osh, pktbuf), PKTLEN(pub->osh, pktbuf), &fp) != BCME_OK) {
		return BCME_ERROR;
	}

	if (fp.l3_t == FRAME_L3_ARP_H) {
		result = dhd_handle_proxyarp(pub, ifidx, &fp, &reply, &reply_to_bss, pktbuf, istx);
	} else if (fp.l4_t == FRAME_L4_ICMP6_H) {
		result = dhd_handle_proxyarp_icmp6(pub, ifidx, &fp, &reply, &reply_to_bss,
			pktbuf, istx);
	} else if (fp.l4_t == FRAME_L4_UDP_H) {
		result = dhd_handle_proxyarp_dhcp4(pub, ifidx, &fp, istx);
	}

	switch (result) {
		case PARP_TAKEN:
			if (reply != NULL) {
				if (reply_to_bss)
					dhd_sendpkt(pub, ifidx, reply);
				else
					dhd_sendup(pub, ifidx, reply);

				/* return OK to drop original packet */
				return BCME_OK;
			}
			break;
		case PARP_DROP:
			return BCME_OK;
			break;
		default:
			break;
	}

	/* return fail to let original packet keep traverse */
	return BCME_ERROR;
}

/* store the arp source /destination IP and MAC entries, based on ARP request
 * and ARP probe command. Create the ARP response packet, if destination entry
 * exist in ARP table, else if the parp_discard flag is set, packet will be
 * dropped.
 * return values :
 *	FRAME_TAKEN:	packet response has been created, response will be directed either to
 *			dhd_sendpkt or dhd_sendup(network stack)
 *	FRAME_DROP:	packet will be freed
 *	FRAME_NOP:	No operation
 */

static uint8
dhd_handle_proxyarp(dhd_pub_t *pub, int ifidx, frame_proto_t *fp, void **reply,
	uint8 *reply_to_bss, void *pktbuf, bool istx)
{
	parp_entry_t *entry;
	struct bcmarp *arp;
	uint16 op;
	arp_table_t *ptable;
	unsigned int time_sec;
	arp = (struct bcmarp *)fp->l3;
	op = ntoh16(arp->oper);

	/* basic ether addr check */
	if (ETHER_ISNULLADDR(arp->src_eth) || ETHER_ISBCAST(arp->src_eth) ||
	    ETHER_ISMULTI(arp->src_eth)) {
		return PARP_NOP;
	}

	if (op > ARP_OPC_REPLY) {
		DHD_ERROR(("%s: dhd%d: Invalid ARP operation(%d)\n",
			dhd_ifname(pub, ifidx), ifidx, op));
		return PARP_NOP;
	}

	ptable = (arp_table_t*)dhd_get_ifp_arp_table_handle(pub, ifidx);
	time_sec = (pub->tickcnt)/100;
	/* handle learning on ARP-REQ|ARP-REPLY|ARP-Announcement */
	if (!IPV4_ADDR_NULL(arp->src_ip) && !IPV4_ADDR_BCAST(arp->src_ip)) {
		entry = bcm_l2_filter_parp_findentry(ptable, arp->src_ip, IP_VER_4,
			TRUE, time_sec);
		if (entry == NULL) {
			bcm_l2_filter_parp_addentry(pub->osh, ptable,
				(struct ether_addr *)arp->src_eth, arp->src_ip, IP_VER_4,
				TRUE, time_sec);
		} else {
			/* overwrite the mac value corresponding to existing IP, else add */
			bcm_l2_filter_parp_modifyentry(ptable, (struct ether_addr *)arp->src_eth,
			arp->src_ip, IP_VER_4, TRUE, time_sec);
		}
	} else {
		/* only learning ARP-Probe(DAD) in receiving path */
		if (!istx && op == ARP_OPC_REQUEST) {
			entry = bcm_l2_filter_parp_findentry(ptable, arp->dst_ip,
				IP_VER_4, TRUE, time_sec);
			if (entry == NULL)
				entry = bcm_l2_filter_parp_findentry(ptable, arp->dst_ip,
					IP_VER_4, FALSE, time_sec);
			if (entry == NULL)
				bcm_l2_filter_parp_addentry(pub->osh, ptable,
					(struct ether_addr *)arp->src_eth, arp->dst_ip,
					IP_VER_4, FALSE, time_sec);
		}
	}

	/* perform candidate entry delete if some STA reply with ARP-Announcement */
	if (op == ARP_OPC_REPLY) {
		entry = bcm_l2_filter_parp_findentry(ptable, arp->src_ip, IP_VER_4,
			FALSE, time_sec);
		if (entry) {
			struct ether_addr ea;
			bcopy(&entry->ea, &ea, ETHER_ADDR_LEN);
			bcm_l2_filter_parp_delentry(pub->osh, ptable, &ea,
				arp->src_ip, IP_VER_4, FALSE);
		}
	}

	/* handle sending path */
	if (istx) {
		/* Drop ARP-Announcement(Gratuitous ARP) on sending path */
		if (bcmp(arp->src_ip, arp->dst_ip, IPV4_ADDR_LEN) == 0) {
			return PARP_DROP;
		}

		if (op == ARP_OPC_REQUEST) {
			struct bcmarp *arp_reply;
			uint16 pktlen = ETHER_HDR_LEN + ARP_DATA_LEN;
			bool snap = FALSE;

			if ((entry = bcm_l2_filter_parp_findentry(ptable,
				arp->dst_ip, IP_VER_4, TRUE, time_sec)) == NULL) {
				DHD_INFO(("No entry avaailable for %d.%d.%d.%d\n",
					arp->dst_ip[0], arp->dst_ip[1], arp->dst_ip[2],
					arp->dst_ip[3]));

				if (dhd_parp_discard_is_enabled(pub, ifidx))
					return PARP_DROP;
				else
					return PARP_NOP;
			}

			if (bcmp(arp->src_eth, (uint8 *)&entry->ea, ETHER_ADDR_LEN) == 0)
				return PARP_DROP;

			/* STA asking to some address not belong to BSS. Drop frame */
			if (!dhd_sta_associated(pub, ifidx, (uint8 *)&entry->ea.octet)) {
				return PARP_DROP;
			}

			/* determine dst is within bss or outside bss */
			if (dhd_sta_associated(pub, ifidx, arp->src_eth)) {
				/* dst is within bss, mark it */
				*reply_to_bss = 1;
			}

			if (fp->l2_t == FRAME_L2_SNAP_H || fp->l2_t == FRAME_L2_SNAPVLAN_H) {
				pktlen += SNAP_HDR_LEN + ETHER_TYPE_LEN;
				snap = TRUE;
			}

			/* Create 42-byte arp-reply data frame */
			if ((*reply = bcm_l2_filter_proxyarp_alloc_reply(pub->osh, pktlen,
				&entry->ea, (struct ether_addr *)arp->src_eth,
				ETHER_TYPE_ARP, snap, (void **)&arp_reply)) == NULL) {
				DHD_ERROR(("%s: dhd%d: failed to allocate reply frame. drop it\n",
					dhd_ifname(pub, ifidx), ifidx));
				return PARP_NOP;
			}

			/* copy first 6 bytes from ARP-Req to ARP-Reply(htype, ptype, hlen, plen) */
			bcopy(arp, arp_reply, ARP_OPC_OFFSET);
			hton16_ua_store(ARP_OPC_REPLY, &arp_reply->oper);
			bcopy(&entry->ea, arp_reply->src_eth, ETHER_ADDR_LEN);
			bcopy(&entry->ip.data, arp_reply->src_ip, IPV4_ADDR_LEN);
			bcopy(arp->src_eth, arp_reply->dst_eth, ETHER_ADDR_LEN);
			bcopy(arp->src_ip, arp_reply->dst_ip, IPV4_ADDR_LEN);
			return PARP_TAKEN;
		}
		/* ARP REPLY */
		else {
			entry = bcm_l2_filter_parp_findentry(ptable, arp->src_ip, IP_VER_4,
				TRUE, time_sec);

			/* If SMAC-SIP in reply frame is inconsistent
			 * to exist entry, drop frame(HS2-4.5.C)
			 */
			if (entry && bcmp(arp->src_eth, &entry->ea, ETHER_ADDR_LEN) != 0) {
				return PARP_DROP;
			}
		}

	}
	return PARP_NOP;
}

/* watchdog timer routine calls arp update routine per interface every 1 sec */
void
dhd_l2_filter_watchdog(dhd_pub_t *dhdp)
{
	int i;
	unsigned int time_sec;
	dhdp->l2_filter_cnt++;
	if (dhdp->l2_filter_cnt != BCM_ARP_TABLE_UPDATE_TIMEOUT)
	    return;
	for (i = 0; i < DHD_MAX_IFS; i++) {
		if (dhd_get_parp_status(dhdp, i)) {
			arp_table_t *ptable = (arp_table_t*)dhd_get_ifp_arp_table_handle(dhdp, i);
			time_sec = (dhdp->tickcnt)/100;	/* convert msec to sec */
			bcm_l2_filter_arp_table_update(dhdp->osh, ptable, FALSE, NULL,
				TRUE, time_sec);
		}
	}
	dhdp->l2_filter_cnt = 0;
}

/* store the arp source /destination IPV6 and MAC entries, based on ARP request
 * and ARP probe command. Create the ARP response packet, if destination entry
 * exist in ARP table, else if the parp_discard flag is set, packet will be
 * dropped.
 * return values :
 *	PARP_TAKEN:	packet response has been created, response will be directed either to
 *			dhd_sendpkt or dhd_sendup(network stack)
 *	PARP_DROP:	packet will be freed
 *	PARP_NOP:	No operation
 */
static uint8
dhd_handle_proxyarp_icmp6(dhd_pub_t *pub, int ifidx, frame_proto_t *fp, void **reply,
	uint8 *reply_to_bss, void *pktbuf, bool istx)
{
	struct ether_header *eh = (struct ether_header *)fp->l2;
	struct ipv6_hdr *ipv6_hdr = (struct ipv6_hdr *)fp->l3;
	struct bcm_nd_msg *nd_msg = (struct bcm_nd_msg *)(fp->l3 + sizeof(struct ipv6_hdr));
	parp_entry_t *entry;
	arp_table_t* ptable;
	unsigned int time_sec;
	struct ether_addr *entry_ea = NULL;
	uint8 *entry_ip = NULL;
	bool dad = FALSE;	/* Duplicate Address Detection */
	uint8 link_type = 0;
	bcm_tlv_t *link_addr = NULL;
	int16 ip6_icmp6_len = sizeof(struct ipv6_hdr) + sizeof(struct bcm_nd_msg);
	char ipbuf[64], eabuf[32];

	ptable = (arp_table_t*)dhd_get_ifp_arp_table_handle(pub, ifidx);
	time_sec = (pub->tickcnt)/100;
	/* basic check */
	if ((fp->l3_len < ip6_icmp6_len) ||
	    (ipv6_hdr->nexthdr != ICMPV6_HEADER_TYPE))
		return PARP_NOP;

	/* Neighbor Solicitation */
	if (nd_msg->icmph.icmp6_type == ICMPV6_PKT_TYPE_NS) {
	    link_type = ICMPV6_ND_OPT_TYPE_SRC_MAC;
	    if (IPV6_ADDR_NULL(ipv6_hdr->saddr.addr)) {
		/* ip6 src field is null, set offset to icmp6 target field */
		entry_ip = nd_msg->target.addr;
		dad = TRUE;
	    } else {
		/* ip6 src field not null, set offset to ip6 src field */
		entry_ip = ipv6_hdr->saddr.addr;
	    }
	}
	/* Neighbor Advertisement */
	else if (nd_msg->icmph.icmp6_type == ICMPV6_PKT_TYPE_NA) {
	    link_type = ICMPV6_ND_OPT_TYPE_TARGET_MAC;
	    entry_ip = nd_msg->target.addr;
	} else {
	    /* not an interesting frame, return without action */
	    return PARP_NOP;
	}

	/* if icmp6-option exists, retrieve layer2 link address from icmp6-option */
	if (fp->l3_len > ip6_icmp6_len) {
	    link_addr = parse_nd_options(fp->l3 + ip6_icmp6_len,
	    fp->l3_len - ip6_icmp6_len, link_type);
	    if (link_addr && link_addr->len == ICMPV6_ND_OPT_LEN_LINKADDR)
		entry_ea = (struct ether_addr *)&link_addr->data;
	}
	/* if no ea, retreive layer2 link address from ether header */
	if (entry_ea == NULL)
	    entry_ea = (struct ether_addr *)eh->ether_shost;

	/* basic ether addr check */
	if (ETHER_ISNULLADDR(eh->ether_shost) || ETHER_ISBCAST(eh->ether_shost) ||
	    ETHER_ISMULTI(eh->ether_shost)) {
		DHD_ERROR(("%s: dhd%d: Invalid Ether addr(%s) of icmp6 pkt\n",
			dhd_ifname(pub, ifidx), ifidx,
			bcm_ether_ntoa((struct ether_addr *)eh->ether_shost, eabuf)));
	    return PARP_NOP;
	}

	/* handle learning on Neighbor-Advertisement | Neighbor-Solicition(non-DAD) */
	if (nd_msg->icmph.icmp6_type == ICMPV6_PKT_TYPE_NA ||
	    (nd_msg->icmph.icmp6_type == ICMPV6_PKT_TYPE_NS && !dad)) {
	    entry = bcm_l2_filter_parp_findentry(ptable, entry_ip, IP_VER_6, TRUE, time_sec);
	    if (entry == NULL) {
		DHD_INFO(("%s: dhd%d: Add new parp_entry by ICMP6 %s %s\n",
		    dhd_ifname(pub, ifidx), ifidx, bcm_ether_ntoa(entry_ea, eabuf),
		    bcm_ipv6_ntoa((void *)entry_ip, ipbuf)));
		bcm_l2_filter_parp_addentry(pub->osh, ptable, entry_ea, entry_ip, IP_VER_6,
			TRUE, time_sec);
	    }
	} else {
	    /* only learning Neighbor-Solicition(DAD) in receiving path */
	    if (!istx) {
		entry = bcm_l2_filter_parp_findentry(ptable, entry_ip, IP_VER_6, TRUE, time_sec);
		if (entry == NULL)
			entry = bcm_l2_filter_parp_findentry(ptable, entry_ip, IP_VER_6,
				FALSE, time_sec);
		if (entry == NULL) {
			DHD_INFO(("%s: dhd%d: create candidate parp_entry by ICMP6 %s %s\n",
			dhd_ifname(pub, ifidx), ifidx, bcm_ether_ntoa((struct ether_addr *)entry_ea,
			    eabuf), bcm_ipv6_ntoa((void *)entry_ip, ipbuf)));
			bcm_l2_filter_parp_addentry(pub->osh, ptable, entry_ea, entry_ip, IP_VER_6,
				FALSE, time_sec);
		}
	    }
	}

	/* perform candidate entry delete if some STA reply with Neighbor-Advertisement */
	if (nd_msg->icmph.icmp6_type == ICMPV6_PKT_TYPE_NA) {
	    entry = bcm_l2_filter_parp_findentry(ptable, entry_ip, IP_VER_6, FALSE, time_sec);
	    if (entry) {
		struct ether_addr ea;
		bcopy(&entry->ea, &ea, ETHER_ADDR_LEN);
		DHD_INFO(("%s: dhd%d: withdraw candidate parp_entry IPv6 %s %s\n",
			dhd_ifname(pub, ifidx), ifidx, bcm_ether_ntoa(&ea, eabuf),
			bcm_ipv6_ntoa((void *)entry_ip, ipbuf)));
		bcm_l2_filter_parp_delentry(pub->osh, ptable, &ea, entry_ip, IP_VER_6, FALSE);
	    }
	}

	/* handle sending path */
	if (istx) {
		if (nd_msg->icmph.icmp6_type == ICMPV6_PKT_TYPE_NA) {
			/* Unsolicited Network Advertisment from STA, drop frame(HS2-4.5.F) */
			if (!(nd_msg->icmph.opt.nd_advt.router) &&
				(!nd_msg->icmph.opt.nd_advt.solicited)) {

				struct nd_msg_opt* nd_msg_opt_reply = (struct nd_msg_opt *)
					(((uint8 *)nd_msg) + sizeof(struct bcm_nd_msg));

				if (nd_msg_opt_reply != NULL) {
					if (nd_msg_opt_reply->type != ICMPV6_ND_OPT_TYPE_TARGET_MAC)
						return PARP_DROP;
				}
			}
		}

		/* try to reply if trying to send arp request frame */
		if (nd_msg->icmph.icmp6_type == ICMPV6_PKT_TYPE_NS) {
			struct ether_addr *reply_mac;
			struct ipv6_hdr *ipv6_reply;
			struct bcm_nd_msg *nd_msg_reply;
			struct nd_msg_opt *nd_msg_opt_reply;
			uint16 pktlen = ETHER_HDR_LEN + sizeof(struct ipv6_hdr) +
			    sizeof(struct bcm_nd_msg) + sizeof(struct nd_msg_opt);
			bool snap = FALSE;
			uint8 ipv6_mcast_allnode_ea[6] = {0x33, 0x33, 0x0, 0x0, 0x0, 0x1};
			uint8 ipv6_mcast_allnode_ip[16] = {0xff, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};

			if ((entry = bcm_l2_filter_parp_findentry(ptable, nd_msg->target.addr,
				IP_VER_6, TRUE, time_sec)) == NULL) {
			    if (dhd_parp_discard_is_enabled(pub, ifidx))
				return PARP_DROP;
			    else
				return PARP_NOP;
			}
			/* STA asking itself address. drop this frame */
			if (bcmp(entry_ea, (uint8 *)&entry->ea, ETHER_ADDR_LEN) == 0) {
			    return PARP_DROP;
			}

			/* STA asking to some address not belong to BSS.  Drop frame */
			if (!dhd_sta_associated(pub, ifidx, (uint8 *)&entry->ea.octet)) {
			    return PARP_DROP;
			}

			/* determine dst is within bss or outside bss */
			if (dhd_sta_associated(pub, ifidx, eh->ether_shost)) {
			    /* dst is within bss, mark it */
			    *reply_to_bss = 1;
			}

			if (fp->l2_t == FRAME_L2_SNAP_H || fp->l2_t == FRAME_L2_SNAPVLAN_H) {
			    pktlen += SNAP_HDR_LEN + ETHER_TYPE_LEN;
			    snap = TRUE;
			}

			/* Create 72 bytes neighbor advertisement data frame */
			/* determine l2 mac address is unicast or ipv6 mcast */
			if (dad) {
			    /* XXX NS DA should use ipv6_mcast_allnode_ea.
			     * however, wireshark can't interpreter encrypted frame
			     * with ipv6 mcast ea in HS2.0, to fix this on plugfest,
			     * we use ucast ea as NA DA.
			     */
			    if (dhd_parp_allnode_is_enabled(pub, ifidx))
				reply_mac = (struct ether_addr *)ipv6_mcast_allnode_ea;
			    else
				reply_mac = (struct ether_addr *)eh->ether_shost;
			} else {
			    reply_mac = entry_ea;
			}
			if ((*reply = bcm_l2_filter_proxyarp_alloc_reply(pub->osh, pktlen,
				&entry->ea, reply_mac, ETHER_TYPE_IPV6, snap,
				(void **)&ipv6_reply)) == NULL) {
				return PARP_NOP;
			}
			/* construct 40 bytes ipv6 header */
			bcopy((uint8 *)ipv6_hdr, (uint8 *)ipv6_reply, sizeof(struct ipv6_hdr));
			hton16_ua_store(sizeof(struct bcm_nd_msg) + sizeof(struct nd_msg_opt),
				&ipv6_reply->payload_len);
			ipv6_reply->hop_limit = 255;

			bcopy(nd_msg->target.addr, ipv6_reply->saddr.addr, IPV6_ADDR_LEN);
			/* if Duplicate address detected, filled all-node address as destination */
			if (dad)
			    bcopy(ipv6_mcast_allnode_ip, ipv6_reply->daddr.addr, IPV6_ADDR_LEN);
			else
			    bcopy(entry_ip, ipv6_reply->daddr.addr, IPV6_ADDR_LEN);

			/* Create 32 bytes icmpv6 NA frame body */
			nd_msg_reply = (struct bcm_nd_msg *)
			    (((uint8 *)ipv6_reply) + sizeof(struct ipv6_hdr));
			nd_msg_reply->icmph.icmp6_type = ICMPV6_PKT_TYPE_NA;
			nd_msg_reply->icmph.icmp6_code = 0;
			nd_msg_reply->icmph.opt.reserved = 0;
			nd_msg_reply->icmph.opt.nd_advt.override = 1;
			/* from observing win7 behavior, only non dad will set solicited flag */
			if (!dad)
			    nd_msg_reply->icmph.opt.nd_advt.solicited = 1;
			bcopy(nd_msg->target.addr, nd_msg_reply->target.addr, IPV6_ADDR_LEN);
			nd_msg_opt_reply = (struct nd_msg_opt *)
						(((uint8 *)nd_msg_reply) +
						 sizeof(struct bcm_nd_msg));

			nd_msg_opt_reply->type = ICMPV6_ND_OPT_TYPE_TARGET_MAC;
			nd_msg_opt_reply->len = ICMPV6_ND_OPT_LEN_LINKADDR;
			bcopy((uint8 *)&entry->ea, nd_msg_opt_reply->mac_addr, ETHER_ADDR_LEN);

			/* calculate ICMPv6 check sum */
			nd_msg_reply->icmph.icmp6_cksum = 0;
			nd_msg_reply->icmph.icmp6_cksum =
				calc_checksum(ipv6_reply->saddr.addr, ipv6_reply->daddr.addr,
				sizeof(struct bcm_nd_msg) + sizeof(struct nd_msg_opt),
				IP_PROT_ICMP6, (uint8 *)nd_msg_reply);

			return PARP_TAKEN;
		}
	}

	return PARP_NOP;
}

static uint8
dhd_handle_proxyarp_dhcp4(dhd_pub_t *pub, int ifidx, frame_proto_t *fp, bool istx)
{
	uint8 *dhcp;
	bcm_tlv_t *msg_type;
	uint16 opt_len, offset = DHCP_OPT_OFFSET;
	arp_table_t *ptable;
	unsigned int time_sec;
	uint8 smac_addr[ETHER_ADDR_LEN];
	uint8 cmac_addr[ETHER_ADDR_LEN];
	char eabuf[32];

	ptable = (arp_table_t*)dhd_get_ifp_arp_table_handle(pub, ifidx);
	time_sec = (pub->tickcnt)/100;
	dhcp = (uint8 *)(fp->l4 + UDP_HDR_LEN);

	/* First option must be magic cookie */
	if ((dhcp[offset + 0] != 0x63) || (dhcp[offset + 1] != 0x82) ||
	    (dhcp[offset + 2] != 0x53) || (dhcp[offset + 3] != 0x63))
		return PARP_NOP;

	/* skip 4 byte magic cookie and calculate dhcp opt len */
	offset += 4;
	opt_len = fp->l4_len - UDP_HDR_LEN - offset;

	bcm_l2_filter_parp_get_smac(ptable, smac_addr);
	bcm_l2_filter_parp_get_cmac(ptable, cmac_addr);
	/* sending path, process DHCP Ack frame only */
	if (istx) {
		msg_type = bcm_find_tlv(&dhcp[offset], opt_len, DHCP_OPT_MSGTYPE);
		if (msg_type == NULL || msg_type->data[0] != DHCP_OPT_MSGTYPE_ACK)
			return PARP_NOP;

		/* compared to DHCP Req client mac */
		if (bcmp((void*)cmac_addr, &dhcp[DHCP_CHADDR_OFFSET], ETHER_ADDR_LEN)) {
			bcopy(cmac_addr, eabuf, ETHER_ADDR_LEN);
			DHD_INFO(("%s: dhd%d: Unmatch DHCP Req Client MAC (%s)",
				dhd_ifname(pub, ifidx), ifidx, eabuf));
			DHD_INFO(("to DHCP Ack Client MAC(%s)\n",
				bcm_ether_ntoa((struct ether_addr *)&dhcp[DHCP_CHADDR_OFFSET],
				eabuf)));
			return PARP_NOP;
		}

		/* If client transmit DHCP Inform, server will response DHCP Ack with NULL YIADDR */
		if (IPV4_ADDR_NULL(&dhcp[DHCP_YIADDR_OFFSET]))
			return PARP_NOP;

		/* STA asking to some address not belong to BSS. Drop frame */
		if (!dhd_sta_associated(pub, ifidx, (uint8 *)smac_addr)) {
			parp_entry_t *entry = bcm_l2_filter_parp_findentry(ptable,
				&dhcp[DHCP_YIADDR_OFFSET], IP_VER_4, TRUE, time_sec);

			if (entry == NULL) {
				bcm_l2_filter_parp_addentry(pub->osh, ptable,
					(struct ether_addr*)smac_addr, &dhcp[DHCP_YIADDR_OFFSET],
					IP_VER_4, TRUE, time_sec);
			}
		}
	}
	else {	/* receiving path, process DHCP Req frame only */
		struct ether_header *eh = (struct ether_header *)fp->l2;

		msg_type = bcm_find_tlv(&dhcp[offset], opt_len, DHCP_OPT_MSGTYPE);
		if (msg_type == NULL || msg_type->data[0] != DHCP_OPT_MSGTYPE_REQ)
			return FRAME_NOP;

		/* basic ether addr check */
		if (ETHER_ISNULLADDR(&dhcp[DHCP_CHADDR_OFFSET]) ||
		    ETHER_ISBCAST(&dhcp[DHCP_CHADDR_OFFSET]) ||
		    ETHER_ISMULTI(&dhcp[DHCP_CHADDR_OFFSET]) ||
		    ETHER_ISNULLADDR(eh->ether_shost) ||
		    ETHER_ISBCAST(eh->ether_shost) ||
		    ETHER_ISMULTI(eh->ether_shost)) {
			DHD_ERROR(("%s: dhd%d: Invalid Ether addr(%s)",
				dhd_ifname(pub, ifidx),	ifidx,
				bcm_ether_ntoa((struct ether_addr *)eh->ether_shost, eabuf)));
			DHD_INFO(("(%s) of DHCP Req pkt\n",
				bcm_ether_ntoa((struct ether_addr *)&dhcp[DHCP_CHADDR_OFFSET],
				eabuf)));
			return PARP_NOP;
		}
		/*
		 * In URE mode, EA might different from Client Mac in BOOTP and SMAC in L2 hdr.
		 * We need to saved SMAC addr and Client Mac.  So that when receiving DHCP Ack,
		 * we can compare saved Client Mac and Client Mac in DHCP Ack frame.  If it's
		 * matched, then our target MAC would be saved L2 SMAC
		 */
		bcm_l2_filter_parp_set_smac(ptable, eh->ether_shost);
		bcm_l2_filter_parp_set_cmac(ptable, &dhcp[DHCP_CHADDR_OFFSET]);
	}

	return PARP_NOP;
}
