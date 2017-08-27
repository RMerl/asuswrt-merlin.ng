/*
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * This source code was modified by Broadcom. It is distributed under the
 * original license terms described below.
 *
 * Copyright (c) 2002-2006 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
/*
 * $Id: wl_mdns_main.h 467328 2014-04-03 01:23:40Z $
 *
 * Description: Multicast DNS Service Discovery offload main defines.
 *
 */


#ifndef _MDNS_SD_MAIN_H_
#define _MDNS_SD_MAIN_H_

/* timer.h */
#define TIME_UNIT_5MS 5
#define TIME_UNIT_1S  1000

#define ONE_SECOND_IN_5MS (1000 / TIME_UNIT_5MS)
#define ONE_SECOND_IN_1S  (1000 / TIME_UNIT_1S)

/* 	prot.h */

/* frame protocol type */
#define PROTOCOL_ARP       0x0806
#define PROTOCOL_IP        0x0800
#define PROTOCOL_IPV6      0x86dd
#define PROTOCOL_VLAN      0x8100

/* VLAN */
#define VLAN_ID_MASK       0xfff

/* ARP type */
#define ARP_REQUEST        1
#define ARP_RESPONSE       2

/* IP packet */
#define IPV4_VER             0x4
#define IPV6_VER           0x6
#define IP_DEFAULT_TTL     32

/* IP address */
#define IPV4_ADDR_BYTE_LEN	4
#define IPV6_ADDR_BYTE_LEN	16

/* IP packet type */
#define PROTOCOL_HOP_OPT	0
#define PROTOCOL_ICMP		1
#define PROTOCOL_IGMP      	2
#define PROTOCOL_TCP       	6
#define PROTOCOL_UDP       	17
#define PROTOCOL_ICMPV6		58

/* UDP port type */
#define UDP_PORT_RMCP				0x26F
#define UDP_PORT_RMCP_SECURE    		0x298	/* ASF2.0 */
#define UDP_PORT_DHCP_SERVER    		67
#define UDP_PORT_DHCP_CLIENT    		68
#define UDP_PORT_RMCP20				UDP_PORT_RMCP_SECURE	/* legacy */
#define UDP_PORT_SNMP				161
#define UDP_PORT_SNMP_TRAP			162
#define UDP_PORT_USER_DEFINE_TRAP_SRC_PORT 	0x0402      /* trap source port, use 1026 */
#define UDP_PORT_MDNS				5353

typedef struct _MAC_HDR
{
	u8  dest_addr[6];
	u8  src_addr[6];
	u16 fr_type;
} MAC_HDR, *pMAC_HDR;

typedef struct
{
	u8  ver_hdr_len;
	u8  svc_type;

	u16 tot_len;
	u16 id;

	u16 flags_frag_offset;
	#define IP_FRAG_NOT_USED         0x8000
	#define IP_FRAG_DONT_FRAG        0x4000
	#define IP_FRAG_FRAGMENT         0x2000
	#define IP_FRAG_OFFSET_MASK      0x1fff
	u8  time_to_live;
	u8  protocol;

	u16 hdr_checksum;
	u8  src_addr[IPV4_ADDR_BYTE_LEN];       /* Legacy: not dword aligned; 06/08:alinged */
	u8  dest_addr[IPV4_ADDR_BYTE_LEN];      /* Legacy: not dword aligned; 06/08:alinged */
} ipHdr_t, *pIpHdr;

typedef struct
{
	/*
	u32  ver_traffic_class_flow_label;
	*/
	union
	{
		u32  ver:4;
		u32  trafficClass:8;
		u32  flowLable:20;

		u8   b[4];
	} vtf;

	u16  payload_len;
	u8 	next_hdr;
	u8 	hop_limit;

	u8  src_addr[IPV6_ADDR_BYTE_LEN];
	u8  dest_addr[IPV6_ADDR_BYTE_LEN];
} ipV6Hdr_t, *pIpV6Hdr;

typedef struct
{
	u16 src_port;
	u16 dest_port;

	u16 len;
	u16 checksum;
} UdpHdr_t, *pUdpHdr;

typedef struct
{
	u16 src_port;
	u16 dest_port;

	u32 seq_num;
	u32 ack_num;

	u16 data_ecn_ctrl;
#define TCP_FLAG_FIN    0x001
#define TCP_FLAG_SYN    0x002
#define TCP_FLAG_RST    0x004
#define TCP_FLAG_PSH    0x008
#define TCP_FLAG_ACK    0x010
#define TCP_FLAG_URG    0x020
#define TCP_FLAG_ECE    0x040
#define TCP_FLAG_CWR    0x080
#define TCP_FLAG_NS     0x100

	u16 window;

	u16 checksum;
	u16 urgent_ptr;
} PACKED_STRUCT TcpHdr_t, *pTcpHdr;

typedef struct
{
	u16   hw_type;

	u16   prot_type;
	u8    hw_addr_len;
	u8    prot_addr_len;

	u16   opcode;
	u8    src_hw_addr[6];

	u8    src_prot_addr[4];

	u8    dest_hw_addr[6];
	u8    dest_prot_addr[4];  /* Legacy: not dword aligned; 06/08:alinged */
} ARP_PKT;

typedef struct
{
	u8  source_ip[IPV6_ADDR_BYTE_LEN];
	u8  dest_ip[IPV6_ADDR_BYTE_LEN];
	u32 upper_layer_len;
	u32 zero_next_hdr;
} IPV6_PSEUDO_HDR, *pIPV6_PSEUDO_HDR;

typedef struct
{
	MAC_HDR        mac;
	ipHdr_t        ip;
	union
	{
		UdpHdr_t       udp;
	} txp;
} TXPORT_HDR, *pTXPORT_HDR;

typedef struct
{
	MAC_HDR      mac;
	ipV6Hdr_t    ip;
	UdpHdr_t     udp;
} PACKED_STRUCT TXPORT_IPV6_UDP_HDR, *pTXPORT_IPV6_UDP_HDR;

typedef struct
{
	MAC_HDR      mac;
	ipV6Hdr_t    ip;
	TcpHdr_t     tcp;
} PACKED_STRUCT TXPORT_IPV6_TCP_HDR, *pTXPORT_IPV6_TCP_HDR;

/* mdns_sd_main.h */

typedef struct
{
	MAC_HDR       mac;
	ipHdr_t       ip;
	UdpHdr_t      udp;
	DNSMessageHeader   h;
} PACKED_STRUCT MDNS_IPV4_PKT, *pMDNS_IPV4_PKT;

typedef struct
{
	MAC_HDR			mac;
	ipV6Hdr_t		ip;
	UdpHdr_t		udp;
	DNSMessageHeader 	h;
} PACKED_STRUCT MDNS_IPV6_PKT, *pMDNS_IPV6_PKT;

typedef struct
{
	mDNSOpaque64 InitiatorCookie;
	mDNSOpaque64 ResponderCookie;
	mDNSu8       NextPayload;
	mDNSu8       Version;
	mDNSu8       ExchangeType;
	mDNSu8       Flags;
	mDNSOpaque32 MessageID;
	mDNSu32      Length;
} PACKED_STRUCT IKEHeader, *pIKEHeader;			/* 28 bytes */

#define IPSECPort	4500
#define SSHPort 	22
#define ARDPort 	3283

extern mDNSAddr AllDNSLinkGroup_v4;
extern mDNSv4Addr zerov4Addr;
extern mDNSv6Addr zerov6Addr;

extern void mdns_sd_main_init(void);
extern s32 mDNS_Execute(void);
extern void mDNSCoreReceive(void *pkt, u8 *end, u8 *srcMacAddr, mDNSAddr *srcaddr,
	mDNSIPPort srcport, mDNSAddr *dstaddr, mDNSIPPort dstport);

typedef void (*callBackFunc)(void);

#define BONJOUR_PROXY_SIG		0x424A5030	/* "BJP0" */
#define MDNS_SD_DRV_INTF_ADDR		SYSTEM_VAR_START
#define MDNS_SD_DRV_INTF_SIZE		(MAX_PKT_SIZE * 2) /* ~3K */


#define MAX_MDNS_SD_PROXY_SERVICES 	16
#define MAX_MDNS_IPV4_ADDRESSES 	3
#define MAX_MDNS_IPV6_ADDRESSES 	3
#define MAX_MDNS_IPV6_SOL_ADDRESSES	3
/*
	Signature
		Flags

	IP addresses for this interface
	{
		Number of IPv4 addresses
		Number of IPv6 addresses
		Number of IPv6 solicited addresses
		List of { IPv4 address, masks}
		List of { IPv6 address, prefix length}
		List of IPv6 solicited addresses
	}

	IP addresses for the other interfaces
	{
		Number of IPv4 addresses
		Number of IPv6 addresses
		List of { IPv4 address, masks}
		List of { IPv6 address, prefix length}
	}

	Number of UDP Wakeup ports
	Number of TCP Wakeup ports
	Number of RR SRV records
	Number of RR TXT records

	UDP Wakeup ports list
	TCP Wakeup ports list

	Array of #SRV offsets
	Array of #TXT offsets

	RR SRV records (and compression base)
	RR TXT records
*/
typedef struct
{
	u8		numIPv4;
	u8		numIPv6;
	u8		numIPv6Sol;
	/* Remaining fields are variable size
	uint32  ipv4Addr[];
	uint32  ipv4Mask[];
	uint8	ipv6Addr[][16];
	uint8	ipv6SubnetLen[];
	uint8	ipv6SolAddr[][16];
	*/
} PACKED_STRUCT proxy_services_offload_addr_info_t;

typedef struct
{
	mDNSv4Addr	addr;
	mDNSv4Addr	mask;
} PACKED_STRUCT proxy_services_offload_ipv4_addr_t;

typedef struct
{
	mDNSv6Addr	addr;
	mDNSu8		prexfixLen;
} PACKED_STRUCT proxy_services_offload_ipv6_addr_t;

typedef struct
{
	u8		numIPv4;
	u8		numIPv6;
	/* Remaining fields are variable size
	uint32  ipv4Addr[];
	uint32  ipv4Mask[];
	uint8	ipv6Addr[][16];
	uint8	ipv6SubnetLen[];
	*/
} PACKED_STRUCT proxy_services_secondary_addr_info_t;

typedef struct
{
	u16		numUDPWakeupPorts;
	u16		numTCPWakeupPorts;
	u16		numSRVRecords;
	u16		numTXTRecords;

	/* Remaining fields are variable size
	u16		updPorts[];
	u16		tcpPorts[];
	...		srvRecords;
	...		txtRecords;
	*/
} PACKED_STRUCT proxy_services_data_t;

typedef struct
{
	/* Table 7 of donwload spec */
	u32		signature;
	u32		flags;		/* See OffloadFlags field in mDNS_struct for defines. */
	proxy_services_offload_addr_info_t	offloadAddresses; /* Table 8 from download spec */
	/* u8		numIPv4; */
	/* u8		numIPv6; */
	/* u8		numIPv6Sol; */
	/*
	proxy_services_secondary_addr_t		secondaryAddresses;
	proxy_services_data_t			data;
	*/
} PACKED_STRUCT proxy_services_t;

extern mDNSAddr AllDNSLinkGroup_v4;


#endif  /* _MDNS_SD_MAIN_H_ */
