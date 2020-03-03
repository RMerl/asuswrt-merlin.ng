/*
* <:copyright-BRCM:2018:proprietary:standard
* 
*    Copyright (c) 2018 Broadcom 
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

/*
 * rdpa_net_sim.h
 *
 *  Created on: July 18, 2018
 *      Author: Shachar Rosenberg
 */

#ifndef __RDPA_NET_SIM_INC_H_INCLUDED__
#define __RDPA_NET_SIM_INC_H_INCLUDED__

#ifdef __KERNEL__
  #error "This file is not intended to be compiled into Kernel space.  It is for UT/RDPA simulation only\n"
#endif


#include "netinet/ip.h"
#include "netinet/ip6.h"

/* From:  drbd.h */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD
#define CONFIG_CPU_LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#define __BIG_ENDIAN_BITFIELD
#define CONFIG_CPU_BIG_ENDIAN
#else
# error "sorry, weird endianness on this box"
#endif

#define IP_DF       0x4000      /* Flag: "Don't Fragment"   */

/* From:  linux/types.h */
typedef uint16_t __sum16;
typedef uint16_t __be16;
typedef uint8_t  __u8;
typedef uint32_t __be32;

/* From: ipv6.h */
typedef struct {
    uint8_t u6_addr8[16];
} in6_addr;

struct ipv6hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u8            priority:4,
                version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u8            version:4,
                priority:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
    __u8            flow_lbl[3];

    __be16          payload_len;
    __u8            nexthdr;
    __u8            hop_limit;

    struct  in6_addr    saddr;
    struct  in6_addr    daddr;
};

/*
 *  fragmentation header
 */

struct frag_hdr {
    __u8    nexthdr;
    __u8    reserved;
    __be16  frag_off;
    __be32  identification;
};


/* For some reason (if you know, please let me know) when compiling */
/* in kernel space "struct name" implies "typedef struct name name" */
/* So, in userspace I'm forced to explicitly typedef these.         */
typedef struct iphdr iphdr;
typedef struct tcphdr tcphdr;
typedef struct udphdr udphdr;
typedef struct pppoe_hdr pppoe_hdr;
typedef struct vlan_hdr vlan_hdr;
typedef struct ipv6hdr ipv6hdr;
typedef struct frag_hdr frag_hdr;


#endif /* __RDPA_NET_SIM_INC_H_INCLUDED__ */
