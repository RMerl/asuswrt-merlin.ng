/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef __SKB_ACCESS_H__
#define __SKB_ACCESS_H__

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/igmp.h>

#if 0	/* mit: This is useless?! @20141204 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
  #if !defined(CONFIG_NF_CONNTRACK) &&	\
      !defined(CONFIG_NF_CONNTRACK_MODULE)
    #undef TMCFG_E_CORE_KERN_CONN_TRACK
  #endif
#endif
#endif

#ifndef TMCFG_E_CORE_KERN_CONN_TRACK
#define TMCFG_E_CORE_KERN_CONN_TRACK 0
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
  #include <net/netfilter/nf_conntrack.h>
#else
  #include <linux/netfilter_ipv4/ip_conntrack.h>
#endif

#define SKB_PACKET_LEN(skb)	((skb)->len)

/* Ether header */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))

#define SKB_ETH(skb)		( (struct ethhdr*)  (skb->head + skb->mac_header) )
#define SKB_IP(skb)			( (struct iphdr*)   (skb->head + skb->network_header) )
#define SKB_IP6(skb)		( (struct ipv6hdr*) (skb->head + skb->network_header) )
#define SKB_ICMP(skb)		( (struct icmphdr*) (skb->head + skb->transport_header) )
#define SKB_IGMP(skb)		( (struct igmphdr*) (skb->head + skb->transport_header) )
#define SKB_TCP(skb)		( (struct tcphdr*)  (skb->head + skb->transport_header) )
#define SKB_UDP(skb)		( (struct udphdr*)  (skb->head + skb->transport_header) )

#define SKB_ETH_ADDR(skb)	( skb->head + skb->mac_header )
#define SKB_L3_HEAD_ADDR(skb)	( skb->head + skb->network_header )
#define SKB_L4_HEAD_ADDR(skb)	( skb->head + skb->transport_header )

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22))
/*
 * It's annoying to determine when the skb stuff is changed,
 * the preprocessor condition checks only 2.6.26 kernel.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32))

#define SKB_ETH(skb) 		((struct ethhdr *)  skb_mac_header(skb))
#define SKB_IP(skb)			((struct iphdr *)   skb_network_header(skb))
#define SKB_IP6(skb) 		((struct ipv6hdr *) skb_network_header(skb))
#define SKB_ICMP(skb)		((struct icmphdr *) skb_transport_header(skb))
#define SKB_IGMP(skb)		((struct igmphdr *) skb_transport_header(skb))
#define SKB_TCP(skb)		((struct tcphdr *)  skb_transport_header(skb))
#define SKB_UDP(skb) 		((struct udphdr *)  skb_transport_header(skb))

#define SKB_ETH_ADDR(skb)	skb_mac_header(skb)
#define SKB_L3_HEAD_ADDR(skb)	skb_network_header(skb)
#define SKB_L4_HEAD_ADDR(skb)	skb_transport_header(skb)

#else // LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)

#ifdef NET_SKBUFF_DATA_USES_OFFSET
#define SKB_ETH(skb) 		( (struct ethhdr*)  (skb->head + skb->mac_header) )
#define SKB_IP(skb)			( (struct iphdr*)   (skb->head + skb->network_header) )
#define SKB_IP6(skb) 		( (struct ipv6hdr*) (skb->head + skb->network_header) )
#define SKB_ICMP(skb)		( (struct icmphdr*) (skb->head + skb->transport_header) )
#define SKB_IGMP(skb)		( (struct igmphdr*) (skb->head + skb->transport_header) )
#define SKB_TCP(skb)		( (struct tcphdr*)  (skb->head + skb->transport_header) )
#define SKB_UDP(skb) 		( (struct udphdr*)  (skb->head + skb->transport_header) )

#define SKB_ETH_ADDR(skb)	( skb->head + skb->mac_header )
#define SKB_L3_HEAD_ADDR(skb)	( skb->head + skb->network_header )
#define SKB_L4_HEAD_ADDR(skb)	( skb->head + skb->transport_header )

#else

#define SKB_ETH(skb) 		( (struct ethhdr*)  skb->mac_header )
#define SKB_IP(skb)			( (struct iphdr*)   skb->network_header )
#define SKB_IP6(skb) 		( (struct ipv6hdr*) skb->network_header )
#define SKB_ICMP(skb)		( (struct icmphdr*) skb->transport_header )
#define SKB_IGMP(skb)		( (struct igmphdr*) skb->transport_header )
#define SKB_TCP(skb)		( (struct tcphdr*)  skb->transport_header )
#define SKB_UDP(skb) 		( (struct udphdr*)  skb->transport_header )

#define SKB_ETH_ADDR(skb)	( skb->mac_header )
#define SKB_L3_HEAD_ADDR(skb)	( skb->network_header )
#define SKB_L4_HEAD_ADDR(skb)	( skb->transport_header )

#endif	/* ! NET_SKBUFF_DATA_USES_OFFSET */
#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
#else

#define SKB_ETH(skb)		( (struct ethhdr*) skb->mac.raw )
#define SKB_IP(skb) 		( skb->nh.iph )
#define SKB_IP6(skb)		( skb->nh.ipv6h )
#define SKB_ICMP(skb) 		( skb->h.icmph )
#define SKB_IGMP(skb)		( skb->h.igmph )
#define SKB_TCP(skb)		( skb->h.th )
#define SKB_UDP(skb)		( skb->h.uh )

#define SKB_ETH_ADDR(skb)	( (skb)->mac.raw )
#define SKB_L3_HEAD_ADDR(skb)	( (skb)->nh.raw )
#define SKB_L4_HEAD_ADDR(skb)	( (skb)->h.raw )

#endif	/* ! (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)) */

#define SKB_ETH_DST(skb)	( SKB_ETH((skb))->h_dest )
#define SKB_ETH_SRC(skb)	( SKB_ETH((skb))->h_source )
#define SKB_ETH_PRO(skb)	( SKB_ETH((skb))->h_proto )

/* IPv4 header */
#define SKB_IP_HEAD_ADDR(skb)	( SKB_IP(skb) )
#define SKB_IP_SIP(skb)		( SKB_IP(skb)->saddr )
#define SKB_IP_DIP(skb)		( SKB_IP(skb)->daddr )
#define SKB_IP_VER(skb)		( SKB_IP(skb)->version )
#define SKB_IP_IHL(skb)		( SKB_IP(skb)->ihl )
#define SKB_IP_ID(skb)		( SKB_IP(skb)->id )
#define SKB_IP_FRAG_OFF(skb)	( SKB_IP(skb)->frag_off )
#define SKB_IP_TTL(skb)		( SKB_IP(skb)->ttl )
#define SKB_IP_PRO(skb)		( SKB_IP(skb)->protocol )
#define SKB_IP_TOS(skb)		( SKB_IP(skb)->tos )
#define SKB_IP_TOT_LEN(skb)	( SKB_IP(skb)->tot_len )
#define SKB_IP_CHECK(skb)	( SKB_IP(skb)->check )

/* IPv6 header */
#define SKB_IPV6_HEAD_ADDR(skb)	( SKB_IP6(skb) )
#define SKB_IPV6_VER(skb)	( SKB_IP6(skb)->version )
#define SKB_IPV6_PRIO(skb)	( SKB_IP6(skb)->priority )
#define SKB_IPV6_FLB(skb)	( SKB_IP6(skb)->flow_lbl )
#define SKB_IPV6_SIP(skb)	( SKB_IP6(skb)->saddr.in6_u.u6_addr8 )
#define SKB_IPV6_IN6SIP(skb)	( SKB_IP6(skb)->saddr )
#define SKB_IPV6_DIP(skb)	( SKB_IP6(skb)->daddr.in6_u.u6_addr8 )
#define SKB_IPV6_IN6DIP(skb)	( SKB_IP6(skb)->daddr )
#define SKB_IPV6_PLEN(skb)	( SKB_IP6(skb)->payload_len )
#define SKB_IPV6_HOPL(skb)	( SKB_IP6(skb)->hop_limit )
#define SKB_IPV6_NHDR(skb)	( SKB_IP6(skb)->nexthdr )

/* ICMP header */
#define SKB_ICMP_HEAD_ADDR(skb)	( SKB_ICMP(skb) )
#define SKB_ICMP_TYPE(skb)	( SKB_ICMP(skb)->type )
#define SKB_ICMP_CODE(skb)	( SKB_ICMP(skb)->code )
#define SKB_ICMP_CHECK(skb)	( SKB_ICMP(skb)->checksum )
#define SKB_ICMP_ID(skb)	( SKB_ICMP(skb)->un.echo.id )
#define SKB_ICMP_SEQ(skb)	( SKB_ICMP(skb)->un.echo.sequence )

/* IGMP header */
#define SKB_IGMP_HEAD_ADDR(skb)	( SKB_IGMP(skb) )
#define SKB_IGMP_TYPE(skb)	( SKB_IGMP(skb)->type )
#define SKB_IGMP_CODE(skb)	( SKB_IGMP(skb)->code )
#define SKB_IGMP_CHECK(skb)	( SKB_IGMP(skb)->csum )
#define SKB_IGMP_GROUP(skb)	( SKB_IGMP(skb)->group )


/* TCP header */
#define SKB_TCP_HEAD_ADDR(skb)	( SKB_TCP(skb) )
#define SKB_TCP_SPORT(skb)	( SKB_TCP(skb)->source )
#define SKB_TCP_DPORT(skb)	( SKB_TCP(skb)->dest )
#define SKB_TCP_SEQ(skb)	( SKB_TCP(skb)->seq )
#define SKB_TCP_ACK(skb)	( SKB_TCP(skb)->ack_seq )
#define SKB_TCP_WIN(skb)	( SKB_TCP(skb)->window )
#define SKB_TCP_CHECK(skb)	( SKB_TCP(skb)->check )
#define SKB_TCP_URG(skb)	( SKB_TCP(skb)->urg_ptr )
#define SKB_TCP_FLAGS_RES1(skb)	( SKB_TCP(skb)->res1 )
#define SKB_TCP_FLAGS_FIN(skb)	( SKB_TCP(skb)->fin )
#define SKB_TCP_FLAGS_SYN(skb)	( SKB_TCP(skb)->syn )
#define SKB_TCP_FLAGS_RST(skb)	( SKB_TCP(skb)->rst )
#define SKB_TCP_FLAGS_PSH(skb)	( SKB_TCP(skb)->psh )
#define SKB_TCP_FLAGS_ACK(skb)	( SKB_TCP(skb)->ack )
#define SKB_TCP_FLAGS_URG(skb)	( SKB_TCP(skb)->urg )
#define SKB_TCP_FLAGS_ECE(skb)	( SKB_TCP(skb)->ece )
#define SKB_TCP_FLAGS_CWR(skb)	( SKB_TCP(skb)->cwr )
#define SKB_TCP_FLAGS(skb)			\
	((SKB_TCP_FLAGS_FIN(skb) << 0) |	\
	 (SKB_TCP_FLAGS_SYN(skb) << 1) |	\
	 (SKB_TCP_FLAGS_RST(skb) << 2) |	\
	 (SKB_TCP_FLAGS_PSH(skb) << 3) |	\
	 (SKB_TCP_FLAGS_ACK(skb) << 4) |	\
	 (SKB_TCP_FLAGS_URG(skb) << 5))

#define SKB_TCP_FIN_VAL(skb)	(SKB_TCP_FLAGS_FIN(skb) << 0)
#define SKB_TCP_SYN_VAL(skb)	(SKB_TCP_FLAGS_SYN(skb) << 1)
#define SKB_TCP_RST_VAL(skb)	(SKB_TCP_FLAGS_RST(skb) << 2)
#define SKB_TCP_PSH_VAL(skb)	(SKB_TCP_FLAGS_PSH(skb) << 3)
#define SKB_TCP_ACK_VAL(skb)	(SKB_TCP_FLAGS_ACK(skb) << 4)
#define SKB_TCP_URG_VAL(skb)	(SKB_TCP_FLAGS_URG(skb) << 5)

#define SKB_TCP_HLEN(skb)	( SKB_TCP(skb)->doff )

/* UDP header */
#define SKB_UDP_HEAD_ADDR(skb)	( SKB_UDP(skb) )
#define SKB_UDP_SPORT(skb)	( SKB_UDP(skb)->source )
#define SKB_UDP_DPORT(skb)	( SKB_UDP(skb)->dest )
#define SKB_UDP_LEN(skb)	( SKB_UDP(skb)->len )
#define SKB_UDP_CHECK(skb)	( SKB_UDP(skb)->check )

static inline
char* ip_str(unsigned char *ip, int ip_ver)
{
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN	48
#endif

	static int iter = 0;
	static unsigned char ipstr_buf[4][INET6_ADDRSTRLEN];

	unsigned char *ipstr = ipstr_buf[iter++ & 3];
	*ipstr = 0;

	if (ip_ver == 4) {
		snprintf(ipstr, INET6_ADDRSTRLEN,
				 "%d.%d.%d.%d", 
				 ip[0], ip[1], ip[2], ip[3]);
	} else if (ip_ver == 6) {
		/* No support for address abbrevation */
		snprintf(ipstr, INET6_ADDRSTRLEN,
				 "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:"
				 "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
				 ip[0], ip[1], ip[2], ip[3], 
				 ip[4], ip[5], ip[6], ip[7], 
				 ip[8], ip[9], ip[10], ip[11], 
				 ip[12], ip[13], ip[14], ip[15]);
	}

	return ipstr;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
  /* u3 */
  #define GET_TUPLE_SIP(__tuple)	(__tuple)->src.u3.ip
  #define GET_TUPLE_DIP(__tuple)	(__tuple)->dst.u3.ip
  #define GET_TUPLE_SIP6(__tuple)	(__tuple)->src.u3.ip6
  #define GET_TUPLE_DIP6(__tuple)	(__tuple)->dst.u3.ip6
  /* u */
  #define GET_TUPLE_SPORT(__tuple)	(__tuple)->src.u.all
  #define GET_TUPLE_DPORT(__tuple)	(__tuple)->dst.u.all
  #define GET_TUPLE_PROTO(__tuple)	(__tuple)->dst.protonum

  /* ICMP */
  #define GET_TUPLE_ICMP_ID(__tuple)	(__tuple)->src.u.icmp.id
  #define GET_TUPLE_ICMP_TYPE(__tuple)	(__tuple)->dst.u.icmp.type
  #define GET_TUPLE_ICMP_CODE(__tuple)	(__tuple)->dst.u.icmp.code
#else
  #define GET_TUPLE_SIP(__tuple)	(__tuple)->src.ip
  #define GET_TUPLE_DIP(__tuple)	(__tuple)->dst.ip
  #define GET_TUPLE_SPORT(__tuple)	(__tuple)->src.u.all
  #define GET_TUPLE_DPORT(__tuple)	(__tuple)->dst.u.all
  /* ICMP */
  #define GET_TUPLE_ICMP_ID(__tuple)	(__tuple)->src.u.icmp.id
  #define GET_TUPLE_ICMP_TYPE(__tuple)	(__tuple)->dst.u.icmp.type
  #define GET_TUPLE_ICMP_CODE(__tuple)	(__tuple)->dst.u.icmp.code
#endif

#endif
