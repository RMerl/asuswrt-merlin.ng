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
#ifndef __BLOG_RULE_H_INCLUDED__
#define __BLOG_RULE_H_INCLUDED__



#include <string.h>
#include "access_macros.h"
#include "bdmf_interface.h"
#define PBR_BLOG_LOG bdmf_trace

#define BLOG_RULE_VERSION      "v1.0"

#define ETH_ALEN                6       /* Octets in one ethernet addr	 */

#define BLOG_RULE_VLAN_TAG_MAX  2

#define BLOG_RULE_ACTION_MAX    16

#define BLOG_RULE_PBITS_MASK    0xE000
#define BLOG_RULE_PBITS_SHIFT   13
#define BLOG_RULE_DEI_MASK      0x1000
#define BLOG_RULE_DEI_SHIFT     12
#define BLOG_RULE_VID_MASK      0x0FFF
#define BLOG_RULE_VID_SHIFT     0

#define BLOG_RULE_GET_TCI_PBITS(_tci) \
    ( ((_tci) & BLOG_RULE_PBITS_MASK) >> BLOG_RULE_PBITS_SHIFT )

#define BLOG_RULE_GET_TCI_DEI(_tci) \
    ( ((_tci) & BLOG_RULE_DEI_MASK) >> BLOG_RULE_DEI_SHIFT )

#define BLOG_RULE_GET_TCI_VID(_tci) \
    ( (_tci) & BLOG_RULE_VID_MASK )

#define BLOG_RULE_DSCP_IN_TOS_MASK    0xFC
#define BLOG_RULE_DSCP_IN_TOS_SHIFT   2

#define BLOG_RULE_IP_PROTO_MASK    0xFF
#define BLOG_RULE_IP_PROTO_SHIFT   0
#define BLOG_RULE_IP6_NXT_HDR_MASK    0xFF
#define BLOG_RULE_IP6_NXT_HDR_SHIFT   0

#define blog_rule_filterInUse(_filter)                          \
    ({                                                          \
        char *_filter_p = (char *)(&_filter);                   \
        int _i, _val;                                           \
        for(_i=0; _i<sizeof(_filter); ++_i) {                   \
            if((_val = _filter_p[_i]) != 0) break;              \
        }                                                       \
        _val;                                                   \
    })

typedef struct {
    /* only contains the fields we are interested */
    uint8_t tos;
    uint8_t ip_proto;
} blogRuleIpv4Header_t;

typedef struct {
    blogRuleIpv4Header_t mask;
    blogRuleIpv4Header_t value;
} blogRuleFilterIpv4_t;

typedef struct {
    /* only contains the fields we are interested */
    uint8_t tclass;
    uint8_t nxtHdr;
} blogRuleIpv6Header_t;

typedef struct {
    blogRuleIpv6Header_t mask;
    blogRuleIpv6Header_t value;
} blogRuleFilterIpv6_t;

typedef struct {
    uint32_t priority;     /* skb priority filter value is offset by 1 because
                            * 0 is reserved to indicate filter not in use.
                            * Therefore the supported skb priority range is
                            * [0 to 0xfffffffe].
                            */
    uint16_t markFlowId;
    uint16_t markPort;     /* port mark filter value is offset by 1 because
                            * 0 is reserved to indicate filter not in use.
                            * Therefore use 16-bit to cover the supported
                            * port range [0 to 255].
                            */ 
} blogRuleFilterSkb_t;

typedef struct {
    uint32_t nbrOfVlanTags;
    uint32_t hasPppoeHeader;
    blogRuleFilterIpv4_t ipv4;
    blogRuleFilterIpv6_t ipv6;
    blogRuleFilterSkb_t  skb;
    uint32_t flags;
#define BLOG_RULE_FILTER_FLAGS_IS_UNICAST   0x0001
#define BLOG_RULE_FILTER_FLAGS_IS_MULTICAST 0x0002
#define BLOG_RULE_FILTER_FLAGS_IS_BROADCAST 0x0004
} blogRuleFilter_t;

#define BLOG_RULE_FILTER_FLAGS_ALL               \
    ( BLOG_RULE_FILTER_FLAGS_IS_UNICAST   |      \
      BLOG_RULE_FILTER_FLAGS_IS_MULTICAST |      \
      BLOG_RULE_FILTER_FLAGS_IS_BROADCAST )

#undef  BLOG_RULE_DECL
#define BLOG_RULE_DECL(x) x

typedef enum {
    BLOG_RULE_DECL(BLOG_RULE_CMD_NOP=0),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_MAC_DA),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_MAC_SA),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_ETHERTYPE),
    BLOG_RULE_DECL(BLOG_RULE_CMD_PUSH_VLAN_HDR),
    BLOG_RULE_DECL(BLOG_RULE_CMD_POP_VLAN_HDR),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_PBITS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_DEI),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_VID),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_VLAN_PROTO),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_PBITS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_DEI),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_VID),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_VLAN_PROTO),
//    BLOG_RULE_DECL(BLOG_RULE_CMD_XLATE_DSCP_TO_PBITS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_POP_PPPOE_HDR),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_DSCP),
    BLOG_RULE_DECL(BLOG_RULE_CMD_DECR_TTL),
    BLOG_RULE_DECL(BLOG_RULE_CMD_DECR_HOP_LIMIT),
    BLOG_RULE_DECL(BLOG_RULE_CMD_DROP),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_SKB_MARK_PORT),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_SKB_MARK_QUEUE),
    BLOG_RULE_DECL(BLOG_RULE_CMD_OVRD_LEARNING_VID),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_STA_MAC_ADDRESS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_MAX)
} blogRuleCommand_t;

typedef struct {
    uint8_t cmd; // blogRuleCommand_t
    uint8_t toTag;
    union {
        uint16_t etherType;
        uint16_t tpid;
        uint16_t pbits;
        uint16_t dei;
        uint16_t vid;
        uint16_t vlanProto;
        uint16_t dscp;
        uint16_t fromTag;
        uint16_t skbMarkQueue;
        uint16_t skbMarkPort;
        uint16_t arg;
        uint8_t macAddr[ETH_ALEN];
    };
} blogRuleAction_t;

typedef struct blogRule {
    blogRuleFilter_t filter;
    uint32_t actionCount;
    blogRuleAction_t action[BLOG_RULE_ACTION_MAX];
    struct blogRule *next_p;
} blogRule_t;

typedef enum {
    BLOG_RULE_VLAN_NOTIFY_DIR_RX,
    BLOG_RULE_VLAN_NOTIFY_DIR_TX,
    BLOG_RULE_VLAN_NOTIFY_DIR_MAX
} blogRuleVlanNotifyDirection_t;

static inline void blog_rule_init(blogRule_t *blogRule_p)
{
    MEMSET(blogRule_p, 0, sizeof(blogRule_t));
}

static inline int blog_rule_add_action(blogRule_t *blogRule_p, blogRuleAction_t *action_p)
{
    if(blogRule_p->actionCount == BLOG_RULE_ACTION_MAX)
    {
        PBR_BLOG_LOG("ERROR : Maximum number of actions reached for blogRule_p <%p>\n",
               blogRule_p);

        return -1;
    }

    blogRule_p->action[blogRule_p->actionCount] = *action_p;

    blogRule_p->actionCount++;

    return 0;
}

#endif /* defined(__BLOG_RULE_H_INCLUDED__) */
