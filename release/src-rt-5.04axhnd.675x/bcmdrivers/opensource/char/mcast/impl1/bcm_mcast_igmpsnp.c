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

#include "bcm_mcast_priv.h"
#include "bcm_mcast_igmpsnp.h"
#include "bcm_mcast_fc.h"

int bcm_mcast_igmp_add_entry(struct net_device *from_dev,
                             int wan_ops,
                             bcm_mcast_ifdata *pif, 
                             struct net_device *dst_dev, 
                             struct net_device *to_accel_dev, 
                             struct in_addr *rxGrp, 
                             struct in_addr *txGrp, 
                             struct in_addr *rep,
                             unsigned char *repMac,
                             unsigned char rep_proto_ver,
                             int mode, 
                             uint16_t tci, 
                             uint16_t grpVid, 
                             struct in_addr *src,
                             int lanppp,
                             int excludePort,
                             char enRtpSeqCheck,
                             uint32_t info)
{
   t_igmp_grp_entry *mc_fdb = NULL;
   t_igmp_rep_entry *rep_entry = NULL;
   struct hlist_head *head = NULL;
   int ret = 0;

   mc_fdb = kmem_cache_alloc(mcast_ctrl->ipv4_grp_cache, GFP_ATOMIC);
   if ( !mc_fdb )
   {
      __logError("Unable to allocate ipv4 group entry");
      return -ENOMEM;
   }

   rep_entry = kmem_cache_alloc(mcast_ctrl->ipv4_rep_cache, GFP_ATOMIC);
   if ( !rep_entry )
   {
      __logError("Unable to allocate ipv4 report entry");
      kmem_cache_free(mcast_ctrl->ipv4_grp_cache, mc_fdb);
      return -ENOMEM;
   }
   mc_fdb->txGrp.s_addr = txGrp->s_addr;
   mc_fdb->rxGrp.s_addr = rxGrp->s_addr;
   memcpy(&mc_fdb->src_entry, src, sizeof(struct in_addr));
   mc_fdb->src_entry.filt_mode = mode;
   mc_fdb->dst_dev = dst_dev;
   mc_fdb->to_accel_dev = to_accel_dev;
   mc_fdb->lan_tci = tci;
   mc_fdb->wan_tci = grpVid;
   mc_fdb->num_tags = 0;
   mc_fdb->from_dev = from_dev;
   mc_fdb->type = wan_ops;
   mc_fdb->excludePort = excludePort;
   mc_fdb->enRtpSeqCheck = enRtpSeqCheck;
   mc_fdb->flowhdl = 0;
   mc_fdb->info = info;
   mc_fdb->lanppp = lanppp;
   INIT_LIST_HEAD(&mc_fdb->rep_list);
   rep_entry->rep.s_addr = rep->s_addr;
   rep_entry->tstamp = jiffies + mcast_ctrl->igmp_general_query_timeout_secs * HZ;
   memcpy(rep_entry->repMac, repMac, ETH_ALEN);
   rep_entry->rep_proto_ver = rep_proto_ver;
   list_add_tail(&rep_entry->list, &mc_fdb->rep_list);
   head = &pif->mc_ipv4_hash[bcm_mcast_igmp_hash(txGrp->s_addr)];
   hlist_add_head(&mc_fdb->hlist, head);

#if defined(CONFIG_BLOG)
   ret = bcm_mcast_create_flow(pif, 
                               (void*)mc_fdb, 
                               BCM_MCAST_PROTO_IPV4, 
                               head,
                               &(mc_fdb->flowhdl));
   if(ret == -1)
   {
      hlist_del(&mc_fdb->hlist);
      kmem_cache_free(mcast_ctrl->ipv4_grp_cache, mc_fdb);
      kmem_cache_free(mcast_ctrl->ipv4_rep_cache, rep_entry);
      __logInfo("Unable to activate blog");
      return ret;
   }
#endif
   bcm_mcast_notify_event(BCM_MCAST_EVT_SNOOP_ADD, BCM_MCAST_PROTO_IPV4, mc_fdb, rep_entry);
   return 0;
}

int bcm_mcast_igmp_del_entry(bcm_mcast_ifdata *pif, 
                              t_igmp_grp_entry *igmp_fdb,
                              struct in_addr   *rep,
                              unsigned char    *repMac)
{
   t_igmp_rep_entry *rep_entry = NULL;
   t_igmp_rep_entry *rep_entry_n = NULL;

   list_for_each_entry_safe(rep_entry, 
                            rep_entry_n, &igmp_fdb->rep_list, list) 
   {
      if (((NULL == rep) && (NULL == repMac)) ||
          (rep && (rep_entry->rep.s_addr == rep->s_addr)) ||
          (repMac && (0 == memcmp(rep_entry->repMac, repMac, ETH_ALEN))))
      {
         if ( pif->igmp_snooping )
         {
             bcm_mcast_netlink_send_igmp_purge_entry(pif, igmp_fdb, rep_entry);
         }
         bcm_mcast_notify_event(BCM_MCAST_EVT_SNOOP_DEL, BCM_MCAST_PROTO_IPV4, igmp_fdb, rep_entry);
         list_del(&rep_entry->list);
         kmem_cache_free(mcast_ctrl->ipv4_rep_cache, rep_entry);
         if (rep || repMac)
         {
            break;
         }
      }
   }
   if(list_empty(&igmp_fdb->rep_list)) 
   {
      hlist_del(&igmp_fdb->hlist);
#if defined(CONFIG_BLOG) 
      bcm_mcast_delete_flow(BCM_MCAST_PROTO_IPV4, &(igmp_fdb->flowhdl));
#endif
      kmem_cache_free(mcast_ctrl->ipv4_grp_cache, igmp_fdb);
      return -ENOENT;
   }

   return 0;
}

void bcm_mcast_igmp_remove_entry(struct net_device *from_dev,
                                 bcm_mcast_ifdata *pif, 
                                 struct net_device *dst_dev, 
                                 struct in_addr *rxGrp, 
                                 struct in_addr *txGrp, 
                                 struct in_addr *rep, 
                                 int mode,
                                 struct in_addr *src,
                                 uint32_t info)
{
   t_igmp_grp_entry *mc_fdb;
   struct hlist_head *head = NULL;
   struct hlist_node *n;

   head = &pif->mc_ipv4_hash[bcm_mcast_igmp_hash(txGrp->s_addr)];
   hlist_for_each_entry_safe(mc_fdb, n, head, hlist)
   {
      if ((mc_fdb->rxGrp.s_addr == rxGrp->s_addr) && 
          (mc_fdb->txGrp.s_addr == txGrp->s_addr) && 
          (mode == mc_fdb->src_entry.filt_mode) && 
          (mc_fdb->src_entry.src.s_addr == src->s_addr) &&
          (mc_fdb->from_dev == from_dev) &&
          (mc_fdb->dst_dev == dst_dev) &&
          (bcm_mcast_blog_cmp_wlan_reporter_info(dst_dev, mc_fdb->info, info) == 0))
      {
         bcm_mcast_igmp_del_entry(pif, mc_fdb, rep, NULL);
      }
   }
}

t_igmp_rep_entry *bcm_mcast_igmp_rep_find(const t_igmp_grp_entry *mc_fdb,
                                          const struct in_addr *rep,
                                          unsigned char *repMac)
{
   t_igmp_rep_entry *rep_entry;

   list_for_each_entry(rep_entry, &mc_fdb->rep_list, list)
   {
      if ((rep && (rep_entry->rep.s_addr == rep->s_addr)) ||
          (repMac && (0 == memcmp(rep_entry->repMac, repMac, ETH_ALEN))))
      {
         return rep_entry;
      }
   }

   return NULL;
}

int bcm_mcast_igmp_update_entry(bcm_mcast_ifdata *pif, 
                                struct net_device *dst_dev, 
                                struct in_addr *rxGrp,
                                struct in_addr *txGrp,
                                struct in_addr *rep,
                                unsigned char *repMac,
                                unsigned char rep_proto_ver,
                                int mode, 
                                struct in_addr *src,
                                struct net_device *from_dev,
                                uint32_t info)
{
    t_igmp_grp_entry *dst;
    int ret = 0;
    struct hlist_head *head;

    head = &pif->mc_ipv4_hash[bcm_mcast_igmp_hash(txGrp->s_addr)];
    hlist_for_each_entry(dst, head, hlist)
    {
        if ( (dst->txGrp.s_addr == txGrp->s_addr) && (dst->rxGrp.s_addr == rxGrp->s_addr) )
        {
            if ( (src->s_addr == dst->src_entry.src.s_addr) &&
                 (mode == dst->src_entry.filt_mode) &&
                 (dst->from_dev == from_dev) &&
                 (dst->dst_dev == dst_dev) &&
                 (bcm_mcast_blog_cmp_wlan_reporter_info(dst_dev, dst->info, info) == 0) )
            {
                /* found entry - update TS */
                t_igmp_rep_entry *reporter = bcm_mcast_igmp_rep_find(dst, rep, NULL);
                if ( reporter == NULL )
                {
                    reporter = kmem_cache_alloc(mcast_ctrl->ipv4_rep_cache, GFP_ATOMIC);
                    if ( reporter )
                    {
                        reporter->rep.s_addr = rep->s_addr;
                        reporter->tstamp = jiffies + (mcast_ctrl->igmp_general_query_timeout_secs * HZ);
                        memcpy(reporter->repMac, repMac, ETH_ALEN);
                        reporter->rep_proto_ver = rep_proto_ver;
                        list_add_tail(&reporter->list, &dst->rep_list);
                        bcm_mcast_notify_event(BCM_MCAST_EVT_SNOOP_ADD, BCM_MCAST_PROTO_IPV4, dst, reporter);
                        bcm_mcast_igmp_set_timer(pif);
                    }
                } 
                else
                {
                    reporter->tstamp = jiffies + (mcast_ctrl->igmp_general_query_timeout_secs * HZ);
                    bcm_mcast_igmp_set_timer(pif);
                }
                ret = 1;
            }
        }
    }

    return ret;
}
