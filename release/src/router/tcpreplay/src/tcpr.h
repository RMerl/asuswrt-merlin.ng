/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This header is heavily based off of (in other words basically stolen from)
 * libnet 1.1.3's libnet-headers.h.  Many thanks to Mike D. Schiffman for doing
 * all this work so I basically just needed to do a search and replace to get
 * things to work.
 */

#pragma once

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef WIN32

#include "msvc_inttypes.h"
#include "msvc_stdint.h"

#else

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif

#include "config.h"

#define ETHER_ADDR_LEN 0x6
#define FDDI_ADDR_LEN 0x6
#define TOKEN_RING_ADDR_LEN 0x6
#define TCPR_ORG_CODE_SIZE 0x3

/**
 * Libnet defines header sizes for every builder function exported.
 */
#define TCPR_802_1Q_H 0x12            /**< 802.1Q header:       18 bytes */
#define TCPR_802_1X_H 0x04            /**< 802.1X header:        4 bytes */
#define TCPR_802_2_H 0x03             /**< 802.2 LLC header:     3 bytes */
#define TCPR_802_2SNAP_H 0x08         /**< 802.2 LLC/SNAP header:8 bytes */
#define TCPR_802_3_H 0x0e             /**< 802.3 header:        14 bytes */
#define TCPR_ARP_H 0x08               /**< ARP header w/o addrs: 8 bytes */
#define TCPR_ARP_ETH_IP_H 0x1c        /**< ARP w/ ETH and IP:   28 bytes */
#define TCPR_BGP4_HEADER_H 0x13       /**< BGP header:          19 bytes */
#define TCPR_BGP4_OPEN_H 0x0a         /**< BGP open header:     10 bytes */
#define TCPR_BGP4_UPDATE_H 0x04       /**< BGP open header:      4 bytes */
#define TCPR_BGP4_NOTIFICATION_H 0x02 /**< BGP notif. header:    2 bytes */
#define TCPR_CDP_H 0x08               /**< CDP header base:      8 bytes */
#define TCPR_DHCPV4_H 0xf0            /**< DHCP v4 header:     240 bytes */
#define TCPR_UDP_DNSV4_H 0x0c         /**< UDP DNS v4 header:   12 bytes */
#define TCPR_TCP_DNSV4_H 0x0e         /**< TCP DNS v4 header:   14 bytes */
#define TCPR_ETH_H 0x0e               /**< Ethernet header:     14 bytes */
#define TCPR_ETH_MTU 1500             /**< Ethernet MTU size: 1500 bytes */
#define TCPR_FDDI_H 0x15              /**< FDDI header:         21 bytes */
#define TCPR_ICMPV4_H 0x04            /**< ICMP header base:     4 bytes */
#define TCPR_ICMPV4_ECHO_H 0x08       /**< ICMP_ECHO header:     8 bytes */
#define TCPR_ICMPV4_MASK_H 0x0c       /**< ICMP_MASK header:    12 bytes */
#define TCPR_ICMPV4_UNREACH_H 0x08    /**< ICMP_UNREACH header:  8 bytes */
#define TCPR_ICMPV4_TIMXCEED_H 0x08   /**< ICMP_TIMXCEED header: 8 bytes */
#define TCPR_ICMPV4_REDIRECT_H 0x08   /**< ICMP_REDIRECT header: 8 bytes */
#define TCPR_ICMPV4_TS_H 0x14         /**< ICMP_TIMESTAMP headr:20 bytes */
#define TCPR_ICMPV6_H 0x08            /**< ICMP6 header base:    8 bytes */
#define TCPR_IGMP_H 0x08              /**< IGMP header:          8 bytes */
#define TCPR_IPV4_H 0x14              /**< IPv4 header:         20 bytes */
#define TCPR_IPV6_H 0x28              /**< IPv6 header:         40 bytes */
#define TCPR_IPV6_FRAG_H 0x08         /**< IPv6 frag header:     8 bytes */
#define TCPR_IPV6_ROUTING_H 0x04      /**< IPv6 frag header base:4 bytes */
#define TCPR_IPV6_DESTOPTS_H 0x02     /**< IPv6 dest opts base:  2 bytes */
#define TCPR_IPV6_HBHOPTS_H 0x02      /**< IPv6 hop/hop opt base:2 bytes */
#define TCPR_IPSEC_ESP_HDR_H 0x0c     /**< IPSEC ESP header:    12 bytes */
#define TCPR_IPSEC_ESP_FTR_H 0x02     /**< IPSEC ESP footer:     2 bytes */
#define TCPR_IPSEC_AH_H 0x10          /**< IPSEC AH header:     16 bytes */
#define TCPR_ISL_H 0x1a               /**< ISL header:          26 bytes */
#define TCPR_GRE_H 0x04               /**< GRE header:           4 bytes */
#define TCPR_GRE_SRE_H 0x04           /**< GRE SRE header:       4 bytes */
#define TCPR_MPLS_H 0x04              /**< MPLS header:          4 bytes */
#define TCPR_OSPF_H 0x10              /**< OSPF header:         16 bytes */
#define TCPR_OSPF_HELLO_H 0x18        /**< OSPF hello header:   24 bytes */
#define TCPR_OSPF_DBD_H 0x08          /**< OSPF DBD header:      8 bytes */
#define TCPR_OSPF_LSR_H 0x0c          /**< OSPF LSR header:     12 bytes */
#define TCPR_OSPF_LSU_H 0x04          /**< OSPF LSU header:      4 bytes */
#define TCPR_OSPF_LSA_H 0x14          /**< OSPF LSA header:     20 bytes */
#define TCPR_OSPF_AUTH_H 0x08         /**< OSPF AUTH header:     8 bytes */
#define TCPR_OSPF_CKSUM 0x10          /**< OSPF CKSUM header:   16 bytes */
#define TCPR_OSPF_LS_RTR_H 0x10       /**< OSPF LS RTR header:  16 bytes */
#define TCPR_OSPF_LS_NET_H 0x08       /**< OSPF LS NET header:   8 bytes */
#define TCPR_OSPF_LS_SUM_H 0x0c       /**< OSPF LS SUM header:  12 bytes */
#define TCPR_OSPF_LS_AS_EXT_H 0x10    /**< OSPF LS AS header:   16 bytes */
#define TCPR_NTP_H 0x30               /**< NTP header:          48 bytes */
#define TCPR_RIP_H 0x18               /**< RIP header base:     24 bytes */
#define TCPR_RPC_CALL_H                                                                                                \
    0x28 /**< RPC header:          40 bytes                                                                            \
          * (assuming 8 byte auth header)                                                                              \
          */
#define TCPR_RPC_CALL_TCP_H                                                                                            \
    0x2c                       /**< RPC header:          44 bytes                                                      \
                                * (with record marking)                                                                \
                                */
#define TCPR_SEBEK_H 0x30      /* sebek header:          48 bytes */
#define TCPR_STP_CONF_H 0x23   /**< STP conf header:     35 bytes */
#define TCPR_STP_TCN_H 0x04    /**< STP tcn header:       4 bytes */
#define TCPR_TOKEN_RING_H 0x16 /**< Token Ring header:   22 bytes */
#define TCPR_TCP_H 0x14        /**< TCP header:          20 bytes */
#define TCPR_UDP_H 0x08        /**< UDP header:           8 bytes */
#define TCPR_VRRP_H 0x08       /**< VRRP header:          8 bytes */
#define TCPR_HSRP_H 0x14       /**< HSRP header:          8 bytes */

/**
 * IEEE 802.1Q (Virtual Local Area Network) VLAN headr
 * size: 8 bytes
 */
struct tcpr_802_1q_hdr {
    uint16_t vlan_tci;             /* VLAN TCI */
    uint16_t vlan_tpid;            /* Next ETH_TYPE */
#define TCPR_802_1Q_PRIMASK 0xe000 /**< priority mask */
#define TCPR_802_1Q_CFIMASK 0x1000 /**< CFI mask */
#define TCPR_802_1Q_VIDMASK 0x0fff /**< vid mask */
};

/**
 * IEEE 802.1X EAP (Extensible Authentication Protocol) header, static header
 * size: 4 bytes
 */
struct tcpr_802_1x_hdr {
    uint8_t dot1x_version;        /**< protocol version */
    uint8_t dot1x_type;           /**< frame type */
#define TCPR_802_1X_PACKET 0x00   /**< 802.1x packet */
#define TCPR_802_1X_START 0x01    /**< 802.1x start */
#define TCPR_802_1X_LOGOFF 0x02   /**< 802.1x logoff */
#define TCPR_802_1X_KEY 0x03      /**< 802.1x key */
#define TCPR_802_1X_ENCASFAL 0x04 /**< 802.1x encasfal */
    uint16_t dot1x_length;        /**< total frame length */
};

/*
 *  IEEE 802.2 LLC header
 *  Link Layer Control
 *  static header size: 4 bytes
 */
struct tcpr_802_2_hdr {
    uint8_t llc_dsap; /* destination service access point */
    uint8_t llc_ssap; /* source service access point */
#define TCPR_SAP_STP 0x42
#define TCPR_SAP_SNAP 0xaa
    uint16_t llc_control; /* control field */
};

/*
 *  IEEE 802.2 LLC/SNAP header
 *  SubNetwork Attachment Point
 *  static header size: 8 bytes
 */
struct tcpr_802_2snap_hdr {
    uint8_t snap_dsap;    /* destination service access point */
    uint8_t snap_ssap;    /* destination service access point */
    uint8_t snap_control; /* control field */
    uint8_t snap_oui[3];  /* OUI */
    uint16_t snap_type;   /* type */
};

/*
 *  802.3 header
 *  IEEE Ethernet
 *  Static header size: 14 bytes
 */
struct tcpr_802_3_hdr {
    uint8_t _802_3_dhost[ETHER_ADDR_LEN]; /* destination ethernet address */
    uint8_t _802_3_shost[ETHER_ADDR_LEN]; /* source ethernet address */
    uint16_t _802_3_len;                  /* packet type ID */
};

/*
 *  ARP header
 *  Address Resolution Protocol
 *  Base header size: 8 bytes
 */
struct tcpr_arp_hdr {
    uint16_t ar_hrd;        /* format of hardware address */
#define ARPHRD_NETROM 0     /* from KA9Q: NET/ROM pseudo */
#define ARPHRD_ETHER 1      /* Ethernet 10Mbps */
#define ARPHRD_EETHER 2     /* Experimental Ethernet */
#define ARPHRD_AX25 3       /* AX.25 Level 2 */
#define ARPHRD_PRONET 4     /* PROnet token ring */
#define ARPHRD_CHAOS 5      /* Chaosnet */
#define ARPHRD_IEEE802 6    /* IEEE 802.2 Ethernet/TR/TB */
#define ARPHRD_ARCNET 7     /* ARCnet */
#define ARPHRD_APPLETLK 8   /* APPLEtalk */
#define ARPHRD_LANSTAR 9    /* Lanstar */
#define ARPHRD_DLCI 15      /* Frame Relay DLCI */
#define ARPHRD_ATM 19       /* ATM */
#define ARPHRD_METRICOM 23  /* Metricom STRIP (new IANA id) */
#define ARPHRD_IPSEC 31     /* IPsec tunnel */
#define ARPHRD_LOOPBACK 772 /* Loopback device */
    uint16_t ar_pro;        /* format of protocol address */
    uint8_t ar_hln;         /* length of hardware address */
    uint8_t ar_pln;         /* length of protocol address */
    uint16_t ar_op;         /* operation type */
#define ARPOP_REQUEST 1     /* req to resolve address */
#define ARPOP_REPLY 2       /* resp to previous request */
#define ARPOP_REVREQUEST 3  /* req protocol address given hardware */
#define ARPOP_REVREPLY 4    /* resp giving protocol address */
#define ARPOP_INVREQUEST 8  /* req to identify peer */
#define ARPOP_INVREPLY 9    /* resp identifying peer */
    /* address information allocated dynamically */
};

/*
 * BGP4 header
 * Border Gateway Protocol 4
 * Base header size : 19 bytes
 */
struct tcpr_bgp4_header_hdr {
#define TCPR_BGP4_MARKER_SIZE 16
    uint8_t marker[TCPR_BGP4_MARKER_SIZE];
    uint16_t len;
    uint8_t type;
#define TCPR_BGP4_OPEN 1
#define TCPR_BGP4_UPDATE 2
#define TCPR_BGP4_NOTIFICATION 3
#define TCPR_BGP4_KEEPALIVE 4
};

/*
 * BGP4 open header
 * Border Gateway Protocol 4
 * Base header size : 10 bytes
 */
struct tcpr_bgp4_open_hdr {
    uint8_t version;
    uint16_t src_as;
    uint16_t hold_time;
    uint32_t bgp_id;
    uint8_t opt_len;
};

/*
 * BGP4 notification message
 *
 * Border Gateway Protocol 4
 * Base header size : 2 bytes
 *
 * Use payload if you need data
 */
struct tcpr_bgp4_notification_hdr {
#define TCPR_BGP4_MESSAGE_HEADER_ERROR 1
#define TCPR_BGP4_OPEN_MESSAGE_ERROR 2
#define TCPR_BGP4_UPDATE_MESSAGE_ERROR 3
#define TCPR_BGP4_HOLD_TIMER_EXPIRED 4
#define TCPR_BGP4_FINITE_STATE__ERROR 5
#define TCPR_BGP4_CEASE 6
    uint8_t err_code;

/* Message Header Error subcodes */
#define TCPR_BGP4_CONNECTION_NOT_SYNCHRONIZED 1
#define TCPR_BGP4_BAD_MESSAGE_LENGTH 2
#define TCPR_BGP4_BAD_MESSAGE_TYPE 3
/* OPEN Message Error subcodes */
#define TCPR_BGP4_UNSUPPORTED_VERSION_NUMBER 1
#define TCPR_BGP4_BAD_PEER_AS 2
#define TCPR_BGP4_BAD_BGP_IDENTIFIER 3
#define TCPR_BGP4_UNSUPPORTED_OPTIONAL_PARAMETER 4
#define TCPR_BGP4_AUTHENTICATION_FAILURE 5
#define TCPR_BGP4_UNACCEPTABLE_HOLD_TIME 6
/* UPDATE Message Error subcodes */
#define TCPR_BGP4_MALFORMED_ATTRIBUTE_LIST
#define TCPR_BGP4_UNRECOGNIZED_WELL_KNOWN_ATTRIBUTE
#define TCPR_BGP4_MISSING_WELL_KNOWN_ATTRIBUTE
#define TCPR_BGP4_ATTRIBUTE_FLAGS_ERROR
#define TCPR_BGP4_ATTRIBUTE_LENGTH_ERROR
#define TCPR_BGP4_INVALID_ORIGIN_ATTRIBUTE
#define TCPR_BGP4_AS_ROUTING_LOOP
#define TCPR_BGP4_INVALID_NEXT_HOP_ATTRIBUTE
#define TCPR_BGP4_OPTIONAL_ATTRIBUTE_ERROR
#define TCPR_BGP4_INVALID_NETWORK_FIELD
#define TCPR_BGP4_MALFORMED_AS_PATH
    uint8_t err_subcode;
};

/*
 *  CDP header
 *  Cisco Discovery Protocol
 *  Base header size: 8 bytes
 */

/*
 *  For checksum stuff -- IANA says 135-254 is "unassigned" as of 12.2001.
 *  Let's hope this one stays that way for a while!
 */
#define TCPR_PROTO_CDP 200
struct tcpr_cdp_hdr {
    uint8_t cdp_version;      /* version (should always be 0x01) */
    uint8_t cdp_ttl;          /* time receiver should hold info in this packet */
    uint16_t cdp_sum;         /* checksum */
    uint16_t cdp_type;        /* type */
#define TCPR_CDP_DEVID 0x1    /* device id */
#define TCPR_CDP_ADDRESS 0x2  /* address */
#define TCPR_CDP_PORTID 0x3   /* port id */
#define TCPR_CDP_CAPABIL 0x4  /* capabilities */
#define TCPR_CDP_VERSION 0x5  /* version */
#define TCPR_CDP_PLATFORM 0x6 /* platform */
#define TCPR_CDP_IPPREFIX 0x7 /* ip prefix */
    uint16_t cdp_len;         /* type + length + value */
    /* value information done dynamically */

/* CDP capabilities */
#define TCPR_CDP_CAP_L3R 0x01   /* performs level 3 routing */
#define TCPR_CDP_CAP_L2B 0x02   /* performs level 2 transparent bridging */
#define TCPR_CDP_CAP_L2SRB 0x04 /* performs level 2 sourceroute bridging */
#define TCPR_CDP_CAP_L2S 0x08   /* performs level 2 switching */
#define TCPR_CDP_CAP_SR 0x10    /* sends and receives packets on a network */
#define TCPR_CDP_CAP_NOI 0x20   /* does not forward IGMP on non-router ports */
#define TCPR_CDP_CAP_L1F 0x40   /* provides level 1 functionality */
};

/*
 *  Used as an overlay for type/len/values
 */
struct tcpr_cdp_value_hdr {
    uint16_t cdp_type;
    uint16_t cdp_len;
};

/*
 *  DHCP header
 *  Dynamic Host Configuration Protocol
 *  Static header size: f0 bytes
 */
struct tcpr_dhcpv4_hdr {
    uint8_t dhcp_opcode; /* opcode */
#define TCPR_DHCP_REQUEST 0x1
#define TCPR_DHCP_REPLY 0x2
    uint8_t dhcp_htype;      /* hardware address type */
    uint8_t dhcp_hlen;       /* hardware address length */
    uint8_t dhcp_hopcount;   /* used by proxy servers */
    uint32_t dhcp_xid;       /* transaction ID */
    uint16_t dhcp_secs;      /* number of seconds since trying to bootstrap */
    uint16_t dhcp_flags;     /* flags for DHCP, unused for BOOTP */
    uint32_t dhcp_cip;       /* client's IP */
    uint32_t dhcp_yip;       /* your IP */
    uint32_t dhcp_sip;       /* server's IP */
    uint32_t dhcp_gip;       /* gateway IP */
    uint8_t dhcp_chaddr[16]; /* client hardware address */
    uint8_t dhcp_sname[64];  /* server host name */
    uint8_t dhcp_file[128];  /* boot file name */
    uint32_t dhcp_magic;     /* BOOTP magic header */
#define DHCP_MAGIC 0x63825363
#define TCPR_BOOTP_MIN_LEN 0x12c
#define TCPR_DHCP_PAD 0x00
#define TCPR_DHCP_SUBNETMASK 0x01
#define TCPR_DHCP_TIMEOFFSET 0x02
#define TCPR_DHCP_ROUTER 0x03
#define TCPR_DHCP_TIMESERVER 0x04
#define TCPR_DHCP_NAMESERVER 0x05
#define TCPR_DHCP_DNS 0x06
#define TCPR_DHCP_LOGSERV 0x07
#define TCPR_DHCP_COOKIESERV 0x08
#define TCPR_DHCP_LPRSERV 0x09
#define TCPR_DHCP_IMPSERV 0x0a
#define TCPR_DHCP_RESSERV 0x0b
#define TCPR_DHCP_HOSTNAME 0x0c
#define TCPR_DHCP_BOOTFILESIZE 0x0d
#define TCPR_DHCP_DUMPFILE 0x0e
#define TCPR_DHCP_DOMAINNAME 0x0f
#define TCPR_DHCP_SWAPSERV 0x10
#define TCPR_DHCP_ROOTPATH 0x11
#define TCPR_DHCP_EXTENPATH 0x12
#define TCPR_DHCP_IPFORWARD 0x13
#define TCPR_DHCP_SRCROUTE 0x14
#define TCPR_DHCP_POLICYFILTER 0x15
#define TCPR_DHCP_MAXASMSIZE 0x16
#define TCPR_DHCP_IPTTL 0x17
#define TCPR_DHCP_MTUTIMEOUT 0x18
#define TCPR_DHCP_MTUTABLE 0x19
#define TCPR_DHCP_MTUSIZE 0x1a
#define TCPR_DHCP_LOCALSUBNETS 0x1b
#define TCPR_DHCP_BROADCASTADDR 0x1c
#define TCPR_DHCP_DOMASKDISCOV 0x1d
#define TCPR_DHCP_MASKSUPPLY 0x1e
#define TCPR_DHCP_DOROUTEDISC 0x1f
#define TCPR_DHCP_ROUTERSOLICIT 0x20
#define TCPR_DHCP_STATICROUTE 0x21
#define TCPR_DHCP_TRAILERENCAP 0x22
#define TCPR_DHCP_ARPTIMEOUT 0x23
#define TCPR_DHCP_ETHERENCAP 0x24
#define TCPR_DHCP_TCPTTL 0x25
#define TCPR_DHCP_TCPKEEPALIVE 0x26
#define TCPR_DHCP_TCPALIVEGARBAGE 0x27
#define TCPR_DHCP_NISDOMAIN 0x28
#define TCPR_DHCP_NISSERVERS 0x29
#define TCPR_DHCP_NISTIMESERV 0x2a
#define TCPR_DHCP_VENDSPECIFIC 0x2b
#define TCPR_DHCP_NBNS 0x2c
#define TCPR_DHCP_NBDD 0x2d
#define TCPR_DHCP_NBTCPIP 0x2e
#define TCPR_DHCP_NBTCPSCOPE 0x2f
#define TCPR_DHCP_XFONT 0x30
#define TCPR_DHCP_XDISPLAYMGR 0x31
#define TCPR_DHCP_DISCOVERADDR 0x32
#define TCPR_DHCP_LEASETIME 0x33
#define TCPR_DHCP_OPTIONOVERLOAD 0x34
#define TCPR_DHCP_MESSAGETYPE 0x35
#define TCPR_DHCP_SERVIDENT 0x36
#define TCPR_DHCP_PARAMREQUEST 0x37
#define TCPR_DHCP_MESSAGE 0x38
#define TCPR_DHCP_MAXMSGSIZE 0x39
#define TCPR_DHCP_RENEWTIME 0x3a
#define TCPR_DHCP_REBINDTIME 0x3b
#define TCPR_DHCP_CLASSSID 0x3c
#define TCPR_DHCP_CLIENTID 0x3d
#define TCPR_DHCP_NISPLUSDOMAIN 0x40
#define TCPR_DHCP_NISPLUSSERVERS 0x41
#define TCPR_DHCP_MOBILEIPAGENT 0x44
#define TCPR_DHCP_SMTPSERVER 0x45
#define TCPR_DHCP_POP3SERVER 0x46
#define TCPR_DHCP_NNTPSERVER 0x47
#define TCPR_DHCP_WWWSERVER 0x48
#define TCPR_DHCP_FINGERSERVER 0x49
#define TCPR_DHCP_IRCSERVER 0x4a
#define TCPR_DHCP_STSERVER 0x4b
#define TCPR_DHCP_STDASERVER 0x4c
#define TCPR_DHCP_END 0xff

#define TCPR_DHCP_MSGDISCOVER 0x01
#define TCPR_DHCP_MSGOFFER 0x02
#define TCPR_DHCP_MSGREQUEST 0x03
#define TCPR_DHCP_MSGDECLINE 0x04
#define TCPR_DHCP_MSGACK 0x05
#define TCPR_DHCP_MSGNACK 0x06
#define TCPR_DHCP_MSGRELEASE 0x07
#define TCPR_DHCP_MSGINFORM 0x08
};

/*
 *  Base DNSv4 header
 *  Domain Name System
 *  Base header size: 12/14 bytes
 */
struct tcpr_dnsv4_hdr {
    uint16_t h_len;       /* length of the packet - only used with TCP */
    uint16_t id;          /* DNS packet ID */
    uint16_t flags;       /* DNS flags */
    uint16_t num_q;       /* Number of questions */
    uint16_t num_answ_rr; /* Number of answer resource records */
    uint16_t num_auth_rr; /* Number of authority resource records */
    uint16_t num_addi_rr; /* Number of additional resource records */
};

#define TCPR_DNS_H TCPR_UDP_DNSV4_H
struct tcpr_dnsv4udp_hdr {
    uint16_t id;          /* DNS packet ID */
    uint16_t flags;       /* DNS flags */
    uint16_t num_q;       /* Number of questions */
    uint16_t num_answ_rr; /* Number of answer resource records */
    uint16_t num_auth_rr; /* Number of authority resource records */
    uint16_t num_addi_rr; /* Number of additional resource records */
};

/*
 * PPP over Serial with HDLC encapsulation for NetBSD
 */
struct tcpr_pppserial_hdr {
    uint8_t address;
    uint8_t control;
    uint16_t protocol;
};

/*
 *  Ethernet II header
 *  Static header size: 14 bytes
 */
struct tcpr_ethernet_hdr {
    uint8_t ether_dhost[ETHER_ADDR_LEN]; /* destination ethernet address */
    uint8_t ether_shost[ETHER_ADDR_LEN]; /* source ethernet address */
    uint16_t ether_type;                 /* protocol */
};

#ifndef ETHERTYPE_PUP
#define ETHERTYPE_PUP 0x0200 /* PUP protocol */
#endif
#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP 0x0800 /* IP protocol */
#endif
#ifndef ETHERTYPE_ARP
#define ETHERTYPE_ARP 0x0806 /* addr. resolution protocol */
#endif
#ifndef ETHERTYPE_REVARP
#define ETHERTYPE_REVARP 0x8035 /* reverse addr. resolution protocol */
#endif
#ifndef ETHERTYPE_VLAN
#define ETHERTYPE_VLAN 0x8100 /* IEEE 802.1Q VLAN tagging */
#endif
#ifndef ETHERTYPE_EAP
#define ETHERTYPE_EAP 0x888e /* IEEE 802.1X EAP authentication */
#endif
#ifndef ETHERTYPE_MPLS
#define ETHERTYPE_MPLS 0x8847 /* MPLS */
#endif
#ifndef ETHERTYPE_LOOPBACK
#define ETHERTYPE_LOOPBACK 0x9000 /* used to test interfaces */
#endif
#ifndef ETHERTYPE_IP6
#define ETHERTYPE_IP6 0x86DD /* IPv6 */
#endif
#ifndef ETHERTYPE_Q_IN_Q
#define ETHERTYPE_Q_IN_Q 0x88A8 /* 802.1ad Service VLAN */
#endif
#ifndef ETHERTYPE_8021QINQ
#define ETHERTYPE_8021QINQ 0x9100 /* 802.1Q in Q VLAN */
#endif
#ifndef ETHERTYPE_MPLS_MULTI
#define ETHERTYPE_MPLS_MULTI 0x8848 /* MPLS multicast packet */
#endif

struct tcpr_ether_addr {
    uint8_t ether_addr_octet[6]; /* Ethernet address */
};

/*
 *  Fiber Distributed Data Interface header
 *
 *  Static header size: 21 bytes (LLC and 48-bit address addr only)
 *
 *  Note: Organization field is 3 bytes which throws off the
 *        alignment of type.  Therefore fddi_type (19 bytes in)
 *        is specified as two uint8_ts.
 */
struct tcpr_fddi_hdr {
    uint8_t fddi_frame_control; /* Class/Format/Priority */
#define TCPR_FDDI_LLC_FRAME 0x10
#define TCPR_FDDI_48BIT_ADDR 0x40
#define TCPR_FDDI_FC_REQD TCPR_FDDI_LLC_FRAME | TCPR_FDDI_48BIT_ADDR
    uint8_t fddi_dhost[FDDI_ADDR_LEN];             /* destination fddi address */
    uint8_t fddi_shost[FDDI_ADDR_LEN];             /* source fddi address */
    uint8_t fddi_llc_dsap;                         /* DSAP */
    uint8_t fddi_llc_ssap;                         /* SSAP */
    uint8_t fddi_llc_control_field;                /* Class/Format/Priority */
    uint8_t fddi_llc_org_code[TCPR_ORG_CODE_SIZE]; /* Organization Code 3-bytes */
    uint8_t fddi_type;                             /* Protocol Type */
    uint8_t fddi_type1;                            /* see note above. */
#define FDDI_TYPE_IP 0x0800                        /* IP protocol */
#define FDDI_TYPE_ARP 0x0806                       /* addr. resolution protocol */
#define FDDI_TYPE_REVARP 0x8035                    /* reverse addr. resolution protocol */
};

struct tcpr_fddi_addr {
    uint8_t fddi_addr_octet[6]; /* FDDI address */
};

/*
 * GRE header - RFC 1701 & 2637
 * Generic Routing Encapsulation (GRE)
 * Base header size: 4 bytes
 */
struct tcpr_gre_hdr {
    uint16_t flags_ver;
#define GRE_CSUM 0x8000
#define GRE_ROUTING 0x4000
#define GRE_KEY 0x2000
#define GRE_SEQ 0x1000
#define GRE_STRICT 0x0800
#define GRE_REC 0x0700
#define GRE_ACK 0x0080

#define GRE_FLAGS_MASK 0x00F8
#define GRE_VERSION_MASK 0x0007

#define GRE_VERSION_0 0x0000
#define GRE_VERSION_1 0x0001

    uint16_t type;
#define GRE_SNA 0x0004
#define GRE_OSI_NETWORK_LAYER 0x00FE
#define GRE_PUP 0x0200
#define GRE_XNS 0x0600
#define GRE_IP 0x0800
#define GRE_CHAOS 0x0804
#define GRE_RFC_826_ARP 0x0806
#define GRE_FRAME_RELAY_ARP 0x0808
#define GRE_VINES 0x0BAD
#define GRE_VINES_ECHO 0x0BAE
#define GRE_VINES_LOOPBACK 0x0BAF
#define GRE_DECNET 0x6003
#define GRE_TRANSPARENT_ETHERNET_BRIDGING 0x6558
#define GRE_RAW_FRAME_RELAY 0x6559
#define GRE_APOLLO_DOMAIN 0x8019
#define GRE_ETHERTALK 0x809B
#define GRE_NOVELL_IPX 0x8137
#define GRE_RFC_1144_TCP_IP_COMPRESSION 0x876B
#define GRE_IP_AUTONOMOUS_SYSTEMS 0x876C
#define GRE_SECURE_DATA 0x876D
#define GRE_PPP 0x880b /* taken from RFC 2637 */

    union {
        struct {
            uint16_t sum;    /* optional */
            uint16_t offset; /* optional */
            uint32_t key;    /* optional */
            uint32_t seq;    /* optional */
        } _gre;

        struct {
            uint16_t payload_s; /* optional */
            uint16_t callID;    /* optional */
            uint32_t seq;       /* optional */
            uint32_t ack;       /* optional */
        } _egre;
    } _data;

#define gre_sum _data._gre.sum
#define gre_offset _data._gre.offset
#define gre_key _data._gre.key
#define gre_seq _data._gre.seq

#define egre_payload_s _data._egre.payload_s
#define egre_callID _data._egre.callID
#define egre_seq _data._egre.seq
#define egre_ack _data._egre.ack
};

#ifndef IPPROTO_GRE
#define IPPROTO_GRE 47
#endif

/*
 * Source Route Entries (SRE)
 * This is used for GRE as the Routing field is a list of SREs - RFC 1701
 * Base header size: 4 bytes
 */
struct tcpr_gre_sre_hdr {
    uint16_t af; /* address family */
    uint8_t sre_offset;
    uint8_t sre_length;
    uint8_t *routing;
};

/*
 *  IPv4 header
 *  Internet Protocol, version 4
 *  Static header size: 20 bytes
 */
struct tcpr_ipv4_hdr {
#ifdef WORDS_BIGENDIAN
    uint8_t ip_v:4,  /* version */
            ip_hl:4; /* header length */
#else
    uint8_t ip_hl:4,    /* header length */
            ip_v:4;     /* version */
#endif
    uint8_t ip_tos; /* type of service */
#ifndef IPTOS_LOWDELAY
#define IPTOS_LOWDELAY 0x10
#endif
#ifndef IPTOS_THROUGHPUT
#define IPTOS_THROUGHPUT 0x08
#endif
#ifndef IPTOS_RELIABILITY
#define IPTOS_RELIABILITY 0x04
#endif
#ifndef IPTOS_LOWCOST
#define IPTOS_LOWCOST 0x02
#endif
    uint16_t ip_len; /* total length */
    uint16_t ip_id;  /* identification */
    uint16_t ip_off;
#ifndef IP_RF
#define IP_RF 0x8000 /* reserved fragment flag */
#endif
#ifndef IP_DF
#define IP_DF 0x4000 /* don't fragment flag */
#endif
#ifndef IP_MF
#define IP_MF 0x2000 /* more fragments flag */
#endif
#ifndef IP_OFFMASK
#define IP_OFFMASK 0x1fff /* mask for fragmenting bits */
#endif
    uint8_t ip_ttl;                /* time to live */
    uint8_t ip_p;                  /* protocol */
    uint16_t ip_sum;               /* checksum */
    struct in_addr ip_src, ip_dst; /* source and dest address */
};

/*
 *  IP options
 */
#ifndef IPOPT_EOL
#define IPOPT_EOL 0 /* end of option list */
#endif
#ifndef IPOPT_NOP
#define IPOPT_NOP 1 /* no operation */
#endif
#ifndef IPOPT_RR
#define IPOPT_RR 7 /* record packet route */
#endif
#ifndef IPOPT_TS
#define IPOPT_TS 68 /* timestamp */
#endif
#ifndef IPOPT_SECURITY
#define IPOPT_SECURITY 130 /* provide s,c,h,tcc */
#endif
#ifndef IPOPT_LSRR
#define IPOPT_LSRR 131 /* loose source route */
#endif
#ifndef IPOPT_SATID
#define IPOPT_SATID 136 /* satnet id */
#endif
#ifndef IPOPT_SSRR
#define IPOPT_SSRR 137 /* strict source route */
#endif

struct tcpr_in6_addr {
    union {
        u_int8_t __u6_addr8[16];
        u_int16_t __u6_addr16[8];
        u_int32_t __u6_addr32[4];
    } __u6_addr; /* 128-bit IP6 address */
};
#define tcpr_s6_addr __u6_addr.__u6_addr8
#define tcpr_s6_addr8 __u6_addr.__u6_addr8
#define tcpr_s6_addr16 __u6_addr.__u6_addr16
#define tcpr_s6_addr32 __u6_addr.__u6_addr32

/*
 *  IPv6 header
 *  Internet Protocol, version 6
 *  Static header size: 40 bytes
 */
struct tcpr_ipv6_hdr {
    uint8_t ip_flags[4];                 /* version, traffic class, flow label */
    uint16_t ip_len;                     /* total length */
    uint8_t ip_nh;                       /* next header */
    uint8_t ip_hl;                       /* hop limit */
    struct tcpr_in6_addr ip_src, ip_dst; /* source and dest address */
};

struct tcpr_ipv6_ext_hdr_base {
    uint8_t ip_nh;  /* next header */
    uint8_t ip_len; /* length of header in 8 octet units (sans 1st) */
    /* some more bytes are always here, but we don't know what kind */
};

#define TCPR_IPV6_NH_NO_NEXT 59
#define TCPR_IPV6_NH_IPV6 41
#define TCPR_IPV6_NH_ESP 50
#define TCPR_IPV6_NH_AH 51

/*
 *  IPv6 frag header
 *  Internet Protocol, version 6
 *  Static header size: 8 bytes
 */
#define TCPR_IPV6_NH_FRAGMENT 44
struct tcpr_ipv6_frag_hdr {
    uint8_t ip_nh;       /* next header */
    uint8_t ip_reserved; /* reserved */
    uint16_t ip_frag;    /* fragmentation stuff */
    uint32_t ip_id;      /* id */
};

/*
 *  IPv6 routing header
 *  Internet Protocol, version 6
 *  Base header size: 4 bytes
 */
#define TCPR_IPV6_NH_ROUTING 43
struct tcpr_ipv6_routing_hdr {
    uint8_t ip_nh;       /* next header */
    uint8_t ip_len;      /* length of header in 8 octet units (sans 1st) */
    uint8_t ip_rtype;    /* routing type */
    uint8_t ip_segments; /* segments left */
    /* routing information allocated dynamically */
};

/*
 *  IPv6 destination options header
 *  Internet Protocol, version 6
 *  Base header size: 2 bytes
 */
#define TCPR_IPV6_NH_DESTOPTS 60
struct tcpr_ipv6_destopts_hdr {
    uint8_t ip_nh;  /* next header */
    uint8_t ip_len; /* length of header in 8 octet units (sans 1st) */
    /* destination options information allocated dynamically */
};

/*
 *  IPv6 hop by hop options header
 *  Internet Protocol, version 6
 *  Base header size: 2 bytes
 */
#define TCPR_IPV6_NH_HBH 0
struct tcpr_ipv6_hbhopts_hdr {
    uint8_t ip_nh;  /* next header */
    uint8_t ip_len; /* length of header in 8 octet units (sans 1st) */
    /* destination options information allocated dynamically */
};

/*
 *  ICMP6 header
 *  Internet Control Message Protocol v6
 *  Base header size: 8 bytes
 */
#ifndef IPPROTO_ICMP6
#define IPPROTO_ICMP6 0x3a
#endif
struct tcpr_icmpv6_hdr {
    uint8_t icmp_type; /* ICMP type */
#ifndef ICMP6_ECHO
#define ICMP6_ECHO 128
#endif
#ifndef ICMP6_ECHOREPLY
#define ICMP6_ECHOREPLY 129
#endif
#ifndef ICMP6_UNREACH
#define ICMP6_UNREACH 1
#endif
#ifndef ICMP6_PKTTOOBIG
#define ICMP6_PKTTOOBIG 2
#endif
#ifndef ICMP6_TIMXCEED
#define ICMP6_TIMXCEED 3
#endif
#ifndef ICMP6_PARAMPROB
#define ICMP6_PARAMPROB 4
#endif
    uint8_t icmp_code; /* ICMP code */
    uint16_t icmp_sum; /* ICMP Checksum */
    uint16_t id;       /* ICMP id */
    uint16_t seq;      /* ICMP sequence number */
};

/*
 *  ICMP header
 *  Internet Control Message Protocol
 *  Base header size: 4 bytes
 */
struct tcpr_icmpv4_hdr {
    uint8_t icmp_type; /* ICMP type */
#ifndef ICMP_ECHOREPLY
#define ICMP_ECHOREPLY 0
#endif
#ifndef ICMP_UNREACH
#define ICMP_UNREACH 3
#endif
#ifndef ICMP_SOURCEQUENCH
#define ICMP_SOURCEQUENCH 4
#endif
#ifndef ICMP_REDIRECT
#define ICMP_REDIRECT 5
#endif
#ifndef ICMP_ECHO
#define ICMP_ECHO 8
#endif
#ifndef ICMP_ROUTERADVERT
#define ICMP_ROUTERADVERT 9
#endif
#ifndef ICMP_ROUTERSOLICIT
#define ICMP_ROUTERSOLICIT 10
#endif
#ifndef ICMP_TIMXCEED
#define ICMP_TIMXCEED 11
#endif
#ifndef ICMP_PARAMPROB
#define ICMP_PARAMPROB 12
#endif
#ifndef ICMP_TSTAMP
#define ICMP_TSTAMP 13
#endif
#ifndef ICMP_TSTAMPREPLY
#define ICMP_TSTAMPREPLY 14
#endif
#ifndef ICMP_IREQ
#define ICMP_IREQ 15
#endif
#ifndef ICMP_IREQREPLY
#define ICMP_IREQREPLY 16
#endif
#ifndef ICMP_MASKREQ
#define ICMP_MASKREQ 17
#endif
#ifndef ICMP_MASKREPLY
#define ICMP_MASKREPLY 18
#endif
    uint8_t icmp_code; /* ICMP code */
#ifndef ICMP_UNREACH_NET
#define ICMP_UNREACH_NET 0
#endif
#ifndef ICMP_UNREACH_HOST
#define ICMP_UNREACH_HOST 1
#endif
#ifndef ICMP_UNREACH_PROTOCOL
#define ICMP_UNREACH_PROTOCOL 2
#endif
#ifndef ICMP_UNREACH_PORT
#define ICMP_UNREACH_PORT 3
#endif
#ifndef ICMP_UNREACH_NEEDFRAG
#define ICMP_UNREACH_NEEDFRAG 4
#endif
#ifndef ICMP_UNREACH_SRCFAIL
#define ICMP_UNREACH_SRCFAIL 5
#endif
#ifndef ICMP_UNREACH_NET_UNKNOWN
#define ICMP_UNREACH_NET_UNKNOWN 6
#endif
#ifndef ICMP_UNREACH_HOST_UNKNOWN
#define ICMP_UNREACH_HOST_UNKNOWN 7
#endif
#ifndef ICMP_UNREACH_ISOLATED
#define ICMP_UNREACH_ISOLATED 8
#endif
#ifndef ICMP_UNREACH_NET_PROHIB
#define ICMP_UNREACH_NET_PROHIB 9
#endif
#ifndef ICMP_UNREACH_HOST_PROHIB
#define ICMP_UNREACH_HOST_PROHIB 10
#endif
#ifndef ICMP_UNREACH_TOSNET
#define ICMP_UNREACH_TOSNET 11
#endif
#ifndef ICMP_UNREACH_TOSHOST
#define ICMP_UNREACH_TOSHOST 12
#endif
#ifndef ICMP_UNREACH_FILTER_PROHIB
#define ICMP_UNREACH_FILTER_PROHIB 13
#endif
#ifndef ICMP_UNREACH_HOST_PRECEDENCE
#define ICMP_UNREACH_HOST_PRECEDENCE 14
#endif
#ifndef ICMP_UNREACH_PRECEDENCE_CUTOFF
#define ICMP_UNREACH_PRECEDENCE_CUTOFF 15
#endif
#ifndef ICMP_REDIRECT_NET
#define ICMP_REDIRECT_NET 0
#endif
#ifndef ICMP_REDIRECT_HOST
#define ICMP_REDIRECT_HOST 1
#endif
#ifndef ICMP_REDIRECT_TOSNET
#define ICMP_REDIRECT_TOSNET 2
#endif
#ifndef ICMP_REDIRECT_TOSHOST
#define ICMP_REDIRECT_TOSHOST 3
#endif
#ifndef ICMP_TIMXCEED_INTRANS
#define ICMP_TIMXCEED_INTRANS 0
#endif
#ifndef ICMP_TIMXCEED_REASS
#define ICMP_TIMXCEED_REASS 1
#endif
#ifndef ICMP_PARAMPROB_OPTABSENT
#define ICMP_PARAMPROB_OPTABSENT 1
#endif

    uint16_t icmp_sum; /* ICMP Checksum */

    union {
        struct {
            uint16_t id;  /* ICMP id */
            uint16_t seq; /* ICMP sequence number */
        } echo;

#undef icmp_id
#undef icmp_seq
#define icmp_id hun.echo.id
#define icmp_seq hun.echo.seq

        uint32_t gateway; /* gateway host */
        struct {
            uint16_t pad; /* padding */
            uint16_t mtu; /* MTU size */
        } frag;
    } hun;
    union {
        struct {
            uint32_t its_otime;
            uint32_t its_rtime;
            uint32_t its_ttime;
        } ts;
        struct {
            struct tcpr_ipv4_hdr idi_ip;
            /* options and then 64 bits of data */
        } ip;
        uint32_t mask;
        int8_t data[1];

#undef icmp_mask
#define icmp_mask dun.mask
#undef icmp_data
#define icmp_data dun.data

#undef icmp_otime
#define icmp_otime dun.ts.its_otime
#undef icmp_rtime
#define icmp_rtime dun.ts.its_rtime
#undef icmp_ttime
#define icmp_ttime dun.ts.its_ttime
    } dun;
};

/*
 *  IGMP header
 *  Internet Group Message Protocol
 *  Static header size: 8 bytes
 */
struct tcpr_igmp_hdr {
    uint8_t igmp_type; /* IGMP type */
#ifndef IGMP_MEMBERSHIP_QUERY
#define IGMP_MEMBERSHIP_QUERY 0x11 /* membership query */
#endif
#ifndef IGMP_V1_MEMBERSHIP_REPORT
#define IGMP_V1_MEMBERSHIP_REPORT 0x12 /* Ver. 1 membership report */
#endif
#ifndef IGMP_V2_MEMBERSHIP_REPORT
#define IGMP_V2_MEMBERSHIP_REPORT 0x16 /* Ver. 2 membership report */
#endif
#ifndef IGMP_LEAVE_GROUP
#define IGMP_LEAVE_GROUP 0x17 /* Leave-group message */
#endif
    uint8_t igmp_code;         /* IGMP code */
    uint16_t igmp_sum;         /* IGMP checksum */
    struct in_addr igmp_group; /* IGMP host IP */
};

/*
 *  IPSEC header
 *  Internet Protocol Security Protocol
 *  Encapsulating Security Payload Header Static header size: 12 bytes
 *  Encapsulating Security Payload Footer Base header size: 2 bytes
 *  Authentication Header Static Size: 16 bytes
 */
#ifndef IPPROTO_ESP
#define IPPROTO_ESP 50 /* not everyone's got this */
#endif
struct tcpr_esp_hdr {
    uint32_t esp_spi; /* security parameter index */
    uint32_t esp_seq; /* ESP sequence number */
    uint32_t esp_iv;  /* initialization vector */
};

struct tcpr_esp_ftr {
    uint8_t esp_pad_len; /* padding length */
    uint8_t esp_nh;      /* next header pointer */
    int8_t *esp_auth;    /* authentication data */
};

#ifndef IPPROTO_AH
#define IPPROTO_AH 51 /* not everyone's got this */
#endif
struct tcpr_ah_hdr {
    uint8_t ah_nh;    /* next header */
    uint8_t ah_len;   /* payload length */
    uint16_t ah_res;  /* reserved */
    uint32_t ah_spi;  /* security parameter index  */
    uint32_t ah_seq;  /* AH sequence number */
    uint32_t ah_auth; /* authentication data */
};

/*
 *  ISL header
 *  Cisco Inter-Switch Link
 *  Static header size: 26 bytes
 */
/*
 *  For checksum stuff -- IANA says 135-254 is "unassigned" as of 12.2001.
 *  Let's hope this one stays that way for a while!
 */
#define TCPR_PROTO_ISL 201
struct tcpr_isl_hdr {
    uint8_t isl_dhost[5]; /* destination address "01:00:0c:00:00" */
#ifdef WORDS_BIGENDIAN
    uint8_t isl_user:4, /* user defined bits */
            isl_type:4; /* type of frame */
#else
    uint8_t isl_type:4, /* type of frame */
            isl_user:4; /* user defined bits */
#endif
    uint8_t isl_shost[6];  /* source address */
    uint16_t isl_len;      /* total length of packet - 18 bytes */
    uint8_t isl_snap[6];   /* 0xaaaa03 + vendor code */
    uint16_t isl_vid;      /* 15 bit VLAN ID, 1 bit BPDU / CDP indicator */
    uint16_t isl_index;    /* port index */
    uint16_t isl_reserved; /* used for FDDI and token ring */
    /* ethernet frame and 4 byte isl crc */
};

#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF 89 /* not everyone's got this */
#endif
#define IPPROTO_OSPF_LSA 890 /* made this up.  Hope it's unused */
#define TCPR_MODX 4102       /* used in LSA checksum */

/*
 *  Options used in multiple OSPF packets
 *  More info can be found in section A.2 of RFC 2328.
 */
#define TCPR_OPT_EBIT 0x02  /* describes the way AS-external-LSAs are flooded */
#define TCPR_OPT_MCBIT 0x04 /* whether or not IP multicast dgrams are fwdd */
#define TCPR_OPT_NPBIT 0x08 /* describes handling of type-7 LSAs */
#define TCPR_OPT_EABIT 0x10 /* rtr's willingness to send/recv EA-LSAs */
#define TCPR_OPT_DCBIT 0x20 /* describes handling of demand circuits */

/*
 *  MPLS header
 *  Multi-Protocol Label Switching
 *  Static header size: 4 bytes
 */
struct tcpr_mpls_hdr {
    uint32_t mpls_les; /* 20 bits label, 3 bits exp, 1 bit bos, ttl */
#define TCPR_MPLS_BOS_ON 1
#define TCPR_MPLS_BOS_OFF 0
};

/*
 *  NTP header
 *  Network Time Protocol
 *  Static header size: 48 bytes
 */
struct tcpr_ntp_hdr_l_fp /* int32_t floating point (64-bit) */
{
    uint32_t integer;  /* integer */
    uint32_t fraction; /* fraction */
};

struct tcpr_ntp_hdr_s_fp /* int16_t floating point (32-bit) */
{
    uint16_t integer;  /* integer */
    uint16_t fraction; /* fraction */
};

struct tcpr_ntp_hdr {
    uint8_t ntp_li_vn_mode; /* leap indicator, version, mode */
#define TCPR_NTP_LI_NW 0x0  /* no warning */
#define TCPR_NTP_LI_AS 0x1  /* last minute has 61 seconds */
#define TCPR_NTP_LI_DS 0x2  /* last minute has 59 seconds */
#define TCPR_NTP_LI_AC 0x3  /* alarm condition */

#define TCPR_NTP_VN_2 0x2 /* version 2 */
#define TCPR_NTP_VN_3 0x3 /* version 3 */
#define TCPR_NTP_VN_4 0x4 /* version 4 */

#define TCPR_NTP_MODE_R 0x0                  /* reserved */
#define TCPR_NTP_MODE_A 0x1                  /* symmetric active */
#define TCPR_NTP_MODE_P 0x2                  /* symmetric passive */
#define TCPR_NTP_MODE_C 0x3                  /* client */
#define TCPR_NTP_MODE_S 0x4                  /* server */
#define TCPR_NTP_MODE_B 0x5                  /* broadcast */
#define TCPR_NTP_MODE_RC 0x6                 /* reserved for NTP control message */
#define TCPR_NTP_MODE_RP 0x7                 /* reserved for private use */
    uint8_t ntp_stratum;                     /* stratum */
#define TCPR_NTP_STRATUM_UNAVAIL 0x0         /* unspecified or unavailable */
#define TCPR_NTP_STRATUM_PRIMARY 0x1         /* primary reference (radio clock) */
                                             /* 2 - 15 is secondary */
                                             /* 16 - 255 is reserved */
    uint8_t ntp_poll;                        /* poll interval (should be 4 - 12) */
    uint8_t ntp_precision;                   /* local clock precision */
    struct tcpr_ntp_hdr_s_fp ntp_delay;      /* roundtrip delay */
    struct tcpr_ntp_hdr_s_fp ntp_dispersion; /* nominal error */
    uint32_t ntp_reference_id;               /* reference source id */
#define TCPR_NTP_REF_LOCAL 0x4c4f434c        /* uncalibrated local clock */
#define TCPR_NTP_REF_PPS 0x50505300          /* atomic / pulse-per-second clock */
#define TCPR_NTP_REF_ACTS 0x41435453         /* NIST dialup modem */
#define TCPR_NTP_REF_USNO 0x55534e4f         /* USNO modem service */
#define TCPR_NTP_REF_PTB 0x50544200          /* PTB (German) modem service */
#define TCPR_NTP_REF_TDF 0x54444600          /* Allouis (French) radio */
#define TCPR_NTP_REF_DCF 0x44434600          /* Mainflingen (German) radio */
#define TCPR_NTP_REF_MSF 0x4d534600          /* Rugby (UK) radio */
#define TCPR_NTP_REF_WWV 0x57575600          /* Ft Collins (US) radio */
#define TCPR_NTP_REF_WWVB 0x57575642         /* Boulder (US) radio */
#define TCPR_NTP_REF_WWVH 0x57575648         /* Kaui Hawaii (US) radio */
#define TCPR_NTP_REF_CHU 0x43485500          /* Ottaha (Canada) radio */
#define TCPR_NTP_REF_LORC 0x4c4f5243         /* LORAN-C radionavigation */
#define TCPR_NTP_REF_OMEG 0x4f4d4547         /* OMEGA radionavigation */
#define TCPR_NTP_REF_GPS 0x47505300          /* global positioning system */
#define TCPR_NTP_REF_GOES 0x474f4553         /* geostationary orbit env satellite */
    struct tcpr_ntp_hdr_l_fp ntp_ref_ts;     /* reference timestamp */
    struct tcpr_ntp_hdr_l_fp ntp_orig_ts;    /* originate timestamp */
    struct tcpr_ntp_hdr_l_fp ntp_rec_ts;     /* receive timestamp */
    struct tcpr_ntp_hdr_l_fp ntp_xmt_ts;     /* transmit timestamp */
};

/*
 *  OSPFv2 header
 *  Open Shortest Path First
 *  Static header size: 16 bytes
 */
struct tcpr_ospf_hdr {
    uint8_t ospf_v; /* version */
#define OSPFVERSION 2
    uint8_t ospf_type;           /* type */
#define TCPR_OSPF_UMD 0          /* UMd monitoring packet */
#define TCPR_OSPF_HELLO 1        /* HELLO packet */
#define TCPR_OSPF_DBD 2          /* dataBase description packet */
#define TCPR_OSPF_LSR 3          /* link state request packet */
#define TCPR_OSPF_LSU 4          /* link state Update Packet */
#define TCPR_OSPF_LSA 5          /* link state acknowledgement packet */
    uint16_t ospf_len;           /* length */
    struct in_addr ospf_rtr_id;  /* source router ID */
    struct in_addr ospf_area_id; /* roam ID */
    uint16_t ospf_sum;           /* checksum */
    uint16_t ospf_auth_type;     /* authentication type */
#define TCPR_OSPF_AUTH_NULL 0    /* null password */
#define TCPR_OSPF_AUTH_SIMPLE 1  /* simple, plaintext, 8 int8_t password */
#define TCPR_OSPF_AUTH_MD5 2     /* MD5 */
};

/*
 *  OSPF authentication header
 *  Open Shortest Path First
 *  Static header size: 8 bytes
 */
struct tcpr_auth_hdr {
    uint16_t ospf_auth_null; /* NULL */
    uint8_t ospf_auth_keyid; /* authentication key ID */
    uint8_t ospf_auth_len;   /* auth data length */
    uint32_t ospf_auth_seq;  /* cryptographic sequence number */
};

/*
 *  OSPF hello header
 *  Open Shortest Path First
 *  Static header size: 28 bytes
 */
struct tcpr_ospf_hello_hdr {
    struct in_addr hello_nmask;    /* netmask associated with the interface */
    uint16_t hello_intrvl;         /* num of seconds between routers last packet */
    uint8_t hello_opts;            /* Options for HELLO packets (look above) */
    uint8_t hello_rtr_pri;         /* router's priority (if 0, can't be backup) */
    uint32_t hello_dead_intvl;     /* # of secs a router is silent till deemed down */
    struct in_addr hello_des_rtr;  /* Designated router on the network */
    struct in_addr hello_bkup_rtr; /* Backup router */
    struct in_addr hello_nbr;      /* neighbor router, memcpy more as needed */
};

/*
 *  Database Description header.
 */
struct tcpr_dbd_hdr {
    uint16_t dbd_mtu_len;   /* max length of IP dgram that this 'if' can use */
    uint8_t dbd_opts;       /* DBD packet options (from above) */
    uint8_t dbd_type;       /* type of exchange occurring */
#define TCPR_DBD_IBI 0x01   /* init */
#define TCPR_DBD_MBIT 0x02  /* more DBD packets are to come */
#define TCPR_DBD_MSBIT 0x04 /* If 1, sender is the master in the exchange */
    uint32_t dbd_seq;       /* DBD sequence number */
};

/*
 *  used for the LS type field in all LS* headers
 */
#define TCPR_LS_TYPE_RTR 1   /* router-LSA */
#define TCPR_LS_TYPE_NET 2   /* network-LSA */
#define TCPR_LS_TYPE_IP 3    /* summary-LSA (IP Network) */
#define TCPR_LS_TYPE_ASBR 4  /* summary-LSA (ASBR) */
#define TCPR_LS_TYPE_ASEXT 5 /* AS-external-LSA */

/*
 *  Link State Request header
 */
struct tcpr_lsr_hdr {
    uint32_t lsr_type;        /* type of LS being requested */
    uint32_t lsr_lsid;        /* link state ID */
    struct in_addr lsr_adrtr; /* advertising router (memcpy more as needed) */
};

/*
 *  Link State Update header
 */
struct tcpr_lsu_hdr {
    uint32_t lsu_num; /* number of LSAs that will be broadcasted */
};

/*
 *  Link State Acknowledgement header.
 */
struct tcpr_lsa_hdr {
    uint16_t lsa_age;       /* time in seconds since the LSA was originated */
    uint8_t lsa_opts;       /* look above for OPTS_* */
    uint8_t lsa_type;       /* look below for LS_TYPE_* */
    uint32_t lsa_id;        /* link State ID */
    struct in_addr lsa_adv; /* router ID of Advertising router */
    uint32_t lsa_seq;       /* LSA sequence number to detect old/bad ones */
    uint16_t lsa_sum;       /* "Fletcher Checksum" of all fields minus age */
    uint16_t lsa_len;       /* length in bytes including the 20 byte header */
};

/*
 *  Router LSA data format
 *
 *  Other stuff for TOS can be added for backward compatibility, for this
 *  version, only OSPFv2 is being FULLY supported.
 */
struct tcpr_rtr_lsa_hdr {
    uint16_t rtr_flags;         /* set to help describe packet */
#define TCPR_RTR_FLAGS_W 0x0100 /* W bit */
#define TCPR_RTR_FLAGS_E 0x0200 /* E bit */
#define TCPR_RTR_FLAGS_B 0x0400 /* B bit */
    uint16_t rtr_num;           /* number of links within that packet */
    uint32_t rtr_link_id;       /* describes link_data (look below) */
#define TCPR_LINK_ID_NBR_ID 1   /* Neighbors router ID, also can be 4 */
#define TCPR_LINK_ID_IP_DES 2   /* IP address of designated router */
#define TCPR_LINK_ID_SUB 3      /* IP subnet number */
    uint32_t rtr_link_data;     /* Depending on link_id, info is here */
    uint8_t rtr_type;           /* Description of router link */
#define TCPR_RTR_TYPE_PTP 1     /* Point-To-Point */
#define TCPR_RTR_TYPE_TRANS 2   /* Connection to a "transit network" */
#define TCPR_RTR_TYPE_STUB 3    /* Connectin to a "stub network" */
#define RTR_TYPE_VRTL 4         /* connects to a "virtual link" */
    uint8_t rtr_tos_num;        /* number of different TOS metrics for this link */
    uint16_t rtr_metric;        /* the "cost" of using this link */
};

/*
 *  Network LSA data format.
 */
struct tcpr_net_lsa_hdr {
    struct in_addr net_nmask; /* Netmask for that network */
    uint32_t net_rtr_id;      /* ID of router attached to that network */
};

/*
 *  Summary LSA data format.
 */
struct tcpr_sum_lsa_hdr {
    struct in_addr sum_nmask; /* Netmask of destination IP address */
    uint32_t sum_metric;      /* Same as in rtr_lsa (&0xfff to use last 24bit */
    uint32_t sum_tos_metric;  /* first 8bits are TOS, 24bits are TOS Metric */
};

/*
 *  AS External LSA data format.
 *  & 0xfff logic operator for as_metric to get last 24bits.
 */
struct tcpr_as_lsa_hdr {
    struct in_addr as_nmask;        /* Netmask for advertised destination */
    uint32_t as_metric;             /* May have to set E bit in first 8bits */
#define TCPR_AS_E_BIT_ON 0x80000000 /* as_metric */
    struct in_addr as_fwd_addr;     /* Forwarding address */
    uint32_t as_rte_tag;            /* External route tag */
};

/*
 *  Base RIP header
 *  Routing Information Protocol
 *  Base header size: 24 bytes
 */
struct tcpr_rip_hdr {
    uint8_t rip_cmd;       /* RIP command */
#define RIPCMD_REQUEST 1   /* want info */
#define RIPCMD_RESPONSE 2  /* responding to request */
#define RIPCMD_TRACEON 3   /* turn tracing on */
#define RIPCMD_TRACEOFF 4  /* turn it off */
#define RIPCMD_POLL 5      /* like request, but anyone answers */
#define RIPCMD_POLLENTRY 6 /* like poll, but for entire entry */
#define RIPCMD_MAX 7       /* ? command */
    uint8_t rip_ver;       /* RIP version */
#define RIPVER_0 0
#define RIPVER_1 1
#define RIPVER_2 2
    uint16_t rip_rd;       /* Zero (v1) or Routing Domain (v2) */
    uint16_t rip_af;       /* Address family */
    uint16_t rip_rt;       /* Zero (v1) or Route Tag (v2) */
    uint32_t rip_addr;     /* IP address */
    uint32_t rip_mask;     /* Zero (v1) or Subnet Mask (v2) */
    uint32_t rip_next_hop; /* Zero (v1) or Next hop IP address (v2) */
    uint32_t rip_metric;   /* Metric */
};

/*
 *  RPC headers
 *  Remote Procedure Call
 */
#define TCPR_RPC_CALL 0
#define TCPR_RPC_REPLY 1
#define TCPR_RPC_VERS 2
#define TCPR_RPC_LAST_FRAG 0x80000000

/*
 *  Portmap defines
 */
#define TCPR_PMAP_PROGRAM 100000
#define TCPR_PMAP_PROC_NULL 0
#define TCPR_PMAP_PROC_SET 1
#define TCPR_PMAP_PROC_UNSET 2
#define TCPR_PMAP_PROC_GETADDR 3
#define TCPR_PMAP_PROC_DUMP 4
#define TCPR_PMAP_PROC_CALLIT 5
#define TCPR_PMAP_PROC_BCAST 5 /* Not a typo */
#define TCPR_PMAP_PROC_GETTIME 6
#define TCPR_PMAP_PROC_UADDR2TADDR 7
#define TCPR_PMAP_PROC_TADDR2UADDR 8
#define TCPR_PMAP_PROC_GETVERSADDR 9
#define TCPR_PMAP_PROC_INDIRECT 10
#define TCPR_PMAP_PROC_GETADDRLIST 11
#define TCPR_PMAP_PROC_GETSTAT 12

/* There will be more to add... */

struct tcpr_rpc_opaque_auth {
    uint32_t rpc_auth_flavor;
    uint32_t rpc_auth_length;
#if 0
    uint8_t *rpc_auth_data;
#endif
};

struct tcpr_rpc_call {
    uint32_t rpc_rpcvers;   /* RPC version - must be 2 */
    uint32_t rpc_prognum;   /* Program Number */
    uint32_t rpc_vers;      /* Program Version */
    uint32_t rpc_procedure; /* RPC procedure */
    struct tcpr_rpc_opaque_auth rpc_credentials;
    struct tcpr_rpc_opaque_auth rpc_verifier;
};

struct tcpr_rpc_call_hdr {
    uint32_t rpc_xid; /* xid (transaction identifier) */
    uint32_t rpc_type;
    struct tcpr_rpc_call rpc_call;
};

struct tcpr_rpc_call_tcp_hdr {
    uint32_t rpc_record_marking; /* used with byte stream protocols */
    struct tcpr_rpc_call_hdr rpc_common;
};

/*
 *  STP configuration header
 *  Spanning Tree Protocol
 *  Static header size: 35 bytes
 */
struct tcpr_stp_conf_hdr {
    uint16_t stp_id;         /* protocol id */
    uint8_t stp_version;     /* protocol version */
    uint8_t stp_bpdu_type;   /* bridge protocol data unit type */
    uint8_t stp_flags;       /* control flags */
    uint8_t stp_rootid[8];   /* root id */
    uint32_t stp_rootpc;     /* root path cost */
    uint8_t stp_bridgeid[8]; /* bridge id */
    uint16_t stp_portid;     /* port id */
    uint16_t stp_mage;       /* message age */
    uint16_t stp_maxage;     /* max age */
    uint16_t stp_hellot;     /* hello time */
    uint16_t stp_fdelay;     /* forward delay */
};

/*
 *  STP topology change notification header
 *  Spanning Tree Protocol
 *  Static header size: 4 bytes
 */
struct tcpr_stp_tcn_hdr {
    uint16_t stp_id;       /* protocol id */
    uint8_t stp_version;   /* protocol version */
    uint8_t stp_bpdu_type; /* bridge protocol data unit type */
};

/*
 *  TCP header
 *  Transmission Control Protocol
 *  Static header size: 20 bytes
 */
struct tcpr_tcp_hdr {
    uint16_t th_sport; /* source port */
    uint16_t th_dport; /* destination port */
    uint32_t th_seq;   /* sequence number */
    uint32_t th_ack;   /* acknowledgement number */
#ifdef WORDS_BIGENDIAN
    uint8_t th_off:4, /* data offset */
            th_x2:4;  /* (unused) */
#else
    uint8_t th_x2:4,    /* (unused) */
            th_off:4;   /* data offset */
#endif

    uint8_t th_flags; /* control flags */
#ifndef TH_FIN
#define TH_FIN 0x01 /* finished send data */
#endif
#ifndef TH_SYN
#define TH_SYN 0x02 /* synchronize sequence numbers */
#endif
#ifndef TH_RST
#define TH_RST 0x04 /* reset the connection */
#endif
#ifndef TH_PUSH
#define TH_PUSH 0x08 /* push data to the app layer */
#endif
#ifndef TH_ACK
#define TH_ACK 0x10 /* acknowledge */
#endif
#ifndef TH_URG
#define TH_URG 0x20 /* urgent! */
#endif
#ifndef TH_ECE
#define TH_ECE 0x40
#endif
#ifndef TH_CWR
#define TH_CWR 0x80
#endif
    uint16_t th_win; /* window */
    uint16_t th_sum; /* checksum */
    uint16_t th_urp; /* urgent pointer */
};

/*
 *  Token Ring Header
 */
struct tcpr_token_ring_hdr {
    uint8_t token_ring_access_control;
#define TCPR_TOKEN_RING_FRAME 0x10
    uint8_t token_ring_frame_control;
#define TCPR_TOKEN_RING_LLC_FRAME 0x40
    uint8_t token_ring_dhost[TOKEN_RING_ADDR_LEN];
    uint8_t token_ring_shost[TOKEN_RING_ADDR_LEN];
    uint8_t token_ring_llc_dsap;
    uint8_t token_ring_llc_ssap;
    uint8_t token_ring_llc_control_field;
    uint8_t token_ring_llc_org_code[TCPR_ORG_CODE_SIZE];
    uint16_t token_ring_type;
#define TOKEN_RING_TYPE_IP 0x0800     /* IP protocol */
#define TOKEN_RING_TYPE_ARP 0x0806    /* addr. resolution protocol */
#define TOKEN_RING_TYPE_REVARP 0x8035 /* reverse addr. resolution protocol */
};

struct tcpr_token_ring_addr {
    uint8_t token_ring_addr_octet[6]; /* Token Ring address */
};

/*
 *  UDP header
 *  User Data Protocol
 *  Static header size: 8 bytes
 */
struct tcpr_udp_hdr {
    uint16_t uh_sport; /* source port */
    uint16_t uh_dport; /* destination port */
    uint16_t uh_ulen;  /* length */
    uint16_t uh_sum;   /* checksum */
};

/*
 *  Sebek header
 *  Static header size: 48 bytes
 */
struct tcpr_sebek_hdr {
    uint32_t magic;   /* identify packets that should be hidden */
    uint16_t version; /* protocol version, currently 1 */
#define SEBEK_PROTO_VERSION 1
    uint16_t type;        /* type of record (read data is type 0, write data is type 1) */
#define SEBEK_TYPE_READ 0 /* Currently, only read is supported */
#define SEBEK_TYPE_WRITE 1
    uint32_t counter;   /*  PDU counter used to identify when packet are lost */
    uint32_t time_sec;  /* seconds since EPOCH according to the honeypot */
    uint32_t time_usec; /* residual microseconds */
    uint32_t pid;       /* PID */
    uint32_t uid;       /* UID */
    uint32_t fd;        /* FD */
#define SEBEK_CMD_LENGTH 12
    uint8_t cmd[SEBEK_CMD_LENGTH]; /* 12 first characters of the command */
    uint32_t length;               /* length in bytes of the PDU's body */
};

/*
 *  VRRP header
 *  Virtual Router Redundancy Protocol
 *  Static header size: 8 bytes
 */
#ifndef IPPROTO_VRRP
#define IPPROTO_VRRP 112 /* not everyone's got this */
#endif
struct tcpr_vrrp_hdr {
#ifdef WORDS_BIGENDIAN
    uint8_t vrrp_t:4, /* packet type */
            vrrp_v:4; /* protocol version */
#else
    uint8_t vrrp_v:4,   /* protocol version */
            vrrp_t:4;   /* packet type */
#endif
#define TCPR_VRRP_VERSION_01 0x1
#define TCPR_VRRP_VERSION_02 0x2
#define TCPR_VRRP_TYPE_ADVERT 0x1
    uint8_t vrrp_vrouter_id; /* virtual router id */
    uint8_t vrrp_priority;   /* priority */
    uint8_t vrrp_ip_count;   /* number of IP addresses */
    uint8_t vrrp_auth_type;  /* authorization type */
#define TCPR_VRRP_AUTH_NONE 0x1
#define TCPR_VRRP_AUTH_PASSWD 0x2
#define TCPR_VRRP_AUTH_IPAH 0x3
    uint8_t vrrp_advert_int; /* advertisement interval */
    uint16_t vrrp_sum;       /* checksum */
    /* additional addresses */
    /* authentication info */
};

/*
 *  HSRP header
 *  Static header size: 20 bytes
 */
struct tcpr_hsrp_hdr {
#define TCPR_HSRP_VERSION 0x0
    uint8_t version; /* Version of the HSRP messages */
#define TCPR_HSRP_TYPE_HELLO 0x0
#define TCPR_HSRP_TYPE_COUP 0x1
#define TCPR_HSRP_TYPE_RESIGN 0x2
    uint8_t opcode; /* Type of message */
#define TCPR_HSRP_STATE_INITIAL 0x0
#define TCPR_HSRP_STATE_LEARN 0x1
#define TCPR_HSRP_STATE_LISTEN 0x2
#define TCPR_HSRP_STATE_SPEAK 0x4
#define TCPR_HSRP_STATE_STANDBY 0x8
#define TCPR_HSRP_STATE_ACTIVE 0x10
    uint8_t state;      /* Current state of the router */
    uint8_t hello_time; /* Period in seconds between hello messages */
    uint8_t hold_time;  /* Seconds that the current hello message is valid */
    uint8_t priority;   /* Priority for the election process */
    uint8_t group;      /* Standby group */
    uint8_t reserved;   /* Reserved field */
#define HSRP_AUTHDATA_LENGTH 8
    uint8_t authdata[HSRP_AUTHDATA_LENGTH]; /* Password */
    uint32_t virtual_ip;                    /* Virtual IP address */
};

/* MPLS label:: RFC 5462, RFC 3032
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                Label                  | TC  |S|       TTL     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *      Label:  Label Value, 20 bits
 *      TC:     Traffic Class field, 3 bits
 *      S:      Bottom of Stack, 1 bit
 *      TTL:    Time to Live, 8 bits
 */

struct tcpr_mpls_label {
    uint32_t entry;
#define MPLS_LABEL_GACH 13
};

#ifndef MPLS_LS_LABEL_MASK
#define MPLS_LS_LABEL_MASK 0xFFFFF000
#endif
#ifndef MPLS_LS_LABEL_SHIFT
#define MPLS_LS_LABEL_SHIFT 12
#endif
#ifndef MPLS_LS_TC_MASK
#define MPLS_LS_TC_MASK 0x00000E00
#endif
#ifndef MPLS_LS_TC_SHIFT
#define MPLS_LS_TC_SHIFT 9
#endif
#ifndef MPLS_LS_S_MASK
#define MPLS_LS_S_MASK 0x00000100
#endif
#ifndef MPLS_LS_S_SHIFT
#define MPLS_LS_S_SHIFT 8
#endif
#ifndef MPLS_LS_TTL_MASK
#define MPLS_LS_TTL_MASK 0x000000FF
#endif
#ifndef MPLS_LS_TTL_SHIFT
#define MPLS_LS_TTL_SHIFT 0
#endif
