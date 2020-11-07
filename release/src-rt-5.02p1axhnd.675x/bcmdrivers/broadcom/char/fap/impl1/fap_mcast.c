/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : fapProtocol.c
 *
 * Description: This implementation supports the dynamically learnt NAT Flows
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/ppp_defs.h>
#include <linux/version.h>
#include <net/ip.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>
#include "pktHdr.h"
#include "fap_hw.h"
#include "fap.h"
#include "fap_task.h"
#include "bcmtypes.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap_packet.h"
#include "fap_protocol.h"
#include "fap_mcast.h"
#include "fap4ke_iopDma.h"
#include "bcmnet.h"             /* SKBMARK_GET_Q_PRIO */
#include "bcmxtmrt.h"
#include "bcmPktDma.h"
#include "bcm_vlan.h"           
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
#include <net/ipv6.h>
#endif


#define fapMcast_dumpBlogRule(_blogRule_p)                            \
    do {\
        if(bcmLog_logIsEnabled(BCM_LOG_ID_FAPPROTO, BCM_LOG_LEVEL_DEBUG)){ \
            printk("\n========================================\n"); \
            blog_rule_dump(_blogRule_p); printk("\n");\
            printk("\n========================================\n"); \
         } \
       } while(0)

#define fapMcast_dumpBlog(blog_p) \
    do { \
          if(bcmLog_logIsEnabled(BCM_LOG_ID_FAPPROTO, BCM_LOG_LEVEL_DEBUG)){ \
            printk("\n========================================\n"); \
            blog_dump(blog_p); \
            printk("\n========================================\n"); \
          } \
       } while(0)

#define fapMcast_debug(fmt, arg...) BCM_LOG_DEBUG(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define fapMcast_info(fmt, arg...) BCM_LOG_INFO(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define fapMcast_error(fmt, arg...) BCM_LOG_ERROR(BCM_LOG_ID_FAPPROTO, fmt, ##arg) 
#define fapMcast_assert(condition)  BCM_ASSERT(condition)

#define FAP_MCAST_HTABLE_SIZE     64   /* must not be greater than 255 */

#undef  FAP4KE_LEARNACTION_DECL
#define FAP4KE_LEARNACTION_DECL(x) #x
static char *learnActionName[]= {
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_SET_MAC_DA),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_INSERT_MAC_DA),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_SET_MAC_SA),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_INSERT_MAC_SA),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_INSERT_ETHERTYPE),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_POP_BRCM_TAG),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_POP_BRCM2_TAG),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_PUSH_BRCM2_TAG),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_POP_VLAN_HDR),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_PUSH_VLAN_HDR),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_COPY_VLAN_HDR),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_SET_VLAN_PROTO),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_SET_VID),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_SET_DEI),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_SET_PBITS),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_POP_PPPOE_HDR),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_POP_PPPOA_HDR),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_DECR_TTL),
    FAP4KE_LEARNACTION_DECL(FAP4KE_PKT_CMD_MAX)
};

typedef struct {
    uint16_t rxPhyHdr;
    uint16_t rxChannel;
    struct {
        uint16_t isIPv6    : 1;
        uint16_t isSsm     : 1;
        uint16_t reserved  : 14;
    } flags;
    uint16_t nbrOfTags;
    fap4kePkt_vlanIdFilter_t vlanId;
    union {
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        struct {
            ip6_addr_t ipSa6;
            ip6_addr_t ipDa6;
        };
#endif
        struct {
            uint32 ipSa4;
            uint32 ipDa4;
        };
    };
} fapMcast_mcastFlowTuple_t;

typedef struct {
    Dll_t node;
    uint16_t flowIndex;
    uint8_t nClients;
    uint8_t rsvd_byte;
    fapMcast_mcastFlowTuple_t tuple;
    fapPkt_flowHandle_t flowHandle;
    McCfglog_t *mcCfgloglist; 
} fapMcast_mcastFlow_t;

typedef struct {
    struct kmem_cache *mcastFlowCache;
    Dll_t mcastHashTable[FAP_MCAST_HTABLE_SIZE];
    uint16_t flowIndexAlloc;
    uint16_t useIudma2Bd;
} fapMcast_t;


static fapMcast_t fapMcast;

static void setUseIudma2BD(void)
{
#ifdef USE_IUDMA_2BD
    fapMcast.useIudma2Bd = 1;
#else /* USE_IUDMA_2BD */
    fapMcast.useIudma2Bd =0;
#endif    
}

static fapMcast_mcastFlow_t *mcastFlowAlloc(void)
{
    return kmem_cache_alloc(fapMcast.mcastFlowCache, GFP_ATOMIC);
}

static void mcastFlowFree(fapMcast_mcastFlow_t *mcastFlow_p)
{
    kmem_cache_free(fapMcast.mcastFlowCache, mcastFlow_p);
}


static void __dumpMcastFlow(fapMcast_mcastFlow_t *mcastFlow_p)
{
    McCfglog_t *mcCfglog_p;
    int i;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
	if(mcastFlow_p->tuple.flags.isIPv6)
	{
		printk("FAP Mcast Flow <%d> : 4keflowId <%d>, RxPhy <%s>, rxChannel <%d>\n"
                "nvlanTags <%u>, outerVlanId <0x%04X>, innerVlanId <0x%04X>\n"
				"saddr" IP6HEX ", daddr" IP6HEX "\n",
				fapGetHwEntIx(mcastFlow_p->flowIndex),
				mcastFlow_p->flowHandle.flowId,
				(mcastFlow_p->tuple.rxPhyHdr == BLOG_ENETPHY) ? "ENET" : ((mcastFlow_p->tuple.rxPhyHdr == BLOG_GPONPHY) ? "GPON" :  "XTM"),
				mcastFlow_p->tuple.rxChannel,
                mcastFlow_p->tuple.nbrOfTags, mcastFlow_p->tuple.vlanId.outer, mcastFlow_p->tuple.vlanId.inner,
				IP6(mcastFlow_p->tuple.ipSa6), IP6(mcastFlow_p->tuple.ipDa6));
	}
	else
#endif
	{
		printk("FAP Mcast Flow <%d> : 4keflowId <%d>, RxPhy <%s>, rxChannel <%d>\n"
                "nvlanTags <%u>, outerVlanId <0x%04X>, innerVlanId <0x%04X>\n"
				"saddr" IP4DDN ", daddr" IP4DDN "\n",
				fapGetHwEntIx(mcastFlow_p->flowIndex),
				mcastFlow_p->flowHandle.flowId,
				(mcastFlow_p->tuple.rxPhyHdr == BLOG_ENETPHY) ? "ENET" : ((mcastFlow_p->tuple.rxPhyHdr == BLOG_GPONPHY) ? "GPON" :  "XTM"),
				mcastFlow_p->tuple.rxChannel,
                mcastFlow_p->tuple.nbrOfTags, mcastFlow_p->tuple.vlanId.outer, mcastFlow_p->tuple.vlanId.inner,
				IP4(mcastFlow_p->tuple.ipSa4), IP4(mcastFlow_p->tuple.ipDa4) );
	}

    printk("MCAST Clients< %d>\n",mcastFlow_p->nClients);
    mcCfglog_p = mcastFlow_p->mcCfgloglist;
    while(mcCfglog_p)
    {
        printk("mccfgIdx=%d DnatAddr=" IP4DDN " dest_portmask=0x%04x, isextSWPortonIntSW=%d nTxDevs=%d txDevs=",
                mcCfglog_p->mccfgIdx, IP4(mcCfglog_p->destIpAddr), mcCfglog_p->portmask, mcCfglog_p->isextSWPortonIntSW,
                mcCfglog_p->nTxDevs);    
        for(i=0; i<FAP_MCCFGLOG_MAX_TXDEVS;i++)
        {
            if(mcCfglog_p->txDev_pp[i])
            printk("  %pS",mcCfglog_p->txDev_pp[i]);
        }
        printk("\n");
        mcCfglog_p = mcCfglog_p->next;
    }
}

static inline void dumpMcastFlow(fapMcast_mcastFlow_t *mcastFlow_p)
{
    if(bcmLog_logIsEnabled(BCM_LOG_ID_FAPPROTO, BCM_LOG_LEVEL_INFO))
    {
        printk("\n========================================\n"); 
        __dumpMcastFlow(mcastFlow_p);
        /*dump 4ke flow */
        printk("------4keflow-------\n");
        fapPkt_printFlow(mcastFlow_p->flowHandle); 
        printk("\n========================================\n");
    }
}

static inline uint32_t hashMcastIpv4(fapMcast_mcastFlowTuple_t *mcastFlowTuple_p)
{
    uint32_t hashix;
    uint32_t hashSum;

    hashSum = mcastFlowTuple_p->nbrOfTags + mcastFlowTuple_p->ipDa4;

    hashix = _hash(hashSum);

    return hashix % FAP_MCAST_HTABLE_SIZE;
}

static inline uint32_t matchMcastIpv4(fapMcast_mcastFlowTuple_t *mcastFlowTuple_p,
                                     fapMcast_mcastFlow_t *mcastFlow_p)
{
    if( !(mcastFlow_p->tuple.flags.isIPv6) &&
	    (mcastFlow_p->tuple.rxPhyHdr == mcastFlowTuple_p->rxPhyHdr) &&
        (mcastFlow_p->tuple.rxChannel == mcastFlowTuple_p->rxChannel) &&
        (mcastFlow_p->tuple.nbrOfTags == mcastFlowTuple_p->nbrOfTags) &&
        (mcastFlow_p->tuple.vlanId.u32 == mcastFlowTuple_p->vlanId.u32) &&
        (mcastFlow_p->tuple.ipDa4 == mcastFlowTuple_p->ipDa4))
    {
        if(mcastFlow_p->tuple.flags.isSsm)
        {
            if(mcastFlow_p->tuple.ipSa4 == mcastFlowTuple_p->ipSa4)
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
static inline uint32_t hashMcastIpv6(fapMcast_mcastFlowTuple_t *mcastFlowTuple_p)
{
    uint32_t hashix;
    uint32_t hashSum;
    uint32_t *ipDa6_p; 


	ipDa6_p = mcastFlowTuple_p->ipDa6.p32;

    hashSum = mcastFlowTuple_p->nbrOfTags;
	hashSum += ipDa6_p[0] + ipDa6_p[1] +ipDa6_p[2] +ipDa6_p[3];

    hashix = _hash(hashSum);

    return hashix % FAP_MCAST_HTABLE_SIZE;
}

static inline uint32_t matchMcastIpv6(fapMcast_mcastFlowTuple_t *mcastFlowTuple_p,
                                     fapMcast_mcastFlow_t *mcastFlow_p)
{
	int i;

    if( (mcastFlow_p->tuple.flags.isIPv6) &&
	    (mcastFlow_p->tuple.rxPhyHdr == mcastFlowTuple_p->rxPhyHdr) &&
        (mcastFlow_p->tuple.rxChannel == mcastFlowTuple_p->rxChannel) &&
        (mcastFlow_p->tuple.nbrOfTags == mcastFlowTuple_p->nbrOfTags) &&
        (mcastFlow_p->tuple.vlanId.u32 == mcastFlowTuple_p->vlanId.u32))
    {

		for(i=0; i<4; i++)
		{
        	if(mcastFlow_p->tuple.ipDa6.p32[i] != mcastFlowTuple_p->ipDa6.p32[i]) 
			return 0;
		}

        // IMPORTANT NOTE: isSsm flag in mcastFlowTuple_p may not be reliable
        // as this was generated from the blog by buildMcastFlowTuple() function
        // based on whether the source address is non-zero. It is possible that 
        // Flowcache has learnt the source address and filled in the blog with a 
        // non-zero source address even though this is not a SSM flow. Hence use
		// isSsm flag in mcastFlow_p to determine whether this is a Ssm flow.
		if(mcastFlow_p->tuple.flags.isSsm)
		{
			for(i=0; i<4; i++)
			{
				if(mcastFlow_p->tuple.ipSa6.p32[i] != mcastFlowTuple_p->ipSa6.p32[i]) 
					return 0;
			}
			return 1;
		}
		else
		{
			return 1;
		}
	}

	return 0;
}

#endif

static PDll_t getMcastHashTable(fapMcast_mcastFlowTuple_t *mcastFlowTuple_p)
{
    uint32_t hashIx;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
	if(mcastFlowTuple_p->flags.isIPv6)
	{
    	hashIx = hashMcastIpv6(mcastFlowTuple_p);
	}
	else
#endif
	{
    	hashIx = hashMcastIpv4(mcastFlowTuple_p);
	}

    fapMcast_assert(hashIx < FAP_MCAST_HTABLE_SIZE);

    return (&fapMcast.mcastHashTable[hashIx]);
}

static fapMcast_mcastFlow_t *getMcastFlow(fapMcast_mcastFlowTuple_t *mcastFlowTuple_p,
                                            PDll_t mcastHashTable_p)
{
	PDll_t list_p;
	PDll_t node_p;
	PDll_t nextNode_p;
	fapMcast_mcastFlow_t *mcastFlow_p;

	list_p = mcastHashTable_p;

	for(node_p = dll_head_p(list_p);
			!dll_end(list_p, node_p);
			node_p = nextNode_p)
	{
		nextNode_p = dll_next_p(node_p);

		mcastFlow_p = (fapMcast_mcastFlow_t *)node_p;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
		if(mcastFlowTuple_p->flags.isIPv6)
		{
			if(matchMcastIpv6(mcastFlowTuple_p, mcastFlow_p))
			{
				return mcastFlow_p;
			}
		}
		else
#endif
		{
			if(matchMcastIpv4(mcastFlowTuple_p, mcastFlow_p))
			{
				return mcastFlow_p;
			}
		}
	}

	return NULL;
}

static fapMcast_mcastFlow_t *getMcastFlowByIndex(uint16_t flowIndex)
{
    uint32_t hashIx;
    PDll_t mcastHashTable_p;
    PDll_t list_p;
    PDll_t node_p;
    PDll_t nextNode_p;
    fapMcast_mcastFlow_t *mcastFlow_p;

    for(hashIx=0; hashIx<FAP_MCAST_HTABLE_SIZE; ++hashIx)
    {
        mcastHashTable_p = &fapMcast.mcastHashTable[hashIx];

        list_p = mcastHashTable_p;

        for(node_p = dll_head_p(list_p);
            !dll_end(list_p, node_p);
            node_p = nextNode_p)
        {
            nextNode_p = dll_next_p(node_p);

            mcastFlow_p = (fapMcast_mcastFlow_t *)node_p;

            if(mcastFlow_p->flowIndex == flowIndex)
            {
                return mcastFlow_p;
            }
        }
    }

    return NULL;
}

static fapMcast_mcastFlow_t *getMcastFlowByFlowHandle(fapPkt_flowHandle_t flowHandle)
{
    uint32_t hashIx;
    PDll_t mcastHashTable_p;
    PDll_t list_p;
    PDll_t node_p;
    PDll_t nextNode_p;
    fapMcast_mcastFlow_t *mcastFlow_p;

    for(hashIx=0; hashIx<FAP_MCAST_HTABLE_SIZE; ++hashIx)
    {
        mcastHashTable_p = &fapMcast.mcastHashTable[hashIx];

        list_p = mcastHashTable_p;

        for(node_p = dll_head_p(list_p);
            !dll_end(list_p, node_p);
            node_p = nextNode_p)
        {
            nextNode_p = dll_next_p(node_p);

            mcastFlow_p = (fapMcast_mcastFlow_t *)node_p;

            if(mcastFlow_p->flowHandle.u16 == flowHandle.u16)
            {
                return mcastFlow_p;
            }
        }
    }

    return NULL;
}

static McCfglog_t *alloc_mcCfglog(uint32 fapIdx)
{
	static int allocIndex =0;
	int i;
	McCfglog_t *mcCfglog_p;
	
	/*TODO maintain alloc index for each FAP separately */
	for(i=0; i< FAP_MAX_MCCFGLOGS; i++)
	{
		if(allocIndex == FAP_MAX_MCCFGLOGS)
			allocIndex = 0;

        mcCfglog_p = &(pHostFapSdram(fapIdx)->mcCfglogpool[allocIndex]);
        if(!mcCfglog_p->isAlloc)
        {
			memset(mcCfglog_p, 0, sizeof(McCfglog_t)); 
            mcCfglog_p->isAlloc = 1;
            mcCfglog_p->mccfgIdx = allocIndex;
			mcCfglog_p->hdrptr_noncached = mcCfglog_p->hdrBuf;
			allocIndex++;
			return mcCfglog_p;
        }
		allocIndex++;
	} 	
	return NULL;
}

static blogRuleAction_t *__findBlogRuleCommand(blogRule_t *blogRule_p,
                                               blogRuleCommand_t blogRuleCommand,
                                               uint32 *cmdIndex_p)
{
    blogRuleAction_t *action_p;
    int i;

    for(i=*cmdIndex_p; i<blogRule_p->actionCount; ++i)
    {
        action_p = &blogRule_p->action[i];

        if(action_p->cmd == blogRuleCommand)
        {
            *cmdIndex_p = i;

            return action_p;
        }
    }

    return NULL;
}

static inline blogRuleAction_t *findBlogRuleCommand(blogRule_t *blogRule_p,
                                                    blogRuleCommand_t blogRuleCommand)
{
    uint32 cmdIndex = 0;

    return __findBlogRuleCommand(blogRule_p, blogRuleCommand, &cmdIndex);
}

static void createFlowInfo(Blog_t *blog_p,
                           fapMcast_mcastFlow_t *mcastFlow_p,
                           fap4kePkt_flowInfo_t *flowInfo_p)
{
    /* set destination info */
    blogRuleAction_t *blogRuleAction_p = NULL;

    if(blog_p->tx.info.phyHdrType == BLOG_ENETPHY)
    {
        flowInfo_p->dest.phy = FAP4KE_PKT_PHY_ENET;
    }
    else
    {
        flowInfo_p->dest.phy = FAP4KE_PKT_PHY_XTM;
    }

    if ( blog_p->blogRule_p )
    {
        blogRuleAction_p = findBlogRuleCommand(blog_p->blogRule_p, BLOG_RULE_CMD_SET_SKB_MARK_QUEUE);
    }
    if(blogRuleAction_p)
    {
        flowInfo_p->dest.queue = blogRuleAction_p->arg;
    }
    else
    {
        flowInfo_p->dest.queue = (uint8_t)(SKBMARK_GET_Q_PRIO(blog_p->mark));
    }

    /* set source info */

    flowInfo_p->source.u32 = 0;

    if(mcastFlow_p->tuple.rxPhyHdr == BLOG_ENETPHY)
    {
        flowInfo_p->source.phy = FAP4KE_PKT_PHY_ENET;
    }
    else if (mcastFlow_p->tuple.rxPhyHdr == BLOG_GPONPHY)
    {
        flowInfo_p->source.phy = FAP4KE_PKT_PHY_GPON;
    }
    else
    {
        flowInfo_p->source.phy = FAP4KE_PKT_PHY_XTM;
    }

    flowInfo_p->source.nbrOfTags = mcastFlow_p->tuple.nbrOfTags;
    flowInfo_p->source.channel = mcastFlow_p->tuple.rxChannel;

    /* L4 protocol is set in the Blog Key for both IPv4 and IPv6 */
    flowInfo_p->source.protocol = blog_p->key.protocol;

    /* set ip tuple info */

    /* Note: "isRouted" flag is managed via FAP4KE_PKT_CMD_DECR_TTL */

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    if(mcastFlow_p->tuple.flags.isIPv6)
    {
        flowInfo_p->type = FAP4KE_PKT_FT_IPV6;
        flowInfo_p->isSsm = mcastFlow_p->tuple.flags.isSsm;
        flowInfo_p->ipv6.tuple.flags.learn=1;
        memcpy(flowInfo_p->ipv6.tuple.ipDa6.u8, mcastFlow_p->tuple.ipDa6.p8, sizeof(ip6_addr_t));
        memcpy(flowInfo_p->ipv6.tuple.ipSa6.u8, mcastFlow_p->tuple.ipSa6.p8, sizeof(ip6_addr_t));
        flowInfo_p->ipv6.tuple.vlanId.u32 = mcastFlow_p->tuple.vlanId.u32;

    }
    else
#endif
    {
        flowInfo_p->type = FAP4KE_PKT_FT_IPV4;
        flowInfo_p->isSsm = mcastFlow_p->tuple.flags.isSsm;
        flowInfo_p->ipv4.tuple.flags.learn=1;
        flowInfo_p->ipv4.tuple.ipDa4 = mcastFlow_p->tuple.ipDa4;
        flowInfo_p->ipv4.tuple.ipSa4 = mcastFlow_p->tuple.ipSa4;
        flowInfo_p->ipv4.tuple.vlanId.u32 = mcastFlow_p->tuple.vlanId.u32;
    }


    /* set ingress qos */

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    flowInfo_p->iq.prio = blog_p->iq_prio;
#endif
}

static void dumpLearnActions(fap4kePkt_learnAction_t *learnAction_p)
{
    uint32_t Idx, cmd;

    if(!bcmLog_logIsEnabled(BCM_LOG_ID_FAPPROTO, BCM_LOG_LEVEL_DEBUG))
    {
        return;
    }
    
	printk("\n========================================\n"); \
    printk("FAP4KE LEARN ACTIONS<%d>:\n", learnAction_p->cmdCount);
    if(learnAction_p->cmdCount)
    {
        for(Idx=0; Idx < learnAction_p->cmdCount; Idx++)
        {
            cmd = learnAction_p->cmd[Idx];

            if(cmd>FAP4KE_PKT_CMD_MAX)
                printk("Invalid learnAction<%d>\n", cmd);
            else
                printk("\t%s\n",learnActionName[cmd]);
        }
    }
    printk("LEARN ACTION ARGS:\n");
    printk("\tInsert Ethernet Hdr=%d\n", learnAction_p->cmdArg.insertEth); 
    printk("\tmacDa <%pM> macSa <%pM> etherType<%4x>\n", learnAction_p->cmdArg.macDa,
             learnAction_p->cmdArg.macSa, learnAction_p->cmdArg.etherType);

    for(Idx=0; Idx<FAP4KE_PKT_LEARN_MAX_VLAN_HEADERS; Idx++)
    {
        fap4kePkt_vlanHeader2_t *vlanHdr_p = &learnAction_p->cmdArg.vlanHdr[Idx];

        printk("\tVLAN HDR%d:\n", Idx);
        printk("\t\tTPID<%04x> VID<%d> DEI<%d> PBITS<%d>\n", vlanHdr_p->tpid,
                vlanHdr_p->tci.vid, vlanHdr_p->tci.dei, vlanHdr_p->tci.pbits);
    }
	printk("\n========================================\n"); \
}

static void createLearnActions(Blog_t *blog_p,
                               blogRule_t *blogRule_p,
                               fap4kePkt_learnAction_t *learnAction_p,
                               fap4kePkt_flowInfo_t *flowInfo_p)
{
    blogRuleAction_t *blogRuleAction_p;
    uint32_t cmdIndex;
    int numVlans;
    fap4kePkt_vlanHeader2_t *vlanHdr_p=NULL;
    uint8_t isSrcExtSWPort=0;
    uint8_t isDstExtSWPort=0;

    if((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_ENET) && (IsExternalSwitchPort(blog_p->tx.info.channel)) )
        isDstExtSWPort=1;
    if((flowInfo_p->source.phy == FAP4KE_PKT_PHY_ENET) && (IsExternalSwitchPort(blog_p->rx.info.channel)) )
        isSrcExtSWPort=1;

    /* Learn Rule actions MUST be specified in header sequence order */
    blogRuleAction_p = findBlogRuleCommand(blogRule_p, BLOG_RULE_CMD_DROP);
    if(blogRuleAction_p)
    {
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        if(flowInfo_p->type == FAP4KE_PKT_FT_IPV6)
        {
            flowInfo_p->ipv6.tuple.flags.drop = 1;
        }
        else
#endif
        {
            flowInfo_p->ipv4.tuple.flags.drop = 1;
        }

        return;
    }

    learnAction_p->cmdArg.insertEth = blog_p->insert_eth;

    blogRuleAction_p = findBlogRuleCommand(blogRule_p, BLOG_RULE_CMD_SET_MAC_DA);
    if(blogRuleAction_p)
    {
        if( blog_p->insert_eth )
        {
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_INSERT_MAC_DA;
        }
        else
        {
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_SET_MAC_DA;
        }

        memcpy(learnAction_p->cmdArg.macDa, blogRuleAction_p->macAddr, ETH_ALEN);
    }

    blogRuleAction_p = findBlogRuleCommand(blogRule_p, BLOG_RULE_CMD_SET_MAC_SA);
    if(blogRuleAction_p)
    {
        if( blog_p->insert_eth )
        {
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_INSERT_MAC_SA;
        }
        else
        {
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_SET_MAC_SA;
        }

        memcpy(learnAction_p->cmdArg.macSa, blogRuleAction_p->macAddr, ETH_ALEN);
    }
        if(isSrcExtSWPort)
        {
            /* pop BRCM2 tag when packet comes from external switch port */
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_POP_BRCM2_TAG;
        }

        if(isDstExtSWPort)
        {
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_PUSH_BRCM2_TAG;
        }
        
        //flowInfo_p->source.l2Type = FAP4KE_L2TYPE_IPOE;

        numVlans = 0;
        for(cmdIndex=0; cmdIndex < blogRule_p->actionCount; cmdIndex++)
        { 
            blogRuleAction_p = &blogRule_p->action[cmdIndex];

            switch( blogRuleAction_p->cmd)
            {
                case BLOG_RULE_CMD_POP_VLAN_HDR:
                    learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_POP_VLAN_HDR;
                    break;

                case BLOG_RULE_CMD_PUSH_VLAN_HDR:
                    vlanHdr_p = &learnAction_p->cmdArg.vlanHdr[numVlans];
                    vlanHdr_p->tpid = BCM_VLAN_DEFAULT_TAG_TPID;
                    vlanHdr_p->tci.vid = BCM_VLAN_DEFAULT_TAG_VID;
                    vlanHdr_p->tci.dei = 0;/*Todo fix this with proper macro*/
                    vlanHdr_p->tci.pbits = BCM_VLAN_DEFAULT_TAG_PBITS;
                    learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_PUSH_VLAN_HDR;
                    numVlans++;
                    break;

                case BLOG_RULE_CMD_SET_VLAN_PROTO:
                case BLOG_RULE_CMD_SET_VID:
                case BLOG_RULE_CMD_SET_DEI:
                case BLOG_RULE_CMD_SET_PBITS:
                    if( (!numVlans))
                    {
                        /*TODO fix this for multiple vlans on lan side, when there is no PUSH CMD 
                          how to differntitae the commands are for vlan0 or vlan1, what is the marker for
                          end of one vlan? ex: set_vid + set_dei for vlan 0 follwed by set_pbits for vlan1*/
                        vlanHdr_p = &learnAction_p->cmdArg.vlanHdr[numVlans];
                        learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_COPY_VLAN_HDR;
                        numVlans++;
                    }

                    if(blogRuleAction_p->cmd == BLOG_RULE_CMD_SET_VLAN_PROTO)
                    {
                        vlanHdr_p->tpid = blogRuleAction_p->arg;
                        learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_SET_VLAN_PROTO;

                    }
                    else if (blogRuleAction_p->cmd == BLOG_RULE_CMD_SET_VID)
                    {
                        vlanHdr_p->tci.vid = blogRuleAction_p->arg;
                        learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_SET_VID;

                    }
                    else if (blogRuleAction_p->cmd == BLOG_RULE_CMD_SET_DEI)
                    {
                        vlanHdr_p->tci.dei = blogRuleAction_p->arg;
                        learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_SET_DEI;

                    }
                    else if (blogRuleAction_p->cmd == BLOG_RULE_CMD_SET_PBITS)
                    {
                        vlanHdr_p->tci.pbits = blogRuleAction_p->arg;
                        learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_SET_PBITS;
                    }

                    break;

                default:
                    break;
            }
        }
        if( blog_p->insert_eth )
        {
           learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_INSERT_ETHERTYPE;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
           if(flowInfo_p->type == FAP4KE_PKT_FT_IPV6)
           {
              learnAction_p->cmdArg.etherType = ETH_P_IPV6;
           }
           else
#endif
           {
              learnAction_p->cmdArg.etherType = ETH_P_IP;
           }

           if(!blog_p->pop_pppoa)
              flowInfo_p->source.l2Type = FAP4KE_L2TYPE_IPOA;
        }
        else
        {
           flowInfo_p->source.l2Type = FAP4KE_L2TYPE_IPOE;
        }

    blogRuleAction_p = findBlogRuleCommand(blogRule_p, BLOG_RULE_CMD_POP_PPPOE_HDR);
    if(blogRuleAction_p)
    {
        if( blog_p->pop_pppoa )
        {
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_POP_PPPOA_HDR;
            flowInfo_p->source.l2Type = FAP4KE_L2TYPE_PPPOA;
        }
        else
        {
            learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_POP_PPPOE_HDR;
            flowInfo_p->source.l2Type = FAP4KE_L2TYPE_PPPOE;
        }
    }
    else if ( blog_p->has_pppoe )
    {
        flowInfo_p->source.l2Type = FAP4KE_L2TYPE_PPPOE;
    }

    blogRuleAction_p = findBlogRuleCommand(blogRule_p, BLOG_RULE_CMD_DECR_TTL);
	
    if(blogRuleAction_p)
    {
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
		/* TODO: FIX this properly by adding  dec hop limit blog rule */
		if(flowInfo_p->type == FAP4KE_PKT_FT_IPV6 )
		{
    		flowInfo_p->ipv6.tuple.flags.isRouted = 1;
		}
		else
#endif
        learnAction_p->cmd[learnAction_p->cmdCount++] = FAP4KE_PKT_CMD_DECR_TTL;
    }
    dumpLearnActions(learnAction_p);
    /* all remaining Blog Rule actions are NOT supported */
}

static int compareLearnAction(fap4kePkt_learnAction_t *lA1_p, fap4kePkt_learnAction_t *lA2_p)
{
    int i;
    int numVlans=0;
    fap4kePkt_vlanHeader2_t *vlanHdr1_p=NULL;
    fap4kePkt_vlanHeader2_t *vlanHdr2_p=NULL;

    if(lA1_p->cmdCount == lA2_p->cmdCount)
    {
        for(i=0; i<lA1_p->cmdCount; i++)
        {
            if(lA1_p->cmd[i] == lA2_p->cmd[i])
            {
                switch (lA1_p->cmd[i])
                {
                    case FAP4KE_PKT_CMD_SET_MAC_DA:
                    case FAP4KE_PKT_CMD_INSERT_MAC_DA:
                        if(memcmp(lA1_p->cmdArg.macDa,
                                    lA2_p->cmdArg.macDa, ETH_ALEN))
                            return 0;

                        break;

                    case FAP4KE_PKT_CMD_SET_MAC_SA:
                    case FAP4KE_PKT_CMD_INSERT_MAC_SA:
                        if(memcmp(lA1_p->cmdArg.macSa,
                                    lA2_p->cmdArg.macSa, ETH_ALEN))
                            return 0;

                        break;

                    case FAP4KE_PKT_CMD_INSERT_ETHERTYPE:
                        if(lA1_p->cmdArg.etherType !=
                                lA2_p->cmdArg.etherType)
                            return 0;

                        break;

                    case FAP4KE_PKT_CMD_COPY_VLAN_HDR:
                    case FAP4KE_PKT_CMD_PUSH_VLAN_HDR:
                        vlanHdr1_p = &lA1_p->cmdArg.vlanHdr[numVlans];
                        vlanHdr2_p = &lA2_p->cmdArg.vlanHdr[numVlans];
                        numVlans++;
                        if(memcmp(vlanHdr1_p, vlanHdr2_p, sizeof(fap4kePkt_vlanHeader2_t)))
                            return 0;
                        break;

                    case FAP4KE_PKT_CMD_SET_VLAN_PROTO:
                        if(vlanHdr1_p->tpid != vlanHdr2_p->tpid)
                            return 0;
                        break;

                    case FAP4KE_PKT_CMD_SET_VID:
                        if(vlanHdr1_p->tci.vid != vlanHdr2_p->tci.vid)
                            return 0;
                        break;

                    case FAP4KE_PKT_CMD_SET_DEI:
                        if(vlanHdr1_p->tci.dei != vlanHdr2_p->tci.dei)
                            return 0;
                        break;
                    case FAP4KE_PKT_CMD_SET_PBITS:
                        if(vlanHdr1_p->tci.pbits != vlanHdr2_p->tci.pbits)
                            return 0;
                        break;

                    default:
                        break;
                }  
            }
            else
            {
                /*not a match*/
                return 0;
            }
        }
        /*match */
        return 1;
    }
    else
    {
        return 0;
    }
}

static int mcCfglog_findTxDev(McCfglog_t *mcCfglog_p, void *txDev_p)
{
    int i;

    if(!txDev_p) 
    {
        fapMcast_error("txDev_p is NULL\n");
    }

    for(i=0; i<FAP_MCCFGLOG_MAX_TXDEVS; i++)
    {
        if(mcCfglog_p->txDev_pp[i] == txDev_p)
        {
            /* txDev_p exists return index*/
            return i;
        }
    } 
    /* match not found */
    return -1;
}
static int mcCfglog_addTxDev(McCfglog_t *mcCfglog_p, void *txDev_p)
{
    int i;

    if(!txDev_p) 
    {
        fapMcast_error("txDev_p is NULL\n");
    }

    for(i=0; i<FAP_MCCFGLOG_MAX_TXDEVS; i++)
    {
        if(!mcCfglog_p->txDev_pp[i])
        {
            /*empty slot found add new txDev_p */
            mcCfglog_p->txDev_pp[i] = txDev_p;
            mcCfglog_p->nTxDevs++;
            return i;
        }
    } 
    /* no emty slots found */
    fapMcast_error("Cannot Add txDev_p to mcCfglog, no empty slot\n");
    return -1;
}

static int mcCfglog_delTxDev(McCfglog_t *mcCfglog_p, void *txDev_p)
{
    int index;

    if(!txDev_p) 
    {
        fapMcast_error("txDev_p is NULL\n");
    }

    index = mcCfglog_findTxDev(mcCfglog_p,txDev_p);
    if(index < 0 )    
    {
        fapMcast_error("Cannot find txDev_p=%p in mcCfglog=%p\n",txDev_p, mcCfglog_p);
        return -1;
    }
    else
    {
        /*found txDev_p set corresponding entry to null */
        mcCfglog_p->txDev_pp[index] = NULL;
        mcCfglog_p->nTxDevs--;
        return index;
    }
}



static int createMcastFlows(Blog_t *blog_p,
                            fapMcast_mcastFlow_t *mcastFlow_p,
                            fap4kePkt_flowInfo_t *flowInfo_p)
{
    int ret = FAP_SUCCESS;
    blogRule_t *blogRule_p;
    fap4kePkt_learnAction_t learnAction;
    uint32 fapIdx;
    McCfglog_t *mcCfglog_p;
    fapPkt_flowHandle_t flowHandle;

    memset(&learnAction, 0, sizeof(fap4kePkt_learnAction_t));
    memset(flowInfo_p, 0, sizeof(fap4kePkt_flowInfo_t));

    /* set classification and forwarding information */
    createFlowInfo(blog_p, mcastFlow_p, flowInfo_p);

    /* create FAP Learn Actions */

    blogRule_p = (blogRule_t *)blog_p->blogRule_p;

    if(blogRule_p != NULL)
    {
        fapMcast_dumpBlogRule(blogRule_p);

        if(blogRule_p->next_p != NULL)
        {
            blogRule_t * blognextRule_p = blogRule_p->next_p;

            fapMcast_error("More than one Blog Rule specified!");

            while(blognextRule_p)
            {
                fapMcast_dumpBlogRule(blognextRule_p);
                blognextRule_p = blognextRule_p->next_p;
            }

            ret = FAP_ERROR;
            goto out;
        }
        /* create learn actions */
        createLearnActions(blog_p, blogRule_p, &learnAction, flowInfo_p);
    }

    if(blog_p->mcast_learn)
    {
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        if(blog_p->rx.info.bmap.PLD_IPv6)
        {
            if(blog_p->rx.tuple.tos != PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0))
            {
                flowInfo_p->ipv6.tuple.flags.mangleTos = 1;
                flowInfo_p->ipv6.tuple.flags.tos = PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0);

                fapMcast_debug("Mangle IPv6 Traffic Class %u", flowInfo_p->ipv6.tuple.flags.tos);
            }
        }
        else 
#endif
        {
            if(blog_p->rx.tuple.tos != blog_p->tx.tuple.tos)
            {
                flowInfo_p->ipv4.tuple.flags.mangleTos = 1;
                flowInfo_p->ipv4.tuple.flags.tos = blog_p->tx.tuple.tos;

                fapMcast_debug("Mangle IPv4 ToS %u", flowInfo_p->ipv4.tuple.flags.tos);
            }
        }
    }

    /* Determine which FAP to send the flow to */
    if(blog_p->rx.info.bmap.BCM_XPHY)
        fapIdx = getFapIdxFromXtmRxPort(blog_p->rx.info.channel);
    else if (blog_p->rx.info.phyHdrType == BLOG_GPONPHY) 
        fapIdx = getFapIdxForGPONPort();
    else
        fapIdx = getFapIdxFromEthRxPort(blog_p->rx.info.channel);

    if (pHostPsmGbl(fapIdx)->mtuOverride)
        flowInfo_p->fapMtu = pHostPsmGbl(fapIdx)->mtuOverride;
    else
        flowInfo_p->fapMtu = blog_getTxMtu(blog_p);

    /*first allocate mccfgLog in FAP SDRAM */
    mcCfglog_p = alloc_mcCfglog(fapIdx);

    if(mcCfglog_p == NULL )
    {
        fapMcast_error("mcCfglog Allocation failed !");
        ret= FAP_ERROR;
        goto out;
    }

    mcCfglog_p->learnAction = learnAction;
    fapMcast_assert(blog_p->tx.info.channel < MAX_TOTAL_SWITCH_PORTS);
    mcCfglog_p->portmask = (1 << LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));
    mcCfglog_p->isextSWPort = mcCfglog_p->isextSWPortonIntSW = IsExternalSwitchPort(blog_p->tx.info.channel);
    mcCfglog_addTxDev(mcCfglog_p,blog_p->tx_dev_p);
    mcCfglog_p->destIpAddr = blog_p->tx.tuple.daddr;

    /* FAP Traffic Management Info */
    mcCfglog_p->virtDestPort = blog_p->tx.info.channel;
    /* External Switch Port */
    flowInfo_p->extSwTagLen = sizeof(uint32_t);

    /* Configure Excluded UDP Destination Port.
       UDP exclusion only supported when mcast learning is disabled */
    if (!mcastFlow_p->tuple.flags.isIPv6)
    {
        flowInfo_p->ipv4.tuple.excludeDestPort = blog_p->mcast_excl_udp_port;
    }

    /* activate flow in the FAP */
    flowHandle = fapPkt_mcastActivate(fapIdx, flowInfo_p, NULL, NULL, mcCfglog_p->mccfgIdx);

    if(flowHandle.u16 == FAP4KE_PKT_INVALID_FLOWHANDLE)
    {
        ret = FAP_ERROR;
        /*free mcCfglog */
        mcCfglog_p->isAlloc=0;
        goto out;
    }

    mcastFlow_p->flowHandle = flowHandle;

    mcCfglog_p->next =mcastFlow_p->mcCfgloglist;
    mcastFlow_p->mcCfgloglist = mcCfglog_p;
    mcastFlow_p->nClients =1;

out:
    return ret;
}

static uint16_t allocFlowIndex(void)
{
    uint16_t flowIndex;

    flowIndex = fapMcast.flowIndexAlloc;

    if(++fapMcast.flowIndexAlloc == FAP4KE_PKT_MAX_FLOWS)
    {
        /* roll over at FAP4KE_PKT_MAX_FLOWS */
        fapMcast.flowIndexAlloc = 0;
    }

    return FAP_HW_TUPLE(FAP_HW_ENGINE_MCAST, flowIndex);
}

static inline void buildMcastFlowTuple(Blog_t *blog_p,
                                       fapMcast_mcastFlowTuple_t *mcastFlowTuple_p)
{
	mcastFlowTuple_p->rxPhyHdr = blog_p->rx.info.phyHdr;
	mcastFlowTuple_p->rxChannel = blog_p->rx.info.channel;
	mcastFlowTuple_p->nbrOfTags = blog_p->vtag_num;
	mcastFlowTuple_p->vlanId.u32 = ((blog_p->vtag[1] & 0xFFFF) <<16) | (blog_p->vtag[0] & 0xFFFF);
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
	if( blog_p->rx.info.bmap.PLD_IPv6 )
	{
		mcastFlowTuple_p->flags.isIPv6 =1;

		memcpy(mcastFlowTuple_p->ipDa6.p8, blog_p->tupleV6.daddr.p8, sizeof(ip6_addr_t));

		/* SSM multicasts will have a non-zero source address */
		if (blog_p->is_ssm)
		{
			mcastFlowTuple_p->flags.isSsm = 1;
			memcpy(mcastFlowTuple_p->ipSa6.p8, blog_p->tupleV6.saddr.p8, sizeof(ip6_addr_t));
		}
		else
		{
			mcastFlowTuple_p->flags.isSsm = 0;
			memset(mcastFlowTuple_p->ipSa6.p8, 0, sizeof(ip6_addr_t));
		}

	}
	else
#endif
	{
		mcastFlowTuple_p->flags.isIPv6 =0;
		mcastFlowTuple_p->ipDa4 = blog_p->rx.tuple.daddr;

		/* SSM multicasts will have a non-zero source address */
		if(blog_p->is_ssm)
		{
			mcastFlowTuple_p->flags.isSsm = 1;
			mcastFlowTuple_p->ipSa4 = blog_p->rx.tuple.saddr;
		}
		else
		{
			mcastFlowTuple_p->flags.isSsm = 0;
			mcastFlowTuple_p->ipSa4 = 0;
		}
	}
}



static int syncMcastFlow(Blog_t *blog_p,
                         fapMcast_mcastFlow_t *mcastFlow_p,
                         fap4kePkt_flowInfo_t *flowInfo_p)
{
    fapPkt_flowHandle_t prevFlowHandle;

    /* save existing flow allocation, we will free later */
    prevFlowHandle = mcastFlow_p->flowHandle;

    mcastFlow_p->flowHandle.u16 = FAP4KE_PKT_INVALID_FLOWHANDLE;

    /* allocate the new FAP flows */
    createMcastFlows(blog_p, mcastFlow_p, flowInfo_p);

    /* De-allocate the previous FAP flow, regardless if allocation
       of the new flow was succesfull or not. If the new flow could
       not be allocated in the FAP it will be handled in the Host
       MIPS by Flow Cache */
    return fapPkt_deactivate(prevFlowHandle);
}

static int updateFlowDestPortMask(fapMcast_mcastFlow_t *mcastFlow_p, uint32 msgType, uint32 newMtu, uint32 mcCfglogIdx)
{
    fap4kePkt_flowInfo_t flowInfo;
    int ret;

    ret = fapPkt_getFlowInfo(mcastFlow_p->flowHandle, &flowInfo);
    if (ret == FAP_ERROR)
    {
        fapMcast_error("Cannot fapPkt_getFlowInfo, error=%d", ret);
        goto out;
    }

    if (newMtu && (newMtu < flowInfo.fapMtu))
        flowInfo.fapMtu = newMtu;

    ret = fapPkt_mcastUpdate(mcastFlow_p->flowHandle, msgType, &flowInfo, mcCfglogIdx);
    if (ret == FAP_ERROR)
    {
        fapMcast_error("Cannot fapPkt_setFlowInfo, error=%d", ret);
        goto out;
    }
out:
    return ret;
}

static void blog_tupleV6_dump( BlogTupleV6_t * bTupleV6_p )
{
    printk( "\tIPv6:\n"
            "\t\tSrc" BLOG_IPV6_ADDR_PORT_FMT "\n"
            "\t\tDst" BLOG_IPV6_ADDR_PORT_FMT "\n"
            "\t\thop_limit<%3u>\n",
            BLOG_IPV6_ADDR(bTupleV6_p->saddr), bTupleV6_p->port.source,
            BLOG_IPV6_ADDR(bTupleV6_p->daddr), bTupleV6_p->port.dest,
            bTupleV6_p->rx_hop_limit );
}


int fapMcast_activate(Blog_t *blog_p,
                      fap4kePkt_flowInfo_t *flowInfo_p,
                      int *isActivation_p)
{
    int ret = FAP_ERROR;
    fapMcast_mcastFlowTuple_t mcastFlowTuple;
    PDll_t mcastHashTable_p;
    fapMcast_mcastFlow_t *mcastFlow_p;

    fapMcast_assert(blog_p);

    fapMcast_info("ACTIVATE");
    fapMcast_dumpBlog(blog_p);

    *isActivation_p = 0;

    if(!(FAP4KE_PKT_IS_MCAST_IPV4(blog_p->rx.tuple.daddr) || FAP4KE_PKT_IS_MCAST_IPV6(blog_p->tupleV6.daddr.p8[0])))
    {
        fapMcast_error("Not IPv4 or IPv6 Multicast : " IP4DDN,
                         IP4(blog_p->rx.tuple.daddr));
        blog_tupleV6_dump(&blog_p->tupleV6);

        goto out;
    }

    buildMcastFlowTuple(blog_p, &mcastFlowTuple);

    /* try to locate the flow */
    mcastHashTable_p = getMcastHashTable(&mcastFlowTuple);

    mcastFlow_p = getMcastFlow(&mcastFlowTuple, mcastHashTable_p);

    if(blog_p->mc_sync)
    {
        fapMcast_info("SYNC :dev=%pS >>>>", blog_p->tx_dev_p);

        if(mcastFlow_p == NULL)
        {
            fapMcast_error("Cannot sync unexisting Mcast Flow");

            goto out;
        }

        ret = syncMcastFlow(blog_p, mcastFlow_p, flowInfo_p);

        if(ret != FAP_SUCCESS)
        {
            fapMcast_error("Cannot syncMcastFlow, error=%d", ret);
            /* free FAP allocations */
            fapPkt_deactivate(mcastFlow_p->flowHandle);
            /* remove mcast flow from the hash table */
            dll_delete((PDll_t)mcastFlow_p);

            /* free mcast flow */
            mcastFlowFree(mcastFlow_p);
            goto out;
        }

        fapMcast_info("SYNC : SUCCESSFUL <<<<");
    }
    else if(mcastFlow_p == NULL)
    {
        *isActivation_p = 1;

        /* allocate a new flow */

        fapMcast_info("CREATE :dev=%pS >>>>", blog_p->tx_dev_p);

        mcastFlow_p = mcastFlowAlloc();
        if(mcastFlow_p == NULL)
        {
            fapMcast_error("Cannot Allocate Flow");
            goto out;
        }

        /* initialize flow */
        memset(mcastFlow_p, 0, sizeof(fapMcast_mcastFlow_t));
        mcastFlow_p->flowHandle.u16 = FAP4KE_PKT_INVALID_FLOWHANDLE;

        mcastFlow_p->tuple = mcastFlowTuple;

        ret = createMcastFlows(blog_p, mcastFlow_p, flowInfo_p);
        if (ret != FAP_SUCCESS)
        {
            if(mcastFlow_p->flowHandle.u16 != FAP4KE_PKT_INVALID_FLOWHANDLE)
            {
                /* free FAP allocations */
                fapPkt_deactivate(mcastFlow_p->flowHandle);
            }
            /* free mcast flow */
            mcastFlowFree(mcastFlow_p);

            goto out;
        }

        mcastFlow_p->flowIndex = allocFlowIndex();

        /* add mcast flow to the hash table */
        dll_append(mcastHashTable_p, (PDll_t)mcastFlow_p);

        fapMcast_info("CREATE : SUCCESSFUL <<<<");
    }
    else /* mcast flow already exists */
    {
        fap4kePkt_learnAction_t learnAction;
        blogRule_t *blogRule_p;
        McCfglog_t *mcCfglog_p;

        fapMcast_info("ADD DEST PORT :dev=%pS >>>>", blog_p->tx_dev_p);

        ret = fapPkt_getFlowInfo(mcastFlow_p->flowHandle, flowInfo_p);

        if (ret == FAP_ERROR)
        {
            fapMcast_error("Cannot fapPkt_getFlowInfo, error=%d", ret);
            goto out;
        }

        /* we have to check for 2 things
         * 1. Check if a client for this txdev already exists
         * 2. Check if we have any client with same modifications
         */

        mcCfglog_p = mcastFlow_p->mcCfgloglist;
        while(mcCfglog_p)
        {
            if(mcCfglog_findTxDev(mcCfglog_p, blog_p->tx_dev_p) >=0)
            {
                /* destination port already exists - return index */
                fapMcast_info("Destination dev %pS already exists", blog_p->tx_dev_p);
                ret = mcastFlow_p->flowIndex;
                goto out;
            }
            mcCfglog_p = mcCfglog_p->next;
        }

        /* create FAP Learn Actions */
        blogRule_p = (blogRule_t *)blog_p->blogRule_p;

        if(blogRule_p != NULL)
        {
            fapMcast_dumpBlogRule(blogRule_p);

            if(blogRule_p->next_p != NULL)
            {
                blogRule_t * blognextRule_p = blogRule_p->next_p;

                /* currently we expect only one blog rule */
                fapMcast_error("More than one Blog Rule specified!");

                while(blognextRule_p)
                {
                    fapMcast_dumpBlogRule(blognextRule_p);
                    blognextRule_p = blognextRule_p->next_p;
                }

                ret = FAP_ERROR;
                goto out;
            }
            memset(&learnAction, 0, sizeof(fap4kePkt_learnAction_t));
            /* create learn actions */
            createLearnActions(blog_p, blogRule_p, &learnAction, flowInfo_p);
        }

        if(!fapMcast.useIudma2Bd)
        {
            mcCfglog_p = mcastFlow_p->mcCfgloglist;

            while(mcCfglog_p)
            {
                /* Check if a client with same modificationis exist, if we find one
                 * just update the portMask and switch will replicate the packets
                 */
                if(compareLearnAction(&learnAction, &mcCfglog_p->learnAction))
                {
                    fapMcast_info("Found a mclog with same modifications, just update portmask");
                    /* found a match*/
                    break;
                }
                mcCfglog_p = mcCfglog_p->next;
            }

            if(mcCfglog_p)
            {
                /* we have a match just update the port mask in mcCfglog*/ 
                if(mcCfglog_addTxDev(mcCfglog_p,blog_p->tx_dev_p) < 0)
                {
                    ret = FAP_ERROR;
                    goto out;
                } 

                fapMcast_assert(blog_p->tx.info.channel < MAX_TOTAL_SWITCH_PORTS);
                mcCfglog_p->portmask |= (1 << LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));

                /* currently assuming MTU of all the devices in a mcCfglog_p will be equal*/
                ret = updateFlowDestPortMask(mcastFlow_p, FAP_MSG_FLW_MCAST_UPDATE_CLIENT, blog_getTxMtu(blog_p), mcCfglog_p->mccfgIdx);
                if (ret == FAP_ERROR)
                {
                    mcCfglog_delTxDev(mcCfglog_p,blog_p->tx_dev_p);
                    fapMcast_error("Failed to update mclog");
                    goto out;
                }
                mcastFlow_p->nClients++;
                ret = mcastFlow_p->flowIndex;

                fapMcast_info("ADD DESTPORT : SUCCESSFUL <<<<");
                dumpMcastFlow(mcastFlow_p);
                goto out;
            }
            else
            {
                /* cannot support clients with diffrent modifications in FAP
                 * evict the flow from FAP
                 */
                mcCfglog_p = mcastFlow_p->mcCfgloglist;
                fapMcast_info("Egress modifications not supported in FAP,Evicting flow");
                dumpMcastFlow(mcastFlow_p);
                /* send msg to 4ke to de-allocate flow */
                fapPkt_mcastDeactivate(mcastFlow_p->flowHandle, mcCfglog_p->mccfgIdx);
                /* remove mcast flow from the hash table */
                dll_delete((PDll_t)mcastFlow_p);

                mcastFlow_p->mcCfgloglist = NULL;
                /* free mcast flow */
                mcastFlowFree(mcastFlow_p);

                update_Fapdeactivates();
                /*decrement the failures as we dont want this case to be notifed as
                 * failure */ 
                decrement_Fapfailures();
                ret = FAP_ERROR;
                goto out;
            }
        }

        fapMcast_info("Adding a new mclog to flow");
        /*first allocate a new mccfgLog in FAP SDRAM */
        mcCfglog_p = alloc_mcCfglog(mcastFlow_p->flowHandle.fapIdx);

        if(mcCfglog_p == NULL )
        {
            fapMcast_error("mcCfglog Allocation failed !");
            ret= FAP_ERROR;
            goto out;
        }

        mcCfglog_p->learnAction = learnAction;
        mcCfglog_addTxDev(mcCfglog_p,blog_p->tx_dev_p);

        fapMcast_assert(blog_p->tx.info.channel < MAX_TOTAL_SWITCH_PORTS);
        mcCfglog_p->portmask = (1 << LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));
        mcCfglog_p->isextSWPortonIntSW = mcCfglog_p->isextSWPort = IsExternalSwitchPort(blog_p->tx.info.channel);

        /* FAP Traffic Management Info */
        mcCfglog_p->virtDestPort = blog_p->tx.info.channel;
        /* External Switch Port */
        flowInfo_p->extSwTagLen = sizeof(uint32_t);

        /* Add dest port to the FAP flow associated with the Multicast Flow */

        ret = updateFlowDestPortMask(mcastFlow_p, FAP_MSG_FLW_MCAST_ADD_CLIENT, blog_getTxMtu(blog_p), mcCfglog_p->mccfgIdx);
        if (ret == FAP_ERROR)
        {
            mcCfglog_p->isAlloc=0;
        	fapMcast_error("Failed to add Mcast client to existing flow");
            goto out;
        }
        /*Add mcCfglog_p to mcCfgloglist */
        mcCfglog_p->next =mcastFlow_p->mcCfgloglist;
        mcastFlow_p->mcCfgloglist = mcCfglog_p;

        mcastFlow_p->nClients++;

        fapMcast_info("ADD DESTPORT : SUCCESSFUL <<<<");
    }
    dumpMcastFlow(mcastFlow_p);
    /* if we reach here, the activation succeeded */
    ret = mcastFlow_p->flowIndex;

out:
    return ret;
}

int fapMcast_deactivate(Blog_t *blog_p, int *isDeactivation_p)
{
    fapMcast_mcastFlowTuple_t mcastFlowTuple;
    PDll_t mcastHashTable_p;
    fapMcast_mcastFlow_t *mcastFlow_p;
    McCfglog_t *mcCfglog_p;
    McCfglog_t *prev_mcCfglog_p;
    int ret = FAP_SUCCESS;
    int nClients = 0;

    fapMcast_assert(blog_p != NULL);

    fapMcast_info("DEACTIVATE");
    fapMcast_dumpBlog(blog_p);

    *isDeactivation_p = 0;

    buildMcastFlowTuple(blog_p, &mcastFlowTuple);

    /* try to locate the flow */
    mcastHashTable_p = getMcastHashTable(&mcastFlowTuple);

    mcastFlow_p = getMcastFlow(&mcastFlowTuple, mcastHashTable_p);

    if(mcastFlow_p == NULL)
    {
        fapMcast_error("Cannot getMcastFlow");
        goto out;
    }

    prev_mcCfglog_p = mcastFlow_p->mcCfgloglist;
    mcCfglog_p = mcastFlow_p->mcCfgloglist;

    while(mcCfglog_p)
    {
        if(mcCfglog_findTxDev(mcCfglog_p, blog_p->tx_dev_p) >=0)
        {
            break;
        }

        prev_mcCfglog_p = mcCfglog_p;
        mcCfglog_p = mcCfglog_p->next;
    }

    if(!mcCfglog_p)
    {
        fapMcast_error("mcCfglog does not exist for destination dev:%p",
                blog_p->tx_dev_p);
        nClients = mcastFlow_p->nClients;
        goto out;
    }

    /* remove port from mcast flow */
    if(mcastFlow_p->nClients == 1)
    {
        *isDeactivation_p = 1;
        nClients =0;

        fapMcast_info("DELETE FLOW: >>>>");
        dumpMcastFlow(mcastFlow_p);

        /* the last user left the group, lets de-allocate all flows */
        fapPkt_mcastDeactivate(mcastFlow_p->flowHandle, mcCfglog_p->mccfgIdx);

        /* remove mcast flow from the hash table */
        dll_delete((PDll_t)mcastFlow_p);

        mcastFlow_p->mcCfgloglist = NULL;
        /* free mcast flow */
        mcastFlowFree(mcastFlow_p);

        fapMcast_info("DELETE FLOW: SUCCESSFUL <<<<");
    }
    else
    {
        fapMcast_info("REM DESTPORT: dev=%pS >>>>", blog_p->tx_dev_p);
        mcastFlow_p->nClients--;
        nClients = mcastFlow_p->nClients;

        /* Remove dest port from FAP flow associated with the Multicast Flow */
        if(mcCfglog_delTxDev(mcCfglog_p, blog_p->tx_dev_p) < 0)
        {
            ret = FAP_ERROR;
            goto out;
        }

        if(mcCfglog_p->nTxDevs)
        {
            fapMcast_info("Deleting txdev from mclog");
            /*update port mask */
            fapMcast_assert(blog_p->tx.info.channel < MAX_TOTAL_SWITCH_PORTS);
            mcCfglog_p->portmask &= ~(1 << LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));
            ret = updateFlowDestPortMask(mcastFlow_p, FAP_MSG_FLW_MCAST_UPDATE_CLIENT, 0, mcCfglog_p->mccfgIdx);
        }
        else
        {
            fapMcast_info("Deleting mclog from flow");
            /*remove mcCfglog_p from mcCfgloglist */
            if(mcCfglog_p == mcastFlow_p->mcCfgloglist)
                mcastFlow_p->mcCfgloglist = mcCfglog_p->next;
            else
                prev_mcCfglog_p->next = mcCfglog_p->next;

            /* TBD: note: if remove entry with lowest mtu, mtu will not go up
               as a result of this call... Maybe we should recalculate here? */
            ret = updateFlowDestPortMask(mcastFlow_p, FAP_MSG_FLW_MCAST_DEL_CLIENT, 0, mcCfglog_p->mccfgIdx);
        }
        if (ret == FAP_ERROR)
        {
            goto out;
        }

        fapMcast_info("REM DESTPORT : SUCCESSFUL <<<<");
        dumpMcastFlow(mcastFlow_p);
    }

out:
    /* return number of associations to the mcast flow */
    return nClients;
}

int fapMcast_deactivateAll(void)
{
    fapMcast_mcastFlow_t *mcastFlow_p;
    McCfglog_t *mcCfglog_p;
    uint16_t flowIndex, fapIdx;
    int count = 0;
    fapPkt_flowHandle_t flowHandle;

    for(fapIdx = FAP0_IDX; fapIdx < FAP0_IDX+NUM_FAPS; fapIdx++)
    {
        for (flowIndex=0; flowIndex < FAP4KE_PKT_MAX_FLOWS; flowIndex++)
        { 
            flowHandle.u16 = 0;
            flowHandle.fapIdx = fapIdx;
            flowHandle.flowId = flowIndex;

            /* try to locate the flow */
            mcastFlow_p = getMcastFlowByFlowHandle(flowHandle);

            if(mcastFlow_p == NULL)
            {
                continue;
            }

            mcCfglog_p = mcastFlow_p->mcCfgloglist;

            fapMcast_info("DELETE FLOW: >>>>");

            /* remove mcast flow from the hash table */
            dll_delete((PDll_t)mcastFlow_p);

            mcastFlow_p->mcCfgloglist = NULL;
            /* free mcast flow */
            mcastFlowFree(mcastFlow_p);

            fapMcast_info("DELETE FLOW: SUCCESSFUL <<<<");

            count++;
        }
    }

    fapMcast.flowIndexAlloc = 0;

    /* return number of associations to the mcast flow */
    return count;
}


unsigned int fapMcast_refresh(uint16_t fhwTuple,
                              uint32_t *pktCount_p,
                              uint32_t *octCount_p)
{
    fapMcast_mcastFlow_t *mcastFlow_p;
    fap4kePkt_flowStats_t *flowStats_p;
    uint32_t hitCount;
    uint32_t octCount;

    hitCount = 0;
    octCount = 0;

    mcastFlow_p = getMcastFlowByIndex(fhwTuple);
    if(mcastFlow_p == NULL)
    {
        fapMcast_error("Cannot getMcastFlowByIndex = %d", fhwTuple);
        dump_stack();
    }
    else
    {
        flowStats_p = fapPkt_getFlowStats(mcastFlow_p->flowHandle);

        if(flowStats_p == NULL)
        {
            fapMcast_error("Cannot fapPkt_getFlowStats");

            goto out;
        }

        /* calculate the total hit count for the Multicast flow */
        hitCount = flowStats_p->hits;
        octCount = flowStats_p->bytes;
    }

    *pktCount_p = hitCount;
    *octCount_p = octCount;

out:
    return hitCount;
}

unsigned int fapMcast_resetStats(uint16_t cmfTuple16)
{
    fapMcast_mcastFlow_t *mcastFlow_p;

    mcastFlow_p = getMcastFlowByIndex(cmfTuple16);
    if(mcastFlow_p == NULL)
    {
        fapMcast_error("Cannot getMcastFlowByIndex");
        dump_stack();
    }
    else
    {
        fapPkt_resetStats(mcastFlow_p->flowHandle);
    }

    return 0;
}

uint32 fapMcast_getDestPortMask(fapPkt_flowHandle_t flowHandle)
{
    fapMcast_mcastFlow_t *mcastFlow_p = getMcastFlowByFlowHandle(flowHandle);

    if(mcastFlow_p == NULL)
    {
        fapMcast_error("Cannot getMcastFlowByFlowHandle");

        dump_stack();
    }
    else
    {
        McCfglog_t *mcCfglog_p = mcastFlow_p->mcCfgloglist;
        uint32 destPortMask = 0;

        while(mcCfglog_p)
        {
            destPortMask |= mcCfglog_p->portmask;

            mcCfglog_p = mcCfglog_p->next;
        }

        return destPortMask;
    }

    return 0;
}

void fapMcast_destruct(void)
{
    if(fapMcast.mcastFlowCache)
    {
        kmem_cache_destroy(fapMcast.mcastFlowCache);
        fapMcast.mcastFlowCache = NULL;
    }
}

int __init fapMcast_construct(void)
{
    int ret = 0;
    int i;

    /* reset the main control structure */
    memset(&fapMcast, 0, sizeof(fapMcast_t));

    setUseIudma2BD();
    /* initialize hash tables */
    for(i=0; i<FAP_MCAST_HTABLE_SIZE; ++i)
    {
        dll_init(&fapMcast.mcastHashTable[i]);
    }

    /* Create a slab cache for the Mcast Flows */
    fapMcast.mcastFlowCache = kmem_cache_create("fapMcast_flowCache",
                                                sizeof(fapMcast_mcastFlow_t),
                                                0, /* align */
                                                SLAB_HWCACHE_ALIGN, /* flags */
                                                NULL); /* ctor */
    if(fapMcast.mcastFlowCache == NULL)
    {
        fapMcast_error("Unable to create Mcast Flow Cache\n");

        fapMcast_destruct();

        ret = -ENOMEM;
        goto out;
    }

out:
    return ret;
}
