/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Allied Telesis Labs NZ
 * Chris Packham, <judge.packham@gmail.com>
 *
 * Copyright (C) 2022 YADRO
 * Viacheslav Mitrofanov <v.v.mitrofanov@yadro.com>
 */

#ifndef __NDISC_H__
#define __NDISC_H__

#include <ndisc.h>

/* struct nd_msg - ICMPv6 Neighbour Discovery message format */
struct nd_msg {
	struct icmp6hdr	icmph;
	struct in6_addr	target;
	__u8		opt[0];
};

/* struct rs_msg - ICMPv6 Router Solicitation message format */
struct rs_msg {
	struct icmp6hdr	icmph;
	__u8		opt[0];
};

/* struct ra_msg - ICMPv6 Router Advertisement message format */
struct ra_msg {
	struct icmp6hdr	icmph;
	__u32	reachable_time;
	__u32	retransmission_timer;
	__u8	opt[0];
};

/* struct echo_msg - ICMPv6 echo request/reply message format */
struct echo_msg {
	struct icmp6hdr	icmph;
	__u16		id;
	__u16		sequence;
};

/* Neigbour Discovery option types */
enum {
	__ND_OPT_PREFIX_INFO_END	= 0,
	ND_OPT_SOURCE_LL_ADDR		= 1,
	ND_OPT_TARGET_LL_ADDR		= 2,
	ND_OPT_PREFIX_INFO		= 3,
	ND_OPT_REDIRECT_HDR		= 4,
	ND_OPT_MTU			= 5,
	__ND_OPT_MAX
};

/* IPv6 destination address of packet waiting for ND */
extern struct in6_addr net_nd_sol_packet_ip6;
/* MAC destination address of packet waiting for ND */
extern uchar *net_nd_packet_mac;
/* pointer to packet waiting to be transmitted after ND is resolved */
extern uchar *net_nd_tx_packet;
/* size of packet waiting to be transmitted */
extern int net_nd_tx_packet_size;
/* the timer for ND resolution */
extern ulong net_nd_timer_start;
/* the number of requests we have sent so far */
extern int net_nd_try;

#ifdef CONFIG_IPV6
/**
 * ndisc_init() - Make initial steps for ND state machine.
 * Usually move variables into initial state.
 */
void ndisc_init(void);

/*
 * ip6_send_rs() - Send IPv6 Router Solicitation Message
 */
void ip6_send_rs(void);

/**
 * ndisc_receive() - Handle ND packet
 *
 * @et:		pointer to incoming packet
 * @ip6:	pointer to IPv6 header
 * @len:	incoming packet length
 * Return: 0 if handle successfully, -1 if unsupported/unknown ND packet type
 */
int ndisc_receive(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len);

/**
 * ndisc_request() - Send ND request
 */
void ndisc_request(void);

/**
 * ndisc_init() - Check ND response timeout
 *
 * Return: 0 if no timeout, -1 otherwise
 */
int ndisc_timeout_check(void);
bool validate_ra(struct ip6_hdr *ip6);
int process_ra(struct ip6_hdr *ip6, int len);
#else
static inline void ndisc_init(void)
{
}

static inline int
ndisc_receive(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len)
{
	return -1;
}

static inline void ndisc_request(void)
{
}

static inline int ndisc_timeout_check(void)
{
	return 0;
}

static inline void ip6_send_rs(void)
{
}

static inline bool validate_ra(struct ip6_hdr *ip6)
{
	return true;
}

static inline int process_ra(struct ip6_hdr *ip6, int len)
{
	return 0;
}
#endif

#endif /* __NDISC_H__ */
