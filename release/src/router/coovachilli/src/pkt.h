/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#ifndef _PKT_H
#define _PKT_H

#define PKT_ETH_ALEN              6 /* Ethernet Address Length */
#define PKT_ETH_HLEN             14 /* Ethernet Header Length */

/*
 * http://www.iana.org/assignments/ethernet-numbers
 */
#define PKT_ETH_PROTO_IP     0x0800
#define PKT_ETH_PROTO_ARP    0x0806
#define PKT_ETH_PROTO_WOL    0x0842	
#define PKT_ETH_PROTO_ETHBR  0x6558 
#define PKT_ETH_PROTO_8021Q  0x8100
#define PKT_ETH_PROTO_IPX    0x8137
#define PKT_ETH_PROTO_IPv6   0x86dd
#define PKT_ETH_PROTO_PPP    0x880b
#define PKT_ETH_PROTO_PPPOED 0x8863
#define PKT_ETH_PROTO_PPPOES 0x8864
#define PKT_ETH_PROTO_EAPOL  0x888e
#define PKT_ETH_PROTO_CHILLI 0xbeef

#define PKT_IP_PLEN            1500 /* IP Payload Length */
#define PKT_IP_VER_HLEN        0x45 
#define PKT_IP_ALEN               4
#define PKT_IP_HLEN              20
#define PKT_IPv6_HLEN            40

#define PKT_IP_PROTO_ICMP         1 /* ICMP Protocol number */
#define PKT_IP_PROTO_IGMP         2 /* IGMP Protocol number */
#define PKT_IP_PROTO_TCP          6 /* TCP Protocol number */
#define PKT_IP_PROTO_UDP         17 /* UDP Protocol number */
#define PKT_IP_PROTO_GRE         47 /* GRE Protocol number */
#define PKT_IP_PROTO_ESP         50
#define PKT_IP_PROTO_AH          51
#define PKT_IP_PROTO_SKIP        57
#define PKT_IP_PROTO_EIGRP       88
#define PKT_IP_PROTO_OSPF        89
#define PKT_IP_PROTO_L2TP       115

#define PKT_UDP_HLEN              8
#define PKT_TCP_HLEN             20
#define PKT_DOT1X_HLEN            4

#define PKT_EAP_PLEN          10240 /* Dot1x Payload length */

#define DHCP_TAG_VLEN           255 /* Tag value always shorter than this */
#define EAPOL_TAG_VLEN          255 /* Tag value always shorter than this */

#define DHCP_HTYPE_ETH            1
#define DHCP_CHADDR_LEN          16 /* Length of client hardware address */
#define DHCP_SNAME_LEN           64 /* Length of server host name */
#define DHCP_FILE_LEN           128 /* Length of boot file name*/
#define DHCP_OPTIONS_LEN        312 /* Length of optional parameters field */
#define DHCP_MIN_LEN   28+16+64+128 /* Length of packet excluding options */
#define DHCP_LEN  (DHCP_MIN_LEN+DHCP_OPTIONS_LEN)

#define PKT_BUFFER PKT_MAX_LEN

struct pkt_ethhdr_t {
  uint8_t  dst[PKT_ETH_ALEN];
  uint8_t  src[PKT_ETH_ALEN];
  uint16_t prot;
} __attribute__((packed));

struct pkt_ethhdr8021q_t {
  uint8_t  dst[PKT_ETH_ALEN];
  uint8_t  src[PKT_ETH_ALEN];
  uint16_t tpid;
#define PKT_8021Q_MASK_VID htons(0x0FFF)
#define PKT_8021Q_MASK_PCP htons(0xE000)
#define PKT_8021Q_MASK_CFI htons(0x1000)
  uint16_t pcp_cfi_vid;
  uint16_t prot;
} __attribute__((packed));

#ifdef ENABLE_IEEE8023
struct pkt_llc_t {
  uint8_t dsap;
  uint8_t ssap;
  uint8_t cntl;
} __attribute__((packed));

struct pkt_llc_snap_t {
  uint8_t code[3];
  uint16_t type;
} __attribute__((packed));
#endif

#ifdef ENABLE_PPPOE
struct pkt_pppoe_hdr_t {
#define PKT_PPPoE_VERSION 0x11
  uint8_t version_type;
#define PKT_PPPoE_PADI 0x09
#define PKT_PPPoE_PADO 0x07
#define PKT_PPPoE_PADR 0x19
#define PKT_PPPoE_PADS 0x65
#define PKT_PPPoE_PADT 0xa7
  uint8_t code;
  uint16_t session_id;
  uint16_t length;
} __attribute__((packed));

struct pkt_pppoe_taghdr_t {
#define PPPoE_TAG_ServiceName        0x0101
#define PPPoE_TAG_ACName             0x0102
#define PPPoE_TAG_HostUniq           0x0103
#define PPPoE_TAG_ACCookie           0x0104
#define PPPoE_TAG_VendorSpecific     0x0105
#define PPPoE_TAG_ServiceNameError   0x0201
#define PPPoE_TAG_ACSystemError      0x0202
  uint16_t type;
  uint16_t length;
} __attribute__((packed));

#define PKT_PPP_PROTO_LCP 0xc021

struct pkt_ppp_lcp_t {
#define PPP_LCP_ConfigRequest 0x01
#define PPP_LCP_ConfigAck 0x02
#define PPP_LCP_ConfigNak 0x03
#define PPP_LCP_ConfigReject 0x04
  uint8_t code;
  uint8_t id;
  uint16_t length;
} __attribute__((packed));

struct pkt_lcp_opthdr_t {
#define PPP_LCP_OptMTU 0x01
#define PPP_LCP_OptACCM 0x02
#define PPP_LCP_OptAuthProto 0x03
#define PPP_LCP_OptMagic 0x05
#define PPP_LCP_OptCompress 0x07
  uint8_t type;
  uint8_t length;
} __attribute__((packed));
#endif

struct pkt_iphdr_t {
  uint8_t  version_ihl;
  uint8_t  tos;
  uint16_t tot_len;
  uint16_t id;
  uint8_t opt_off_high;
  uint8_t off_low;
  uint8_t  ttl;
  uint8_t  protocol;
  uint16_t check;
  uint32_t saddr;
  uint32_t daddr;
} __attribute__((packed));

#define iphdr_dont_frag(p) ((p)->opt_off_high & 0x40)
#define iphdr_more_frag(p) ((p)->opt_off_high & 0x20)
#define iphdr_offset(p) ntohs((((p)->opt_off_high & 0x13) << 8)|(p)->off_low)

#ifdef ENABLE_IPV6
#define PKT_IPv6_ALEN 16
struct pkt_ip6hdr_t {
  uint32_t ver_class_label; /* 4bit version, 8bit class, 20bit label */
  uint16_t data_len;
  uint8_t next_header;
  uint8_t hop_limit;
  uint8_t src_addr[PKT_IPv6_ALEN];
  uint8_t dst_addr[PKT_IPv6_ALEN];
} __attribute__((packed));

struct pkt_ip6pseudo_t {
  uint8_t src_addr[PKT_IPv6_ALEN];
  uint8_t dst_addr[PKT_IPv6_ALEN];
  uint32_t packet_len;
  uint8_t zero[3];
  uint8_t next_header;
} __attribute__((packed));

int chksum6(struct pkt_ip6hdr_t *iph);

struct pkt_dhcp6hdr_t {
  uint8_t type;
  uint8_t id[3];
} __attribute__((packed));

#define ICMPv6_NEXT_HEADER 58
#define ipv6_version(x)  /*lazy!*/ \
((((1<<31)|(1<<30)|(1<<29)|(1<<28))&(ntohl((x)->ver_class_label)))>>28)
#define ipv6_class(x) \
((((1<<27)|(1<<26)|(1<<25)|(1<<24)|(1<<23)|(1<<22)|(1<<21)|(1<<20))&(ntohl((x)->ver_class_label)))>>20)

#define IPv6_ADDR_FMT "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"
#define ipv6_exlode_addr(x) \
  (int) x[0], (int) x[1], (int) x[2], (int) x[3], (int) x[4], (int) x[5], \
  (int) x[6], (int) x[7], (int) x[8], (int) x[9], (int) x[10], (int) x[11],\
  (int) x[12], (int) x[13], (int) x[14], (int) x[15]

#define ipv6_nat64_pack(d, p) \
  *(d)++ = 0x11; *(d)++ = 0x12; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = (p)[0]; *(d)++ = (p)[1]; *(d)++ = (p)[2] ;*(d)++ = (p)[3]

#define ipv6_nat64_prefix(d) \
  *(d)++ = 0x11; *(d)++ = 0x12; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0

#define ipv6_eui64_pack(d, p) \
  *(d)++ = 0x11; *(d)++ = 0x11; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = (p)[0]; *(d)++ = (p)[1]; *(d)++ = (p)[2] ;*(d)++ = 0xff; \
  *(d)++ = 0xfe; *(d)++ = (p)[3]; *(d)++ = (p)[4] ;*(d)++ = (p)[5]

#define ipv6_eui64_prefix(d) \
  *(d)++ = 0x11; *(d)++ = 0x11; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; \
  *(d)++ = 0; *(d)++ = 0; *(d)++ = 0; *(d)++ = 0

#endif

struct pkt_ipphdr_t {
  /* Convenience structure:
     Same as pkt_iphdr_t, but also
     with ports (UDP and TCP packets) */
  uint8_t  version_ihl;
  uint8_t  tos;
  uint16_t tot_len;
  uint16_t id;
  uint16_t frag_off;
  uint8_t  ttl;
  uint8_t  protocol;
  uint16_t check;
  uint32_t saddr;
  uint32_t daddr;
  uint16_t sport;
  uint16_t dport;
} __attribute__((packed));


struct pkt_icmphdr_t {
  uint8_t type;
  uint8_t code;
  uint16_t check;
} __attribute__((packed));


/*
  0      7 8     15 16    23 24    31  
  +--------+--------+--------+--------+ 
  |     Source      |   Destination   | 
  |      Port       |      Port       | 
  +--------+--------+--------+--------+ 
  |                 |                 | 
  |     Length      |    Checksum     | 
  +--------+--------+--------+--------+ 
  |                                     
  |          data octets ...            
  +---------------- ...                 
  
  User Datagram Header Format
*/

struct pkt_udphdr_t {
  uint16_t src;
  uint16_t dst;
  uint16_t len;
  uint16_t check;
} __attribute__((packed));

/*
  TCP Header Format

    0                   1                   2                   3   
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          Source Port          |       Destination Port        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Sequence Number                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Acknowledgment Number                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Data |           |U|A|P|R|S|F|                               |
   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
   |       |           |G|K|H|T|N|N|                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Checksum            |         Urgent Pointer        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             data                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

struct pkt_tcphdr_t {
  uint16_t src;
  uint16_t dst;
  uint32_t seq;
  uint32_t ack;
  uint8_t  offres;
  uint8_t  flags;
  uint16_t win;
  uint16_t check;
  uint16_t urgent;
  uint8_t options[4];
} __attribute__((packed));

#define TCPHDR_FLAG_FIN (1<<0)
#define TCPHDR_FLAG_SYN (1<<1)
#define TCPHDR_FLAG_RST (1<<2)
#define TCPHDR_FLAG_PSH (1<<3)
#define TCPHDR_FLAG_ACK (1<<4)
#define TCPHDR_FLAG_URG (1<<5)

#define tcphdr_fin(hdr) (((hdr)->flags & TCPHDR_FLAG_FIN)==TCPHDR_FLAG_FIN)
#define tcphdr_syn(hdr) (((hdr)->flags & TCPHDR_FLAG_SYN)==TCPHDR_FLAG_SYN)
#define tcphdr_rst(hdr) (((hdr)->flags & TCPHDR_FLAG_RST)==TCPHDR_FLAG_RST)
#define tcphdr_ack(hdr) (((hdr)->flags & TCPHDR_FLAG_ACK)==TCPHDR_FLAG_ACK)
#define tcphdr_psh(hdr) (((hdr)->flags & TCPHDR_FLAG_PSH)==TCPHDR_FLAG_PSH)

/*
  0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
   +---------------+---------------+---------------+---------------+
   |                            xid (4)                            |
   +-------------------------------+-------------------------------+
   |           secs (2)            |           flags (2)           |
   +-------------------------------+-------------------------------+
   |                          ciaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          yiaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          siaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          giaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          chaddr  (16)                         |
   |                             (cont)                            |
   +---------------------------------------------------------------+
   |                          sname   (64)                         |
   |                             (cont)                            |
   +---------------------------------------------------------------+
   |                          file    (128)                        |
   |                             (cont)                            |
   +---------------------------------------------------------------+
   |                          options (variable)                   |
   |                             (cont)                            |
   +---------------------------------------------------------------+
*/

struct dhcp_packet_t { /* From RFC 2131 */
  uint8_t op;       /* 1 Message op code / message type.  1 = BOOTREQUEST, 2 = BOOTREPLY */
  uint8_t htype;    /* 1 Hardware address type, see ARP section in "Assigned Numbers" RFC */
  uint8_t hlen;     /* 1 Hardware address length (e.g. '6' for 10mb ethernet).*/
  uint8_t hops;     /* 1 Client sets to zero, optionally used by relay agents when booting via a relay agent.*/
  uint32_t xid;     /* 4 Transaction ID, a random number chosen by the client, used by the client and
		       server to associate messages and responses between a client and a server. */
  uint16_t secs;    /* 2 Filled in by client, seconds elapsed since client began address acquisition or renewal process.*/
  uint8_t flags[2]; /* 2  Flags (see figure 2).*/
  uint32_t ciaddr;  /* 4 Client IP address; only filled in if client is in BOUND, RENEW or REBINDING state
		       and can respond to ARP requests.*/
  uint32_t yiaddr;  /* 4 'your' (client) IP address.*/
  uint32_t siaddr;  /* 4 IP address of next server to use in bootstrap; returned in DHCPOFFER, DHCPACK by server.*/
  uint32_t giaddr;  /* 4 Relay agent IP address, used in booting via a relay agent.*/
  uint8_t chaddr[DHCP_CHADDR_LEN];   /* 16 Client hardware address.*/
  uint8_t sname[DHCP_SNAME_LEN];     /* 64 Optional server host name, null terminated string.*/
  uint8_t file[DHCP_FILE_LEN];       /* 128 Boot file name, null terminated string; "generic" name or null in
					DHCPDISCOVER, fully qualified directory-path name in DHCPOFFER.*/
  uint8_t options[DHCP_OPTIONS_LEN]; /* var Optional parameters field.  See the options documents for a list
					of defined options.*/
} __attribute__((packed));


struct dhcp_tag_t {
  uint8_t t;
  uint8_t l;
  uint8_t v[DHCP_TAG_VLEN];
} __attribute__((packed));


struct arp_packet_t { /* From RFC 826 */
  uint16_t hrd; /* 16.bit: (ar$hrd) Hardware address space (e.g.,
		    Ethernet, Packet Radio Net.) */
  uint16_t pro; /* 16.bit: (ar$pro) Protocol address space.  For
		    Ethernet hardware, this is from the set of type
		    fields ether_typ$<protocol>. */
  uint8_t hln;  /* 8.bit: (ar$hln) byte length of each hardware address */
  uint8_t pln;  /* 8.bit: (ar$pln) byte length of each protocol address */
  uint16_t op;  /* 16.bit: (ar$op)  opcode (ares_op$REQUEST | ares_op$REPLY) */
  uint8_t sha[PKT_ETH_ALEN]; /* nbytes: (ar$sha) Hardware address of
		    sender of this packet, n from the ar$hln field. */
  uint8_t spa[PKT_IP_ALEN];  /* mbytes: (ar$spa) Protocol address of
		    sender of this packet, m from the ar$pln field. */
  uint8_t tha[PKT_ETH_ALEN]; /* nbytes: (ar$tha) Hardware address of
		  target of this packet (if known). */
  uint8_t tpa[PKT_IP_ALEN]; /* mbytes: (ar$tpa) Protocol address of
				 target.*/
} __attribute__((packed));


struct dns_packet_t { /* From RFC 1035 */
  uint16_t id;      /* 16 bit: Generated by requester. Copied in reply */
  uint16_t flags;   /* 16 bit: Flags */
  uint16_t qdcount; /* 16 bit: Number of questions */
  uint16_t ancount; /* 16 bit: Number of answer records */
  uint16_t nscount; /* 16 bit: Number of name servers */
  uint16_t arcount; /* 16 bit: Number of additional records */
  uint8_t  records[PKT_IP_PLEN];
} __attribute__((packed));

struct pkt_dot1xhdr_t {
  uint8_t  ver;
  uint8_t  type;
  uint16_t len;
} __attribute__((packed));

struct eap_packet_t {
  uint8_t  code;
  uint8_t  id;
  uint16_t length;
  uint8_t  type;
  uint8_t  payload[PKT_EAP_PLEN];
} __attribute__((packed));

#ifdef ENABLE_CLUSTER
struct pkt_chillihdr_t {
  uint8_t from;
  uint8_t type;
  struct in_addr addr;
  uint8_t mac[PKT_ETH_ALEN];
  uint8_t state;
} __attribute__((packed));
#define CHILLI_PEER_INIT    1
#define CHILLI_PEER_HELLO   2
#define CHILLI_PEER_GOODBYE 3
#define CHILLI_PEER_LIST    4
#define CHILLI_PEER_CMD     5
#define CHILLI_PEER_CMD_RES 6
#endif

#ifdef ENABLE_IEEE8021Q
#define is_8021q(pkt) (((struct pkt_ethhdr8021q_t *)pkt)->tpid == htons(PKT_ETH_PROTO_8021Q))
#define get8021q(pkt) (((struct pkt_ethhdr8021q_t *)pkt)->pcp_cfi_vid)

#define sizeofeth2(is8021q)   (PKT_ETH_HLEN+((is8021q)?4:0))
#define sizeofip2(is8021q)    (sizeofeth2(is8021q)+PKT_IP_HLEN)
#define sizeofdot1x2(is8021q) (sizeofeth2(is8021q)+PKT_DOT1X_HLEN)
#define sizeofudp2(is8021q)   (sizeofip2(is8021q)+PKT_UDP_HLEN)
#define sizeoftcp2(is8021q)   (sizeofip2(is8021q)+PKT_TCP_HLEN)
#ifdef ENABLE_IPV6
#define sizeofip62(is8021q)   (sizeofeth2(is8021q)+PKT_IPv6_HLEN)
#define sizeofudp62(is8021q)  (sizeofip62(is8021q)+PKT_UDP_HLEN)
#define sizeoftcp62(is8021q)  (sizeofip62(is8021q)+PKT_TCP_HLEN)
#endif

#define sizeofeth(pkt)   sizeofeth2(is_8021q(pkt))
#define sizeofip(pkt)    sizeofip2(is_8021q(pkt))
#define sizeofdot1x(pkt) sizeofdot1x2(is_8021q(pkt))
#define sizeofudp(pkt)   sizeofudp2(is_8021q(pkt))
#define sizeoftcp(pkt)   sizeoftcp2(is_8021q(pkt))
#define sizeofarp(pkt)   (sizeofeth(pkt)+sizeof(struct arp_packet_t))
#define ethhdr8021q(pkt) ((struct pkt_ethhdr8021q_t *)pkt)
#ifdef ENABLE_IPV6
#define sizeofip6(pkt)   sizeofip62(is_8021q(pkt))
#define sizeofudp6(pkt)  sizeofudp62(is_8021q(pkt))
#define sizeoftcp6(pkt)  sizeoftcp62(is_8021q(pkt))
#endif

#define copy_ethproto(o,n)  \
  if (is_8021q(o)) { \
    ((struct pkt_ethhdr8021q_t *)n)->tpid = htons(PKT_ETH_PROTO_8021Q); \
    ((struct pkt_ethhdr8021q_t *)n)->pcp_cfi_vid = ((struct pkt_ethhdr8021q_t *)o)->pcp_cfi_vid; \
    ((struct pkt_ethhdr8021q_t *)n)->prot = ((struct pkt_ethhdr8021q_t *)o)->prot; \
  } else { \
    ((struct pkt_ethhdr_t *)n)->prot = ((struct pkt_ethhdr_t *)o)->prot; \
  }

#else

#define sizeofeth2(x)   (PKT_ETH_HLEN)
#define sizeofip2(x)    (sizeofeth2(x)+PKT_IP_HLEN)
#define sizeofdot1x2(x) (sizeofeth2(x)+PKT_DOT1X_HLEN)
#define sizeofudp2(x)   (sizeofip2(x)+PKT_UDP_HLEN)
#define sizeoftcp2(x)   (sizeofip2(x)+PKT_TCP_HLEN)
#define sizeofeth(pkt)   sizeofeth2(0)
#define sizeofip(pkt)    sizeofip2(0)
#define sizeofdot1x(pkt) sizeofdot1x2(0)
#define sizeofudp(pkt)   sizeofudp2(0)
#define sizeoftcp(pkt)   sizeoftcp2(0)
#define sizeofarp(pkt)   (sizeofeth(pkt)+sizeof(struct arp_packet_t))

#define copy_ethproto(o,n) { \
    ((struct pkt_ethhdr_t *)n)->prot = ((struct pkt_ethhdr_t *)o)->prot; \
  }
#endif

#define pkt_ethhdr(pkt)   ((struct pkt_ethhdr_t *)pkt)
#define pkt_ipphdr(pkt)   ((struct pkt_ipphdr_t *)  (((uint8_t*)(pkt)) + sizeofeth(pkt)))
#define pkt_iphdr(pkt)    ((struct pkt_iphdr_t *)   (((uint8_t*)(pkt)) + sizeofeth(pkt)))
#define pkt_icmphdr(pkt)  ((struct pkt_icmphdr_t *) (((uint8_t*)(pkt)) + sizeofip(pkt)))
#define pkt_udphdr(pkt)   ((struct pkt_udphdr_t *)  (((uint8_t*)(pkt)) + sizeofip(pkt)))
#define pkt_tcphdr(pkt)   ((struct pkt_tcphdr_t *)  (((uint8_t*)(pkt)) + sizeofip(pkt)))
#define pkt_dot1xhdr(pkt) ((struct pkt_dot1xhdr_t *)(((uint8_t*)(pkt)) + sizeofeth(pkt)))
#define pkt_dhcppkt(pkt)  ((struct dhcp_packet_t *) (((uint8_t*)(pkt)) + sizeofudp(pkt)))
#define pkt_arppkt(pkt)   ((struct arp_packet_t *)  (((uint8_t*)(pkt)) + sizeofeth(pkt)))
#define pkt_dnspkt(pkt)   ((struct dns_packet_t *)  (((uint8_t*)(pkt)) + sizeofudp(pkt)))
#define pkt_eappkt(pkt)   ((struct eap_packet_t *)  (((uint8_t*)(pkt)) + sizeofdot1x(pkt)))
#ifdef ENABLE_IPV6
#define pkt_ip6hdr(pkt)   ((struct pkt_ip6hdr_t *)  (((uint8_t*)(pkt)) + sizeofeth(pkt)))
#define pkt_udp6hdr(pkt)   ((struct pkt_udphdr_t *)  (((uint8_t*)(pkt)) + sizeofip6(pkt)))
#define pkt_tcp6hdr(pkt)   ((struct pkt_tcphdr_t *)  (((uint8_t*)(pkt)) + sizeofip6(pk)))
#endif

#define chilli_ethhdr(pkt)((struct pkt_chillihdr_t *)(((uint8_t*)(pkt)) + sizeofeth(pkt)))

struct eapol_tag_t {
  uint8_t t;
  uint8_t l;
  uint8_t v[EAPOL_TAG_VLEN];
} __attribute__((packed));

int chksum(struct pkt_iphdr_t *iph);
int pkt_shape_tcpwin(struct pkt_iphdr_t *iph, uint16_t win);
int pkt_shape_tcpmss(uint8_t *packet, size_t *length);

#if defined(ENABLE_IPV6)
#define PKT_BUFFER_IPOFF  (sizeof(struct pkt_ethhdr8021q_t)+20)
#else
#define PKT_BUFFER_IPOFF  (sizeof(struct pkt_ethhdr8021q_t))
#endif

struct pkt_buffer {
  uint8_t *   buf;
  size_t      buflen;
  size_t      offset;
  size_t      length;
};

#define pkt_buffer_init(pb, b, blen, off)	\
      (pb)->buf = (b);				\
      (pb)->buflen = (blen);			\
      (pb)->offset = (off);			\
      (pb)->length = 0

#define pkt_buffer_init2(pb, b, blen, off, len)	\
      (pb)->buf = (b);				\
      (pb)->buflen = (blen);			\
      (pb)->offset = (off);			\
      (pb)->length = (len)

#define pkt_buffer_head(pb)    ((pb)->buf + (pb)->offset)
#define pkt_buffer_length(pb)  ((pb)->length)
#define pkt_buffer_size(pb)    ((pb)->buflen - (pb)->offset)
#define pkt_buffer_grow(pb,l)  (pb)->offset -= (l); (pb)->length += (l)
#define pkt_buffer_is_ip(pb)   ((pb)->offset == PKT_BUFFER_IPOFF)
#define pkt_buffer_is_eth(pb)  ((pb)->offset == PKT_BUFFER_ETHOFF)
#define pkt_buffer_is_vlan(pb) ((pb)->offset == PKT_BUFFER_VLANOFF)

#define MAC_FMT "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X"
#define MAC_ARG(x) (x)[0],(x)[1],(x)[2],(x)[3],(x)[4],(x)[5]



#endif
