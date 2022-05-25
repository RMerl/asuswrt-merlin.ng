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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>

#include "bcm_mcast_priv.h"
#include "bcm_mcast_igmpsnp.h"

/*****
  all functions in this file must be called with the lock already held 
 *****/

int bcm_mcast_igmp_control_filter(__be32 dest_ip)
{
   struct bcm_mcast_control_filter_entry *filter_entry = NULL;

   // The range 224.0.0.x is always blocked
   if ((dest_ip & htonl(0xFFFFFF00)) == htonl(0xE0000000))
   {
      return 0;
   }
   spin_lock_bh(&mcast_ctrl->cfgLock);
   hlist_for_each_entry(filter_entry, &mcast_ctrl->igmp_snoopExceptionList, node)
   {
      if ( (dest_ip & filter_entry->mask.s6_addr32[0]) == (filter_entry->group.s6_addr32[0] & filter_entry->mask.s6_addr32[0]) )
      {
         spin_unlock_bh(&mcast_ctrl->cfgLock);
         return 0;
      }
   }
   spin_unlock_bh(&mcast_ctrl->cfgLock);
   return 1;
} /* bcm_mcast_igmp_control_filter */


void bcm_mcast_igmp_wipe_ignore_group_list ( void )
{
   struct bcm_mcast_control_filter_entry *filter_entry = NULL;
   struct hlist_node *n_group;

   hlist_for_each_entry_safe(filter_entry, n_group, &mcast_ctrl->igmp_snoopExceptionList, node)
   {
      hlist_del(&filter_entry->node);
      kmem_cache_free(mcast_ctrl->ipv4_exception_cache, filter_entry);
   }
}

int bcm_mcast_igmp_process_ignore_group_list (int count, t_BCM_MCAST_IGNORE_GROUP_ENTRY* ignoreGroupEntries)
{
   int inputIndex = 0;
   int ret = 0;
   
   spin_lock_bh(&mcast_ctrl->cfgLock);
   bcm_mcast_igmp_wipe_ignore_group_list();
   
   for ( ; inputIndex < count; inputIndex ++) {
      if ((ignoreGroupEntries[inputIndex].mask.s6_addr32[0] & htonl(0xF0000000)) != htonl(0xF0000000) ) 
      {
         ret = -EINVAL;
      }
      else if ((ignoreGroupEntries[inputIndex].address.s6_addr32[0] & htonl(0xF0000000)) != htonl(0xE0000000) ) 
      {
         ret = -EINVAL;
      }
      else 
      {
         struct bcm_mcast_control_filter_entry *newFilter = kmem_cache_alloc(mcast_ctrl->ipv4_exception_cache, GFP_ATOMIC);
         if (newFilter) 
         {
            newFilter->group.s6_addr32[0] = ignoreGroupEntries[inputIndex].address.s6_addr32[0];
            newFilter->mask.s6_addr32[0] = ignoreGroupEntries[inputIndex].mask.s6_addr32[0];
            hlist_add_head(&newFilter->node, &mcast_ctrl->igmp_snoopExceptionList);
         }
         else 
         {
            ret = -ENOMEM;
         }
      }
   }
   spin_unlock_bh(&mcast_ctrl->cfgLock);

   return ret;
}

void bcm_mcast_igmp_set_timer( bcm_mcast_ifdata *pif)
{
   t_igmp_grp_entry *mcast_group;
   int               i;
   unsigned long     tstamp = jiffies + (mcast_ctrl->igmp_general_query_timeout_secs * HZ * 2);
   unsigned int      found = 0;
   int               pendingIndex;
   unsigned long     skbTimeout = jiffies + (BCM_MCAST_NETLINK_SKB_TIMEOUT_MS*2);

   if (pif->brtype == BRTYPE_OVS) 
   {
       /* No snoop aging timers required for OvS bridges. OvS will handle snooping
          Only acceleration support is needed in multicast driver */
       return;
   }

   found = 0;
   for ( pendingIndex = 0; pendingIndex < BCM_MCAST_MAX_DELAYED_SKB_COUNT; pendingIndex ++)
   {
      if ( (pif->igmp_delayed_skb[pendingIndex].skb != NULL) && 
           (time_after(skbTimeout, pif->igmp_delayed_skb[pendingIndex].expiryTime)) )
      {
         skbTimeout = pif->igmp_delayed_skb[pendingIndex].expiryTime;
         found = 1;
      }
   } 

   if ( (BCM_MCAST_SNOOPING_DISABLED_FLOOD == pif->igmp_snooping) && (0 == found))
   {
      del_timer(&pif->igmp_timer);
      return;
   }

   if (found) 
   {
      if (time_after(tstamp, skbTimeout) )
      {
         tstamp = skbTimeout;
      }
   }

   if ( pif->igmp_snooping != 0 ) 
   {
      for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
      {
         hlist_for_each_entry(mcast_group, &pif->mc_ipv4_hash[i], hlist) 
         {
            t_igmp_rep_entry *reporter_group;
            list_for_each_entry(reporter_group, &mcast_group->rep_list, list)
            {
               if ( time_after(tstamp, reporter_group->tstamp) )
               {
                  tstamp = reporter_group->tstamp;
                  found  = 1;
               }
            }
         }
      }
   }
  
   if ( 0 == found )
   {
      del_timer(&pif->igmp_timer);
   }
   else
   {
      mod_timer(&pif->igmp_timer, (tstamp + BCM_MCAST_FDB_TIMER_TIMEOUT));
   }
}

int bcm_mcast_igmp_wipe_group(bcm_mcast_ifdata *parent_if, int dest_ifindex, struct in_addr *gpAddr)
{
   t_igmp_grp_entry *mcast_group;
   int               i;
   int               retVal = -EINVAL;
 
   struct net_device* destDev = dev_get_by_index (&init_net, dest_ifindex);
   if (NULL != destDev)
   {
      spin_lock_bh(&parent_if->mc_igmp_lock);
      retVal = 0;
      for (i = 0; i < BCM_MCAST_HASH_SIZE; i++)
      {
         struct hlist_node *n_group;
         hlist_for_each_entry_safe(mcast_group, n_group, &parent_if->mc_ipv4_hash[i], hlist)
         {
            t_igmp_rep_entry *reporter_group, *n_reporter;
            list_for_each_entry_safe(reporter_group, n_reporter, &mcast_group->rep_list, list)
            {
               if ((mcast_group->rxGrp.s_addr == gpAddr->s_addr) && (mcast_group->dst_dev == destDev))
               {
                   __logInfo("Invoke bcm_mcast_igmp_del_entry for grp 0x%x rep 0x%x dst_dev %s",
                             htonl(gpAddr->s_addr), htonl(reporter_group->rep.s_addr), destDev->name);
                   if (bcm_mcast_igmp_del_entry(parent_if, mcast_group, &reporter_group->rep, NULL))
                  {
                     break;
                  }
               }
            }
         }
      }
      bcm_mcast_igmp_set_timer(parent_if);
      spin_unlock_bh(&parent_if->mc_igmp_lock);
      dev_put(destDev);
   }
   return retVal;
}

static void bcm_mcast_igmp_reporter_timeout(bcm_mcast_ifdata *pif)
{
   t_igmp_grp_entry *mcast_group;
   int               i;
    
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      struct hlist_node *n_group;
      hlist_for_each_entry_safe(mcast_group, n_group, &pif->mc_ipv4_hash[i], hlist) 
      {
         t_igmp_rep_entry *reporter_group, *n_reporter;
         list_for_each_entry_safe(reporter_group, n_reporter, &mcast_group->rep_list, list)
         {
            if (time_after_eq(jiffies, reporter_group->tstamp)) 
            {
               if (bcm_mcast_igmp_del_entry(pif, mcast_group, &reporter_group->rep, NULL))
               {
                  break;
               }
            }
         }
      }
   }

}

static void bcm_mcast_igmp_admission_timeout(bcm_mcast_ifdata *pif)
{
   int pendingIndex;

   for (pendingIndex = 0; pendingIndex < BCM_MCAST_MAX_DELAYED_SKB_COUNT; pendingIndex ++)
   {
      if (( pif->igmp_delayed_skb[pendingIndex].skb != NULL ) && 
          ( time_before(pif->igmp_delayed_skb[pendingIndex].expiryTime, jiffies) ) )
      {
         kfree_skb(pif->igmp_delayed_skb[pendingIndex].skb);
         pif->igmp_delayed_skb[pendingIndex].skb = NULL;
         pif->igmp_delayed_skb[pendingIndex].expiryTime = 0;
      }
   }    
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static void bcm_mcast_igmp_timer(struct timer_list *t)
#else
static void bcm_mcast_igmp_timer(unsigned long param)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   bcm_mcast_ifdata *pif = from_timer(pif, t, igmp_timer);
   int               ifindex = pif->ifindex;
#else
   int               ifindex = (int)param;
   bcm_mcast_ifdata *pif;
#endif

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(ifindex);
   if ( NULL == pif )
   {
      rcu_read_unlock();
      __logError("unknown interface");
      return;
   }

   spin_lock_bh(&pif->mc_igmp_lock);
   bcm_mcast_igmp_reporter_timeout(pif);
   bcm_mcast_igmp_admission_timeout(pif);
   bcm_mcast_igmp_set_timer(pif);
   spin_unlock_bh(&pif->mc_igmp_lock);
   rcu_read_unlock();
}

void bcm_mcast_igmp_init(bcm_mcast_ifdata *pif)
{
   int i;
   spin_lock_init(&pif->mc_igmp_lock);
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++ )
   {
      INIT_HLIST_HEAD(&pif->mc_ipv4_hash[i]);
   }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   timer_setup(&pif->igmp_timer, bcm_mcast_igmp_timer, 0);
#else
   setup_timer(&pif->igmp_timer, bcm_mcast_igmp_timer, (unsigned long)pif->ifindex);
#endif
   pif->igmp_snooping = BCM_MCAST_SNOOPING_BLOCKING_MODE;
   pif->igmp_rate_limit = 0;
}

void bcm_mcast_igmp_fini(bcm_mcast_ifdata *pif)
{
   del_timer_sync(&pif->igmp_timer);
}

int bcm_mcast_igmp_admission_queue(bcm_mcast_ifdata *pif, struct sk_buff *skb)
{
   int q_index;
   int index = -1;

   spin_lock_bh(&pif->mc_igmp_lock);
   for (q_index = 0; q_index < BCM_MCAST_MAX_DELAYED_SKB_COUNT; q_index ++) 
   {
      if ( pif->igmp_delayed_skb[q_index].skb == NULL ) 
      {
         pif->igmp_delayed_skb[q_index].skb = skb;
         pif->igmp_delayed_skb[q_index].expiryTime = jiffies + BCM_MCAST_NETLINK_SKB_TIMEOUT_MS;
         index = q_index;
         bcm_mcast_igmp_set_timer(pif);
         break;
      }
   }
   spin_unlock_bh(&pif->mc_igmp_lock);

   if ( -1 == index )
   {
      __logNotice("unable to queue skb"); 
   }
   return index;
}

void bcm_mcast_igmp_admission_update_bydev(bcm_mcast_ifdata *pif, struct net_device *dev)
{
   int pendingIndex;
   __logInfo("flushing skbs from queue for dev %s", dev ? dev->name : "all");
   for ( pendingIndex = 0; pendingIndex < BCM_MCAST_MAX_DELAYED_SKB_COUNT; pendingIndex++)
   {
      if ( (pif->igmp_delayed_skb[pendingIndex].skb != NULL) &&
           ((NULL == dev) || (pif->igmp_delayed_skb[pendingIndex].skb->dev == dev)) )
      {
         kfree_skb(pif->igmp_delayed_skb[pendingIndex].skb);
         pif->igmp_delayed_skb[pendingIndex].skb = NULL;
         pif->igmp_delayed_skb[pendingIndex].expiryTime = 0;
      }
   }
}

int bcm_mcast_igmp_admission_process(bcm_mcast_ifdata *pif, int packet_index, int admitted)
{
   struct sk_buff *skb;

   if ( (packet_index < 0) || (packet_index >= BCM_MCAST_MAX_DELAYED_SKB_COUNT))
   {
       __logNotice("skb for index %d not set", packet_index);
       return -1;
   }

   skb = pif->igmp_delayed_skb[packet_index].skb;
   if ( skb )
   {
      if (true == admitted)
      {
         /* send the packet on - called with rcu read lock */
         __logNotice("skb for index %d has been admitted", packet_index);
         br_bcm_mcast_flood_forward(pif->dev, skb);
      }
      else
      {
         __logNotice("skb for index %d has been dropped", packet_index);
         kfree_skb(skb);
      }
      pif->igmp_delayed_skb[packet_index].skb = NULL;
   }
   else
   {
      __logNotice("skb for index %d not set", packet_index);
   }

   return 0;
}

void bcm_mcast_igmp_update_bydev( bcm_mcast_ifdata *pif, struct net_device *dev, int activate)
{
   t_igmp_grp_entry *mc_fdb;
   int i;
#if defined(CONFIG_BLOG)
   int ret;
#endif

   __logInfo("flushing entries for dev %s activate %d", dev ? dev->name : "all", activate);

   if(NULL == pif)
   {
      return;
   }


   /* remove all entries if dev is NULL or activate is false otherwise 
      deactivate root entries and remove all other entries involving dev */
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      struct hlist_node *n;
      hlist_for_each_entry_safe(mc_fdb, n, &pif->mc_ipv4_hash[i], hlist) 
      {
         if (NULL == dev)
         {
             __logInfo("Invoke bcm_mcast_igmp_del_entry for grp 0x%x dstdev %s srcdev %s num_tags %d dev NULL",
                       htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                       ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                       ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                       ((t_igmp_grp_entry *)mc_fdb)->num_tags);
             bcm_mcast_igmp_del_entry(pif, mc_fdb, NULL, NULL);
         }
         else if ( (mc_fdb->dst_dev == dev) ||
                   (mc_fdb->from_dev == dev) )
         {
#if defined(CONFIG_BLOG)
            if (0 == activate)
            {
                __logInfo("Invoke bcm_mcast_igmp_del_entry for grp 0x%x dstdev %s srcdev %s num_tags %d dev %s",
                          htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                          ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->num_tags,
                          dev->name);
                bcm_mcast_igmp_del_entry(pif, mc_fdb, NULL, NULL);
            }
            else
            {
                __logInfo("Invoke bcm_mcast_delete_flow for grp 0x%x dstdev %s srcdev %s num_tags %d dev %s",
                          htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                          ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->num_tags,
                          dev->name);
                bcm_mcast_delete_flow(BCM_MCAST_PROTO_IPV4, &(mc_fdb->flowhdl));
            }
#else
            bcm_mcast_igmp_del_entry(pif, mc_fdb, NULL, NULL);
#endif
         }
      }
   }

   /* activate entries for dev */
#if defined(CONFIG_BLOG)
   if ((dev != NULL) && (1 == activate))
   {
      for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
      {
         struct hlist_node *n;
         hlist_for_each_entry_safe(mc_fdb, n, &pif->mc_ipv4_hash[i], hlist) 
         { 
            if ( (mc_fdb->dst_dev == dev) ||
                 (mc_fdb->from_dev == dev) )
            {
               ret = bcm_mcast_create_flow(pif, 
                                           (void*)mc_fdb, 
                                           BCM_MCAST_PROTO_IPV4, 
                                           &pif->mc_ipv4_hash[bcm_mcast_igmp_hash(mc_fdb->txGrp.s_addr)],
                                           &(mc_fdb->flowhdl));
               if(ret < 0)
               {
                  /* bcm_mcast_blog_process may return -1 if there are no blog rules
                   * which may be a valid scenario, in which case we delete the
                   * multicast entry.
                   */
                  bcm_mcast_igmp_del_entry(pif, mc_fdb, NULL, NULL);
                  __logInfo("Unable to activate blog");
               }
            }
         }
      }
   }
#endif   
   bcm_mcast_igmp_set_timer(pif);

   return;
}

void bcm_mcast_igmp_wipe_reporter_for_port (bcm_mcast_ifdata *pif,
                                            struct in_addr *rep, 
                                            struct net_device *rep_dev)
{
    int hashIndex;
    struct hlist_node *n = NULL;
    struct hlist_head *head = NULL;
    t_igmp_grp_entry *mc_fdb;

    __logInfo("Wiping IGMP snoop entries for Reporter 0x%x reporter dev %s pif %s", 
              rep->s_addr, rep_dev ? rep_dev->name:"NULL", pif ? pif->dev->name : "NULL");
    spin_lock_bh(&pif->mc_igmp_lock);
    for (hashIndex = 0; hashIndex < BCM_MCAST_HASH_SIZE ; hashIndex++)
    {
        head = &pif->mc_ipv4_hash[hashIndex];
        hlist_for_each_entry_safe(mc_fdb, n, head, hlist)
        {
            if ((mc_fdb->dst_dev == rep_dev) &&
                (bcm_mcast_igmp_rep_find(mc_fdb, rep, NULL) != NULL))
            {
                /* The reporter we're looking for has been found
                   in a record pointing to its old port */
                __logInfo("Invoke bcm_mcast_igmp_del_entry for grp 0x%x dstdev %s srcdev %s num_tags %d client 0x%x",
                          htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                          ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->num_tags,
                          rep->s_addr);
                bcm_mcast_igmp_del_entry(pif, mc_fdb, rep, NULL);
            }
        }
    }

    bcm_mcast_igmp_set_timer(pif);
    spin_unlock_bh(&pif->mc_igmp_lock);
}

void bcm_mcast_igmp_wipe_reporter_by_mac(bcm_mcast_ifdata *pif,
                                         unsigned char *repMac)
{
    int hashIndex;
    struct hlist_node *n = NULL;
    struct hlist_head *head = NULL;
    t_igmp_grp_entry *mc_fdb;

    __logInfo("Wiping IGMP snoop entries for MAC %pM pif %s", 
              repMac, pif ? pif->dev->name : "NULL");

    for ( hashIndex = 0; hashIndex < BCM_MCAST_HASH_SIZE ; hashIndex++)
    {
        head = &pif->mc_ipv4_hash[hashIndex];
        hlist_for_each_entry_safe(mc_fdb, n, head, hlist)
        {
            if ((bcm_mcast_igmp_rep_find(mc_fdb, NULL, repMac) != NULL))
            {
                __logInfo("Invoke bcm_mcast_igmp_del_entry for grp 0x%x dstdev %s srcdev %s num_tags %d repMac %pM",
                          htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                          ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->num_tags,
                          repMac);
                bcm_mcast_igmp_del_entry(pif, mc_fdb, NULL, repMac);
            }
        }
    }
    bcm_mcast_igmp_set_timer(pif);
}

void bcm_mcast_igmp_wipe_grp_reporter_for_port (bcm_mcast_ifdata *pif,
                                                struct in_addr *grp, 
                                                struct in_addr *rep, 
                                                struct net_device *rep_dev)
{
    int hashIndex;
    struct hlist_node *n = NULL;
    struct hlist_head *head = NULL;
    t_igmp_grp_entry *mc_fdb;

    __logInfo("Wiping IGMP snoop entries for Grp 0x%x Reporter 0x%x reporter dev %s pif %s", 
              grp->s_addr, rep->s_addr, rep_dev ? rep_dev->name:"NULL", pif ? pif->dev->name : "NULL");

    spin_lock_bh(&pif->mc_igmp_lock);
    for (hashIndex = 0; hashIndex < BCM_MCAST_HASH_SIZE ; hashIndex++)
    {
        head = &pif->mc_ipv4_hash[hashIndex];
        hlist_for_each_entry_safe(mc_fdb, n, head, hlist)
        {
            if ((mc_fdb->dst_dev == rep_dev) &&
                (grp && (mc_fdb->rxGrp.s_addr == grp->s_addr)) &&
                (bcm_mcast_igmp_rep_find(mc_fdb, rep, NULL) != NULL))
            {
                /* The reporter we're looking for has been found
                   in a record pointing to its old port */
                __logInfo("Invoke bcm_mcast_igmp_del_entry for grp 0x%x dstdev %s srcdev %s num_tags %d client 0x%x",
                          htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                          ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->num_tags,
                          rep->s_addr);
                bcm_mcast_igmp_del_entry(pif, mc_fdb, rep, NULL);
            }
        }
    }

    bcm_mcast_igmp_set_timer(pif);
    spin_unlock_bh(&pif->mc_igmp_lock);
}

#if defined(CONFIG_BLOG)
void bcm_mcast_igmp_process_blog_enable( bcm_mcast_ifdata *pif, int enable )
{
   t_igmp_grp_entry *mc_fdb;
   int i;
   int ret;

   __logInfo("process blog enable setting %d", enable);

   if(NULL == pif)
   {
      return;
   }

   /* remove all entries if dev is NULL or activate is false otherwise 
      deactivate root entries and remove all other entries involving dev */
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      struct hlist_node *n;
      hlist_for_each_entry_safe(mc_fdb, n, &pif->mc_ipv4_hash[i], hlist) 
      {
         if ( 0 == enable )
         {
                __logInfo("Invoke bcm_mcast_delete_flow for grp 0x%x dstdev %s srcdev %s num_tags %d",
                          htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                          ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->num_tags);
                bcm_mcast_delete_flow(BCM_MCAST_PROTO_IPV4, &(mc_fdb->flowhdl));
         }
         else
         {
             ret = bcm_mcast_create_flow(pif, 
                                         (void*)mc_fdb, 
                                         BCM_MCAST_PROTO_IPV4, 
                                         &pif->mc_ipv4_hash[bcm_mcast_igmp_hash(mc_fdb->txGrp.s_addr)],
                                         &(mc_fdb->flowhdl));
             if(ret < 0)
             {
                 /* bcm_mcast_blog_process may return -1 if there are no blog rules
                  * which may be a valid scenario, in which case we delete the
                  * multicast entry.
                  */
                 __logInfo("Unable to create flow, Invoke bcm_mcast_igmp_del_entry for grp 0x%x dstdev %s srcdev %s num_tags %d",
                          htonl(((t_igmp_grp_entry *)mc_fdb)->rxGrp.s_addr),
                          ((t_igmp_grp_entry *)mc_fdb)->dst_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->from_dev->name,
                          ((t_igmp_grp_entry *)mc_fdb)->num_tags);
                  bcm_mcast_igmp_del_entry(pif, mc_fdb, NULL, NULL);
             }
         }
      }
   }
   bcm_mcast_igmp_set_timer(pif);

   return;
}
#endif

int bcm_mcast_igmp_add(struct net_device *from_dev,
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
   if(!pif || !dst_dev || !rxGrp || !txGrp || !rep || !from_dev)
      return 0;

   __logInfo("from %s pif %s dst %s to_accel_dev %s rxGrp 0x%x txGrp 0x%x Client 0x%x mode %d src 0x%x info 0x%x clientMac %pM",
              from_dev ? from_dev->name:"NULL", pif ? (pif->dev?pif->dev->name:"NULL"):"NULL",
              dst_dev ? dst_dev->name : "NULL", to_accel_dev ? to_accel_dev->name:"NULL",
              htonl(rxGrp->s_addr), htonl(txGrp->s_addr), htonl(rep->s_addr), mode, htonl(src->s_addr), info, repMac);

   if( !bcm_mcast_igmp_control_filter(rxGrp->s_addr) )
   {
      __logDebug("IGMP control filter applied rxGrp->s_addr 0x%x", htonl(rxGrp->s_addr));
      return 0;
   }

   if( !bcm_mcast_igmp_control_filter(txGrp->s_addr) )
   {
      __logDebug("IGMP control filter applied txGrp->s_addr 0x%x", htonl(txGrp->s_addr));
      return 0;
   }

   if((MCAST_INCLUDE != mode) && (MCAST_EXCLUDE != mode))
   {
      __logDebug("mode %d not supported", mode);
      return 0;
   }

   spin_lock_bh(&pif->mc_igmp_lock);
   if (bcm_mcast_igmp_update_entry(pif, dst_dev, rxGrp, txGrp, rep, repMac, rep_proto_ver, mode, src, from_dev, info))
   {
      spin_unlock_bh(&pif->mc_igmp_lock);
      return 0;
   }

   if (bcm_mcast_igmp_add_entry(from_dev,
                                wan_ops,
                                pif, 
                                dst_dev, 
                                to_accel_dev, 
                                rxGrp, 
                                txGrp, 
                                rep,
                                repMac,
                                rep_proto_ver,
                                mode, 
                                tci, 
                                grpVid,
                                src,
                                lanppp,
                                excludePort,
                                enRtpSeqCheck,
                                info) != 0)
   {
      spin_unlock_bh(&pif->mc_igmp_lock);
      return -1;
   }

   bcm_mcast_igmp_set_timer(pif);
   spin_unlock_bh(&pif->mc_igmp_lock);
   return 0;
}

int bcm_mcast_igmp_remove(struct net_device *from_dev,
                          bcm_mcast_ifdata *pif, 
                          struct net_device *dst_dev, 
                          struct in_addr *rxGrp, 
                          struct in_addr *txGrp, 
                          struct in_addr *rep, 
                          int mode, 
                          struct in_addr *src,
                          uint32_t info)
{
   if(!pif || !dst_dev || !txGrp || !rxGrp || !rep || !from_dev)
      return 0;

   __logInfo("from %s pif %s dst %s rxGrp 0x%x txGrp 0x%x Client 0x%x mode %d src 0x%x info 0x%x",
              from_dev ? from_dev->name:"NULL", pif ? (pif->dev?pif->dev->name:"NULL"):"NULL",
              dst_dev ? dst_dev->name : "NULL", htonl(rxGrp->s_addr), htonl(txGrp->s_addr), 
              htonl(rep->s_addr), mode, htonl(src->s_addr), info);

   if(!bcm_mcast_igmp_control_filter(txGrp->s_addr))
   {
      __logDebug("IGMP control filter applied txGrp->s_addr 0x%x", 
                 htonl(txGrp->s_addr));
      return 0;
   }

   if(!bcm_mcast_igmp_control_filter(rxGrp->s_addr))
   {
      __logDebug("IGMP control filter applied rxGrp->s_addr 0x%x", 
                 htonl(rxGrp->s_addr));
      return 0;
   }

   if((MCAST_INCLUDE != mode) && (MCAST_EXCLUDE != mode))
   {
      __logDebug("mode %d not supported", mode);
      return 0;
   }

   spin_lock_bh(&pif->mc_igmp_lock);
   bcm_mcast_igmp_remove_entry(from_dev,
                               pif, 
                               dst_dev, 
                               rxGrp, 
                               txGrp, 
                               rep, 
                               mode,
                               src,
                               info);
   bcm_mcast_igmp_set_timer(pif);
   spin_unlock_bh(&pif->mc_igmp_lock);
   return 0;
}

int bcm_mcast_igmp_should_deliver(bcm_mcast_ifdata *pif, 
                                  const struct iphdr *pip,
                                  struct net_device *src_dev,
                                  struct net_device *dst_dev,
                                  int pkt_type)
{
   t_igmp_grp_entry  *dst;
   struct hlist_head *head = NULL;
   int                should_deliver;

   if (0 == bcm_mcast_igmp_control_filter(pip->daddr))
   {
      /* accept packet */
        __logDebug("source device %s, dst device %s control filter applied", src_dev->name, dst_dev->name);
      return 1;
   }

   /* drop traffic by default when snooping is enabled
      in blocking mode */
   if (BCM_MCAST_SNOOPING_BLOCKING_MODE == pif->igmp_snooping)
   {
      should_deliver = 0;
   }
   else
   {
      should_deliver = 1;
   }

   /* adhere to forwarding entries regardless of snooping mode */

   spin_lock_bh(&pif->mc_igmp_lock);
   head = &pif->mc_ipv4_hash[bcm_mcast_igmp_hash(pip->daddr)];
   hlist_for_each_entry(dst, head, hlist)
   {
      if ( (dst->txGrp.s_addr != pip->daddr) ||
           (dst->dst_dev != dst_dev) )
      {
         continue;
      }

      /* Forward as long as the snooping entry matches the group info for now.
         TODO: The plan is to have the snooping entries maintain a list of
               allowed source interfaces received from mcpd.conf. ANY would be one
               of the options in mcpd.conf which would enable the multicast driver
               to allow all different source interfaces */

      /* if this is an include entry source must match */
      if(dst->src_entry.filt_mode == MCAST_INCLUDE)
      {
         if (pip->saddr == dst->src_entry.src.s_addr)
         {
            /* matching entry */
            should_deliver = 1;
            break;
         }
      }
      else
      {
         /* exclude filter - exclude source not supported RFC4607/5771 */
         should_deliver = 1;
         break;
      }
   }
   spin_unlock_bh(&pif->mc_igmp_lock);
   return should_deliver;
}

static void bcm_mcast_igmp_display_entry(struct seq_file *seq,
                                         bcm_mcast_ifdata *pif,
                                         t_igmp_grp_entry *dst)
{
   t_igmp_rep_entry *rep_entry;
   int               first;
   int               tstamp;
   unsigned char    *txAddressP = (unsigned char *)&(dst->txGrp.s_addr);
   unsigned char    *rxAddressP = (unsigned char *)&(dst->rxGrp.s_addr);
   unsigned char    *srcAddressP = (unsigned char *)&(dst->src_entry.src.s_addr);

   seq_printf(seq, "%-6s %-6s %-6s %-7s %4d %02d    0x%04x   0x%04x%04x 0x%x",
              pif->dev->name, 
              dst->dst_dev->name, 
              dst->to_accel_dev->name, 
              dst->from_dev->name,
              dst->type,
              dst->num_tags,
              ntohs(dst->lan_tci),
              ((dst->wan_tci >> 16) & 0xFFFF),
              (dst->wan_tci & 0xFFFF),
              dst->info);

   seq_printf(seq, " %03u.%03u.%03u.%03u", txAddressP[0],txAddressP[1],txAddressP[2],txAddressP[3]);

   seq_printf(seq, " %-4s %03u.%03u.%03u.%03u %03u.%03u.%03u.%03u", 
              (dst->src_entry.filt_mode == MCAST_EXCLUDE) ? 
              "EX" : "IN",  
              rxAddressP[0],rxAddressP[1],rxAddressP[2],rxAddressP[3], 
              srcAddressP[0],srcAddressP[1],srcAddressP[2],srcAddressP[3] );

   first = 1;
   list_for_each_entry(rep_entry, &dst->rep_list, list)
   { 
      unsigned char *repAddressP = (unsigned char *)&(rep_entry->rep.s_addr);

      if ( 0 == pif->igmp_snooping )
      {
         tstamp = 0;
      }
      else
      {
         tstamp = (int)(rep_entry->tstamp - jiffies) / HZ;
      }

      if(first)
      {
#if defined(CONFIG_BLOG)
         bcm_mcast_flowhdl_t *flowhdl_p = (bcm_mcast_flowhdl_t *)dst->flowhdl;
         seq_printf(seq, " %03u.%03u.%03u.%03u %-7d %-5d  %d \n", 
                    repAddressP[0],repAddressP[1],repAddressP[2],repAddressP[3],
                    tstamp, dst->excludePort, flowhdl_p ? flowhdl_p->numkeys : -1);
#else
         seq_printf(seq, " %03u.%03u.%03u.%03u %-7d %d\n",
                    repAddressP[0],repAddressP[1],repAddressP[2],repAddressP[3],
                    tstamp, dst->excludePort);
#endif
         first = 0;
      }
      else
      {
         seq_printf(seq, "%100s %03u.%03u.%03u.%03u %-7d\n", " ", 
                    repAddressP[0],repAddressP[1],repAddressP[2],repAddressP[3],
                    tstamp);
      }
   }
}

int bcm_mcast_igmp_display(struct seq_file *seq, bcm_mcast_ifdata *pif)
{
   int i;
   bcm_mcast_lower_port* lowerPort = NULL;

   seq_printf(seq, "Bridge %s igmp snooping %d  rate-limit %dpps, priority %d brtype %s\n",
              pif->dev->name,
              pif->igmp_snooping, 
              pif->igmp_rate_limit,
              mcast_ctrl->mcastPriQueue,
              pif->brtype ? "OVS":"Linux");
   seq_printf(seq, " Port Name    Querier      Timeout\n");
   hlist_for_each_entry_rcu (lowerPort, &pif->lower_port_list, hlist)
   {
      int tstamp = lowerPort->querying_port ? (int)(lowerPort->querying_port_timer.expires - jiffies) / HZ : 0;
      struct net_device* lowerDev = dev_get_by_index(&init_net, lowerPort->port_ifindex);
      if (lowerDev) 
      {
         seq_printf(seq, "    %-6s        %-3s          %03d\n", lowerDev->name,
                                        lowerPort->querying_port ? "YES" : " NO", 
                                        tstamp);
         dev_put(lowerDev);
      }
   }
   seq_printf(seq, "bridge dstdev dstaccdev src-dev type #tags lan-tci  wan-tci info");
#if defined(CONFIG_BLOG)
   seq_printf(seq, "    group           mode RxGroup         source          reporter        timeout ExcludPt Index      \n");
#else
   seq_printf(seq, "    group           mode RxGroup         source          reporter        timeout ExcludPt\n");
#endif

   spin_lock_bh(&pif->mc_igmp_lock);
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      t_igmp_grp_entry *entry;
      hlist_for_each_entry(entry, &pif->mc_ipv4_hash[i], hlist) 
      {
         bcm_mcast_igmp_display_entry(seq, pif, entry);
      }
   }
   spin_unlock_bh(&pif->mc_igmp_lock);

   if (bcmLog_logIsEnabled(BCM_LOG_ID_MCAST, BCM_LOG_LEVEL_INFO)) 
   {
       bcm_mcast_fc_flowhdl_dump();
       bcm_mcast_dump_grpinfo_all();
   }
   return 0;
}

