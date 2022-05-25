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
#include "bcm_mcast_mldsnp.h"

int bcm_mcast_mld_control_filter(const struct in6_addr *ipv6)
{
   struct bcm_mcast_control_filter_entry *filter_entry = NULL;
   int retVal = 1;

    /* (1)ignore any packets that are not multicast and also not L2 multicast,
         (PON IPTV support L2 mcast which the lookup_method of obj set as mac or mac_vid.
          To support IPV6 L2 mcast feature of HW, here let L2 mcast pass.
          More details please refer pktrunner_blog_parse_mcast_group())
       (2)ignore scope0, 
       (3)ignore node and link local address,
       (4)ignore IPv6 all DHCP servers address
   */
   if(!(BCM_IN6_IS_ADDR_MULTICAST(ipv6) ||
        BCM_IN6_IS_ADDR_L2_MCAST(ipv6)) ||
       (BCM_IN6_IS_ADDR_MC_SCOPE0(ipv6)) ||
       (BCM_IN6_IS_ADDR_MC_NODELOCAL(ipv6)) ||
       (BCM_IN6_IS_ADDR_MC_LINKLOCAL(ipv6)))
   {
      /* do not process these groups */
       retVal = 0;
   }
   else 
   {
      spin_lock_bh(&mcast_ctrl->cfgLock);
      hlist_for_each_entry(filter_entry, &mcast_ctrl->mld_snoopExceptionList, node)
      {
         int wordIndex = 0;
         for ( ; wordIndex < 4; wordIndex ++)
         {
            if ( (ipv6->s6_addr32[wordIndex] & filter_entry->mask.s6_addr32[wordIndex]) != 
                 (filter_entry->group.s6_addr32[wordIndex] & filter_entry->mask.s6_addr32[wordIndex] ) )
            {
               break;
            }
         }
         if (wordIndex == 4) 
         {
            retVal = 0;
            break;
         }
      }
      spin_unlock_bh(&mcast_ctrl->cfgLock);
   }   
   return retVal;
}

void bcm_mcast_mld_wipe_ignore_group_list ( void )
{
   struct bcm_mcast_control_filter_entry *filter_entry = NULL;
   struct hlist_node *n_group;

   hlist_for_each_entry_safe(filter_entry, n_group, &mcast_ctrl->mld_snoopExceptionList, node)
   {
      hlist_del(&filter_entry->node);
      kmem_cache_free(mcast_ctrl->ipv6_exception_cache, filter_entry);
   }
}


int bcm_mcast_mld_process_ignore_group_list (int count, t_BCM_MCAST_IGNORE_GROUP_ENTRY* ignoreMsgPtr)
{
   int inputIndex = 0;
   int ret = 0;
   
   spin_lock_bh(&mcast_ctrl->cfgLock);
   bcm_mcast_mld_wipe_ignore_group_list();

   for ( ; inputIndex < count; inputIndex ++)
   {
      if ((ignoreMsgPtr[inputIndex].mask.s6_addr32[0] & htonl(0xFF000000)) != htonl(0xFF000000) ) 
      {
         ret = -EINVAL;
      }
      else if ((ignoreMsgPtr[inputIndex].address.s6_addr32[0] & htonl(0xFF000000)) != htonl(0xFF000000) ) 
      {
         ret = -EINVAL;
      }
      else 
      {
         struct bcm_mcast_control_filter_entry *newFilter = kmem_cache_alloc(mcast_ctrl->ipv6_exception_cache, GFP_ATOMIC);
         if (newFilter) 
         {
            memcpy(newFilter->group.s6_addr32, ignoreMsgPtr[inputIndex].address.s6_addr32, sizeof(newFilter->group));
            memcpy(newFilter->mask.s6_addr32, ignoreMsgPtr[inputIndex].mask.s6_addr32, sizeof(newFilter->mask));
            hlist_add_head(&newFilter->node, &mcast_ctrl->mld_snoopExceptionList);
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

void bcm_mcast_mld_set_timer(bcm_mcast_ifdata *pif)
{
   t_mld_grp_entry *mcast_group;
   int               i;
   /* the largest timeout is BCM_MCAST_MEMBERSHIP_TIMEOUT */
   unsigned long     tstamp = jiffies + (mcast_ctrl->mld_general_query_timeout_secs * HZ * 2);
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
      if ( (pif->mld_delayed_skb[pendingIndex].skb != NULL) && 
           (time_after(skbTimeout, pif->mld_delayed_skb[pendingIndex].expiryTime)) )
      {
         skbTimeout = pif->mld_delayed_skb[pendingIndex].expiryTime;
         found = 1;
      }
   }   

   if ( (BCM_MCAST_SNOOPING_DISABLED_FLOOD == pif->mld_snooping) && (0 == found))
   {
      del_timer(&pif->mld_timer);
      return;
   }

   if (found) 
   {
      if (time_after(tstamp, skbTimeout) )
      {
         tstamp = skbTimeout;
      }
   }

   if ( pif->mld_snooping != 0 ) 
   {
      for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
      {
         hlist_for_each_entry(mcast_group, &pif->mc_ipv6_hash[i], hlist) 
         {
            t_mld_rep_entry *reporter_group;
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
      del_timer(&pif->mld_timer);
   }
   else
   {
      mod_timer(&pif->mld_timer, (tstamp + BCM_MCAST_FDB_TIMER_TIMEOUT));
   }

}

int bcm_mcast_mld_wipe_group(bcm_mcast_ifdata *parent_if, int dest_ifindex, struct in6_addr *gpAddr)
{
   t_mld_grp_entry *mcast_group;
   int              i;
   int              retVal = -EINVAL;
 
   struct net_device* destDev = dev_get_by_index (&init_net, dest_ifindex);
   if (NULL != destDev)
   {
      spin_lock_bh(&parent_if->mc_mld_lock);
      retVal = 0;
      for (i = 0; i < BCM_MCAST_HASH_SIZE; i++)
      {
         struct hlist_node *n_group;
         hlist_for_each_entry_safe(mcast_group, n_group, &parent_if->mc_ipv6_hash[i], hlist)
         {
            t_mld_rep_entry *reporter_group, *n_reporter;
            list_for_each_entry_safe(reporter_group, n_reporter, &mcast_group->rep_list, list)
            {
               if (BCM_IN6_ARE_ADDR_EQUAL(&mcast_group->grp, gpAddr) && (mcast_group->dst_dev == destDev))
               {
                   __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d rep %x...%x",
                             htons(mcast_group->grp.s6_addr16[0]), 
                             htons(mcast_group->grp.s6_addr16[7]),
                             mcast_group->dst_dev->name,
                             mcast_group->from_dev->name,
                             mcast_group->num_tags,
                             htons(reporter_group->rep.s6_addr16[0]),
                             htons(reporter_group->rep.s6_addr16[7]));
                   if (bcm_mcast_mld_del_entry(parent_if, mcast_group, &reporter_group->rep, NULL))
                   {
                      break;
                   }
               }
            }
         }
      }
      bcm_mcast_mld_set_timer(parent_if);
      spin_unlock_bh(&parent_if->mc_mld_lock);
      dev_put(destDev);
   }
   return retVal;
}



static void bcm_mcast_mld_reporter_timeout(bcm_mcast_ifdata *pif)
{
   t_mld_grp_entry *mcast_group;
   int i;

   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      struct hlist_node *n_group;
      hlist_for_each_entry_safe(mcast_group, n_group, &pif->mc_ipv6_hash[i], hlist) 
      {
         t_mld_rep_entry *reporter, *n_reporter;
         list_for_each_entry_safe(reporter, n_reporter, &mcast_group->rep_list, list)
         {
            if (time_after_eq(jiffies, reporter->tstamp))
            {
                __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d rep %x...%x",
                          htons(mcast_group->grp.s6_addr16[0]), 
                          htons(mcast_group->grp.s6_addr16[7]),
                          mcast_group->dst_dev->name,
                          mcast_group->from_dev->name,
                          mcast_group->num_tags,
                          htons(reporter->rep.s6_addr16[0]),
                          htons(reporter->rep.s6_addr16[7]));
                if (bcm_mcast_mld_del_entry(pif, mcast_group, &reporter->rep, NULL))
                {
                   break;
                }
            }
         }
      }
   }
}

static void bcm_mcast_mld_admission_timeout(bcm_mcast_ifdata *pif)
{
   int pendingIndex;

   for (pendingIndex = 0; pendingIndex < BCM_MCAST_MAX_DELAYED_SKB_COUNT; pendingIndex ++)
   {
      if (( pif->mld_delayed_skb[pendingIndex].skb != NULL ) && 
          ( time_before(pif->mld_delayed_skb[pendingIndex].expiryTime, jiffies) ) )
      {
         kfree_skb(pif->mld_delayed_skb[pendingIndex].skb);
         pif->mld_delayed_skb[pendingIndex].skb = NULL;
         pif->mld_delayed_skb[pendingIndex].expiryTime = 0;
      }
   }    
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static void bcm_mcast_mld_timer(struct timer_list *t)
#else
static void bcm_mcast_mld_timer(unsigned long param)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   bcm_mcast_ifdata *pif = from_timer(pif, t, mld_timer);
   int               ifindex = pif->ifindex;
#else
   int               ifindex = (int)param;
   bcm_mcast_ifdata *pif;
#endif

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(ifindex);
   if ( NULL == pif )
   {
      __logError("unknown interface");
      rcu_read_unlock();
      return;
   }

   spin_lock_bh(&pif->mc_mld_lock);
   bcm_mcast_mld_reporter_timeout(pif);
   bcm_mcast_mld_admission_timeout(pif);
   bcm_mcast_mld_set_timer(pif);
   spin_unlock_bh(&pif->mc_mld_lock);
   rcu_read_unlock();
}

void bcm_mcast_mld_init( bcm_mcast_ifdata *pif )
{
   int i;
   spin_lock_init(&pif->mc_mld_lock);
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++ )
   {
      INIT_HLIST_HEAD(&pif->mc_ipv6_hash[i]);
   }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   timer_setup(&pif->mld_timer, bcm_mcast_mld_timer, 0);  
#else
   setup_timer(&pif->mld_timer, bcm_mcast_mld_timer, (unsigned long)pif->ifindex);  
#endif
   pif->mld_snooping = BCM_MCAST_SNOOPING_BLOCKING_MODE;
}

void bcm_mcast_mld_fini( bcm_mcast_ifdata *pif )
{
    del_timer_sync(&pif->mld_timer);
}

int bcm_mcast_mld_admission_queue(bcm_mcast_ifdata *pif, struct sk_buff *skb)
{
   int q_index;
   int index = -1;
#if defined(CONFIG_BR_MLD_SNOOP)
   spin_lock_bh(&pif->mc_mld_lock);
   for (q_index = 0; q_index < BCM_MCAST_MAX_DELAYED_SKB_COUNT; q_index ++) 
   {
      if ( pif->mld_delayed_skb[q_index].skb == NULL ) 
      {
         pif->mld_delayed_skb[q_index].skb = skb;
         pif->mld_delayed_skb[q_index].expiryTime = jiffies + BCM_MCAST_NETLINK_SKB_TIMEOUT_MS;
         index = q_index;
         bcm_mcast_mld_set_timer(pif);
         break;
      }
   }
   spin_unlock_bh(&pif->mc_mld_lock);
#endif
   if ( -1 == index )
   {
      __logNotice("unable to queue skb"); 
   }
   return index;
}

void bcm_mcast_mld_admission_update_bydev(bcm_mcast_ifdata *pif, struct net_device *dev)
{
   int pendingIndex;
   __logInfo("flushing skbs from queue for dev %s", dev ? dev->name : "all");
   for ( pendingIndex = 0; pendingIndex < BCM_MCAST_MAX_DELAYED_SKB_COUNT; pendingIndex++)
   {
      if ( (pif->mld_delayed_skb[pendingIndex].skb != NULL) &&
           ((NULL == dev) || (pif->mld_delayed_skb[pendingIndex].skb->dev == dev)) )
      {
         kfree_skb(pif->mld_delayed_skb[pendingIndex].skb);
         pif->mld_delayed_skb[pendingIndex].skb = NULL;
         pif->mld_delayed_skb[pendingIndex].expiryTime = 0;
      }
   }   
}

int bcm_mcast_mld_admission_process(bcm_mcast_ifdata *pif, int packet_index, int admitted)
{
   struct sk_buff *skb;

   if ( (packet_index < 0) || (packet_index >= BCM_MCAST_MAX_DELAYED_SKB_COUNT))
   {
       __logNotice("skb for index %d not set", packet_index);
       return -1;
   }

   skb = pif->mld_delayed_skb[packet_index].skb;
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
      pif->mld_delayed_skb[packet_index].skb = NULL;
   }
   else
   {
      __logNotice("skb for index %d not set", packet_index);
   }

   return 0;
}

void bcm_mcast_mld_update_bydev( bcm_mcast_ifdata *pif, struct net_device *dev, int activate)
{
   t_mld_grp_entry *mc_fdb;
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
      deactivate snoop entries and remove all other entries involving dev */
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      struct hlist_node *n;
      hlist_for_each_entry_safe(mc_fdb, n, &pif->mc_ipv6_hash[i], hlist) 
      {
         if (NULL == dev)
         {
             __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d dev NULL",
                       htons(mc_fdb->grp.s6_addr16[0]), 
                       htons(mc_fdb->grp.s6_addr16[7]),
                       mc_fdb->dst_dev->name,
                       mc_fdb->from_dev->name,
                       mc_fdb->num_tags);
             bcm_mcast_mld_del_entry(pif, mc_fdb, NULL, NULL);
         }
         else if ( (mc_fdb->dst_dev == dev) ||
                   (mc_fdb->from_dev == dev) )
         {
#if defined(CONFIG_BLOG)
            if (0 == activate)
            {
                __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d dev %s",
                          htons(mc_fdb->grp.s6_addr16[0]), 
                          htons(mc_fdb->grp.s6_addr16[7]),
                          mc_fdb->dst_dev->name,
                          mc_fdb->from_dev->name,
                          mc_fdb->num_tags,
                          dev->name);
                bcm_mcast_mld_del_entry(pif, mc_fdb, NULL, NULL);
            }
            else
            {
                __logInfo("Invoke bcm_mcast_delete_flow for grp %x...%x dstdev %s srcdev %s num_tags %d dev %s",
                          htons(mc_fdb->grp.s6_addr16[0]), 
                          htons(mc_fdb->grp.s6_addr16[7]),
                          mc_fdb->dst_dev->name,
                          mc_fdb->from_dev->name,
                          mc_fdb->num_tags,
                          dev->name);
               bcm_mcast_delete_flow(BCM_MCAST_PROTO_IPV6, &(mc_fdb->flowhdl));
            }
#else
            bcm_mcast_mld_del_entry(pif, mc_fdb, NULL, NULL);
#endif
         }
      }
   }

   /* activate snoop entries for dev*/
#if defined(CONFIG_BLOG)
   if ((dev != NULL) && (1 == activate))
   {
      for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
      {
         struct hlist_node *n;
         hlist_for_each_entry_safe(mc_fdb, n, &pif->mc_ipv6_hash[i], hlist) 
         { 
            if ( (mc_fdb->dst_dev == dev) ||
                 (mc_fdb->from_dev == dev) )
            {
               ret = bcm_mcast_create_flow(pif, 
                                           (void*)mc_fdb, 
                                           BCM_MCAST_PROTO_IPV6,
                                           &pif->mc_ipv6_hash[bcm_mcast_mld_hash(&mc_fdb->grp)],
                                           &(mc_fdb->flowhdl));
               if(ret < 0)
               {
                  /* bcm_mcast_blog_process may return -1 if there are no blog rules
                   * which may be a valid scenario, in which case we delete the
                   * multicast entry.
                   */
                __logInfo("Unable to create flow, Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d dev %s",
                          htons(mc_fdb->grp.s6_addr16[0]), 
                          htons(mc_fdb->grp.s6_addr16[7]),
                          mc_fdb->dst_dev->name,
                          mc_fdb->from_dev->name,
                          mc_fdb->num_tags,
                          dev->name);
                  bcm_mcast_mld_del_entry(pif, mc_fdb, NULL, NULL);
               }
            }
         }
      }
   }
#endif   
   bcm_mcast_mld_set_timer(pif);

   return;
}


/* In the case where a reporter has changed ports, this function
   will remove all records pointing to the old port */
void bcm_mcast_mld_wipe_reporter_for_port (bcm_mcast_ifdata *pif,
                                                struct in6_addr *rep, 
                                                struct net_device *rep_dev)
{
    int hashIndex;
    struct hlist_node *n = NULL;
    struct hlist_head *head = NULL;
    t_mld_grp_entry *mc_fdb;
  
    __logInfo("Wiping MLD snoop entries for Reporter %x...%x reporter dev %s pif %s", 
              rep->s6_addr16[0], rep->s6_addr16[7], rep_dev ? rep_dev->name:"NULL", pif ? pif->dev->name : "NULL");
    
    spin_lock_bh(&pif->mc_mld_lock);
    for (hashIndex = 0 ; hashIndex < BCM_MCAST_HASH_SIZE ; hashIndex++)
    {
        head = &pif->mc_ipv6_hash[hashIndex];
        hlist_for_each_entry_safe(mc_fdb, n, head, hlist)
        {
            if ((bcm_mcast_mld_rep_find(mc_fdb, rep, NULL)) &&
                (mc_fdb->dst_dev == rep_dev))
            {
                /* The reporter we're looking for has been found
                   in a record pointing to its old port */
                __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d rep %x...%x",
                          htons(mc_fdb->grp.s6_addr16[0]), 
                          htons(mc_fdb->grp.s6_addr16[7]),
                          mc_fdb->dst_dev->name,
                          mc_fdb->from_dev->name,
                          mc_fdb->num_tags,
                          rep ? htons(rep->s6_addr16[0]):0,
                          rep ? htons(rep->s6_addr16[7]):0);
                bcm_mcast_mld_del_entry (pif, mc_fdb, rep, NULL);
            }
        }
    }
    bcm_mcast_mld_set_timer(pif);
    spin_unlock_bh(&pif->mc_mld_lock);
}

void bcm_mcast_mld_wipe_reporter_by_mac (bcm_mcast_ifdata *pif,
                                              unsigned char *repMac)
{
    int hashIndex;
    struct hlist_node *n = NULL;
    struct hlist_head *head = NULL;
    t_mld_grp_entry *mc_fdb;

    __logInfo("Wiping MLD snoop entries for MAC %pM pif %s", 
              repMac, pif ? pif->dev->name : "NULL");

    for (hashIndex = 0; hashIndex < BCM_MCAST_HASH_SIZE ; hashIndex++)
    {
        head = &pif->mc_ipv6_hash[hashIndex];
        hlist_for_each_entry_safe(mc_fdb, n, head, hlist)
        {
            if (bcm_mcast_mld_rep_find(mc_fdb, NULL, repMac))
            {
                __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d repMac %pM",
                          htons(mc_fdb->grp.s6_addr16[0]), 
                          htons(mc_fdb->grp.s6_addr16[7]),
                          mc_fdb->dst_dev->name,
                          mc_fdb->from_dev->name,
                          mc_fdb->num_tags,
                          repMac);
                bcm_mcast_mld_del_entry (pif, mc_fdb, NULL, repMac);
            }
        }
    }
    bcm_mcast_mld_set_timer(pif);
}

void bcm_mcast_mld_wipe_grp_reporter_for_port (bcm_mcast_ifdata *pif,
                                               struct in6_addr *grp, 
                                               struct in6_addr *rep, 
                                               struct net_device *rep_dev)
{
    int hashIndex;
    struct hlist_node *n = NULL;
    struct hlist_head *head = NULL;
    t_mld_grp_entry *mc_fdb;
    
    __logInfo("Wiping MLD snoop entries for Reporter %x...%x reporter dev %s pif %s", 
              rep->s6_addr16[0], rep->s6_addr16[7], rep_dev ? rep_dev->name:"NULL", pif ? pif->dev->name : "NULL");

    spin_lock_bh(&pif->mc_mld_lock);
    for (hashIndex = 0 ; hashIndex < BCM_MCAST_HASH_SIZE ; hashIndex++)
    {
        head = &pif->mc_ipv6_hash[hashIndex];
        hlist_for_each_entry_safe(mc_fdb, n, head, hlist)
        {
            if (BCM_IN6_ARE_ADDR_EQUAL(&mc_fdb->grp, grp) &&
                (bcm_mcast_mld_rep_find(mc_fdb, rep, NULL)) &&
                (mc_fdb->dst_dev == rep_dev))
            {
                /* The reporter we're looking for has been found
                   in a record pointing to its old port */
                __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d rep %x...%x",
                          htons(mc_fdb->grp.s6_addr16[0]), 
                          htons(mc_fdb->grp.s6_addr16[7]),
                          mc_fdb->dst_dev->name,
                          mc_fdb->from_dev->name,
                          mc_fdb->num_tags,
                          rep ? htons(rep->s6_addr16[0]):0,
                          rep ? htons(rep->s6_addr16[7]):0);
                bcm_mcast_mld_del_entry (pif, mc_fdb, rep, NULL);
            }
        }
    }
    bcm_mcast_mld_set_timer(pif);
    spin_unlock_bh(&pif->mc_mld_lock);
}

#if defined(CONFIG_BLOG)
void bcm_mcast_mld_process_blog_enable( bcm_mcast_ifdata *pif, int enable)
{
   t_mld_grp_entry *mc_fdb;
   int i;
   int ret;

   __logDebug("process blog enable setting %d", enable);

   if(NULL == pif)
   {
      return;
   }

   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      struct hlist_node *n;
      hlist_for_each_entry_safe(mc_fdb, n, &pif->mc_ipv6_hash[i], hlist) 
      {
         if ( 0 == enable )
         {
             __logInfo("Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d",
                       htons(mc_fdb->grp.s6_addr16[0]), 
                       htons(mc_fdb->grp.s6_addr16[7]),
                       mc_fdb->dst_dev->name,
                       mc_fdb->from_dev->name,
                       mc_fdb->num_tags);
             bcm_mcast_delete_flow(BCM_MCAST_PROTO_IPV6, &(mc_fdb->flowhdl));
         }
         else
         {
             ret = bcm_mcast_create_flow(pif, 
                                         (void*)mc_fdb, 
                                         BCM_MCAST_PROTO_IPV6,
                                         &pif->mc_ipv6_hash[bcm_mcast_mld_hash(&mc_fdb->grp)],
                                         &(mc_fdb->flowhdl));
             if(ret < 0)
             {
                 /* bcm_mcast_blog_process may return -1 if there are no blog rules
                  * which may be a valid scenario, in which case we delete the
                  * multicast entry.
                  */
                 __logInfo("Unable to create flow, Invoke bcm_mcast_mld_del_entry for grp %x...%x dstdev %s srcdev %s num_tags %d",
                           htons(mc_fdb->grp.s6_addr16[0]), 
                           htons(mc_fdb->grp.s6_addr16[7]),
                           mc_fdb->dst_dev->name,
                           mc_fdb->from_dev->name,
                           mc_fdb->num_tags);
                 bcm_mcast_mld_del_entry(pif, mc_fdb, NULL, NULL);
            }
         }
      }
   }
   bcm_mcast_mld_set_timer(pif);

   return;
}
#endif

int bcm_mcast_mld_add(struct net_device *from_dev,
                      int wan_ops,
                      bcm_mcast_ifdata *pif, 
                      struct net_device *dst_dev, 
                      struct net_device *to_accel_dev, 
                      struct in6_addr *grp, 
                      struct in6_addr *rep,
                      unsigned char *repMac,
                      unsigned char rep_proto_ver,
                      int mode, 
                      uint16_t tci, 
                      uint16_t grpVid,
                      struct in6_addr *src,
                      int lanppp,
                      uint32_t info)
{
   if(!pif || !dst_dev || !grp|| !rep || !from_dev)
      return 0;

   __logInfo("from %s pif %s dst %s to_accel_dev %s mode %d info 0x%x clientmac %pM",
              from_dev ? from_dev->name:"NULL", pif ? (pif->dev?pif->dev->name:"NULL"):"NULL",
              dst_dev ? dst_dev->name:"NULL", to_accel_dev?to_accel_dev->name:"NULL",
              mode, info, repMac);

   BCM_MCAST_DBG_PRINT_V6_ADDR("RxGrp", grp->s6_addr16);
   BCM_MCAST_DBG_PRINT_V6_ADDR("Client", rep->s6_addr16);
   BCM_MCAST_DBG_PRINT_V6_ADDR("Src", src->s6_addr16);

   if(!(bcm_mcast_mld_control_filter(grp)))
   {
      __logInfo("MLD control filter applied");
      return 0;
   }

   if((MCAST_INCLUDE != mode) && (MCAST_EXCLUDE != mode))
   {
      __logInfo("MLD mode not supported");
      return 0;
   }

   spin_lock_bh(&pif->mc_mld_lock);
   if (bcm_mcast_mld_update_entry(pif, dst_dev, grp, rep, repMac, rep_proto_ver, mode, src, from_dev, info))
   {
      spin_unlock_bh(&pif->mc_mld_lock);
      return 0;
   }

   if (bcm_mcast_mld_add_entry(from_dev,
                               wan_ops,
                               pif, 
                               dst_dev, 
                               to_accel_dev, 
                               grp, 
                               rep,
                               repMac,
                               rep_proto_ver,
                               mode, 
                               tci, 
                               grpVid,
                               src,
                               lanppp,
                               info) != 0)
   {
       spin_unlock_bh(&pif->mc_mld_lock);
       return -1;
   }
   bcm_mcast_mld_set_timer(pif);
   spin_unlock_bh(&pif->mc_mld_lock);
   return 0;
}

int bcm_mcast_mld_remove(struct net_device *from_dev,
                         bcm_mcast_ifdata *pif, 
                         struct net_device *dst_dev, 
                         struct in6_addr *grp, 
                         struct in6_addr *rep, 
                         int mode, 
                         struct in6_addr *src,
                         uint32_t info)
{   
   if(!pif || !dst_dev || !grp|| !rep || !from_dev)
      return 0;

   __logInfo("from %s pif %s dst %s mode %d info 0x%x",
              from_dev ? from_dev->name:"NULL", pif ? (pif->dev?pif->dev->name:"NULL"):"NULL",
              dst_dev ? dst_dev->name:"NULL", mode, info);

   BCM_MCAST_DBG_PRINT_V6_ADDR("Group", grp->s6_addr16);
   BCM_MCAST_DBG_PRINT_V6_ADDR("Client", rep->s6_addr16);
   BCM_MCAST_DBG_PRINT_V6_ADDR("Src", src->s6_addr16);

   if(!(bcm_mcast_mld_control_filter(grp)))
   {
       __logInfo("MLD control filter applied");
      return 0;
   }

   if((MCAST_INCLUDE != mode) && (MCAST_EXCLUDE != mode))
   {
      __logInfo("MLD mode not supported");
      return 0;
   }

   spin_lock_bh(&pif->mc_mld_lock);
   bcm_mcast_mld_remove_entry(from_dev,
                              pif, 
                              dst_dev, 
                              grp, 
                              rep, 
                              mode, 
                              src,
                              info);
   bcm_mcast_mld_set_timer(pif);
   spin_unlock_bh(&pif->mc_mld_lock);
   return 0;
}

int bcm_mcast_mld_should_deliver(bcm_mcast_ifdata *pif,
                                  const struct ipv6hdr *pipv6,
                                  struct net_device *src_dev,
                                  struct net_device *dst_dev)
{
   t_mld_grp_entry  *dst;
   struct hlist_head *head = NULL;
   int                should_deliver;

   if (0 == bcm_mcast_mld_control_filter(&pipv6->daddr))
   {
      /* accept packet */
      /*__logDebug("MLD Control filter Accept pkt: src %s, dst %s "
                 "dstip6 %08x:%08x:%08x:%08x", src_dev->name, dst_dev->name, 
                 ntohl(pipv6->daddr.in6_u.u6_addr32[0]), 
                 ntohl(pipv6->daddr.in6_u.u6_addr32[1]), 
                 ntohl(pipv6->daddr.in6_u.u6_addr32[2]), 
                 ntohl(pipv6->daddr.in6_u.u6_addr32[3]));*/
      return 1;
   }

   /* drop traffic by default when snooping is enabled
      in blocking mode */
   if (BCM_MCAST_SNOOPING_BLOCKING_MODE == pif->mld_snooping)
   {
      should_deliver = 0;
   }
   else
   {
      should_deliver = 1;
   }

   /* adhere to forwarding entries regardless of snooping mode */

   spin_lock_bh(&pif->mc_mld_lock);
   head = &pif->mc_ipv6_hash[bcm_mcast_mld_hash(&pipv6->daddr)];
   hlist_for_each_entry(dst, head, hlist)
   {
      if ( (!BCM_IN6_ARE_ADDR_EQUAL(&dst->grp, &pipv6->daddr)) ||
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
         if (BCM_IN6_ARE_ADDR_EQUAL(&pipv6->saddr, &dst->src_entry.src))
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
      }
   }
   spin_unlock_bh(&pif->mc_mld_lock);
   __logDebug("source device %s, dst device %s dstip %08x:%08x:%08x:%08x forward: %s", 
              src_dev->name, dst_dev->name, ntohl(pipv6->daddr.in6_u.u6_addr32[0]), 
              ntohl(pipv6->daddr.in6_u.u6_addr32[1]), ntohl(pipv6->daddr.in6_u.u6_addr32[2]), 
              ntohl(pipv6->daddr.in6_u.u6_addr32[3]), should_deliver ? "Yes":"No");
   return should_deliver;
}

static void bcm_mcast_mld_display_entry(struct seq_file *seq,
                                         bcm_mcast_ifdata *pif,
                                         t_mld_grp_entry *dst)
{
   t_mld_rep_entry *rep_entry;
   int               first;
   int               tstamp;

   seq_printf(seq, "%-6s %-6s %-6s %-7s %02d    0x%04x   0x%04x%04x  0x%x", 
              pif->dev->name, 
              dst->dst_dev->name, 
              dst->to_accel_dev->name, 
              dst->from_dev->name, 
              dst->num_tags,
              ntohs(dst->lan_tci),
              ((dst->wan_tci >> 16) & 0xFFFF),
              (dst->wan_tci & 0xFFFF),
              dst->info);

   seq_printf(seq, " %08x:%08x:%08x:%08x",
              htonl(dst->grp.s6_addr32[0]),
              htonl(dst->grp.s6_addr32[1]),
              htonl(dst->grp.s6_addr32[2]),
              htonl(dst->grp.s6_addr32[3]));

   seq_printf(seq, " %-4s %08x:%08x:%08x:%08x", 
              (dst->src_entry.filt_mode == MCAST_EXCLUDE) ? 
               "EX" : "IN",
              htonl(dst->src_entry.src.s6_addr32[0]), 
              htonl(dst->src_entry.src.s6_addr32[1]), 
              htonl(dst->src_entry.src.s6_addr32[2]), 
              htonl(dst->src_entry.src.s6_addr32[3]));

   first = 1;
   list_for_each_entry(rep_entry, &dst->rep_list, list)
   { 

      if ( 0 == pif->mld_snooping )
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
         seq_printf(seq, " %-7d %08x:%08x:%08x:%08x %d\n", 
                    tstamp,
                    htonl(rep_entry->rep.s6_addr32[0]),
                    htonl(rep_entry->rep.s6_addr32[1]),
                    htonl(rep_entry->rep.s6_addr32[2]),
                    htonl(rep_entry->rep.s6_addr32[3]), 
                    flowhdl_p->numkeys);
#else
         seq_printf(seq, " %-7d %08x:%08x:%08x:%08x\n", 
                    tstamp,
                    htonl(rep_entry->rep.s6_addr32[0]),
                    htonl(rep_entry->rep.s6_addr32[1]),
                    htonl(rep_entry->rep.s6_addr32[2]),
                    htonl(rep_entry->rep.s6_addr32[3]));
#endif
         first = 0;
      }
      else 
      {
         seq_printf(seq, "%124s %-7d %08x:%08x:%08x:%08x\n", " ", 
                    tstamp,
                    htonl(rep_entry->rep.s6_addr32[0]),
                    htonl(rep_entry->rep.s6_addr32[1]),
                    htonl(rep_entry->rep.s6_addr32[2]),
                    htonl(rep_entry->rep.s6_addr32[3]));
      }
   }
}

int bcm_mcast_mld_display(struct seq_file *seq, bcm_mcast_ifdata *pif)
{
   int i;
   bcm_mcast_lower_port* lowerPort = NULL;

   seq_printf(seq, "mld snooping %d  priority %d brtype %s\n",
              pif->mld_snooping,
              mcast_ctrl->mcastPriQueue,
              pif->brtype ? "OVS":"Linux");

   seq_printf(seq, " Port Name    Querier      Timeout\n");
   hlist_for_each_entry_rcu (lowerPort, &pif->lower_port_list, hlist)
   {
      int tstamp = lowerPort->mld_querying_port ? (int)(lowerPort->mld_querying_port_timer.expires - jiffies) / HZ : 0;
      struct net_device* lowerDev = dev_get_by_index(&init_net, lowerPort->port_ifindex);
      if (lowerDev) 
      {
         seq_printf(seq, "    %-6s        %-3s          %03d\n", lowerDev->name,
                                        lowerPort->mld_querying_port ? "YES" : " NO", 
                                        tstamp);
         dev_put(lowerDev);
      }
   }
   seq_printf(seq, "bridge dstdev dstaccdev src-dev #tags lan-tci  wan-tci info");
   seq_printf(seq, "    group                               mode source");
#if defined(CONFIG_BLOG)
   seq_printf(seq, "                              timeout reporter");
   seq_printf(seq, "                            Index\n");
#else
   seq_printf(seq, "                              timeout reporter\n");
#endif
   spin_lock_bh(&pif->mc_mld_lock);
   for (i = 0; i < BCM_MCAST_HASH_SIZE; i++) 
   {
      t_mld_grp_entry *entry;
      hlist_for_each_entry(entry, &pif->mc_ipv6_hash[i], hlist) 
      {
         bcm_mcast_mld_display_entry(seq, pif, entry);
      }
   }
   spin_unlock_bh(&pif->mc_mld_lock);

   if (bcmLog_logIsEnabled(BCM_LOG_ID_MCAST, BCM_LOG_LEVEL_INFO)) 
   {
       bcm_mcast_fc_flowhdl_dump();
       bcm_mcast_dump_grpinfo_all();
   }
   return 0;
}


