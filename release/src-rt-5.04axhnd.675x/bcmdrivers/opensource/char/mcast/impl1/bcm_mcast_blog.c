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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#include <linux/bcm_netdevice.h>
#include <linux/bcm_log.h>
#endif
#include "bcm_mcast_blogrule.h"
#include "bcm_mcast_igmpsnp.h"

bcm_mcast_flow_add_hook_t bcm_mcast_flow_add_hook = NULL;
bcm_mcast_flow_delete_hook_t bcm_mcast_flow_delete_hook = NULL;

int bcm_mcast_blog_get_wlan_reporter_info(struct net_device *repDev, unsigned char *repMac, uint32_t *info)
{ 
   wlan_client_info_t wlInfo = { 0 };
   int                rc;
   uint32_t           phyType = netdev_path_get_hw_port_type(repDev);
   wlan_client_get_info_t wlan_client_get_info_func_ptr;

   /* Get root reporting device since wlan_client_get_info hook is only registered
      on the root device. Any VLANCtl interfaces created on top of the root
      devices do not have this function pointer set. */
   struct net_device *rootRepDev = netdev_path_get_root(repDev);

   *info = 0;

   wlan_client_get_info_func_ptr = netdev_wlan_client_get_info(rootRepDev);
   if( (BLOG_WLANPHY == BLOG_GET_PHYTYPE(phyType) || (BLOG_NETXLPHY == BLOG_GET_PHYTYPE(phyType))) &&
        wlan_client_get_info_func_ptr)
   {
      rc = wlan_client_get_info_func_ptr(rootRepDev, repMac, mcast_ctrl->mcastPriQueue, &wlInfo);
      if ( rc != 0 )
      {
          return -1;
      }
      *info = wlInfo.wl;
   }
   return 0;
}

/* Compare info_1 with info_2.
** if the info is for blog_p->wl, ignore the prio/flowring_idx in comparing.
** Return 0: ==, others: !=
*/
int bcm_mcast_blog_cmp_wlan_reporter_info(struct net_device *repDev, uint32_t info_1, uint32_t info_2)
{
   uint32_t  phyType = netdev_path_get_hw_port_type(repDev);   
   wlan_client_info_t wlInfo_1 = { 0 };
   wlan_client_info_t wlInfo_2 = { 0 };

   wlInfo_1.wl = info_1;
   wlInfo_2.wl = info_2;

   /* not wlan */
   if ((BLOG_WLANPHY != BLOG_GET_PHYTYPE(phyType)) && (BLOG_NETXLPHY != BLOG_GET_PHYTYPE(phyType)))
      goto exit;

   if (wlInfo_1.wfd.mcast.is_wfd != wlInfo_2.wfd.mcast.is_wfd)
      return 1;

   if (wlInfo_1.wfd.mcast.is_wfd)
   {
      wlInfo_1.wfd.mcast.wfd_prio = 0;
      wlInfo_2.wfd.mcast.wfd_prio = 0;
      goto exit;
   }
   else
   {
      wlInfo_1.rnr.priority = 0;      
      wlInfo_2.rnr.priority = 0;
      wlInfo_1.rnr.flowring_idx = 0;
      wlInfo_2.rnr.flowring_idx = 0;
      goto exit;
   }

exit:
   return (wlInfo_1.wl == wlInfo_2.wl) ? 0 : 1;
   
}

void bcm_mcast_blog_release(int proto, bcm_mcast_flowkey_t flow_key)
{
   Blog_t *blog_p = BLOG_NULL;
   BlogTraffic_t traffic;

   do
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
      if(proto == BCM_MCAST_PROTO_IPV4)
      {
         traffic = BlogTraffic_IPV4_MCAST;
         break;
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if(proto == BCM_MCAST_PROTO_IPV6)
      {
         traffic = BlogTraffic_IPV6_MCAST;
         break;
      }
#endif
      /* invalid protocol */
      __logError("invalid protocol specified");
      return;
   } while ( 0 );

   blog_p = bcm_mcast_flow_delete_hook(flow_key, traffic);
   if ( blog_p )
   {
      blog_rule_free_list(blog_p->blogRule_p);
      blog_put(blog_p);
   }

   return;
}


#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
static void bcm_mcast_blog_vlan_notify_for_blog_update(struct net_device             *ndev,
                                                       blogRuleVlanNotifyDirection_t  direction,
                                                       uint32_t                       nbrOfTags)
{
   if(is_netdev_wan(ndev) && (direction == BLOG_RULE_VLAN_NOTIFY_DIR_TX))
   {
      return;
   }

   bcm_mcast_if_update_bydev(BCM_MCAST_PROTO_ALL, ndev, 1);
   return;
}
#endif

int bcm_mcast_activate_blog(bcm_mcast_ifdata  *pif,
                                           blogRule_t        *rule_p,
                                           void              *mc_fdb,
                                           int                proto,
                                           void              *arg_p,
                                           uintptr_t          flowhdl)
{
    void              *rxDev = NULL;
    void              *txDev = NULL;
    int                wanType = BCM_MCAST_IF_UNKNOWN;
    BlogTraffic_t      traffic = BlogTraffic_MAX;
    Blog_t            *new_blog_p;
    Blog_t            *blog_p = (Blog_t *)arg_p;
    uint8_t            vtag_num = 0;
    bcm_mcast_flowkey_t flow_key;

    __logDebug("ENTER");
    /* get a new blog and copy original blog */
    new_blog_p = blog_get();
    if (new_blog_p == BLOG_NULL) 
    {
        return -1;
    }
    blog_copy(new_blog_p, blog_p);

    /* pop the rule off the original blog now that a new fdb and blog have been
       allocated. This is to ensure that all rules are freed in case of error */
    blog_p->blogRule_p = rule_p->next_p;
    rule_p->next_p = NULL;
    new_blog_p->blogRule_p = rule_p;

    bcm_mcast_blogrule_get_vlan_info(rule_p,
                                     &vtag_num,
                                     &new_blog_p->vtag[0],
                                     &new_blog_p->vtag[1]);
    new_blog_p->vtag_num = vtag_num;

#if defined(CONFIG_BR_IGMP_SNOOP)
    if(BCM_MCAST_PROTO_IPV4 == proto)
    {
        traffic = BlogTraffic_IPV4_MCAST;
        rxDev   = ((t_igmp_grp_entry *)mc_fdb)->from_dev;
        txDev   = ((t_igmp_grp_entry *)mc_fdb)->dst_dev;
        wanType = ((t_igmp_grp_entry *)mc_fdb)->type;
    }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
    if(BCM_MCAST_PROTO_IPV6 == proto)
    {
        traffic = BlogTraffic_IPV6_MCAST;
        rxDev   = ((t_mld_grp_entry *)mc_fdb)->from_dev;
        txDev   = ((t_mld_grp_entry *)mc_fdb)->dst_dev;
        wanType = ((t_mld_grp_entry *)mc_fdb)->type;
    }
#endif
    
    if ( bcm_mcast_flow_add_hook(new_blog_p, traffic, &flow_key) )
    {
        blog_rule_free_list(new_blog_p->blogRule_p);
        if (BCM_MCAST_PROTO_IPV4 == proto) 
        {
            __logInfo("%pS failed Grp 0x%x vlan0 0x%x vlan1 0x%x numtags %d rxdev %s txdev %s "
                      "wl 0x%x wlsta_id/wlinfo 0x%x", bcm_mcast_flow_add_hook,
                       htonl(new_blog_p->rx.tuple.daddr), htonl(new_blog_p->vtag[0]), htonl(new_blog_p->vtag[1]), 
                       new_blog_p->vtag_num, ((struct net_device *)rxDev)->name, ((struct net_device *)txDev)->name,
                      new_blog_p->wl, new_blog_p->wlsta_id);
        }
        else
        {
            __logInfo("%pS failed vlan0 0x%x vlan1 0x%x numtags %d rxdev %s txdev %s "
                      "wl 0x%x  wlsta_id/wlinfo 0x%x", bcm_mcast_flow_add_hook,
                       new_blog_p->vtag[0], new_blog_p->vtag[1], new_blog_p->vtag_num, 
                      ((struct net_device *)rxDev)->name, ((struct net_device *)txDev)->name, new_blog_p->wl, 
                      new_blog_p->wlsta_id);
            BCM_MCAST_DBG_PRINT_V6_ADDR("Group", new_blog_p->tupleV6.daddr.p16);
        }
        blog_put(new_blog_p);
        return -1;
    }
    else
    {
        if (BCM_MCAST_PROTO_IPV4 == proto) 
        {
            __logInfo("%pS successful Group 0x%x vlan0 0x%x vlan1 0x%x numtags %d rxdev %s txdev %s "
                      "wl 0x%x wlsta_id/wlinfo 0x%x", bcm_mcast_flow_add_hook,
                      htonl(new_blog_p->rx.tuple.daddr), new_blog_p->vtag[0], new_blog_p->vtag[1], 
                      new_blog_p->vtag_num, ((struct net_device *)rxDev)->name, ((struct net_device *)txDev)->name, 
                      new_blog_p->wl, new_blog_p->wlsta_id);
        }
        else
        {
            __logInfo("%pS successful vlan0 0x%x vlan1 0x%x numtags %d rxdev %s txdev %s wl 0x%x "
                      "wlsta_id/wlinfo 0x%x", bcm_mcast_flow_add_hook,
                       new_blog_p->vtag[0], new_blog_p->vtag[1], new_blog_p->vtag_num, 
                      ((struct net_device *)rxDev)->name, ((struct net_device *)txDev)->name, new_blog_p->wl,
                      new_blog_p->wlsta_id);
            BCM_MCAST_DBG_PRINT_V6_ADDR("Group", blog_p->tupleV6.daddr.p16);
        }
        if (bcm_mcast_add_flowkey_to_flowhdl(flowhdl, &flow_key) != 0)
        {
            /* Flowkey add failed. Delete the flow */
            bcm_mcast_flow_delete_hook(flow_key, traffic);
            blog_rule_free_list(new_blog_p->blogRule_p);
            blog_put(new_blog_p);
            return -1;
        }
    }
    __logDebug("EXIT");
    return 0;
}    

/*
 * returns
 *   -2 - flow should not exist, please delete all conflicting blogs.
 *   -1 - failed to activate a flow
 *    0 - activated flow
 */
int bcm_mcast_blog_process(bcm_mcast_ifdata *pif, 
                           void *mc_fdb, 
                           int proto, 
                           struct hlist_head *headMcHash,
                           uintptr_t flowhdl)
{
   Blog_t *blog_p = BLOG_NULL;
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
   struct net_device *to_accel_dev = NULL;
   uint32_t phyType;
   int numActivates = 0;
   int lan2lan = 0;
   char *repMac= NULL;
   bcm_mcast_vlandev_list_t vlandev_list = { 0 };
   int i = 0;

   if(!mc_fdb)
   {
      __logError("mc_fdb NULL\n");
      return -1;
   }

   if ( 0 == mcast_ctrl->blog_enable )
   {
      __logError("mcast ctrl blog disabled\n");
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
         to_accel_dev = igmp_fdb->to_accel_dev;
         break;
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if(BCM_MCAST_PROTO_IPV6 == proto)
      {
         mld_fdb = (t_mld_grp_entry *)mc_fdb;
         from_dev = mld_fdb->from_dev;
         dst_dev = mld_fdb->dst_dev;
         to_accel_dev = mld_fdb->to_accel_dev;
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

   /* find LAN devices */
   if ((lan_dev_p = bcm_mcast_find_root_dev(dst_dev)) == NULL)
   {
       __logError("Unable to find root lan dev for %s", dst_dev->name);
       return -1;
   }
   if (to_accel_dev != NULL && !netif_is_bond_master(lan_dev_p))
   {
       lan_dev_p = to_accel_dev;
   }
   bcm_mcast_find_vlan_devs(dst_dev, &vlandev_list);
   if ( vlandev_list.vlandev_count == 1 )
   {
       lan_vlan_dev_p = vlandev_list.vlandev[0];
   }
   else if ( vlandev_list.vlandev_count > 1 )
   {
       int i = 0;
       __logError("Multiple vlan devices found on the LAN side for dev %s. Scenario not supported", dst_dev->name);
       for (i = 0; i < vlandev_list.vlandev_count; i++)
       {
           printk("idx %d LAN vlandev %s\n", i, vlandev_list.vlandev[i]->name);
       }
       return -1;
   }
   /* NOTE: vlandev_count == 0 is a valid case. lan_vlan_dev_p would be NULL */

   /* Initialize vlandev list to 0 */
   memset(&vlandev_list, 0, sizeof(vlandev_list));

   /* for LAN2LAN don't do anything */
   if(pif->dev == from_dev) 
   {
      blog_p->rx.info.phyHdr = 0;
      blog_p->rx.info.channel = BCM_MCAST_RX_CHAN_LAN; /* for lan2lan mcast */
      blog_p->rx.info.bmap.BCM_SWC = 1;
      wan_dev_p = from_dev;
      lan2lan = 1;
   }
   else
   {
      /* find WAN devices */
       bcm_mcast_find_vlan_devs(from_dev, &vlandev_list);
       wan_dev_p = bcm_mcast_find_root_dev(from_dev);

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
            blog_put(blog_p);
            return -1;
         }
      }
      else if (blog_p->rx.info.phyHdrType == BLOG_EPONPHY)
      {  /* For EPON, the mcast data can come from any LLID */
         blog_p->rx.info.channel = BLOG_CHAN_XPON_MCAST_ANY;		
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
   blog_p->tx.info.channel = netdev_path_get_hw_tx_port(lan_dev_p);

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
      blog_p->wlsta_id=blog_p->wfd.mcast.sta_id;

      repMac = (list_last_entry(&((t_igmp_grp_entry *)mc_fdb)->rep_list, t_igmp_rep_entry, list)->repMac);
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
      blog_p->wlsta_id=blog_p->wfd.mcast.sta_id;

      /* SSM is defined as "Multicast with non-zero Source Address" */
      if (!( ( blog_p->tupleV6.saddr.p32[0] == 0 ) &&
             ( blog_p->tupleV6.saddr.p32[1] == 0 ) &&
             ( blog_p->tupleV6.saddr.p32[2] == 0 ) &&
             ( blog_p->tupleV6.saddr.p32[3] == 0 ) ))
      {
          blog_p->is_ssm = 1;
      }

      repMac = (list_last_entry(&((t_mld_grp_entry *)mc_fdb)->rep_list, t_mld_rep_entry, list)->repMac);
   }
#endif

   /* copy mcast client's MAC */
   memcpy(blog_p->src_mac.u8, repMac, BLOG_ETH_ADDR_LEN);

   if (blog_p->wl && ((BlogWfd_t *)(&blog_p->wl))->mcast.is_wmf_enabled) 
   {
       blog_p->mcast_client_type = BLOG_MCAST_CLIENT_TYPE_TXDEV_WLINFO;
   }
   else
       blog_p->mcast_client_type = BLOG_MCAST_CLIENT_TYPE_TXDEV;

   __logInfo("mcast_client_type %d wl 0x%x src mac %pM wmf enabled %d", 
              blog_p->mcast_client_type, blog_p->wl, repMac, 
              blog_p->wl ? ((BlogWfd_t *)(&blog_p->wl))->mcast.is_wmf_enabled:0);

   blog_p->rx_dev_p = wan_dev_p;
   blog_p->rx.multicast = 1;
   blog_p->tx_dev_p = lan_dev_p;

   if (phyType != BLOG_XTMPHY)
   {
      // if mcast precedence is disabled(-1), set to default queue 0
      int mcastDefTxPriorityQ = (mcast_ctrl->mcastPriQueue == -1) ? 0 :mcast_ctrl->mcastPriQueue;
      if(blog_p->tx.info.phyHdrType == BLOG_ENETPHY)
      {
         bcmEnet_QueueReMap_t queRemap;
         bcmFun_t *enetFun = bcmFun_get(BCM_FUN_ID_ENET_REMAP_TX_QUEUE);

         if (enetFun)
         {
             queRemap.dev = lan_dev_p;
             queRemap.tx_queue = mcastDefTxPriorityQ;
             mcastDefTxPriorityQ = enetFun(&queRemap);
         }
      }

      blog_p->mark = SKBMARK_SET_Q(blog_p->mark, mcastDefTxPriorityQ);
   }

   /* Note that the blogrule process function must be called even if there are no
      vlandevices in the vlandev list. This just means that it is either a LAN2LAN
      scenario or the WAN from_dev received from MCPD does not have any lower
      VLANctl devices. For example, the WAN from device could be a bridge with
      no vlanctl devices. Another example is RDK platforms where vlanctl driver is
      not enabled. In this case, both lan and wan vlan devices would be empty/NULL
      and the blog rule would have no actions */
   i = 0;
   do
   {
       __logInfo("Calling bcm_mcast_blogrule_process for vlandev %s lan2lan %d",
                  vlandev_list.vlandev[i], lan2lan);
       numActivates += bcm_mcast_blogrule_process(pif,
                                                  mc_fdb,
                                                  proto,
                                                  blog_p->key.protocol,
                                                  (blogRule_t **)(&(blog_p->blogRule_p)),
                                                  headMcHash,
                                                  (void *)blog_p,
                                                  lan2lan,
                                                  vlandev_list.vlandev[i],
                                                  lan_vlan_dev_p,
                                                  flowhdl);
       i++;
   }while ( i < vlandev_list.vlandev_count ); 

   blog_put(blog_p);

   return numActivates;
} /* bcm_mcast_blog_process */

__init int bcm_mcast_blog_init(void)
{
#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
    blogRuleVlanNotifyHook = bcm_mcast_blog_vlan_notify_for_blog_update;
#endif
   return 0;
}

void bcm_mcast_blog_exit(void)
{
#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
    blogRuleVlanNotifyHook = NULL;
#endif   
}

#endif /* CONFIG_BLOG */

