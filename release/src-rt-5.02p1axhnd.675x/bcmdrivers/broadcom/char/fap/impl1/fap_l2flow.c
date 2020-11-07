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
 * File Name  : fap_l2flow.c
 *
 * Description: This implementation supports the layer 2 flow acceleration.
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
#include "fap_l2flow.h"
#include "fap4ke_iopDma.h"
#include "bcmnet.h"             /* SKBMARK_GET_Q_PRIO */
#include "bcmxtmrt.h"
#include "bcmPktDma.h"
#include "bcm_vlan.h"

#if defined(CONFIG_BCM_FAP_LAYER2)

//#define CC_FAP_L2FLOW_DEBUG

#if defined(CC_FAP_L2FLOW_DEBUG)
#define fapL2flow_print(fmt, arg...) printk(fmt, ##arg)
#define fapL2flow_dumpBlogRule(_blogRule_p)                            \
    do {printk("\n===============================================================\n"); \
        blog_rule_dump(_blogRule_p); printk("\n");} while(0)
#define fapL2flow_dumpFlow(_flowHandle) fapPkt_printFlow(_flowHandle)
#define fapL2flow_dumpBlog(blog_p) blog_dump(blog_p)
#else
#define fapL2flow_print(fmt, arg...)
#define fapL2flow_dumpBlogRule(_blogRule_p)
#define fapL2flow_dumpFlow(_flowHandle)
#define fapL2flow_dumpBlog(blog_p) 
#endif

#define fapL2flow_debug(fmt, arg...) BCM_LOG_DEBUG(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define fapL2flow_info(fmt, arg...) BCM_LOG_INFO(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define fapL2flow_error(fmt, arg...) BCM_LOG_ERROR(BCM_LOG_ID_FAPPROTO, fmt, ##arg) 
#define fapL2flow_assert(condition)  BCM_ASSERT(condition)


/*
 * Global Variables
 */
static int fapL2flow_defaultVlanTag_g = 0;


/*
 *------------------------------------------------------------------------------
 * Function:
 *   void fapL2flow_createFlowInfo(Blog_t *blog_p, fap4kePkt_flowInfo_t *flowInfo_p)
 *
 * Description:
 *   Fills in the layer 2 flow info based on the blog info.
 *
 * Parameter:
 *   blog_p (input): pointer to a blog
 *   flowInfo_p (output): pointer to flow info
 *------------------------------------------------------------------------------
 */
static void fapL2flow_createFlowInfo(Blog_t *blog_p, fap4kePkt_flowInfo_t *flowInfo_p)
{
   blogRule_t           *blogRule_p       = (blogRule_t *)(blog_p->blogRule_p);
   blogRuleFilterVlan_t *ruleFilterVlan_p = NULL;
   blogRuleFilterEth_t  *ruleFilterEth_p  = &blogRule_p->filter.eth;
   blogRuleFilterIpv4_t *ruleFilterIpv4_p = &blogRule_p->filter.ipv4;
   blogRuleAction_t     *ruleAction_p     = NULL;
   
   fap4kePkt_l2TupleFilters_t    *flowFilter_p     = &flowInfo_p->l2.tuple.filters;
   fap4kePkt_l2TupleActions_t    *flowAction_p     = &flowInfo_p->l2.tuple.actions;
   fap4kePkt_l2TupleVlanAction_t *flowActionVlan_p = NULL;
   
   uint32_t totalVlanTags = blogRule_p->filter.nbrOfVlanTags; 
   uint8_t  cmd;
   int      i;

   fapL2flow_assert(blog_p->tx.info.channel < FAP4KE_PKT_MAX_DEST_PORTS);
   fapL2flow_assert(FAP4KE_PKT_L2_MAX_VLAN_HEADERS == 2);
   fapL2flow_assert(totalVlanTags <= FAP4KE_PKT_L2_MAX_VLAN_HEADERS);

   /* set destination info */
   flowInfo_p->dest.u32 = 0;

   if(blog_p->tx.info.phyHdrType == BLOG_ENETPHY)
      flowInfo_p->dest.phy = FAP4KE_PKT_PHY_ENET;
   else
      flowInfo_p->dest.phy = FAP4KE_PKT_PHY_XTM;
   flowInfo_p->dest.channel = (1 << blog_p->tx.info.channel);

#if defined(CC_FAP4KE_TM)
   fapTm_setFlowInfo(flowInfo_p, (1 << blog_p->tx.info.channel));
#endif

   /* set source info */
   flowInfo_p->source.u32 = 0;

   if(blog_p->rx.info.phyHdr == BLOG_ENETPHY)
      flowInfo_p->source.phy = FAP4KE_PKT_PHY_ENET;
   else
      flowInfo_p->source.phy = FAP4KE_PKT_PHY_XTM;

   flowInfo_p->type = FAP4KE_PKT_FT_L2;
   flowInfo_p->source.isLayer2  = 1;
   flowInfo_p->source.nbrOfTags = blog_p->vtag_num;
   flowInfo_p->source.channel   = blog_p->rx.info.channel;

   /* set l2 tuple filter info */
   for(i = 0; i < totalVlanTags; ++i)
   {
      ruleFilterVlan_p = &blogRule_p->filter.vlan[i];

      if(ruleFilterVlan_p->mask.h_vlan_TCI & BLOG_RULE_PBITS_MASK)
      {
         if (i == 0)
            flowFilter_p->ctrl.v0_pbits = 1;
         else
            flowFilter_p->ctrl.v1_pbits = 1;
      }
      if(ruleFilterVlan_p->mask.h_vlan_TCI & BLOG_RULE_DEI_MASK)
      {
         if (i == 0)
            flowFilter_p->ctrl.v0_dei = 1;
         else
            flowFilter_p->ctrl.v1_dei = 1;
      }
      if(ruleFilterVlan_p->mask.h_vlan_TCI & BLOG_RULE_VID_MASK)
      {
         if (i == 0)
            flowFilter_p->ctrl.v0_vid = 1;
         else
            flowFilter_p->ctrl.v1_vid = 1;
      }
       
      if(flowFilter_p->ctrl.u16)
      {
         flowFilter_p->vlan[i].tci.u16 = ruleFilterVlan_p->value.h_vlan_TCI;
      }
   }
   
   if(blog_rule_filterInUse(ruleFilterEth_p->mask.h_proto))
   {
      flowFilter_p->ctrl.etherType = 1;
      flowFilter_p->misc.etherType = ruleFilterEth_p->value.h_proto;
   }
   if(blog_rule_filterInUse(ruleFilterIpv4_p->mask.tos))
   {
      flowFilter_p->ctrl.tos = 1;
      flowFilter_p->misc.tos = ruleFilterIpv4_p->value.tos;
   }
   if(blog_rule_filterInUse(ruleFilterIpv4_p->mask.ip_proto))
   {
      flowFilter_p->ctrl.ipProtocol = 1;
      flowFilter_p->misc.ipProtocol = ruleFilterIpv4_p->value.ip_proto;
   }
      
   /* set l2 tuple action info */
   for(i = 0; i < blogRule_p->actionCount; ++i)
   {
      ruleAction_p = &(blogRule_p->action[i]);
      cmd          = ruleAction_p->cmd;
      
      fapL2flow_assert(ruleAction_p->toTag < FAP4KE_PKT_L2_MAX_VLAN_HEADERS);
            
      switch(cmd)
      {
         case BLOG_RULE_CMD_PUSH_VLAN_HDR:
            flowInfo_p->txAdjust -= sizeof(fap4kePkt_vlanHeader_t);
            totalVlanTags++;
            
            flowAction_p->ctrl.v0_tag = 1;
            
            flowActionVlan_p = &(flowAction_p->vlan[0]);
            flowActionVlan_p->tag.tpid     = BCM_VLAN_DEFAULT_TAG_TPID;
            flowActionVlan_p->tag.tci.vid  = BCM_VLAN_DEFAULT_TAG_VID;
            flowActionVlan_p->tciMask.u16 |= VLAN_TCI_PBITS_MASK | VLAN_TCI_DEI_MASK | VLAN_TCI_VID_MASK;
            break;
            
         case BLOG_RULE_CMD_POP_VLAN_HDR:
            flowInfo_p->txAdjust += sizeof(fap4kePkt_vlanHeader_t);
            totalVlanTags--;
            break;
            
         case BLOG_RULE_CMD_SET_ETHERTYPE:
         case BLOG_RULE_CMD_SET_VLAN_PROTO:
         case BLOG_RULE_CMD_SET_PBITS:
         case BLOG_RULE_CMD_SET_DEI:
         case BLOG_RULE_CMD_SET_VID:
            if(totalVlanTags <= ruleAction_p->toTag)
            {
               /* error */
               fapL2flow_error("Setting non-exist VLAN tag %d. Number of VLAN tags = %d!",
                               ruleAction_p->toTag, totalVlanTags);
            }
            else if(ruleAction_p->arg)
            {
               flowActionVlan_p = &(flowAction_p->vlan[ruleAction_p->toTag]);
               
               if(cmd == BLOG_RULE_CMD_SET_VID)
               {
                  flowActionVlan_p->tag.tci.vid  = ruleAction_p->arg;
                  flowActionVlan_p->tciMask.u16 |= VLAN_TCI_VID_MASK;
               }
               else if(cmd == BLOG_RULE_CMD_SET_PBITS)
               {
                  flowActionVlan_p->tag.tci.pbits = ruleAction_p->arg;
                  flowActionVlan_p->tciMask.u16  |= VLAN_TCI_PBITS_MASK;
               }
               else if(cmd == BLOG_RULE_CMD_SET_DEI)
               {
                  flowActionVlan_p->tag.tci.dei  = ruleAction_p->arg;
                  flowActionVlan_p->tciMask.u16 |= VLAN_TCI_DEI_MASK;
               }
               else  /* set tpid */
               {
                  flowActionVlan_p->tag.tpid = ruleAction_p->arg;
               }
            }
            break;

         case BLOG_RULE_CMD_SET_DSCP:
            flowAction_p->ctrl.tos = 1;
            flowAction_p->tos      = ruleAction_p->arg;
            break;
            
         case BLOG_RULE_CMD_DROP:
            flowInfo_p->dest.drop = 1;
            break;
            
         case BLOG_RULE_CMD_SET_SKB_MARK_QUEUE:
            flowInfo_p->dest.queue = ruleAction_p->arg;
            break;

         case BLOG_RULE_CMD_OVRD_LEARNING_VID:
            flowAction_p->ovrdLearningVid = 1;
            flowAction_p->learnVlanId = ruleAction_p->vid;
            break;
            
         default:
            break;
      }
   }  /* for */

   for(i = 0; (i < 2) && (i < FAP4KE_PKT_L2_MAX_VLAN_HEADERS); i++)
   {   
      flowActionVlan_p = &(flowAction_p->vlan[i]);
      if(flowActionVlan_p->tag.tpid && flowActionVlan_p->tciMask.u16)
      {
         if(i == 0)
            flowAction_p->ctrl.v0_tag = 1;
         else
            flowAction_p->ctrl.v1_tag = 1;
      }
      else if(flowActionVlan_p->tag.tpid)
      {
         if(i == 0)
            flowAction_p->ctrl.v0_tpid = 1;
         else
            flowAction_p->ctrl.v1_tpid = 1;
      }
      else if(flowActionVlan_p->tag.tci.u16)
      {
         if(i == 0)
            flowAction_p->ctrl.v0_tci = 1;
         else
            flowAction_p->ctrl.v1_tci = 1;
      }
   }

   {
       fap4kePkt_l2TupleFiltersCtrl_t revIvlFilters = { .u16 = 0 };
       fap4kePkt_l2TupleActionsCtrl_t revIvlActions = { .u8 = 0 };

       revIvlFilters.v0_vid = 1;
       revIvlActions.v0_tci = 1;

       if((flowFilter_p->ctrl.u16 == revIvlFilters.u16) &&
          (flowAction_p->ctrl.u8 == revIvlActions.u8) &&
          (flowAction_p->vlan[0].tciMask.u16 == VLAN_TCI_VID_MASK) &&
          (flowInfo_p->source.nbrOfTags == totalVlanTags))
       {
           /* enable reverse ivl for this flow */
           flowAction_p->ctrl.revIvl = 1;
       }
   }

   /* set ingress qos */

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
   flowInfo_p->iq.prio = blog_p->iq_prio;
#endif
   
}  /* fapL2flow_createFlowInfo() */


/*
 *------------------------------------------------------------------------------
 * Function:
 *   int fapL2flow_removeDefaultVlanTag(Blog_t *blog_p)
 *
 * Description:
 *   Modifies existing Blog Rule to deal with the default switch tag.
 *
 * Parameters:
 *   blog_p (input): pointer to the layer 2 flow blog
 *
 * Returns:
 *   FAP_SUCCESS:  succeeded
 *   FAP_ERROR: failed
 *------------------------------------------------------------------------------
 */
static int fapL2flow_removeDefaultVlanTag(Blog_t *blog_p)
{
    blogRule_t *blogRule_p = blog_p->blogRule_p;
    blogRuleFilter_t *filter_p = &blogRule_p->filter;

    if(filter_p->nbrOfVlanTags == 0)
    {
        int ret;
        blogRuleAction_t ruleAction;

        filter_p->nbrOfVlanTags = blog_p->vtag_num = 1;

        memset(&filter_p->vlan[0], 0, sizeof(blogRuleFilterVlan_t));
        filter_p->vlan[0].mask.h_vlan_TCI = BLOG_RULE_VID_MASK;
        filter_p->vlan[0].value.h_vlan_TCI = 0xFFF;

        memset(&ruleAction, 0, sizeof(blogRuleAction_t));
        ruleAction.cmd = BLOG_RULE_CMD_POP_VLAN_HDR;
        ret = blog_rule_add_action(blogRule_p, &ruleAction);
        if(ret)
        {
            return FAP_ERROR;
        }
    }

    return FAP_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function:
 *   int fapL2flow_defaultVlanTagConfig(int enable)
 *
 * Description:
 *   Enable/Disable Switch Default Tag
 *
 * Parameters:
 *   enable (input): 0: Disable, 1: Enable.
 *
 * Returns:
 *   FAP_SUCCESS:  succeeded
 *   FAP_ERROR: failed
 *------------------------------------------------------------------------------
 */
void fapL2flow_defaultVlanTagConfig(int enable)
{
    fapL2flow_defaultVlanTag_g = enable;
}


/*
 *------------------------------------------------------------------------------
 * Function:
 *   int fapL2flow_activate(Blog_t *blog_p)
 *
 * Description:
 *   Activate a layer2 flow.
 *
 * Parameters:
 *   blog_p (input): pointer to the layer 2 flow blog
 *
 * Returns:
 *   flow handle:  succeeded
 *   FAP_ERROR: failed
 *------------------------------------------------------------------------------
 */
int fapL2flow_activate(Blog_t *blog_p)
{
   int ret = FAP_SUCCESS;
   blogRule_t *blogRule_p;
   fap4kePkt_flowInfo_t flowInfo;
   fapPkt_flowHandle_t flowHandle;
   uint32 fapIdx;
   

   fapL2flow_assert(blog_p);

   fapL2flow_debug("ACTIVATE");
//   fapL2flow_dumpBlog(blog_p);    /* calling this function causes kernel crash??? */

   blogRule_p = (blogRule_t *)blog_p->blogRule_p;
   if(blogRule_p == NULL)
   {
      fapL2flow_debug("blogRule_p is NULL. No activation.");
      ret = FAP_ERROR;
      goto out;
   }
   
   fapL2flow_dumpBlogRule(blogRule_p);

   if(blogRule_p->next_p != NULL)
   {
      blogRule_t *blognextRule_p = blogRule_p->next_p;

      fapL2flow_error("More than one Blog Rule specified!");

      while(blognextRule_p)
      {
         fapL2flow_dumpBlogRule(blognextRule_p);
         blognextRule_p = blognextRule_p->next_p;
      }

      ret = FAP_ERROR;
      goto out;
   }

   if(blogRule_p->filter.flags == BLOG_RULE_FILTER_FLAGS_IS_MULTICAST)
   {
       /* This is a Multicast only rule, do not activate it.
          Only Unicast or Broadcast rules are needed in the FAP bridge */

       fapL2flow_debug("fapL2flow_activate: Multicast Blog Rule rejected!\n");

       ret = FAP_ERROR;
       goto out;
   }

   /* DM_TBD: FIX... fap4kePkt_flowInfoSizeL2  --  flowInfo points to sdram array for now, so it doesn't kill us */
   memset(&flowInfo, 0, sizeof(fap4kePkt_flowInfo_t));

   flowInfo.type = FAP4KE_PKT_FT_L2;

   if(fapL2flow_defaultVlanTag_g)
   {
       ret = fapL2flow_removeDefaultVlanTag(blog_p);
       if(ret != FAP_SUCCESS)
       {
           goto out;
       }
   }

   /* set classification and forwarding information */
   fapL2flow_createFlowInfo(blog_p, &flowInfo);

   /* Determine which FAP to send the flow to */
   if(blog_p->rx.info.bmap.BCM_XPHY)
      fapIdx = getFapIdxFromXtmRxPort(blog_p->rx.info.channel);
   else
      fapIdx = getFapIdxFromEthRxPort(blog_p->rx.info.channel);

   /* activate flow in the FAP */
   flowHandle = fapPkt_activate(fapIdx, &flowInfo, NULL, 0, NULL, NULL, NULL);
   if(flowHandle.u16 == FAP4KE_PKT_INVALID_FLOWHANDLE)
   {
      ret = FAP_ERROR;
      goto out;
   }

   /* return flow handle of blog rule */
   ret = FAP_HW_TUPLE(FAP_HW_ENGINE_L2FLOW, flowHandle.u16);
   
out:
   return ret;
    
}  /* fapL2flow_activate() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   int fapL2flow_deactivate(fapPkt_flowHandle_t flowHandle)
 *
 * Description:
 *   Deactivate a layer2 flow.
 *
 * Parameters:
 *   flowHandle (input): flow handle of the layer 2 flow
 *
 * Returns:
 *   FAP_SUCCESS
 *------------------------------------------------------------------------------
 */
int fapL2flow_deactivate(fapPkt_flowHandle_t flowHandle)
{
   fapL2flow_debug("DELETE");

   fapPkt_deactivate(flowHandle);

   return FAP_SUCCESS;

}  /* fapL2flow_deactivate() */

#endif   /* CONFIG_BCM_FAP_LAYER2 */
