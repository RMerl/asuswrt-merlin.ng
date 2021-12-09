/*
*    Copyright (c) 2015 Broadcom Corporation
*    All Rights Reserved
* 
<:label-BRCM:2015:DUAL/GPL:standard

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
#if defined(CONFIG_BLOG)
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/if_bridge.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/ip.h>
#include <linux/igmp.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/ipv6.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>
#include <linux/bcm_dslcpe_wlan_info.h>
#include <skb_defines.h>
#include "bcm_mcast_priv.h"

int bcm_mcast_blog_get_rep_info(struct net_device *repDev, unsigned char *repMac, uint32_t *info)
{ 
   wlan_client_info_t wlInfo = { 0 };
   int                rc;
   uint32_t           phyType = netdev_path_get_hw_port_type(repDev);

   *info = 0;
   if( (BLOG_WLANPHY == BLOG_GET_PHYTYPE(phyType) || (BLOG_NETXLPHY == BLOG_GET_PHYTYPE(phyType))) &&
       (repDev->wlan_client_get_info != NULL) )
   {
      rc = repDev->wlan_client_get_info(repDev, repMac, mcast_ctrl->mcastPriQueue, &wlInfo);
      if ( rc != 0 )
      {
          return -1;
      }
      *info = wlInfo.wl;
   }

   return 0;
}


void bcm_mcast_blog_release(int proto, void *mc_fdb)
{
   Blog_t *blog_p = BLOG_NULL;
   BlogActivateKey_t blog_idx;
   BlogTraffic_t traffic;

   blog_idx.fc.word = BLOG_KEY_FC_INVALID;
   blog_idx.mc.word = BLOG_KEY_MCAST_INVALID;

   do
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
      if(proto == BCM_MCAST_PROTO_IPV4)
      {
         blog_idx =  ((t_igmp_grp_entry *)mc_fdb)->blog_idx;
         ((t_igmp_grp_entry *)mc_fdb)->blog_idx.fc.word = BLOG_KEY_FC_INVALID;
         ((t_igmp_grp_entry *)mc_fdb)->blog_idx.mc.word = BLOG_KEY_MCAST_INVALID;
         traffic = BlogTraffic_IPV4_MCAST;
         break;
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if(proto == BCM_MCAST_PROTO_IPV6)
      {
         blog_idx =  ((t_mld_grp_entry *)mc_fdb)->blog_idx;
         ((t_mld_grp_entry *)mc_fdb)->blog_idx.fc.word = BLOG_KEY_FC_INVALID;
         ((t_mld_grp_entry *)mc_fdb)->blog_idx.mc.word = BLOG_KEY_MCAST_INVALID;
         traffic = BlogTraffic_IPV6_MCAST;
         break;
      }
#endif
      /* invalid protocol */
      __logError("invalid protocol specified");
      return;
   } while ( 0 );

   if((BLOG_KEY_FC_INVALID == blog_idx.fc.word) || (BLOG_KEY_MCAST_INVALID == blog_idx.mc.word))
   {
      return;
   }

   blog_p = blog_deactivate(blog_idx, traffic, BlogClient_fcache);
   if ( blog_p )
   {
      blog_rule_free_list(blog_p);
      blog_put(blog_p);
   }

   return;
}

static blogRuleAction_t *bcm_mcast_blog_find_command(blogRule_t *blogRule_p,
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

static void bcm_mcast_blog_process_wan(blogRule_t         *rule_p,
                                       void               *mc_fdb,
                                       int                 proto,
                                       struct net_device **wan_dev_pp,
                                       struct net_device **wan_vlan_dev_pp)
{
   blogRuleAction_t   ruleAction;
   struct net_device *dev_p = NULL;
#if defined(CONFIG_BR_IGMP_SNOOP)
   t_igmp_grp_entry  *igmp_fdb = NULL;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   t_mld_grp_entry  *mld_fdb = NULL;
#endif 
   uint8_t           *dev_addr = NULL;
   uint32_t           phyType;
   char               wan_ops;
   uint32_t           index = 0;

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
         *wan_dev_pp = dev_p;
         break;
      }

      if(dev_p->priv_flags & IFF_PPP)
      {
         rule_p->filter.hasPppoeHeader = 1;
         memset(&ruleAction, 0, sizeof(blogRuleAction_t));
         ruleAction.cmd = BLOG_RULE_CMD_POP_PPPOE_HDR;
         blog_rule_add_action(rule_p, &ruleAction);

         if ( NULL == bcm_mcast_blog_find_command(rule_p, BLOG_RULE_CMD_SET_MAC_DA, &index) )
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
      else if(*wan_vlan_dev_pp == NULL &&
              dev_p->priv_flags & IFF_BCM_VLAN)
      {
         *wan_vlan_dev_pp = dev_p;
      }
      dev_p = netdev_path_next_dev(dev_p);
   }

   /* For IPoA */
   phyType = netdev_path_get_hw_port_type(*wan_dev_pp);
   phyType = BLOG_GET_HW_ACT(phyType);
   if((phyType == VC_MUX_IPOA) || (phyType == LLC_SNAP_ROUTE_IP))
   {
      if ( NULL == bcm_mcast_blog_find_command(rule_p, BLOG_RULE_CMD_SET_MAC_DA, &index) )
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

static void bcm_mcast_blog_process_lan(blogRule_t         *rule_p,
                                       void               *mc_fdb,
                                       int                 proto,
                                       struct net_device **lan_dev_pp,
                                       struct net_device **lan_vlan_dev_pp)
{
   struct net_device *dev_p = NULL;
#if defined(CONFIG_BR_IGMP_SNOOP)
   t_igmp_grp_entry *igmp_fdb = NULL;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   t_mld_grp_entry *mld_fdb = NULL;
#endif
#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
   int phyType;
#endif

   if(NULL == mc_fdb)
   {
      return;
   }

   do
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
       if (BCM_MCAST_PROTO_IPV4 == proto )
       {
           igmp_fdb = (t_igmp_grp_entry *)mc_fdb;
           dev_p = igmp_fdb->dst_dev;
           break;
       }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
       if (BCM_MCAST_PROTO_IPV6 == proto )
       {
           mld_fdb = (t_mld_grp_entry *)mc_fdb;
           dev_p = mld_fdb->dst_dev;
           break;
       }
#endif
      /* invalid protocol */
      __logError("invalid protocol specified");
      return;
   } while (0);


    while(1)
    {
        if(netdev_path_is_root(dev_p))
        {
            *lan_dev_pp = dev_p;
            break;
        }

        if(*lan_vlan_dev_pp == NULL &&
           dev_p->priv_flags & IFF_BCM_VLAN)
        {
            *lan_vlan_dev_pp = dev_p;
        }

        dev_p = netdev_path_next_dev(dev_p);
    }

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

#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
static void bcm_mcast_blog_vlan_notify_for_blog_update(struct net_device             *ndev,
                                                       blogRuleVlanNotifyDirection_t  direction,
                                                       uint32_t                       nbrOfTags)
{
   if((ndev->priv_flags & IFF_WANDEV) && (direction == BLOG_RULE_VLAN_NOTIFY_DIR_TX))
   {
      return;
   }

   bcm_mcast_if_update_bydev(BCM_MCAST_PROTO_ALL, ndev, 1);
   return;
}
#endif

static void bcm_mcast_blog_link_devices(bcm_mcast_ifdata  *pif,
                                        Blog_t            *blog_p, 
                                        struct net_device *rxDev, 
                                        struct net_device *txDev, 
                                        int                wanType)
{
    struct net_device *dev_p;
    uint32_t           delta;
    struct net_device *rxPath[MAX_VIRT_DEV];
    int                rxPathIdx = 0;
    int                i;

    /* save rx path required for reverse path traversal for delta calc */
    memset(&rxPath[0], 0, (MAX_VIRT_DEV * sizeof(struct net_device *)));
    dev_p = rxDev;
    while(1)
    {
        if(netdev_path_is_root(dev_p))
        {
            break;
        }
        rxPath[rxPathIdx] = dev_p;
        rxPathIdx++;
        dev_p = netdev_path_next_dev(dev_p);
    }

    /* omit Ethernet header from virtual dev RX stats */
    delta = BLOG_ETH_HDR_LEN;
    for(i = (MAX_VIRT_DEV-1); i >= 0; i--)
    {
        if(NULL == rxPath[i])
        {
            continue;
        }

        if ( rxPath[i]->priv_flags & IFF_PPP )
        {
            delta += BLOG_PPPOE_HDR_LEN;
        }

        if ( rxPath[i]->priv_flags & IFF_802_1Q_VLAN )
        {
            delta += BLOG_VLAN_HDR_LEN;
        }

        if ( (rxPath[i]->priv_flags & IFF_BCM_VLAN) && 
             (blog_p->vtag_num > 0) )
        {
            delta += BLOG_VLAN_HDR_LEN;
        }

        blog_lock();
        blog_link(IF_DEVICE_MCAST, blog_p, rxPath[i], DIR_RX, delta);
        blog_unlock();
        dev_p = netdev_path_next_dev(dev_p);
    }

    /* include Ethernet header in virtual TX stats */
    delta -= BLOG_ETH_HDR_LEN;
    if ( wanType == BCM_MCAST_IF_ROUTED )
    {
        /* routed packets will come through bridge device, link bridge
           device with blog */
        blog_lock();
        blog_link(IF_DEVICE_MCAST, blog_p, pif->dev, DIR_TX, delta );
        blog_unlock();
    }

    dev_p = txDev;
    while(1)
    {
        if(netdev_path_is_root(dev_p))
        {
            break;
        }

        if ( dev_p->priv_flags & IFF_802_1Q_VLAN )
        {
            delta -= BLOG_VLAN_HDR_LEN;
        }

        if ( dev_p->priv_flags & IFF_BCM_VLAN )
        {
            delta -= BLOG_VLAN_HDR_LEN;
        }

        blog_lock();
        blog_link(IF_DEVICE_MCAST, blog_p, dev_p, DIR_TX, delta);
        blog_unlock();
        dev_p = netdev_path_next_dev(dev_p);
    }
}

static int bcm_mcast_blog_vlan_process(bcm_mcast_ifdata *pif,
                                       void             *mc_fdb,
                                       int               proto,
                                       Blog_t           *blog_p)
 {
    Blog_t           *new_blog_p;
    void             *new_mc_fdb = NULL;
    blogRule_t       *rule_p = NULL;
    int               firstRule = 1;
    uint32_t          vid = 0;
    blogRuleFilter_t *rule_filter = NULL;
    BlogTraffic_t     traffic;
    int               activates = 0;
    void             *rxDev;
    void             *txDev;
    int               wanType;
    BlogActivateKey_t *blog_key_p;

    if(!mc_fdb || !blog_p || !pif)
    {
        return 0;
    }

    if( (BCM_MCAST_PROTO_IPV4 != proto) && 
        (BCM_MCAST_PROTO_IPV6 != proto) )
    {
        return 0;
    }

    firstRule = 1;
    rule_p = (blogRule_t *)blog_p->blogRule_p;
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
                if (proto != blog_p->key.protocol)
                {
                    /* skip this rule */
                    blog_p->blogRule_p = rule_p->next_p;
                    blog_rule_free(rule_p);
                    rule_p = blog_p->blogRule_p;
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
                if (nxtHdr != blog_p->key.protocol)
                {
                    /* skip this rule */
                    blog_p->blogRule_p = rule_p->next_p;
                    blog_rule_free(rule_p);
                    rule_p = blog_p->blogRule_p;
                    continue;
                }
            }
        }
#endif

        /* create new fdb entry unless this is the first rule. For the
           first rule use the fdb entry that was passed in */
        if ( 1 == firstRule )
        {
            firstRule  = 0;
            new_mc_fdb = mc_fdb;
        }
        else
        {
#if defined(CONFIG_BR_IGMP_SNOOP)
            if ( BCM_MCAST_PROTO_IPV4 == proto)
            {
               new_mc_fdb = bcm_mcast_igmp_fdb_copy(pif, mc_fdb);
            }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
            if ( BCM_MCAST_PROTO_IPV6 == proto)
            {
               new_mc_fdb = bcm_mcast_mld_fdb_copy(pif, mc_fdb);
            }
#endif
            if(!new_mc_fdb)
            {
                __logError("mcast fdb allocation failure");
                break;
            }
        }

        /* get a new blog and copy original blog */
        new_blog_p = blog_get();
        if (new_blog_p == BLOG_NULL) 
        {
            if (new_mc_fdb != mc_fdb) 
            {
#if defined(CONFIG_BR_IGMP_SNOOP)
               if ( BCM_MCAST_PROTO_IPV4 == proto)
               {
                  bcm_mcast_igmp_del_entry(pif, mc_fdb, NULL, NULL);
               }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
               if ( BCM_MCAST_PROTO_IPV6 == proto)
               {
                  bcm_mcast_mld_del_entry(pif, mc_fdb, NULL, NULL);
               }
#endif
            }
            break;
        }
        blog_copy(new_blog_p, blog_p);

        /* pop the rule off the original blog now that a new fdb and blog have been
           allocated. This is to ensure that all rules are freed in case of error */
        blog_p->blogRule_p = rule_p->next_p;
        rule_p->next_p = NULL;
        new_blog_p->blogRule_p = rule_p;

        rule_filter = &(((blogRule_t *)new_blog_p->blogRule_p)->filter);
        new_blog_p->vtag_num = rule_filter->nbrOfVlanTags;
        if (rule_filter->vlan[0].mask.h_vlan_TCI)
        {
            vid = ((rule_filter->vlan[0].value.h_vlan_TCI &
                rule_filter->vlan[0].mask.h_vlan_TCI) & 0xFFF);
        }
        else
        {
            vid = 0xFFF;
        }
        new_blog_p->vtag[0] = vid; 
        if (rule_filter->vlan[1].mask.h_vlan_TCI)
        {
            vid = ((rule_filter->vlan[1].value.h_vlan_TCI &
                rule_filter->vlan[1].mask.h_vlan_TCI) & 0xFFF);
        }
        else
        {
            vid = 0xFFF;
        }
        new_blog_p->vtag[1] = vid;

        blog_lock();
        blog_link(MCAST_FDB, new_blog_p, (void *)new_mc_fdb, 0, 0);
        blog_unlock();

#if defined(CONFIG_BR_IGMP_SNOOP)
        if(BCM_MCAST_PROTO_IPV4 == proto)
        {
            traffic = BlogTraffic_IPV4_MCAST;
            ((t_igmp_grp_entry *)new_mc_fdb)->wan_tci = (new_blog_p->vtag[1] << 16) | new_blog_p->vtag[0];
            ((t_igmp_grp_entry *)new_mc_fdb)->num_tags = new_blog_p->vtag_num;
            rxDev   = ((t_igmp_grp_entry *)new_mc_fdb)->from_dev;
            txDev   = ((t_igmp_grp_entry *)new_mc_fdb)->dst_dev;
            wanType = ((t_igmp_grp_entry *)new_mc_fdb)->type;
        }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
        if(BCM_MCAST_PROTO_IPV6 == proto)
        {
            traffic = BlogTraffic_IPV6_MCAST;
            ((t_mld_grp_entry *)new_mc_fdb)->wan_tci = (new_blog_p->vtag[1] << 16) | new_blog_p->vtag[0];
            ((t_mld_grp_entry *)new_mc_fdb)->num_tags = new_blog_p->vtag_num;
            rxDev   = ((t_mld_grp_entry *)new_mc_fdb)->from_dev;
            txDev   = ((t_mld_grp_entry *)new_mc_fdb)->dst_dev;
            wanType = ((t_mld_grp_entry *)new_mc_fdb)->type;
        }
#endif
        bcm_mcast_blog_link_devices(pif, new_blog_p, rxDev, txDev, wanType);

        blog_key_p = blog_activate(new_blog_p, traffic, BlogClient_fcache);
        if ( blog_key_p == NULL )
        {
            blog_rule_free_list(new_blog_p);
            blog_put(new_blog_p);
            if ( new_mc_fdb != mc_fdb )
            {
#if defined(CONFIG_BR_IGMP_SNOOP)
               if ( BCM_MCAST_PROTO_IPV4 == proto)
               {
                  bcm_mcast_igmp_del_entry(pif, new_mc_fdb, NULL, NULL);
               }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
               if ( BCM_MCAST_PROTO_IPV6 == proto)
               {
                  bcm_mcast_mld_del_entry(pif, new_mc_fdb, NULL, NULL);
               }
#endif
            }
        }
        else
        {
           do
           {
#if defined(CONFIG_BR_IGMP_SNOOP)
              if ( BCM_MCAST_PROTO_IPV4 == proto)
              {
                 ((t_igmp_grp_entry *)new_mc_fdb)->blog_idx = *blog_key_p;
                 break;
              }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
              if ( BCM_MCAST_PROTO_IPV6 == proto)
              {
                 ((t_mld_grp_entry *)new_mc_fdb)->blog_idx = *blog_key_p;
                 break;
              }
#endif
           } while (0);
           activates++;
        }

        /* advance to the next rule */
        rule_p = blog_p->blogRule_p;
    }

    /* Free blog. The blog will only have rules if there was an error */
    blog_rule_free_list(blog_p);
    blog_put(blog_p);

    return activates;
} /* bcm_mcast_blog_vlan_process */

static uint16_t br_mcast_fetch_lan_tci(Blog_t *blog_p)
{
    blogRule_t *rule_p = (blogRule_t *)blog_p->blogRule_p;
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

/*
 * returns
 *   -2 - flow should not exist, please delete all conflicting blogs.
 *   -1 - failed to activate a flow
 *    0 - activated flow
 */
int bcm_mcast_blog_process(bcm_mcast_ifdata *pif, void *mc_fdb, int proto, struct hlist_head *headMcHash)
{
   Blog_t *blog_p = BLOG_NULL;
   blogRule_t *rule_p = NULL;
   struct net_device *wan_vlan_dev_p = NULL;
   struct net_device *lan_vlan_dev_p = NULL;
   struct net_device *wan_dev_p = NULL;
   struct net_device *lan_dev_p = NULL;
#if defined(CONFIG_BR_IGMP_SNOOP)
   t_igmp_grp_entry *igmp_fdb = NULL;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   t_mld_grp_entry *mld_fdb = NULL;
#endif
   struct net_device *from_dev = NULL;
   struct net_device *dst_dev = NULL;
   uint32_t phyType;
   int numActivates;

   if(!mc_fdb)
   {
      return -1;
   }

   if ( 0 == mcast_ctrl->blog_enable )
   {
      return 0;
   }

   do
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
      if(BCM_MCAST_PROTO_IPV4 == proto)
      {
         igmp_fdb = (t_igmp_grp_entry *)mc_fdb;
         from_dev = igmp_fdb->from_dev;
         dst_dev = igmp_fdb->dst_dev;
         break;
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if(BCM_MCAST_PROTO_IPV6 == proto)
      {
         mld_fdb = (t_mld_grp_entry *)mc_fdb;
         from_dev = mld_fdb->from_dev;
         dst_dev = mld_fdb->dst_dev;
         break;
      }
#endif
      __logError("invalid protocol specified");
      return -1;
   } while(0);

   /* allocate blog */
   blog_p = blog_get();
   if(blog_p == BLOG_NULL)
   {
      __logError("blog allocation failure");
      return -1;
   }

   /* allocate blog rule */
   rule_p = blog_rule_alloc();
   if(rule_p == NULL)
   {
      __logError("blog rule allocation failure");
      blog_put(blog_p);
      return -1;
   }

   blog_rule_init(rule_p);
   blog_p->blogRule_p = (void *)rule_p;

   /* find LAN devices */
   bcm_mcast_blog_process_lan(rule_p, mc_fdb, proto, &lan_dev_p, &lan_vlan_dev_p);

   /* for LAN2LAN don't do anything */
   if(pif->dev == from_dev) 
   {
      blog_p->rx.info.phyHdr = 0;
      blog_p->rx.info.channel = 0xFF; /* for lan2lan mcast */
      blog_p->rx.info.bmap.BCM_SWC = 1;
      wan_dev_p = from_dev;
   }
   else
   {
      /* find WAN devices */
      bcm_mcast_blog_process_wan(rule_p, mc_fdb, proto,
                                &wan_dev_p, &wan_vlan_dev_p);

      phyType = netdev_path_get_hw_port_type(wan_dev_p);
      blog_p->rx.info.phyHdrType = BLOG_GET_PHYTYPE(phyType);
      blog_p->rx.info.phyHdrLen = BLOG_GET_PHYLEN(phyType);
      phyType = BLOG_GET_HW_ACT(phyType);

      if(blog_p->rx.info.phyHdrType == BLOG_GPONPHY)
      {
         unsigned int hw_subport_mcast_idx;

         hw_subport_mcast_idx = netdev_path_get_hw_subport_mcast_idx(wan_dev_p);

         if(hw_subport_mcast_idx < CONFIG_BCM_MAX_GEM_PORTS)
         {
            blog_p->rx.info.channel = hw_subport_mcast_idx;
         }
         else
         {
            /* Not a GPON Multicast WAN device */
            blog_rule_free_list(blog_p);
            blog_put(blog_p);
            return -1;
         }
      }
      else /* Ethernet or DSL WAN device */
      {
         blog_p->rx.info.channel = netdev_path_get_hw_port(wan_dev_p);
      }

      if( (blog_p->rx.info.phyHdrType == BLOG_XTMPHY) &&
          ((phyType == VC_MUX_PPPOA) ||
           (phyType == VC_MUX_IPOA) ||
           (phyType == LLC_SNAP_ROUTE_IP) ||
           (phyType == LLC_ENCAPS_PPP)) )
      {
         blog_p->insert_eth = 1;
      }

      if( (blog_p->rx.info.phyHdrType == BLOG_XTMPHY) &&
          ((phyType == VC_MUX_PPPOA) ||
           (phyType == LLC_ENCAPS_PPP)) )
      {
         blog_p->pop_pppoa = 1;
      }

      if(blog_p->rx.info.phyHdrType == BLOG_ENETPHY)
      {
         blog_p->rx.info.bmap.BCM_SWC = 1;
      }
      else
      {
         blog_p->rx.info.bmap.BCM_XPHY = 1;
      }
   }

#if defined(CONFIG_BR_IGMP_SNOOP)
   if (igmp_fdb)
   {
      if (igmp_fdb->lanppp) 
      {
         blog_p->has_pppoe = 1;
      }
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if (mld_fdb)
   {
      if (mld_fdb->lanppp) 
      {
         blog_p->has_pppoe = 1;
      }
   }
#endif

   blog_p->tx.info.bmap.BCM_SWC = 1;

   blog_p->key.l1_tuple.phy = blog_p->rx.info.phyHdr;
   blog_p->key.l1_tuple.channel = blog_p->rx.info.channel;
   blog_p->key.protocol = BLOG_IPPROTO_UDP;

   phyType = netdev_path_get_hw_port_type(lan_dev_p);
   blog_p->tx.info.phyHdrType = BLOG_GET_PHYTYPE(phyType);
   blog_p->tx.info.phyHdrLen = BLOG_GET_PHYLEN(phyType);
   blog_p->tx.info.channel = netdev_path_get_hw_port(lan_dev_p);

#if defined(CONFIG_BR_IGMP_SNOOP)
   if ( BCM_MCAST_PROTO_IPV4 == proto )
   {
      blog_p->rx.tuple.saddr = igmp_fdb->src_entry.src.s_addr;
      blog_p->rx.tuple.daddr = igmp_fdb->rxGrp.s_addr;
      blog_p->tx.tuple.saddr = igmp_fdb->src_entry.src.s_addr;
      blog_p->tx.tuple.daddr = igmp_fdb->txGrp.s_addr;
      blog_p->rx.tuple.port.dest = 0;
      if (igmp_fdb->excludePort != -1) {
         blog_p->mcast_excl_udp_port = igmp_fdb->excludePort;
      }
      /* SSM is defined as "Multicast with non-zero Source Address" */
      if (blog_p->rx.tuple.saddr != 0) 
      {
          blog_p->is_ssm = 1;
      }

      blog_p->rtp_seq_chk = igmp_fdb->enRtpSeqCheck;
      blog_p->rx.info.bmap.PLD_IPv4 = 1;
      blog_p->tx.info.bmap.PLD_IPv4 = 1;
      blog_p->wl = igmp_fdb->info;
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if(BCM_MCAST_PROTO_IPV6 == proto)
   {
      BCM_IN6_ASSIGN_ADDR(&blog_p->tupleV6.saddr, &mld_fdb->src_entry.src);
      BCM_IN6_ASSIGN_ADDR(&blog_p->tupleV6.daddr, &mld_fdb->grp);
      blog_p->rx.info.bmap.PLD_IPv6 = 1;
      blog_p->tx.info.bmap.PLD_IPv6 = 1;
      blog_p->wl = mld_fdb->info;

      /* SSM is defined as "Multicast with non-zero Source Address" */
      if (!( ( blog_p->tupleV6.saddr.p32[0] == 0 ) &&
             ( blog_p->tupleV6.saddr.p32[1] == 0 ) &&
             ( blog_p->tupleV6.saddr.p32[2] == 0 ) &&
             ( blog_p->tupleV6.saddr.p32[3] == 0 ) ))
      {
          blog_p->is_ssm = 1;
      }
   }
#endif

   blog_p->rx_dev_p = wan_dev_p;
   blog_p->rx.multicast = 1;
   blog_p->tx_dev_p = lan_dev_p;

   if ( mcast_ctrl->mcastPriQueue != -1 )
   {
      blog_p->mark = SKBMARK_SET_Q(blog_p->mark, mcast_ctrl->mcastPriQueue);
   }

   /* add vlan blog rules, if any vlan interfaces were found */
   if(blogRuleVlanHook && (wan_vlan_dev_p || lan_vlan_dev_p)) {
      if(blogRuleVlanHook(blog_p, wan_vlan_dev_p, lan_vlan_dev_p) < 0) {
         __logError("blogRuleVlanHook error");
         blog_rule_free_list(blog_p);
         blog_put(blog_p);
         return -1;
      }
   }

   /* blog must have at least one rule */
   if (NULL == blog_p->blogRule_p)
   {
      /* blogRule_p == NULL may be valid if there are no 
         VLAN rules and the default behavior for either interface is DROP */
      __logDebug("blogRuleVlanHook no blog rules returned");
      blog_put(blog_p);
      return -1;
   }

   if (headMcHash)
   {
      do 
      {
#if defined(CONFIG_BR_MLD_SNOOP)
        if(BCM_MCAST_PROTO_IPV6 == proto)
        {
           struct mld_grp_entry *dst;
           uint16_t lan_tci = br_mcast_fetch_lan_tci(blog_p);
           struct mld_grp_entry *local_fdb = ((struct mld_grp_entry *)mc_fdb);
           struct net_device *local_root_dst_devp = local_fdb->dst_dev;
           local_fdb->lan_tci = lan_tci;

           /* Get Root device */
           while (local_root_dst_devp && !netdev_path_is_root(local_root_dst_devp))
           {
               local_root_dst_devp = netdev_path_next_dev(local_root_dst_devp);
           }

           /* Check for conflicting VLAN settings before activating the blog */
           hlist_for_each_entry(dst, headMcHash, hlist)
           {
              struct net_device *root_dst_devp = dst->dst_dev;

              /* Get Root device */
              while (root_dst_devp && !netdev_path_is_root(root_dst_devp))
              {
                  root_dst_devp = netdev_path_next_dev(root_dst_devp);
              }

              if ( (local_fdb != dst) && // don't conflict with yourself
                   (memcmp(&local_fdb->grp, &dst->grp, 16) == 0) &&
                   (memcmp(&local_fdb->src_entry.src, &dst->src_entry.src, 16) == 0) &&
                   (local_fdb->src_entry.filt_mode == dst->src_entry.filt_mode) && 
                   (from_dev == dst->from_dev) &&
                    /* Check if the destination root device is the same 
                       We do not support different egress vlan modifications destined to
                       same root device */
                   (local_root_dst_devp == root_dst_devp) &&
                   ( (lan_tci != dst->lan_tci) || (dst->blog_idx.mc.word == BLOG_KEY_INVALID) ) )
              {
                 // Conflict has been found.   Acceleration no longer possible.
                 blog_rule_free_list(blog_p);
                 blog_put(blog_p);
                 return -2;
              }
           }
           break;
        }
#endif
#if defined(CONFIG_BR_IGMP_SNOOP)
        if(BCM_MCAST_PROTO_IPV4 == proto)
        {
           struct igmp_grp_entry *dst;
           uint16_t lan_tci = br_mcast_fetch_lan_tci(blog_p);
           struct igmp_grp_entry *local_fdb = ((struct igmp_grp_entry *)mc_fdb);
           struct net_device *local_root_dst_devp = local_fdb->dst_dev;
           local_fdb->lan_tci = lan_tci;

           /* Get Root device */
           while (local_root_dst_devp && !netdev_path_is_root(local_root_dst_devp))
           {
               local_root_dst_devp = netdev_path_next_dev(local_root_dst_devp);
           }

           /* Check for conflicting VLAN settings before activating the blog */
           hlist_for_each_entry(dst, headMcHash, hlist)
           {
              struct net_device *root_dst_devp = dst->dst_dev;

              /* Get Root device */
              while (root_dst_devp && !netdev_path_is_root(root_dst_devp))
              {
                  root_dst_devp = netdev_path_next_dev(root_dst_devp);
              }

              if ( (local_fdb != dst) && // don't conflict with yourself
                     (local_fdb->txGrp.s_addr == dst->txGrp.s_addr) && (local_fdb->rxGrp.s_addr == dst->rxGrp.s_addr) &&
                     (local_fdb->src_entry.src.s_addr == dst->src_entry.src.s_addr) &&
                     (local_fdb->src_entry.filt_mode == dst->src_entry.filt_mode) && 
                     (from_dev == dst->from_dev) &&
                     /* Check if the destination root device is the same 
                        We do not support different egress vlan modifications destined to
                        same root device */
                     (local_root_dst_devp == root_dst_devp) &&
                     ( (lan_tci != dst->lan_tci) || (dst->blog_idx.mc.word == BLOG_KEY_INVALID) ) )
              {
                  // Conflict has been found.   Acceleration no longer possible.
                  blog_rule_free_list(blog_p);
                  blog_put(blog_p);
                  return -2;
              }
           }
           break;
        }
#endif 
     } while (0);
   }
   
   numActivates = bcm_mcast_blog_vlan_process(pif, mc_fdb, proto, blog_p);
   if ( 0 == numActivates )
   {
      return -1;
   }

   return 0;
} /* bcm_mcast_blog_process */

__init int bcm_mcast_blog_init(void)
{
#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
    blogRuleVlanNotifyHook = bcm_mcast_blog_vlan_notify_for_blog_update;
#endif
   return 0;
}

void bcm_mcast_blog_deinit(void)
{
#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
    blogRuleVlanNotifyHook = NULL;
#endif   
}

#endif /* CONFIG_BLOG */

