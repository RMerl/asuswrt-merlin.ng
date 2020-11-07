#ifndef __FAP4KE_PACKET_H_INCLUDED__
#define __FAP4KE_PACKET_H_INCLUDED__

/*

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved

<:label-BRCM:2011:DUAL/GPL:standard

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
 * File Name  : fap4ke_packet.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

//#define CC_FAP4KE_PKT_FFE

//#define CC_FAP4KE_PKT_TEST

//#define CC_FAP4KE_PKT_HW_ICSUM

#define CC_FAP4KE_PKT_ERROR_CHECK

#define CC_FAP4KE_PKT_IPV6_GSO


//#define FAP4KE_MCAST_DEBUG_ENABLE

#if defined(FAP4KE_MCAST_DEBUG_ENABLE)
#define FAP4KE_MCAST_DEBUG(fmt, arg...) fap4kePrt_Print(fmt "\n", ##arg)
#define FAP4KE_MCAST_DUMP_PACKET(_packet_p) dumpHeader((uint8 *)(_packet_p))
#else
#define FAP4KE_MCAST_DEBUG(fmt, arg...)
#define FAP4KE_MCAST_DUMP_PACKET(_packet_p)
#endif


#if defined(CONFIG_BCM_FAP_IPV6)
#define CC_FAP4KE_PKT_IPV6_SUPPORT
#endif

//#define CC_FAP4KE_PKT_IPV6_FRAGMENTATION

#ifdef FAP_4KE
                                            /* First encapsulation type */
#define TYPE_ETH                    0x0000  /* LAN: ETH, WAN: EoA, MER, PPPoE */
#define TYPE_PPP                    0x0001  /*           WAN: PPPoA */
#define TYPE_IP                     0x0002  /*           WAN: IPoA */

#define TYPE_PPP_IP                 0x0021  /* IP in PPP */
#define TYPE_PPP_IPV6               0x0057  /* IPv6 in PPP */
#define TYPE_ETH_P_IP               0x0800  /* IP in Ethernet */
#define TYPE_ETH_P_8021Q            0x8100  /* VLAN in Ethernet */
#define TYPE_ETH_P_8021Qad          0x88A8  /* VLAN in Ethernet */
#define TYPE_ETH_P_PPP_SES          0x8864  /* PPPoE in Ethernet */
#define TYPE_ETH_P_BCM              0x8874  /* BCM Switch Hdr */
#define TYPE_ETH_P_BCM2             0x888A  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_IPV6             0x86DD  /* IPv6 in Ethernet */
#define TYPE_ETH_P_HPAV             0x8912  /* HPAV */

#define TYPE_ETH_P_BCM2_PRIO0       0x2000  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_BCM2_PRIO1       0x2400  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_BCM2_PRIO2       0x2800  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_BCM2_PRIO3       0x2C00  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_BCM2_PRIO4       0x3000  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_BCM2_PRIO5       0x3400  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_BCM2_PRIO6       0x3800  /* BCM Switch Hdr for ext switch */
#define TYPE_ETH_P_BCM2_PRIO7       0x3C00  /* BCM Switch Hdr for ext switch */

/* IPv4 Dot Decimal Notation formating */
#define IP4DDN   " <%03u.%03u.%03u.%03u>"
#define IP4PDDN  " <%03u.%03u.%03u.%03u:%05u>"
#define IP4(ip) ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1], ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]

#define IP6HEX  "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x>"
#define IP6PHEX "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%u>"
#define IP6(ip) ((uint16_t*)&ip)[0], ((uint16_t*)&ip)[1],   \
                ((uint16_t*)&ip)[2], ((uint16_t*)&ip)[3],   \
                ((uint16_t*)&ip)[4], ((uint16_t*)&ip)[5],   \
                ((uint16_t*)&ip)[6], ((uint16_t*)&ip)[7]
#endif

#define ETH_ALEN           6
#define BRCM_TAG_LENGTH    6
#define ETHERTYPE_LENGTH   2
#define IPOA_HLEN          8

#define MAX_FAP_MTU        2048 /* upto mini-jumbo frame size */
#define MIN_FAP_MTU        60

#define    FAP4KE_PKT_FT_IPV4 0
#define    FAP4KE_PKT_FT_IPV6 1
#define    FAP4KE_PKT_FT_L2   2

/* IPv4 Multicast range: 224.0.0.0 to 239.255.255.255 (E0.*.*.* to EF.*.*.*) */
#define FAP4KE_PKT_MCAST_IPV4_MASK  0xF0000000
#define FAP4KE_PKT_MCAST_IPV4_VAL   0xE0000000

#define FAP4KE_PKT_FLOW_L2_DONE     (fap4kePkt_flow_t *)(0xFFFFFFFF)
#define FAP4KE_PKT_FLOW_DROP        (fap4kePkt_flow_t *)(0xFFFFFFFE)

#define FAP4KE_PKT_IS_MCAST_IPV4(_addr)                                  \
    ( ((_addr) & FAP4KE_PKT_MCAST_IPV4_MASK) == FAP4KE_PKT_MCAST_IPV4_VAL)

#define FAP4KE_PKT_IS_L2_BCAST(_macDa16_p) ( ((_macDa16_p)[0] == 0xFFFF) && \
                                             ((_macDa16_p)[1] == 0xFFFF) && \
                                             ((_macDa16_p)[2] == 0xFFFF) )

#define FAP4KE_PKT_IS_L2_MCAST(_macDa16_p) ( (_macDa16_p)[0] & 0x0100 )

#define FAP4KE_PKT_IS_L2_MCAST_IPv4(_macDa16_p) ( ((_macDa16_p)[0] == 0x0100) && \
                                                  (((_macDa16_p)[1] & 0xFF80) == 0x5E00) )


/* IPv6 Multicast range:  FF00::/8  */
#define FAP4KE_PKT_MCAST_IPV6_VAL   0xFF

#define FAP4KE_PKT_IS_MCAST_IPV6(_addr)                                \
    ( (_addr)  == FAP4KE_PKT_MCAST_IPV6_VAL)

/* using dynamic memory -- no max command lists... */
#define FAP4KE_PKT_MAX_FLOWS             512

#define FAP4KE_PKT_HASH_TABLE_SIZE       128  /* 256 MAX! */
#define FAP4KE_PKT_HASH_TABLE_ENTRY_MAX  FAP4KE_PKT_MAX_FLOWS

#define FAP4KE_PKT_MAX_HEADERS           4

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
/* Maximum header size:
 * 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 40 (IPv6) + 20 (IPv4) + 20 (TCP/UDP) = 126 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_ENET      116 /* must be multiple of 4 */
/* Maximum header size:
 * 10 (LLC/SNAP) + 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 40 (IPv6) + 20 (IPv4) + 20 (TCP/UDP) = 126 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_XTM       128 /* must be multiple of 4 */ // 126
#define FAP4KE_PKT_CMD_LIST_SIZE         168   /* max 156 expected */
#else
/* ENET Maximum header size:
 * 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 20 (IPv4) + 20 (TCP/UDP) = 76 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_ENET      76 /* must be multiple of 4 */
/* XTM Maximum header size:
 * 10 (LLC/SNAP) + 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 20 (IPv4) + 20 (TCP/UDP) = 86 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_XTM       88 /* must be multiple of 4 */ //86
#define FAP4KE_PKT_CMD_LIST_SIZE         96
#endif
#define FAP4KE_PKT_HEADER_SIZE_MAX       FAP4KE_PKT_HEADER_SIZE_XTM

#define FAP4KE_PKT_CSUM_CMD_LIST_SIZE    20

/*
 * Only 8 bits are allocated for channel in fap4kePkt_key_t. This
 * restricts the multicast destination port mask to 8 ports.
 */
#define FAP4KE_PKT_MAX_DEST_PORTS        8
#define FAP4KE_PKT_MAX_SRC_PORTS         FAP4KE_PKT_MAX_DEST_PORTS

/*
 * Header access macros for 8-bit and 16-bit fields
 */
#define FAP4KE_PKT_HEADER_READ(_field)         \
    ({                                         \
        typeof(_field) *_field_p = &(_field);  \
        *_field_p;                             \
    })

#define FAP4KE_PKT_HEADER_WRITE(_field, _val)  \
    {                                          \
        typeof(_field) *_field_p = &(_field);  \
        *_field_p = (_val);                    \
    }

typedef enum {
    FAP4KE_PKT_LOOKUP_MISS=0,
    FAP4KE_PKT_LOOKUP_HIT,
    FAP4KE_PKT_LOOKUP_DROP,
    FAP4KE_PKT_LOOKUP_DONE,
    FAP4KE_PKT_LOOKUP_HIT_WLAN,
    FAP4KE_PKT_LOOKUP_MAX
} fap4kePkt_lookup_t;

typedef enum {
    FAP4KE_PKT_PHY_ENET=0,
    FAP4KE_PKT_PHY_XTM,
    FAP4KE_PKT_PHY_ENET_EXT,
    FAP4KE_PKT_PHY_HOST, 
    FAP4KE_PKT_PHY_GPON,
    FAP4KE_PKT_PHY_WLAN,
    FAP4KE_PKT_PHY_MAX
} fap4kePkt_phy_t;

typedef enum {
    FAP4KE_TUNNEL_NONE=0,
    FAP4KE_TUNNEL_4in6_UP,
    FAP4KE_TUNNEL_6in4_UP,
    FAP4KE_TUNNEL_4in6_DN,
    FAP4KE_TUNNEL_6in4_DN
} fap4ke_tunnel_type;

#define FAP_MAX_GSO_FRAGS 18
typedef struct {
    uint32 recycle_key;
    uint16 nr_frags;
    union{
        struct {
            uint16 isAllocated:1;
            uint16 rsvd:15;
        }; 
        uint16 flags;
    };
    uint32 totalLen;
    uint16 mss;
    uint16 rsvd_hw1;
    void *frag_data[FAP_MAX_GSO_FRAGS];
    uint16 frag_len[FAP_MAX_GSO_FRAGS]; 
}fapGsoDesc_t;

typedef struct {
    union{
        uint8 *packet_p;
        uint8 *hdr_p; 
    };

    fapGsoDesc_t *gsoDesc_p;
    fapGsoDesc_t *localGsoDesc_p;

    uint32 recycle_key;

    union{
        uint16 len;
        uint16 hdrLen;
    };

    uint16 dmaStatus;
    uint16 mss;
    uint8 bCSUM;
    uint8 encapType;

    uint8 phy;
    uint8 source;
    uint8 txChannel;
    uint8 rxChannel;

    uint8 isHighPrio : 1,
          isForwardingPath : 7;
    uint8 tunnelType;
    uint16 virtDestPortMask;
    int32 gponWord;
    uint8 virtDestPort;
    uint8 extSwTagLen;
    uint8 destQueue;
} fap4kePkt_gso_pkt;

typedef struct {
    union {
        uint8  macDa[ETH_ALEN];
        uint16 macDa16[ETH_ALEN/2];
    };
    union {
        uint8  macSa[ETH_ALEN];
        uint16 macSa16[ETH_ALEN/2];
    };
    uint16 etherType;
} fap4kePkt_ethHeader_t;

typedef struct {
    union {
        uint8  macDa[ETH_ALEN];
        uint16 macDa16[ETH_ALEN/2];
    };
    union {
        uint8  macSa[ETH_ALEN];
        uint16 macSa16[ETH_ALEN/2];
    };
    uint16 brcmTag[2];
    uint16 etherType;
} fap4kePkt_ethHeader2_t;

typedef struct {
    uint16 brcmTag[2];
    uint16 etherType;
} fap4kePkt_bcmHeader_t;

typedef struct {
    uint16 brcmTag;
    uint16 etherType;
} __attribute__((packed)) fap4kePkt_bcmHeader2_t;

#define VLAN_TCI_PBITS_MASK   0xE000
#define VLAN_TCI_DEI_MASK     0x1000
#define VLAN_TCI_VID_MASK     0x0FFF

typedef union {
    struct {
        uint16 pbits : 3;
        uint16 dei   : 1;
        uint16 vid   : 12;
    };
    uint16 u16;
} fap4kePkt_vlanHeaderTci_t;

typedef struct {
    fap4kePkt_vlanHeaderTci_t tci;
    uint16 etherType;
} fap4kePkt_vlanHeader_t;

typedef struct {
    uint16 tpid;
    fap4kePkt_vlanHeaderTci_t tci;
} __attribute__((packed))fap4kePkt_vlanHeader2_t;

typedef uint16 fap4kePkt_pppType_t;

typedef struct {
    struct {
        uint8  ver  : 4;
        uint8  type : 4;
    };
    uint8  code;
    uint16 sessionId;
    uint16 length;
    fap4kePkt_pppType_t pppType;
} fap4kePkt_pppoeHeader_t;

typedef union {
    struct {
        union {
            struct {
                uint8 version:4;
                uint8 ihl:4;
            };
            uint8 version_ihl;
        };
        uint8  tos;
        uint16 totalLength;
        uint16 id;
        uint16 fragOffset;
#define FLAGS_CE     0x8000 /* Flag: "Congestion" */
#define FLAGS_DF     0x4000 /* Flag: "Don't Fragment" */
#define FLAGS_MF     0x2000 /* Flag: "More Fragments" */
#define OFFSET_MASK  0x1FFF /* "Fragment Offset" part */
#define FLAGS_SHIFT  13
        uint8  ttl;
        uint8  protocol;
        uint16 csum;
        uint32 ipSa;
        uint32 ipDa;
        /* options... */
    };
    uint32 u32[5];
} fap4kePkt_ipv4Header_t;

/* this packed verison of header is used to avoid unaligned exceptions
 * when ipv4 header of packet is not word aligned
 *
 * NOTE: This packed header is required when accessing bit field members, 
 * however accessing data via the u32[] union member may cause alignent 
 * exceptions as the data is not guaranteed to be 32bit aligned.
 */
typedef union {
    struct {
        union {
            struct {
                uint8 version:4;
                uint8 ihl:4;
            };
            uint8 version_ihl;
        };
        uint8  tos;
        uint16 totalLength;
        uint16 id;
        uint16 fragOffset;
#define FLAGS_CE     0x8000 /* Flag: "Congestion" */
#define FLAGS_DF     0x4000 /* Flag: "Don't Fragment" */
#define FLAGS_MF     0x2000 /* Flag: "More Fragments" */
#define OFFSET_MASK  0x1FFF /* "Fragment Offset" part */
#define FLAGS_SHIFT  13
        uint8  ttl;
        uint8  protocol;
        uint16 csum;
        uint32 ipSa;
        uint32 ipDa;
        /* options... */
    };
    uint32 u32[5];
}__attribute__((packed)) fap4kePkt_packed_ipv4Hdr_t;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT) || defined(CC_FAP4KE_PKT_IPV6_GSO)
typedef union {
    struct {
        uint8   nextHeader;
        uint8   reservedU8;
        uint16  ipFragment;
        uint32  identification;
    };
    uint32 u32[2];
} fap4kePkt_ipv6_fragHeader_t;

typedef union {
    uint8  u8[16];
    uint16 u16[8];
    uint32 u32[4];
} fap4kePkt_ipv6Address_t;

typedef union {
    struct {
        union {
            struct {
                uint32 version:4;
                uint32 tclass:8;
                uint32 flowLabel:20;
            };
            uint32 word0;
        };
        uint16 payloadLen;
        uint8 nextHeader;
        uint8 hopLimit;
        fap4kePkt_ipv6Address_t ipSa;
        fap4kePkt_ipv6Address_t ipDa;
    };
    uint32 u32[10];
} fap4kePkt_ipv6Header_t;


/* this packed verison of header is used to avoid unaligned exceptions
 * when ipv6 header of packet is not word aligned
 *
 * NOTE: This packed header is required when accessing bit field members, 
 * however accessing data via the u32[] union member may cause alignent 
 * exceptions as the data is not guaranteed to be 32bit aligned.
*/
typedef union {
    struct {
        union {
            struct {
                uint32 ver:4;
                uint32 tclass:8;
                uint32 flowLabel:20;
            };
            uint32 word0;
        };
        uint16 payloadLen;
        uint8 nextHeader;
        uint8 hopLimit;
        fap4kePkt_ipv6Address_t ipSa;
        fap4kePkt_ipv6Address_t ipDa;
    };
    uint32 u32[10];
}__attribute__((packed)) fap4kePkt_packed_ipv6Hdr_t;

#define FAP4KE_PKT_IPV6_VERSION(_ipv6Header_p)   (((fap4kePkt_packed_ipv6Hdr_t *)_ipv6Header_p)->ver)
#define FAP4KE_PKT_IPV6_TCLASS(_ipv6Header_p)    (((fap4kePkt_packed_ipv6Hdr_t *)_ipv6Header_p)->tclass)
#define FAP4KE_PKT_IPV6_FLOWLABEL(_ipv6Header_p) (((fap4kePkt_packed_ipv6Hdr_t *)_ipv6Header_p)->flowLabel)

typedef struct {
    union {
        struct {
            uint8 next_hdr;
            uint8 hdr_len;
            uint16 u16;
        };
        uint32    word0;
    };
    uint32 word1;
} fap4kePkt_ipv6ExtHeader_t;
#endif /* defined(CC_FAP4KE_PKT_IPV6_SUPPORT) */

typedef union {
    fap4kePkt_ipv4Header_t v4;
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    fap4kePkt_ipv6Header_t v6;
#endif
} fap4kePkt_ipHeader_t;

typedef struct {
     union { 
        uint16    u16;    
        struct {
            uint16 csumIe : 1;
            uint16 rtgIe  : 1;
            uint16 keyIe  : 1;
            uint16 seqIe  : 1;
            uint16 srcRtIe: 1;
            uint16 recurIe: 2;
            uint16 ackIe  : 1;
            uint16 flags  : 5;
            uint16 ver    : 3;    
        };      
    };

}  fap4kePkt_greFlags_t;

#define FAP4KE_TCP_RST_SYN_FIN_MASK 0x07

typedef struct {
    uint16 sPort;
    uint16 dPort;
    uint32 seq;        /* word1 */
    uint32 ack_seq;    /* word2 */
    uint8 doff_res;
#define FAP4KE_PKT_DOFF_SHIFT  4
#define FAP4KE_PKT_DOFF_MASK   0XF
    uint8 flags;
#define FAP4KE_PKT_TCP_FLAGS_CWR (1 << 7)
#define FAP4KE_PKT_TCP_FLAGS_ECE (1 << 6)
#define FAP4KE_PKT_TCP_FLAGS_URG (1 << 5)
#define FAP4KE_PKT_TCP_FLAGS_ACK (1 << 4)
#define FAP4KE_PKT_TCP_FLAGS_PSH (1 << 3)
#define FAP4KE_PKT_TCP_FLAGS_RST (1 << 2)
#define FAP4KE_PKT_TCP_FLAGS_SYN (1 << 1)
#define FAP4KE_PKT_TCP_FLAGS_FIN (1 << 0)
    uint16 window;
    uint16 csum;
    uint16 urg_ptr;
} fap4kePkt_tcpHeader_t;

#define FAP4KE_PKT_TCP_DOFF(t)   ((t->doff_res >> 4) & 0xF)

typedef struct {
    uint16 sPort;
    uint16 dPort;
    uint16 length;
    uint16 csum;
} fap4kePkt_udpHeader_t;

typedef union {
    struct {
        uint16 ip;
        uint16 tu;
    };
    uint32 u32;
} fap4kePkt_icsum_t;

typedef union {
    struct {
        uint16 inner;
        uint16 outer;
    };
    uint32 u32;
} fap4kePkt_vlanIdFilter_t;

typedef struct {
    // note: making this field match the ipv6 version will allow for
    // the compiler to make certain optimizations.
    struct {
        uint16 isRouted  : 1;
        uint16 mangleTos : 1;
        uint16 drop      : 1;
        uint16 learn     : 1;
        uint16           : 4;
        uint16 tos       : 8;
    } flags;
    uint8 tosOrig;
    union {
        struct {
            uint32 ipSa4;
            uint32 ipDa4;
        };
    };
    union {
        /* Unicast */
        union {
            struct {
                uint16 sPort;  /* TCP/UDP source port */
                uint16 dPort;  /* TCP/UDP dest port */
            };
            uint32 l4Ports;
        };
        /* Multicast */
        fap4kePkt_vlanIdFilter_t vlanId;
    };
    fap4kePkt_icsum_t icsum;
    uint16 excludeDestPort;
} fap4kePkt_ipv4Tuple_t;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
typedef struct {
    struct {
        uint16 isRouted  : 1;
        uint16 mangleTos : 1;
        uint16 drop      : 1;
        uint16 learn     : 1;
        uint16           : 4;
        uint16 tos       : 8;
    } flags;
    uint8 tosOrig;
    union {
        struct {
            fap4kePkt_ipv6Address_t ipSa6;
            fap4kePkt_ipv6Address_t ipDa6;
        };
    };
    union {
        /* Unicast */
        union {
            struct {
                uint16 sPort;  /* TCP/UDP source port */
                uint16 dPort;  /* TCP/UDP dest port */
            };
            uint32 l4Ports;
        };
        /* Multicast */
        fap4kePkt_vlanIdFilter_t vlanId;
    };
    fap4kePkt_icsum_t icsum;
    uint32 tunnelIpSa4;
} fap4kePkt_ipv6Tuple_t;
#endif

typedef union {
    struct {
        union {
            struct {
                uint8 phy;
                union {
                    uint8 channel;
                    uint8 channelMask;
                };
            };

            uint16 phyChannel;
            uint16 phyChannelMask;
        };
        union {
            /* Source key */
            struct {
                /* L2 Flows and L3 Mcast Flows */
                uint8 isLayer2  : 1,
                      l2Type    : 2, /*PPPoE, PPPoA,IPoE*/
                      tcp_pure_ack : 1,
                      nbrOfTags : 4;

                /* L3 Flows only */
                uint8 protocol;
            };

            /* Destination key */
            struct {
                /* L2 Flows only */
                uint8 drop      : 1,
                      multiChan : 1,
                      hiPrio    : 1,
                      unused2   : 5;

                /* L2/L3 Flows */
                uint8 queue;
            };
            uint16 key1;	/* 2nd 16 byte of key */
        };
    };

    uint32 u32;
} fap4kePkt_key_t;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
typedef struct {
    uint32 prio;        /* Ingress QoS packet priority */
} fap4kePkt_iq_t;
#endif


/****************************************************************
 * Layer 2 Flows / ARL
 ****************************************************************/

#if defined(CONFIG_BCM_FAP_LAYER2)

#define FAP4KE_ARL_MAX_ENTRIES      64
#define FAP4KE_ARL_HASH_TABLE_SIZE  64

#define FAP4KE_ARL_AGING_SECONDS    300
#define FAP4KE_ARL_AGING_JIFFIES    (FAPTMR_HZ_LORES * FAP4KE_ARL_AGING_SECONDS)

typedef struct sll_node {
    struct sll_node *next_p;
} sll_node_t;

typedef struct sll_list {
    struct sll_node *head_p;
} sll_list_t;

#define sll_init(list_p)        ((list_p)->head_p = NULL)

#define sll_head_p(list_p)      ((list_p)->head_p)

#define sll_next_p(node_p)      ((node_p)->next_p)

#define sll_empty(list_p)       ((list_p)->head_p == NULL)
#define sll_end(node_p)         ((node_p)->next_p == NULL)

#define sll_prepend(list_p, node_p)                     \
    do {                                                \
        if(sll_empty(list_p)) {                         \
            (node_p)->next_p = NULL;                    \
        } else {                                        \
            (node_p)->next_p = (list_p)->head_p;        \
        }                                               \
        (list_p)->head_p = (node_p);                    \
    } while(0)

/* deletes a node from the head */
#define sll_delete_head(list_p)  ((list_p)->head_p = (list_p)->head_p->next_p)

typedef struct {
    union {
        struct {
            uint16 macAddrHigh0;
            uint16 macAddrHigh1;
        };
        uint32 macAddrHigh;
    };
    union {
        struct {
            uint16 macAddrLow;
            uint16 vlanId;
        };
        uint32 macAddrLowVlanId;
    };
} fap4keArl_tableEntryKey_t;

typedef union {
    struct {
        union {
            struct {
                uint8 phy;
                uint8 channelMask;
            };
            uint16 phyChannelMask;
        };
        union {
            struct { /* revIvl = 1 -> Reverse-IVL ARL entry */
                uint16 revIvl  : 1;
                uint16 queue   : 3;
                uint16 vlanId  : 12;
            };
            struct { /* revIvl = 0 -> Regular ARL entry */
                uint16 revIvl_placeholder : 1;
                uint16 multiChan          : 1;
                uint16 unused             : 14;
            };
            uint16 flags;
        };
    };
    uint32 u32;
} fap4keArl_tableEntryInfo_t;

typedef struct {
    uint32 timeStamp;
} fap4keArl_tableEntryAging_t;

typedef struct {
    sll_node_t node;
    fap4keArl_tableEntryKey_t key;
    fap4keArl_tableEntryInfo_t info;
    fap4keArl_tableEntryAging_t aging;
    uint16 hits;
    union {
        struct {
            uint16 isStatic  : 1;
            uint16 canUpdate : 1;
            uint16 unused    : 14;
        };
        uint16 u16;
    } flags;
} fap4keArl_tableEntry_t;

typedef struct {
    sll_list_t hashTable[FAP4KE_ARL_HASH_TABLE_SIZE];
    sll_list_t freePool;
    fap4keArl_tableEntry_t entryPool[FAP4KE_ARL_MAX_ENTRIES];
    fap4keTmr_timer_t agingTimer;
} fap4keArl_Ctrl_t;

static inline void __arlKey(uint16 *macAddr_p, uint16 vlanId, fap4keArl_tableEntryKey_t *arlKey_p)
{
    arlKey_p->macAddrHigh0 = macAddr_p[0];
    arlKey_p->macAddrHigh1 = macAddr_p[1];
    arlKey_p->macAddrLow = macAddr_p[2];
    arlKey_p->vlanId = vlanId;
}

#define FAP4KE_PKT_L2_MAX_VLAN_HEADERS  2

typedef struct {
    uint16 etherType;
    uint8  ipProtocol;
    uint8  tos;
} fap4kePkt_l2TupleFiltersMisc_t;

typedef union {
    struct {
        /* filters */
        uint16 v0_pbits     : 1;
        uint16 v0_dei       : 1;
        uint16 v0_vid       : 1;
        uint16 v0_etherType : 1;
        uint16 v1_pbits     : 1;
        uint16 v1_dei       : 1;
        uint16 v1_vid       : 1;
        uint16 v1_etherType : 1;
        uint16 etherType    : 1;
        uint16 ipProtocol   : 1;
        uint16 tos          : 1;
        uint16 reserved     : 5;
    };
    uint16 u16;
} fap4kePkt_l2TupleFiltersCtrl_t;


typedef struct {
    fap4kePkt_l2TupleFiltersCtrl_t ctrl;
    fap4kePkt_l2TupleFiltersMisc_t misc;
    fap4kePkt_vlanHeader_t vlan[FAP4KE_PKT_L2_MAX_VLAN_HEADERS];
} fap4kePkt_l2TupleFilters_t;

typedef struct {
    fap4kePkt_vlanHeader2_t tag;
    fap4kePkt_vlanHeaderTci_t tciMask;
} fap4kePkt_l2TupleVlanAction_t;

typedef union {
    struct {
        /* actions */
        uint8 v0_tag        : 1;
        uint8 v0_tpid       : 1;
        uint8 v0_tci        : 1;
        uint8 v1_tag        : 1;
        uint8 v1_tpid       : 1;
        uint8 v1_tci        : 1;
        uint8 tos           : 1;
        uint8 revIvl        : 1;
    };
    uint8 u8;
} fap4kePkt_l2TupleActionsCtrl_t;

typedef struct {
    fap4kePkt_l2TupleActionsCtrl_t ctrl;
    uint8 tos;
    struct{
        uint16 unused          : 3;
        uint16 ovrdLearningVid : 1;
        uint16 learnVlanId     : 12;
    };
    fap4kePkt_l2TupleVlanAction_t vlan[FAP4KE_PKT_L2_MAX_VLAN_HEADERS];
} fap4kePkt_l2TupleActions_t;

typedef struct {
    fap4kePkt_l2TupleFilters_t filters;
    fap4kePkt_l2TupleActions_t actions;
} fap4kePkt_l2Tuple_t;

#endif /* CONFIG_BCM_FAP_LAYER2 */


/****************************************************************
 * FAP Flows (L2/L3/Multicast)
 ****************************************************************/
typedef struct mclog {
    struct mclog * next;
    void         * hdr;
    uint16         portmask;
    uint16         mcCfglogIdx;
    union{
        struct {
            uint8 toLearn:1;
            uint8 isextSWPortonIntSW:1;
            uint8 reserved:2;
            uint8 virtDestPort:4;
        };
        uint8 flags;
    };
    uint8 hdrLen;
    uint16 refCount;
    uint32 destIp;
} Mclog_t;

typedef struct {
    union {
        uint8 *                     cmdList_4keAddr;
        Mclog_t *                   mclogLst;
    };
    fap4kePkt_ipv4Tuple_t           tuple;
} fap4kePkt_flowInfo_ipv4_t;

typedef struct {
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    union {
        uint8 *                     cmdList_4keAddr;
        Mclog_t *                   mclogLst;
    };
    fap4kePkt_ipv6Tuple_t           tuple;
#endif
} fap4kePkt_flowInfo_ipv6_t;

typedef struct {
#if defined(CONFIG_BCM_FAP_LAYER2)
    fap4kePkt_l2Tuple_t             tuple;
#endif
} fap4kePkt_flowInfo_l2_t;

typedef struct {
    fap4kePkt_key_t     source;
    fap4kePkt_key_t     dest;
    int32               txAdjust;
    uint16              fapMtu;
    struct {
        uint16          type  : 2,
                       isSsm  : 1,
                    reserved  : 4,
                  inheritTos  : 1,
                  tunnelType  : 8;
    };
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    fap4kePkt_iq_t iq;
#endif
    uint8  virtDestPort;
    uint16 virtDestPortMask;
    // extSwTagLen is only used when FAP directly transmits to Ethernet or XTM. 
    // In the case of WLAN, we will always send to the host and hence declaring 
    // wlChainIdx and extSwTagLen as a union
    union {
        struct {
            uint16  reserved1;
            uint16  extSwTagLen;
        };
        struct {
            uint32 chain_idx        : 16;
            uint32 resvd            : 9;
            uint32 wfd_idx          : 2;
            uint32 priority         : 4;
            uint32 is_chain         : 1;   /* Maintain the bit position */
        }nic;
        struct {
            uint32 resvd            :12;
            uint32 wfd_idx          : 2;
            uint32 ssid             : 4;
            uint32 priority         : 3;
            uint32 flow_ring_idx    :10;
            uint32 is_chain         : 1;   /* Maintain the bit position */
        }dhd;
        uint32  word;
    };
    union {
        fap4kePkt_flowInfo_ipv4_t   ipv4;
        fap4kePkt_flowInfo_ipv6_t   ipv6;
        fap4kePkt_flowInfo_l2_t     l2;
    };
} fap4kePkt_flowInfo_t; 
// tbd: rename this


typedef struct {
    uint32 hits;
    uint32 bytes;
} fap4kePkt_flowStats_t;

typedef struct fap4kePkt_flow {
    struct fap4kePkt_flow *next;

    struct {
        union {
            struct {
                uint8 isActive  : 1;
                uint8 isMcast   : 1;
                uint8           : 6;
            };
            uint8 u8;
        } flags;
        uint8 hashIx;
        uint16 flowId;
    };
    fap4kePkt_flowStats_t stats;

    fap4kePkt_flowInfo_t     info;      /* MUST BE LAST ENTRY IN STRUCTURE (variable size) */
} fap4kePkt_flow_t;

#define fap4kePkt_flowSizeIpv4      ((size_t)&(((fap4kePkt_flow_t *)0)->info.ipv4) + sizeof(fap4kePkt_flowInfo_ipv4_t))
#define fap4kePkt_flowSizeIpv6      ((size_t)&(((fap4kePkt_flow_t *)0)->info.ipv6) + sizeof(fap4kePkt_flowInfo_ipv6_t))
#define fap4kePkt_flowSizeL2        ((size_t)&(((fap4kePkt_flow_t *)0)->info.l2) + sizeof(fap4kePkt_flowInfo_l2_t))
#define fap4kePkt_flowInfoSizeIpv4      ((size_t)&(((fap4kePkt_flowInfo_t *)0)->ipv4) + sizeof(fap4kePkt_flowInfo_ipv4_t))
#define fap4kePkt_flowInfoSizeIpv6      ((size_t)&(((fap4kePkt_flowInfo_t *)0)->ipv6) + sizeof(fap4kePkt_flowInfo_ipv6_t))
#define fap4kePkt_flowInfoSizeL2        ((size_t)&(((fap4kePkt_flowInfo_t *)0)->l2) + sizeof(fap4kePkt_flowInfo_l2_t))

typedef uint16 fap4kePkt_flowId_t;

#define FAP4KE_PKT_LEARN_MAX_VLAN_HEADERS  2

typedef struct {
    uint32 stripLen;
    uint32 vlanHdrCount;
    fap4kePkt_vlanHeader_t *vlanHdr_p[FAP4KE_PKT_LEARN_MAX_VLAN_HEADERS];
} fap4kePkt_learnHeaders_t;

typedef struct {
    uint8 macDa[ETH_ALEN];
    uint8 macSa[ETH_ALEN];
    uint16 etherType;
    union {
        struct {
            uint16 insertEth 	: 1;
            uint16 reserved  	: 15;
        };
        uint16 flags;
    };
    fap4kePkt_vlanHeader2_t vlanHdr[FAP4KE_PKT_LEARN_MAX_VLAN_HEADERS];
} fap4kePkt_cmdArg_t;

typedef enum {
    FAP4KE_PKT_CMD_SET_MAC_DA=0,
    FAP4KE_PKT_CMD_INSERT_MAC_DA,
    FAP4KE_PKT_CMD_SET_MAC_SA,
    FAP4KE_PKT_CMD_INSERT_MAC_SA,
    FAP4KE_PKT_CMD_INSERT_ETHERTYPE,
    FAP4KE_PKT_CMD_POP_BRCM_TAG,
    FAP4KE_PKT_CMD_POP_BRCM2_TAG,
    FAP4KE_PKT_CMD_PUSH_BRCM2_TAG,
    FAP4KE_PKT_CMD_POP_VLAN_HDR,
    FAP4KE_PKT_CMD_PUSH_VLAN_HDR,
    FAP4KE_PKT_CMD_COPY_VLAN_HDR,
    FAP4KE_PKT_CMD_SET_VLAN_PROTO,
    FAP4KE_PKT_CMD_SET_VID,
    FAP4KE_PKT_CMD_SET_DEI,
    FAP4KE_PKT_CMD_SET_PBITS,
    FAP4KE_PKT_CMD_POP_PPPOE_HDR,
    FAP4KE_PKT_CMD_POP_PPPOA_HDR,
    FAP4KE_PKT_CMD_DECR_TTL,
    FAP4KE_PKT_CMD_MAX
} fap4kePkt_learnCmd_t;

enum {
    FAP4KE_L2TYPE_IPOE=0,
    FAP4KE_L2TYPE_PPPOE,
    FAP4KE_L2TYPE_IPOA,
    FAP4KE_L2TYPE_PPPOA
};
typedef struct {
    uint8 cmdCount;
    uint8 cmd[FAP4KE_PKT_CMD_MAX]; /* fap4kePkt_learnCmd_t */
    fap4kePkt_cmdArg_t cmdArg;
} fap4kePkt_learnAction_t;

/*
 * Mapped to DDR
 */
typedef struct {
    fap4kePkt_learnAction_t action[FAP4KE_PKT_MAX_FLOWS];
} fap4kePkt_learn_t;

#define FAP_MAX_MCLOGS 100
#define FAP_MAX_MCCFGLOGS FAP_MAX_MCLOGS
#define FAP_MAX_REFCOUNT_TBL_SIZE 1024
#define FAP_MAX_HDRBUF_SIZE 64
#define FAP_MAX_MCHDRS 512
#define FAP_MCCFGLOG_MAX_TXDEVS 6

typedef struct mccfglog {
    struct mccfglog *next;
    uint16 mccfgIdx;
    uint16  portmask;
    union{
        struct {
            uint16 isAlloc:1;
            uint16 isextSWPortonIntSW:1;
            uint16 isextSWPort:1;
            uint16 nTxDevs:3;
            uint16 reserved:6;
            uint16 virtDestPort:4;
        };
        uint16 flags;
    };
    uint8 rsvd_byte;
    uint8 hdrLen;
	uint8 *hdrptr_noncached;
    uint8 hdrBuf[FAP_MAX_HDRBUF_SIZE];/*try to keep this 8 or 16 byte aligned*/
    void *txDev_pp[FAP_MCCFGLOG_MAX_TXDEVS];
    uint32 destIpAddr;
    fap4kePkt_learnAction_t learnAction;
} McCfglog_t;

typedef struct mchdr {
    struct mchdr *next;
    uint32 reserved[3];
    uint8 hdrBuf[FAP_MAX_HDRBUF_SIZE];
}__attribute__((aligned(16))) Mchdr_t;


#define p4keFlowInfoPool  p4keSdram->packet.flowInfoPool
#define p4keHeaderPool    p4kePsmGbl->packet.headerPool
#if defined(CONFIG_BCM963268)
#define GPON_PORT_ID     7
#define RX_GEM_ID_MASK    0x7F
#define gemid_from_dmaflag(dmaFlag) (dmaFlag & RX_GEM_ID_MASK)
#define p4keHeaderPoolIop p4kePsmIopGbl->packet.headerPool
#else
#error "Unknown FAP-based Chip"
#endif

/*
 * Mapped to DSPRAM
 */

#if defined(CONFIG_BCM_FAP_LAYER2)
typedef struct {
    union {
        struct {
            uint8 drop      : 1;
            uint8 multiChan : 1;
            uint8 reserved  : 6;
        };
        uint8 flags;
    };
    uint8 mask;
} fap4kePkt_l2Flood_t;

typedef struct {
    uint16 l2FlowCount;
    union {
        struct {
            uint8 l2Enable : 1;
            uint8 ivlMode  : 1;
            uint8 reserved : 6;
        };
        uint8 flags;
    };
    fap4kePkt_l2Flood_t bcastFloodMask;
    fap4kePkt_l2Flood_t floodMask[FAP4KE_PKT_MAX_SRC_PORTS];
} fap4kePkt_l2Ctrl_t;
#endif

typedef struct {
    uint32 flowCount;
#if defined(CONFIG_BCM_FAP_LAYER2)
    fap4kePkt_l2Ctrl_t l2Ctrl;
#endif
    fap4kePkt_flow_t *hashTable[FAP4KE_PKT_HASH_TABLE_SIZE];
    int dropMcastMiss;
} fap4kePkt_runtime_t;

#define pktRuntime4ke p4keDspramGbl->packet.runtime

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define IQINFO_FLAGS_MCAST      0x01
#define IQINFO_FLAGS_GRE_CTRL   0x02
typedef union {
    uint32 u32;
    struct {
        uint8 flags;        /* Mcast */
        uint8 proto;        /* TCP/UDP */
        uint16 destPort;    /* Dest Port */
    } s;
} fap4kePkt_iqInfo_t;
#endif

typedef struct {
    uint8 *pBuf;
    uint16 length;
    uint16 portId;
    uint32 dmaFlag;
    fap4kePkt_flow_t *flow_p;
    uint8 *packetTx_p;
    uint16 ipLength;
    uint16 rxHdrLen;
    uint16 mssAdj;
    uint16 lookup;
    uint8 bNeedFragmentation;
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    fap4kePkt_iqInfo_t iqInfo;
#endif
} fap4kePkt_packetInfo_t;

static inline uint32 __countSetBits(uint32 bitmap)
{
    uint32 count = bitmap;
    count = ((count & 0x55555555) + ((count >> 1) & 0x55555555));
    count = ((count & 0x33333333) + ((count >> 2) & 0x33333333));
    count = ((count & 0x0f0f0f0f) + ((count >> 4) & 0x0f0f0f0f));
    count %= 255U;
    return count;
}

static inline int __countSetBitsLoop(uint32 bitMask)
{
    int count;

    for(count = 0; bitMask; bitMask >>= 1)
    {
        count += bitMask & 1;
    }

    return count;
}

static inline uint32 tupleHashL2(fap4kePkt_key_t sourceKey, fap4kePkt_key_t destKey)
{
    uint32 hashIx;

    hashIx = sourceKey.nbrOfTags + (sourceKey.channel << 4) + destKey.channelMask;

    return (hashIx % FAP4KE_PKT_HASH_TABLE_SIZE);
}

void fap4kePktTest_runTests(void);

void fap4kePkt_init(void);
fapRet fap4kePkt_activate(fap4kePkt_flowId_t flowId, uint16 mcCfglogIdx);
fapRet fap4kePkt_deactivate(fap4kePkt_flowId_t flowId);
fapRet fap4kePkt_updateFlowInfo(fap4kePkt_flowId_t flowId);
fapRet fap4kePkt_resetStats(fap4kePkt_flowId_t flowId);
fapRet fap4kePkt_mcastAddClient(fap4kePkt_flowId_t flowId, uint16 mcCfglogIdx);
fapRet fap4kePkt_mcastUpdateClient(fap4kePkt_flowId_t flowId, uint16 mcCfglogIdx);
fapRet fap4kePkt_mcastDelClient(fap4kePkt_flowId_t flowId, uint16 mcCfglogIdx);
void fap4kePkt_mcastSetMissBehavior(int dropMcastMiss);
fapRet fap4kePkt_printFlow(fap4kePkt_flowId_t flowId);
void fap4kePkt_learnMcast(fap4kePkt_flow_t *flow_p, int stripLen, uint8 *localL2Hdr_p, uint8 *localIpHdr_p);
void fap4kePkt_free2BD(uint32 hdrKey, int hdrSource, uint32 dataKey, int dataSource, int dataParam1);
int fap4kePkt_sendMcast(fap4kePkt_packetInfo_t *packetInfo_p,
                        fap4kePkt_flow_t *flow_p,
                        fap4kePkt_ipHeader_t *ipHeader_p,
                        char *localPacket_p,
                        int rxChannel,
                        int txSource);
extern void  fap4ke2BD_hdrFree(uint8 *hdr);
extern void  fap4ke_mclogFree(Mclog_t *mclog_p);
#if defined(CONFIG_BCM_FAP_LAYER2)
void fap4kePkt_setFloodingMask(uint8 channel, uint8 mask, int drop);
void fap4keArl_addEntry(uint16 *macAddr_p, uint16 vlanId, fap4keArl_tableEntryInfo_t *arlInfo_p);
void fap4keArl_removeEntry(fap4keArl_tableEntryKey_t *arlKey_p);
void fap4keArl_flush(uint8 channelMask);
void fap4keArl_dump(void);
#endif
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
void fap4kePkt_parse_gre(char *header_p, fap4kePkt_iqInfo_t *iqInfo_p);
#endif
#endif /* __FAP4KE_PACKET_H_INCLUDED__ */
