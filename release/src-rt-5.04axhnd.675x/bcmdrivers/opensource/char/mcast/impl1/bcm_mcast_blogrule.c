/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
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
* :>
*/

#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/blog_rule.h>
#include "bcm_mcast_blogrule.h"

extern int bcm_mcast_activate_blog(bcm_mcast_ifdata *pif,
                                                  blogRule_t       *rule_p,
                                                  void             *mc_fdb,
                                                  int               proto,
                                                  void             *arg_p,
                                                  uintptr_t         flowhdl);

static void bcm_mcast_add_wlan_blog_rule(blogRule_t        *rule_p,
                                         void              *mc_fdb,
                                         int                proto)
{
#if defined(CONFIG_BR_IGMP_SNOOP)
    t_igmp_grp_entry *igmp_fdb = NULL;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
    t_mld_grp_entry  *mld_fdb  = NULL;
#endif
#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
    int phyType;
#endif
    struct net_device *dev_p;

    dev_p = mc_fdb_get_dstdev(mc_fdb, proto);
    do
    {
 #if defined(CONFIG_BR_IGMP_SNOOP)
        if (BCM_MCAST_PROTO_IPV4 == proto )
        {
            igmp_fdb = (t_igmp_grp_entry *)mc_fdb;
            break;
        }
 #endif
 #if defined(CONFIG_BR_MLD_SNOOP)
        if (BCM_MCAST_PROTO_IPV6 == proto )
        {
            mld_fdb = (t_mld_grp_entry *)mc_fdb;
            break;
        }
 #endif
       /* invalid protocol */
       __logError("invalid protocol specified");
       return;
    } while (0);


#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
    phyType = netdev_path_get_hw_port_type(dev_p);
    if ( BLOG_GET_PHYTYPE(phyType) == BLOG_WLANPHY )
    {
        BlogRnr_t *blogRnr = NULL;
#if defined(CONFIG_BR_IGMP_SNOOP)
        if ( igmp_fdb )
        {
            blogRnr = (BlogRnr_t *)&igmp_fdb->info;
        }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
        if ( mld_fdb )
        {
            blogRnr = (BlogRnr_t *)&mld_fdb->info;
        }
#endif
        if ( blogRnr && (0 == blogRnr->is_wfd) && (1 == blogRnr->is_tx_hw_acc_en))
        {
            blogRuleAction_t ruleAction;
            memset(&ruleAction, 0, sizeof(blogRuleAction_t));
            ruleAction.cmd = BLOG_RULE_CMD_SET_STA_MAC_ADDRESS;
#if defined(CONFIG_BR_IGMP_SNOOP)
            if(igmp_fdb)
            {
                /* can only be one reporter for this forwarding entry */
                t_igmp_rep_entry *rep;

                rep = list_first_entry(&igmp_fdb->rep_list,
                                       t_igmp_rep_entry,
                                       list);
                memcpy(ruleAction.macAddr, rep->repMac, ETH_ALEN);
                blog_rule_add_action(rule_p, &ruleAction);
            }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
            if (mld_fdb)
            {
                /* can only be one reporter for this forwarding entry */
                t_mld_rep_entry *rep;

                rep = list_first_entry(&mld_fdb->rep_list,
                                       t_mld_rep_entry,
                                       list);
                memcpy(ruleAction.macAddr, rep->repMac, ETH_ALEN);
                blog_rule_add_action(rule_p, &ruleAction);
            }
#endif
        }
    }
#endif
}

uint16_t bcm_mcast_blogrule_fetch_lan_tci(blogRule_t *rule_p)
{
    while( rule_p )
    {
        int actionIndex = 0;
        for( ; actionIndex < rule_p->actionCount ; actionIndex++)
        {
            if (rule_p->action[actionIndex].cmd == BLOG_RULE_CMD_SET_VID)
            {
                return rule_p->action[actionIndex].vid;
            }
        }
        rule_p = rule_p->next_p;
    }
    return 0;
}

void bcm_mcast_blogrule_get_vlan_info(blogRule_t *rule_p, 
                                      uint8_t    *numtags, 
                                      uint32_t   *vlan0, 
                                      uint32_t   *vlan1)
{
    blogRuleFilter_t  *rule_filter = NULL;
    uint32_t vid;

    rule_filter = &(rule_p->filter);
    *numtags = rule_filter->nbrOfVlanTags;
    if (rule_filter->vlan[0].mask.h_vlan_TCI)
    {
        vid = ((rule_filter->vlan[0].value.h_vlan_TCI &
            rule_filter->vlan[0].mask.h_vlan_TCI) & 0xFFF);
    }
    else
    {
        vid = 0xFFF;
    }
    *vlan0 = vid; 
    if (rule_filter->vlan[1].mask.h_vlan_TCI)
    {
        vid = ((rule_filter->vlan[1].value.h_vlan_TCI &
            rule_filter->vlan[1].mask.h_vlan_TCI) & 0xFFF);
    }
    else
    {
        vid = 0xFFF;
    }
    *vlan1 = vid;
}
static int bcm_mcast_blog_rule_count(blogRule_t *rule_p)
{
    blogRule_t *temp_rule_p = rule_p;
    int count = 0;

    while ( temp_rule_p )
    {
        count++;
        temp_rule_p = temp_rule_p->next_p;
    }
    return count;
}


int bcm_mcast_blogrule_vlan_process(bcm_mcast_ifdata  *pif,
                                    void              *mc_fdb,
                                    int                proto,
                                    int                blogProto,
                                    blogRule_t       **list_rule_p,
                                    struct hlist_head *headMcHash,
                                    void              *arg_p,
                                    uintptr_t          flowhdl)
{
    int         activates = 0;
    blogRule_t *rule_p = *list_rule_p;
    uint32_t    pre_vid = 0;
    uint32_t    vlan0;
    uint32_t    vlan1;
    uint8_t     vtag_num;

    __logDebug("ENTER rule_p 0x%px", rule_p);
    if(!mc_fdb || !rule_p || !pif)
    {
        __logError("mc_fdb 0x%px rule_p 0x%px pif 0x%px", mc_fdb, rule_p, pif);
        return 0;
    }

    if(BCM_MCAST_PROTO_IPV4 == proto)
    {
        pre_vid = ((t_igmp_grp_entry *)mc_fdb)->wan_tci;
    }
    else if(BCM_MCAST_PROTO_IPV6 == proto)
    {
        pre_vid = ((t_mld_grp_entry *)mc_fdb)->wan_tci;
    }
    else 
    {
        return 0;
    }

    __logDebug("blogrule count %d grp 0x%x dstdev %s srcdev %s num_tags %d",
              bcm_mcast_blog_rule_count(rule_p),
              htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr), 
              ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
              ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
              ((t_igmp_grp_entry *)mc_fdb)->num_tags);

    while( rule_p )
    {
        blogRuleFilter_t *filter_p;

        filter_p = &rule_p->filter;

        /* if there is a rule that specifies a protocol filter that does not match
           blog key protocol skip it */
#if defined(CONFIG_BR_IGMP_SNOOP)
        if(blog_rule_filterInUse(filter_p->ipv4.mask.ip_proto))
        {
            if(filter_p->ipv4.mask.ip_proto & BLOG_RULE_IP_PROTO_MASK)
            {
                uint8_t proto;

                proto = filter_p->ipv4.value.ip_proto >> BLOG_RULE_IP_PROTO_SHIFT;
                if (proto != blogProto)
                {
                    /* skip this rule */
                    __logDebug("proto in blog rule %d different from proto in blog %d",
                               proto, blogProto);
                    *list_rule_p = rule_p->next_p;
                    blog_rule_free(rule_p);
                    rule_p = *list_rule_p;
                    continue;
                }
            }
        }
#endif

#if defined(CONFIG_BR_MLD_SNOOP)
        if(blog_rule_filterInUse(filter_p->ipv6.mask.nxtHdr))
        {
            if(filter_p->ipv6.mask.nxtHdr & BLOG_RULE_IP6_NXT_HDR_MASK)
            {
                uint8_t nxtHdr;

                nxtHdr = filter_p->ipv6.value.nxtHdr >> BLOG_RULE_IP6_NXT_HDR_SHIFT;
                if (nxtHdr != blogProto)
                {
                    /* skip this rule */
                    __logDebug("nxtHdr in blog rule %d different from nxtHdr in blog %d",
                               nxtHdr, blogProto);
                    *list_rule_p = rule_p->next_p;
                    blog_rule_free(rule_p);
                    rule_p = *list_rule_p;
                    continue;
                }
            }
        }
#endif

        /* it is configured by host control, check if the vlan of blog rule is expected */
        if(pre_vid != BCM_MCAST_INVALID_VID) 
        {
            bcm_mcast_blogrule_get_vlan_info(rule_p, &vtag_num, &vlan0, &vlan1);

            if(pre_vid != vlan0)
            {
                __logDebug("previd 0x%x != vlan 0 0x%x", pre_vid, vlan0);
               /* skip this rule */
                *list_rule_p = rule_p->next_p;
                blog_rule_free(rule_p);
                rule_p = *list_rule_p;
                continue;
            }
            __logDebug("previd 0x%x == vlan 0 0x%x", pre_vid, vlan0);
        }
        else
        {
            __logDebug("previd 0x%x == BCM_MCAST_INVALID_VID 0x%x", pre_vid, BCM_MCAST_INVALID_VID);
        }

        if (bcm_mcast_activate_blog(pif, 
                                    rule_p,
                                    mc_fdb,
                                    proto, 
                                    arg_p,
                                    flowhdl) )
        {
            __logError("Unable to create flow, continue");
        }
        else
            activates++;

        /* advance to the next rule */
        rule_p = *list_rule_p;
    }

    /* Free blog. The blog will only have rules if there was an error */
    blog_rule_free_list(*list_rule_p);

    __logDebug("EXIT activates %d", activates);
    return activates;
} /* bcm_mcast_blogrule_vlan_process */

static blogRuleAction_t *bcm_mcast_blogrule_find_command(blogRule_t *blogRule_p,
                                                         blogRuleCommand_t blogRuleCommand,
                                                         uint32_t *cmdIndex_p)
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

void bcm_mcast_blogrule_process_wan(blogRule_t         *rule_p,
                                    void               *mc_fdb,
                                    int                 proto)
{
   blogRuleAction_t   ruleAction;
   struct net_device *dev_p = NULL;
#if defined(CONFIG_BR_IGMP_SNOOP)
   t_igmp_grp_entry  *igmp_fdb = NULL;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   t_mld_grp_entry   *mld_fdb = NULL;
#endif 
   uint8_t           *dev_addr = NULL;
   uint32_t           phyType;
   char               wan_ops;
   uint32_t           index = 0;
   struct net_device *wan_dev_p;

   if(NULL == mc_fdb)
   {
      return;
   }

   do
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
      if ( BCM_MCAST_PROTO_IPV4 == proto )
      {
         igmp_fdb = (t_igmp_grp_entry *)mc_fdb;
         dev_p    = igmp_fdb->from_dev;
         dev_addr = igmp_fdb->dst_dev->dev_addr;
         wan_ops  = igmp_fdb->type;
         break;
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if(BCM_MCAST_PROTO_IPV6 == proto)
      {
         mld_fdb  = (t_mld_grp_entry *)mc_fdb;
         dev_p    = mld_fdb->from_dev;
         dev_addr = mld_fdb->dst_dev->dev_addr;
         wan_ops  = mld_fdb->type;
         break;
      }
#endif
      __logError("invalid protocol specified");
      return;
   } while (0);

   while(1)
   {
      if(netdev_path_is_root(dev_p))
      {
         wan_dev_p = dev_p;
         break;
      }

      if(is_netdev_ppp(dev_p))
      {
         rule_p->filter.hasPppoeHeader = 1;
         memset(&ruleAction, 0, sizeof(blogRuleAction_t));
         ruleAction.cmd = BLOG_RULE_CMD_POP_PPPOE_HDR;
         blog_rule_add_action(rule_p, &ruleAction);

         if ( NULL == bcm_mcast_blogrule_find_command(rule_p, 
                                                      BLOG_RULE_CMD_SET_MAC_DA, 
                                                      &index) )
         {
            memset(&ruleAction, 0, sizeof(blogRuleAction_t));
            ruleAction.cmd = BLOG_RULE_CMD_SET_MAC_DA;
#if defined(CONFIG_BR_IGMP_SNOOP)
            if(igmp_fdb)
            {
               ip_eth_mc_map(igmp_fdb->txGrp.s_addr, (char *)&ruleAction.macAddr[0]);
            }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
            if ( mld_fdb )
            {
               ipv6_eth_mc_map(&mld_fdb->grp, (char *)&ruleAction.macAddr[0]);
            }
#endif
            blog_rule_add_action(rule_p, &ruleAction);
         }
      }

      dev_p = netdev_path_next_dev(dev_p);
   }

   /* For IPoA */
   phyType = netdev_path_get_hw_port_type(wan_dev_p);
   phyType = BLOG_GET_HW_ACT(phyType);
   if((phyType == VC_MUX_IPOA) || (phyType == LLC_SNAP_ROUTE_IP))
   {
      if ( NULL == bcm_mcast_blogrule_find_command(rule_p, 
                                                   BLOG_RULE_CMD_SET_MAC_DA, 
                                                   &index) )
      {
         memset(&ruleAction, 0, sizeof(blogRuleAction_t));
         ruleAction.cmd = BLOG_RULE_CMD_SET_MAC_DA;
#if defined(CONFIG_BR_IGMP_SNOOP)
         if(igmp_fdb)
         {
            ip_eth_mc_map(igmp_fdb->txGrp.s_addr, (char *)&ruleAction.macAddr[0]);
         }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
         if ( mld_fdb )
         {
            ipv6_eth_mc_map(&mld_fdb->grp, (char *)&ruleAction.macAddr[0]);
         }
#endif
         blog_rule_add_action(rule_p, &ruleAction);
      }
   }

   if(wan_ops == BCM_MCAST_IF_ROUTED)
   {
      memset(&ruleAction, 0, sizeof(blogRuleAction_t));
      ruleAction.cmd = BLOG_RULE_CMD_SET_MAC_SA;
      memcpy(ruleAction.macAddr, dev_addr, ETH_ALEN);
      blog_rule_add_action(rule_p, &ruleAction);

      memset(&ruleAction, 0, sizeof(blogRuleAction_t));
      ruleAction.cmd = BLOG_RULE_CMD_DECR_TTL;
      blog_rule_add_action(rule_p, &ruleAction);
   }
}

int bcm_mcast_blogrule_process(bcm_mcast_ifdata  *pif,
                               void              *mc_fdb,
                               int                proto,
                               int                blogProto,
                               blogRule_t       **list_rule_p,
                               struct hlist_head *headMcHash,
                               void              *arg_p,
                               int                lan2lan,
                               struct net_device *wan_vlan_dev_p,
                               struct net_device *lan_vlan_dev_p,
                               uintptr_t          flowhdl)
{
    __logDebug("ENTER");

    /* allocate blog rule */
    *list_rule_p = blog_rule_alloc();
    if(*list_rule_p == NULL)
    {
       __logError("blog rule allocation failure");
       return -1;
    }

    blog_rule_init(*list_rule_p);

     __logInfo("wan_vlan_dev_p %s lan_vlan_dev_p %s", 
               wan_vlan_dev_p ? wan_vlan_dev_p->name:"NULL", lan_vlan_dev_p ? lan_vlan_dev_p->name:"NULL");

    /* add vlan blog rules, if any vlan interfaces were found */
    if(blogRuleVlanHook && (wan_vlan_dev_p || lan_vlan_dev_p)) 
    {
        if(blogRuleVlanHook(arg_p, wan_vlan_dev_p, lan_vlan_dev_p) < 0) 
        {
            __logError("blogRuleVlanHook error");
            blog_rule_free_list(*list_rule_p);
            return -1;
        }
    }

    /* blog must have at least one rule */
    if (NULL == *list_rule_p)
    {
        /* blogRule_p == NULL may be valid if there are no 
           VLAN rules and the default behavior for either interface is DROP */
        __logDebug("blogRuleVlanHook no blog rules returned");
        return -1;
    }

    bcm_mcast_add_wlan_blog_rule(*list_rule_p, 
                                 mc_fdb, 
                                 proto);

    if (!lan2lan) 
    {
        bcm_mcast_blogrule_process_wan(*list_rule_p, 
                                       mc_fdb, 
                                       proto);
    }

    return bcm_mcast_blogrule_vlan_process(pif, 
                                           mc_fdb, 
                                           proto, 
                                           blogProto,
                                           list_rule_p,
                                           headMcHash,
                                           arg_p,
                                           flowhdl);
}


