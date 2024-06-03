/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

      Copyright (c) 2022 Broadcom 
      All Rights Reserved

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

#if defined BCM_ENET_FLCTL_LITE
#include <linux/etherdevice.h>
#ifdef PKTC
#include <osl.h>
#endif
#include <linux/gbpm.h>

#define ENET_FLCTL_PKT_PRIO_FAVOR	4  /* Favor Pkt Prio >= 4 : VI,VO */
#define ENET_FLCTL_EXHAUSTION_LO_PCNT	25
#define ENET_FLCTL_EXHAUSTION_HI_PCNT	10
#define ENET_FLCTL_ENABLE		1

unsigned int        flctl_exhaustion_lo;     /* avail < lo: admit ONLY favored */
unsigned int        flctl_exhaustion_hi;     /* avail < hi: admit none */
unsigned short      flctl_pkt_prio_favor;    /* favor pkt prio >= 4 */
unsigned int        flctl_enable;      /* Is HFC feature enabled/disabled */
unsigned int        count_flctl_pktdrops;

/**
 * =============================================================================
 * Section: Typedefs and Globals
 * =============================================================================
 */
#define ETHER_ADDR_LEN	6

/* ether types */
#define ETHER_TYPE_MIN		0x0600	/* Anything less than MIN is a length */
#define ETHER_TYPE_IP		0x0800	/* IP */
#define ETHER_TYPE_8021Q	0x8100	/* 802.1Q */
#define ETHER_TYPE_IPV6		0x86dd	/* IPv6 */

#define IPV4_TOS_OFFSET		1	/* type of service offset */
#define IPV4_TOS_DSCP_MASK	0xfc	/* DiffServ codepoint mask */
#define IPV4_TOS_DSCP_SHIFT	2	/* DiffServ codepoint shift */

#define IPV4_TOS(ipv4_body)	(((uint8_t *)(ipv4_body))[IPV4_TOS_OFFSET])

#define IPV4_TOS_PREC_MASK	0xe0	/* Historical precedence mask */
#define IPV4_TOS_PREC_SHIFT	5	/* Historical precedence shift */

#define VLAN_PRI_SHIFT		13	/* user priority */
#define VLAN_PRI_MASK		7	/* 3 bits of priority */

/* DSCP type definitions (RFC4594) */
/* AF1x: High-Throughput Data (RFC2597) */
#define DSCP_AF11	0x0A
#define DSCP_AF12	0x0C
#define DSCP_AF13	0x0E
/* AF2x: Low-Latency Data (RFC2597) */
#define DSCP_AF21	0x12
#define DSCP_AF22	0x14
#define DSCP_AF23	0x16
/* AF3x: Multimedia Streaming (RFC2597) */
#define DSCP_AF31	0x1A
#define DSCP_AF32	0x1C
#define DSCP_AF33	0x1E
/* EF: Telephony (RFC3246) */
#define DSCP_EF		0x2E

/* 802.1D priority defines from 802.1d.h */
#define	PRIO_8021D_NONE		2	/* None = - */
#define	PRIO_8021D_BK		1	/* BK - Background */
#define	PRIO_8021D_BE		0	/* BE - Best-effort */
#define	PRIO_8021D_EE		3	/* EE - Excellent-effort */
#define	PRIO_8021D_CL		4	/* CL - Controlled Load */
#define	PRIO_8021D_VI		5	/* Vi - Video */
#define	PRIO_8021D_VO		6	/* Vo - Voice */
#define	PRIO_8021D_NC		7	/* NC - Network Control */
#define	MAXPRIO			7	/* 0-7 */
#define NUMPRIO			(MAXPRIO + 1)

struct ether_header {
	uint8_t		ether_dhost[ETHER_ADDR_LEN];
	uint8_t		ether_shost[ETHER_ADDR_LEN];
	uint16_t	ether_type;
} __attribute__((packed));

struct ethervlan_header {
	uint8_t		ether_dhost[ETHER_ADDR_LEN];
	uint8_t		ether_shost[ETHER_ADDR_LEN];
	uint16_t	vlan_type;		/* 0x8100 */
	uint16_t	vlan_tag;		/* priority, cfi and vid */
	uint16_t	ether_type;
} __attribute__((packed));


#ifndef IL_BIGENDIAN
#define HTON16(val) 	((uint16_t)((((uint16_t)(val) & (uint16_t)0x00ffU) << 8) | \
			(((uint16_t)(val) & (uint16_t)0xff00U) >> 8)))
#define NTOH16(val) 	((uint16_t)((((uint16_t)(val) & (uint16_t)0x00ffU) << 8) | \
			(((uint16_t)(val) & (uint16_t)0xff00U) >> 8)))
#endif

/* IPV4 and IPV6 common */
#define IP_VER_OFFSET		0x0	/* offset to version field */
#define IP_VER_MASK		0xf0	/* version mask */
#define IP_VER_SHIFT		4	/* version shift */
#define IP_VER_4		4	/* version number for IPV4 */
#define IP_VER_6		6	/* version number for IPV6 */

#define IP_VER(ip_body) \
	((((uint8_t *)(ip_body))[IP_VER_OFFSET] & IP_VER_MASK) >> IP_VER_SHIFT)

#define IP_PROT_ICMP		0x1	/* ICMP protocol */
#define IP_PROT_IGMP		0x2	/* IGMP protocol */
#define IP_PROT_TCP		0x6	/* TCP protocol */
#define IP_PROT_UDP		0x11	/* UDP protocol type */
#define IP_PROT_GRE		0x2f	/* GRE protocol type */
#define IP_PROT_ICMP6		0x3a	/* ICMPv6 protocol type */

/* IPV4 field offsets */
#define IPV4_TOS_OFFSET		1	/* type of service offset */

/* IPV4 field decodes */
#define IPV4_VER_MASK		0xf0	/* IPV4 version mask */
#define IPV4_VER_SHIFT		4	/* IPV4 version shift */

#define IPV4_HLEN_MASK		0x0f	/* IPV4 header length mask */
#define IPV4_HLEN(ipv4_body)	(4 * (((uint8_t *)(ipv4_body))[IPV4_VER_HL_OFFSET] & IPV4_HLEN_MASK))

#define IPV4_ADDR_LEN		4	/* IPV4 address length */

#define IPV4_ADDR_NULL(a)	((((uint8_t *)(a))[0] | ((uint8_t *)(a))[1] | \
					((uint8_t *)(a))[2] | ((uint8_t *)(a))[3]) == 0)

#define IPV4_ADDR_BCAST(a)	((((uint8_t *)(a))[0] & ((uint8_t *)(a))[1] & \
i					((uint8_t *)(a))[2] & ((uint8_t *)(a))[3]) == 0xff)

#define	IPV4_TOS_DSCP_MASK	0xfc	/* DiffServ codepoint mask */
#define	IPV4_TOS_DSCP_SHIFT	2	/* DiffServ codepoint shift */

#define	IPV4_TOS(ipv4_body)	(((uint8_t *)(ipv4_body))[IPV4_TOS_OFFSET])

/* IPV6 field decodes */
#define IPV6_TRAFFIC_CLASS(ipv6_body) \
	(((((uint8_t *)(ipv6_body))[0] & 0x0f) << 4) | \
	 ((((uint8_t *)(ipv6_body))[1] & 0xf0) >> 4))

/* IPV4 TOS or IPV6 Traffic Classifier or 0 */
#define IP_TOS46(ip_body) \
	(IP_VER(ip_body) == IP_VER_4 ? IPV4_TOS(ip_body) : \
	 IP_VER(ip_body) == IP_VER_6 ? IPV6_TRAFFIC_CLASS(ip_body) : 0)

#define	IPV4_TOS(ipv4_body)	(((uint8_t *)(ipv4_body))[IPV4_TOS_OFFSET])


static int enet_pktsetprio(struct sk_buff *skb)
{
        struct ether_header *eh;
        struct ethervlan_header *evh;
        uint8_t *pktdata;
        int priority = 0;
        int vlan_prio, dscp_prio = 0;

        pktdata = (uint8_t *)skb->data;
        if (pktdata == NULL)
            return 0;

        eh = (struct ether_header *) pktdata;
        if (eh->ether_type == HTON16(ETHER_TYPE_8021Q)) {
                uint16_t vlan_tag;
                evh = (struct ethervlan_header *)pktdata;
                vlan_tag = NTOH16(evh->vlan_tag);
                vlan_prio = (int) (vlan_tag >> VLAN_PRI_SHIFT) & VLAN_PRI_MASK;

                if ((evh->ether_type == HTON16(ETHER_TYPE_IP)) ||
                        (evh->ether_type == HTON16(ETHER_TYPE_IPV6))) {
                        uint8_t *ip_body = pktdata + sizeof(struct ethervlan_header);
                        uint8_t tos_tc = IP_TOS46(ip_body);
                        dscp_prio = (int)(tos_tc >> IPV4_TOS_PREC_SHIFT);
                }

                /* DSCP priority gets precedence over 802.1P (vlan tag) */
                if (dscp_prio != 0) {
                        priority = dscp_prio;
                } else {
                        priority = vlan_prio;
                }
        } else if ((eh->ether_type == HTON16(ETHER_TYPE_IP)) ||
                (eh->ether_type == HTON16(ETHER_TYPE_IPV6))) {
                uint8_t *ip_body = pktdata + sizeof(struct ether_header);
                uint8_t tos_tc = IP_TOS46(ip_body);
                uint8_t dscp = tos_tc >> IPV4_TOS_DSCP_SHIFT;
                switch (dscp) {
                case DSCP_EF:
                        priority = PRIO_8021D_VO;
                        break;
                case DSCP_AF31:
                case DSCP_AF32:
                case DSCP_AF33:
                        priority = PRIO_8021D_CL;
                        break;
                case DSCP_AF21:
                case DSCP_AF22:
                case DSCP_AF23:
                case DSCP_AF11:
                case DSCP_AF12:
                case DSCP_AF13:
                        priority = PRIO_8021D_EE;
                        break;
                default:
                        priority = (int)(tos_tc >> IPV4_TOS_PREC_SHIFT);
                        break;
                }
        }
        PKTSETPRIO(skb, priority);

        return priority;
} /* enet_pktsetprio */

static inline bool enet_ucast_should_drop_skb(uint32_t avail, uint16_t pkt_prio)
{
    if (flctl_enable == 0)
        return false;

    if (avail <= flctl_exhaustion_hi)
        return true; // drop ALL
    if ((avail <= flctl_exhaustion_lo) && (pkt_prio < flctl_pkt_prio_favor))
        return true; // drop low prio

    return false;
}

int flctl_lite_handle(struct sk_buff *skb)
{
    int prio;
    uint32_t avail = gbpm_get_avail_bufs();

    __skb_push(skb, ETH_HLEN);
    prio = enet_pktsetprio(skb);
    __skb_pull(skb, ETH_HLEN);

    if (enet_ucast_should_drop_skb(avail, prio))
    {
        count_flctl_pktdrops++;
        dev_kfree_skb(skb);
        local_bh_enable();
        return 0;
    }

    return 1;
}

void flctl_init(void)
{
    uint32_t bpm_total;

    bpm_total = gbpm_get_total_bufs();
    flctl_exhaustion_lo = ((bpm_total * ENET_FLCTL_EXHAUSTION_LO_PCNT) / 100);
    flctl_exhaustion_hi = ((bpm_total * ENET_FLCTL_EXHAUSTION_HI_PCNT) / 100);
    flctl_pkt_prio_favor = ENET_FLCTL_PKT_PRIO_FAVOR;
    flctl_enable = ENET_FLCTL_ENABLE;
    count_flctl_pktdrops = 0;
}
#endif /* BCM_ENET_FLCTL_LITE */

