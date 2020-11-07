/*
* <:copyright-BRCM:2014-2015:proprietary:standard
*
*    Copyright (c) 2014-2015 Broadcom
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
 :>
*/

/*
 * pktrnr_rdpa_sim.h
 *
 *  Created on: Oct 1, 2016
 *      Author: Robert Lee
 */

#ifndef __PKTRNR_INC_H_INCLUDED__
#define __PKTRNR_INC_H_INCLUDED__

#ifdef __KERNEL__
  #error "This file is not intended to be compiled into Kernel space.  It is for UT/RDPA simulation only\n"
#endif

/* ############################################################### */
/* PLEASE NOTE:                                                    */
/* This file contains definitions from Linux Kernel header files   */
/* that can not be included because they are, or they include,     */
/* files that are not compatible with compilation for user space.  */
/* When this is done, a note is given to identify its orignial     */
/* source.                                                         */
/*                                                                 */
/* Most, if not all of these defs are well defined typically in    */
/* standards and are unlikely to ever change.                      */
/*                                                                 */
/* Comments or a better way to do this is welcome :-)              */
/* ############################################################### */

#include <stdint.h> 
#include "netinet/tcp.h"
#include "netinet/udp.h"
#include "linux/if_ether.h"
#include "rdpa_net_sim.h"

#define bcm_printk bdmf_trace
#define bcm_print bdmf_trace
#define printk bdmf_trace
#define kmalloc(s,f) malloc(s)
#define kfree free

#define BCM_LOG_ID_CMDLIST 0
#define BCM_LOG_ID_PKTRUNNER 0
#define BCM_LOG_LEVEL_DEBUG 0
#define bcmLog_logIsEnabled(logId, logLevel) 1
#define bcmLog_setLogLevel(arg...)
#define BCM_LOGCODE(code) code
#define BCM_LOG_DEBUG(logId, fmt, arg...) BDMF_TRACE_DBG(fmt, ##arg)
#define BCM_LOG_INFO(logId, fmt, arg...) BDMF_TRACE_INFO(fmt, ##arg)
#define BCM_LOG_NOTICE(logId, fmt, arg...) BDMF_TRACE_INFO(fmt, ##arg)
#define BCM_LOG_ERROR(logId, fmt, arg...) BDMF_TRACE_ERR(fmt, ##arg)
#define blog_dump(_blog_p)
#define blog_rule_dump(_blogRule_p)

#if !defined(BCM_ASSERT_SUPPORTED)
  #define BCM_ASSERT(x) (assert(x))
#endif

#ifndef FALSE
  #define FALSE (0)
  #define TRUE  (!FALSE)
#endif


#define OFFSETOF( structName, memberName )  offsetof( structName, memberName )
#define __force


/* From:  fcachehw.h -> fcache.h (FLOW_HW_INVALID) */
#define FHW_TUPLE_INVALID                   0xFFFFFFFF

/* From:  bcmenet_common.h */
#define SW_PORT_M                           (0x07)
#define LOGICAL_PORT_TO_PHYSICAL_PORT(port) ( (port) & SW_PORT_M )
#define BRCM_TAG2_EGRESS                    0x2000

/* From:  bcm_skb_defines.h */
#define SKBMARK_Q_S                         0
#define SKBMARK_Q_PRIO_S                    (SKBMARK_Q_S)
#define SKBMARK_Q_PRIO_M                    (0x07 << SKBMARK_Q_PRIO_S)
//#define SKBMARK_GET_Q_PRIO(MARK)            ((MARK & SKBMARK_Q_PRIO_M) >> SKBMARK_Q_PRIO_S)
#define SKBMARK_GET_Q_PRIO(MARK)            (MARK)
#define SKBMARK_TC_ID_S                     5
#define SKBMARK_TC_ID_M                     (0x3F << SKBMARK_TC_ID_S)
#define SKBMARK_GET_TC_ID(MARK)             ((MARK & SKBMARK_TC_ID_M) >> SKBMARK_TC_ID_S)

/* From:  if_ether.h */
#define ETH_HLEN                14      /* Total octets in header.	 */

/* From:  bcm_vlan.h -> if_ether.h */
#define BCM_VLAN_DEFAULT_TAG_TPID  0x8100
#define BCM_VLAN_DEFAULT_TAG_PBITS 0
#define BCM_VLAN_DEFAULT_TAG_CFI   0
#define BCM_VLAN_DEFAULT_TAG_VID   1 /* IEEE 802.1Q, Table 9-2 */

/* From: if_vlan.h */
#define VLAN_HLEN	4		/* The additional bytes required by VLAN */
struct vlan_hdr {
        __be16  h_vlan_TCI;
        __be16  h_vlan_encapsulated_proto;
};

/* From: blog.h and bcmxtmcfg.h */
#define BLOG_PTM_US_BONDING_DISABLED                      0
#define BLOG_PTM_US_BONDING_ENABLED                       1

#define XTMRT_PTM_BOND_FRAG_HDR_SIZE         2
#if 1
#define XTMRT_PTM_BOND_MAX_FRAG_PER_PKT      8
#else
/* This is only to test 63268 D0 additional capability, which is not used currently. */
#define XTMRT_PTM_BOND_MAX_FRAG_PER_PKT      24 
#endif

#define XTM_PTM_BOND_HEADER_LEN             (XTMRT_PTM_BOND_MAX_FRAG_PER_PKT*XTMRT_PTM_BOND_FRAG_HDR_SIZE)

/* From:  blog_net.h */
// #define BLOG_ETH_ADDR_LEN       6
// #define BLOG_IP_FLAG_DF         0x4000      /* Do Not Fragment */
// #define BLOG_IPPROTO_IPV6       41      /* IPv6-in-IPv4 tunnelling            */

/* From: linux/if_pppox.h */
#ifndef _UAPI__LINUX_IF_PPPOX_H
#define _UAPI__LINUX_IF_PPPOX_H
struct pppoe_tag {
	__be16 tag_type;
	__be16 tag_len;
	char tag_data[0];
} __attribute__ ((packed));
/* Tag identifiers */
struct pppoe_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8 type : 4;
	__u8 ver : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8 ver : 4;
	__u8 type : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8 code;
	__be16 sid;
	__be16 length;
	struct pppoe_tag tag[0];
} __packed;
#endif // _UAPI__LINUX_IF_PPPOX_H


/* From:  fap4ke_packet */
#define IP4DDN   " <%03u.%03u.%03u.%03u>"
#define IP4PDDN  " <%03u.%03u.%03u.%03u:%05u>"
#define IP4(ip) ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1], ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]

#define IP6HEX  "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x>"
#define IP6PHEX "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%u>"
#define IP6(ip) ((uint16_t*)&ip)[0], ((uint16_t*)&ip)[1],   \
                ((uint16_t*)&ip)[2], ((uint16_t*)&ip)[3],   \
                ((uint16_t*)&ip)[4], ((uint16_t*)&ip)[5],   \
                ((uint16_t*)&ip)[6], ((uint16_t*)&ip)[7]

/* :-( */
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

/* Include files for BLOG & RDPA */
#include "pktrunner_blog_net.h"
#include "pktrunner_blog.h"
#include "rdpa_api.h"
#include "rdpa_mcast.h"

int rdpa_blog_is_wan_port(uint32_t logical_port);
uint16_t blog_getTxMtu(Blog_t * blog_p);

/* From linux/bcm_log.h and linux/bcm_log_mod.h */

typedef enum {
    /* DSL Runner Hooks */
    BCM_FUN_ID_RUNNER_PREPEND,
    BCM_FUN_ID_MAX
} bcmFunId_t;

typedef int (bcmFun_t)(void *);

static bcmFun_t* funTable[BCM_FUN_ID_MAX];

static inline bcmFun_t* bcmFun_get(bcmFunId_t funId) {
  BCM_ASSERT(funId < BCM_FUN_ID_MAX);

  return funTable[funId];
}

#define BCM_RUNNER_PREPEND_SIZE_MAX  32 /* Increasing the size of the prepend data buffer will require
                                           recompiling the pktrunner driver files released as binary */

typedef struct {
    void *blog_p;      /* INPUT: Pointer to the Blog_t structure that triggered the Runner flow creation */
    uint8_t data[BCM_RUNNER_PREPEND_SIZE_MAX]; /* INPUT: The data that will be be prepended to all packets
                                                  forwarded by Runner that match the given Blog/Flow.
                                                  The data must be stored in NETWWORK BYTE ORDER */
    unsigned int size; /* OUTPUT: Size of the prepend data, up to 32 bytes long.
                          When no data is to be prepended, specify size = 0 */
} BCM_runnerPrepend_t;

#endif //  __PKTRNR_INC_H_INCLUDED__
