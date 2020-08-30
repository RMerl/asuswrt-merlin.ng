#if defined(CONFIG_BCM_KF_BLOG)
#ifndef __BLOG_RULE_H_INCLUDED__
#define __BLOG_RULE_H_INCLUDED__

/* 
* <:copyright-BRCM:2010:DUAL/GPL:standard
* 
*    Copyright (c) 2010 Broadcom 
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
 * File Name  : blog_rule.h
 *
 * Description: Blog rules are extensions to a Blog structure that can be used
 *              to specify additional fiters and modifications.
 *
 *******************************************************************************
 */

#define CC_CONFIG_BLOG_RULE_DEBUG

#define BLOG_RULE_VERSION      "v1.0"

#define BLOG_RULE_VLAN_TAG_MAX  3

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
    struct ethhdr mask;
    struct ethhdr value;
} blogRuleFilterEth_t;

typedef struct {
    union {
        struct vlan_hdr mask;
        uint32_t mask32;
    };
    union {
        struct vlan_hdr value;
        uint32_t value32;
    };
} blogRuleFilterVlan_t;

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
    blogRuleFilterEth_t eth;
    uint32_t nbrOfVlanTags;
    blogRuleFilterVlan_t vlan[BLOG_RULE_VLAN_TAG_MAX];
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

/*
 * blogRuleVlanHook_t: The Linux VLAN manager must use this hook to register
 * the handler that creates Blog Rules based on the configured VLAN Rules.
 */
typedef int (* blogRuleVlanHook_t)(Blog_t *blog_p,
                                   struct net_device *rxVlanDev,
                                   struct net_device *txVlanDev);

/*
 * blogRuleVlanNotifyHook_t: The Linux VLAN manager uses this hook to notify
 * the registered handler whenever VLAN Rules are added or removed.
 * The device (dev) can be either a VLAN interface or a Real interface.
 */
typedef void (* blogRuleVlanNotifyHook_t)(struct net_device *dev,
                                          blogRuleVlanNotifyDirection_t direction,
                                          uint32_t nbrOfTags);

extern blogRuleVlanHook_t blogRuleVlanHook;
extern blogRuleVlanNotifyHook_t blogRuleVlanNotifyHook;

typedef int (* blogArlHook_t)(void *e);

extern blogArlHook_t bcm_arl_process_hook_g;

/* -------------- User API -------------- */

blogRule_t *blog_rule_alloc(void);
void blog_rule_free(blogRule_t *blogRule_p);
int blog_rule_free_list(Blog_t *blog_p);
void blog_rule_init(blogRule_t *blogRule_p);
void blog_rule_dump(blogRule_t *blogRule_p);
int blog_rule_add_action(blogRule_t *blogRule_p, blogRuleAction_t *action_p);
int blog_rule_delete_action(void *rule_p);

#endif /* defined(__BLOG_RULE_H_INCLUDED__) */
#endif /* defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG) */
