/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Allied Telesis Labs NZ
 * Chris Packham, <judge.packham@gmail.com>
 *
 * Copyright (C) 2022 YADRO
 * Viacheslav Mitrofanov <v.v.mitrofanov@yadro.com>
 */

#ifndef __NET6_H__
#define __NET6_H__

#include <net.h>
#include <linux/ctype.h>
#include <linux/errno.h>

/* struct in6_addr - 128 bits long IPv6 address */
struct in6_addr {
	union {
		u8	u6_addr8[16];
		__be16	u6_addr16[8];
		__be32	u6_addr32[4];
	} in6_u;

#define s6_addr		in6_u.u6_addr8
#define s6_addr16	in6_u.u6_addr16
#define s6_addr32	in6_u.u6_addr32
} __packed;

#define IN6ADDRSZ	sizeof(struct in6_addr)
#define INETHADDRSZ	sizeof(net_ethaddr)

#define PROT_IP6	0x86DD	/* IPv6 protocol */
#define PROT_ICMPV6	58	/* ICMPv6 protocol*/

#define IPV6_ADDRSCOPE_INTF	0x01
#define IPV6_ADDRSCOPE_LINK	0x02
#define IPV6_ADDRSCOPE_AMDIN	0x04
#define IPV6_ADDRSCOPE_SITE	0x05
#define IPV6_ADDRSCOPE_ORG	0x08
#define IPV6_ADDRSCOPE_GLOBAL	0x0E

#define USE_IP6_CMD_PARAM	"-ipv6"

/**
 * struct ipv6hdr - Internet Protocol V6 (IPv6) header.
 *
 * IPv6 packet header as defined in RFC 2460.
 */
struct ip6_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u8	priority:4,
		version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	u8	version:4,
		priority:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	u8		flow_lbl[3];
	__be16		payload_len;
	u8		nexthdr;
	u8		hop_limit;
	struct in6_addr	saddr;
	struct in6_addr	daddr;
} __packed;
#define IP6_HDR_SIZE (sizeof(struct ip6_hdr))

/* struct udp_hdr - User Datagram Protocol header */
struct udp_hdr {
	u16		udp_src;	/* UDP source port		*/
	u16		udp_dst;	/* UDP destination port		*/
	u16		udp_len;	/* Length of UDP packet		*/
	u16		udp_xsum;	/* Checksum			*/
} __packed;

/*
 * Handy for static initialisations of struct in6_addr, atlhough the
 * c99 '= { 0 }' idiom might work depending on you compiler.
 */
#define ZERO_IPV6_ADDR { { { 0x00, 0x00, 0x00, 0x00, \
			  0x00, 0x00, 0x00, 0x00, \
			  0x00, 0x00, 0x00, 0x00, \
			  0x00, 0x00, 0x00, 0x00 } } }
/*
 * All-routers multicast address is the link-local scope address to reach all
 * routers.
 */
#define ALL_ROUTERS_MULT_ADDR { { { 0xFF, 0x02, 0x00, 0x00, \
				  0x00, 0x00, 0x00, 0x00, \
				  0x00, 0x00, 0x00, 0x00, \
				  0x00, 0x00, 0x00, 0x02 } } }

#define IPV6_LINK_LOCAL_PREFIX	0xfe80
#define IPV6_LINK_LOCAL_MASK	0xffb0 /* The first 10-bit of address mask. */

/* hop limit for neighbour discovery packets */
#define IPV6_NDISC_HOPLIMIT             255
#define NDISC_TIMEOUT			5000UL
#define NDISC_TIMEOUT_COUNT             3

/* struct icmp6hdr - Internet Control Message Protocol header for IPV6 */
struct icmp6hdr {
	u8	icmp6_type;
#define IPV6_ICMP_ECHO_REQUEST			128
#define IPV6_ICMP_ECHO_REPLY			129
#define IPV6_NDISC_ROUTER_SOLICITATION		133
#define IPV6_NDISC_ROUTER_ADVERTISEMENT		134
#define IPV6_NDISC_NEIGHBOUR_SOLICITATION	135
#define IPV6_NDISC_NEIGHBOUR_ADVERTISEMENT	136
#define IPV6_NDISC_REDIRECT			137
	u8	icmp6_code;
	__be16	icmp6_cksum;

	/* ICMPv6 data */
	union {
		__be32	un_data32[1];
		__be16	un_data16[2];
		u8	un_data8[4];

		/* struct icmpv6_echo - echo request/reply message format */
		struct icmpv6_echo {
			__be16		identifier;
			__be16		sequence;
		} u_echo;

		/* struct icmpv6_nd_advt - Neighbor Advertisement format */
		struct icmpv6_nd_advt {
#if defined(__LITTLE_ENDIAN_BITFIELD)
			__be32		reserved:5,
					override:1,
					solicited:1,
					router:1,
					reserved2:24;
#elif defined(__BIG_ENDIAN_BITFIELD)
			__be32		router:1,
					solicited:1,
					override:1,
					reserved:29;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
		} u_nd_advt;

		/* struct icmpv6_nd_ra - Router Advertisement format */
		struct icmpv6_nd_ra {
			u8		hop_limit;
#if defined(__LITTLE_ENDIAN_BITFIELD)
			u8		reserved:6,
					other:1,
					managed:1;

#elif defined(__BIG_ENDIAN_BITFIELD)
			u8		managed:1,
					other:1,
					reserved:6;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
			__be16		rt_lifetime;
		} u_nd_ra;
	} icmp6_dataun;
#define icmp6_identifier	icmp6_dataun.u_echo.identifier
#define icmp6_sequence		icmp6_dataun.u_echo.sequence
#define icmp6_pointer		icmp6_dataun.un_data32[0]
#define icmp6_mtu		icmp6_dataun.un_data32[0]
#define icmp6_unused		icmp6_dataun.un_data32[0]
#define icmp6_maxdelay		icmp6_dataun.un_data16[0]
#define icmp6_router		icmp6_dataun.u_nd_advt.router
#define icmp6_solicited		icmp6_dataun.u_nd_advt.solicited
#define icmp6_override		icmp6_dataun.u_nd_advt.override
#define icmp6_ndiscreserved	icmp6_dataun.u_nd_advt.reserved
#define icmp6_hop_limit		icmp6_dataun.u_nd_ra.hop_limit
#define icmp6_addrconf_managed	icmp6_dataun.u_nd_ra.managed
#define icmp6_addrconf_other	icmp6_dataun.u_nd_ra.other
#define icmp6_rt_lifetime	icmp6_dataun.u_nd_ra.rt_lifetime
} __packed;

/*
 * struct icmp6_ra_prefix_info - Prefix Information option of the ICMPv6 message
 * The Prefix Information option provides hosts with on-link prefixes and
 * prefixes for Address Autoconfiguration. Refer to RFC 4861 for more info.
 */
struct icmp6_ra_prefix_info {
	u8	type;		/* Type is 3 for Prefix Information. */
	u8	len;		/* Len is 4 for Prefix Information. */
	/* The number of leading bits in the Prefix that are valid. */
	u8	prefix_len;
	u8	reserved1:6,	/* MUST be ignored by the receiver. */
		aac:1,		/* autonomous address-configuration flag */
	/* Indicates that this prefix can be used for on-link determination. */
		on_link:1;
	/*
	 * The length of time in seconds that the prefix is valid for the
	 * purpose of on-link determination.
	 */
	__be32	valid_lifetime;
	/* The length of time addresses remain preferred. */
	__be32	preferred_lifetime;
	__be32	reserved2;	/* MUST be ignored by the receiver. */
	/*
	 * Prefix is an IP address or a prefix of an IP address. The Prefix
	 * Length field contains the number of valid leading bits in the prefix.
	 * The bits in the prefix after the prefix length are reserved and MUST
	 * be initialized to zero by the sender and ignored by the receiver.
	 */
	struct in6_addr prefix;
} __packed;

extern struct in6_addr const net_null_addr_ip6;	/* NULL IPv6 address */
extern struct in6_addr net_gateway6;	/* Our gateways IPv6 address */
extern struct in6_addr net_ip6;	/* Our IPv6 addr (0 = unknown) */
extern struct in6_addr net_link_local_ip6;	/* Our link local IPv6 addr */
extern u32 net_prefix_length;	/* Our prefixlength (0 = unknown) */
extern struct in6_addr net_server_ip6;	/* Server IPv6 addr (0 = unknown) */
extern struct in6_addr net_ping_ip6; /* the ipv6 address to ping */
extern bool use_ip6;

#if IS_ENABLED(CONFIG_IPV6)
/**
 * string_to_ip6() - Convert IPv6 string addr to inner IPV6 addr format
 *
 * Examples of valid strings:
 *	2001:db8::0:1234:1
 *	2001:0db8:0000:0000:0000:0000:1234:0001
 *	::1
 *	::ffff:192.168.1.1
 *
 * Examples of invalid strings
 *	2001:db8::0::0          (:: can only appear once)
 *	2001:db8:192.168.1.1::1 (v4 part can only appear at the end)
 *	192.168.1.1             (we don't implicity map v4)
 *
 * @s:		IPv6 string addr format
 * @len:	IPv6 string addr length
 * @addr:	converted IPv6 addr
 * Return: 0 if conversion successful, -EINVAL if fail
 */
int string_to_ip6(const char *s, size_t len, struct in6_addr *addr);

/**
 * ip6_is_unspecified_addr() - Check if IPv6 addr is not set i.e. is zero
 *
 * @addr:	IPv6 addr
 * Return:  0 if addr is not set, -1 if is set
 */
int ip6_is_unspecified_addr(struct in6_addr *addr);

/**
 * ip6_is_our_addr() - Check if IPv6 addr belongs to our host addr
 *
 * We have 2 addresses that we should respond to. A link local address and a
 * global address. This returns true if the specified address matches either
 * of these.
 *
 * @addr:	addr to check
 * Return: 0 if addr is our, -1 otherwise
 */
int ip6_is_our_addr(struct in6_addr *addr);

/**
 * ip6_addr_in_subnet() - Check if two IPv6 addresses are in the same subnet
 *
 * @our_addr:		first IPv6 addr
 * @neigh_addr:		second IPv6 addr
 * @prefix_length:	network mask length
 * Return: 0 if two addresses in the same subnet, -1 otherwise
 */
int ip6_addr_in_subnet(struct in6_addr *our_addr, struct in6_addr *neigh_addr,
		       u32 prefix_length);

/**
 * ip6_make_lladd() - rMake up IPv6 Link Local address
 *
 * @lladdr:	formed IPv6 Link Local address
 * @enetaddr:	MAC addr of a device
 */
void ip6_make_lladdr(struct in6_addr *lladr, unsigned char const enetaddr[6]);

/**
 * ip6_make_snma() - aMake up Solicited Node Multicast Address from IPv6 addr
 *
 * @mcast_addr:	formed SNMA addr
 * @ip6_addr:	base IPv6 addr
 */
void ip6_make_snma(struct in6_addr *mcast_addr, struct in6_addr *ip6_addr);

/**
 * ip6_make_mult_ethdstaddr() - Make up IPv6 multicast addr
 *
 * @enetaddr:	MAC addr of a device
 * @mcast_addr:	formed IPv6 multicast addr
 */
void ip6_make_mult_ethdstaddr(unsigned char enetaddr[6],
			      struct in6_addr *mcast_addr);

/**
 * csum_partial() - Compute an internet checksum
 *
 * @buff:	buffer to be checksummed
 * @len:	length of buffer
 * @sum:	initial sum to be added in
 * Return: internet checksum of the buffer
 */
unsigned int csum_partial(const unsigned char *buff, int len, unsigned int sum);

/**
 * csum_ipv6_magic() - Compute checksum of IPv6 "psuedo-header" per RFC2460 section 8.1
 *
 * @saddr:	source IPv6 addr
 * @daddr:	destination IPv6 add
 * @len:	data length to be checksummed
 * @proto:	IPv6 above protocol code
 * @csum:	upper layer checksum
 * Return: computed checksum
 */
unsigned short int csum_ipv6_magic(struct in6_addr *saddr,
				   struct in6_addr *daddr, u16 len,
				   unsigned short proto, unsigned int csum);

/**
 * ip6_add_hdr() - Make up IPv6 header
 *
 * @xip:	pointer to IPv6 header to be formed
 * @src:	source IPv6 addr
 * @dest:	destination IPv6 addr
 * @nextheader:	next header type
 * @hoplimit:	hop limit
 * @payload_len: payload length
 * Return: IPv6 header length
 */
int ip6_add_hdr(uchar *xip, struct in6_addr *src, struct in6_addr *dest,
		int nextheader, int hoplimit, int payload_len);

/**
 * net_send_udp_packet6() - Make up UDP packet and send it
 *
 * @ether:	destination MAC addr
 * @dest:	destination IPv6 addr
 * @dport:	destination port
 * @sport:	source port
 * @len:	UDP packet length
 * Return: 0 if send successfully, -1 otherwise
 */
int net_send_udp_packet6(uchar *ether, struct in6_addr *dest, int dport,
			 int sport, int len);

/**
 * net_ip6_handler() - Handle IPv6 packet
 *
 * @et:		pointer to the beginning of the packet
 * @ip6:	pointer to the beginning of IPv6 protocol
 * @len:	incoming packet len
 * Return: 0 if handle packet successfully, -EINVAL in case of invalid protocol
 */
int net_ip6_handler(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len);

/**
 * net_copy_ip6() - Copy IPv6 addr
 *
 * @to:		destination IPv6 addr
 * @from:	source IPv6 addr
 */
static inline void net_copy_ip6(void *to, const void *from)
{
	memcpy((void *)to, from, sizeof(struct in6_addr));
}
#else
static inline int
string_to_ip6(const char *s, size_t len, struct in6_addr *addr)
{
	return -EINVAL;
}

static inline int ip6_is_unspecified_addr(struct in6_addr *addr)
{
	return -1;
}

static inline int ip6_is_our_addr(struct in6_addr *addr)
{
	return -1;
}

static inline int
ip6_addr_in_subnet(struct in6_addr *our_addr, struct in6_addr *neigh_addr,
		   u32 prefix_length)
{
	return -1;
}

static inline void
ip6_make_lladdr(struct in6_addr *lladdr, unsigned char const enetaddr[6])
{
}

static inline void
ip6_make_snma(struct in6_addr *mcast_addr, struct in6_addr *ip6_addr)
{
}

static inline void
ip6_make_mult_ethdstaddr(unsigned char enetaddr[6],
			 struct in6_addr *mcast_addr)
{
}

static inline unsigned int
csum_partial(const unsigned char *buff, int len, unsigned int sum)
{
	return 0;
}

static inline unsigned short
csum_ipv6_magic(struct in6_addr *saddr,
		struct in6_addr *daddr, u16 len,
		unsigned short proto, unsigned int csum)
{
	return 0;
}

static inline unsigned int
ip6_add_hdr(uchar *xip, struct in6_addr *src, struct in6_addr *dest,
	    int nextheader, int hoplimit, int payload_len)
{
	return 0;
}

static inline int
net_send_udp_packet6(uchar *ether, struct in6_addr *dest,
		     int dport, int sport, int len)
{
	return -1;
}

static inline int
net_ip6_handler(struct ethernet_hdr *et, struct ip6_hdr *ip6,
		int len)
{
	return -EINVAL;
}

static inline void net_copy_ip6(void *to, const void *from)
{
}
#endif

#if IS_ENABLED(CONFIG_CMD_PING6)
/* Send ping requset */
void ping6_start(void);

/**
 * ping6_receive() - Handle reception of ICMPv6 echo request/reply
 *
 * @et:		pointer to incoming patcket
 * @ip6:	pointer to IPv6 protocol
 * @len:	packet length
 * Return: 0 if success, -EINVAL in case of failure during reception
 */
int ping6_receive(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len);
#else
static inline void ping6_start(void)
{
}

static inline
int ping6_receive(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len)
{
	return -EINVAL;
}
#endif /* CONFIG_CMD_PING6 */

#endif /* __NET6_H__ */
