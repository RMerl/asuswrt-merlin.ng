// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Allied Telesis Labs NZ
 * Chris Packham, <judge.packham@gmail.com>
 *
 * Copyright (C) 2022 YADRO
 * Viacheslav Mitrofanov <v.v.mitrofanov@yadro.com>
 */

/* Neighbour Discovery for IPv6 */

#include <common.h>
#include <net.h>
#include <net6.h>
#include <ndisc.h>
#include <stdlib.h>
#include <linux/delay.h>

/* IPv6 destination address of packet waiting for ND */
struct in6_addr net_nd_sol_packet_ip6 = ZERO_IPV6_ADDR;
/* IPv6 address we are expecting ND advert from */
static struct in6_addr net_nd_rep_packet_ip6 = ZERO_IPV6_ADDR;
/* MAC destination address of packet waiting for ND */
uchar *net_nd_packet_mac;
/* pointer to packet waiting to be transmitted after ND is resolved */
uchar *net_nd_tx_packet;
static uchar net_nd_packet_buf[PKTSIZE_ALIGN + PKTALIGN];
/* size of packet waiting to be transmitted */
int net_nd_tx_packet_size;
/* the timer for ND resolution */
ulong net_nd_timer_start;
/* the number of requests we have sent so far */
int net_nd_try;
struct in6_addr all_routers = ALL_ROUTERS_MULT_ADDR;

#define MAX_RTR_SOLICITATIONS		3
/* The maximum time to delay sending the first router solicitation message. */
#define MAX_SOLICITATION_DELAY		1 // 1 second
/* The time to wait before sending the next router solicitation message. */
#define RTR_SOLICITATION_INTERVAL	4000 // 4 seconds

#define IP6_NDISC_OPT_SPACE(len) (((len) + 2 + 7) & ~7)

/**
 * ndisc_insert_option() - Insert an option into a neighbor discovery packet
 *
 * @opt:	pointer to the option element of the neighbor discovery packet
 * @type:	option type to insert
 * @data:	option data to insert
 * @len:	data length
 * Return: the number of bytes inserted (which may be >= len)
 */
static int ndisc_insert_option(__u8 *opt, int type, u8 *data, int len)
{
	int space = IP6_NDISC_OPT_SPACE(len);

	opt[0] = type;
	opt[1] = space >> 3;
	memcpy(&opt[2], data, len);
	len += 2;

	/* fill the remainder with 0 */
	if (space - len > 0)
		memset(&opt[len], '\0', space - len);

	return space;
}

/**
 * ndisc_extract_enetaddr() - Extract the Ethernet address from a ND packet
 *
 * Note that the link layer address could be anything but the only networking
 * media that u-boot supports is Ethernet so we assume we're extracting a 6
 * byte Ethernet MAC address.
 *
 * @ndisc:	pointer to ND packet
 * @enetaddr:	extracted MAC addr
 */
static void ndisc_extract_enetaddr(struct nd_msg *ndisc, uchar enetaddr[6])
{
	memcpy(enetaddr, &ndisc->opt[2], 6);
}

/**
 * ndisc_has_option() - Check if the ND packet has the specified option set
 *
 * @ip6:	pointer to IPv6 header
 * @type:	option type to check
 * Return: 1 if ND has that option, 0 therwise
 */
static int ndisc_has_option(struct ip6_hdr *ip6, __u8 type)
{
	struct nd_msg *ndisc = (struct nd_msg *)(((uchar *)ip6) + IP6_HDR_SIZE);

	if (ip6->payload_len <= sizeof(struct icmp6hdr))
		return 0;

	return ndisc->opt[0] == type;
}

static void ip6_send_ns(struct in6_addr *neigh_addr)
{
	struct in6_addr dst_adr;
	unsigned char enetaddr[6];
	struct nd_msg *msg;
	__u16 len;
	uchar *pkt;
	unsigned short csum;
	unsigned int pcsum;

	debug("sending neighbor solicitation for %pI6c our address %pI6c\n",
	      neigh_addr, &net_link_local_ip6);

	/* calculate src, dest IPv6 addr and dest Eth addr */
	ip6_make_snma(&dst_adr, neigh_addr);
	ip6_make_mult_ethdstaddr(enetaddr, &dst_adr);
	len = sizeof(struct icmp6hdr) + IN6ADDRSZ +
	    IP6_NDISC_OPT_SPACE(INETHADDRSZ);

	pkt = (uchar *)net_tx_packet;
	pkt += net_set_ether(pkt, enetaddr, PROT_IP6);
	pkt += ip6_add_hdr(pkt, &net_link_local_ip6, &dst_adr, PROT_ICMPV6,
			   IPV6_NDISC_HOPLIMIT, len);

	/* ICMPv6 - NS */
	msg = (struct nd_msg *)pkt;
	msg->icmph.icmp6_type = IPV6_NDISC_NEIGHBOUR_SOLICITATION;
	msg->icmph.icmp6_code = 0;
	memset(&msg->icmph.icmp6_cksum, 0, sizeof(__be16));
	memset(&msg->icmph.icmp6_unused, 0, sizeof(__be32));

	/* Set the target address and llsaddr option */
	net_copy_ip6(&msg->target, neigh_addr);
	ndisc_insert_option(msg->opt, ND_OPT_SOURCE_LL_ADDR, net_ethaddr,
			    INETHADDRSZ);

	/* checksum */
	pcsum = csum_partial((__u8 *)msg, len, 0);
	csum = csum_ipv6_magic(&net_link_local_ip6, &dst_adr,
			       len, PROT_ICMPV6, pcsum);
	msg->icmph.icmp6_cksum = csum;
	pkt += len;

	/* send it! */
	net_send_packet(net_tx_packet, (pkt - net_tx_packet));
}

/*
 * ip6_send_rs() - Send IPv6 Router Solicitation Message.
 *
 * A router solicitation is sent to discover a router. RS message creation is
 * based on RFC 4861 section 4.1. Router Solicitation Message Format.
 */
void ip6_send_rs(void)
{
	unsigned char enetaddr[6];
	struct rs_msg *msg;
	__u16 icmp_len;
	uchar *pkt;
	unsigned short csum;
	unsigned int pcsum;
	static unsigned int retry_count;

	if (!ip6_is_unspecified_addr(&net_gateway6) &&
	    net_prefix_length != 0) {
		net_set_state(NETLOOP_SUCCESS);
		return;
	} else if (retry_count >= MAX_RTR_SOLICITATIONS) {
		net_set_state(NETLOOP_FAIL);
		net_set_timeout_handler(0, NULL);
		retry_count = 0;
		return;
	}

	printf("ROUTER SOLICITATION %d\n", retry_count + 1);

	ip6_make_mult_ethdstaddr(enetaddr, &all_routers);
	/*
	 * ICMP length is the size of ICMP header (8) + one option (8) = 16.
	 * The option is 2 bytes of type and length + 6 bytes for MAC.
	 */
	icmp_len = sizeof(struct icmp6hdr) + IP6_NDISC_OPT_SPACE(INETHADDRSZ);

	pkt = (uchar *)net_tx_packet;
	pkt += net_set_ether(pkt, enetaddr, PROT_IP6);
	pkt += ip6_add_hdr(pkt, &net_link_local_ip6, &all_routers, PROT_ICMPV6,
			   IPV6_NDISC_HOPLIMIT, icmp_len);

	/* ICMPv6 - RS */
	msg = (struct rs_msg *)pkt;
	msg->icmph.icmp6_type = IPV6_NDISC_ROUTER_SOLICITATION;
	msg->icmph.icmp6_code = 0;
	memset(&msg->icmph.icmp6_cksum, 0, sizeof(__be16));
	memset(&msg->icmph.icmp6_unused, 0, sizeof(__be32));

	/* Set the llsaddr option */
	ndisc_insert_option(msg->opt, ND_OPT_SOURCE_LL_ADDR, net_ethaddr,
			    INETHADDRSZ);

	/* checksum */
	pcsum = csum_partial((__u8 *)msg, icmp_len, 0);
	csum = csum_ipv6_magic(&net_link_local_ip6, &all_routers,
			       icmp_len, PROT_ICMPV6, pcsum);
	msg->icmph.icmp6_cksum = csum;
	pkt += icmp_len;

	/* Wait up to 1 second if it is the first try to get the RA */
	if (retry_count == 0)
		udelay(((unsigned int)rand() % 1000000) * MAX_SOLICITATION_DELAY);

	/* send it! */
	net_send_packet(net_tx_packet, (pkt - net_tx_packet));

	retry_count++;
	net_set_timeout_handler(RTR_SOLICITATION_INTERVAL, ip6_send_rs);
}

static void
ip6_send_na(uchar *eth_dst_addr, struct in6_addr *neigh_addr,
	    struct in6_addr *target)
{
	struct nd_msg *msg;
	__u16 len;
	uchar *pkt;
	unsigned short csum;

	debug("sending neighbor advertisement for %pI6c to %pI6c (%pM)\n",
	      target, neigh_addr, eth_dst_addr);

	len = sizeof(struct icmp6hdr) + IN6ADDRSZ +
	    IP6_NDISC_OPT_SPACE(INETHADDRSZ);

	pkt = (uchar *)net_tx_packet;
	pkt += net_set_ether(pkt, eth_dst_addr, PROT_IP6);
	pkt += ip6_add_hdr(pkt, &net_link_local_ip6, neigh_addr,
			   PROT_ICMPV6, IPV6_NDISC_HOPLIMIT, len);

	/* ICMPv6 - NA */
	msg = (struct nd_msg *)pkt;
	msg->icmph.icmp6_type = IPV6_NDISC_NEIGHBOUR_ADVERTISEMENT;
	msg->icmph.icmp6_code = 0;
	memset(&msg->icmph.icmp6_cksum, 0, sizeof(__be16));
	memset(&msg->icmph.icmp6_unused, 0, sizeof(__be32));
	msg->icmph.icmp6_dataun.u_nd_advt.solicited = 1;
	msg->icmph.icmp6_dataun.u_nd_advt.override = 1;
	/* Set the target address and lltargetaddr option */
	net_copy_ip6(&msg->target, target);
	ndisc_insert_option(msg->opt, ND_OPT_TARGET_LL_ADDR, net_ethaddr,
			    INETHADDRSZ);

	/* checksum */
	csum = csum_ipv6_magic(&net_link_local_ip6,
			       neigh_addr, len, PROT_ICMPV6,
			       csum_partial((__u8 *)msg, len, 0));
	msg->icmph.icmp6_cksum = csum;
	pkt += len;

	/* send it! */
	net_send_packet(net_tx_packet, (pkt - net_tx_packet));
}

void ndisc_request(void)
{
	if (!ip6_addr_in_subnet(&net_ip6, &net_nd_sol_packet_ip6,
				net_prefix_length)) {
		if (ip6_is_unspecified_addr(&net_gateway6)) {
			puts("## Warning: gatewayip6 is needed but not set\n");
			net_nd_rep_packet_ip6 = net_nd_sol_packet_ip6;
		} else {
			net_nd_rep_packet_ip6 = net_gateway6;
		}
	} else {
		net_nd_rep_packet_ip6 = net_nd_sol_packet_ip6;
	}

	ip6_send_ns(&net_nd_rep_packet_ip6);
}

int ndisc_timeout_check(void)
{
	ulong t;

	if (ip6_is_unspecified_addr(&net_nd_sol_packet_ip6))
		return 0;

	t = get_timer(0);

	/* check for NDISC timeout */
	if ((t - net_nd_timer_start) > NDISC_TIMEOUT) {
		net_nd_try++;
		if (net_nd_try >= NDISC_TIMEOUT_COUNT) {
			puts("\nNeighbour discovery retry count exceeded; "
			     "starting again\n");
			net_nd_try = 0;
			net_set_state(NETLOOP_FAIL);
		} else {
			net_nd_timer_start = t;
			ndisc_request();
		}
	}
	return 1;
}

/*
 * ndisc_init() - Make initial steps for ND state machine.
 * Usually move variables into initial state.
 */
void ndisc_init(void)
{
	net_nd_packet_mac = NULL;
	net_nd_tx_packet = NULL;
	net_nd_sol_packet_ip6 = net_null_addr_ip6;
	net_nd_rep_packet_ip6 = net_null_addr_ip6;
	net_nd_tx_packet_size = 0;
	net_nd_tx_packet = &net_nd_packet_buf[0] + (PKTALIGN - 1);
	net_nd_tx_packet -= (ulong)net_nd_tx_packet % PKTALIGN;
}

/*
 * validate_ra() - Validate the router advertisement message.
 *
 * @ip6: Pointer to the router advertisement packet
 *
 * Check if the router advertisement message is valid. Conditions are
 * according to RFC 4861 section 6.1.2. Validation of Router Advertisement
 * Messages.
 *
 * Return: true if the message is valid and false if it is invalid.
 */
bool validate_ra(struct ip6_hdr *ip6)
{
	struct icmp6hdr *icmp = (struct icmp6hdr *)(ip6 + 1);

	/* ICMP length (derived from the IP length) should be 16 or more octets. */
	if (ip6->payload_len < 16)
		return false;

	/* Source IP Address should be a valid link-local address. */
	if ((ntohs(ip6->saddr.s6_addr16[0]) & IPV6_LINK_LOCAL_MASK) !=
	    IPV6_LINK_LOCAL_PREFIX)
		return false;

	/*
	 * The IP Hop Limit field should have a value of 255, i.e., the packet
	 * could not possibly have been forwarded by a router.
	 */
	if (ip6->hop_limit != 255)
		return false;

	/* ICMP checksum has already been checked in net_ip6_handler. */

	if (icmp->icmp6_code != 0)
		return false;

	return true;
}

/*
 * process_ra() - Process the router advertisement packet.
 *
 * @ip6: Pointer to the router advertisement packet
 * @len: Length of the router advertisement packet
 *
 * Process the received router advertisement message.
 * Although RFC 4861 requires retaining at least two router addresses, we only
 * keep one because of the U-Boot limitations and its goal of lightweight code.
 *
 * Return: 0 - RA is a default router and contains valid prefix information.
 * Non-zero - RA options are invalid or do not indicate it is a default router
 * or do not contain valid prefix information.
 */
int process_ra(struct ip6_hdr *ip6, int len)
{
	/* Pointer to the ICMP section of the packet */
	struct icmp6hdr *icmp = (struct icmp6hdr *)(ip6 + 1);
	struct ra_msg *msg = (struct ra_msg *)icmp;
	int remaining_option_len = len - IP6_HDR_SIZE - sizeof(struct ra_msg);
	unsigned short int option_len;	/* Length of each option */
	/* Pointer to the ICMPv6 message options */
	unsigned char *option = NULL;
	/* 8-bit identifier of the type of ICMPv6 option */
	unsigned char type = 0;
	struct icmp6_ra_prefix_info *prefix = NULL;

	if (len > ETH_MAX_MTU)
		return -EMSGSIZE;
	/* Ignore the packet if router lifetime is 0. */
	if (!icmp->icmp6_rt_lifetime)
		return -EOPNOTSUPP;

	/* Processing the options */
	option = msg->opt;
	while (remaining_option_len > 0) {
		/* The 2nd byte of the option is its length. */
		option_len = option[1];
		/* All included options should have a positive length. */
		if (option_len == 0)
			return -EINVAL;

		type = option[0];
		/* All option types except Prefix Information are ignored. */
		switch (type) {
		case ND_OPT_SOURCE_LL_ADDR:
		case ND_OPT_TARGET_LL_ADDR:
		case ND_OPT_REDIRECT_HDR:
		case ND_OPT_MTU:
			break;
		case ND_OPT_PREFIX_INFO:
			prefix = (struct icmp6_ra_prefix_info *)option;
			/* The link-local prefix 0xfe80::/10 is ignored. */
			if ((ntohs(prefix->prefix.s6_addr16[0]) &
			     IPV6_LINK_LOCAL_MASK) == IPV6_LINK_LOCAL_PREFIX)
				break;
			if (prefix->on_link && ntohl(prefix->valid_lifetime)) {
				net_prefix_length = prefix->prefix_len;
				net_gateway6 = ip6->saddr;
				return 0;
			}
			break;
		default:
			debug("Unknown IPv6 Neighbor Discovery Option 0x%x\n",
			      type);
		}

		option_len <<= 3; /* Option length is a multiple of 8. */
		remaining_option_len -= option_len;
		option += option_len;
	}

	return -EADDRNOTAVAIL;
}

int ndisc_receive(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len)
{
	struct icmp6hdr *icmp =
	    (struct icmp6hdr *)(((uchar *)ip6) + IP6_HDR_SIZE);
	struct nd_msg *ndisc = (struct nd_msg *)icmp;
	uchar neigh_eth_addr[6];
	int err = 0;	// The error code returned calling functions.

	switch (icmp->icmp6_type) {
	case IPV6_NDISC_NEIGHBOUR_SOLICITATION:
		debug("received neighbor solicitation for %pI6c from %pI6c\n",
		      &ndisc->target, &ip6->saddr);
		if (ip6_is_our_addr(&ndisc->target) &&
		    ndisc_has_option(ip6, ND_OPT_SOURCE_LL_ADDR)) {
			ndisc_extract_enetaddr(ndisc, neigh_eth_addr);
			ip6_send_na(neigh_eth_addr, &ip6->saddr,
				    &ndisc->target);
		}
		break;

	case IPV6_NDISC_NEIGHBOUR_ADVERTISEMENT:
		/* are we waiting for a reply ? */
		if (ip6_is_unspecified_addr(&net_nd_sol_packet_ip6))
			break;

		if ((memcmp(&ndisc->target, &net_nd_rep_packet_ip6,
			    sizeof(struct in6_addr)) == 0) &&
		    ndisc_has_option(ip6, ND_OPT_TARGET_LL_ADDR)) {
			ndisc_extract_enetaddr(ndisc, neigh_eth_addr);

			/* save address for later use */
			if (!net_nd_packet_mac)
				net_nd_packet_mac = neigh_eth_addr;

			/* modify header, and transmit it */
			memcpy(((struct ethernet_hdr *)net_nd_tx_packet)->et_dest,
			       neigh_eth_addr, 6);

			net_send_packet(net_nd_tx_packet,
					net_nd_tx_packet_size);

			/* no ND request pending now */
			net_nd_sol_packet_ip6 = net_null_addr_ip6;
			net_nd_tx_packet_size = 0;
			net_nd_packet_mac = NULL;
		}
		break;
	case IPV6_NDISC_ROUTER_SOLICITATION:
		break;
	case IPV6_NDISC_ROUTER_ADVERTISEMENT:
		debug("Received router advertisement for %pI6c from %pI6c\n",
		      &ip6->daddr, &ip6->saddr);
		/*
		 * If gateway and prefix are set, the RA packet is ignored. The
		 * reason is that the U-Boot code is supposed to be as compact
		 * as possible and does not need to take care of multiple
		 * routers. In addition to that, U-Boot does not want to handle
		 * scenarios like a router setting its lifetime to zero to
		 * indicate it is not routing anymore. U-Boot program has a
		 * short life when the system boots up and does not need such
		 * sophistication.
		 */
		if (!ip6_is_unspecified_addr(&net_gateway6) &&
		    net_prefix_length != 0) {
			break;
		}
		if (!validate_ra(ip6)) {
			debug("Invalid router advertisement message.\n");
			break;
		}
		err = process_ra(ip6, len);
		if (err)
			debug("Ignored router advertisement. Error: %d\n", err);
		else
			printf("Set gatewayip6: %pI6c, prefix_length: %d\n",
			       &net_gateway6, net_prefix_length);
		break;
	default:
		debug("Unexpected ICMPv6 type 0x%x\n", icmp->icmp6_type);
		return -1;
	}

	return 0;
}
