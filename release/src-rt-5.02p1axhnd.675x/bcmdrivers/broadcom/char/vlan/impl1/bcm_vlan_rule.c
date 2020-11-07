/*
<:copyright-BRCM:2011:proprietary:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/
//**************************************************************************
// File Name  : bcm_vlan_rule.c
//
// Description:
//
//**************************************************************************

#include "bcm_vlan_local.h"
#include "bcm_vlan_dev.h"
#include "pktHdr.h"
#include "skb_defines.h"
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#include "rdpa_types.h"
#include "rdpa_api.h"
#endif
#include <linux/version.h>
#include <linux/blog_rule.h>
#include <linux/vlanctl_bind.h>

/*
 * Local macros, type definitions
 */

#if(BCM_VLAN_MAX_TAGS != SKB_VLAN_MAX_TAGS)
#error "SKB VLANs mismatch!"
#endif

#if defined(CONFIG_MIPS)
#define BCM_VLAN_ETHERTYPE_IPv4            0x0800
#define BCM_VLAN_ETHERTYPE_IPv6            0x86DD
#define BCM_VLAN_ETHERTYPE_PPPOE_SESSION   0x8864
#define BCM_VLAN_ETHERTYPE_PPPOE_DISCOVERY 0x8863
#define BCM_VLAN_PPP_PROTOCOL_IPv4         0x0021
#define BCM_VLAN_PPP_PROTOCOL_IPv6         0x0057
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#define BCM_VLAN_ETHERTYPE_IPv4            __constant_htons(0x0800)
#define BCM_VLAN_ETHERTYPE_IPv6            __constant_htons(0x86DD)
#define BCM_VLAN_ETHERTYPE_PPPOE_SESSION   __constant_htons(0x8864)
#define BCM_VLAN_ETHERTYPE_PPPOE_DISCOVERY __constant_htons(0x8863)
#define BCM_VLAN_PPP_PROTOCOL_IPv4         __constant_htons(0x0021)
#define BCM_VLAN_PPP_PROTOCOL_IPv6         __constant_htons(0x0057)
#endif

#define BCM_VLAN_PPPOE_VERSION_TYPE 0x11
#define BCM_VLAN_PPPOE_CODE_SESSION 0

#define BCM_VLAN_IS_DONT_CARE(_x) ( ((_x) == (typeof(_x))(BCM_VLAN_DONT_CARE)) )

#define BCM_VLAN_IPPROTO_IS_VALID(_ipProto) ( ((_ipProto) <= 255) )
#define BCM_VLAN_DSCP_IS_VALID(_dscp) ( ((_dscp) <= 63) )
#define BCM_VLAN_VID_IS_VALID(_vid) ( ((_vid) <= 4094) )
#define BCM_VLAN_CFI_IS_VALID(_cfi) ( ((_cfi) <= 1) )
#define BCM_VLAN_PBITS_IS_VALID(_pbits) ( ((_pbits) <= 7) )
#define BCM_VLAN_FLOWID_IS_VALID(_flowId) ( ((_flowId) <= 255) )
#define BCM_VLAN_PORT_IS_VALID(_port) ( ((_port) <= 255) )
#define BCM_VLAN_QUEUE_IS_VALID(_queue) ( ((_queue) <= 31) )

#define BCM_VLAN_IPPROTO_FILT_IS_VALID(_ipProto) \
    ( BCM_VLAN_IS_DONT_CARE(_ipProto) || BCM_VLAN_IPPROTO_IS_VALID(_ipProto) )

#define BCM_VLAN_DSCP_FILT_IS_VALID(_dscp) \
    ( BCM_VLAN_IS_DONT_CARE(_dscp) || BCM_VLAN_DSCP_IS_VALID(_dscp) )

#define BCM_VLAN_VID_FILT_IS_VALID(_vid) \
    ( BCM_VLAN_IS_DONT_CARE(_vid) || BCM_VLAN_VID_IS_VALID(_vid) )

#define BCM_VLAN_CFI_FILT_IS_VALID(_cfi) \
    ( BCM_VLAN_IS_DONT_CARE(_cfi) || BCM_VLAN_CFI_IS_VALID(_cfi) )

#define BCM_VLAN_PBITS_FILT_IS_VALID(_pbits) \
    ( BCM_VLAN_IS_DONT_CARE(_pbits) || BCM_VLAN_PBITS_IS_VALID(_pbits) )

#define BCM_VLAN_FLOWID_FILT_IS_VALID(_flowId) \
    ( BCM_VLAN_IS_DONT_CARE(_flowId) || BCM_VLAN_FLOWID_IS_VALID(_flowId) )

#define BCM_VLAN_PORT_FILT_IS_VALID(_port)                              \
  ( BCM_VLAN_IS_DONT_CARE(_port) || BCM_VLAN_PORT_IS_VALID(_port) )

#define BCM_VLAN_FILTER_MATCH(_filter_x, _hdr_x)                        \
    ( (BCM_VLAN_IS_DONT_CARE(_filter_x) || (_filter_x) == (typeof(_filter_x))(_hdr_x)) )

#define BCM_VLAN_FILTER_COMPARE(_filter_1, _filter_2)                   \
    ( BCM_VLAN_IS_DONT_CARE(_filter_1) || BCM_VLAN_IS_DONT_CARE(_filter_2) || \
      ( (_filter_1) == (_filter_2) ) )

#define BCM_VLAN_TAG_IS_DONT_CARE(_vlanTag)             \
    ( (BCM_VLAN_IS_DONT_CARE((_vlanTag).pbits) &&       \
       BCM_VLAN_IS_DONT_CARE((_vlanTag).cfi) &&         \
       BCM_VLAN_IS_DONT_CARE((_vlanTag).vid) &&         \
       BCM_VLAN_IS_DONT_CARE((_vlanTag).etherType)) )

#define BCM_VLAN_TAG_MATCH(_vlanTag, _vlanHeader)                       \
    ( ((ntohs((_vlanHeader)->tci) & (_vlanTag)->tciMask) == (_vlanTag)->tci) && \
      BCM_VLAN_FILTER_MATCH((_vlanTag)->etherType, ntohs((_vlanHeader)->etherType)) )

#define BCM_VLAN_PRINT_VAL(_val, _format)                               \
    ( BCM_VLAN_IS_DONT_CARE(_val) ? "-" : printValName((_val), (_format), sizeof(_val)) )

typedef struct {
    struct sk_buff *skb;
    unsigned int *tpidTable;
    unsigned int nbrOfTags;
    bcmVlan_ethHeader_t *ethHeader;
    bcmVlan_vlanHeader_t *vlanHeader[BCM_VLAN_MAX_TAGS];
    bcmVlan_ipHeader_t *ipHeader;
    bcmVlan_ruleTableDirection_t tableDir;
    UINT8 *dscpToPbits;
    bcmVlan_localStats_t *localStats;
    UINT16 protocol;
} bcmVlan_cmdInfo_t;

typedef int (* cmdHandlerFunc_t)(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg);

typedef struct {
    char *name;
    cmdHandlerFunc_t func;
} bcmVlan_cmdHandler_t;


/*
 * Local variables
 */

static struct kmem_cache *tableEntryCache;

static bcmVlan_cmdInfo_t rxCmdInfo;
static bcmVlan_cmdInfo_t txCmdInfo;
#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)) && defined(CONFIG_BLOG)
static int tpidTableChanged = 0;
static unsigned int oldTpidTable[BCM_VLAN_MAX_TPID_VALUES] = {0};
#endif
/*
 * Local Functions
 */
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
static int isRuleVlanAggregate(bcmVlan_tableEntry_t *tableEntry)
{
    int loop;
    bcmVlan_tagRule_t *tagRule = &(tableEntry->tagRule);

    for (loop = 0; loop < tableEntry->cmdCount; loop++)
    {
        if (BCM_VLAN_OPCODE_DEAGGR_TAG == tagRule->cmd[loop].opCode)
            return 1;
    }
    return 0;
}

static int vlanctl_notify_vlan_aggregate(bcmVlan_tableEntry_t *tableEntry, int enable)
{
    vlanctl_vlan_aggregate_t vlanctl_vlan_aggregate;
    struct net_device *aggregate_dev = NULL;
    struct net_device *deaggregate_dev = NULL;
      
    if ((tableEntry->rxVlanDev) && 
      (BCM_VLAN_DEV_INFO(tableEntry->rxVlanDev)->vlanDevCtrl->flags.swOnly))
    {
        return 0;
    }

    if (isRuleVlanAggregate(tableEntry))
    {

        aggregate_dev = dev_get_by_name(&init_net,tableEntry->tagRule.rxVlanDevName);
        deaggregate_dev = dev_get_by_name(&init_net,tableEntry->tagRule.filter.txVlanDevName);
        
        if ((NULL != aggregate_dev) && (NULL != deaggregate_dev))
        {
            vlanctl_vlan_aggregate.aggregate_vlan_dev = aggregate_dev;
            vlanctl_vlan_aggregate.deaggregate_vlan_dev = deaggregate_dev;
            vlanctl_notify(VLANCTL_BIND_NOTIFY_VLAN_AGGREGATE, &vlanctl_vlan_aggregate, VLANCTL_BIND_CLIENT_RUNNER);
        }
        if (NULL != aggregate_dev)
            dev_put(aggregate_dev);
        if (NULL != deaggregate_dev)
            dev_put(deaggregate_dev);
    }
    return 0;
}
#endif
#if defined(CONFIG_BCM_VLAN_ISOLATION)
static int vlanctl_notify_vlan_set(bcmVlan_tableEntry_t *tableEntry, int enable)
{
    vlanctl_vlan_t vlanctl_vlan;
    unsigned int vid;
    char *vids;
    int notify = 0;

    if (!tableEntry->rxVlanDev)
        return 0;

    if (BCM_VLAN_DEV_INFO(tableEntry->rxVlanDev)->vlanDevCtrl->flags.swOnly)
    {
        return 0;
    }

    vid = tableEntry->tagRule.filter.vlanTag[0].vid;
    if (vid == BCM_VLAN_DONT_CARE)
        return 0;

    vids = BCM_VLAN_DEV_INFO(tableEntry->rxVlanDev)->vids;

    if (enable)
    {
            if (!vids[vid])
                notify = 1;

            vids[vid]++;
    }
    else
    {
            vids[vid]--;

            if (!vids[vid])
                notify = 1;
    }

    if (notify)
    {
        vlanctl_vlan.vlan_dev = tableEntry->rxVlanDev;
        vlanctl_vlan.vid = vid;
        vlanctl_vlan.enable = enable;

        vlanctl_notify(VLANCTL_BIND_NOTIFY_VLAN, &vlanctl_vlan, VLANCTL_BIND_CLIENT_RUNNER);
    }

    return 0;
}
#endif
static int vlanctl_notify_vid_set(bcmVlan_tableEntry_t *tableEntry, int enable)
{
#if defined(CONFIG_BCM_VLAN_ISOLATION)
    vlanctl_notify_vlan_set(tableEntry, enable);
#endif
#if (defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION))
    vlanctl_notify_vlan_aggregate(tableEntry, enable);
#endif
    return 0;
}

static inline void _BCM_VLAN_LL_APPEND(bcmVlan_linkedList_t *ll, bcmVlan_tableEntry_t *tableEntry)
{
    if(!tableEntry->tagRule.isIptvOnly)
        vlanctl_notify_vid_set(tableEntry, 1);

    BCM_VLAN_LL_APPEND(ll, tableEntry);
}

static inline void _BCM_VLAN_LL_INSERT(bcmVlan_linkedList_t *ll, bcmVlan_tableEntry_t *tableEntry,
    bcmVlan_ruleInsertPosition_t position, bcmVlan_tableEntry_t *posTableEntry)
{
    if(!tableEntry->tagRule.isIptvOnly)
        vlanctl_notify_vid_set(tableEntry, 1);

    BCM_VLAN_LL_INSERT(ll, tableEntry, position, posTableEntry);
}

static inline void _BCM_VLAN_LL_REMOVE(bcmVlan_linkedList_t *ll, bcmVlan_tableEntry_t *tableEntry)
{
    if(!tableEntry->tagRule.isIptvOnly)
        vlanctl_notify_vid_set(tableEntry, 0);

    BCM_VLAN_LL_REMOVE(ll, tableEntry);
}

static void parseFrameHeader(bcmVlan_cmdInfo_t *cmdInfo)
{
    int i;
    bcmVlan_vlanHeader_t *vlanHeader;

    cmdInfo->nbrOfTags = 0;

    cmdInfo->ethHeader = BCM_VLAN_SKB_ETH_HEADER(cmdInfo->skb);

    vlanHeader = (&cmdInfo->ethHeader->vlanHeader - 1);

    while(BCM_VLAN_TPID_MATCH(cmdInfo->tpidTable, ntohs(vlanHeader->etherType)))
    {
        vlanHeader++;

        if(cmdInfo->nbrOfTags < BCM_VLAN_MAX_TAGS)
        {
            cmdInfo->vlanHeader[cmdInfo->nbrOfTags] = vlanHeader;
        }

        cmdInfo->nbrOfTags++;
    }

    for(i=cmdInfo->nbrOfTags; i<BCM_VLAN_MAX_TAGS; ++i)
    {
        /* initialize remaining VLAN header pointers */
        cmdInfo->vlanHeader[i] = NULL;
    }

    cmdInfo->protocol = vlanHeader->etherType;

    if(vlanHeader->etherType == BCM_VLAN_ETHERTYPE_IPv4 || vlanHeader->etherType == BCM_VLAN_ETHERTYPE_IPv6)
    {
        cmdInfo->ipHeader = (bcmVlan_ipHeader_t *)(vlanHeader + 1);

        BCM_VLAN_DP_DEBUG("No PPPoE");
    }
    else if(vlanHeader->etherType == BCM_VLAN_ETHERTYPE_PPPOE_SESSION)
    {
        bcmVlan_pppoeSessionHeader_t *pppoeSessionHeader = (bcmVlan_pppoeSessionHeader_t *)(vlanHeader + 1);

        if(pppoeSessionHeader->version_type == BCM_VLAN_PPPOE_VERSION_TYPE &&
           pppoeSessionHeader->code == BCM_VLAN_PPPOE_CODE_SESSION &&
           (pppoeSessionHeader->pppHeader == BCM_VLAN_PPP_PROTOCOL_IPv4 ||
            pppoeSessionHeader->pppHeader == BCM_VLAN_PPP_PROTOCOL_IPv6))
        {
            /* this is a valid IPv4 / IPv6 PPPoE session header */
            cmdInfo->ipHeader = &pppoeSessionHeader->ipHeader;

            BCM_VLAN_DP_DEBUG("PPPoE Session Header @ 0x%p", pppoeSessionHeader);
        }
        else
        {
            BCM_VLAN_DP_ERROR("Invalid PPPoE Session Header @ 0x%p", pppoeSessionHeader);

            cmdInfo->ipHeader = NULL;
        }
    }
    else
    {
        cmdInfo->ipHeader = NULL;

        BCM_VLAN_DP_DEBUG("Unknown L3 protocol: 0x%04X cmdInfo->nbrOfTags %d", 
                          cmdInfo->protocol, cmdInfo->nbrOfTags);
    }

    BCM_VLAN_DP_DEBUG("ETH:0x%p, Tags=%d: [0]0x%p, [1]0x%p, [2]0x%p, [3]0x%p, IP:0x%p",
                      (void *)cmdInfo->ethHeader,
                      cmdInfo->nbrOfTags,
                      (void *)cmdInfo->vlanHeader[0],
                      (void *)cmdInfo->vlanHeader[1],
                      (void *)cmdInfo->vlanHeader[2],
                      (void *)cmdInfo->vlanHeader[3],
                      (void *)cmdInfo->ipHeader);
}

static inline void adjustSkb(bcmVlan_cmdInfo_t *cmdInfo, struct sk_buff *oldSkb, struct sk_buff *newSkb)
{
    if(newSkb && (newSkb != oldSkb))
    {
        /* save the new skb */
        cmdInfo->skb = newSkb;
        parseFrameHeader(cmdInfo);

        BCM_VLAN_DP_DEBUG("New skb 0x%p : data 0x%p, mac_header 0x%08X, network_header 0x%08X\n",
                          (void *)newSkb,
                          (void *)newSkb->data,
                          (unsigned int)newSkb->mac_header,
                          (unsigned int)newSkb->network_header);
    }
}

static inline unsigned char *removeSkbHeader(struct sk_buff *skb, unsigned int len)
{
    BCM_ASSERT(len <= skb->len);
    skb->len -= len;
    BCM_ASSERT(skb->len >= skb->data_len);
    skb_postpull_rcsum(skb, skb->data, len);
    return skb->data += len;
}

static inline struct sk_buff *unshareSkb(bcmVlan_cmdInfo_t *cmdInfo, struct sk_buff *skb)
{
    if(skb_shared(skb) || skb_cloned(skb))
    {
        struct sk_buff *nskb = skb_copy(skb, GFP_ATOMIC);

        kfree_skb(skb);

        adjustSkb(cmdInfo, skb, nskb);

        skb = nskb;
    }

    return skb;
}

static inline struct sk_buff *checkSkbHeadroom(bcmVlan_cmdInfo_t *cmdInfo, struct sk_buff *skb, int headroom)
{
    struct sk_buff *sk_tmp = skb;

    if (skb_headroom(skb) < headroom)
    {
        /* skb_realloc_headroom() will only be called when no headroom is
           available. Hence, the skb will always be cloned, and then
           modified to point to a new data buffer (copy of the original) with
           enough headroom (done by pskb_expand_head()).
           If we were to use only skb_realloc_headroom(), a copy of the skb
           would be generated by calling pskb_copy() when headroom is available,
           regardless if the skb was cloned or not. By calling skb_unshare() below,
           we get a better performance since the skb will only be copied if
           not cloned */

        skb = skb_realloc_headroom(sk_tmp, headroom);

        kfree_skb(sk_tmp);
    }
    else
    {
        /* if skb is cloned, get a new private copy via skb_copy() */
        skb = skb_unshare(skb, GFP_ATOMIC);
    }

    adjustSkb(cmdInfo, sk_tmp, skb);

    return skb;
}

static struct sk_buff *removeOuterTagOnTransmit(bcmVlan_cmdInfo_t *cmdInfo, struct sk_buff *skb)
{
    bcmVlan_ethHeader_t *ethHeader;

    /* on transmit direction, skb->data points to the ethernet header */

    skb = unshareSkb(cmdInfo, skb);
    if(skb == NULL)
    {
        BCM_VLAN_DP_ERROR("Failed to unshare skb");

        goto out;
    }

    if((skb->len-BCM_VLAN_HEADER_LEN) < skb->data_len)
    {
       /* normally even with frags we expect the VLAN to be present in
        * skb->data, but with fragmnetation in tunneling the VLAN header
        * can be in frags. To handle such cases linearize the skb
        */ 
        if(skb_linearize(skb))
        {
            BCM_VLAN_DP_ERROR("Failed to linearize skb");
            kfree_skb(skb);
            skb=NULL;
            goto out;
        }
    }

    ethHeader = (bcmVlan_ethHeader_t *)removeSkbHeader(skb, BCM_VLAN_HEADER_LEN);

    memmove(skb->data, skb->data - BCM_VLAN_HEADER_LEN, 2 * BCM_ETH_ADDR_LEN);

    skb->protocol = ethHeader->etherType;

    skb->mac_header += BCM_VLAN_HEADER_LEN;

out:
    return skb;
}

static struct sk_buff *removeOuterTagOnReceive(bcmVlan_cmdInfo_t *cmdInfo, struct sk_buff *skb)
{
    /* On receive direction, skb->data points to the outer tag */

    skb->protocol = ((bcmVlan_vlanHeader_t *)(skb->data))->etherType;
    // similar to eth_type_trans() or saveSkbVlanOnReceive() set proper protocol
    if (ntohs(skb->protocol) < 1536)
    {
        skb->protocol = htons(ETH_P_802_2);
    }


    skb = unshareSkb(cmdInfo, skb);
    if(skb == NULL)
    {
        BCM_VLAN_DP_ERROR("Failed to unshare skb");

        goto out;
    }

    if((skb->len-BCM_VLAN_HEADER_LEN) < skb->data_len)
    {
       /* normally even with frags we expect the VLAN to be present in
        * skb->data, but with fragmnetation in tunneling the VLAN header
        * can be in frags. To handle such cases linearize the skb
        */ 
        if(skb_linearize(skb))
        {
            BCM_VLAN_DP_ERROR("Failed to linearize skb");
            kfree_skb(skb);
            skb=NULL;
            goto out;
        }
    }

    removeSkbHeader(skb, BCM_VLAN_HEADER_LEN);

    memmove(skb->data - BCM_ETH_HEADER_LEN, skb->data - BCM_ETH_VLAN_HEADER_LEN, 2 * BCM_ETH_ADDR_LEN);

    skb->mac_header += BCM_VLAN_HEADER_LEN;
    skb->network_header += BCM_VLAN_HEADER_LEN;

out:
    return skb;
}

static struct sk_buff *insertOuterTagOnTransmit(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg, struct sk_buff *skb)
{
    bcmVlan_ethHeader_t *ethHeader;

    /* on transmit direction, skb->data points to the ethernet header */

    skb = checkSkbHeadroom(cmdInfo, skb, BCM_VLAN_HEADER_LEN);
    if(skb == NULL)
    {
        BCM_VLAN_DP_ERROR("Failed to allocate skb headroom");

        goto out;
    }

    ethHeader = (bcmVlan_ethHeader_t *)skb_push(skb, BCM_VLAN_HEADER_LEN);

    /* Move the mac addresses to the beginning of the new header. */
    memmove(skb->data, skb->data + BCM_VLAN_HEADER_LEN, 2 * BCM_ETH_ADDR_LEN);

    /* use the default TPID of the rule table */
    ethHeader->etherType = htons(BCM_VLAN_CMD_GET_VAL(arg));

    /* use the default TCI of the rule table */
    ethHeader->vlanHeader.tci = htons(BCM_VLAN_CMD_GET_VAL2(arg));

//    skb->protocol = __constant_htons(ETH_P_8021Q);
    skb->protocol = htons(BCM_VLAN_CMD_GET_VAL(arg));

    skb->mac_header -= BCM_VLAN_HEADER_LEN;
    skb->network_header -= BCM_VLAN_HEADER_LEN;

out:
    return skb;
}

static struct sk_buff *insertOuterTagOnReceive(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg, struct sk_buff *skb)
{
    /* On receive direction, skb->data points to the outer tag, or L3 protocol header */
    /* move skb->data to the MAC header position, so the Linux APIs will work correctly */
    skb->data -= BCM_ETH_HEADER_LEN;

    skb = insertOuterTagOnTransmit(cmdInfo, arg, skb);
    if(skb)
    {
        /* Make skb->data point to the tag we just added (the outermost tag), which is how the
           frame would have looked if it was received from the driver. The original VLAN code
           removes all tags before passing any tagged frames through, but we will not do this here.
           It will be up to the user to remove all tags in case the frame will be passed up to
           higher protocol layers, since we might want to keep the tags if the frame is bridged */
        skb->data += BCM_ETH_HEADER_LEN;
    }

    return skb;
}

static int cmdHandler_popVlanTag(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    int ret = 0;
    int i;
    int lastVlanIndex;

    /* make sure we have at least one tag to remove */
    if(cmdInfo->nbrOfTags == 0)
    {
        BCM_VLAN_DP_ERROR("Cannot remove tag from untagged Frame");
        cmdInfo->localStats->error_PopUntagged++;
        ret = 0;
        goto out;
    }

    if(cmdInfo->tableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        cmdInfo->skb = removeOuterTagOnReceive(cmdInfo, cmdInfo->skb);
        if(cmdInfo->skb == NULL)
        {
            ret = -EFAULT;
        }
    }
    else
    {
        cmdInfo->skb = removeOuterTagOnTransmit(cmdInfo, cmdInfo->skb);
        if(cmdInfo->skb == NULL)
        {
            ret = -EFAULT;
        }
    }

    if(!ret)
    {
#if defined(BCM_VLAN_IP_CHECK)
        if(bcmLog_getLogLevel(BCM_LOG_ID_VLAN) >= BCM_LOG_LEVEL_DEBUG)
        {
            if(cmdInfo->ipHeader && (BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader) == 4))
            {
                unsigned int len = ntohs(cmdInfo->ipHeader->totalLength);
                unsigned int ihl = cmdInfo->ipHeader->ihl;

                if (cmdInfo->skb->len < len || len < 4 * ihl)
                {
                    BCM_VLAN_DP_ERROR("Invalid IP total length: ip %d, skb->len %d, ihl %d",
                                      len, cmdInfo->skb->len, ihl);
                }
            }
        }
#endif
        /* adjust command info */
        cmdInfo->nbrOfTags--;

        lastVlanIndex = BCM_VLAN_MAX_TAGS - 1;

        for(i=0; i<lastVlanIndex; ++i)
        {
            cmdInfo->vlanHeader[i] = cmdInfo->vlanHeader[i+1];
        }

        cmdInfo->vlanHeader[lastVlanIndex] = NULL;

        cmdInfo->ethHeader = BCM_VLAN_SKB_ETH_HEADER(cmdInfo->skb);

        BCM_VLAN_DP_DEBUG("ETH:0x%p, Tags=%d: 0x%p, 0x%p, 0x%p, 0x%p, IP:0x%p",
                          (void *)cmdInfo->ethHeader,
                          cmdInfo->nbrOfTags,
                          (void *)cmdInfo->vlanHeader[0],
                          (void *)cmdInfo->vlanHeader[1],
                          (void *)cmdInfo->vlanHeader[2],
                          (void *)cmdInfo->vlanHeader[3],
                          (void *)cmdInfo->ipHeader);
        BCM_VLAN_DP_DEBUG("skb->mac_header 0x%08X, skb->network_header 0x%08X",
                          (unsigned int)cmdInfo->skb->mac_header, (unsigned int)cmdInfo->skb->network_header);
    }
    else
    {
        cmdInfo->localStats->error_PopNoMem++;
    }

out:
    return ret;
}

static int cmdHandler_pushVlanTag(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    int ret = 0;
    int i;

    /* make sure we can still add one tag */
    if(cmdInfo->nbrOfTags >= BCM_VLAN_MAX_TAGS)
    {
        BCM_VLAN_DP_ERROR("Frame already has %d VLAN tags", cmdInfo->nbrOfTags);
        cmdInfo->localStats->error_PushTooManyTags++;
        ret = 0;
        goto out;
    }

    if(cmdInfo->tableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        cmdInfo->skb = insertOuterTagOnReceive(cmdInfo, arg, cmdInfo->skb);
        if(cmdInfo->skb == NULL)
        {
            ret = -EFAULT;
        }
    }
    else
    {
        cmdInfo->skb = insertOuterTagOnTransmit(cmdInfo, arg, cmdInfo->skb);
        if(cmdInfo->skb == NULL)
        {
            ret = -EFAULT;
        }
#if defined(BCM_VLAN_PUSH_PADDING)
        else if(cmdInfo->skb->len < BCM_VLAN_FRAME_SIZE_MIN)
        {
            /* tagged frames generated locally in the modem
               are padded to 64 bytes because some PC NICs
               will strip the tag and pass undersize packets
               up to their drivers, where they will be discarded */
            cmdInfo->skb->len = BCM_VLAN_FRAME_SIZE_MIN;
        }
#endif
    }

    if(!ret)
    {
        /* adjust command info */
        cmdInfo->nbrOfTags++;

        for(i=BCM_VLAN_MAX_TAGS-1; i>0; --i)
        {
            cmdInfo->vlanHeader[i] = cmdInfo->vlanHeader[i-1];
        }

        cmdInfo->ethHeader = BCM_VLAN_SKB_ETH_HEADER(cmdInfo->skb);

        cmdInfo->vlanHeader[0] = &cmdInfo->ethHeader->vlanHeader;

        BCM_VLAN_DP_DEBUG("ETH:0x%p, Tags=%d: [0]0x%p, [1]0x%p, [2]0x%p, [3]0x%p, IP:0x%p",
                          (void *)cmdInfo->ethHeader,
                          cmdInfo->nbrOfTags,
                          (void *)cmdInfo->vlanHeader[0],
                          (void *)cmdInfo->vlanHeader[1],
                          (void *)cmdInfo->vlanHeader[2],
                          (void *)cmdInfo->vlanHeader[3],
                          (void *)cmdInfo->ipHeader);

        BCM_VLAN_DP_DEBUG("skb->mac_header 0x%08X, skb->network_header 0x%08X",
                          (unsigned int)cmdInfo->skb->mac_header, (unsigned int)cmdInfo->skb->network_header);
    }
    else
    {
        cmdInfo->localStats->error_PushNoMem++;
    }

out:
    return ret;
}

#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
extern int br_fdb_get_vid(const unsigned char *addr);
static int cmdHandler_dropFrame(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg);

static int cmdHandler_deaggrVlanTag(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    int ret = 0;
    int vid;

    /* make sure we have at least one tag to remove */
    if(cmdInfo->nbrOfTags == 0)
    {
        BCM_VLAN_DP_ERROR("Cannot de-aggregate tag from untagged Frame");
        ret = 0;
        goto out;
    }

    if(cmdInfo->tableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        BCM_VLAN_DP_ERROR("Cannot de-aggregate tag in RX direction");
        ret = 0;
    }
    else
    {
        cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
        if(cmdInfo->skb == NULL)
        {
            ret = -EFAULT;
        }
        vid = br_fdb_get_vid(cmdInfo->ethHeader->macDest);
        if (vid == -1)
        {
            BCM_VLAN_DP_ERROR("Cannot find bridge FDB, drop the packet.");
            ret = cmdHandler_dropFrame(cmdInfo, NULL);
        }
        else if (vid < 4096)
        {
            BCM_VLAN_SET_TCI_VID_IN_NWK_ORDER(cmdInfo->vlanHeader[0]->tci, (unsigned short)(vid));
            ret = 0;
        }
    }

out:
    return ret;
}
#endif

static int cmdHandler_setEthertype(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    if(cmdInfo->nbrOfTags == 0 || !BCM_VLAN_TPID_MATCH(cmdInfo->tpidTable, BCM_VLAN_CMD_GET_VAL(arg)))
    {
        BCM_VLAN_DP_ERROR("Cannot set Ethertype: nbrOfTags %d, etherType 0x%04X",
                          cmdInfo->nbrOfTags, BCM_VLAN_CMD_GET_VAL(arg));
        cmdInfo->localStats->error_SetEtherType++;
        return 0;
    }

    cmdInfo->ethHeader->etherType = __constant_htons(BCM_VLAN_CMD_GET_VAL(arg));
    cmdInfo->skb->protocol = cmdInfo->ethHeader->etherType;

    return 0;
}

static int cmdHandler_setPbits(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *vlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    vlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(vlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Vlan Tag %d does not exist", BCM_VLAN_CMD_GET_TARGET_TAG(arg));
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    BCM_VLAN_SET_TCI_PBITS_IN_NWK_ORDER(vlanHeader->tci, BCM_VLAN_CMD_GET_VAL(arg));

    return 0;
}

static int cmdHandler_setCfi(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *vlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    vlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(vlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Vlan Tag %d does not exist", BCM_VLAN_CMD_GET_TARGET_TAG(arg));
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    BCM_VLAN_SET_TCI_CFI_IN_NWK_ORDER(vlanHeader->tci, BCM_VLAN_CMD_GET_VAL(arg));

    return 0;
}

static int cmdHandler_setVid(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *vlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    vlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(vlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Vlan Tag %d does not exist", BCM_VLAN_CMD_GET_TARGET_TAG(arg));
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    BCM_VLAN_SET_TCI_VID_IN_NWK_ORDER(vlanHeader->tci, BCM_VLAN_CMD_GET_VAL(arg));

    return 0;
}

static int cmdHandler_setTagEthertype(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *vlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    vlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(vlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Vlan Tag %d does not exist", BCM_VLAN_CMD_GET_TARGET_TAG(arg));
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    if((BCM_VLAN_CMD_GET_TARGET_TAG(arg) == cmdInfo->nbrOfTags-1) ||
       !BCM_VLAN_TPID_MATCH(cmdInfo->tpidTable, BCM_VLAN_CMD_GET_VAL(arg)))
    {
        BCM_VLAN_DP_ERROR("Cannot set Tag Ethertype: nbrOfTags %d, Target %d, etherType 0x%04X",
                          cmdInfo->nbrOfTags, BCM_VLAN_CMD_GET_TARGET_TAG(arg), BCM_VLAN_CMD_GET_VAL(arg));
        cmdInfo->localStats->error_SetTagEtherType++;
        return 0;
    }

    vlanHeader->etherType = __constant_htons(BCM_VLAN_CMD_GET_VAL(arg));

    return 0;
}

static int cmdHandler_copyPbits(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *sourceVlanHeader;
    bcmVlan_vlanHeader_t *targetVlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    sourceVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_SOURCE_TAG(arg)];
    targetVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(sourceVlanHeader == NULL || targetVlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Invalid Source/Target Vlan Tags (s:0x%p, t:0x%p)",
                          (void *)sourceVlanHeader, (void *)targetVlanHeader);
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    BCM_VLAN_COPY_TCI_PBITS(targetVlanHeader, sourceVlanHeader);

    return 0;
}

static int cmdHandler_copyCfi(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *sourceVlanHeader;
    bcmVlan_vlanHeader_t *targetVlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    sourceVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_SOURCE_TAG(arg)];
    targetVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(sourceVlanHeader == NULL || targetVlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Invalid Source/Target Vlan Tags (s:0x%p, t:0x%p)",
                          (void *)sourceVlanHeader, (void *)targetVlanHeader);
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    BCM_VLAN_COPY_TCI_CFI(targetVlanHeader, sourceVlanHeader);

    return 0;
}

static int cmdHandler_copyVid(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *sourceVlanHeader;
    bcmVlan_vlanHeader_t *targetVlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    sourceVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_SOURCE_TAG(arg)];
    targetVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(sourceVlanHeader == NULL || targetVlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Invalid Source/Target Vlan Tags (s:0x%p, t:0x%p)",
                          (void *)sourceVlanHeader, (void *)targetVlanHeader);
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    BCM_VLAN_COPY_TCI_VID(targetVlanHeader, sourceVlanHeader);

    return 0;
}

static int cmdHandler_copyTagEthertype(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *sourceVlanHeader;
    bcmVlan_vlanHeader_t *targetVlanHeader;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    sourceVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_SOURCE_TAG(arg)];
    targetVlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(sourceVlanHeader == NULL || targetVlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Invalid Source/Target Vlan Tags (s:0x%p, t:0x%p)",
                          (void *)sourceVlanHeader, (void *)targetVlanHeader);
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    if(BCM_VLAN_CMD_GET_TARGET_TAG(arg) == cmdInfo->nbrOfTags-1)
    {
        BCM_VLAN_DP_ERROR("Cannot Copy Tag Ethertype: nbrOfTags %d, Target %d",
                          cmdInfo->nbrOfTags, BCM_VLAN_CMD_GET_TARGET_TAG(arg));
        cmdInfo->localStats->error_SetTagEtherType++;
        return 0;
    }

    targetVlanHeader->etherType = sourceVlanHeader->etherType;

    return 0;
}

static int cmdHandler_dscp2pbits(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *vlanHeader;
    __u8 dscp;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    vlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];

    if(cmdInfo->ipHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Unknown IP Header");
        cmdInfo->localStats->error_UnknownL3Header++;
        return 0;
    }

    if(vlanHeader == NULL)
    {
        BCM_VLAN_DP_ERROR("Vlan Tag %d does not exist", BCM_VLAN_CMD_GET_TARGET_TAG(arg));
        cmdInfo->localStats->error_InvalidTagNbr++;
        return 0;
    }

    if ((BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader) == 4) ||
        (BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader) == 6)) 
    {
        dscp = BCM_VLAN_GET_IP_DSCP(cmdInfo->ipHeader);
    } 
    else 
    {
        BCM_VLAN_DP_ERROR("Invalid Vlan IP version %d", BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader));
        return 0;
    }

    BCM_VLAN_DP_DEBUG("dscp = %d -> pbits = %d", dscp, cmdInfo->dscpToPbits[dscp]);

    BCM_VLAN_SET_TCI_PBITS_IN_NWK_ORDER(vlanHeader->tci, cmdInfo->dscpToPbits[dscp]);

    return 0;
}

static int cmdHandler_setDscp(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_ipHeader_t *ipHeader;
    UINT16 oldTos;
    UINT16 newTos;
    UINT16 dscp;
    UINT16 icsum16;

    cmdInfo->skb = unshareSkb(cmdInfo, cmdInfo->skb);
    if(cmdInfo->skb == NULL)
    {
        return -EFAULT;
    }

    ipHeader = cmdInfo->ipHeader;

    if(ipHeader == NULL || BCM_VLAN_GET_IP_VERSION(ipHeader) != 4)
    {
        BCM_VLAN_DP_ERROR("Unknown IP Header");
        cmdInfo->localStats->error_UnknownL3Header++;
        return 0;
    }

    oldTos = ntohs(*((UINT16 *)(ipHeader)));
    dscp = BCM_VLAN_CMD_GET_VAL(arg);

    /* update incremental checksum of the IP header */
    newTos = oldTos & ~0x00FC;
    newTos |= dscp << 2;

    icsum16 = htons(_compute_icsum16(0, oldTos, newTos));

    ipHeader->headerChecksum = _apply_icsum(ipHeader->headerChecksum, icsum16);

    /* set the new dscp value */
    *((UINT16 *)(ipHeader)) = htons(newTos);

    return 0;
}

static int cmdHandler_dropFrame(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    struct rtnl_link_stats64 *stats = bcmVlan_devGetStats(cmdInfo->skb->dev);

    if (stats)
    {
        if(cmdInfo->tableDir == BCM_VLAN_TABLE_DIR_RX)
        {
            stats->rx_dropped++;
        }
        else
        {
            stats->tx_dropped++;
        }
    }

    kfree_skb(cmdInfo->skb);

    cmdInfo->skb = NULL;

    return -EFAULT;
}

static int cmdHandler_setSkbPriority(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    cmdInfo->skb->priority = BCM_VLAN_CMD_GET_VAL(arg);

    return 0;
}

static int cmdHandler_setSkbMarkPort(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    cmdInfo->skb->mark = SKBMARK_SET_PORT(cmdInfo->skb->mark, BCM_VLAN_CMD_GET_VAL(arg));

    return 0;
}

static int cmdHandler_setSkbMarkQueue(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    cmdInfo->skb->mark = SKBMARK_SET_Q(cmdInfo->skb->mark, BCM_VLAN_CMD_GET_VAL(arg));

    return 0;
}

static int cmdHandler_setSkbMarkQueueByPbits(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    bcmVlan_vlanHeader_t *vlanHeader;

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "if:%s, dir:%d\n",
      cmdInfo->skb->dev->name, cmdInfo->tableDir);

    vlanHeader = cmdInfo->vlanHeader[BCM_VLAN_CMD_GET_TARGET_TAG(arg)];
    if (vlanHeader != NULL)
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "pbit:%d\n",
          BCM_VLAN_GET_TCI_PBITS(vlanHeader));
        cmdInfo->skb->mark = SKBMARK_SET_Q(cmdInfo->skb->mark,
          BCM_VLAN_GET_TCI_PBITS(vlanHeader));
    }

    return 0;
}

static int cmdHandler_setSkbMarkFlowId(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    cmdInfo->skb->mark = SKBMARK_SET_FLOW_ID(cmdInfo->skb->mark, BCM_VLAN_CMD_GET_VAL(arg));

    return 0;
}

static int cmdHandler_continue(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    return 0;
}

static int cmdHandler_ovrdLearningVid(bcmVlan_cmdInfo_t *cmdInfo, bcmVlan_cmdArg_t *arg)
{
    /* This command causes the accelerator learning bridge to
       use the specified VID in the command when learning the MAC SA */

    return 0;
}

static bcmVlan_cmdHandler_t cmdHandler[BCM_VLAN_OPCODE_MAX] =
    { [BCM_VLAN_OPCODE_NOP] = {"NOP", NULL},
      [BCM_VLAN_OPCODE_POP_TAG] = {"POP_TAG", cmdHandler_popVlanTag},
      [BCM_VLAN_OPCODE_PUSH_TAG] = {"PUSH_TAG", cmdHandler_pushVlanTag},
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
      [BCM_VLAN_OPCODE_DEAGGR_TAG] = {"DEAGGR_TAG", cmdHandler_deaggrVlanTag},
#endif
      [BCM_VLAN_OPCODE_SET_ETHERTYPE] = {"SET_ETHERTYPE", cmdHandler_setEthertype},
      [BCM_VLAN_OPCODE_SET_PBITS] = {"SET_PBITS", cmdHandler_setPbits},
      [BCM_VLAN_OPCODE_SET_CFI] = {"SET_CFI", cmdHandler_setCfi},
      [BCM_VLAN_OPCODE_SET_VID] = {"SET_VID", cmdHandler_setVid},
      [BCM_VLAN_OPCODE_SET_TAG_ETHERTYPE] = {"SET_TAG_ETHERTYPE", cmdHandler_setTagEthertype},
      [BCM_VLAN_OPCODE_COPY_PBITS] = {"COPY_PBITS", cmdHandler_copyPbits},
      [BCM_VLAN_OPCODE_COPY_CFI] = {"COPY_CFI", cmdHandler_copyCfi},
      [BCM_VLAN_OPCODE_COPY_VID] = {"COPY_VID", cmdHandler_copyVid},
      [BCM_VLAN_OPCODE_COPY_TAG_ETHERTYPE] = {"COPY_TAG_ETHERTYPE", cmdHandler_copyTagEthertype},
      [BCM_VLAN_OPCODE_DSCP2PBITS] = {"DSCP2PBITS", cmdHandler_dscp2pbits},
      [BCM_VLAN_OPCODE_SET_DSCP] = {"SET_DSCP", cmdHandler_setDscp},
      [BCM_VLAN_OPCODE_DROP_FRAME] = {"DROP_FRAME", cmdHandler_dropFrame},
      [BCM_VLAN_OPCODE_SET_SKB_PRIO] = {"SET_SKB_PRIO", cmdHandler_setSkbPriority},
      [BCM_VLAN_OPCODE_SET_SKB_MARK_PORT] = {"SET_SKB_MARK_PORT", cmdHandler_setSkbMarkPort},
      [BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE] = {"SET_SKB_MARK_QUEUE", cmdHandler_setSkbMarkQueue},
      [BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE_BYPBITS] = {"SET_SKB_MARK_QUEUE_PBIT", cmdHandler_setSkbMarkQueueByPbits},
      [BCM_VLAN_OPCODE_SET_SKB_MARK_FLOWID] = {"SET_SKB_MARK_FLOWID", cmdHandler_setSkbMarkFlowId},
      [BCM_VLAN_OPCODE_OVRD_LEARNING_VID] = {"OVRD_LEARNING_VID", cmdHandler_ovrdLearningVid},
      [BCM_VLAN_OPCODE_CONTINUE] = {"CONTINUE", cmdHandler_continue} };

static inline bcmVlan_tableEntry_t *allocTableEntry(void)
{
    return kmem_cache_alloc(tableEntryCache, GFP_ATOMIC);
}

static inline void freeTableEntry(bcmVlan_tableEntry_t *tableEntry)
{
    kmem_cache_free(tableEntryCache, tableEntry);
}

static int tagRulePreProcessor(UINT8 nbrOfTags, bcmVlan_tagRule_t *tagRule,
                               bcmVlan_ruleTable_t *ruleTable, UINT32 *cmdCount)
{
    int i;
    bcmVlan_vlanTag_t *vlanTag;

    /* NULL terminate names, just in case */
    tagRule->rxVlanDevName[IFNAMSIZ-1] = '\0';
    tagRule->filter.txVlanDevName[IFNAMSIZ-1] = '\0';
    tagRule->filter.rxRealDevName[IFNAMSIZ-1] = '\0';

    if (tagRule->type < BCM_VLAN_RULE_TYPE_FLOW ||tagRule->type >= BCM_VLAN_RULE_TYPE_INVALID)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid rule type: type = %d",
                      tagRule->type);
        goto out_error;
    }
	
    if(!BCM_VLAN_DSCP_FILT_IS_VALID(tagRule->filter.dscp))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid filter: DSCP = %d",
                      tagRule->filter.dscp);
        goto out_error;
    }

    if(!BCM_VLAN_FLOWID_FILT_IS_VALID(tagRule->filter.skbMarkFlowId))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid filter: SKB Mark FlowID = %d",
                      tagRule->filter.skbMarkFlowId);
        goto out_error;
    }

    if(!BCM_VLAN_PORT_FILT_IS_VALID(tagRule->filter.skbMarkPort))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid filter: SKB Mark Port = %d",
                      tagRule->filter.skbMarkPort);
        goto out_error;
    }

    if(!BCM_VLAN_IPPROTO_FILT_IS_VALID(tagRule->filter.ipProto))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid filter: IP Proto = %d",
                      tagRule->filter.ipProto);
        goto out_error;
    }

    /* process the VLAN tag filters */
    for(i=0; i<nbrOfTags; ++i)
    {
        vlanTag = &tagRule->filter.vlanTag[i];

        if(!BCM_VLAN_PBITS_FILT_IS_VALID(vlanTag->pbits) ||
           !BCM_VLAN_CFI_FILT_IS_VALID(vlanTag->cfi) ||
           !BCM_VLAN_VID_FILT_IS_VALID(vlanTag->vid))
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid filter: VLAN Tag #%d", i);
            goto out_error;
        }

        vlanTag->tciMask = 0;
        vlanTag->tci = 0;

        if(!BCM_VLAN_IS_DONT_CARE(vlanTag->pbits))
        {
            vlanTag->tciMask |= BCM_VLAN_PBITS_MASK;
            BCM_VLAN_SET_TCI_PBITS(vlanTag->tci, vlanTag->pbits);
        }

        if(!BCM_VLAN_IS_DONT_CARE(vlanTag->cfi))
        {
            vlanTag->tciMask |= BCM_VLAN_CFI_MASK;
            BCM_VLAN_SET_TCI_CFI(vlanTag->tci, vlanTag->cfi);
        }

        if(!BCM_VLAN_IS_DONT_CARE(vlanTag->vid))
        {
            vlanTag->tciMask |= BCM_VLAN_VID_MASK;
            BCM_VLAN_SET_TCI_VID(vlanTag->tci, vlanTag->vid);
        }
    }

    /* initialize the remaining filters so we can match when
       additional tags are pushed */
    for(i=nbrOfTags; i<BCM_VLAN_MAX_TAGS; ++i)
    {
        vlanTag = &tagRule->filter.vlanTag[i];

        vlanTag->tciMask = 0;
        vlanTag->tci = 0;
    }

    *cmdCount = 0;

    for(i=0; i<BCM_VLAN_MAX_RULE_COMMANDS; ++i)
    {
        switch(tagRule->cmd[i].opCode)
        {
            case BCM_VLAN_OPCODE_NOP:
            case BCM_VLAN_OPCODE_SET_ETHERTYPE:
                break;

            case BCM_VLAN_OPCODE_PUSH_TAG:
                if(nbrOfTags >= BCM_VLAN_MAX_TAGS)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Command: #%d: Max number of tags exceeded", i);
                    goto out_error;
                }
                /* save the default tpid and default tci in the command arguments */
                BCM_VLAN_CMD_SET_VAL(tagRule->cmd[i].arg, ruleTable->defaultTpid);
                BCM_VLAN_CMD_SET_VAL2(tagRule->cmd[i].arg, BCM_VLAN_CREATE_TCI(ruleTable->defaultPbits,
                                                                               ruleTable->defaultCfi,
                                                                               ruleTable->defaultVid));
                break;

            case BCM_VLAN_OPCODE_POP_TAG:
                if(nbrOfTags == 0)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Command: #%d: Cannot remove tag from untagged frame", i);
                    goto out_error;
                }
                break;

#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
            case BCM_VLAN_OPCODE_DEAGGR_TAG:
                if(nbrOfTags == 0)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Command: #%d: Cannot de-aggregate tag from untagged frame", i);
                    goto out_error;
                }
                break;
#endif
            case BCM_VLAN_OPCODE_SET_TAG_ETHERTYPE:
            case BCM_VLAN_OPCODE_DSCP2PBITS:
                if(BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg) >= BCM_VLAN_MAX_TAGS)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument: Target %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_SET_PBITS:
                if((BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg) >= BCM_VLAN_MAX_TAGS) ||
                   !BCM_VLAN_PBITS_IS_VALID(BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg)))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Value %d, Target %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg),
                                  BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_SET_CFI:
                if((BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg) >= BCM_VLAN_MAX_TAGS) ||
                   !BCM_VLAN_CFI_IS_VALID(BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg)))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Value %d, Target %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg),
                                  BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_SET_VID:
                if((BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg) >= BCM_VLAN_MAX_TAGS) ||
                   !BCM_VLAN_VID_IS_VALID(BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg)))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Value %d, Target %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg),
                                  BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_COPY_PBITS:
            case BCM_VLAN_OPCODE_COPY_CFI:
            case BCM_VLAN_OPCODE_COPY_VID:
            case BCM_VLAN_OPCODE_COPY_TAG_ETHERTYPE:
                if((BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg) >= BCM_VLAN_MAX_TAGS) ||
                   (BCM_VLAN_CMD_GET_SOURCE_TAG(tagRule->cmd[i].arg) >= BCM_VLAN_MAX_TAGS))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Source %d, Target %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_SOURCE_TAG(tagRule->cmd[i].arg),
                                  BCM_VLAN_CMD_GET_TARGET_TAG(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_SET_DSCP:
                if(!BCM_VLAN_DSCP_IS_VALID(BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg)))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Value %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_DROP_FRAME:
                break;

            case BCM_VLAN_OPCODE_SET_SKB_PRIO:
                break;

            case BCM_VLAN_OPCODE_SET_SKB_MARK_PORT:
                if(!BCM_VLAN_PORT_IS_VALID(BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg)))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Value %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE:
                if(!BCM_VLAN_QUEUE_IS_VALID(BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg)))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Value %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_SET_SKB_MARK_FLOWID:
                if(!BCM_VLAN_FLOWID_IS_VALID(BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg)))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d: %s: Invalid argument(s): Value %d",
                                  i, cmdHandler[tagRule->cmd[i].opCode].name,
                                  BCM_VLAN_CMD_GET_VAL(tagRule->cmd[i].arg));
                    goto out_error;
                }
                break;

            case BCM_VLAN_OPCODE_OVRD_LEARNING_VID:
            case BCM_VLAN_OPCODE_CONTINUE:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE_BYPBITS:
                break;

            default:
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Command #%d': Invalid opCode %d",
                              i, tagRule->cmd[i].opCode);
                goto out_error;
        }

        if(tagRule->cmd[i].opCode != BCM_VLAN_OPCODE_NOP)
        {
            (*cmdCount)++;
        }
        else
        {
            break;
        }
    }

    return 0;

out_error:
    return -EINVAL;
}

static bcmVlan_tableEntry_t *removeTableEntryById(bcmVlan_ruleTable_t *ruleTable,
                                                  bcmVlan_tagRuleIndex_t tagRuleId)
{
    bcmVlan_tableEntry_t *tableEntry;

    BCM_ASSERT(ruleTable);

    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

    while(tableEntry)
    {
        if(tableEntry->tagRule.id == tagRuleId)
        {
            BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Freeing Entry Id: %d", (int)tableEntry->tagRule.id);

            _BCM_VLAN_LL_REMOVE(&ruleTable->tableEntryLL, tableEntry);

            if (tableEntry == ruleTable->lastTableEntry)
            {
                ruleTable->lastTableEntry = NULL;
            }

            break;
        }

        tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
    };

    if(tableEntry == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not find Tag Rule Id %d", (int)tagRuleId);
    }
    else if(BCM_VLAN_LL_IS_EMPTY(&ruleTable->tableEntryLL))
    {
        ruleTable->tagRuleIdCount = 0;
    }

    return tableEntry;
}

static bcmVlan_tableEntry_t *removeTableEntryByFilter(bcmVlan_ruleTable_t *ruleTable,
                                                      bcmVlan_tagRuleFilter_t *tagRuleFilter)
{
    bcmVlan_tableEntry_t *tableEntry;

    BCM_ASSERT(ruleTable);
    BCM_ASSERT(tagRuleFilter);

    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

    while(tableEntry)
    {
        if(!memcmp(&tableEntry->tagRule.filter, tagRuleFilter, sizeof(bcmVlan_tagRuleFilter_t)))
        {
            BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Freeing Entry Id: %d", (int)tableEntry->tagRule.id);

            _BCM_VLAN_LL_REMOVE(&ruleTable->tableEntryLL, tableEntry);

            if (tableEntry == ruleTable->lastTableEntry)
            {
                ruleTable->lastTableEntry = NULL;
            }

            break;
        }

        tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
    };

    if(tableEntry == NULL)
    {
        BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Could not find matching Tag Rule");
    }
    else if(BCM_VLAN_LL_IS_EMPTY(&ruleTable->tableEntryLL))
    {
        ruleTable->tagRuleIdCount = 0;
    }

    return tableEntry;
}

static int removeTableEntries(bcmVlan_ruleTable_t *ruleTable, struct net_device *vlanDev)
{
    int count = 0;
    bcmVlan_tableEntry_t *tableEntry, *nextTableEntry;

    BCM_ASSERT(ruleTable);

    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

    while(tableEntry)
    {
        nextTableEntry = tableEntry;

        nextTableEntry = BCM_VLAN_LL_GET_NEXT(nextTableEntry);

/*         BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "vlanDev 0x%08X, rxVlanDev 0x%08X, txVlanDev 0x%08X", */
/*                       (unsigned int)vlanDev, */
/*                       (unsigned int)tableEntry->rxVlanDev, */
/*                       (unsigned int)tableEntry->txVlanDev); */

        if(vlanDev == NULL || tableEntry->rxVlanDev == vlanDev || tableEntry->txVlanDev == vlanDev)
        {
            BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Freeing Tag Rule Id %d", (int)tableEntry->tagRule.id);

            count++;

            _BCM_VLAN_LL_REMOVE(&ruleTable->tableEntryLL, tableEntry);

            if (tableEntry == ruleTable->lastTableEntry)
            {
                ruleTable->lastTableEntry = NULL;
            }

            freeTableEntry(tableEntry);
        }

        tableEntry = nextTableEntry;
    };

    if(BCM_VLAN_LL_IS_EMPTY(&ruleTable->tableEntryLL))
    {
        ruleTable->tagRuleIdCount = 0;
    }

    return count;
}

static bcmVlan_tableEntry_t *getTableEntryById(bcmVlan_ruleTable_t *ruleTable, bcmVlan_tagRuleIndex_t tagRuleId)
{
    bcmVlan_tableEntry_t *tableEntry;

    BCM_ASSERT(ruleTable);

    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

    while(tableEntry)
    {
        if(tableEntry->tagRule.id == tagRuleId)
        {
            break;
        }

        tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
    };

    return tableEntry;
}

static char *printValName(unsigned int val, int format, int size)
{
    static char name[IFNAMSIZ];

    if(format == 0)
    {
        sprintf(name, "%d", val);
    }
    else
    {
        switch(size)
        {
            case sizeof(UINT8):
                sprintf(name, "0x%02X", val);
                break;

            case sizeof(UINT16):
                sprintf(name, "0x%04X", val);
                break;

            case sizeof(UINT32):
                sprintf(name, "0x%08X", val);
                break;

            default:
                sprintf(name, "ERROR");
        }
    }

    return name;
}

static void dumpTableEntry(char *realDevName,
                           bcmVlan_realDevMode_t realDevMode,
                           UINT8 nbrOfTags,
                           bcmVlan_ruleTableDirection_t tableDir,
                           bcmVlan_tableEntry_t *tableEntry)
{
    int i, j;
    bcmVlan_tagRuleFilter_t *filter_p;

    printk("\n--------------------------------------------------------------------------------\n");

    printk("===> %s (%s) : %s, %d tag(s)\n",
           (realDevName) ? realDevName : "NONAME",
           (realDevName) ? (realDevMode == BCM_VLAN_MODE_ONT ? "ONT" : "RG") : "UNKNOWN",
           (tableDir==BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX", nbrOfTags);

    printk("Tag Rule ID : %d\n", (int)tableEntry->tagRule.id);

    if(tableEntry->rxVlanDev)
    {
        printk("Rx VLAN Device : %s%s\n", tableEntry->rxVlanDev->name, tableEntry->tagRule.isIptvOnly? " <IPTV traffic only>" : "");
    }
    else
    {
        printk("Rx VLAN Device : %s\n", tableEntry->tagRule.rxVlanDevName);
    }

    filter_p = &tableEntry->tagRule.filter;

    printk("\nFilters\n");
    if(tableDir == BCM_VLAN_TABLE_DIR_TX)
    {
        /* --filter-rxif and --filter-txif only apply to tx tag rules */
        printk("\tRx REALIF       : %s\n", filter_p->rxRealDevName);
        printk("\tTx VLANIF       : %s\n", filter_p->txVlanDevName);
    }
    else
    {
        /* --filter-vlan-dev-mac-addr only applies to rx tag rules */
        if (filter_p->vlanDevMacAddr == BCM_VLAN_MATCH_VLANDEV_MACADDR_OR_MULTICAST)
        {
            printk("\tVlanDev MacAddr : Yes (ignore if multicast)\n");
        }
        else if (filter_p->vlanDevMacAddr == BCM_VLAN_MATCH_VLANDEV_MACADDR)
        {
            printk("\tVlanDev MacAddr : Yes\n");
        }
        else
        {
            printk("\tVlanDev MacAddr : No\n");
        }
    }
    if(filter_p->flags)
    {
        printk("\tFlags           : ");
        if(filter_p->flags & BCM_VLAN_FILTER_FLAGS_IS_UNICAST)
        {
            printk("UCAST ");
        }
        if(filter_p->flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST)
        {
            printk("MCAST ");
        }
        if(filter_p->flags & BCM_VLAN_FILTER_FLAGS_IS_BROADCAST)
        {
            printk("BCAST ");
        }
        printk("\n");
    }
    if(!BCM_VLAN_IS_DONT_CARE(filter_p->skbPrio) ||
       !BCM_VLAN_IS_DONT_CARE(filter_p->skbMarkFlowId) ||
       !BCM_VLAN_IS_DONT_CARE(filter_p->skbMarkPort))
    {
        printk("\tSKB             : ");
        printk("priority %s, ", BCM_VLAN_PRINT_VAL(filter_p->skbPrio, 0));
        printk("mark->flowId %s, ", BCM_VLAN_PRINT_VAL(filter_p->skbMarkFlowId, 0));
        printk("mark->port %s\n", BCM_VLAN_PRINT_VAL(filter_p->skbMarkPort, 0));
    }
    if(!BCM_VLAN_IS_DONT_CARE(filter_p->etherType))
    {
        printk("\tEtherType       : %s\n", BCM_VLAN_PRINT_VAL(filter_p->etherType, 1));
    }
    if(!BCM_VLAN_IS_DONT_CARE(filter_p->ipProto))
    {
        printk("\tIP Protocol     : %s\n", BCM_VLAN_PRINT_VAL(filter_p->ipProto, 0));
    }
    if(!BCM_VLAN_IS_DONT_CARE(filter_p->dscp))
    {
        printk("\tIPv4 DSCP       : %s\n", BCM_VLAN_PRINT_VAL(filter_p->dscp, 0));
    }
    for(i=0; i<BCM_VLAN_MAX_TAGS; ++i)
    {
        if(!BCM_VLAN_IS_DONT_CARE(filter_p->vlanTag[i].pbits) ||
           !BCM_VLAN_IS_DONT_CARE(filter_p->vlanTag[i].cfi) ||
           !BCM_VLAN_IS_DONT_CARE(filter_p->vlanTag[i].vid) ||
           !BCM_VLAN_IS_DONT_CARE(filter_p->vlanTag[i].etherType))
        {
            printk("\tVLAN Tag %d      : ", i);
            printk("pbits %s, ", BCM_VLAN_PRINT_VAL(filter_p->vlanTag[i].pbits, 0));
            printk("cfi %s, ", BCM_VLAN_PRINT_VAL(filter_p->vlanTag[i].cfi, 0));
            printk("vid %s, ", BCM_VLAN_PRINT_VAL(filter_p->vlanTag[i].vid, 0));
            printk("(tci 0x%04X/0x%04X), ", filter_p->vlanTag[i].tciMask,
                   filter_p->vlanTag[i].tci);
            printk("ether %s\n", BCM_VLAN_PRINT_VAL(filter_p->vlanTag[i].etherType, 1));
        }
    }

    printk("\nCommands");
    for(i=0; i<BCM_VLAN_MAX_RULE_COMMANDS; ++i)
    {
        if(i > 0 && tableEntry->tagRule.cmd[i].opCode == BCM_VLAN_OPCODE_NOP)
        {
            break;
        }
        printk("\n\t");
        printk("%02d:[%s", i, cmdHandler[tableEntry->tagRule.cmd[i].opCode].name);
        for(j=0; j<BCM_VLAN_MAX_CMD_ARGS; ++j)
        {
            printk(", 0x%08X", (int)tableEntry->tagRule.cmd[i].arg[j]);
        }
        printk("] ");
    }
    printk("\n\n");

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Merge Count : %d\n", tableEntry->mergeCount);

    printk("Rule Type  : %s\n", tableEntry->tagRule.type == BCM_VLAN_RULE_TYPE_FLOW ? "Flow" : "QoS");
    printk("Hit Count   : %d\n", tableEntry->hitCount);
}


/*
 * Global functions
 */

#if defined(CC_BCM_VLAN_FLOW)
int bcmVlan_blogVlanFlows(Blog_t *blog_p,
                          struct net_device *rxVlanDev,
                          struct net_device *txVlanDev);
#endif

int bcmVlan_initTagRules(void)
{
    int ret = 0;

    /* create a slab cache for device descriptors */
    tableEntryCache = kmem_cache_create("bcmvlan_tableEntry",
                                        sizeof(bcmVlan_tableEntry_t),
                                        0, /* align */
                                        SLAB_HWCACHE_ALIGN, /* flags */
                                        NULL); /* ctor */
    if(tableEntryCache == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Unable to create Table Entry cache");

        ret = -ENOMEM;
        goto out;
    }

#if defined(CC_BCM_VLAN_FLOW)
    if(blogRuleVlanHook != NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Unable to bind Blog Rule VLAN Hook");

        ret = -EINVAL;
        goto out;
    }

    blogRuleVlanHook = bcmVlan_blogVlanFlows;
#endif

out:
    return ret;
}

void bcmVlan_cleanupTagRules(void)
{
    kmem_cache_destroy(tableEntryCache);

#if defined(CC_BCM_VLAN_FLOW)
    if(blogRuleVlanHook == bcmVlan_blogVlanFlows)
    {
        blogRuleVlanHook = NULL;
    }
#endif
}

/*
 * Entry points: IOCTL, indirect
 */
void bcmVlan_initTpidTable(struct realDeviceControl *realDevCtrl)
{
    int i;

    for(i=0; i<BCM_VLAN_MAX_TPID_VALUES; ++i)
    {
        realDevCtrl->tpidTable[i] = ETH_P_8021Q;
    }
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_setTpidTable(char *realDevName, unsigned int *tpidTable)
{
    int ret = 0;
    int i;
#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)) && defined(CONFIG_BLOG)
    int j;
    int foundInOldTableCntr = 0;
#endif
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);
        BCM_VLAN_GLOBAL_UNLOCK();
        return -EINVAL;
    }

    for(i=0; i<BCM_VLAN_MAX_TPID_VALUES; ++i)
    {
        realDevCtrl->tpidTable[i] = tpidTable[i];
    }

    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)) && defined(CONFIG_BLOG)
    for(i=0; i<BCM_VLAN_MAX_TPID_VALUES; ++i)
    {
        for (j=0; j<BCM_VLAN_MAX_TPID_VALUES; ++j)
        {
            if(oldTpidTable[j] == tpidTable[i])
            {
                foundInOldTableCntr ++;
                break;
            }
        }
    }
    if(foundInOldTableCntr < BCM_VLAN_MAX_TPID_VALUES)
    {
        tpidTableChanged = 1;
        for (i=0; i<BCM_VLAN_MAX_TPID_VALUES; ++i)
        {
            oldTpidTable[i] = tpidTable[i];
        }
    }
    if(tpidTableChanged)
    {
        tpidTableChanged = 0;
        vlanctl_notify(VLANCTL_BIND_NOTIFY_TPID, (void*)&realDevCtrl->tpidTable, VLANCTL_BIND_CLIENT_RUNNER);
    }
#endif

     return ret;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_dumpTpidTable(char *realDevName)
{
    int ret = 0;
    int i;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    printk("%s: TPID Table : ", realDev->name);

    for(i=0; i<BCM_VLAN_MAX_TPID_VALUES; ++i)
    {
        printk("0x%04X ", realDevCtrl->tpidTable[i]);
    }

    printk("\n");

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

static void initRuleTable(bcmVlan_ruleTable_t *ruleTable_p,
                          bcmVlan_ruleTableDirection_t tableDir)
{
    ruleTable_p->tagRuleIdCount = 0;

    ruleTable_p->defaultTpid = BCM_VLAN_DEFAULT_TAG_TPID;
    ruleTable_p->defaultPbits = BCM_VLAN_DEFAULT_TAG_PBITS;
    ruleTable_p->defaultCfi = BCM_VLAN_DEFAULT_TAG_CFI;
    ruleTable_p->defaultVid = BCM_VLAN_DEFAULT_TAG_VID;

    /*
     * Default actions: The following defaults were chosen based on the most typical usage of
     *                  VLAN interfaces, as well as limitations imposed at initialiation time.
     * Rx = DROP   : When the Rx default is ACCEPT, the default Rx Interface must be defined.
     *               Since we don't know the the default Rx Interface at initialization time,
     *               we must initialize the Rx default as DROP. DROP is also the most common
     *               default behavior.
     * Tx = ACCEPT : In the Tx direction the target interface is always the Real Interface, so
     *               we could choose eithe ACCEPT or DROP. ACCEPT was chosen because it is the
     *               most common behavior for the Tx direction.
     */
    ruleTable_p->defaultAction = (tableDir == BCM_VLAN_TABLE_DIR_RX) ?
        BCM_VLAN_ACTION_DROP : BCM_VLAN_ACTION_ACCEPT;

    ruleTable_p->defaultRxVlanDev = NULL;

    ruleTable_p->lastTableEntry = NULL;

    BCM_VLAN_LL_INIT(&ruleTable_p->tableEntryLL);
}

/*
 * Entry points: IOCTL, indirect
 */
void bcmVlan_initRuleTables(struct realDeviceControl *realDevCtrl)
{
    int i, j;

    for(i=0; i<BCM_VLAN_TABLE_DIR_MAX; ++i)
    {
        for(j=0; j<BCM_VLAN_MAX_RULE_TABLES; ++j)
        {
            initRuleTable(&realDevCtrl->ruleTable[i][j], i);
        }
    }

    for(i=0; i<BCM_VLAN_MAX_DSCP_VALUES; ++i)
    {
        realDevCtrl->dscpToPbits[i] = (UINT8)(i & 0x7);
    }
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_setDefaultAction(char *realDevName, UINT8 nbrOfTags,
                             bcmVlan_ruleTableDirection_t tableDir,
                             bcmVlan_defaultAction_t defaultAction,
                             char *defaultRxVlanDevName)
{
    int ret = 0;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;
    bcmVlan_ruleTable_t *ruleTable;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    if(nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Number of Tags: %d (max %d)",
                      nbrOfTags, BCM_VLAN_MAX_RULE_TABLES);

        ret = -EINVAL;
        goto out_error;
    }

    if(tableDir >= BCM_VLAN_TABLE_DIR_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Direction: %d (max %d)",
                      tableDir, BCM_VLAN_TABLE_DIR_MAX);

        ret = -EINVAL;
        goto out_error;
    }

    if(defaultAction >= BCM_VLAN_ACTION_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Default Action: %d", defaultAction);

        ret = -EINVAL;
        goto out_error;
    }

    ruleTable = &realDevCtrl->ruleTable[tableDir][nbrOfTags];

    if(tableDir == BCM_VLAN_TABLE_DIR_RX && defaultAction == BCM_VLAN_ACTION_ACCEPT)
    {
        ruleTable->defaultRxVlanDev = bcmVlan_getVlanDeviceByName(realDevCtrl, defaultRxVlanDevName);
        if(ruleTable->defaultRxVlanDev == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Default Rx VLAN Device %s does not exist",
                          defaultRxVlanDevName);

            ret = -EINVAL;
            goto out_error;
        }
    }
    else /* TX */
    {
        ruleTable->defaultRxVlanDev = NULL;
    }

    ruleTable->defaultAction = defaultAction;

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL, indirect
 */
void bcmVlan_cleanupRxDefaultActions(struct net_device *realDev,
                                     struct net_device *vlanDev)
{
    UINT8 nbrOfTags;
    bcmVlan_ruleTable_t *ruleTable;
    struct realDeviceControl *realDevCtrl = NULL;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
    if(realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDev->name);

        BCM_VLAN_GLOBAL_UNLOCK();
        return;
    }

    for(nbrOfTags=0; nbrOfTags<BCM_VLAN_MAX_RULE_TABLES; nbrOfTags++)
    {
        ruleTable = &realDevCtrl->ruleTable[BCM_VLAN_TABLE_DIR_RX][nbrOfTags];

        if(ruleTable->defaultAction == BCM_VLAN_ACTION_ACCEPT &&
           ruleTable->defaultRxVlanDev == vlanDev)
        {
            bcmVlan_setDefaultAction(realDev->name, nbrOfTags,
                                     BCM_VLAN_TABLE_DIR_RX,
                                     BCM_VLAN_ACTION_DROP,
                                     NULL);

            BCM_LOG_NOTICE(BCM_LOG_ID_VLAN, "%s, rx, %d tags: default-miss set to DROP",
                           realDev->name, nbrOfTags);
        }
    }

    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_setRealDevMode(char *realDevName, bcmVlan_realDevMode_t mode)
{
    int ret = 0;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    if(mode >= BCM_VLAN_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Real Device Mode: %d (max %d)",
                      mode, BCM_VLAN_MODE_MAX);

        ret = -EINVAL;
        goto out_error;
    }

    realDevCtrl->mode = mode;

    printk("BCMVLAN : %s mode was set to %s\n", realDevCtrl->realDev->name,
           realDevCtrl->mode == BCM_VLAN_MODE_ONT ? "ONT" : "RG");

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL indirect, NOTIFIER, CLEANUP
 */
void bcmVlan_cleanupRuleTables(struct realDeviceControl *realDevCtrl)
{
    int i, j;

    for(i=0; i<BCM_VLAN_TABLE_DIR_MAX; ++i)
    {
        for(j=0; j<BCM_VLAN_MAX_RULE_TABLES; ++j)
        {
            removeTableEntries(&realDevCtrl->ruleTable[i][j], NULL);

            realDevCtrl->ruleTable[i][j].tagRuleIdCount = 0;

            BCM_VLAN_LL_INIT(&realDevCtrl->ruleTable[i][j].tableEntryLL);
        }
    }
}

static void notifyMcast(struct realDeviceControl *realDevCtrl,
                        struct net_device *realDev,
                        UINT8 nbrOfTags, bcmVlan_ruleTableDirection_t tableDir,
                        bcmVlan_tableEntry_t *tableEntry)
{
#if defined(CONFIG_BLOG)
    blogRuleVlanNotifyDirection_t blogRuledir;
#ifdef CONFIG_BCM_PON
    /*for pon platforms, mcast vlan is not expected to be changed while there are active iptv flow*/
    return;      
#endif
    if(blogRuleVlanNotifyHook == NULL)
    {
        return;
    }
    blogRuledir = (tableDir == BCM_VLAN_TABLE_DIR_RX) ?
        BLOG_RULE_VLAN_NOTIFY_DIR_RX : BLOG_RULE_VLAN_NOTIFY_DIR_TX;

    if(tableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        if(realDevCtrl->mode == BCM_VLAN_MODE_RG)
        {
            if(tableEntry->rxVlanDev != NULL)
            {
                blogRuleVlanNotifyHook(tableEntry->rxVlanDev, blogRuledir, nbrOfTags);
            }
            else
            {
                blogRuleVlanNotifyHook(realDev, blogRuledir, nbrOfTags);
            }
        }
        else /* RX, ONT */
        {
            struct vlanDeviceControl *vlanDevCtrl;

            vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);

            while(vlanDevCtrl)
            {
                if(vlanDevCtrl->flags.multicast)
                {
                    blogRuleVlanNotifyHook(vlanDevCtrl->vlanDev, blogRuledir, nbrOfTags);
                }

                vlanDevCtrl = BCM_VLAN_LL_GET_NEXT(vlanDevCtrl);
            }
        }
    }
    else /* TX */
    {
        if(tableEntry->txVlanDev != NULL)
        {
            blogRuleVlanNotifyHook(tableEntry->txVlanDev, blogRuledir, nbrOfTags);
        }
        else /* TX vlan not specified */
        {
            struct vlanDeviceControl *vlanDevCtrl;

            /* rule applies to all Tx interfaces */

            vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);

            while(vlanDevCtrl)
            {
                blogRuleVlanNotifyHook(vlanDevCtrl->vlanDev, blogRuledir, nbrOfTags);

                vlanDevCtrl = BCM_VLAN_LL_GET_NEXT(vlanDevCtrl);
            }
        }
    }
#endif /* CONFIG_BLOG */
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_insertTagRule(char *realDevName, UINT8 nbrOfTags, bcmVlan_ruleTableDirection_t tableDir,
                          bcmVlan_tagRule_t *tagRule, bcmVlan_ruleInsertPosition_t position,
                          bcmVlan_tagRuleIndex_t posTagRuleId)
{
    int ret = 0;
    struct net_device *realDev;
    struct net_device *rxVlanDev;
    struct net_device *txVlanDev;
    struct net_device *rxRealDev;
    struct realDeviceControl *realDevCtrl = NULL;
    bcmVlan_tableEntry_t *tableEntry, *posTableEntry;
    bcmVlan_ruleTable_t *ruleTable;
    UINT32 cmdCount;

    BCM_ASSERT(tagRule);

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out;
    }

    if(nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Number of Tags: %d (max %d)",
                      nbrOfTags, BCM_VLAN_MAX_RULE_TABLES);

        ret = -EINVAL;
        goto out;
    }

    if(tableDir >= BCM_VLAN_TABLE_DIR_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Direction: %d (max %d)",
                      tableDir, BCM_VLAN_TABLE_DIR_MAX);

        ret = -EINVAL;
        goto out;
    }

    ruleTable = &realDevCtrl->ruleTable[tableDir][nbrOfTags];

    ret = tagRulePreProcessor(nbrOfTags, tagRule, ruleTable, &cmdCount);
    if(ret)
    {
        goto out;
    }

    if(position >= BCM_VLAN_POSITION_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid rule insertion position: %d (max %d)",
                      position, BCM_VLAN_POSITION_MAX);

        ret = -EINVAL;
        goto out;
    }

    if(tableDir == BCM_VLAN_TABLE_DIR_TX)
    {
        struct net_device *realDevTmp = NULL;
        struct realDeviceControl *realDevCtrlTmp = NULL;
        rxVlanDev = NULL;
        rxRealDev = NULL;
    
        realDevTmp = bcmVlan_getRealDeviceByName(tagRule->filter.rxRealDevName, &realDevCtrlTmp);
        if (realDevTmp != NULL || realDevCtrlTmp != NULL)
        {
            rxVlanDev = bcmVlan_getVlanDeviceByName(realDevCtrlTmp, tagRule->rxVlanDevName);
        }

        if(tagRule->filter.rxRealDevName[0] != '\0')
        {        
            rxRealDev = dev_get_by_name(&init_net, tagRule->filter.rxRealDevName);
            if(rxRealDev == NULL)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Rx Real Device %s does not exist", tagRule->filter.rxRealDevName);

                ret = -EINVAL;
                goto out;
            }
        }
    }
    else
    {
        rxVlanDev = bcmVlan_getVlanDeviceByName(realDevCtrl, tagRule->rxVlanDevName);
        if(rxVlanDev == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLAN Device %s does not exist", tagRule->rxVlanDevName);

            ret = -EINVAL;
            goto out;
        }
        
        rxRealDev = rxVlanDev;
        while(!netdev_path_is_root(rxRealDev))
            rxRealDev = netdev_path_next_dev(rxRealDev);
        if(rxRealDev == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Can not find root of VLAN Device %s", tagRule->rxVlanDevName);

            ret = -EINVAL;
            goto out;
        }
    }

    if(tableDir == BCM_VLAN_TABLE_DIR_RX || !strcmp(tagRule->filter.txVlanDevName, BCM_VLAN_DEFAULT_DEV_NAME))
    {
        txVlanDev = NULL;
    }
    else
    {
        txVlanDev = bcmVlan_getVlanDeviceByName(realDevCtrl, tagRule->filter.txVlanDevName);
        if(txVlanDev == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLAN Device %s does not exist", tagRule->filter.txVlanDevName);

            ret = -EINVAL;
            goto out;
        }
    }

    /* allocate a new table entry */
    tableEntry = allocTableEntry();
    if(tableEntry == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

        ret = -ENOMEM;
        goto out;
    }

    /* allocate a new Tag Rule ID */
    tagRule->id = ruleTable->tagRuleIdCount++;

    /* initialize table entry */
    tableEntry->cmdCount   = cmdCount;
    tableEntry->hitCount   = 0;
    tableEntry->mergeCount = 0;

    tableEntry->tagRule   = *tagRule;
    tableEntry->rxRealDev = rxRealDev;
    tableEntry->rxVlanDev = rxVlanDev;
    tableEntry->txVlanDev = txVlanDev;
    
    if(position == BCM_VLAN_POSITION_LAST)
    {
        if(ruleTable->lastTableEntry != NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "The last rule table entry has already been occupied");

            freeTableEntry(tableEntry);

            ret = -EINVAL;
            goto out;
        }

        /* append tag rule to the tail */
        _BCM_VLAN_LL_APPEND(&ruleTable->tableEntryLL, tableEntry);

        /* indicate that the last table entry has been occupied. */
        ruleTable->lastTableEntry = tableEntry;
    }
    else if((position == BCM_VLAN_POSITION_APPEND) ||
            ((position == BCM_VLAN_POSITION_AFTER) && BCM_VLAN_IS_DONT_CARE(posTagRuleId)))
    {
        if(ruleTable->lastTableEntry == NULL)
        {
            /* append tag rule to the tail */
            _BCM_VLAN_LL_APPEND(&ruleTable->tableEntryLL, tableEntry);
        }
        else
        {
            /* add tag rule before the last table entry */
            _BCM_VLAN_LL_INSERT(&ruleTable->tableEntryLL, tableEntry, BCM_VLAN_POSITION_BEFORE,
                               ruleTable->lastTableEntry);
        }
    }
    else if((position == BCM_VLAN_POSITION_BEFORE) && BCM_VLAN_IS_DONT_CARE(posTagRuleId))
    {
        /* add tag rule to the head */
        posTableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

        _BCM_VLAN_LL_INSERT(&ruleTable->tableEntryLL, tableEntry, position, posTableEntry);
    }
    else
    {
        posTableEntry = getTableEntryById(ruleTable, posTagRuleId);
        if(posTableEntry == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not find insertion point: Tag Rule ID %d", (int)posTagRuleId);

            freeTableEntry(tableEntry);

            ret = -EINVAL;
            goto out;
        }
        if(posTableEntry == ruleTable->lastTableEntry && position == BCM_VLAN_POSITION_AFTER)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "The last rule table entry has already been occupied");

            freeTableEntry(tableEntry);

            ret = -EINVAL;
            goto out;
        }

        _BCM_VLAN_LL_INSERT(&ruleTable->tableEntryLL, tableEntry, position, posTableEntry);
    }

    if (!ret && tableEntry->tagRule.isIptvOnly && rxVlanDev)
    {
        ret = bcmVlan_setIptvOnlyVlanDevice(rxVlanDev);
    }

out:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    if(!ret)
    {
        if(!tagRule->filter.flags ||
           (tagRule->filter.flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST))
        {
            notifyMcast(realDevCtrl, realDev, nbrOfTags, tableDir, tableEntry);
        }
    }

    return ret;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_removeTagRuleById(char *realDevName, UINT8 nbrOfTags,
                              bcmVlan_ruleTableDirection_t tableDir, bcmVlan_tagRuleIndex_t tagRuleId)
{
    int ret = 0;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;
    bcmVlan_tableEntry_t *tableEntry;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    if(nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Number of Tags: %d (max %d)",
                      nbrOfTags, BCM_VLAN_MAX_RULE_TABLES);

        ret = -EINVAL;
        goto out_error;
    }

    if(tableDir >= BCM_VLAN_TABLE_DIR_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Direction: %d (max %d)",
                      tableDir, BCM_VLAN_TABLE_DIR_MAX);

        ret = -EINVAL;
        goto out_error;
    }

    tableEntry = removeTableEntryById(&realDevCtrl->ruleTable[tableDir][nbrOfTags], tagRuleId);
    if(tableEntry == NULL)
    {
        ret = -EINVAL;
    }
    else
    {
        if(!tableEntry->tagRule.filter.flags ||
           (tableEntry->tagRule.filter.flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST))
        {
            notifyMcast(realDevCtrl, realDev, nbrOfTags, tableDir, tableEntry);
        }

        if(tableEntry->tagRule.isIptvOnly)
        {
            if(!tableEntry->rxVlanDev)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLAN Device %s does not exist", tableEntry->tagRule.rxVlanDevName);

                ret = -EINVAL;
                goto out_error;
            }

            ret = bcmVlan_unsetIptvOnlyVlanDevice(tableEntry->rxVlanDev, FALSE);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "IPTV only flag set failed on VLAN Device %s", tableEntry->tagRule.rxVlanDevName);
                goto out_error;
            }
        }

        freeTableEntry(tableEntry);
    }

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_removeTagRuleByFilter(char *realDevName, UINT8 nbrOfTags,
                                  bcmVlan_ruleTableDirection_t tableDir,
                                  bcmVlan_tagRule_t *tagRule)
{
    int ret;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;
    bcmVlan_ruleTable_t *ruleTable;
    UINT32 cmdCount;
    bcmVlan_tableEntry_t *tableEntry;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    if(nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Number of Tags: %d (max %d)",
                      nbrOfTags, BCM_VLAN_MAX_RULE_TABLES);

        ret = -EINVAL;
        goto out_error;
    }

    if(tableDir >= BCM_VLAN_TABLE_DIR_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Direction: %d (max %d)",
                      tableDir, BCM_VLAN_TABLE_DIR_MAX);

        ret = -EINVAL;
        goto out_error;
    }

    BCM_ASSERT(tagRule);

    ruleTable = &realDevCtrl->ruleTable[tableDir][nbrOfTags];

    /* need to initialize tag rule to get a full match */
    ret = tagRulePreProcessor(nbrOfTags, tagRule, ruleTable, &cmdCount);
    if(ret)
    {
        goto out_error;
    }

    tableEntry = removeTableEntryByFilter(ruleTable, &tagRule->filter);
    if(tableEntry == NULL)
    {
        ret = 0;
    }
    else
    {
        if(!tagRule->filter.flags ||
           (tagRule->filter.flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST))
        {
            notifyMcast(realDevCtrl, realDev, nbrOfTags, tableDir, tableEntry);
        }

        if(tableEntry->tagRule.isIptvOnly)
        {
            if(!tableEntry->rxVlanDev)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLAN Device %s does not exist", tableEntry->tagRule.rxVlanDevName);

                ret = -EINVAL;
                goto out_error;
            }

            ret = bcmVlan_unsetIptvOnlyVlanDevice(tableEntry->rxVlanDev, FALSE);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "IPTV only flag unset failed on VLAN Device %s", tableEntry->tagRule.rxVlanDevName);
                goto out_error;
            }
        }

        freeTableEntry(tableEntry);
    }

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}
int bcmVlan_removeAllTagRulesByDev(char *vlanDevName)
{
    int ret = 0;
    struct net_device *realDev;
    struct net_device *vlanDev;
    struct realDeviceControl *realDevCtrl = NULL;
    struct vlanDeviceControl *vlanDevCtrl, *nextVlanDevCtrl;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    vlanDev = dev_get_by_name(&init_net, vlanDevName);
    if(vlanDev == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Vlan Device %s not found", vlanDevName);
        ret = -EINVAL;
        goto out_error; /* Bail out without releasing the device */
    }
    if(!(vlanDev->priv_flags & IFF_BCM_VLAN))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Device %s is not VLAN device", vlanDevName);
        ret = -EINVAL;
        goto out_release_dev;
    }
    /* Get the real device from vlan device */
    realDev = BCM_VLAN_REAL_DEV(vlanDev);
    if(realDev == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Vlan Device %s has no real Device", vlanDevName);
        ret = -EINVAL;
        goto out_release_dev;
    }
    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
    if(realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Vlan Device <%s> can't find real device <%s> control", vlanDevName, realDev->name);
        ret = -EINVAL;
        goto out_release_dev;
    }

    vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);
    /* Set the return to invalid - this will get set if vlan device is found */
    ret = -EINVAL;
    while(vlanDevCtrl)
    {
        nextVlanDevCtrl = BCM_VLAN_LL_GET_NEXT(vlanDevCtrl);

        if(!strncmp(vlanDevCtrl->vlanDev->name,vlanDevName,sizeof(vlanDevCtrl->vlanDev->name)))
        {
            BCM_LOG_INFO(BCM_LOG_ID_VLAN, "Remove ALL rules for VLAN Device %s", vlanDevCtrl->vlanDev->name);
            ret = bcmVlan_removeTagRulesByVlanDev(vlanDevCtrl);
            break;
        }
        vlanDevCtrl = nextVlanDevCtrl;
    }
out_release_dev:
    dev_put(vlanDev);
out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL, NOTIFIER
 */
int bcmVlan_removeTagRulesByVlanDev(struct vlanDeviceControl *vlanDevCtrl)
{
    int ret = 0;
    int i, j;

    for(i=0; i<BCM_VLAN_TABLE_DIR_MAX; ++i)
    {
        for(j=0; j<BCM_VLAN_MAX_RULE_TABLES; ++j)
        {
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "Direction %s, Tags %u",
                          (i==BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX", j);

            removeTableEntries(&vlanDevCtrl->realDevCtrl->ruleTable[i][j],
                               vlanDevCtrl->vlanDev);
        }
    }

    if(vlanDevCtrl->vlanDev)
    {
        ret = bcmVlan_unsetIptvOnlyVlanDevice(vlanDevCtrl->vlanDev, TRUE);
    }

    return ret;
}


int bcmVlan_dumpTagRulesByTable(struct realDeviceControl *realDevCtrl, UINT8 nbrOfTags, bcmVlan_ruleTableDirection_t tableDir)
{
    bcmVlan_tableEntry_t *tableEntry = NULL;
    bcmVlan_ruleTable_t *ruleTable = NULL;

    if ((nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES) || (tableDir >= BCM_VLAN_TABLE_DIR_MAX))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "SHOULD NOT GET HERE: tags %d (max %d), dir%d (max %d)",
        nbrOfTags, BCM_VLAN_MAX_RULE_TABLES, tableDir, BCM_VLAN_TABLE_DIR_MAX);
        return -EINVAL;
    }

    if (realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "SHOULD NOT GET HERE: real device does not exist");
        return -ENOENT;
    }

    ruleTable = &realDevCtrl->ruleTable[tableDir][nbrOfTags];
    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

    printk("\nVLAN Rule Table : %s, %s, nbrOfTags %u, default %s %s\n",
           realDevCtrl->realDev->name,
           (tableDir == BCM_VLAN_TABLE_DIR_RX) ? "Rx" : "Tx",
           nbrOfTags,
           (ruleTable->defaultAction == BCM_VLAN_ACTION_ACCEPT) ? "ACCEPT" : "DROP",
           (tableDir == BCM_VLAN_TABLE_DIR_RX &&
            ruleTable->defaultAction == BCM_VLAN_ACTION_ACCEPT) ?
           ruleTable->defaultRxVlanDev->name : " ");

    if (tableEntry == NULL)
    {
        printk("No entries found\n");
    }

    while (tableEntry)
    {
        dumpTableEntry(realDevCtrl->realDev->name, realDevCtrl->mode, nbrOfTags, tableDir, tableEntry);
        tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
    };

    printk("\n--------------------------------------------------------------------------------\n");

    return 0;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_dumpTagRules(char *realDevName, unsigned int nbrOfTags, bcmVlan_ruleTableDirection_t tableDir)
{
    int ret = 0;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;
    UINT8 tagLoop;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if (realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);
        ret = -EINVAL;
        goto out_error;
    }

    if (BCM_VLAN_IS_DONT_CARE(nbrOfTags) && BCM_VLAN_IS_DONT_CARE(tableDir))
    {
        for (tagLoop = 0; tagLoop < BCM_VLAN_MAX_RULE_TABLES;  tagLoop ++)
        {            
            bcmVlan_dumpTagRulesByTable(realDevCtrl, tagLoop, BCM_VLAN_TABLE_DIR_RX);
            bcmVlan_dumpTagRulesByTable(realDevCtrl, tagLoop, BCM_VLAN_TABLE_DIR_TX);
        }
        goto out_error;
    }

    if (nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Number of Tags: %d (max %d)",
                      nbrOfTags, BCM_VLAN_MAX_RULE_TABLES);
        ret = -EINVAL;
        goto out_error;
    }

    if (tableDir >= BCM_VLAN_TABLE_DIR_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Direction: %d (max %d)",
                      tableDir, BCM_VLAN_TABLE_DIR_MAX);
        ret = -EINVAL;
        goto out_error;
    }

    bcmVlan_dumpTagRulesByTable(realDevCtrl, nbrOfTags, tableDir);

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_getNbrOfTagRulesByTable(char *realDevName, UINT8 nbrOfTags,
                                    bcmVlan_ruleTableDirection_t tableDir, unsigned int *nbrOfRules)
{
    int ret = 0;
    unsigned int numberOfTagRules = 0;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;
    bcmVlan_tableEntry_t *tableEntry;
    bcmVlan_ruleTable_t *ruleTable;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    if(nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Number of Tags: %d (max %d)",
                      nbrOfTags, BCM_VLAN_MAX_RULE_TABLES);

        ret = -EINVAL;
        goto out_error;
    }

    if(tableDir >= BCM_VLAN_TABLE_DIR_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Direction: %d (max %d)",
                      tableDir, BCM_VLAN_TABLE_DIR_MAX);

        ret = -EINVAL;
        goto out_error;
    }

    ruleTable = &realDevCtrl->ruleTable[tableDir][nbrOfTags];

    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

    while(tableEntry)
    {
        numberOfTagRules++;

        tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "\nNumber of tag rules %d", *nbrOfRules);

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    *nbrOfRules = numberOfTagRules;

    return ret;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_setDefaultVlanTag(char *realDevName, UINT8 nbrOfTags, bcmVlan_ruleTableDirection_t tableDir,
                              UINT16 defaultTpid, UINT8 defaultPbits, UINT8 defaultCfi, UINT16 defaultVid)
{
    int ret = 0;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;
    bcmVlan_ruleTable_t *ruleTable;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    if(nbrOfTags >= BCM_VLAN_MAX_RULE_TABLES)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Number of Tags: %d (max %d)",
                      nbrOfTags, BCM_VLAN_MAX_RULE_TABLES);

        ret = -EINVAL;
        goto out_error;
    }

    if(tableDir >= BCM_VLAN_TABLE_DIR_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid Direction: %d (max %d)",
                      tableDir, BCM_VLAN_TABLE_DIR_MAX);

        ret = -EINVAL;
        goto out_error;
    }

    if(!BCM_VLAN_PBITS_FILT_IS_VALID(defaultPbits) ||
       !BCM_VLAN_CFI_FILT_IS_VALID(defaultCfi) ||
       !BCM_VLAN_VID_FILT_IS_VALID(defaultVid))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid default VLAN TCI: defaultPbits %d, defaultCfi %d, defaultVid %d",
                      defaultPbits, defaultCfi, defaultVid);
        goto out_error;
    }

    ruleTable = &realDevCtrl->ruleTable[tableDir][nbrOfTags];

    if(!BCM_VLAN_IS_DONT_CARE(defaultTpid))
    {
        ruleTable->defaultTpid = defaultTpid;
    }

    if(!BCM_VLAN_IS_DONT_CARE(defaultPbits))
    {
        ruleTable->defaultPbits = defaultPbits;
    }

    if(!BCM_VLAN_IS_DONT_CARE(defaultCfi))
    {
        ruleTable->defaultCfi = defaultCfi;
    }

    if(!BCM_VLAN_IS_DONT_CARE(defaultVid))
    {
        ruleTable->defaultVid = defaultVid;
    }

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_setDscpToPbitsTable(char *realDevName, UINT8 dscp, UINT8 pbits)
{
    int ret = 0;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    if(!BCM_VLAN_DSCP_IS_VALID(dscp) || !BCM_VLAN_PBITS_IS_VALID(pbits))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid arguments: DSCP %d, PBITS %d", dscp, pbits);

        ret = -EINVAL;
        goto out_error;
    }

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    realDevCtrl->dscpToPbits[dscp] = pbits;

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

/*
 * Entry points: IOCTL
 */
int bcmVlan_dumpDscpToPbitsTable(char *realDevName, UINT8 dscp)
{
    int ret = 0;
    int i;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl = NULL;

    /******** CRITICAL REGION BEGIN ********/
    BCM_VLAN_GLOBAL_LOCK();

    if(!BCM_VLAN_IS_DONT_CARE(dscp) && !BCM_VLAN_DSCP_IS_VALID(dscp))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Invalid DSCP value: %d", dscp);

        ret = -EINVAL;
        goto out_error;
    }

    realDev = bcmVlan_getRealDeviceByName(realDevName, &realDevCtrl);
    if(realDev == NULL || realDevCtrl == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Real Device %s has no VLAN Interfaces", realDevName);

        ret = -EINVAL;
        goto out_error;
    }

    printk("%s: DSCP to PBITS Table\n", realDev->name);

    if(BCM_VLAN_IS_DONT_CARE(dscp))
    {
        for(i=0; i<BCM_VLAN_MAX_DSCP_VALUES; ++i)
        {
            printk("%d : %d\n", i, realDevCtrl->dscpToPbits[i]);
        }
    }
    else
    {
        printk("%d : %d\n", dscp, realDevCtrl->dscpToPbits[dscp]);
    }

out_error:
    BCM_VLAN_GLOBAL_UNLOCK();
    /******** CRITICAL REGION END ********/

    return ret;
}

static inline int isDevRxVlanDevInStack(struct net_device *dev, struct net_device *realDev)
{
    if(dev != realDev &&
       dev->priv_flags & IFF_BCM_VLAN)
    {
        struct realDeviceControl *realDevCtrl;

        realDevCtrl = bcmVlan_getRealDevCtrl(dev);
        if(realDevCtrl)
        {
            return 1;
        }
    }

    return 0;
}

#if defined(BCM_VLAN_ENABLE_SKB_VLAN)
static struct sk_buff *saveSkbVlanOnReceive(bcmVlan_cmdInfo_t *cmdInfo,
                                            struct sk_buff *skb, bcmVlan_ethHeader_t *ethHeader,
                                            unsigned int nbrOfTags, UINT16 protocol);
#endif

static void forwardMulticast(bcmVlan_cmdInfo_t *cmdInfo, struct realDeviceControl *realDevCtrl,
                             struct sk_buff *skb, int broadcast)
{
    struct vlanDeviceControl *vlanDevCtrl;

    vlanDevCtrl = BCM_VLAN_LL_GET_HEAD(realDevCtrl->vlanDevCtrlLL);

    while(vlanDevCtrl)
    {
        if(vlanDevCtrl->flags.multicast)
        {
                struct sk_buff *skb2;
                struct rtnl_link_stats64 *stats;

                skb2 = skb_copy(skb, GFP_ATOMIC);
                BCM_ASSERT(skb2 != NULL);
#if defined(CONFIG_BLOG)
                blog_clone(skb, blog_ptr(skb2));
#endif
                skb2->dev = vlanDevCtrl->vlanDev;
                skb2->pkt_type = broadcast? PACKET_BROADCAST : PACKET_MULTICAST;
                
                /* Gather basic RX statistics */
                stats = bcmVlan_devGetStats(skb2->dev);
                stats->rx_packets++;
                stats->rx_bytes += skb2->len;

                
                /* Gather extended statistics */
                if(skb2->pkt_type == PACKET_MULTICAST)
                {
                    /* Record multicast statistics */
                    stats->multicast++;
                    stats->rx_multicast_bytes += skb2->len;
                }
                else
                {
                    /* Record broadcast statistics */
                    stats->rx_broadcast_packets++;
                }

#ifdef CONFIG_BLOG
                blog_lock();
                blog_link( IF_DEVICE, blog_ptr(skb2), (void*)skb2->dev, DIR_RX, skb2->len );
                blog_unlock();
#endif
#if defined(BCM_VLAN_ENABLE_SKB_VLAN)
                if(!isDevRxVlanDevInStack(skb2->dev, realDevCtrl->realDev))
                {
                    /* we only save the vlan tags in the SKB if the skb->dev is NOT an
                       inner interface of a VLAN interface stack */

                    skb2 = saveSkbVlanOnReceive(cmdInfo, skb2, BCM_VLAN_SKB_ETH_HEADER(skb2),
                                                cmdInfo->nbrOfTags, cmdInfo->protocol);
                }
                if(skb2 != NULL)
                {
                   skb2->bcm_flags.restore_rx_vlan = 1;
#else
                {
#endif
                    netif_rx(skb2);

                    BCM_VLAN_DP_DEBUG("Multicast: Forward to %s", skb2->dev->name);
                }
            }

        vlanDevCtrl = BCM_VLAN_LL_GET_NEXT(vlanDevCtrl);
    }
    /* 
        Copies of the original packet were forwarded to all interfaces. 
        Free the original packet.
    */  
    kfree_skb(skb);
}

#if 0
#define BCM_VLAN_MATCH_DEBUG(_fmt, _arg...) BCM_VLAN_DP_DEBUG(_fmt, ##_arg)
#else
#define BCM_VLAN_MATCH_DEBUG(_fmt, _arg...)
#endif

static int tagRuleMatch(bcmVlan_cmdInfo_t *cmdInfo,
                        bcmVlan_tableEntry_t *tableEntry,
                        struct net_device *vlanDev,
                        int rxMulticast)
{

    BCM_VLAN_MATCH_DEBUG("@@@ Tag Rule ID %d @@@", tableEntry->tagRule.id);

    /* match on Tx VLAN Device */
    if(tableEntry->txVlanDev == NULL || tableEntry->txVlanDev == vlanDev)
    {
        bcmVlan_tagRuleFilter_t *tagRuleFilter = &tableEntry->tagRule.filter;
        bcmVlan_ethHeader_t *ethHeader = BCM_VLAN_SKB_ETH_HEADER(cmdInfo->skb);

        /* match on tx tag rule filter rxRealDev.
         * Note that rule filter rxRealDevName is defined for tx tag rule only.
         * it should be set to NULL in rx tag rule entry.
         */                                                                                                      
        if(tagRuleFilter->rxRealDevName[0] != '\0' &&
           ((strcmp(tagRuleFilter->rxRealDevName,"lo") && tableEntry->rxRealDev != cmdInfo->skb->rxdev) ||
            (!strcmp(tagRuleFilter->rxRealDevName,"lo") && cmdInfo->skb->rxdev)))
        {
            /* the rx device of this packet does not match the rx real device
             * specified for this tx tag rule.
             */
            BCM_VLAN_MATCH_DEBUG("Pkt Rx device does not match Rx real device. skb->rxdev 0x%p tagRuleFilter->rxRealDevName %s\n", 
            cmdInfo->skb->rxdev, tagRuleFilter->rxRealDevName);
            return 0; /* no match */
        }
        
        /* match on rx vlan interface */
        if(tableEntry->rxVlanDev != NULL &&
           tagRuleFilter->vlanDevMacAddr != BCM_VLAN_MATCH_NO_VLANDEV_MACADDR)
        {
            if(rxMulticast)
            {
                if(tagRuleFilter->vlanDevMacAddr == BCM_VLAN_MATCH_VLANDEV_MACADDR)
                {
                    /* the filter does not allow for multicast frame. No match. */
                    BCM_VLAN_MATCH_DEBUG("Filter does not allow Multicast MAC Address %d", tagRuleFilter->vlanDevMacAddr);
                    return 0;
                }
            }
            else
            {
                /* match the destination mac with the rx interface mac address */
                if(memcmp(tableEntry->rxVlanDev->dev_addr, ethHeader->macDest, ETH_ALEN))
                {
                    /* mac addresses don't match */
                    BCM_VLAN_MATCH_DEBUG("Mac mismatch rx_dev_mac %x %x %x %x %x %x ethheaderMac %x %x %x %x %x %x",
                                         tableEntry->rxVlanDev->dev_addr[0],
                                         tableEntry->rxVlanDev->dev_addr[1],
                                         tableEntry->rxVlanDev->dev_addr[2],
                                         tableEntry->rxVlanDev->dev_addr[3],
                                         tableEntry->rxVlanDev->dev_addr[4],
                                         tableEntry->rxVlanDev->dev_addr[5],
                                         ethHeader->macDest[0],
                                         ethHeader->macDest[1],
                                         ethHeader->macDest[2],
                                         ethHeader->macDest[3],
                                         ethHeader->macDest[4],
                                         ethHeader->macDest[5]);
                    return 0;
                }
            }

            BCM_VLAN_MATCH_DEBUG("MATCH: VLAN DEV MAC <%02x:%02x:%02x:%02x:%02x:%02x>",
                                 tableEntry->rxVlanDev->dev_addr[0],
                                 tableEntry->rxVlanDev->dev_addr[1],
                                 tableEntry->rxVlanDev->dev_addr[2],
                                 tableEntry->rxVlanDev->dev_addr[3],
                                 tableEntry->rxVlanDev->dev_addr[4],
                                 tableEntry->rxVlanDev->dev_addr[5]);
        }

        /* the filters specified in flags are OR'd together */
        if(tagRuleFilter->flags)
        {
            if(tagRuleFilter->flags & BCM_VLAN_FILTER_FLAGS_IS_UNICAST &&
               !is_multicast_ether_addr(ethHeader->macDest))
            {
                BCM_VLAN_MATCH_DEBUG("MATCH: IS_UNICAST");
            }
            /* match on Multicast packets */
            else if(tagRuleFilter->flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST &&
                    is_multicast_ether_addr(ethHeader->macDest) &&
                    !is_broadcast_ether_addr(ethHeader->macDest))
            {
                BCM_VLAN_MATCH_DEBUG("MATCH: IS_MULTICAST");
            }
            /* match on Broadcast packets */
            else if(tagRuleFilter->flags & BCM_VLAN_FILTER_FLAGS_IS_BROADCAST &&
                    is_broadcast_ether_addr(ethHeader->macDest))
            {
                BCM_VLAN_MATCH_DEBUG("MATCH: IS_BROADCAST");
            }
            else
            {
                return 0;
            }
        }

        /* match on SKB Priority */
        if(BCM_VLAN_FILTER_MATCH(tagRuleFilter->skbPrio, cmdInfo->skb->priority))
        {
            BCM_VLAN_MATCH_DEBUG("MATCH: SKB PRIORITY <%d>", tagRuleFilter->skbPrio);

            /* match on SKB Mark FlowId */
            if(BCM_VLAN_FILTER_MATCH(tagRuleFilter->skbMarkFlowId, SKBMARK_GET_FLOW_ID(cmdInfo->skb->mark)))
            {
                BCM_VLAN_MATCH_DEBUG("MATCH: FLOWID <%d>", tagRuleFilter->skbMarkFlowId);

                /* match on SKB Mark Port */
                if(BCM_VLAN_FILTER_MATCH(tagRuleFilter->skbMarkPort, SKBMARK_GET_PORT(cmdInfo->skb->mark)))
                {
                    BCM_VLAN_MATCH_DEBUG("MATCH: PORT <%d>", tagRuleFilter->skbMarkPort);

                    /* match on Ethertype */
                    if(BCM_VLAN_FILTER_MATCH(tagRuleFilter->etherType, ntohs(cmdInfo->ethHeader->etherType)))
                    {
                        BCM_VLAN_MATCH_DEBUG("MATCH: ETHERTYPE <0x%04X>", tagRuleFilter->etherType);

                        /* match on DSCP */
                        if(BCM_VLAN_IS_DONT_CARE(tagRuleFilter->dscp) ||
                           (cmdInfo->ipHeader && tagRuleFilter->dscp == BCM_VLAN_GET_IP_DSCP(cmdInfo->ipHeader)))
                        {
                            BCM_VLAN_MATCH_DEBUG("MATCH: DSCP <%d>", tagRuleFilter->dscp);

                            /* match on DSCP2PBITS */
                            if(BCM_VLAN_IS_DONT_CARE(tagRuleFilter->dscp2pbits) ||
                               (cmdInfo->ipHeader && tagRuleFilter->dscp2pbits ==
                                cmdInfo->dscpToPbits[BCM_VLAN_GET_IP_DSCP(cmdInfo->ipHeader)]))
                            {
                                /* match on IPPROTO */
                                if(BCM_VLAN_IS_DONT_CARE(tagRuleFilter->ipProto) ||
                                   (cmdInfo->ipHeader &&
                                   (BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader) == 4) &&
                                   tagRuleFilter->ipProto == BCM_VLAN_GET_IP_PROTO((bcmVlan_ipHeader_t *)cmdInfo->ipHeader)) ||
                                   (cmdInfo->ipHeader &&
                                   (BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader) == 6) &&
                                   tagRuleFilter->ipProto == BCM_VLAN_GET_IPV6_PROTO((bcmVlan_ipv6Header_t *)cmdInfo->ipHeader)))
                                {
                                    /* match on VLAN tags */
                                    int i;
                                    bcmVlan_vlanHeader_t *vlanHeader = &cmdInfo->ethHeader->vlanHeader;

                                    BCM_VLAN_MATCH_DEBUG("MATCH: IPPROTO <%d>, VER %d",
                                      tagRuleFilter->ipProto, BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader));
    
                                    for(i=0; i<cmdInfo->nbrOfTags; ++i)
                                    {
                                        if(!BCM_VLAN_TAG_MATCH(&tagRuleFilter->vlanTag[i], vlanHeader))
                                        {
                                            return 0;
                                        }
    
                                        BCM_VLAN_MATCH_DEBUG("MATCH: VLAN Tag %d", i);
    
                                        vlanHeader++;
                                    }
    
                                    /* if we get here, there was a match */
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

#if defined(BCM_VLAN_ENABLE_SKB_VLAN)
#ifndef CONFIG_BCM_PON
static struct sk_buff *keepSkbVlanOnReceive(bcmVlan_cmdInfo_t *cmdInfo,
                                            struct sk_buff *skb, bcmVlan_ethHeader_t *ethHeader,
                                            unsigned int nbrOfTags, UINT16 protocol)
{
    bcmVlan_vlanHeader_t *vlanHeader = &ethHeader->vlanHeader;
    int i;

    if(nbrOfTags > SKB_VLAN_MAX_TAGS)
    {
#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Too many VLAN Tags : %u (max %u)",
                      nbrOfTags, SKB_VLAN_MAX_TAGS);
#endif
        nbrOfTags = SKB_VLAN_MAX_TAGS;
    }

    skb->vlan_count = nbrOfTags;

    if(nbrOfTags == 0)
    {
        /* no VLAN tags to save */
        goto out;
    }


    skb = unshareSkb(cmdInfo, skb);
    if(skb == NULL)
    {
        BCM_VLAN_DP_ERROR("Failed to unshare skb");
        goto out;
    }


    /* save VLAN Tags in the SKB */
    /* keep skb->vlanHeader host order, eg. VLAN 100, IPV4: 0x00640800*/
    skb->vlan_tpid = ntohs(ethHeader->etherType);
    for(i=0; i<nbrOfTags; ++i)
    {
        skb->vlan_header[i] = ntohl(*((UINT32*)vlanHeader));
        BCM_VLAN_DP_DEBUG("%d: skb->VlanHeader[%d]: 0x%x\n\r", __LINE__,i,skb->vlan_header[i]);
        vlanHeader++;
    }

out:
    return skb;
}
#endif

static struct sk_buff *saveSkbVlanOnReceive(bcmVlan_cmdInfo_t *cmdInfo,
                                            struct sk_buff *skb, bcmVlan_ethHeader_t *ethHeader,
                                            unsigned int nbrOfTags, UINT16 protocol)
{
    bcmVlan_vlanHeader_t *vlanHeader = &ethHeader->vlanHeader;
    int vlanHeaderLen;
    int i;

    if(nbrOfTags > SKB_VLAN_MAX_TAGS)
    {
#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Too many VLAN Tags : %u (max %u)",
                      nbrOfTags, SKB_VLAN_MAX_TAGS);
#endif
        nbrOfTags = SKB_VLAN_MAX_TAGS;
    }

    skb->vlan_count = nbrOfTags;

    if(nbrOfTags == 0)
    {
        /* no VLAN tags to save */
        goto out;
    }

    skb = unshareSkb(cmdInfo, skb);
    if(skb == NULL)
    {
        BCM_VLAN_DP_ERROR("Failed to unshare skb");

        goto out;
    }

    /* save VLAN Tags in the SKB */
    /* keep skb->vlanHeader host order, eg. VLAN 100, IPV4: 0x00640800*/
    skb->vlan_tpid = ntohs(ethHeader->etherType);
    for(i=0; i<nbrOfTags; ++i)
    {
        skb->vlan_header[i] = ntohl(*((UINT32*)vlanHeader));
        BCM_VLAN_DP_DEBUG("%d: skb->VlanHeader[%d]: 0x%x\n\r", __LINE__,i,skb->vlan_header[i]);
        vlanHeader++;
    }

    /* remove VLAN Tags from the packet */
    vlanHeaderLen = nbrOfTags * BCM_VLAN_HEADER_LEN;

    if((skb->len-vlanHeaderLen) < skb->data_len)
    {
       /* normally even with frags we expect the VLAN to be present in
        * skb->data, but with fragmnetation in tunneling the VLAN header
        * can be in frags. To handle such cases linearize the skb
        */ 
        if(skb_linearize(skb))
        {
            BCM_VLAN_DP_ERROR("Failed to linearize skb");
            kfree_skb(skb);
            skb=NULL;
            goto out;
        }
    }

    removeSkbHeader(skb, vlanHeaderLen);

    memmove(skb->data - BCM_ETH_HEADER_LEN,
            skb->data - (BCM_ETH_HEADER_LEN + vlanHeaderLen),
            2 * BCM_ETH_ADDR_LEN);

    skb->mac_header += vlanHeaderLen;
    skb->network_header += vlanHeaderLen;

    if(ntohs(protocol) >= 1536)
    {
        skb->protocol = protocol;
    }
    else
    {
        UINT16 *rawp = (UINT16 *)skb->data;
        if(*rawp == 0xFFFF)
        {
            /*
             * This is a magic hack to spot IPX packets. Older Novell
             * breaks the protocol design and runs IPX over 802.3 without
             * an 802.2 LLC layer. We look for FFFF which isn't a used
             * 802.2 SSAP/DSAP. This won't work for fault tolerant netware
             * but does for the rest.
             */
            skb->protocol = htons(ETH_P_802_3);
        }
        else
        {
            /*
             * Real 802.2 LLC
             */
            skb->protocol = htons(ETH_P_802_2);
        }
    }

out:
    return skb;
}

static struct sk_buff *restoreSkbVlanOnTransmit(bcmVlan_cmdInfo_t *cmdInfo, struct sk_buff *skb)
{
    bcmVlan_ethHeader_t *ethHeader;
    bcmVlan_vlanHeader_t *vlanHeader;
    UINT32 *skbVlanHeader;
    int vlanHeaderLen;
    unsigned int nbrOfTags = skb->vlan_count;
    int i;

    if(nbrOfTags > SKB_VLAN_MAX_TAGS)
    {
#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Too many VLAN Tags : %u (max %u)",
                      nbrOfTags, SKB_VLAN_MAX_TAGS);
#endif
        nbrOfTags = SKB_VLAN_MAX_TAGS;
    }

    if(nbrOfTags == 0)
    {
        /* no VLAN tags to restore */
        goto out;
    }

    /* insert VLAN Tags from SKB to the packet */
    vlanHeaderLen = nbrOfTags * BCM_VLAN_HEADER_LEN;

    skb = checkSkbHeadroom(cmdInfo, skb, vlanHeaderLen);
    if(skb == NULL)
    {
        BCM_VLAN_DP_ERROR("Failed to allocate skb headroom");
        goto out;
    }

    /* save ethertype */
    ethHeader = (bcmVlan_ethHeader_t *)(skb->data);

    /* push VLAN headers */
    ethHeader = (bcmVlan_ethHeader_t *)skb_push(skb, vlanHeaderLen);

    /* Move the mac addresses to the beginning of the new header. */
    memmove(skb->data, skb->data + vlanHeaderLen, 2 * BCM_ETH_ADDR_LEN);

    /* set ethertype */
    ethHeader->etherType = htons(skb->vlan_tpid);

    /* copy tags from SKB */
    vlanHeader = &ethHeader->vlanHeader;

    skbVlanHeader = (UINT32*)(vlanHeader);
    for(i=0; i<nbrOfTags; ++i)
    {
        *skbVlanHeader = htonl(skb->vlan_header[i]);
        BCM_VLAN_DP_DEBUG("%d: skbVlanHeader[%d] : 0x%x\n\r", __LINE__,i,*skbVlanHeader);
        skbVlanHeader++;
    }

    skb->mac_header -= vlanHeaderLen;
    skb->network_header -= vlanHeaderLen;

    skb->vlan_count = 0;

out:
    return skb;
}

static struct sk_buff *restoreSkbVlanOnReceive(bcmVlan_cmdInfo_t *cmdInfo, struct sk_buff *skb)
{
    skb_push(skb, BCM_ETH_HEADER_LEN);

    skb = restoreSkbVlanOnTransmit(cmdInfo, skb);

    if(skb)
    {
        __skb_pull(skb, BCM_ETH_HEADER_LEN);

        bcmVlan_dumpPacket(cmdInfo->tpidTable, skb);
    }

    return skb;
}
#endif /* BCM_VLAN_ENABLE_SKB_VLAN */

static int modifyRxMulticastRG(bcmVlan_cmdInfo_t *cmdInfo,
                               bcmVlan_tableEntry_t *tableEntry)
{
    struct net_device *rxdev = cmdInfo->skb->rxdev;
    bcmVlan_tagRuleCommand_t *cmd;
    int ret = 0;
    int i;

    for(i=0; i<tableEntry->cmdCount && !ret; ++i)
    {
        cmd = &tableEntry->tagRule.cmd[i];

        BCM_VLAN_DP_DEBUG("[%s, 0x%08X, 0x%08X]",
                          cmdHandler[cmd->opCode].name, cmd->arg[0], cmd->arg[1]);

// fix me later.  For now, OPCODE_CONTINUE is not supported in RG mode.
//                if(cmd->opCode == BCM_VLAN_OPCODE_CONTINUE)
//                {
//                    goto next_table_entry;
//                }

        ret = cmdHandler[cmd->opCode].func(cmdInfo, cmd->arg);
        if(ret == -EFAULT)
        {
            BCM_VLAN_DP_DEBUG("Command returned -EFAULT");
            break;
        }

        /* cmdInfo->skb may change after cmdHandler.func is called.
         * restore the rx real dev.
         */
        cmdInfo->skb->rxdev = rxdev;
    }

    return ret;
}

/*
 * Entry points: RECV, XMIT
 */
static int bcmVlan_processRxMulticastRG(bcmVlan_cmdInfo_t *cmdInfo,
                                        struct net_device *vlanDev,
                                        struct sk_buff **skbp,
                                        struct realDeviceControl *realDevCtrl,
                                        int *rxVlanDevInStackp)
{
    bcmVlan_ruleTable_t *ruleTable = NULL;
    bcmVlan_tableEntry_t *tableEntry;
    bcmVlan_tableEntry_t *firstHitTableEntry = NULL;
    bcmVlan_cmdInfo_t cmdInfoSave;
    int ret = 0;

    /* save the original cmdInfo */
    cmdInfoSave = *cmdInfo;

    *rxVlanDevInStackp = 0;

    /* get rule table */
    ruleTable = &realDevCtrl->ruleTable[BCM_VLAN_TABLE_DIR_RX][cmdInfo->nbrOfTags];

    /* try to find a matching rule, if any */
    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);
    while(tableEntry)
    {
        /* If this rule table entry is the last table entry, which in RG implementation
         * is reserved for nonVlan bridge interface rule, and 
         * this is a vlan tagged frame and
         * there has been a hit,
         * then, we want to skip matching with this rule to avoid forwarding the frame
         * to the nonVlan bridge interface.
         */
        if(tableEntry == ruleTable->lastTableEntry &&
           cmdInfoSave.nbrOfTags > 0 &&
           firstHitTableEntry != NULL)
        {
            goto next_table_entry;
        }

         /* if vlan dev is not IFF_UP, don't forward frame */
        if (tableEntry->rxVlanDev && !(tableEntry->rxVlanDev->flags & IFF_UP))
        {
            goto next_table_entry;
        }

        /* match on the original skb */
        if(tagRuleMatch(&cmdInfoSave, tableEntry, vlanDev, 1))
        {
            tableEntry->hitCount++;

            BCM_VLAN_DP_DEBUG("*** HIT *** RX, %s, Tags %d, RuleId %d",
                              cmdInfo->skb->dev->name,
                              cmdInfo->nbrOfTags,
                              (int)tableEntry->tagRule.id);

            /* If this is the first hit, save the skb and let it be forwarded by the
             * caller of this function.
             */
            if(firstHitTableEntry == NULL)
            {
                /* This is the first hit. Save the table entry for later processing */
                firstHitTableEntry = tableEntry;
            }
            else
            {
                struct rtnl_link_stats64 *stats;

                /* Make a copy of the original SKB, process it, and forward it
                   to the backlog device */

                *cmdInfo = cmdInfoSave;

                *rxVlanDevInStackp = 1;

                /* make a copy of the original skb for command execution. */
                cmdInfo->skb = skb_copy(cmdInfoSave.skb, GFP_ATOMIC);
                BCM_ASSERT(cmdInfo->skb != NULL);
                cmdInfo->skb->rxdev = cmdInfoSave.skb->rxdev;
            
#if defined(CONFIG_BLOG)
                blog_clone(cmdInfoSave.skb, blog_ptr(cmdInfo->skb));
#endif
                parseFrameHeader(cmdInfo);

                if(tableEntry->rxVlanDev != NULL)
                {
                    /* Forward the frame to the specified rx vlan interface. */
                    cmdInfo->skb->dev = tableEntry->rxVlanDev;
                }
                /* else, forward the frame to the real device set by cmdInfo->skb->dev */

                /* execute commands */
                ret = modifyRxMulticastRG(cmdInfo, tableEntry);
                if(ret == -EFAULT)
                {
                    BCM_VLAN_DP_DEBUG("Command returned -EFAULT");
                    goto out;
                }

                /* Gather statistics */
                stats = bcmVlan_devGetStats(cmdInfo->skb->dev);
                stats->rx_packets++;
                stats->rx_bytes += cmdInfo->skb->len;

                stats->multicast++;
                stats->rx_multicast_bytes += cmdInfo->skb->len;

#ifdef CONFIG_BLOG
                blog_lock();
                blog_link( IF_DEVICE, blog_ptr(cmdInfo->skb), (void*)cmdInfo->skb->dev, DIR_RX, cmdInfo->skb->len );
                blog_unlock();
#endif
                /* Forward packet */
                netif_rx(cmdInfo->skb);
              
                BCM_VLAN_DP_DEBUG("Multicast: Forward to %s", cmdInfo->skb->dev->name);
            }
        }

    next_table_entry:
        tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);

    } /* while (tableEntry) */

    if(firstHitTableEntry == NULL)
    {
        /* There was never a match. */

        /* Note: Since there was never a match, cmdInfo->skb must be the original skb.
         *       we don't need to free the original skb.
         */

        BCM_VLAN_DP_DEBUG("*** MISS *** RX, %s, Tags %d",
                          cmdInfo->skb->dev->name,
                          cmdInfo->nbrOfTags);

        /* update local stats on misses */
        cmdInfo->localStats->rx_Misses++;

        /* drop packet, if this is the default action */
        if(ruleTable && ruleTable->defaultAction == BCM_VLAN_ACTION_DROP)
        {
            BCM_VLAN_DP_DEBUG("Default Action : RX, DROP");

            ret = cmdHandler_dropFrame(cmdInfo, NULL);
        }
        else /* ACCEPT */
        {
            BCM_ASSERT(ruleTable->defaultRxVlanDev != NULL);

            BCM_VLAN_DP_DEBUG("Default Action : RX, ACCEPT %s", ruleTable->defaultRxVlanDev->name);

            cmdInfo->skb->dev = ruleTable->defaultRxVlanDev;
            *rxVlanDevInStackp = isDevRxVlanDevInStack(cmdInfo->skb->dev, realDevCtrl->realDev);
            BCM_VLAN_DP_DEBUG("Multicast: Forward to %s", cmdInfo->skb->dev->name);
        }
    }
    else
    {
        /* There was a match, process and forward the original SKB */

        tableEntry = firstHitTableEntry;
        cmdInfo = &cmdInfoSave;

        parseFrameHeader(cmdInfo);

        if(tableEntry->rxVlanDev != NULL)
        {
            /* Forward the frame to the specified rx vlan interface. */
            cmdInfo->skb->dev = tableEntry->rxVlanDev;
        }
        /* else, forward the frame to the real device set by cmdInfo->skb->dev */

        /* execute commands */
        ret = modifyRxMulticastRG(cmdInfo, tableEntry);
        if(ret == -EFAULT)
        {
            BCM_VLAN_DP_DEBUG("Command returned -EFAULT");
            goto out;
        }

        /* return the original skb to the caller */
        *skbp = cmdInfo->skb;
        *rxVlanDevInStackp = isDevRxVlanDevInStack(cmdInfo->skb->dev, realDevCtrl->realDev);
        BCM_VLAN_DP_DEBUG("Multicast: Forward to %s", (*skbp)->dev->name);
    }

out:
    return ret;
}

static int bcmVlan_restoreSkbVlanTags(struct sk_buff **skbp)
{
    /* push the VLAN tag again */
    struct sk_buff *skb = *skbp;
    __be16 vlan_proto[SKB_VLAN_MAX_TAGS]; 
    u16 vlan_tci[SKB_VLAN_MAX_TAGS];
    int ret = 0;
    int vlan_count = 0;
    unsigned int offset;

    /* Retrieve all the VLAN tags that Linux dev.c has stored in skb */
    while(!ret && vlan_count < SKB_VLAN_MAX_TAGS && skb_vlan_tag_present(skb))
    {
        vlan_tci[vlan_count] = skb_vlan_tag_get(skb);
        vlan_tci[vlan_count] |= skb->cfi_save;
        vlan_proto[vlan_count] = skb->vlan_proto;
        /* Pop the existing vlan tag from SKB - possibility of loosing CFI ?*/
        ret = skb_vlan_pop(skb);
        vlan_count++;
    }
    /* Push these tags into packet */
    while (!ret && skb && vlan_count)
    {
        offset = skb->data - skb_mac_header(skb);
        vlan_count--;
        /*
         * vlan_insert_tag expect skb->data pointing to mac header.
         * So change skb->data before calling it and change back to
         * original position later
         */
        skb_push(skb, offset);
        skb = vlan_insert_tag(skb, vlan_proto[vlan_count], vlan_tci[vlan_count]);
        if (!skb)
        {
            break;
        }
        skb_pull(skb, offset);
        skb_reset_network_header(skb);
        skb_reset_mac_len(skb);
    }
    if (unlikely(ret) || unlikely(!skb))
    {
        ret = -EFAULT;
    }
    *skbp = skb;
    return ret;
}
/*
 * Entry points: RECV, XMIT
 */
int bcmVlan_processFrame(struct net_device *realDev, struct net_device *vlanDev,
                         struct sk_buff **skbp, bcmVlan_ruleTableDirection_t tableDir,
                         int *rxVlanDevInStackp)
{
    int ret = 0;
    int i;
    struct realDeviceControl *realDevCtrl;
    bcmVlan_cmdInfo_t *cmdInfo;
    bcmVlan_ruleTable_t *ruleTable = NULL;
    bcmVlan_tableEntry_t *tableEntry;
    bcmVlan_tagRuleCommand_t *cmd;
    bcmVlan_ethHeader_t *ethHeader   = NULL;
    struct net_device *rxVlanDevRule = NULL;
    int match;
    int rxMulticast = 0;
    unsigned int rxNbrOfTags = 0;
    bcmVlan_tableEntry_t *qosTableEntry[BCM_VLAN_FLOW_MAX_QOS_RULES], **qosTableEntry_p = qosTableEntry;

    if(tableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        /* Set command information pointer to the Rx global */
        cmdInfo = &rxCmdInfo;

        /* save the receive real netdev */
        (*skbp)->rxdev = (*skbp)->dev;
        while(!netdev_path_is_root((*skbp)->rxdev))
            (*skbp)->rxdev = netdev_path_next_dev((*skbp)->rxdev);
    }
    else
    {
        /* Set command information pointer to the Tx global */
        cmdInfo = &txCmdInfo;
    }

    realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
    if(realDevCtrl == NULL)
    {
        /* this frame is not for us */
//        BCM_VLAN_DP_DEBUG("%s has no VLAN Interfaces", realDev->name);

        ret = -ENODEV;
        goto out;
    }
    /* Check if VLAN tag was popped and stored in SKB by dev.c */
    if (tableDir == BCM_VLAN_TABLE_DIR_RX && skb_vlan_tag_present(*skbp))
    {
        ret = bcmVlan_restoreSkbVlanTags(skbp);
        if (ret)
        {
            ret = -EFAULT;
            goto out;
        }
    }

    cmdInfo->skb         = *skbp;
    cmdInfo->tpidTable   = realDevCtrl->tpidTable;
    cmdInfo->localStats  = &realDevCtrl->localStats;
    cmdInfo->tableDir    = tableDir;
    cmdInfo->dscpToPbits = realDevCtrl->dscpToPbits;

    BCM_VLAN_DP_DEBUG("************ %s Frame ************",
                      (tableDir == BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX");
    bcmVlan_dumpPacket(cmdInfo->tpidTable, cmdInfo->skb);

#if defined(BCM_VLAN_ENABLE_SKB_VLAN)
    if(realDevCtrl->mode == BCM_VLAN_MODE_ONT && cmdInfo->skb->bcm_flags.restore_rx_vlan)
    {
        if(tableDir == BCM_VLAN_TABLE_DIR_TX)
        {
            cmdInfo->skb = restoreSkbVlanOnTransmit(cmdInfo, cmdInfo->skb);

            if(cmdInfo->skb == NULL)
            {
                ret = -EFAULT;

                goto out;
            }
        }
        else if(realDev->priv_flags & IFF_EBRIDGE)
        {
            cmdInfo->skb = restoreSkbVlanOnReceive(cmdInfo, cmdInfo->skb);

            if(cmdInfo->skb == NULL)
            {
                ret = -EFAULT;

                goto out;
            }
        }
    }
#endif

    parseFrameHeader(cmdInfo);

    if(cmdInfo->nbrOfTags > BCM_VLAN_MAX_TAGS)
    {
        BCM_VLAN_DP_ERROR("Too many VLAN Tags (%d, max %d)", cmdInfo->nbrOfTags, BCM_VLAN_MAX_TAGS);

        /* update local stats on misses */
        if(tableDir == BCM_VLAN_TABLE_DIR_RX)
        {
            cmdInfo->localStats->rx_Misses++;
        }
        else
        {
            cmdInfo->localStats->tx_Misses++;
        }

        goto out;
    }
    
#if defined(BCM_VLAN_ENABLE_SKB_VLAN) && !defined(CONFIG_BCM_PON)
    if(realDevCtrl->mode != BCM_VLAN_MODE_ONT)
    {
        if (!isDevRxVlanDevInStack(cmdInfo->skb->dev, realDev))
        {
            cmdInfo->skb = keepSkbVlanOnReceive(cmdInfo, cmdInfo->skb, cmdInfo->ethHeader,
                                                cmdInfo->nbrOfTags, cmdInfo->protocol);
        }
    }
#endif

#if defined(BCM_VLAN_IP_CHECK)
    if(bcmLog_getLogLevel(BCM_LOG_ID_VLAN) >= BCM_LOG_LEVEL_DEBUG)
    {
        if(cmdInfo->ipHeader && BCM_VLAN_GET_IP_VERSION(cmdInfo->ipHeader) == 4)
        {
            unsigned int ihl = cmdInfo->ipHeader->ihl;

            if (ip_fast_csum((UINT8 *)cmdInfo->ipHeader, ihl) != 0)
            {
                BCM_VLAN_DP_ERROR("Invalid IP checksum: version %d, ihl %d",
                                  cmdInfo->ipHeader->version, ihl);
            }
        }
    }
#endif

#if defined(BCM_VLAN_DATAPATH_ERROR_CHECK)
    BCM_ASSERT(tableDir < BCM_VLAN_TABLE_DIR_MAX);
#endif

    if(tableDir == BCM_VLAN_TABLE_DIR_RX)
    {
       /* restore the receive real netdev */
       cmdInfo->skb->rxdev = (*skbp)->rxdev;
        
       ethHeader = BCM_VLAN_SKB_ETH_HEADER(*skbp);

       if(is_multicast_ether_addr(ethHeader->macDest))
       {
          rxMulticast = 1;

          if(realDevCtrl->mode == BCM_VLAN_MODE_RG)
          {
             /* set packet type */
             (*skbp)->pkt_type = is_broadcast_ether_addr(ethHeader->macDest)?
                                     PACKET_BROADCAST : PACKET_MULTICAST;

             ret = bcmVlan_processRxMulticastRG(cmdInfo, vlanDev, skbp, realDevCtrl,
                                                rxVlanDevInStackp);
             goto out;
          }
       }
    }

    /* save the original number of tags in the frame */
    rxNbrOfTags = cmdInfo->nbrOfTags;

    /* get rule table */
    ruleTable = &realDevCtrl->ruleTable[tableDir][cmdInfo->nbrOfTags];

    /*Record all matching qos rules before flow rule cmd action is taken, 
      as flow rule cmd action might modify filter fields in qos rule*/
    memset(qosTableEntry, 0, sizeof(qosTableEntry));
    for (tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL); tableEntry; tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry))
    {
        if (tableEntry->tagRule.type == BCM_VLAN_RULE_TYPE_QOS && tagRuleMatch(cmdInfo, tableEntry, vlanDev, rxMulticast))
        {
            *qosTableEntry_p = tableEntry;
            qosTableEntry_p ++;
            tableEntry->hitCount++;

            /* check continue commands */
            for(cmd=NULL, i=0; i<tableEntry->cmdCount; ++i)
            {
                cmd = &tableEntry->tagRule.cmd[i];

                BCM_VLAN_DP_DEBUG("[%s, 0x%08X, 0x%08X]",
                                  cmdHandler[cmd->opCode].name, cmd->arg[0], cmd->arg[1]);

                if(cmd->opCode == BCM_VLAN_OPCODE_CONTINUE)
                    break;
            }

            if (cmd && cmd->opCode == BCM_VLAN_OPCODE_CONTINUE) 
                continue;
			
            break;
        }
    }  
    
    /* try to find a matching rule, if any */
    match = 0;
    tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);
    while(tableEntry)
    {
        if(tableEntry->tagRule.type == BCM_VLAN_RULE_TYPE_FLOW && tagRuleMatch(cmdInfo, tableEntry, vlanDev, rxMulticast))
        {
            BCM_VLAN_DP_DEBUG("*** HIT *** %s, %s, Tags %d, RuleId %d",
                              (tableDir == BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX",
                              cmdInfo->skb->dev->name,
                              cmdInfo->nbrOfTags,
                              (int)tableEntry->tagRule.id);

            /* save target VLAN device to the VLAN device specified in the rule */
            rxVlanDevRule = tableEntry->rxVlanDev;

            /* if vlan dev is not IFF_UP, ignore match */
            if (rxVlanDevRule && !(rxVlanDevRule->flags & IFF_UP))
                goto next_table_entry;

            match = 1;
            tableEntry->hitCount++;

            /* execute commands */
            for(i=0; i<tableEntry->cmdCount && !ret; ++i)
            {
                cmd = &tableEntry->tagRule.cmd[i];

                BCM_VLAN_DP_DEBUG("[%s, 0x%08X, 0x%08X]",
                                  cmdHandler[cmd->opCode].name, cmd->arg[0], cmd->arg[1]);

                if(cmd->opCode == BCM_VLAN_OPCODE_CONTINUE)
                {
                    goto next_table_entry;
                }

                ret = cmdHandler[cmd->opCode].func(cmdInfo, cmd->arg);
                if(ret == -EFAULT)
                {
                    BCM_VLAN_DP_DEBUG("Command returned -EFAULT");
                    goto out;
                }
                /* cmdInfo->skb may change after cmdHandler.func is called.
                 * restore the rx real dev.
                 */
                cmdInfo->skb->rxdev = (*skbp)->rxdev;
            }

            break;
        }

    next_table_entry:
        tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
    }

   /* Apply previsous record qos rule cmd action
    * Valid action could be SET_PBITS/SET_DSCP/SET_QUEUE/... 
    * Qos rule doesn't affect accept or drop a pkt
   */
    for (qosTableEntry_p = qosTableEntry, tableEntry =*qosTableEntry_p;  tableEntry; qosTableEntry_p++, tableEntry =*qosTableEntry_p)
    {
         /* execute commands */
         for(cmd=NULL, i=0; i<tableEntry->cmdCount; ++i)
         {
             cmd = &tableEntry->tagRule.cmd[i];
 
             BCM_VLAN_DP_DEBUG("[%s, 0x%08X, 0x%08X]",
                               cmdHandler[cmd->opCode].name, cmd->arg[0], cmd->arg[1]);
 
             if(cmd->opCode == BCM_VLAN_OPCODE_CONTINUE)
                 break;
 
             if(cmd->opCode != BCM_VLAN_OPCODE_DROP_FRAME)
                 cmdHandler[cmd->opCode].func(cmdInfo, cmd->arg);
         }
 
         if (cmd && cmd->opCode == BCM_VLAN_OPCODE_CONTINUE) 
             continue;
 		
         break;
    }
	
    if(!match)
    {
        BCM_VLAN_DP_DEBUG("*** MISS *** %s, %s, Tags %d",
                          (tableDir == BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX",
                          cmdInfo->skb->dev->name,
                          cmdInfo->nbrOfTags);

        /* update local stats on misses */
        if(tableDir == BCM_VLAN_TABLE_DIR_RX)
        {
            cmdInfo->localStats->rx_Misses++;
        }
        else
        {
            cmdInfo->localStats->tx_Misses++;
        }

        if(ruleTable->defaultAction == BCM_VLAN_ACTION_DROP)
        {
            BCM_VLAN_DP_DEBUG("Default Action : %s DROP", (tableDir == BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX");

            ret = cmdHandler_dropFrame(cmdInfo, NULL);
        }
        else /* ACCEPT */
        {
            if(tableDir == BCM_VLAN_TABLE_DIR_RX)
            {
                BCM_ASSERT(ruleTable->defaultRxVlanDev != NULL);

                BCM_VLAN_DP_DEBUG("Default Action : RX, ACCEPT %s", ruleTable->defaultRxVlanDev->name);

                cmdInfo->skb->dev = ruleTable->defaultRxVlanDev;

                *rxVlanDevInStackp = isDevRxVlanDevInStack(cmdInfo->skb->dev, realDev);

#if defined(BCM_VLAN_ENABLE_SKB_VLAN)
                if(realDevCtrl->mode == BCM_VLAN_MODE_ONT)
                {
                    if(!(*rxVlanDevInStackp))
                    {
                        /* we only save the vlan tags in the SKB if the skb->dev is NOT an
                           inner interface of a VLAN interface stack */

                        cmdInfo->skb = saveSkbVlanOnReceive(cmdInfo, cmdInfo->skb, cmdInfo->ethHeader,
                                                            cmdInfo->nbrOfTags, cmdInfo->protocol);
                        if(cmdInfo->skb == NULL)
                        {
                            ret = -EFAULT;

                            goto out;
                        }
                        cmdInfo->skb->bcm_flags.restore_rx_vlan = 1;
                    }
                }
#endif
            }
            else /* TX */
            {
                BCM_VLAN_DP_DEBUG("Default Action : TX, ACCEPT");

                cmdInfo->skb->dev = realDev;
            }
        }

        *skbp = cmdInfo->skb;

        goto out;
    }

    /* if we get here, there was a HIT */

    if(tableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        BCM_ASSERT(rxVlanDevRule != NULL);

        ethHeader = BCM_VLAN_SKB_ETH_HEADER(cmdInfo->skb);

        if(realDevCtrl->mode == BCM_VLAN_MODE_RG)
        {
           if(rxNbrOfTags == 0)
           {
               /* Admit the unicast frame to the rule rx interface. */
               cmdInfo->skb->dev = rxVlanDevRule;
           }
           else
           {
               /* This is a tagged frame. */
               /* The frame will be admitted to the rule rx interface,
                *   if the rule interface is bridged,
                *   or if the frame destination MAC matches the rule rx interface MAC.
                * Otherwise, the frame will be dropped.
                */
               if(!BCM_VLAN_DEV_INFO(rxVlanDevRule)->vlanDevCtrl->flags.routed ||
                  !memcmp(rxVlanDevRule->dev_addr, ethHeader->macDest, ETH_ALEN))
               {
                   cmdInfo->skb->dev = rxVlanDevRule;
               }
               else
               {
                   ret = cmdHandler_dropFrame(cmdInfo, NULL);

                   goto out;
               }
           }

           *rxVlanDevInStackp = isDevRxVlanDevInStack(cmdInfo->skb->dev, realDev);
        }
        else   /* ONT */
        {
           if(is_multicast_ether_addr(ethHeader->macDest))
           {
               /* Regardless hit or miss, admit the multicast frame to all multicast
                * enabled interfaces, including the rx virtual interfaces.
                */
               forwardMulticast(cmdInfo, realDevCtrl, cmdInfo->skb, is_broadcast_ether_addr(ethHeader->macDest));
               ret = -EFAULT;

               goto out;
           }
           else
           {
               /* Rx vlan device was specified. */
               cmdInfo->skb->dev = rxVlanDevRule;
           }

           *rxVlanDevInStackp = isDevRxVlanDevInStack(cmdInfo->skb->dev, realDev);

#if defined(BCM_VLAN_ENABLE_SKB_VLAN)
           if(!(*rxVlanDevInStackp))
           {
               /* we only save the vlan tags in the SKB if the skb->dev is NOT an
                  inner interface of a VLAN interface stack */

               cmdInfo->skb = saveSkbVlanOnReceive(cmdInfo, cmdInfo->skb, cmdInfo->ethHeader,
                                                   cmdInfo->nbrOfTags, cmdInfo->protocol);
               if(cmdInfo->skb == NULL)
               {
                   ret = -EFAULT;

                   goto out;
               }
               cmdInfo->skb->bcm_flags.restore_rx_vlan = 1;
           }
#endif
        }
    }
    else /* TX */
    {
        cmdInfo->skb->dev = realDev;
    }

    *skbp = cmdInfo->skb;

out:
    return ret;
}

#if defined(CC_BCM_VLAN_FLOW)

//#define BCM_VLAN_FLOW_DEBUG_ENABLE

#ifdef BCM_VLAN_FLOW_DEBUG_ENABLE
#define BCM_VLAN_FLOW_DEBUG(_fmt, _arg...) \
    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "FLOW : " _fmt, ##_arg)
#define BCM_VLAN_FLOW_DUMP_ENTRY(_realDevName, _realDevMode, _nbrOfTags, _tableDir, _tableEntry) \
    dumpTableEntry(_realDevName, _realDevMode, _nbrOfTags, _tableDir, _tableEntry)
#define BCM_VLAN_FLOW_DUMP_BLOG_RULE(_blogRule_p)                       \
    do {printk("\n===============================================================\n"); \
        blog_rule_dump(_blogRule_p); printk("\n");} while(0)
#else
#define BCM_VLAN_FLOW_DEBUG(_fmt, _arg...)
#define BCM_VLAN_FLOW_DUMP_ENTRY(_realDevName, _realDevMode, _nbrOfTags, _tableDir, _tableEntry)
#define BCM_VLAN_FLOW_DUMP_BLOG_RULE(_blogRule_p)
#endif

#define BCM_VLAN_FLOW_VLAN_STACK_MAX      8

#define BCM_VLAN_MASK_ALL_ONES            ~0

#define BCM_VLAN_FLOW_COPY_PBITS          (1 << 0)
#define BCM_VLAN_FLOW_COPY_CFI            (1 << 1)
#define BCM_VLAN_FLOW_COPY_VID            (1 << 2)
#define BCM_VLAN_FLOW_COPY_TAG_ETHERTYPE  (1 << 3)

typedef struct {
    struct net_device *dev;
    bcmVlan_ruleTableDirection_t tableDir;
} bcmVlan_vlanDevStackEntry_t;

typedef struct {
    int count;
    struct net_device *rxRealDevStack;
    bcmVlan_vlanDevStackEntry_t entry[BCM_VLAN_FLOW_VLAN_STACK_MAX];
} bcmVlan_vlanDevStack_t;

typedef struct {
    int count;
    bcmVlan_ruleTable_t ruleTable[2][BCM_VLAN_MAX_RULE_TABLES];
} bcmVlan_flowRuleTables_t;

static int vlanRuleToBlogRule(blogRule_t *blogRule_p,
                              bcmVlan_tableEntry_t *tableEntry,
                              unsigned int nbrOfTags)
{
    int ret = 0;
    bcmVlan_tagRuleFilter_t *tagRuleFilter;
    bcmVlan_vlanTag_t *vlanTag;
    bcmVlan_tagRuleCommand_t *cmd;
    blogRuleAction_t *blogRuleAction_p;
    blogRuleFilterVlan_t *blogRuleFilterVlan_p;
    int i;

    /*
     * Parse VLAN Rule Filters
     */

    tagRuleFilter = &tableEntry->tagRule.filter;

    if(!BCM_VLAN_IS_DONT_CARE(tagRuleFilter->etherType))
    {
        blogRule_p->filter.eth.mask.h_proto = BCM_VLAN_MASK_ALL_ONES;
        blogRule_p->filter.eth.value.h_proto = tagRuleFilter->etherType;

//        BCM_VLAN_FLOW_DEBUG("FILTER: ETHERTYPE <0x%04X>", tagRuleFilter->etherType);
    }
    
    // IPPROTO
    if(!BCM_VLAN_IS_DONT_CARE(tagRuleFilter->ipProto))
    {
        blogRule_p->filter.ipv4.mask.ip_proto = BLOG_RULE_IP_PROTO_MASK;
        blogRule_p->filter.ipv4.value.ip_proto = tagRuleFilter->ipProto << BLOG_RULE_IP_PROTO_SHIFT;

//        BCM_VLAN_FLOW_DEBUG("FILTER: IP-PROTO <%u>", tagRuleFilter->ipProto);
    }

    if(!BCM_VLAN_IS_DONT_CARE(tagRuleFilter->dscp))
    {
        blogRule_p->filter.ipv4.mask.tos = BLOG_RULE_DSCP_IN_TOS_MASK;
        blogRule_p->filter.ipv4.value.tos = tagRuleFilter->dscp << BLOG_RULE_DSCP_IN_TOS_SHIFT;

//        BCM_VLAN_FLOW_DEBUG("FILTER: DSCP <%u>", tagRuleFilter->dscp);
    }

    /* Note: skb priority filter value is offset by 1 because 0 is reserved
     * to indicate filter not in use. Therefore the supported skb priority
     * range is [0 to 0xfffffffe].
     */
    if(!BCM_VLAN_IS_DONT_CARE(tagRuleFilter->skbPrio))
    {
        blogRule_p->filter.skb.priority = tagRuleFilter->skbPrio + 1;

//        BCM_VLAN_FLOW_DEBUG("FILTER: skbPrio <%u>", tagRuleFilter->skbPrio);
    }
    
    if(!BCM_VLAN_IS_DONT_CARE(tagRuleFilter->skbMarkFlowId))
    {
        blogRule_p->filter.skb.markFlowId = tagRuleFilter->skbMarkFlowId;

//        BCM_VLAN_FLOW_DEBUG("FILTER: skbMarkFlowId <%u>", tagRuleFilter->skbMarkFlowId);
    }
    
    /* Note: port mark filter value is offset by 1 because 0 is reserved
     * to indicate filter not in use.
     */
    if(!BCM_VLAN_IS_DONT_CARE(tagRuleFilter->skbMarkPort))
    {
        blogRule_p->filter.skb.markPort = tagRuleFilter->skbMarkPort + 1;

//        BCM_VLAN_FLOW_DEBUG("FILTER: skbMarkPort <%u>", tagRuleFilter->skbMarkPort);
    }
    
    if(!BCM_VLAN_IS_DONT_CARE(tagRuleFilter->dscp2pbits))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "DSCP2PBITS Filter is not supported");

        ret = -EINVAL;

        goto out;
    }

    if(nbrOfTags > BLOG_RULE_VLAN_TAG_MAX)
    {
        BCM_VLAN_FLOW_DEBUG("Number of VLAN filters <%u> exceeds Blog Rule VLAN filters <%u>",
                            nbrOfTags, BLOG_RULE_VLAN_TAG_MAX);
        ret = -EINVAL;

        goto out;
    }

    blogRule_p->filter.nbrOfVlanTags = nbrOfTags;

    for(i=0; i<nbrOfTags; ++i)
    {
        vlanTag = &tagRuleFilter->vlanTag[i];

        blogRuleFilterVlan_p = &blogRule_p->filter.vlan[i];

        if(!BCM_VLAN_IS_DONT_CARE(vlanTag->pbits))
        {
            blogRuleFilterVlan_p->mask.h_vlan_TCI |= BCM_VLAN_PBITS_MASK;
            BCM_VLAN_SET_TCI_PBITS(blogRuleFilterVlan_p->value.h_vlan_TCI, vlanTag->pbits);

//            BCM_VLAN_FLOW_DEBUG("FILTER: PBITS <%u>, Tag <%u>", vlanTag->pbits, i);
        }

        if(!BCM_VLAN_IS_DONT_CARE(vlanTag->cfi))
        {
            blogRuleFilterVlan_p->mask.h_vlan_TCI |= BCM_VLAN_CFI_MASK;
            BCM_VLAN_SET_TCI_CFI(blogRuleFilterVlan_p->value.h_vlan_TCI, vlanTag->cfi);

//            BCM_VLAN_FLOW_DEBUG("FILTER: CFI <%u>, Tag <%u>", vlanTag->cfi, i);
        }

        if(!BCM_VLAN_IS_DONT_CARE(vlanTag->vid))
        {
            blogRuleFilterVlan_p->mask.h_vlan_TCI |= BCM_VLAN_VID_MASK;
            BCM_VLAN_SET_TCI_VID(blogRuleFilterVlan_p->value.h_vlan_TCI, vlanTag->vid);

//            BCM_VLAN_FLOW_DEBUG("FILTER: VID <%u>, Tag <%u>", vlanTag->vid, i);
        }

        if(!BCM_VLAN_IS_DONT_CARE(vlanTag->etherType))
        {
            blogRuleFilterVlan_p->mask.h_vlan_encapsulated_proto = BCM_VLAN_MASK_ALL_ONES;
            blogRuleFilterVlan_p->value.h_vlan_encapsulated_proto = vlanTag->etherType;

//            BCM_VLAN_FLOW_DEBUG("FILTER: TAG ETHERTYPE <%u>, Tag <%u>", vlanTag->etherType, i);
        }
    }

    blogRule_p->filter.flags = tagRuleFilter->flags;

    /*
     * Parse VLAN Rule Commands
     */

    if(tableEntry->cmdCount > (BLOG_RULE_ACTION_MAX - blogRule_p->actionCount))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN,
                      "VLAN commands <%u> exceeds available nbr of Blog Rule actions <%u>",
                      tableEntry->cmdCount, BLOG_RULE_ACTION_MAX - blogRule_p->actionCount);

        ret = -EINVAL;
        goto out;
    }

    for(i=0; i<tableEntry->cmdCount; ++i)
    {
        blogRuleAction_p = &blogRule_p->action[blogRule_p->actionCount];
        blogRule_p->actionCount++;

        cmd = &tableEntry->tagRule.cmd[i];

//        BCM_VLAN_FLOW_DEBUG("[%s, 0x%08X, 0x%08X]",
//                            cmdHandler[cmd->opCode].name, cmd->arg[0], cmd->arg[1]);

        switch(cmd->opCode)
        {
            case BCM_VLAN_OPCODE_NOP:
                break;

            case BCM_VLAN_OPCODE_POP_TAG:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_POP_VLAN_HDR;
                break;

            case BCM_VLAN_OPCODE_PUSH_TAG:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_PUSH_VLAN_HDR;
                break;

            case BCM_VLAN_OPCODE_SET_ETHERTYPE:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_ETHERTYPE;
                blogRuleAction_p->etherType = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_PBITS:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_PBITS;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->pbits = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_CFI:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_DEI;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->dei = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_VID:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_VID;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->vid = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_TAG_ETHERTYPE:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_VLAN_PROTO;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->vlanProto = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_COPY_PBITS:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_COPY_PBITS;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->fromTag = BCM_VLAN_CMD_GET_SOURCE_TAG(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_COPY_CFI:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_COPY_DEI;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->fromTag = BCM_VLAN_CMD_GET_SOURCE_TAG(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_COPY_VID:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_COPY_VID;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->fromTag = BCM_VLAN_CMD_GET_SOURCE_TAG(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_COPY_TAG_ETHERTYPE:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_COPY_VLAN_PROTO;
                blogRuleAction_p->toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
                blogRuleAction_p->fromTag = BCM_VLAN_CMD_GET_SOURCE_TAG(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_DSCP2PBITS:
                /* Not supported */
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "DSCP2PBITS Command is not supported");
                ret = -EINVAL;
                goto out;

            case BCM_VLAN_OPCODE_SET_DSCP:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_DSCP;
                blogRuleAction_p->dscp = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_DROP_FRAME:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_DROP;
                break;

            case BCM_VLAN_OPCODE_SET_SKB_MARK_PORT:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_SKB_MARK_PORT;
                blogRuleAction_p->skbMarkPort = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_SET_SKB_MARK_QUEUE;
                blogRuleAction_p->skbMarkQueue = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_SKB_PRIO:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_FLOWID:
                /* Not supported */
                break;

            case BCM_VLAN_OPCODE_OVRD_LEARNING_VID:
                blogRuleAction_p->cmd = BLOG_RULE_CMD_OVRD_LEARNING_VID;
                blogRuleAction_p->vid = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_CONTINUE:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE_BYPBITS:
                break;

#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
            case BCM_VLAN_OPCODE_DEAGGR_TAG:
                break;
#endif
            default:
                BCM_ASSERT(0);
        }
    }

out:
    return ret;
}

static void moveVlanTagFiltersUp(bcmVlan_tableEntry_t *tableEntry)
{
    bcmVlan_tagRuleFilter_t *tagRuleFilter;
    int i;

    tagRuleFilter = &tableEntry->tagRule.filter;

    for(i=0; i<BCM_VLAN_MAX_TAGS-1; ++i)
    {
        tagRuleFilter->vlanTag[i] = tagRuleFilter->vlanTag[i+1];
    }

    memset(&tagRuleFilter->vlanTag[BCM_VLAN_MAX_TAGS-1],
           BCM_VLAN_DONT_CARE, sizeof(bcmVlan_vlanTag_t));

    tagRuleFilter->vlanTag[BCM_VLAN_MAX_TAGS-1].tciMask = 0;
    tagRuleFilter->vlanTag[BCM_VLAN_MAX_TAGS-1].tci = 0;
}

static int moveVlanTagFiltersDown(bcmVlan_tableEntry_t *tableEntry,
                                  unsigned int nbrOfTags)
{
    int ret = 0;
    bcmVlan_tagRuleFilter_t *tagRuleFilter;
    int i;

    if(nbrOfTags >= BCM_VLAN_MAX_TAGS)
    {
        ret = -EINVAL;
        goto out;
    }

    tagRuleFilter = &tableEntry->tagRule.filter;

    for(i=BCM_VLAN_MAX_TAGS-1; i>0; --i)
    {
        tagRuleFilter->vlanTag[i] = tagRuleFilter->vlanTag[i-1];
    }

    memset(&tagRuleFilter->vlanTag[0],
           BCM_VLAN_DONT_CARE, sizeof(bcmVlan_vlanTag_t));

    tagRuleFilter->vlanTag[0].tciMask = 0;
    tagRuleFilter->vlanTag[0].tci = 0;

out:
    return ret;
}

static int parseSrcVlanRule(bcmVlan_tableEntry_t *srcTableEntry,
                            bcmVlan_tableEntry_t *flowTableEntry,
                            bcmVlan_ruleTableDirection_t srcTableDir,
                            struct net_device *srcVlanDev,
                            unsigned int srcNbrOfTags,
                            unsigned int *dstNbrOfTags_p,
                            int isMulticast)
{
    int ret = 0;
    bcmVlan_tagRuleFilter_t *flowTagRuleFilter;
    bcmVlan_tagRuleCommand_t *cmd;
    bcmVlan_vlanTag_t *flowVlanTag;
    int toTag;
    int i;

    if(srcTableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        BCM_VLAN_FLOW_DEBUG("RX : srcTableEntry->rxVlanDev %s, srcVlanDev %s",
                            srcTableEntry->rxVlanDev ? srcTableEntry->rxVlanDev->name : "NULL",
                            srcVlanDev->name);

        if(srcTableEntry->rxVlanDev &&
           srcTableEntry->rxVlanDev != srcVlanDev)
        {
            /* This VLAN Rule does not match the Rx VLAN Device */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
    }
    else
    {
        BCM_VLAN_FLOW_DEBUG("TX : srcTableEntry->txVlanDev %s, srcVlanDev %s",
                            srcTableEntry->txVlanDev ? srcTableEntry->txVlanDev->name : "NULL",
                            srcVlanDev->name);

        if(srcTableEntry->txVlanDev &&
           srcTableEntry->txVlanDev != srcVlanDev)
        {
            /* This VLAN Rule does not match the Tx VLAN Device */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
    }

    if(isMulticast)
    {
        if(srcTableEntry->tagRule.filter.vlanDevMacAddr == BCM_VLAN_MATCH_VLANDEV_MACADDR ||
           (srcTableEntry->tagRule.filter.flags &&
            !(srcTableEntry->tagRule.filter.flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST)))
        {
            /* This filter excludes multicast packets */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
    }

    /* Copy the Rx VLAN Rule Filters */
    flowTableEntry->tagRule.filter = srcTableEntry->tagRule.filter;

    /* Copy the Rx VLAN Rule Rx/Tx VLAN Devices */
    memcpy(flowTableEntry->tagRule.rxVlanDevName,
           srcTableEntry->tagRule.rxVlanDevName, BCM_VLAN_IFNAMSIZ);
    flowTableEntry->rxVlanDev = srcTableEntry->rxVlanDev;
    flowTableEntry->txVlanDev = srcTableEntry->txVlanDev;

    /* Process Flow Rule */
    flowTagRuleFilter = &flowTableEntry->tagRule.filter;

    for(i=0; i<BCM_VLAN_MAX_TAGS; ++i)
    {
        /* we use the tciMask field as a way to save/remember COPY commands */
        flowTagRuleFilter->vlanTag[i].tciMask = 0;
    }

    /* Convert Rx actions into Flow filters */
    for(i=0; i<srcTableEntry->cmdCount; ++i)
    {
        cmd = &srcTableEntry->tagRule.cmd[i];

//        BCM_VLAN_FLOW_DEBUG("[%s, 0x%08X, 0x%08X]",
//                            cmdHandler[cmd->opCode].name, cmd->arg[0], cmd->arg[1]);

        toTag = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);

        flowVlanTag = &flowTagRuleFilter->vlanTag[toTag];

        switch(cmd->opCode)
        {
            case BCM_VLAN_OPCODE_NOP:
                break;

            case BCM_VLAN_OPCODE_POP_TAG:
                (*dstNbrOfTags_p)--;

                /* move VLAN filters one tag up (e.g., 1 to 0) */
                moveVlanTagFiltersUp(flowTableEntry);
                break;

            case BCM_VLAN_OPCODE_PUSH_TAG:
                (*dstNbrOfTags_p)++;

                /* move VLAN filters one tag down (e.g., 0 to 1) */
                ret = moveVlanTagFiltersDown(flowTableEntry, srcNbrOfTags);
                if(ret)
                {
                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto out;
                }
                break;

            case BCM_VLAN_OPCODE_SET_SKB_MARK_PORT:
                flowTagRuleFilter->skbMarkPort = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_ETHERTYPE:
                flowTagRuleFilter->etherType = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_PBITS:
                flowVlanTag->pbits = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_CFI:
                flowVlanTag->cfi = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_VID:
                flowVlanTag->vid = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_SET_TAG_ETHERTYPE:
                flowVlanTag->etherType = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_COPY_PBITS:
                flowVlanTag->tciMask |= BCM_VLAN_FLOW_COPY_PBITS;
                break;

            case BCM_VLAN_OPCODE_COPY_CFI:
                flowVlanTag->tciMask |= BCM_VLAN_FLOW_COPY_CFI;
                break;

            case BCM_VLAN_OPCODE_COPY_VID:
                flowVlanTag->tciMask |= BCM_VLAN_FLOW_COPY_VID;
                break;

            case BCM_VLAN_OPCODE_COPY_TAG_ETHERTYPE:
                flowVlanTag->tciMask |= BCM_VLAN_FLOW_COPY_TAG_ETHERTYPE;
                break;

            case BCM_VLAN_OPCODE_DSCP2PBITS:
                /* Not supported */
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "DSCP2PBITS Command is not supported");
                ret = -EINVAL;
                goto out;

            case BCM_VLAN_OPCODE_SET_DSCP:
                flowTagRuleFilter->dscp = BCM_VLAN_CMD_GET_VAL(cmd->arg);
                break;

            case BCM_VLAN_OPCODE_DROP_FRAME:
                break;

            case BCM_VLAN_OPCODE_SET_SKB_PRIO:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE_BYPBITS:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_FLOWID:
                /* Not supported */
                break;

            case BCM_VLAN_OPCODE_OVRD_LEARNING_VID:
            case BCM_VLAN_OPCODE_CONTINUE:
                break;

#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
            case BCM_VLAN_OPCODE_DEAGGR_TAG:
                break;
#endif

            default:
                BCM_ASSERT(0);
        }
    }

out:
    return ret;
}

static int mergeVlanRuleFilter(unsigned int rxFilter,
                               unsigned int txFilter,
                               unsigned int *flowFilter_p)
{
    int ret = 0;

    if(BCM_VLAN_FILTER_COMPARE(*flowFilter_p, txFilter))
    {
        /* filters are compatible, let's merge them */

        if(rxFilter != *flowFilter_p)
        {
            /* flow filter was derived from an Rx command */
            *flowFilter_p = rxFilter;
        }
        else /* rxFilter == *flowFilter_p */
        {
            /* flow filter was the original Rx filter */
            if(!BCM_VLAN_IS_DONT_CARE(txFilter))
            {
                *flowFilter_p = txFilter;
            }
            else
            {
                *flowFilter_p = rxFilter;
            }
        }
    }
    else
    {
        ret = -EINVAL;
    }

//    BCM_VLAN_FLOW_DEBUG("FILTER: RX <0x%08X> TX <0x%08X> FLOW <0x%08X>",
//                        rxFilter, txFilter, *flowFilter_p);

    return ret;
}

static bcmVlan_tagRuleCommand_t *findVlanRuleCommand(bcmVlan_tableEntry_t *tableEntry,
                                                     bcmVlan_tagRuleCommand_t *matchCmd)
{
    bcmVlan_tagRuleCommand_t *cmd;
    int i;

    for(i=0; i<tableEntry->cmdCount; ++i)
    {
        cmd = &tableEntry->tagRule.cmd[i];

//        BCM_VLAN_DP_DEBUG("[%s, 0x%08X, 0x%08X]",
//                          cmdHandler[cmd->opCode].name, cmd->arg[0], cmd->arg[1]);

        if(cmd->opCode == matchCmd->opCode)
        {
            switch(cmd->opCode)
            {
                case BCM_VLAN_OPCODE_NOP:
                case BCM_VLAN_OPCODE_POP_TAG:
                case BCM_VLAN_OPCODE_PUSH_TAG:
                case BCM_VLAN_OPCODE_SET_ETHERTYPE:
                case BCM_VLAN_OPCODE_DSCP2PBITS:
                case BCM_VLAN_OPCODE_SET_DSCP:
                case BCM_VLAN_OPCODE_DROP_FRAME:
                case BCM_VLAN_OPCODE_SET_SKB_PRIO:
                case BCM_VLAN_OPCODE_SET_SKB_MARK_PORT:
                case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE:
                case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE_BYPBITS:
                case BCM_VLAN_OPCODE_SET_SKB_MARK_FLOWID:
                case BCM_VLAN_OPCODE_OVRD_LEARNING_VID:
                case BCM_VLAN_OPCODE_CONTINUE:
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
                case BCM_VLAN_OPCODE_DEAGGR_TAG:
#endif
                    return cmd;

                case BCM_VLAN_OPCODE_SET_PBITS:
                case BCM_VLAN_OPCODE_SET_CFI:
                case BCM_VLAN_OPCODE_SET_VID:
                case BCM_VLAN_OPCODE_SET_TAG_ETHERTYPE:
                    if(BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg) ==
                       BCM_VLAN_CMD_GET_TARGET_TAG(matchCmd->arg))
                    {
                        return cmd;
                    }
                    break;

                case BCM_VLAN_OPCODE_COPY_PBITS:
                case BCM_VLAN_OPCODE_COPY_CFI:
                case BCM_VLAN_OPCODE_COPY_VID:
                case BCM_VLAN_OPCODE_COPY_TAG_ETHERTYPE:
                    if((BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg) ==
                        BCM_VLAN_CMD_GET_TARGET_TAG(matchCmd->arg)) &&
                       (BCM_VLAN_CMD_GET_SOURCE_TAG(cmd->arg) ==
                        BCM_VLAN_CMD_GET_SOURCE_TAG(matchCmd->arg)))
                    {
                        return cmd;
                    }
                    break;

                default:
                    BCM_ASSERT(0);
            }
        }
    }

    return NULL;
}

static int addVlanRuleCommand(bcmVlan_tableEntry_t *tableEntry,
                              bcmVlan_tagRuleCommand_t *cmd)
{
    int ret = -EINVAL;

    if(tableEntry->cmdCount < BCM_VLAN_MAX_RULE_COMMANDS)
    {
        tableEntry->tagRule.cmd[tableEntry->cmdCount] = *cmd;
        tableEntry->cmdCount++;

        ret = 0;
    }

    return ret;
}

static int mergeVlanRuleCommand(bcmVlan_tableEntry_t *tableEntry,
                                bcmVlan_tagRuleCommand_t *mergeCmd)
{
    int ret = 0;
    bcmVlan_tagRuleCommand_t *cmd;

    cmd = findVlanRuleCommand(tableEntry, mergeCmd);
    if(cmd == NULL)
    {
        /* command does not exist, let's add it */
        ret = addVlanRuleCommand(tableEntry, mergeCmd);
    }
    else
    {
        /* command already exists, overwrite it */
        *cmd = *mergeCmd;
    }

    return ret;
}

static void removeVlanRuleCommand(bcmVlan_tableEntry_t *tableEntry,
                                  int cmdIndex)
{
    int i;

    BCM_ASSERT(tableEntry->cmdCount > 0);
    BCM_ASSERT(cmdIndex < tableEntry->cmdCount);

    BCM_VLAN_FLOW_DEBUG("cmdIndex %d, tableEntry->cmdCount %u",
                        cmdIndex, tableEntry->cmdCount);

    for(i=cmdIndex; i<tableEntry->cmdCount; ++i)
    {
        if(i == BCM_VLAN_MAX_RULE_COMMANDS-1)
        {
            tableEntry->tagRule.cmd[i].opCode = BCM_VLAN_OPCODE_NOP;
        }
        else
        {
            tableEntry->tagRule.cmd[i] = tableEntry->tagRule.cmd[i+1];
        }
    }

    tableEntry->cmdCount--;
}

static int prependVlanRuleCommand(bcmVlan_tableEntry_t *tableEntry,
                                  bcmVlan_tagRuleCommand_t *cmd)
{
    int ret = -EINVAL;
    int i;

    if(tableEntry->cmdCount < BCM_VLAN_MAX_RULE_COMMANDS)
    {
        /* move existing commands down */

        for(i=tableEntry->cmdCount; i>0; --i)
        {
            tableEntry->tagRule.cmd[i] = tableEntry->tagRule.cmd[i-1];
        }

        /* prepend new command */
        tableEntry->tagRule.cmd[0] = *cmd;

        tableEntry->cmdCount++;

        ret = 0;
    }

    return ret;
}

static int cleanupVlanCommands(bcmVlan_tableEntry_t *tableEntry,
                               unsigned int origNbrOfTags)
{
    int ret = 0;
    bcmVlan_tagRuleCommand_t *cmd;
    bcmVlan_tagRuleCommand_t newCmd;
    int finalNbrOfTags = (int)origNbrOfTags;
    int i;

    /* Remove all POP and PUSH commands and calculate final number of tags */

    i = 0;

    while(i < tableEntry->cmdCount)
    {
        cmd = &tableEntry->tagRule.cmd[i];

        switch(cmd->opCode)
        {
            case BCM_VLAN_OPCODE_POP_TAG:
                removeVlanRuleCommand(tableEntry, i);
                finalNbrOfTags--;
                continue;

            case BCM_VLAN_OPCODE_PUSH_TAG:
                removeVlanRuleCommand(tableEntry, i);
                finalNbrOfTags++;
                continue;

            default:
                break;
        }

        i++;
    }

    BCM_VLAN_FLOW_DEBUG("finalNbrOfTags %d", finalNbrOfTags);

    if(finalNbrOfTags < 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Too many POP commands (finalNbrOfTags = %d)",
                      finalNbrOfTags);
        ret = -EINVAL;
        goto out;
    }

    /* Prepend POP and PUSH commands to match final number of tags */

    if(finalNbrOfTags > origNbrOfTags)
    {
        memset(&newCmd, 0, sizeof(bcmVlan_tagRuleCommand_t));
        newCmd.opCode = BCM_VLAN_OPCODE_PUSH_TAG;

        for(i=0; i<(finalNbrOfTags-origNbrOfTags); ++i)
        {
            ret = prependVlanRuleCommand(tableEntry, &newCmd);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not prependVlanRuleCommand");
                goto out;
            }
        }
    }
    else if(finalNbrOfTags < origNbrOfTags)
    {
        memset(&newCmd, 0, sizeof(bcmVlan_tagRuleCommand_t));
        newCmd.opCode = BCM_VLAN_OPCODE_POP_TAG;

        for(i=0; i<(origNbrOfTags-finalNbrOfTags); ++i)
        {
            ret = prependVlanRuleCommand(tableEntry, &newCmd);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not prependVlanRuleCommand");
                goto out;
            }
        }

        /* If finalNbrOfTags is zero after popping tags, then
         * BCM_VLAN_OPCODE_SET_ETHERTYPE will no longer apply. We
         * want to remove them from tag rule table.
         */
        if(finalNbrOfTags == 0)
        {
            i = 0;

            while(i < tableEntry->cmdCount)
            {
                cmd = &tableEntry->tagRule.cmd[i];

                switch(cmd->opCode)
                {
                    case BCM_VLAN_OPCODE_SET_ETHERTYPE:
                        removeVlanRuleCommand(tableEntry, i);
                        break;

                    default:
                        break;
                }

                i++;
            }
        }
    }

out:
    return ret;
}

static void adjustVlanCommands(bcmVlan_tableEntry_t *tableEntry, int push)
{
    bcmVlan_tagRuleCommand_t *cmd;
    int toTagNbr;
    int fromTagNbr;
    int i;

    i = 0;

    while(i < tableEntry->cmdCount)
    {
        cmd = &tableEntry->tagRule.cmd[i];

        toTagNbr = BCM_VLAN_CMD_GET_TARGET_TAG(cmd->arg);
        fromTagNbr = BCM_VLAN_CMD_GET_SOURCE_TAG(cmd->arg);

        switch(cmd->opCode)
        {
            case BCM_VLAN_OPCODE_NOP:
            case BCM_VLAN_OPCODE_POP_TAG:
            case BCM_VLAN_OPCODE_PUSH_TAG:
            case BCM_VLAN_OPCODE_SET_ETHERTYPE:
            case BCM_VLAN_OPCODE_SET_DSCP:
            case BCM_VLAN_OPCODE_DROP_FRAME:
            case BCM_VLAN_OPCODE_SET_SKB_PRIO:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_PORT:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE_BYPBITS:
            case BCM_VLAN_OPCODE_SET_SKB_MARK_FLOWID:
            case BCM_VLAN_OPCODE_OVRD_LEARNING_VID:
            case BCM_VLAN_OPCODE_CONTINUE:
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
            case BCM_VLAN_OPCODE_DEAGGR_TAG:
#endif
                break;

            case BCM_VLAN_OPCODE_DSCP2PBITS:
            case BCM_VLAN_OPCODE_SET_PBITS:
            case BCM_VLAN_OPCODE_SET_CFI:
            case BCM_VLAN_OPCODE_SET_VID:
            case BCM_VLAN_OPCODE_SET_TAG_ETHERTYPE:
                if(push)
                {
                    if(toTagNbr < BCM_VLAN_MAX_TAGS-1)
                    {
                        BCM_VLAN_CMD_SET_TARGET_TAG(cmd->arg, toTagNbr+1);
                    }
                    else
                    {
                        removeVlanRuleCommand(tableEntry, i);
                        continue;
                    }
                }
                else /* pop */
                {
                    if(toTagNbr > 0)
                    {
                        BCM_VLAN_CMD_SET_TARGET_TAG(cmd->arg, toTagNbr-1);
                    }
                    else
                    {
                        removeVlanRuleCommand(tableEntry, i);
                        continue;
                    }
                }
                break;

            case BCM_VLAN_OPCODE_COPY_PBITS:
            case BCM_VLAN_OPCODE_COPY_CFI:
            case BCM_VLAN_OPCODE_COPY_VID:
            case BCM_VLAN_OPCODE_COPY_TAG_ETHERTYPE:
                if(push)
                {
                    if(toTagNbr < BCM_VLAN_MAX_TAGS-1 &&
                       fromTagNbr < BCM_VLAN_MAX_TAGS-1)
                    {
                        BCM_VLAN_CMD_SET_TARGET_TAG(cmd->arg, toTagNbr+1);
                        BCM_VLAN_CMD_SET_SOURCE_TAG(cmd->arg, fromTagNbr+1);
                    }
                    else
                    {
                        removeVlanRuleCommand(tableEntry, i);
                        continue;
                    }
                }
                else /* pop */
                {
                    if(toTagNbr > 0 && fromTagNbr > 0)
                    {
                        BCM_VLAN_CMD_SET_TARGET_TAG(cmd->arg, toTagNbr-1);
                        BCM_VLAN_CMD_SET_SOURCE_TAG(cmd->arg, fromTagNbr-1);
                    }
                    else
                    {
                        removeVlanRuleCommand(tableEntry, i);
                        continue;
                    }
                }
                break;

            default:
                BCM_ASSERT(0);
        }

        i++;
    }
}

/********************************************************************************
 *
 * Merging requirements for VLAN Rule commands:
 *
 * 1) If a Rx Rule has a COPY command targeted to a given field, a matching Tx
 *    Rule must NOT have a COPY or SET command targeted to the same field.
 *
 * 2) If 1) is satisfied, Tx commands targeted to a given field override Rx
 *    commands targeted to the same field.
 *
 *******************************************************************************/
static int mergeVlanRules(Blog_t *blog_p,
                          bcmVlan_tableEntry_t *srcTableEntry,
                          bcmVlan_tableEntry_t *dstTableEntry,
                          bcmVlan_tableEntry_t *flowTableEntry,
                          bcmVlan_ruleTableDirection_t dstTableDir,
                          struct net_device *dstVlanDev,
                          struct net_device *rxRealDevStack,
                          unsigned int srcNbrOfTags,
                          int isMulticast)
{
    int ret = 0;
    bcmVlan_tagRuleFilter_t *rxTagRuleFilter;
    bcmVlan_tagRuleFilter_t *txTagRuleFilter;
    bcmVlan_tagRuleFilter_t *flowTagRuleFilter;
    bcmVlan_vlanTag_t *rxVlanTag;
    bcmVlan_vlanTag_t *txVlanTag;
    bcmVlan_vlanTag_t *flowVlanTag;
    bcmVlan_tagRuleCommand_t *txCmd;
    int i;

    if (srcTableEntry->tagRule.type == BCM_VLAN_RULE_TYPE_QOS || 
        dstTableEntry->tagRule.type == BCM_VLAN_RULE_TYPE_QOS)
    {
        /* QOS Rule merging is not supported */
        ret = -EINVAL;

        BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

        goto out;
    }

    if(dstTableDir == BCM_VLAN_TABLE_DIR_RX)
    {
        BCM_VLAN_FLOW_DEBUG("RX : dstTableEntry->rxVlanDev %s, dstVlanDev %s",
                            dstTableEntry->rxVlanDev ? dstTableEntry->rxVlanDev->name : "NULL",
                            dstVlanDev->name);

        if(dstTableEntry->rxVlanDev &&
           dstTableEntry->rxVlanDev != dstVlanDev)
        {
            /* This VLAN Rule does not match the Rx VLAN Device */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
    }
    else
    {
        BCM_VLAN_FLOW_DEBUG("TX : dstTableEntry->txVlanDev %s, dstVlanDev %s",
                            dstTableEntry->txVlanDev ? dstTableEntry->txVlanDev->name : "NULL",
                            dstVlanDev->name);

        if(dstTableEntry->txVlanDev &&
           dstTableEntry->txVlanDev != dstVlanDev)
        {
            /* This VLAN Rule does not match the Tx VLAN Device */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
        
        if(dstTableEntry->rxRealDev && rxRealDevStack &&
           dstTableEntry->rxRealDev != rxRealDevStack)
        {
            /* The src tag rule rxRealDev does not match the dst tag rule rxRealDev */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
    }

    if(isMulticast)
    {
        if((dstTableEntry->tagRule.filter.vlanDevMacAddr == BCM_VLAN_MATCH_VLANDEV_MACADDR) ||
           (srcTableEntry->tagRule.filter.flags &&
            !(srcTableEntry->tagRule.filter.flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST)) ||
           (dstTableEntry->tagRule.filter.flags &&
            !(dstTableEntry->tagRule.filter.flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST)))
        {
            /* This filter excludes multicast packets */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
    }

    /*
     * Create Flow Filters
     */

    rxTagRuleFilter = &srcTableEntry->tagRule.filter;
    txTagRuleFilter = &dstTableEntry->tagRule.filter;
    flowTagRuleFilter = &flowTableEntry->tagRule.filter;

    if(!BCM_VLAN_IS_DONT_CARE(rxTagRuleFilter->skbMarkPort))
    {
        BCM_VLAN_FLOW_DEBUG("isMulticast %u blog_p->rx.info.channel %u, rxTagRuleFilter->skbMarkPort %u",
                            isMulticast, blog_p->rx.info.channel, rxTagRuleFilter->skbMarkPort);

        if(blog_p->rx.info.phyHdrType == BLOG_GPONPHY)
        {
            /* For GPONPHY, we check for matching rx channel and skbMarkPort for
             * multicast blog only, because for unicast blog, the rx channel is
             * always the switch gpon port (7), not the gem port id.
             */ 
            if(isMulticast)
            {
                if(blog_p->rx.info.channel != rxTagRuleFilter->skbMarkPort)
                {
                    ret = -EINVAL;

                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto out;
                }
            }
        }
        else if(blog_p->rx.info.phyHdrType == BLOG_EPONPHY)
        {
            ; /* For EPONPHY, do nothing*/
        }
        else
        {
            /* other PHY */
            if(blog_p->rx.info.channel != rxTagRuleFilter->skbMarkPort)
            {
                ret = -EINVAL;

                BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                goto out;
            }
        }
    }

    if(!BCM_VLAN_IS_DONT_CARE(txTagRuleFilter->skbMarkPort))
    {
        BCM_VLAN_FLOW_DEBUG("isMulticast %u blog_p->rx.info.channel %u, txTagRuleFilter->skbMarkPort %u",
                            isMulticast, blog_p->rx.info.channel, txTagRuleFilter->skbMarkPort);

        if(blog_p->rx.info.phyHdrType == BLOG_GPONPHY)
        {
            /* For GPONPHY, we check for matching rx channel and skbMarkPort for
             * multicast blog only, because for unicast blog, the rx channel is
             * always the switch gpon port (7), not the gem port id.
             */ 
            if(isMulticast)
            {
                if(blog_p->rx.info.channel != txTagRuleFilter->skbMarkPort)
                {
                    ret = -EINVAL;

                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto out;
                }
            }
        }
        else if(blog_p->rx.info.phyHdrType == BLOG_EPONPHY)
        {
            ; /* For EPONPHY, do nothing*/
        }
        else
        {
            /* other PHY */
            if(blog_p->rx.info.channel != txTagRuleFilter->skbMarkPort)
            {
                ret = -EINVAL;

                BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                goto out;
            }
        }
    }

    strcpy(flowTagRuleFilter->rxRealDevName, txTagRuleFilter->rxRealDevName);

    ret = mergeVlanRuleFilter(rxTagRuleFilter->skbMarkPort,
                              txTagRuleFilter->skbMarkPort,
                              &flowTagRuleFilter->skbMarkPort);
    if(ret)
    {
        BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

        goto out;
    }

    ret = mergeVlanRuleFilter(rxTagRuleFilter->etherType,
                              txTagRuleFilter->etherType,
                              &flowTagRuleFilter->etherType);
    if(ret)
    {
        BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

        goto out;
    }

    ret = mergeVlanRuleFilter(rxTagRuleFilter->ipProto,
                              txTagRuleFilter->ipProto,
                              &flowTagRuleFilter->ipProto);
    if(ret)
    {
        BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

        goto out;
    }

    ret = mergeVlanRuleFilter(rxTagRuleFilter->dscp,
                              txTagRuleFilter->dscp,
                              &flowTagRuleFilter->dscp);
    if(ret)
    {
        BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

        goto out;
    }

    {
        /* When no flags are set or all flags are set (BCM_VLAN_FILTER_FLAGS_ALL)
           it means "Don't Care" */
        unsigned int srcFlags = (!srcTableEntry->tagRule.filter.flags) ?
            BCM_VLAN_FILTER_FLAGS_ALL : srcTableEntry->tagRule.filter.flags;
        unsigned int dstFlags = (!dstTableEntry->tagRule.filter.flags) ?
            BCM_VLAN_FILTER_FLAGS_ALL : dstTableEntry->tagRule.filter.flags;

        flowTagRuleFilter->flags = srcFlags & dstFlags;

        if(!flowTagRuleFilter->flags)
        {
            /* Traffic types are incompatible */
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }

        if(flowTagRuleFilter->flags == BCM_VLAN_FILTER_FLAGS_ALL)
        {
            flowTagRuleFilter->flags = 0;
        }
    }

    for(i=0; i<BCM_VLAN_MAX_TAGS; ++i)
    {
        rxVlanTag = &rxTagRuleFilter->vlanTag[i];
        txVlanTag = &txTagRuleFilter->vlanTag[i];
        flowVlanTag = &flowTagRuleFilter->vlanTag[i];

        ret = mergeVlanRuleFilter(rxVlanTag->pbits,
                                  txVlanTag->pbits,
                                  &flowVlanTag->pbits);
        if(ret)
        {
            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }

        if((flowVlanTag->tciMask & BCM_VLAN_FLOW_COPY_CFI) &&
           (!BCM_VLAN_IS_DONT_CARE(txVlanTag->cfi)))
        {
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
        ret = mergeVlanRuleFilter(rxVlanTag->cfi,
                                  txVlanTag->cfi,
                                  &flowVlanTag->cfi);
        if(ret)
        {
            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }

        if((flowVlanTag->tciMask & BCM_VLAN_FLOW_COPY_VID) &&
           (!BCM_VLAN_IS_DONT_CARE(txVlanTag->vid)))
        {
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }

        ret = mergeVlanRuleFilter(rxVlanTag->vid,
                                  txVlanTag->vid,
                                  &flowVlanTag->vid);
        if(ret)
        {
            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }

        if((flowVlanTag->tciMask & BCM_VLAN_FLOW_COPY_TAG_ETHERTYPE) &&
           (!BCM_VLAN_IS_DONT_CARE(txVlanTag->etherType)))
        {
            ret = -EINVAL;

            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
        ret = mergeVlanRuleFilter(rxVlanTag->etherType,
                                  txVlanTag->etherType,
                                  &flowVlanTag->etherType);
        if(ret)
        {
            BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

            goto out;
        }
    }

    /*
     * Merge iptv flag
     */

    flowTableEntry->tagRule.isIptvOnly = srcTableEntry->tagRule.isIptvOnly |
                                         dstTableEntry->tagRule.isIptvOnly;

    /*
     * Process Flow Commands
     */

    flowTableEntry->rxVlanDev = dstTableEntry->rxVlanDev;
    flowTableEntry->txVlanDev = dstTableEntry->txVlanDev;

    /* Add the Rx commands to the flow rule */

    flowTableEntry->cmdCount = srcTableEntry->cmdCount;
    memcpy(flowTableEntry->tagRule.cmd, srcTableEntry->tagRule.cmd,
           sizeof(bcmVlan_tagRuleCommand_t) * BCM_VLAN_MAX_RULE_COMMANDS);

    /* Adjust Rx commands based on Tx POP and PUSH commands */

    for(i=0; i<dstTableEntry->cmdCount; ++i)
    {
        txCmd = &dstTableEntry->tagRule.cmd[i];

        switch(txCmd->opCode)
        {
            case BCM_VLAN_OPCODE_POP_TAG:
                adjustVlanCommands(flowTableEntry, 0);
                break;

            case BCM_VLAN_OPCODE_PUSH_TAG:
                adjustVlanCommands(flowTableEntry, 1);
                break;

            default:
                break;
        }
    }

    /* Merge the Tx commands */

    for(i=0; i<dstTableEntry->cmdCount; ++i)
    {
        txCmd = &dstTableEntry->tagRule.cmd[i];

        switch(txCmd->opCode)
        {
            case BCM_VLAN_OPCODE_POP_TAG:
            case BCM_VLAN_OPCODE_PUSH_TAG:
                ret = addVlanRuleCommand(flowTableEntry, txCmd);
                if(ret)
                {
                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto out;
                }
                break;

            default:
                ret = mergeVlanRuleCommand(flowTableEntry, txCmd);
                if(ret)
                {
                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto out;
                }
        }
    }

    /* Remove the unnecessary POP/PUSH commands and move the resulting POP/PUSH
       commands to the beginning of the command list */

    ret = cleanupVlanCommands(flowTableEntry, srcNbrOfTags);

out:
    return ret;
}

static int blogVlanRule(Blog_t *blog_p,
                        blogRule_t *rootBlogRule_p,
                        blogRule_t **tailBlogRule_pp,
                        bcmVlan_tableEntry_t *tableEntry,
                        unsigned int nbrOfTags)
{
    int ret;
    blogRule_t *blogRule_p;

    blogRule_p = blog_rule_alloc();
    if(blogRule_p == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate Blog Rule");

        ret = -ENOMEM;
        goto out;
    }

    if(rootBlogRule_p != NULL)
    {
        /* we have a root blog rule, copy it over the blog rule we
           just allocated */
        *blogRule_p = *rootBlogRule_p;
    }
    else
    {
        /* no root blog rule, initialize the blog rule we just allocated */
        blog_rule_init(blogRule_p);
    }

    BCM_VLAN_FLOW_DUMP_ENTRY("Blogged", 0, nbrOfTags, BCM_VLAN_TABLE_DIR_MAX, tableEntry);

    /* add the vlan flow to the new blog rule */
    ret = vlanRuleToBlogRule(blogRule_p, tableEntry, nbrOfTags);
    if(!ret)
    {
        /* blog rule was created successfully, let's add it to the Blog */
        BCM_VLAN_FLOW_DUMP_BLOG_RULE(blogRule_p);

        if(blog_p->blogRule_p == NULL)
        {
            /* first blog rule */
            blog_p->blogRule_p = (void *)blogRule_p;
        }
        else
        {
            (*tailBlogRule_pp)->next_p = blogRule_p;
        }

        blogRule_p->next_p = NULL;

        *tailBlogRule_pp = blogRule_p;
    }
    else
    {
        blog_rule_free(blogRule_p);
    }

out:
    return ret;
}

static void initFlowRuleTables(bcmVlan_flowRuleTables_t *flowRuleTables_p)
{
    int i;
    int j;
    bcmVlan_ruleTable_t *ruleTable;

    flowRuleTables_p->count = 0;

    for(i=0; i<2; ++i)
    {
        for(j=0; j<BCM_VLAN_MAX_RULE_TABLES; ++j)
        {
            ruleTable = &flowRuleTables_p->ruleTable[i][j];

            initRuleTable(ruleTable, BCM_VLAN_TABLE_DIR_RX);
        }
    }
}

static void freeFlowRuleTables(bcmVlan_flowRuleTables_t *flowRuleTables_p)
{
    int i;
    int j;
    bcmVlan_ruleTable_t *ruleTable;

    flowRuleTables_p->count = 0;

    for(i=0; i<2; ++i)
    {
        for(j=0; j<BCM_VLAN_MAX_RULE_TABLES; ++j)
        {
            ruleTable = &flowRuleTables_p->ruleTable[i][j];

            removeTableEntries(ruleTable, NULL);
        }
    }
}

#define getDstFlowRuleTableIndex(__flowRuleTables_p) ((__flowRuleTables_p)->count & 1)
#define getSrcFlowRuleTableIndex(__flowRuleTables_p) (~(__flowRuleTables_p->count) & 1)

static inline bcmVlan_ruleTable_t *getDstFlowRuleTable(bcmVlan_flowRuleTables_t *flowRuleTables_p,
                                                       uint32 nbrOfTags)
{
    int index = getDstFlowRuleTableIndex(flowRuleTables_p);

//    BCM_VLAN_FLOW_DEBUG("FlowRuleTable Dst <%u>", index);

    return &flowRuleTables_p->ruleTable[index][nbrOfTags];
}

static inline bcmVlan_ruleTable_t *getSrcFlowRuleTable(bcmVlan_flowRuleTables_t *flowRuleTables_p,
                                                       uint32 nbrOfTags)
{
    int index = getSrcFlowRuleTableIndex(flowRuleTables_p);

//    BCM_VLAN_FLOW_DEBUG("FlowRuleTable Src <%u>", index);

    return &flowRuleTables_p->ruleTable[index][nbrOfTags];
}

static void swapFlowRuleTables(bcmVlan_flowRuleTables_t *flowRuleTables_p)
{
    bcmVlan_ruleTable_t *srcFlowRuleTable;
    unsigned int nbrOfTags;

    BCM_VLAN_FLOW_DEBUG("*** SWAP ***");

    if(flowRuleTables_p->count)
    {
        srcFlowRuleTable = getSrcFlowRuleTable(flowRuleTables_p, 0);

        BCM_VLAN_FLOW_DEBUG("Freeing SrcFlowRuleTable <%u>", getSrcFlowRuleTableIndex(flowRuleTables_p));

        /* free active flow rule tables */
        for(nbrOfTags=0; nbrOfTags<=BCM_VLAN_MAX_TAGS; ++nbrOfTags)
        {
            removeTableEntries(&srcFlowRuleTable[nbrOfTags], NULL);
        }
    }

    flowRuleTables_p->count++;
}

static int copyEntryToRuleTable(bcmVlan_ruleTable_t *ruleTable,
                                bcmVlan_tableEntry_t *origTableEntry)
{
    int ret = 0;
    bcmVlan_tableEntry_t *tableEntry;

    /* allocate a new flow entry */

    if(ruleTable == NULL || origTableEntry == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "ruleTable %p, origTableEntry %p",
                      ruleTable, origTableEntry);

        ret = -EINVAL;
        goto out;
    }

    tableEntry = allocTableEntry();
    if(tableEntry == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

        ret = -ENOMEM;
        goto out;
    }

    /* copy original rule */

    *tableEntry = *origTableEntry;

    /* add copy to rule table */

    _BCM_VLAN_LL_APPEND(&ruleTable->tableEntryLL, tableEntry);

out:
    return ret;
}

static void addUnmergedVlanRules(Blog_t *blog_p,
                                 struct net_device *vlanDev,
                                 bcmVlan_ruleTable_t *zeroRuleTable,
                                 bcmVlan_ruleTableDirection_t tableDir,
                                 bcmVlan_flowRuleTables_t *flowRuleTables_p,
                                 int isMulticast)
{
    int ret;
    bcmVlan_ruleTable_t *ruleTable;
    bcmVlan_tableEntry_t *tableEntry;
    unsigned int nbrOfTags;
    bcmVlan_ruleTable_t *flowRuleTable;

    for(nbrOfTags=0; nbrOfTags<=BCM_VLAN_MAX_TAGS; ++nbrOfTags)
    {
        /* process Rx rules from all tables */

        ruleTable = &zeroRuleTable[nbrOfTags];

        flowRuleTable = getDstFlowRuleTable(flowRuleTables_p, nbrOfTags);

        tableEntry = BCM_VLAN_LL_GET_HEAD(ruleTable->tableEntryLL);

        while(tableEntry)
        {
            BCM_VLAN_FLOW_DEBUG("******************************** Interface ********************************\n");
            BCM_VLAN_FLOW_DEBUG("DstFlowRuleTable <%u>", getDstFlowRuleTableIndex(flowRuleTables_p));
            BCM_VLAN_FLOW_DUMP_ENTRY(BCM_VLAN_REAL_DEV(vlanDev)->name, 0,
                                     nbrOfTags, tableDir, tableEntry);
			
            if (tableEntry->tagRule.type == BCM_VLAN_RULE_TYPE_QOS)
            {
                /* QOS Rule merging is not supported */
                BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);
        
                goto next_entry;
            }

            if(tableDir == BCM_VLAN_TABLE_DIR_RX)
            {
                if(tableEntry->rxVlanDev && vlanDev &&
                   tableEntry->rxVlanDev != vlanDev)
                {
                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto next_entry;
                }
            }
            else
            {
                if(tableEntry->txVlanDev && vlanDev &&
                   tableEntry->txVlanDev != vlanDev)
                {
                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto next_entry;
                }
            }

            if(isMulticast)
            {
                if(tableEntry->tagRule.filter.vlanDevMacAddr == BCM_VLAN_MATCH_VLANDEV_MACADDR ||
                   (tableEntry->tagRule.filter.flags &&
                    !(tableEntry->tagRule.filter.flags & BCM_VLAN_FILTER_FLAGS_IS_MULTICAST)))
                {
                    /* This filter excludes multicast packets */
                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto next_entry;
                }
            }

            if(!BCM_VLAN_IS_DONT_CARE(tableEntry->tagRule.filter.skbMarkPort))
            {
                if(blog_p->rx.info.channel != tableEntry->tagRule.filter.skbMarkPort)
                {
                    ret = -EINVAL;

                    BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                    goto next_entry;
                }
            }

            if(tableEntry->mergeCount != 0)
            {
                /* rule has already been merged, let's skip it */
                BCM_VLAN_FLOW_DEBUG("%s, %u: Mismatch", __FUNCTION__, __LINE__);

                goto next_entry;
            }

            /* add table entry to the flow rule table */

            ret = copyEntryToRuleTable(flowRuleTable, tableEntry);
            if(ret)
            {
                return;
            }

        next_entry:
            tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
        }
    }
}

static void resetMergeCounts(bcmVlan_ruleTable_t *zeroRuleTable)
{
    bcmVlan_tableEntry_t *tableEntry;
    unsigned int nbrOfTags;

    for(nbrOfTags=0; nbrOfTags<=BCM_VLAN_MAX_TAGS; ++nbrOfTags)
    {
        /* process Rx rules from all tables */

        tableEntry = BCM_VLAN_LL_GET_HEAD(zeroRuleTable[nbrOfTags].tableEntryLL);

        while(tableEntry)
        {
            tableEntry->mergeCount = 0;

            tableEntry = BCM_VLAN_LL_GET_NEXT(tableEntry);
        }
    }
}

static int mergeRuleTables(Blog_t *blog_p,
                           struct net_device *srcVlanDev,
                           struct net_device *dstVlanDev,
                           struct net_device *rxRealDevStack,
                           struct realDeviceControl *dstRealDevCtrl,
                           bcmVlan_ruleTable_t *srcZeroRuleTable,
                           bcmVlan_ruleTable_t *dstZeroRuleTable,
                           bcmVlan_ruleTableDirection_t srcTableDir,
                           bcmVlan_ruleTableDirection_t dstTableDir,
                           bcmVlan_flowRuleTables_t *flowRuleTables_p,
                           int isMulticast)
{
    int ret = 0;
    bcmVlan_ruleTable_t *srcRuleTable;
    bcmVlan_tableEntry_t *srcTableEntry;
    unsigned int srcNbrOfTags;
    bcmVlan_ruleTable_t *dstRuleTable;
    bcmVlan_tableEntry_t *dstTableEntry;
    unsigned int dstNbrOfTags;
    bcmVlan_tableEntry_t *flowTableEntry;
    bcmVlan_tableEntry_t *flowTableEntrySave;
    bcmVlan_ruleTable_t *flowRuleTable;
    bcmVlan_tableEntry_t *defaultTableEntry;

    /* Merge Src and Dst rules */

    /* allocate flow entry for merging rules */
    flowTableEntry = allocTableEntry();
    if(flowTableEntry == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

        ret = -ENOMEM;
        goto out;
    }

    flowTableEntrySave = allocTableEntry();
    if(flowTableEntrySave == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

        freeTableEntry(flowTableEntry);

        ret = -ENOMEM;
        goto out;
    }

    defaultTableEntry = allocTableEntry();
    if(defaultTableEntry == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

        freeTableEntry(flowTableEntry);
        freeTableEntry(flowTableEntrySave);

        ret = -ENOMEM;
        goto out;
    }
    memset(defaultTableEntry, 0, sizeof(bcmVlan_tableEntry_t));
    bcmVlan_initTagRule(&defaultTableEntry->tagRule);

    for(srcNbrOfTags=0; srcNbrOfTags<=BCM_VLAN_MAX_TAGS; ++srcNbrOfTags)
    {
        /* get Src rules from all tables */
        srcRuleTable = &srcZeroRuleTable[srcNbrOfTags];

        flowRuleTable = getDstFlowRuleTable(flowRuleTables_p, srcNbrOfTags);

        srcTableEntry = BCM_VLAN_LL_GET_HEAD(srcRuleTable->tableEntryLL);

        if(srcTableEntry == NULL &&
           srcRuleTable->defaultAction == BCM_VLAN_ACTION_ACCEPT)
        {
            /* we need to create flows for the MISS case when the default
               Table Rule action is ACCEPT */
            defaultTableEntry->mergeCount = 0;
            srcTableEntry = defaultTableEntry;
        }

        while(srcTableEntry)
        {
            BCM_VLAN_FLOW_DEBUG("******************************** Source Rule ********************************");
//            BCM_VLAN_FLOW_DEBUG("DstFlowRuleTable <%u>", getDstFlowRuleTableIndex(flowRuleTables_p));
            BCM_VLAN_FLOW_DUMP_ENTRY(BCM_VLAN_REAL_DEV(srcVlanDev)->name,
                                     0, srcNbrOfTags,
                                     srcTableDir, srcTableEntry);

            dstNbrOfTags = srcNbrOfTags;

            /* create merge flow (flowTableEntry) */

            ret = parseSrcVlanRule(srcTableEntry,
                                   flowTableEntry,
                                   srcTableDir,
                                   srcVlanDev,
                                   srcNbrOfTags,
                                   &dstNbrOfTags,
                                   isMulticast);
            if(!ret)
            {
                if(dstNbrOfTags <= BCM_VLAN_MAX_TAGS)
                {
                    /* merge with Dst rules */

                    *flowTableEntrySave = *flowTableEntry;

                    dstRuleTable = &dstZeroRuleTable[dstNbrOfTags];

                    dstTableEntry = BCM_VLAN_LL_GET_HEAD(dstRuleTable->tableEntryLL);

                    while(dstTableEntry)
                    {
                        BCM_VLAN_FLOW_DEBUG("---> Destination Rule");
                        BCM_VLAN_FLOW_DUMP_ENTRY(BCM_VLAN_REAL_DEV(dstVlanDev)->name,
                                                 0, dstNbrOfTags,
                                                 dstTableDir, dstTableEntry);

                        *flowTableEntry = *flowTableEntrySave;

                        ret = mergeVlanRules(blog_p,
                                             srcTableEntry,
                                             dstTableEntry,
                                             flowTableEntry,
                                             dstTableDir,
                                             dstVlanDev,
                                             rxRealDevStack,
                                             srcNbrOfTags,
                                             isMulticast);
                        if(!ret)
                        {
                            /* Rule merging was successfull */

                            BCM_VLAN_FLOW_DEBUG("Merge Success");
                            BCM_VLAN_FLOW_DUMP_ENTRY("Merged", 0, srcNbrOfTags,
                                                     srcTableDir, flowTableEntry);

                            /* add flow (merged rules) to the flow rule table */

                            ret = copyEntryToRuleTable(flowRuleTable, flowTableEntry);
                            if(ret)
                            {
                                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

                                ret = -ENOMEM;
                                goto out_free;
                            }

                            srcTableEntry->mergeCount++;

                            dstTableEntry->mergeCount++;
                        }
                        else /* Could not merge */
                        {
                            BCM_VLAN_FLOW_DEBUG("Merge Failure");
                            ret = 0;
                        }

                        dstTableEntry = BCM_VLAN_LL_GET_NEXT(dstTableEntry);
                    }

                    if(srcTableEntry->mergeCount == 0 || srcTableEntry == defaultTableEntry)
                    {
                        /* Src rule did not merge with any Dst rule or this is the default
                           source rule (Rx default action is ACCEPT), let's merge it with the
                           Dst rule table default action */

                        if(dstRuleTable->defaultAction == BCM_VLAN_ACTION_ACCEPT)
                        {
                            /* - if the Dst default action is DROP, only Src rules that
                               have successfully merged with Dst rules are allowed since
                               a Dst MISS would result in a packet drop.
                               - if the Dst default action is ACCEPT, all *UNMERGED* Src rules
                               must be included here */

                            ret = copyEntryToRuleTable(flowRuleTable, srcTableEntry);
                            if(ret)
                            {
                                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

                                ret = -ENOMEM;
                                goto out_free;
                            }

                            srcTableEntry->mergeCount++;

                            BCM_VLAN_FLOW_DEBUG("Adding Original Source Rule");
                        }
                    }
                }
                else /* resulting Dst number of Tags is not supported */
                {
                    /* add Src rule as is */

                    ret = copyEntryToRuleTable(flowRuleTable, srcTableEntry);
                    if(ret)
                    {
                        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Could not allocate table entry memory");

                        ret = -ENOMEM;
                        goto out_free;
                    }

                    srcTableEntry->mergeCount++;

                    BCM_VLAN_FLOW_DEBUG("Adding Original Source Rule");
                }
            }
            else
            {
                BCM_VLAN_FLOW_DEBUG("Could not parseSrcVlanRule");
                ret = 0;
            }

            if(srcTableEntry == defaultTableEntry)
            {
                break;
            }
            else
            {
                srcTableEntry = BCM_VLAN_LL_GET_NEXT(srcTableEntry);

                if(srcTableEntry == NULL &&
                   srcRuleTable->defaultAction == BCM_VLAN_ACTION_ACCEPT)
                {
                    /* we need to create flows for the MISS case when the default
                       Table Rule action is ACCEPT */
                    defaultTableEntry->mergeCount = 0;
                    srcTableEntry = defaultTableEntry;
                }
            }
        }
    }

out_free:
    freeTableEntry(flowTableEntry);
    freeTableEntry(flowTableEntrySave);
    freeTableEntry(defaultTableEntry);

out:
    return ret;
}

static int buildVlanDevStack(struct net_device *rxVlanDev,
                             struct net_device *txVlanDev,
                             bcmVlan_vlanDevStack_t *vlanDevStack_p)
{
    int ret = 0;
    struct net_device *vlanDevList[BCM_VLAN_FLOW_VLAN_STACK_MAX];
    struct net_device *dev_p;
    int vlanDevCount;
    bcmVlan_vlanDevStackEntry_t *vlanDevStackEntry_p;

    memset(vlanDevStack_p, 0, sizeof(bcmVlan_vlanDevStack_t));

    if(rxVlanDev != NULL)
    {
        /* find Rx VLAN devices */

        dev_p = rxVlanDev;
        vlanDevCount = 0;

        while(1)
        {
            if(netdev_path_is_root(dev_p))
            {
                vlanDevStack_p->rxRealDevStack = dev_p;
                break;
            }

            if(vlanDevCount == BCM_VLAN_FLOW_VLAN_STACK_MAX)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Too many stacked VLAN Interfaces");

                ret = -EINVAL;
                goto out;
            }

            if(dev_p->priv_flags & IFF_BCM_VLAN)
            {
                vlanDevList[vlanDevCount++] = dev_p;
            }

            dev_p = netdev_path_next_dev(dev_p);
        }

        /* add Rx VLAN devices to the main stack */

        vlanDevStack_p->count = 0;

        while(vlanDevCount)
        {
            vlanDevStackEntry_p = &vlanDevStack_p->entry[vlanDevStack_p->count++];

            vlanDevStackEntry_p->dev = vlanDevList[--vlanDevCount];
            vlanDevStackEntry_p->tableDir = BCM_VLAN_TABLE_DIR_RX;
        }
    }

    if(txVlanDev != NULL)
    {
        /* find and add Tx VLAN devices to the main stack */

        dev_p = txVlanDev;

        while(1)
        {
            if(netdev_path_is_root(dev_p))
            {
                break;
            }

            if(vlanDevStack_p->count == BCM_VLAN_FLOW_VLAN_STACK_MAX)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Too many stacked VLAN Interfaces");

                ret = -EINVAL;
                goto out;
            }

            if(dev_p->priv_flags & IFF_BCM_VLAN)
            {
                vlanDevStackEntry_p = &vlanDevStack_p->entry[vlanDevStack_p->count++];

                vlanDevStackEntry_p->dev = dev_p;
                vlanDevStackEntry_p->tableDir = BCM_VLAN_TABLE_DIR_TX;
            }

            dev_p = netdev_path_next_dev(dev_p);
        }
    }

#ifdef BCM_VLAN_FLOW_DEBUG_ENABLE
    {
        int i;

        printk("VLAN Stack : ");

        for(i=0; i<vlanDevStack_p->count; ++i)
        {
            vlanDevStackEntry_p = &vlanDevStack_p->entry[i];

            printk("%s[%s] ", 
                   vlanDevStackEntry_p->dev->name,
                   (vlanDevStackEntry_p->tableDir) == BCM_VLAN_TABLE_DIR_RX ? "RX" : "TX");
        }

        printk("\n");
    }
#endif

out:
    return ret;
}

static int mergeVlanDevStack(Blog_t *blog_p,
                             bcmVlan_vlanDevStack_t *vlanDevStack_p,
                             bcmVlan_flowRuleTables_t *flowRuleTables_p,
                             int isMulticast)
{
    int ret = 0;
    int srcVlanIndex;
    int dstVlanIndex;
    struct net_device *realDev;
    struct realDeviceControl *realDevCtrl;
    struct net_device *srcVlanDev;
    struct net_device *dstVlanDev;
    bcmVlan_ruleTableDirection_t srcTableDir;
    bcmVlan_ruleTableDirection_t dstTableDir;
    bcmVlan_ruleTable_t *srcRuleTable;
    bcmVlan_ruleTable_t *dstRuleTable;
    bcmVlan_vlanDevStackEntry_t *vlanDevStackEntry_p;
    bcmVlan_defaultAction_t prevDstDefaultActions[BCM_VLAN_MAX_RULE_TABLES];
    int i;

    dstVlanIndex = 1;

    while(1)
    {
        /* find Source Rule Table */

        srcVlanIndex = dstVlanIndex-1;

        vlanDevStackEntry_p = &vlanDevStack_p->entry[srcVlanIndex];

        srcVlanDev = vlanDevStackEntry_p->dev;
        srcTableDir = vlanDevStackEntry_p->tableDir;

        if(srcVlanIndex == 0)
        {
            /* This is the first VLAN device, let's use the associated Rule Tables
               as the source for merging */

            realDev = BCM_VLAN_REAL_DEV(srcVlanDev);

            realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
            if(realDevCtrl == NULL)
            {
                ret = -ENODEV;
                goto out;
            }

            srcRuleTable = realDevCtrl->ruleTable[srcTableDir];

            BCM_VLAN_FLOW_DEBUG("Merge Source: %s[%s]", srcVlanDev->name,
                                (srcTableDir == BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX");
        }
        else
        {
            srcRuleTable = getSrcFlowRuleTable(flowRuleTables_p, 0);

            for(i=0; i<BCM_VLAN_MAX_RULE_TABLES; ++i)
            {
                srcRuleTable[i].defaultAction = prevDstDefaultActions[i];
            }

            BCM_VLAN_FLOW_DEBUG("Merge Source: Flow Rule Table[%u], srcVlanDev %s[%s]",
                                getSrcFlowRuleTableIndex(flowRuleTables_p), srcVlanDev->name,
                                (srcTableDir == BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX");
        }

        resetMergeCounts(srcRuleTable);

        /* find Destination Rule Table */

        vlanDevStackEntry_p = &vlanDevStack_p->entry[dstVlanIndex];

        dstVlanDev = vlanDevStackEntry_p->dev;

        if(dstVlanDev != NULL)
        {
            realDev = BCM_VLAN_REAL_DEV(dstVlanDev);

            realDevCtrl = bcmVlan_getRealDevCtrl(realDev);
            if(realDevCtrl == NULL)
            {
                ret = -ENODEV;
                goto out;
            }

            dstTableDir = vlanDevStackEntry_p->tableDir;
            dstRuleTable = realDevCtrl->ruleTable[dstTableDir];

            BCM_VLAN_FLOW_DEBUG("Merge Destination: %s[%s]", dstVlanDev->name,
                                (dstTableDir == BCM_VLAN_TABLE_DIR_RX) ? "RX" : "TX");

            resetMergeCounts(dstRuleTable);

            for(i=0; i<BCM_VLAN_MAX_RULE_TABLES; ++i)
            {
                prevDstDefaultActions[i] = dstRuleTable[i].defaultAction;
            }

            /* Merge Rule Tables */

            ret = mergeRuleTables(blog_p,
                                  srcVlanDev,
                                  dstVlanDev,
                                  vlanDevStack_p->rxRealDevStack,
                                  realDevCtrl,
                                  srcRuleTable, dstRuleTable,
                                  srcTableDir, dstTableDir,
                                  flowRuleTables_p,
                                  isMulticast);
            if(ret)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "mergeRuleTables failed");

                goto out;
            }
        }
        else
        {
            /* Add receive VLAN rules. We can only get here if there is a single
               Rx Interface in the VLAN stack */

            addUnmergedVlanRules(blog_p,
                                 srcVlanDev,
                                 srcRuleTable,
                                 srcTableDir,
                                 flowRuleTables_p,
                                 isMulticast);
        }

        /* Note: Unmerged source and destination VLAN rules are added by mergeRuleTables() to
           preserve the match order */

        if(dstVlanIndex >= vlanDevStack_p->count-1)
        {
            break;
        }

        dstVlanIndex++;

        swapFlowRuleTables(flowRuleTables_p);
    }

out:
    return ret;
}

static bool isIptvOnlyDev(bcmVlan_vlanDevStack_t *vlanDevStack_p)
{
    int i;
    bcmVlan_vlanDevStackEntry_t *vlanDevStackEntry_p;
    bool flag = 0;

    for(i = 0; i < vlanDevStack_p->count; ++i)
    {
        vlanDevStackEntry_p = &vlanDevStack_p->entry[i];

        if((vlanDevStackEntry_p->tableDir == BCM_VLAN_TABLE_DIR_RX) && 
           bcmVlan_isIptvOnlyVlanDevice(vlanDevStackEntry_p->dev))
        {
            flag = 1;
            break;
        }
    }

    return flag;
}

static int createBlogRules(Blog_t *blog_p,
                           bcmVlan_vlanDevStack_t *vlanDevStack_p,
                           bcmVlan_flowRuleTables_t *flowRuleTables_p)
{
    int ret = 0;
    blogRule_t *rootBlogRule_p;
    blogRule_t *tailBlogRule_p;
    unsigned int rxNbrOfTags;
    bcmVlan_ruleTable_t *flowRuleTable;
    bcmVlan_tableEntry_t *flowTableEntry;
    bool iptvOnlyDev;
    tailBlogRule_p = NULL;

    /* get the root blog rule */

    rootBlogRule_p = (blogRule_t *)blog_p->blogRule_p;
    if(rootBlogRule_p != NULL && rootBlogRule_p->next_p != NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Only one Root Blog Rule is supported");

        ret = -EINVAL;
        goto out;
    }

    /* remove root blog rule from the Blog, if any */

    blog_p->blogRule_p = NULL;

    /* convert VLAN flows to Blog Rules */

    for(rxNbrOfTags=0; rxNbrOfTags<=BCM_VLAN_MAX_TAGS; ++rxNbrOfTags)
    {
        flowRuleTable = getDstFlowRuleTable(flowRuleTables_p, rxNbrOfTags);
        BCM_VLAN_FLOW_DEBUG("DstFlowRuleTable <%u>", getDstFlowRuleTableIndex(flowRuleTables_p));

        flowTableEntry = BCM_VLAN_LL_GET_HEAD(flowRuleTable->tableEntryLL);

        iptvOnlyDev = isIptvOnlyDev(vlanDevStack_p);
        while(flowTableEntry)
        {
            if(blog_p->rx.multicast && iptvOnlyDev &&
               !(flowTableEntry->tagRule.isIptvOnly))
            {
                BCM_VLAN_FLOW_DUMP_ENTRY("Skipped", 0, rxNbrOfTags, BCM_VLAN_TABLE_DIR_MAX, flowTableEntry);
            }
            else
            {
                blogVlanRule(blog_p, rootBlogRule_p, &tailBlogRule_p,
                             flowTableEntry, rxNbrOfTags);
            }

            flowTableEntry = BCM_VLAN_LL_GET_NEXT(flowTableEntry);
        }
    }
    if(rootBlogRule_p != NULL)
    {
        /* If we could not merge we will not return any Blog Rules so the caller
           can fail the activation */

        blog_rule_free(rootBlogRule_p);
    }

out:
    return ret;
}

int bcmVlan_blogVlanFlows(Blog_t *blog_p,
                          struct net_device *rxVlanDev,
                          struct net_device *txVlanDev)
{
    int ret = 0;
    bcmVlan_vlanDevStack_t vlanDevStack;
    bcmVlan_flowRuleTables_t flowRuleTables;

    /**************************************************************
     * Error Checking
     **************************************************************/

    BCM_ASSERT(blog_p != NULL);

    if(rxVlanDev == NULL && txVlanDev == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "VLAN Interfaces not specified");

        ret = -EINVAL;
        goto out;
    }

    if(rxVlanDev != NULL)
    {
        if(!(rxVlanDev->priv_flags & IFF_BCM_VLAN))
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "%s is NOT a VLAN device", rxVlanDev->name);

            ret = -EINVAL;
            goto out;
        }
    }

    if(txVlanDev != NULL)
    {
        if(!(txVlanDev->priv_flags & IFF_BCM_VLAN))
        {
            BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "%s is NOT a VLAN device", txVlanDev->name);

            ret = -EINVAL;
            goto out;
        }
    }

    /**************************************************************
     * Initialization
     **************************************************************/

    /* initialize the flow rule tables */

    initFlowRuleTables(&flowRuleTables);

    /**************************************************************
     * Build the end-to-end VLAN Stack
     **************************************************************/

    ret = buildVlanDevStack(rxVlanDev, txVlanDev, &vlanDevStack);
    if(ret)
    {
        goto out;
    }

    /**************************************************************
     * Merge the VLAN stack tables
     **************************************************************/

    ret = mergeVlanDevStack(blog_p, &vlanDevStack, &flowRuleTables, blog_p->rx.multicast);

    /**************************************************************
     * Create Blog Rules
     **************************************************************/

    ret = createBlogRules(blog_p, &vlanDevStack, &flowRuleTables);

    /**************************************************************
     * Free Flow rule tables
     **************************************************************/

    freeFlowRuleTables(&flowRuleTables);

out:
    return ret;
}

#endif /* CC_BCM_VLAN_FLOW */
