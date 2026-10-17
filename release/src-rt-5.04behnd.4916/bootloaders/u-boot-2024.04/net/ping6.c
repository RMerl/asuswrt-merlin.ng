// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Allied Telesis Labs NZ
 * Chris Packham, <judge.packham@gmail.com>
 *
 * Copyright (C) 2022 YADRO
 * Viacheslav Mitrofanov <v.v.mitrofanov@yadro.com>
 */

/* Simple ping6 implementation */

#include <common.h>
#include <net.h>
#include <net6.h>
#include "ndisc.h"

static ushort seq_no;

/* the ipv6 address to ping */
struct in6_addr net_ping_ip6;

int
ip6_make_ping(uchar *eth_dst_addr, struct in6_addr *neigh_addr, uchar *pkt)
{
	struct echo_msg *msg;
	u16 len;
	u16 csum_p;
	uchar *pkt_old = pkt;

	len = sizeof(struct echo_msg);

	pkt += net_set_ether(pkt, eth_dst_addr, PROT_IP6);
	pkt += ip6_add_hdr(pkt, &net_ip6, neigh_addr, PROT_ICMPV6,
			   IPV6_NDISC_HOPLIMIT, len);

	/* ICMPv6 - Echo */
	msg = (struct echo_msg *)pkt;
	msg->icmph.icmp6_type = IPV6_ICMP_ECHO_REQUEST;
	msg->icmph.icmp6_code = 0;
	msg->icmph.icmp6_cksum = 0;
	msg->icmph.icmp6_identifier = 0;
	msg->icmph.icmp6_sequence = htons(seq_no++);
	msg->id = msg->icmph.icmp6_identifier;	/* these seem redundant */
	msg->sequence = msg->icmph.icmp6_sequence;

	/* checksum */
	csum_p = csum_partial((u8 *)msg, len, 0);
	msg->icmph.icmp6_cksum = csum_ipv6_magic(&net_ip6, neigh_addr, len,
						 PROT_ICMPV6, csum_p);

	pkt += len;

	return pkt - pkt_old;
}

int ping6_send(void)
{
	uchar *pkt;
	static uchar mac[6];

	/* always send neighbor solicit */

	memcpy(mac, net_null_ethaddr, 6);

	net_nd_sol_packet_ip6 = net_ping_ip6;
	net_nd_packet_mac = mac;

	pkt = net_nd_tx_packet;
	pkt += ip6_make_ping(mac, &net_ping_ip6, pkt);

	/* size of the waiting packet */
	net_nd_tx_packet_size = (pkt - net_nd_tx_packet);

	/* and do the ARP request */
	net_nd_try = 1;
	net_nd_timer_start = get_timer(0);
	ndisc_request();
	return 1;		/* waiting */
}

static void ping6_timeout(void)
{
	eth_halt();
	net_set_state(NETLOOP_FAIL);	/* we did not get the reply */
}

void ping6_start(void)
{
	printf("Using %s device\n", eth_get_name());
	net_set_timeout_handler(10000UL, ping6_timeout);

	ping6_send();
}

int ping6_receive(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len)
{
	struct icmp6hdr *icmp =
	    (struct icmp6hdr *)(((uchar *)ip6) + IP6_HDR_SIZE);
	struct in6_addr src_ip;

	switch (icmp->icmp6_type) {
	case IPV6_ICMP_ECHO_REPLY:
		src_ip = ip6->saddr;
		if (memcmp(&net_ping_ip6, &src_ip, sizeof(struct in6_addr)))
			return -EINVAL;
		net_set_state(NETLOOP_SUCCESS);
		break;
	case IPV6_ICMP_ECHO_REQUEST:
		/* ignore for now.... */
		debug("Got ICMPv6 ECHO REQUEST from %pI6c\n", &ip6->saddr);
		return -EINVAL;
	default:
		debug("Unexpected ICMPv6 type 0x%x\n", icmp->icmp6_type);
		return -EINVAL;
	}

	return 0;
}
