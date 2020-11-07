/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/
#ifndef __BLOG_H_INCLUDED__
#define __BLOG_H_INCLUDED__


/* Blog.h FOR PYTHON PKTRUNNER EXTENSION   */
/*-----------------------------------------*/

/* 
* <:copyright-BRCM:2003:DUAL/GPL:standard
* 
*    Copyright (c) 2003 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
:>
*/

/*
 *******************************************************************************
 *
 * File Name  : pktrnr_blog.h
 *
 * Description:
 *
 * This is a smaller copy of the file found in comengine/kernel...
 * It contains only the necessary fields to run PktRunner.
 * If it fails compilation it is because PktRunner is now
 * using new fields from Blog.h.  Modify this file and add
 * the new fields.
 *
 *******************************************************************************
 */

#define BLOG_VERSION            "v3.0"

#include "pktrunner_blog_rule.h"

extern const uint8_t    rfc2684HdrLength[];

#define BLOG_DECL(x) x,
#define BLOG_NULL                   ((Blog_t*)NULL)


/*
 *------------------------------------------------------------------------------
 * RFC 2684 header logging.
 * CAUTION: 0'th enum corresponds to either header was stripped or zero length
 *          header. VC_MUX_PPPOA and VC_MUX_IPOA have 0 length RFC2684 header.
 *          PTM does not have an rfc2684 header.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(RFC2684_NONE)         /*                               */
        BLOG_DECL(LLC_SNAP_ETHERNET)    /* AA AA 03 00 80 C2 00 07 00 00 */
        BLOG_DECL(LLC_SNAP_ROUTE_IP)    /* AA AA 03 00 00 00 08 00       */
        BLOG_DECL(LLC_ENCAPS_PPP)       /* FE FE 03 CF                   */
        BLOG_DECL(VC_MUX_ETHERNET)      /* 00 00                         */
        BLOG_DECL(VC_MUX_IPOA)          /*                               */
        BLOG_DECL(VC_MUX_PPPOA)         /*                               */
        BLOG_DECL(PTM)                  /*                               */
        BLOG_DECL(RFC2684_MAX)
} Rfc2684_t;


/*
 *------------------------------------------------------------------------------
 * Denotes the type of physical interface and the presence of a preamble.
 *------------------------------------------------------------------------------
 */
typedef enum {
    BLOG_DECL(BLOG_XTMPHY)
    BLOG_DECL(BLOG_ENETPHY)
    BLOG_DECL(BLOG_GPONPHY)
    BLOG_DECL(BLOG_EPONPHY)
    BLOG_DECL(BLOG_USBPHY)
    BLOG_DECL(BLOG_WLANPHY)
    BLOG_DECL(BLOG_MOCAPHY)
    BLOG_DECL(BLOG_EXTRA1PHY)
    BLOG_DECL(BLOG_LTEPHY)
    BLOG_DECL(BLOG_SIDPHY)
    BLOG_DECL(BLOG_TCP4_LOCALPHY)
    BLOG_DECL(BLOG_SPU_DS)
    BLOG_DECL(BLOG_SPU_US)
    BLOG_DECL(BLOG_MAXPHY)
    BLOG_DECL(BLOG_SPDTST)
} BlogPhy_t;

/* CAUTION: Following macros have binary dependencies. Please do not change these
   macros without consulting with Broadcom or the subsystem owners
   Macro definition START */
#define BLOG_IS_HWACC_DISABLED_WLAN_EXTRAPHY(rxphy,txphy) ((rxphy == BLOG_EXTRA1PHY) || \
                                                           (txphy == BLOG_EXTRA1PHY))
#define BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(txphy) (txphy == BLOG_WLANPHY)
/* Macro definition END */

/*
 *------------------------------------------------------------------------------
 * Logging of a maximum 4 "virtual" network devices that a flow can traverse.
 * Virtual devices are interfaces that do not perform the actual DMA transfer.
 * E.g. an ATM interface would be referred to as a physical interface whereas
 * a ppp interface would be referred to as a Virtual interface.
 *------------------------------------------------------------------------------
 */
#define MAX_VIRT_DEV           7

#define DEV_DIR_MASK           0x3ul
#define DEV_PTR_MASK           (~DEV_DIR_MASK)
#define DEV_DIR(ptr)           ((uintptr_t)(ptr) & DEV_DIR_MASK)

#define IS_RX_DIR(ptr)         ( DEV_DIR(ptr) == DIR_RX )
#define IS_TX_DIR(ptr)         ( DEV_DIR(ptr) == DIR_TX )

/*
 *------------------------------------------------------------------------------
 * Device pointer conversion between with and without embeded direction info
 *------------------------------------------------------------------------------
 */
#define DEVP_APPEND_DIR(ptr,dir) ((void *)((uintptr_t)(ptr) | (uintptr_t)(dir)))
#define DEVP_DETACH_DIR(ptr)     ((void *)((uintptr_t)(ptr) & (uintptr_t) \
                                                              DEV_PTR_MASK))
/*
 *------------------------------------------------------------------------------
 * Denotes the tos mode.
 *------------------------------------------------------------------------------
 */
typedef enum {
    BLOG_DECL(BLOG_TOS_FIXED)
    BLOG_DECL(BLOG_TOS_INHERIT)
    BLOG_DECL(BLOG_TOS_MAX)
} BlogTos_t;


/*
 * -----------------------------------------------------------------------------
 * Support accleration of L2, L3 packets.
 *
 * When acceleration support is enabled system wide, the default to be used may
 * be set in CC_BLOG_SUPPORT_ACCEL_MODE which gets saved in blog_support_accel_mode_g.
 * One may change the default (at runtime) by invoking blog_support_accel_mode().
 * -----------------------------------------------------------------------------
 */


/* Traffic type */
typedef enum {
    BLOG_DECL(BlogTraffic_IPV4_UCAST)
    BLOG_DECL(BlogTraffic_IPV6_UCAST)
    BLOG_DECL(BlogTraffic_IPV4_MCAST)
    BLOG_DECL(BlogTraffic_IPV6_MCAST)
    BLOG_DECL(BlogTraffic_Layer2_Flow)
    BLOG_DECL(BlogTraffic_MAX)
} BlogTraffic_t;


#define BLOG_SET_PHYHDR(a, b)   ( (((a) & 0xf) << 4) | ((b) & 0xf) )
#define BLOG_GET_PHYTYPE(a)     ( (a) & 0xf )
#define BLOG_GET_PHYLEN(a)      ( (a) >> 4 )

#define BLOG_PHYHDR_MASK        0xff
#define BLOG_SET_HW_ACT(a)      ( ((a) & 0xf) << 8 )
#define BLOG_GET_HW_ACT(a)      ( (a) >> 8 )



/*
 * -----------------------------------------------------------------------------
 *                      Section: Definition of a Blog_t
 * -----------------------------------------------------------------------------
 */


#define BLOG_ENCAP_MAX          6       /* Maximum number of L2 encaps        */
#define BLOG_HDRSZ_MAX          32      /* Maximum size of L2 encaps          */

typedef enum {
        BLOG_DECL(GRE_ETH)             /* e.g. BLOG_XTMPHY, BLOG_GPONPHY     */
        BLOG_DECL(BCM_XPHY)             /* e.g. BLOG_XTMPHY, BLOG_GPONPHY     */
        BLOG_DECL(BCM_SWC)              /* BRCM LAN Switch Tag/Header         */
        BLOG_DECL(ETH_802x)             /* Ethernet                           */
        BLOG_DECL(VLAN_8021Q)           /* Vlan 8021Q (incld stacked)         */
        BLOG_DECL(PPPoE_2516)           /* PPPoE RFC 2516                     */
        BLOG_DECL(PPP_1661)             /* PPP RFC 1661                       */
        BLOG_DECL(PLD_IPv4)             /* Payload IPv4                       */
        BLOG_DECL(PLD_IPv6)             /* Payload IPv6                       */
        BLOG_DECL(PPTP)                 /* PPTP Header                        */
        BLOG_DECL(L2TP)                 /* L2TP Header                        */
        BLOG_DECL(GRE)                  /* GRE Header                         */
        BLOG_DECL(ESP)                  /* ESP Header                         */
        BLOG_DECL(DEL_IPv4)             /* Outer IPv4                         */
        BLOG_DECL(DEL_IPv6)             /* Outer IPv6                         */
        BLOG_DECL(DEL_L2)               /* L2 DEL                             */
        BLOG_DECL(PLD_L2)               /* L2 PLD                             */
        BLOG_DECL(HDR0_IPv4)            /* IPv4 Inner Header 0                */
        BLOG_DECL(HDR0_IPv6)            /* IPv6 Inner Header 0                */
        BLOG_DECL(HDR0_L2)              /* L2 Inner Header 0                  */
        BLOG_DECL(GREoESP_type)         /* GRE over ESP type                  */
        BLOG_DECL(GREoESP_type_resvd)   /* GRE over ESP type                  */
        BLOG_DECL(GREoESP)              /* GRE over ESP                       */
        BLOG_DECL(unused1)              /* unused1                            */
        BLOG_DECL(PASS_THRU)            /* pass-through                       */
        BLOG_DECL(unused)               /* unused                             */
        BLOG_DECL(PROTO_MAX)
} BlogEncap_t;

#define BLOG_CHAN_INVALID   0xFF

#define HDR_BMAP_IPV4           1
#define HDR_BMAP_IPV6           2
#define HDR_BMAP_L2             3

/* GREoESP flag indicates whether it is GRE over ESP, or ESP over GRE */
#define BLOG_ESPoGRE            0
#define BLOG_GREoESP            1

#define BLOG_GREoESP_44         0
#define BLOG_GREoESP_64         1
#define BLOG_GREoESP_46         2
#define BLOG_GREoESP_66         3

#define BLOG_ESPoGRE_44         0
#define BLOG_ESPoGRE_64         1
#define BLOG_ESPoGRE_46         2
#define BLOG_ESPoGRE_66         3

typedef struct {
    uint8_t         channel;        /* e.g. port number, txchannel, ... */

    union {
        struct {
            uint8_t         phyHdrLen   : 4;
            uint8_t         phyHdrType  : 4;
        };
        uint8_t             phyHdr;
    };

    union {
        struct {
                uint32_t         GRE_ETH     : 1;    /* Ethernet over GRE */
                uint32_t         BCM_XPHY    : 1;    /* e.g. BCM_XTM */
                uint32_t         BCM_SWC     : 1;
                uint32_t         ETH_802x    : 1;
                uint32_t         VLAN_8021Q  : 1;
                uint32_t         PPPoE_2516  : 1;
                uint32_t         PPP_1661    : 1;
                uint32_t         PLD_IPv4    : 1;

                uint32_t         PLD_IPv6    : 1;
                uint32_t         PPTP        : 1;
                uint32_t         L2TP        : 1;
                uint32_t         GRE         : 1;
                uint32_t         ESP         : 1;
                uint32_t         DEL_IPv4    : 1;
                uint32_t         DEL_IPv6    : 1;
                uint32_t         DEL_L2      : 1;

                uint32_t         PLD_L2      : 1;
                uint32_t         HDR0_IPv4   : 1;
                uint32_t         HDR0_IPv6   : 1;
                uint32_t         HDR0_L2     : 1;
                uint32_t         GREoESP_type: 2;
                uint32_t         GREoESP     : 1;
                uint32_t         unused1     : 1;

                uint32_t         PASS_THRU   : 1;
                uint32_t         MAPE        : 1;
                uint32_t         unused      : 6;


        }               bmap;/* as per order of BlogEncap_t enums declaration */
        uint32_t        hdrs;
    }; 
} BlogInfo_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log IP Tuple.
 * Packed: 1 16byte cacheline.
 *------------------------------------------------------------------------------
 */
typedef struct {
    uint16_t   source;
    uint16_t   dest;
} ports_t;
 

struct blogTuple_t {
    uint32_t        saddr;          /* IP header saddr */
    uint32_t        daddr;          /* IP header daddr */

    union {
        ports_t     port;
        uint32_t    ports;
    };

    uint8_t         ttl;            /* IP header ttl */
    uint8_t         tos;            /* IP header tos */
    uint16_t        check;          /* checksum: rx tuple=l3, tx tuple=l4 */

};
typedef struct blogTuple_t BlogTuple_t;





#define NEXTHDR_IPV4 IPPROTO_IPIP
#define GRE_MAX_HDR_LEN  (sizeof(struct ipv6hdr) + sizeof(BLOG_HDRSZ_MAX) + BLOG_GRE_HDR_LEN)

#define HDRS_IPinIP     ((1<<GREoESP) | (3<<GREoESP_type) | (1<<GRE) | (1<<ESP) | \
                         (1<<PLD_IPv4) | (1<<PLD_IPv6) | (1<<PLD_L2) | \
                         (1<<HDR0_IPv4) | (1<<HDR0_IPv6) | (1<<HDR0_L2) |  \
                         (1<<DEL_IPv4) | (1<<DEL_IPv6) | (1<<DEL_L2))
#define HDRS_IP4in4     ((1<<PLD_IPv4) | (1<<DEL_IPv4))
#define HDRS_IP6in4     ((1<<PLD_IPv6) | (1<<DEL_IPv4))
#define HDRS_IP4in6     ((1<<PLD_IPv4) | (1<<DEL_IPv6))
#define HDRS_IP6in6     ((1<<PLD_IPv6) | (1<<DEL_IPv6))
#define HDRS_GIP4       ((1<<PLD_IPv4) | (1<<GRE))
#define HDRS_GIP6       ((1<<PLD_IPv6) | (1<<GRE))
#define HDRS_GL2        ((1<<PLD_L2) | (1<<GRE))
#define HDRS_EIP4       ((1<<PLD_IPv4) | (1<<ESP))
#define HDRS_IP2in4     ((1<<PLD_L2) | (1<<DEL_IPv4))
#define HDRS_IP2in6     ((1<<PLD_L2) | (1<<DEL_IPv6))

#define RX_IP4in6(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_IP4in6)
#define RX_IP6in4(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_IP6in4)
#define TX_IP4in6(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_IP4in6)
#define TX_IP6in4(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_IP6in4)

#define RX_IPV4(b)      ((b)->rx.info.bmap.PLD_IPv4)
#define TX_IPV4(b)      ((b)->tx.info.bmap.PLD_IPv4)
#define RX_IPV6(b)      ((b)->rx.info.bmap.PLD_IPv6)
#define TX_IPV6(b)      ((b)->tx.info.bmap.PLD_IPv6)
#define RX_IPV4_DEL(b)  ((b)->rx.info.bmap.DEL_IPv4)
#define TX_IPV4_DEL(b)  ((b)->tx.info.bmap.DEL_IPv4)
#define RX_IPV6_DEL(b)  ((b)->rx.info.bmap.DEL_IPv6)
#define TX_IPV6_DEL(b)  ((b)->tx.info.bmap.DEL_IPv6)
#define PT(b)           ((b)->tx.info.bmap.PASS_THRU)

#define RX_GRE(b)       ((b)->rx.info.bmap.GRE)
#define TX_GRE(b)       ((b)->tx.info.bmap.GRE)
#define RX_ESP(b)       ((b)->rx.info.bmap.ESP)
#define TX_ESP(b)       ((b)->tx.info.bmap.ESP)
#define RX_GRE_ETH(b)   ((b)->rx.info.bmap.GRE_ETH)
#define TX_GRE_ETH(b)   ((b)->tx.info.bmap.GRE_ETH)

#define RX_IPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv4))
#define TX_IPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv4))
#define RX_IPV6ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv6))
#define TX_IPV6ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << PLD_IPv6))
#define RX_L2ONLY(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << PLD_L2))
#define TX_L2ONLY(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << PLD_L2))

#define CHK_RX_GIPV4(b)    (((b)->rx.info.hdrs & ((1 << DEL_IPv4) | (1 << PLD_IPv4))) && RX_GRE(b))
#define CHK_RX_GIPV6(b)    (((b)->rx.info.hdrs & ((1 << DEL_IPv6) | (1 << PLD_IPv6))) && RX_GRE(b))


#define RX_IPV4_OUTER(b) (RX_IPV4ONLY(b) || RX_IPV4_DEL(b))
#define TX_IPV4_OUTER(b) (TX_IPV4ONLY(b) || TX_IPV4_DEL(b))
#define PT4(b)          (RX_IPV4ONLY(b) && TX_IPV4ONLY(b) && PT(b))

#define RX_IPV6_OUTER(b) (RX_IPV6ONLY(b) || RX_IPV6_DEL(b))
#define TX_IPV6_OUTER(b) (TX_IPV6ONLY(b) || TX_IPV6_DEL(b))
#define PT6(b)          (RX_IPV6ONLY(b) && TX_IPV6ONLY(b) && PT(b))

#define HDRS_IPV4       ((1 << PLD_IPv4) | (1 << DEL_IPv4))
#define HDRS_IPV6       ((1 << PLD_IPv6) | (1 << DEL_IPv6))

#define MAPT_UP(b)      (RX_IPV4ONLY(b) && TX_IPV6ONLY(b))
#define MAPT_DN(b)      (RX_IPV6ONLY(b) && TX_IPV4ONLY(b))
#define MAPT(b)         (MAPT_DN(b) || MAPT_UP(b))

#define T4in6UP(b)      (RX_IPV4ONLY(b) && TX_IP4in6(b))
#define T4in6DN(b)      (RX_IP4in6(b) && TX_IPV4ONLY(b))

#define T6in4UP(b)      (RX_IPV6ONLY(b) && TX_IP6in4(b))
#define T6in4DN(b)      (RX_IP6in4(b) && TX_IPV6ONLY(b))

#define CHK4in6(b)      (T4in6UP(b) || T4in6DN(b))
#define CHK6in4(b)      (T6in4UP(b) || T6in4DN(b)) 
#define CHK4to4(b)      (RX_IPV4ONLY(b) && TX_IPV4ONLY(b))
#define CHK6to6(b)      (RX_IPV6ONLY(b) && TX_IPV6ONLY(b))

#define HDRS_GIP4in4    ((1<<GRE) | HDRS_IP4in4)
#define HDRS_GIP6in4    ((1<<GRE) | HDRS_IP6in4)
#define HDRS_GIP2in4    ((1<<GRE) | HDRS_IP2in4)

#define HDRS_GIP4in6    ((1<<GRE) | HDRS_IP4in6)
#define HDRS_GIP6in6    ((1<<GRE) | HDRS_IP6in6)
#define HDRS_GIP2in6    ((1<<GRE) | HDRS_IP2in6)

#define RX_GIPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_GIP4)
#define TX_GIPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_GIP4)
#define RX_GIPV6ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_GIP6)
#define TX_GIPV6ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_GIP6)
#define RX_GL2ONLY(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_GL2)
#define TX_GL2ONLY(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_GL2)

#define RX_GIP4in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in4)
#define TX_GIP4in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in4)
#define RX_GIP6in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in4)
#define TX_GIP6in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in4)
#define RX_GIP2in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in4)
#define TX_GIP2in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in4)
#define RX_GIP46in4(b)  (RX_GIP4in4(b) || RX_GIP6in4(b))
#define TX_GIP46in4(b)  (TX_GIP4in4(b) || TX_GIP6in4(b))

#define RX_GIP4in6(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in6)
#define TX_GIP4in6(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP4in6)
#define RX_GIP6in6(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in6)
#define TX_GIP6in6(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP6in6)
#define RX_GIP2in6(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in6)
#define TX_GIP2in6(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_GIP2in6)
#define RX_GIP46in6(b)  (RX_GIP4in6(b) || RX_GIP6in6(b))
#define TX_GIP46in6(b)  (TX_GIP4in6(b) || TX_GIP6in6(b))

#define TG4in4UP(b)     (RX_IPV4ONLY(b) && TX_GIP4in4(b))
#define TG4in4DN(b)     (RX_GIP4in4(b) && TX_IPV4ONLY(b))
#define TG6in4UP(b)     (RX_IPV6ONLY(b) && TX_GIP6in4(b))
#define TG6in4DN(b)     (RX_GIP6in4(b) && TX_IPV6ONLY(b))
#define TG2in4UP(b)     (RX_L2ONLY(b) && TX_GIP2in4(b))
#define TG2in4DN(b)     (RX_GIP2in4(b) && TX_L2ONLY(b))

#define TGL3_4in4UP(b)  (RX_IPV4ONLY(b) && !TX_GRE_ETH(b) && TX_GIP4in4(b))
#define TGL3_4in4DN(b)  (!RX_GRE_ETH(b) && RX_GIP4in4(b) && TX_IPV4ONLY(b))
#define TGL2_4in4UP(b)  (RX_IPV4ONLY(b) && TX_GRE_ETH(b) && TX_GIP4in4(b))
#define TGL2_4in4DN(b)  (RX_GRE_ETH(b) && RX_GIP4in4(b) && TX_IPV4ONLY(b))
#define TGL2_2in4UP(b)  (RX_L2ONLY(b) && TX_GRE_ETH(b) && TX_GIP2in4(b))
#define TGL2_2in4DN(b)  (RX_GRE_ETH(b) && RX_GIP2in4(b) && TX_L2ONLY(b))

#define TG4in6UP(b)     (RX_IPV4ONLY(b) && TX_GIP4in6(b))
#define TG4in6DN(b)     (RX_GIP4in6(b) && TX_IPV4ONLY(b))
#define TG6in6UP(b)     (RX_IPV6ONLY(b) && TX_GIP6in6(b))
#define TG6in6DN(b)     (RX_GIP6in6(b) && TX_IPV6ONLY(b))
#define TG2in6UP(b)     (RX_L2ONLY(b) && TX_GIP2in6(b))
#define TG2in6DN(b)     (RX_GIP2in6(b) && TX_L2ONLY(b))

#define TG24in4UP(b)    (TG4in4UP(b) || TG2in4UP(b))
#define TG24in4DN(b)    (TG4in4DN(b) || TG2in4DN(b))

#define TG24in6UP(b)    (TG4in6UP(b) || TG2in6UP(b))
#define TG24in6DN(b)    (TG4in6DN(b) || TG2in6DN(b))

#define CHKG4in4(b)     (TG4in4UP(b) || TG4in4DN(b))
#define CHKG6in4(b)     (TG6in4UP(b) || TG6in4DN(b))
#define CHKG2in4(b)     (TG2in4UP(b) || TG2in4DN(b))
#define CHKG46in4UP(b)  (TG4in4UP(b) || TG6in4UP(b))
#define CHKG46in4DN(b)  (TG4in4DN(b) || TG6in4DN(b))
#define CHKG46in4(b)    (CHKG4in4(b) || CHKG6in4(b))
#define CHKG246in4UP(b) (TG4in4UP(b) || TG6in4UP(b) || TG2in4UP(b))
#define CHKG246in4DN(b) (TG4in4DN(b) || TG6in4DN(b) || TG2in4DN(b))
#define CHKG246in4(b)   (CHKG4in4(b) || CHKG6in4(b) || CHKG2in4(b))

#define CHKG4in6(b)     (TG4in6UP(b) || TG4in6DN(b))
#define CHKG6in6(b)     (TG6in6UP(b) || TG6in6DN(b))
#define CHKG2in6(b)     (TG2in6UP(b) || TG2in6DN(b))
#define CHKG46in6UP(b)  (TG4in6UP(b) || TG6in6UP(b))
#define CHKG46in6DN(b)  (TG4in6DN(b) || TG6in6DN(b))
#define CHKG46in6(b)    (CHKG4in6(b) || CHKG6in6(b))
#define CHKG246in6UP(b) (TG4in6UP(b) || TG6in6UP(b) || TG2in6UP(b))
#define CHKG246in6DN(b) (TG4in6DN(b) || TG6in6DN(b) || TG2in6DN(b))
#define CHKG246in6(b)   (CHKG4in6(b) || CHKG6in6(b) || CHKG2in6(b))

#define PTG4(b)         (RX_GIPV4ONLY(b) && TX_GIPV4ONLY(b) && PT(b))
#define PTG6(b)         (RX_GIPV6ONLY(b) && TX_GIPV6ONLY(b) && PT(b))
#define TOTG4(b)        ((RX_GIP4in4(b) && TX_GIP4in4(b)) || \
                         (RX_GIP6in4(b) && TX_GIP6in4(b)))
#define TOTG6(b)        ((RX_GIP4in6(b) && TX_GIP4in6(b)) || \
                         (RX_GIP6in6(b) && TX_GIP6in6(b)))

#define L2ACCEL_PTG(b)  (RX_GL2ONLY(b) && TX_GL2ONLY(b))

#define HDRS_EIP4in4    ((1<<ESP) | HDRS_IP4in4)
#define HDRS_EIP6in4    ((1<<ESP) | HDRS_IP6in4)

#define RX_EIPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)== HDRS_EIP4)
#define TX_EIPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)== HDRS_EIP4)

#define RX_EIP4in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_EIP4in4)
#define TX_EIP4in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_EIP4in4)
#define RX_EIP6in4(b)   (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_EIP6in4)
#define TX_EIP6in4(b)   (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_EIP6in4)

#define TE4in4UP(b)     (RX_IPV4ONLY(b) && TX_EIP4in4(b))
#define TE4in4DN(b)     (RX_EIP4in4(b) && TX_IPV4ONLY(b))
#define TE6in4UP(b)     (RX_IPV6ONLY(b) && TX_EIP6in4(b))
#define TE6in4DN(b)     (RX_EIP6in4(b) && TX_IPV6ONLY(b))

#define CHKE4in4(b)     (TE4in4UP(b) || TE4in4DN(b))
#define CHKE6in4(b)     (TE6in4UP(b) || TE6in4DN(b))
#define CHKE46in4(b)    (CHKE4in4(b) || CHKE6in4(b))

#define RX_PPTP(b)       ((b)->rx.info.bmap.PPTP)
#define TX_PPTP(b)       ((b)->tx.info.bmap.PPTP)

#define RX_L2TP(b)       ((b)->rx.info.bmap.L2TP)
#define TX_L2TP(b)       ((b)->tx.info.bmap.L2TP)

#define RX_PPPOE(b)       ((b)->rx.info.bmap.PPPoE_2516)
#define TX_PPPOE(b)       ((b)->tx.info.bmap.PPPoE_2516)
#define PT_PPPOE(b)       (RX_PPPOE(b) && TX_PPPOE(b))

#define PKT_IPV6_GET_TOS_WORD(word)       \
   ((ntohl(word) & 0x0FF00000) >> 20)

#define PKT_IPV6_SET_TOS_WORD(word, tos)  \
   (word = htonl((ntohl(word) & 0xF00FFFFF) | ((tos << 20) & 0x0FF00000)))

   
   
typedef struct ip6_addr {
    union {
        uint8_t     p8[16];
        uint16_t    p16[8];
        uint32_t    p32[4];
    };
} ip6_addr_t;


/*
 *------------------------------------------------------------------------------
 * Buffer to log IPv6 Tuple.
 * Packed: 3 16byte cachelines
 *------------------------------------------------------------------------------
 */
struct blogTupleV6_t {
    union {
        uint32_t    word0;
    };

    union {
        uint32_t    word1;
        struct {
            uint16_t length; 
            uint8_t next_hdr; 
            uint8_t rx_hop_limit;
        };
    };

    ip6_addr_t      saddr;
    ip6_addr_t      daddr;

    union {
        ports_t     port;
        uint32_t    ports;
    };

    union {
        struct {
            uint8_t     exthdrs:6;  /* Bit field of IPv6 extension headers */
            uint8_t     fragflag:1; /* 6in4 Upstream IPv4 fragmentation flag */
            uint8_t     tunnel:1;   /* Indication of IPv6 tunnel */
            uint8_t     tx_hop_limit;
            uint16_t    ipid;       /* 6in4 Upstream IPv4 identification */
        };
        uint32_t   word2;
    };

};
typedef struct blogTupleV6_t BlogTupleV6_t;

typedef union blogGreFlags {
    uint16_t    u16;
    struct {
            uint16_t csumIe : 1;
            uint16_t rtgIe  : 1;
            uint16_t keyIe  : 1;
            uint16_t seqIe  : 1;
            uint16_t srcRtIe: 1;
            uint16_t recurIe: 3;
            uint16_t ackIe  : 1;

            uint16_t flags  : 4;
            uint16_t ver    : 3;
    };
} BlogGreFlags_t;

struct blogGre_t {
    BlogGreFlags_t  gre_flags;
    union {
        uint16_t    u16;
        struct {
                uint16_t reserved   : 10;
                uint16_t fragflag   :  1;
                uint16_t hlen       :  5;
        };
    };
    uint16_t    ipid;
    uint16_t    l2_hlen;
    uint8_t     l2hdr[ BLOG_HDRSZ_MAX ];    /* Data of all L2 headers */
	
    union { //pptp
        struct {
            uint16_t    keyLen;     
            uint16_t    keyId;      
        }; 
        uint32_t    key;
    };
    uint32_t            seqNum; 
    uint32_t            ackNum;
    uint16_t            pppInfo;
    uint16_t            pppProto;	
};
typedef struct blogGre_t BlogGre_t;


typedef enum {
        BLOG_DECL(BLOG_UPDATE_DPI_QUEUE)  /* DPI Queue assignment has changed */
        BLOG_DECL(BLOG_UPDATE_DPI_PRIORITY)  /* DPI Priority assignment has changed */
        BLOG_DECL(BLOG_UPDATE_MAX)
} BlogUpdate_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log Layer 2 and IP Tuple headers.
 * Packed: 4 16byte cachelines
 *------------------------------------------------------------------------------
 */
struct blogHeader_t {
    BlogTuple_t         tuple;                     /* L3+L4 IP Tuple log */
    void              * dev_p;                     /* physical network device */
    BlogInfo_t          info;
    uint8_t             multicast;                 /* multicast flag */
    uint8_t             count;                     /* # of L2 encapsulations */
    uint8_t             length;                    /* L2 header total length */
    BlogEncap_t         encap[ BLOG_ENCAP_MAX ];   /* All L2 header types */
    uint8_t             l2hdr[ BLOG_HDRSZ_MAX ];   /* Data of all L2 headers */
};

typedef struct blogHeader_t BlogHeader_t;           /* L2 and L3+4 tuple */


union blogWfd_t {
    uint32_t    u32;
    struct {
           uint32_t            chain_idx            : 16;/* Tx chain index */
           uint32_t            priority             : 4;/* Tx Priority */
           uint32_t            reserved0            : 1;/* unused */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            reserved1            : 4;/* unused */
           uint32_t            is_chain             : 1;/* is_chain=1 */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
    } nic_ucast;

    struct {
           uint32_t            flowring_idx         :10;/* Tx flowring index */
           uint32_t            priority             : 3;/* Tx Priority */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            reserved1            : 8;/* unused */
           uint32_t            ssid                 : 4;/* SSID for WLAN */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
    } dhd_ucast;

    struct {
           uint32_t            reserved0            :19;/* unused */
           uint32_t            ssid                 : 4;/* SSID */
           uint32_t            reserved1            : 2;/* unused */
           uint32_t            wfd_prio             : 1;/* 0=high, 1=low */
           uint32_t            wfd_idx              : 2;/* WFD idx */
           uint32_t            is_chain             : 1;/* is_chain=0 */
           uint32_t            is_wfd               : 1;/* is_wfd=1 */
           uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
           uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
    } mcast;
};
typedef union blogWfd_t BlogWfd_t;

struct blogRnr_t {
       uint32_t            flowring_idx         :10;/* Tx flowring index */
       uint32_t            flow_prio            : 2;/* flow priority - used for packet buffer reservation */
       uint32_t            reserved1            : 7;/* unused */
       uint32_t            ssid                 : 4;/* SSID */
       uint32_t            priority             : 3;/* Tx Priority */
       uint32_t            llcsnap_flag         : 1;/* llcsnap_flag */
       uint32_t            radio_idx            : 2;/* Radio index */
       uint32_t            is_wfd               : 1;/* rnr (is_wfd=0) */
       uint32_t            is_tx_hw_acc_en      : 1;/* =1 if WLAN Transmit is capable of HW Acceleartion */
       uint32_t            is_rx_hw_acc_en      : 1;/* =1 if WLAN Receive is capable of HW Acceleration */
};

typedef struct blogRnr_t BlogRnr_t;


/* Coarse hash key: L1, L3, L4 hash */
union blogHash_t {
    uint32_t        match;
    struct {
        union {
            struct {
                uint8_t  tcp_pure_ack : 1;
                uint8_t  unused       : 7;
            };
            uint8_t ext_match;
        };
        uint8_t     protocol;           /* IP protocol */
        uint16_t    reserved;
    };
};

typedef union blogHash_t BlogHash_t;




/*
 *------------------------------------------------------------------------------
 * TCP ACK flow prioritization.
 * Any of the parameters given below can be changed based on the requirements. 
 * The len parameter is IPv4/6 total/payload length and does not include any 
 * L2 fields (like MAC DA, SA, EthType, VLAN, etc.)
 *------------------------------------------------------------------------------
 */
#define BLOG_TCPACK_IPV4_LEN   64   /* IPv4 total len value for pure TCP ACK  */
#define BLOG_TCPACK_IPV6_LEN   32   /* IPv6 len value for pure TCP ACK        */
#define BLOG_TCPACK_MAX_COUNT   4   /* max # of back-to-back TCP ACKs received
                                       after which the ACK flow is prioritized */
#define BLOG_TCPACK_XTM_PRIO    1   /* TCP ACK packets will be sent to priority 1
                                       queue. If the queue does not exist,
                                       packets will be sent to the default queue,
                                       which is the first queue configured for
                                       the interface. */

#define MAX_NUM_VLAN_TAG        2



struct blog_t {
    BlogHash_t          key;            /* Coarse hash search key */
    union {
        BlogWfd_t       wfd;
        BlogRnr_t       rnr;
    };
    
    uint8_t             spdtst;
    uint8_t             vtag_num;
    uint16_t            eth_type;

    union {
        uint32_t        flags;
        struct {
            uint32_t    rsvd:        6;
            uint32_t    is_routed:   1;
            uint32_t    fc_hybrid:   1; /* hybrid flow accelerate in HW and SW */
            uint32_t    l2_pppoe:    1; /* L2 packet is PPPoE */
            uint32_t    l2_ipv6:     1; /* L2 packet is IPv6  */
            uint32_t    l2_ipv4:     1; /* L2 packet is IPv4  */
            uint32_t    is_mapt_us:  1; /* MAP-T Upstream flow */   
            uint32_t    is_df:       1; /* IPv4 DF flag set */
            uint32_t    lag_port:    2; /* LAG port when trunking is done by internal switch/runner */
            uint32_t    ptm_us_bond: 1; /* PTM US Bonding Mode */
            uint32_t    tos_mode_us: 1; /* ToS mode for US: fixed, inherit */
            uint32_t    tos_mode_ds: 1; /* ToS mode for DS: fixed, inherit */
            uint32_t    has_pppoe:   1;
            uint32_t    ack_cnt:     4; /* back to back TCP ACKs for prio */
            uint32_t    ack_done:    1; /* TCP ACK prio decision made */
            uint32_t    nf_dir_pld:  1;
            uint32_t    nf_dir_del:  1;
            uint32_t    pop_pppoa:   1;
            uint32_t    insert_eth:  1;
            uint32_t    iq_prio:     1;  /* not used */
            uint32_t    mc_sync:     1;
            uint32_t    rtp_seq_chk: 1; /* RTP sequence check enable       */
            uint32_t    incomplete:  1;
        };
    };

    unsigned long       mark;           /* NF mark value on tx */
    uint32_t            priority;       /* Tx  priority */
    blogRule_t          * blogRule_p;   /* List of Blog Rules */

    /* vtag[] stored in network order to improve fcache performance */
    uint32_t            vtag[MAX_NUM_VLAN_TAG];

    BlogTupleV6_t       tupleV6;        /* L3+L4 IP Tuple log */
    BlogTupleV6_t       del_tupleV6;    /* Del GRE L3+L4 IP Tuple log */

    BlogHeader_t        tx;             /* Transmit path headers */
    BlogHeader_t        rx;             /* Receive path headers */

    void                *tx_dev_p;      /* TX physical network device */

    BlogHeader_t        *grerx_tuple_p;
    BlogHeader_t        *gretx_tuple_p;
    struct {
        BlogGre_t grerx;
        BlogGre_t gretx;
    };

    BlogTuple_t         delrx_tuple;    /* Del proto RX L3+L4 IP Tuple log */
    BlogTuple_t         deltx_tuple;    /* Del proto TX L3+L4 IP Tuple log */

    uint8_t             mcast_learn;
    uint8_t             is_ssm;
    uint8_t             dpi_queue;
    uint8_t             hw_pathstat_idx;    /* HWACC Pathstat index */
    uint16_t            max_pkt_len;       /* max packet size */
    uint32_t            fc_context;
};

typedef struct blog_t Blog_t;
// typedef struct blog_t Blog;

#endif /* defined(__BLOG_H_INCLUDED__) */

