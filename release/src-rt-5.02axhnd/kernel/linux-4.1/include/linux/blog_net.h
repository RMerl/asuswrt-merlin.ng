#if defined(CONFIG_BCM_KF_BLOG)
#ifndef __BLOG_NET_H_INCLUDED__
#define __BLOG_NET_H_INCLUDED__

/*
<:copyright-BRCM:2003:DUAL/GPL:standard

   Copyright (c) 2003 Broadcom 
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

/*
 *******************************************************************************
 *
 * File Name  : blog_net.h
 *
 * Description:
 *
 * Global definitions and declaration of Protocol Headers independent of OS as
 * per IEEE and RFC standards.  Inlined utilities for header access.
 *
 * CAUTION: All protocol header structures are declared for Big Endian access
 * and are not compatible for a Little Endian machine.
 *
 * CAUTION: It is also assumed that the Headers are AT LEAST 16bit aligned.
 *
 *******************************************************************************
 */

#if defined(CONFIG_CPU_BIG_ENDIAN)
#define BE_DECL(declarations)   declarations
#define BE_CODE(statements)     do { statements } while (0)
#define LE_DECL(declarations)
#define LE_CODE(statements)     NULL_STMT
#elif defined(CONFIG_CPU_LITTLE_ENDIAN) || defined(CONFIG_ARM)
#define BE_DECL(declarations)
#define BE_CODE(statements)     NULL_STMT
#define LE_DECL(declarations)   declarations
#define LE_CODE(statements)     do { statements } while (0)
#else
#error "Compile: fix endianess in platform.h"
#endif


/*----- ETH_TYPE: Standard well-defined Ethernet Encapsulations --------------*/
#define BLOG_ETH_P_ETH_BRIDGING 0x6558  /* Transparent Ethernet bridging      */
#define BLOG_ETH_P_IPV4         0x0800  /* IPv4 in Ethernet                   */
#define BLOG_ETH_P_ARP          0x0806  /* Address Resolution packet          */
#define BLOG_ETH_P_RARP         0x8035  /* Reverse ARP                        */
#define BLOG_ETH_P_8021Q        0x8100  /* 802.1Q VLAN Extended Header        */
#define BLOG_ETH_P_8021AD       0x88A8  /* VLAN Stacking 802.1ad              */
#define BLOG_ETH_P_IPV6         0x86DD  /* Internet Protocol Version 6        */
#define BLOG_ETH_P_MPLS_UC      0x8847  /* MPLS - Unicast                     */
#define BLOG_ETH_P_MPLS_MC      0x8848  /* MPLS - Multicast                   */
#define BLOG_ETH_P_ATMMPOA      0x884c  /* MultiProtocol Over ATM             */
#define BLOG_ETH_P_PPP_DIS      0x8863  /* PPPoE Discovery                    */
#define BLOG_ETH_P_PPP_SES      0x8864  /* PPPoE Session                      */
#define BLOG_ETH_JUMBO_FRAME    0x8870  /* Jumbo frame indicator              */
#define BLOG_ETH_P_BRCM6TAG     0x8874  /* BRCM Switch Hdr : 6 byte           */
#define BLOG_ETH_P_BRCM4TAG     0x888A  /* BRCM Switch Hdr : 4 byte           */
#define BLOG_ETH_P_PAUSE        0x8808  /* IEEE Pause frames. 802.3 31B       */
#define BLOG_ETH_P_SLOW         0x8809  /* Slow Protocol. See 802.3ad 43B     */
#define BLOG_ETH_P_8021AG       0x8902  /* 802.1ag Connectivity FaultMgmt     */
                                            /* ITU-T recomm Y.1731 (OAM)      */
#define BLOG_ETH_FCOE           0x8906  /* Fibre Channel over Ethernet        */
#define BLOG_ETH_FCOE_INIT      0x8914  /* FCoE Initialization Protocol       */
#define BLOG_ETH_QINQ1          0x9100  /* 802.1Q in Q, alternate 1           */
#define BLOG_ETH_QINQ2          0x9200  /* 802.1Q in Q, alternate 2           */

/*----- PPP_TYPE: Standard well-defined PPP Encapsulations -------------------*/
#define BLOG_PPP_IPV4           0x0021  /* IPv4 in PPP                        */
#define BLOG_PPP_IPCP           0x8021  /* IP Control Protocol                */
#define BLOG_PPP_LCP            0xC021  /* Link Control Protocol              */
#define BLOG_PPP_MP             0x003D  /* Multilink protocol                 */
#define BLOG_PPP_IPV6           0x0057  /* IPv6 in PPP                        */
#define BLOG_PPP_IPV6CP         0x8057  /* IPv6 Control Protocol              */
#define BLOG_PPP_MPLSCP         0x80FD  /* MPLS Control Protocol???           */
#define BLOG_PPP_MPLS_UC        0x0281  /* MPLS - Unicast                     */
#define BLOG_PPP_MPLS_MC        0x0283  /* MPLS - Multicast                   */

#define BLOG_GRE_PPP            0x880B  /* PPTP: PPP in GRE Tunnel            */

/*----- IPPROTO: Standard well-defined IP Encapsulations ---------------------*/
#define BLOG_IPPROTO_HOPOPTV6   0       /* IPv6 ext: Hop-by-Hop Option Header */
#define BLOG_IPPROTO_ICMP       1       /* Internet Control Message Protocol  */
#define BLOG_IPPROTO_IGMP       2       /* Internet Group Management Protocol */
#define BLOG_IPPROTO_IPIP       4       /* IPIP tunnels e.g. 4in6             */
#define BLOG_IPPROTO_TCP        6       /* Transmission Control Protocol      */
#define BLOG_IPPROTO_EGP        8       /* Exterior Gateway Protocol          */
#define BLOG_IPPROTO_UDP        17      /* User Datagram Protocol             */
#define BLOG_IPPROTO_IPV6       41      /* IPv6-in-IPv4 tunnelling            */
#define BLOG_IPPROTO_ROUTING    43      /* IPv6 ext: Routing Header           */
#define BLOG_IPPROTO_FRAGMENT   44      /* IPv6 ext: Fragmentation Header     */
#define BLOG_IPPROTO_RSVP       46      /* RSVP Protocol                      */
#define BLOG_IPPROTO_GRE        47      /* Cisco GRE tunnels (rfc 1701,1702)  */
#define BLOG_IPPROTO_ESP        50      /* Encapsulation Security Payload     */
#define BLOG_IPPROTO_AH         51      /* Authentication Header Protocol     */
#define BLOG_IPPROTO_ICMPV6     58      /* IPv6 ext: ICMPv6 Header            */
#define BLOG_IPPROTO_NONE       59      /* IPv6 ext: NONE                     */
#define BLOG_IPPROTO_DSTOPTS    60      /* IPv6 ext: Destination Options Hdr  */
#define BLOG_IPPROTO_ANY_HOST_INTERNAL_PROTO   61  /* Any host internel proto */
#define BLOG_IPPROTO_MTP        92      /* IPv6 ext: Mcast Transport Protocol */
#define BLOG_IPPROTO_ENCAP      98      /* IPv6 ext: Encapsulation Header     */
#define BLOG_IPPROTO_PIM        103     /* Protocol Independent Multicast     */
#define BLOG_IPPROTO_COMP       108     /* Compression Header Protocol        */
#define BLOG_IPPROTO_ANY_0HOP   114     /* Any Zero HOP                       */
#define BLOG_IPPROTO_SCTP       132     /* Stream Control Transport Protocol  */
#define BLOG_IPPROTO_UDPLITE    136     /* UDP-Lite (RFC 3828)                */

#define BLOG_IPPROTO_UNASSIGN_B 141     /* Begin of unassigned range          */
#define BLOG_IPPROTO_UNASSIGN_E 252     /* End of unassigned range            */
#define BLOG_IPPROTO_RSVD_EXPT1 253     /* Reserved for experimentation       */
#define BLOG_IPPROTO_RSVD_EXPT2 254     /* Reserved for experimentation       */
#define BLOG_IPPROTO_RAW        255     /* Raw IP Packets                     */


/* IGRS/UPnP using Simple Service Discovery Protocol SSDP over HTTPMU         */
#define BLOG_HTTP_MCAST_UDP_DSTPORT 1900

/* Known L4 Ports */
#define BLOG_DNS_SERVER_PORT      53
#define BLOG_DHCP_SERVER_PORT     67
#define BLOG_DHCP_CLIENT_PORT     68

/*----- Ethernet IEEE 802.3 definitions ------------------------------------- */
#define BLOG_LLC_SAP_SNAP       (0xAA)
#define BLOG_LLC_SNAP_8023_DSAP (BLOG_LLC_SAP_SNAP)
#define BLOG_LLC_SNAP_8023_SSAP (BLOG_LLC_SAP_SNAP)
#define BLOG_LLC_SNAP_8023_Ctrl (0x3)
#define BLOG_LLC_SNAP_8023_LEN  6

#define BLOG_ETH_ADDR_LEN       6
#define BLOG_ETH_TYPE_LEN       sizeof(uint16_t)
#define BLOG_ETH_HDR_LEN        ((BLOG_ETH_ADDR_LEN * 2) + BLOG_ETH_TYPE_LEN)
#define BLOG_ETH_TYPE_MIN       1536

#define BLOG_ETH_MIN_LEN        60
#define BLOG_ETH_FCS_LEN        4
#define BLOG_ETH_MTU_LEN        0xFFFF    /* Initial minMtu value               */

#define BLOG_ETH_ADDR_FMT       "[%02X:%02X:%02X:%02X:%02X:%02X]"
#define BLOG_ETH_ADDR(e)        e.u8[0],e.u8[1],e.u8[2],e.u8[3],e.u8[4],e.u8[5]

typedef union BlogEthAddr {
    uint8_t      u8[BLOG_ETH_ADDR_LEN];
    uint16_t    u16[BLOG_ETH_ADDR_LEN/sizeof(uint16_t)];
} BlogEthAddr_t;

typedef struct BlogEthHdr {
    union {
        uint8_t     u8[BLOG_ETH_HDR_LEN];
        uint16_t   u16[BLOG_ETH_HDR_LEN/sizeof(uint16_t)];
        struct {
            BlogEthAddr_t macDa;
            BlogEthAddr_t macSa;
    /*
     * CAUTION: Position of ethType field of an Ethernet header depends on
     * the presence and the number of VLAN Tags
     * E.g. A single tagged Ethernet frame will have the ethType at offset 16.
     */
            uint16_t    ethType;    /* or length */
        };
    };
} BlogEthHdr_t;

/* 16bit aligned access MAC Address functgions */
static inline int blog_is_zero_eth_addr(uint8_t * addr_p)
{
    uint16_t * u16_p = (uint16_t *)addr_p;  /* assert u16_p is 16bit aligned */
    return ( (u16_p[0] & u16_p[1] & u16_p[2]) == 0x0000 );
}

static inline int blog_is_bcast_eth_addr(uint8_t * addr_p)
{
    uint16_t * u16_p = (uint16_t *)addr_p;  /* assert u16_p is 16bit aligned */
    return ( (u16_p[0] & u16_p[1] & u16_p[2]) == 0xFFFF );
}

/* Caution an IP mcast over PPPoE need not have a mcast MacDA */
static inline int blog_is_mcast_eth_addr(uint8_t * addr_p)
{
#if 1
    return *(addr_p+0) & 0x01;
#else   /* Multicast (e.g. over PPPoE) may use unicast MacDA */
    uint16_t * u16_p = (uint16_t *)addr_p;  /* assert u16_p is 16bit aligned */
    if ( ((u16_p[0] == 0x0100)              /* IPv4: 01:00:5E:`1b0 */
           && (*(addr_p+2) == 0x5e) && ((*(addr_p+3) & 0x80) == 0) )
       || ( u16_p[0] == 0x3333)             /* IPv6: 33:33 */
       )
        return 1;
    else
        return 0;
#endif
}

static inline int blog_cmp_eth_addr(uint8_t * addr1_p, uint8_t * addr2_p)
{
    uint16_t *a1 = (uint16_t *)addr1_p;
    uint16_t *a2 = (uint16_t *)addr2_p;
    return ( ((a1[0] ^ a2[0]) | (a1[1] ^ a2[1]) | (a1[2] ^ a2[2])) != 0 );
}


/*----- 6Byte Brcm6Hdr layout for 5397/98 Switch Management Port Tag ---------*/
#define BLOG_BRCM6_HDR_LEN      6

typedef struct BlogBrcm6Hdr {
    union {
        uint8_t     u8[BLOG_BRCM6_HDR_LEN];
        uint16_t   u16[BLOG_BRCM6_HDR_LEN/sizeof(uint16_t)];
            /*
             * egress:          opcode:3, fbcount:14, rsvd:11, srcPortId:4
             * ingress_port     opcode:3, rsvd:25, dstPortId:4
             * ingress_map      opcode:3, rsvd:20, fwdMap:9
             */
    };
} BlogBrcm6Hdr_t;


/*----- 4Byte Brcm4Hdr layout for 53115 Switch Management Port Tag -----------*/
#define BLOG_BRCM4_HDR_LEN      4

typedef struct BlogBrcm4Hdr {
    union {
        uint8_t      u8[BLOG_BRCM4_HDR_LEN];
        uint16_t    u16[BLOG_BRCM4_HDR_LEN/sizeof(uint16_t)];
        /*
         * egress       opcode:3, rsvd:13, rsvd2:2,
         *              flooding:1, snooping:1, protocol:1, switching:1
         *              learning:1, mirroring:1, tclass:3, srcpid:5
         * ingress      opcode:3, tclass:3,
         *              tagenforce:2, rsvd:1, dstmap:23
         */
    };
} BlogBrcm4Hdr_t;

/*----- Composite Ethernet with BRCM Tag -------------------------------------*/

#define BLOG_ETHBRCM6_HDR_LEN   (BLOG_ETH_HDR_LEN + BLOG_BRCM6_HDR_LEN)
#define BLOG_ETHBRCM4_HDR_LEN   (BLOG_ETH_HDR_LEN + BLOG_BRCM4_HDR_LEN)

typedef struct BlogEthBrcm6Hdr {
    union {
        uint8_t      u8[BLOG_ETHBRCM6_HDR_LEN];
        uint16_t    u16[BLOG_ETHBRCM6_HDR_LEN/sizeof(uint16_t)];
        struct {
            BlogEthAddr_t   macDa;
            BlogEthAddr_t   macSa;
            BlogBrcm6Hdr_t  brcm6;
            uint16_t        ethType;
        };
    };
} BlogEthBrcm6Hdr_t;

typedef struct BlogEthBrcm4Hdr {
    union {
        uint8_t      u8[BLOG_ETHBRCM4_HDR_LEN];
        uint16_t    u16[BLOG_ETHBRCM4_HDR_LEN/sizeof(uint16_t)];
        struct {
            BlogEthAddr_t   macDa;
            BlogEthAddr_t   macSa;
            BlogBrcm4Hdr_t  brcm4;
            uint16_t        ethType;
        };
    };
} BlogEthBrcm4Hdr_t;


/*----- Vlan IEEE 802.1Q definitions -----------------------------------------*/
#define BLOG_VLAN_HDR_LEN       4
#define BLOG_VLAN_HDR_FMT       "[0x%08X] tpid<0x%04X> tci<0x%04X> "\
                                "pbit<%u> dei<%u> vid<0x%03X>"
#define BLOG_VLAN_HDR(v)        v.u32[0], v.tpid, v.tci.u16[0], \
                                v.tci.pbits, v.tci.dei, v.tci.vid

typedef struct BlogVlanTci {
    union {
        uint8_t     u8[sizeof(uint16_t)];
        uint16_t    u16[1];
        struct {
            BE_DECL( uint16_t pbits:3; uint16_t dei:1; uint16_t vid:12; )
            LE_DECL( uint16_t vid:12; uint16_t dei:1; uint16_t pbits:3; )
        };
    };
} BlogVlanTci_t;

typedef struct BlogVlanHdr {
    union {
        uint8_t      u8[BLOG_VLAN_HDR_LEN];
        uint16_t    u16[BLOG_VLAN_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_VLAN_HDR_LEN/sizeof(uint32_t)];
        struct {
            uint16_t tpid; BlogVlanTci_t tci; /* u8[ 88, A8, EA, AA ] */
        };
    };
} BlogVlanHdr_t;


/*----- PPPoE + PPP Header layout. PPPoE RFC 2516, PPP RFC 1661 --------------*/
#define BLOG_PPPOE_HDR_LEN      8   /* Including PPP Header "PPP Type" */
#define BLOG_PPP_HDR_LEN        sizeof(uint16_t)
#define BLOG_PPPOE_HDR_FMT      "[0x%08X 0x%08X] ver<%u> type<%u> code<0x%02X>"\
                                " sId<0x%04X> len<%u> pppType<0x%04X>"
#define BLOG_PPPOE_HDR(p)       p.u32[0], p.u32[1], p.ver, p.type, p.code,\
                                p.sId, p.len, p.pppType

typedef uint16_t BlogPppHdr_t;

typedef struct BlogPppoeHdr {   /* includes 2 byte PPP Type */
    union {
        uint8_t      u8[BLOG_PPPOE_HDR_LEN];
        uint16_t    u16[BLOG_PPPOE_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_PPPOE_HDR_LEN/sizeof(uint32_t)];
        struct {
            BE_DECL( uint16_t ver:4; uint16_t type:4; uint16_t code:8; )
            LE_DECL( uint16_t code:8; uint16_t type:4; uint16_t ver:4; )
            uint16_t sId; uint16_t len; BlogPppHdr_t pppType;
        };
    };
} BlogPppoeHdr_t;


/*----- Multi Protocol Label Switiching Architecture: RFC 3031 -----------------
 *
 * 20b-label, 3b-tos, 1b-Stack, 8b-TTL
 * StackBit==1? if label==0 then next is IPV4, if label==1 then next is IPV6
 *------------------------------------------------------------------------------
 */
#define BLOG_MPLS_HDR_LEN       4

typedef struct BlogMplsHdr {
    union {
        uint8_t   u8[BLOG_MPLS_HDR_LEN];
        uint16_t u16[BLOG_MPLS_HDR_LEN/sizeof(uint16_t)];
        uint32_t u32[BLOG_MPLS_HDR_LEN/sizeof(uint32_t)];
        struct {
            BE_DECL( uint32_t label:20; uint32_t cos:3; uint32_t sbit:1; uint32_t ttl:8; )
            LE_DECL( uint32_t ttl:8; uint32_t sbit:1; uint32_t cos:3; uint32_t label:20; )
        };
    };
} BlogMplsHdr_t;


/*----- IPv4: RFC 791 definitions --------------------------------------------*/
#define BLOG_IPV4_HDR_LEN       20  /* Not including IP Options */
#define BLOG_IPV4_ADDR_LEN      4
#define BLOG_IPV4_HDR_FMT       "[0x%08X] ver<%u> ihl<%u> tos<0x%02X> len<%u> "\
                                "[0x%08X] id<%u> df<%u> mf<%u> "\
                                "fragOffset<0x%04X> [0x%08X] "\
                                "ttl<%u> proto<%u> chkSum<0x%04X> "\
#define BLOG_IPV4_HDR(i)        i.u32[0], i.ver, i.ihl, i.tos, i.len, \
                                i.u32[1], i.id, i.df, i.mf, i.fragOffset,\
                                i.u32[2], i.ttl, i.proto, i.chkSum,
#define BLOG_IPTOS2DSCP(tos)    ((tos) >> 2)
#define BLOG_IPDSCP2TOS(dscp)   ((dscp) << 2)

#define BLOG_IPV4_ADDR_FMT      "<%03u.%03u.%03u.%03u>"
#define BLOG_IPV4_ADDR_PORT_FMT "<%03u.%03u.%03u.%03u:%u>"
#define BLOG_IPV4_ADDR(ip)      ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1],     \
                                ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]

#if defined(CONFIG_CPU_BIG_ENDIAN)
#define BLOG_IPV4_ADDR_HOST(ip)      ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1], \
                                     ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]
#elif defined(CONFIG_CPU_LITTLE_ENDIAN) || defined(CONFIG_ARM)
#define BLOG_IPV4_ADDR_HOST(ip)      ((uint8_t*)&ip)[3], ((uint8_t*)&ip)[2], \
                                     ((uint8_t*)&ip)[1], ((uint8_t*)&ip)[0]
#endif

typedef union BlogIpv4Addr {
    uint8_t   u8[BLOG_IPV4_ADDR_LEN];
    uint16_t u16[BLOG_IPV4_ADDR_LEN/sizeof(uint16_t)];
    uint32_t u32[BLOG_IPV4_ADDR_LEN/sizeof(uint32_t)];
} BlogIpv4Addr_t;

#define BLOG_IP_FLAG_CE         0x8000      /* Congestion */
#define BLOG_IP_FLAG_DF         0x4000      /* Do Not Fragment */
#define BLOG_IP_FLAG_MF         0x2000      /* More Fragment */
#define BLOG_IP_FRAG_OFFSET     0x1FFF 

typedef struct BlogIpv4Hdr {
    union {
        uint8_t      u8[BLOG_IPV4_HDR_LEN];
        uint16_t    u16[BLOG_IPV4_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_IPV4_HDR_LEN/sizeof(uint32_t)];
        struct {
            union {
                struct {
                    BE_DECL( uint8_t ver:4; uint8_t ihl:4; ) 
                    LE_DECL( uint8_t ihl:4; uint8_t ver:4; )
                };
                uint8_t ver_ihl;
            };
            uint8_t   tos; uint16_t len;
            uint16_t  id;
            union {
                uint16_t flagsFrag;
                struct {
                    BE_DECL( uint16_t cong:1; uint16_t df:1; 
                             uint16_t moreFrag:1; uint16_t fragOffset:13; )
                    LE_DECL( uint16_t fragOffset:13; uint16_t moreFrag:1; 
                             uint16_t df:1; uint16_t cong:1; )
                };
            };
            uint8_t ttl; uint8_t proto; uint16_t chkSum;
            BlogIpv4Addr_t  sAddr;
            BlogIpv4Addr_t  dAddr;
        };
    };
} BlogIpv4Hdr_t;


/*----- IPv6: RFC 2460 RFC 3513 definitions ----------------------------------*/
/*
  * Well know IPv6 Address prefixes
 *      Multicast:   FFXX::
 *      Site local:  FEC0::
 *      Link Local:  FE80::
 *      Ucast 6to4:  2002::
 */
#define BLOG_IPV6_HDR_LEN       40
#define BLOG_IPV6_ADDR_LEN      16

#define BLOG_IPV6_ADDR_FMT      "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x>"
#define BLOG_IPV6_ADDR_PORT_FMT "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%u>"
#define BLOG_IPV6_ADDR(ip)      \
    ntohs(((uint16_t*)&ip)[0]), ntohs(((uint16_t*)&ip)[1]),   \
    ntohs(((uint16_t*)&ip)[2]), ntohs(((uint16_t*)&ip)[3]),   \
    ntohs(((uint16_t*)&ip)[4]), ntohs(((uint16_t*)&ip)[5]),   \
    ntohs(((uint16_t*)&ip)[6]), ntohs(((uint16_t*)&ip)[7])

typedef union BlogIpv6Addr {
    uint8_t   u8[BLOG_IPV6_ADDR_LEN];
    uint16_t  u16[BLOG_IPV6_ADDR_LEN/sizeof(uint16_t)];
    uint32_t  u32[BLOG_IPV6_ADDR_LEN/sizeof(uint32_t)];
} BlogIpv6Addr_t;

typedef struct BlogIpv6Hdr {
    union {
        uint8_t      u8[BLOG_IPV6_HDR_LEN];
        uint16_t    u16[BLOG_IPV6_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_IPV6_HDR_LEN/sizeof(uint32_t)];
        struct {
            /* ver_tos bits -> ver 4: tos 8: flowlblHi 4 
               using bit field results in unaligned access */
            uint16_t ver_tos; uint16_t flowLblLo;
            uint16_t len; uint8_t nextHdr; uint8_t hopLmt;
            BlogIpv6Addr_t  sAddr;
            BlogIpv6Addr_t  dAddr;
        };
    };
} BlogIpv6Hdr_t;

#define BLOG_IPV6EXT_HDR_LEN    8   /* multiple of 8 octets */
typedef struct BlogIpv6ExtHdr {
    union {
        uint8_t      u8[BLOG_IPV6EXT_HDR_LEN];
        uint16_t    u16[BLOG_IPV6EXT_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_IPV6EXT_HDR_LEN/sizeof(uint32_t)];
        struct {
            uint8_t nextHdr; uint8_t hdrLen; uint16_t data16;
            uint32_t data32;
        };
    };
} BlogIpv6ExtHdr_t;


/*----- Transmission Control Protocol: RFC 793 definitions -------------------*/

#define BLOG_TCP_HDR_LEN        20

#define TCPH_DOFF(t)            (((htons(t->offFlags.u16)) >> 12) & 0xF)
#define TCPH_CWR(t)             (((htons(t->offFlags.u16)) >>  7) & 0x1)
#define TCPH_ECE(t)             (((htons(t->offFlags.u16)) >>  6) & 0x1)
#define TCPH_URG(t)             (((htons(t->offFlags.u16)) >>  5) & 0x1)
#define TCPH_ACK(t)             (((htons(t->offFlags.u16)) >>  4) & 0x1)
#define TCPH_PSH(t)             (((htons(t->offFlags.u16)) >>  3) & 0x1)
#define TCPH_RST(t)             (((htons(t->offFlags.u16)) >>  2) & 0x1)
#define TCPH_SYN(t)             (((htons(t->offFlags.u16)) >>  1) & 0x1)
#define TCPH_FIN(t)             (((htons(t->offFlags.u16)) >>  0) & 0x1)

typedef struct BlogTcpOffFlags {
    union {
        uint16_t u16;
        struct { uint8_t off; uint8_t flags; };
        struct {
            BE_DECL(
                uint16_t   dOff:   4;
                uint16_t   res1:   4;
                uint16_t   cwr :   1;
                uint16_t   ece :   1;
                uint16_t   urg :   1;
                uint16_t   ack :   1;
                uint16_t   psh :   1;
                uint16_t   rst :   1;
                uint16_t   syn :   1;
                uint16_t   fin :   1;
            )
            LE_DECL(
                uint16_t   fin :   1;
                uint16_t   syn :   1;
                uint16_t   rst :   1;
                uint16_t   psh :   1;
                uint16_t   ack :   1;
                uint16_t   urg :   1;
                uint16_t   ece :   1;
                uint16_t   cwr :   1;
                uint16_t   res1:   4;
                uint16_t   dOff:   4;
            )
        };
    };
} BlogTcpOffFlags_t;

typedef struct BlogTcpHdr {
    union {
        uint8_t      u8[BLOG_TCP_HDR_LEN];
        uint16_t    u16[BLOG_TCP_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_TCP_HDR_LEN/sizeof(uint32_t)];
        struct {
            uint16_t sPort; uint16_t dPort;
            uint32_t seq;
            uint32_t ackSeq;
            BlogTcpOffFlags_t offFlags; uint16_t window;
            uint16_t chkSum; uint16_t urgPtr;
        };
    };
} BlogTcpHdr_t;


/*----- User Datagram Protocol: RFC 768 definitions --------------------------*/
#define BLOG_UDP_HDR_LEN        8

typedef struct BlogUdpHdr {
    union {
        uint8_t      u8[BLOG_UDP_HDR_LEN];
        uint16_t    u16[BLOG_UDP_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_UDP_HDR_LEN/sizeof(uint32_t)];
        struct {
            uint16_t sPort; uint16_t dPort;
            uint16_t len; uint16_t chkSum;
        };
    };
} BlogUdpHdr_t;


/*----- L2TP: RFC 2661 definitions -------------------------------------------*/
#define BLOG_L2TP_HDR_LEN       8

typedef struct BlogL2tpIeFlagsVer {
    union {
        uint16_t u16;
        struct {
            BE_DECL(
                uint16_t   type   : 1;
                uint16_t   lenIe  : 1;
                uint16_t   rsvd2  : 2;
                uint16_t   seqIe  : 1;
                uint16_t   rsvd1  : 1;
                uint16_t   offIe  : 1;
                uint16_t   prio   : 1;
                uint16_t   rsvd4  : 4;
                uint16_t   ver    : 4;
            )
            LE_DECL(
                uint16_t   ver    : 4;
                uint16_t   rsvd4  : 4;
                uint16_t   prio   : 1;
                uint16_t   offIe  : 1;
                uint16_t   rsvd1  : 1;
                uint16_t   seqIe  : 1;
                uint16_t   rsvd2  : 2;
                uint16_t   lenIe  : 1;
                uint16_t   type   : 1;
            )
        };
    };
} BlogL2tpIeFlagsVer_t;

typedef struct BlogL2tpHdr {
    union {
        uint8_t      u8[BLOG_L2TP_HDR_LEN];
        uint16_t    u16[BLOG_L2TP_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_L2TP_HDR_LEN/sizeof(uint32_t)];
        struct {
            BlogL2tpIeFlagsVer_t ieFlagsVer; uint16_t len;
            uint16_t tId; uint16_t sId;
            /* uint16_t ns; uint16_t nr;
               uint16_t offSz; uint16_t offPad; */
        };
    };
} BlogL2tpHdr_t;


/*----- Generic Routing Encapsulation: RFC 2637, PPTP session, RFC 2784 ------*/
#define BLOG_GRE_HDR_LEN        8

typedef struct BlogGreIeFlagsVer {
    union {
        uint16_t    u16;
        struct {
            BE_DECL(
                uint16_t   csumIe : 1;
                uint16_t   rtgIe  : 1;
                uint16_t   keyIe  : 1;
                uint16_t   seqIe  : 1;
                uint16_t   srcRtIe: 1;
                uint16_t   recurIe: 3;
                uint16_t   ackIe  : 1;
                uint16_t   flags  : 4;
                uint16_t   ver    : 3;
            )
            LE_DECL(
                uint16_t   ver    : 3;
                uint16_t   flags  : 4;
                uint16_t   ackIe  : 1;
                uint16_t   recurIe: 3;
                uint16_t   srcRtIe: 1;
                uint16_t   seqIe  : 1;
                uint16_t   keyIe  : 1;
                uint16_t   rtgIe  : 1;
                uint16_t   csumIe : 1;
            )
        };
    };
} BlogGreIeFlagsVer_t;

typedef struct BlogGreHdr {
    union {
        uint8_t      u8[BLOG_GRE_HDR_LEN];
        uint16_t    u16[BLOG_GRE_HDR_LEN/sizeof(uint16_t)];
        uint32_t    u32[BLOG_GRE_HDR_LEN/sizeof(uint32_t)];
        struct {
            BlogGreIeFlagsVer_t ieFlagsVer; uint16_t proto;
            /* RFC2784 specifies csum instead of len, for GRE ver = 0 */
            /* RFC2637 specifies len, for GRE ver=1 used with PPTP    */
            uint16_t len; uint16_t callId;
            /* uint32_t seqNum; present if seqIe = 1 */
            /* uint32_t ackNum; present if ackIe = 1 */
        };
    };
} BlogGreHdr_t;

/*
 *------------------------------------------------------------------------------
 *  Assert that headers are properly packed (without using attribute packed) 
 *
 *  #include <stdio.h>
 *  #include <stdint.h>
 *  #include "blog_net.h"
 *  int main() {
 *      printf("blog_net_audit_hdrs %d\n", blog_net_audit_hdrs() );
 *      return blog_net_audit_hdrs();
 *  }
 *------------------------------------------------------------------------------
 */
static inline int blog_net_audit_hdrs(void)
{
#define BLOG_NET_AUDIT(hdrlen,hdrtype)  \
    if (hdrlen != sizeof(hdrtype))      \
        return (-1)

    BLOG_NET_AUDIT( BLOG_ETH_ADDR_LEN, BlogEthAddr_t );
    BLOG_NET_AUDIT( BLOG_ETH_HDR_LEN, BlogEthHdr_t );
    BLOG_NET_AUDIT( BLOG_BRCM6_HDR_LEN, BlogBrcm6Hdr_t );
    BLOG_NET_AUDIT( BLOG_BRCM4_HDR_LEN, BlogBrcm4Hdr_t );
    BLOG_NET_AUDIT( BLOG_ETHBRCM6_HDR_LEN, BlogEthBrcm6Hdr_t );
    BLOG_NET_AUDIT( BLOG_ETHBRCM4_HDR_LEN, BlogEthBrcm4Hdr_t );
    BLOG_NET_AUDIT( BLOG_VLAN_HDR_LEN, BlogVlanHdr_t );
    BLOG_NET_AUDIT( BLOG_PPPOE_HDR_LEN, BlogPppoeHdr_t );
    BLOG_NET_AUDIT( BLOG_MPLS_HDR_LEN, BlogMplsHdr_t );
    BLOG_NET_AUDIT( BLOG_IPV4_ADDR_LEN, BlogIpv4Addr_t );
    BLOG_NET_AUDIT( BLOG_IPV4_HDR_LEN, BlogIpv4Hdr_t );
    BLOG_NET_AUDIT( BLOG_IPV6_ADDR_LEN, BlogIpv6Addr_t );
    BLOG_NET_AUDIT( BLOG_IPV6_HDR_LEN, BlogIpv6Hdr_t );
    BLOG_NET_AUDIT( BLOG_TCP_HDR_LEN, BlogTcpHdr_t );
    BLOG_NET_AUDIT( BLOG_UDP_HDR_LEN, BlogUdpHdr_t );
    BLOG_NET_AUDIT( BLOG_L2TP_HDR_LEN, BlogL2tpHdr_t );
    BLOG_NET_AUDIT( BLOG_GRE_HDR_LEN, BlogGreHdr_t );

    return 0;
}


/*
 *------------------------------------------------------------------------------
 * Network Utilities  : 16bit aligned
 *------------------------------------------------------------------------------
 */
#if defined(CONFIG_CPU_LITTLE_ENDIAN) || defined(CONFIG_ARM)
/*
 *------------------------------------------------------------------------------
 * Function     : blog_read32_align16
 * Description  : Read a 32bit value from a 16 byte aligned data stream
 *------------------------------------------------------------------------------
 */
static inline uint32_t blog_read32_align16( uint16_t * from )
{
    return (uint32_t)( (from[1] << 16) | from[0] );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_write32_align16
 * Description  : Write a 32bit value to a 16bit aligned data stream
 *------------------------------------------------------------------------------
 */
static inline void blog_write32_align16( uint16_t * to, uint32_t from )
{
    to[1] = (uint16_t)htons(from >> 16);
    to[0] = (uint16_t)htons(from >> 0);
}

#elif defined(CONFIG_CPU_BIG_ENDIAN)

/*
 *------------------------------------------------------------------------------
 * Function     : blog_read32_align16
 * Description  : Read a 32bit value from a 16 byte aligned data stream
 *------------------------------------------------------------------------------
 */
static inline uint32_t blog_read32_align16( uint16_t * from )
{
    return (uint32_t)( (from[0] << 16) | (from[1]) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_write32_align16
 * Description  : Write a 32bit value to a 16bit aligned data stream
 *------------------------------------------------------------------------------
 */
static inline void blog_write32_align16( uint16_t * to, uint32_t from )
{
    to[0] = (uint16_t)(from >> 16);
    to[1] = (uint16_t)(from >>  0);
}
#endif /* defined(CONFIG_CPU_BIG_ENDIAN) */

#endif /* defined(__BLOG_NET_H_INCLUDED__) */
#endif
