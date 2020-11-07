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
#include <linux/namei.h>
#include "br_private.h"

#include "bcm_mcast_priv.h"

/* must be called with rcu lock */
bcm_mcast_ifdata *bcm_mcast_if_lookup(int ifindex)
{
   bcm_mcast_ifdata  *pif;

   hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
   {
      if ( pif->ifindex == ifindex )
      {
         return pif;
      }
   }
   return NULL;
}

#if defined(CONFIG_BR_IGMP_SNOOP)
static void bcm_mcast_if_handle_lower_querying_timer(unsigned long data)
{
   bcm_mcast_lower_port* lowerPort = (void *) data;
   lowerPort->querying_port = 0;
}
#endif

#if defined(CONFIG_BR_MLD_SNOOP)
static void bcm_mcast_if_handle_lower_mld_querying_timer(unsigned long data)
{
   bcm_mcast_lower_port* lowerPort = (void *) data;
   lowerPort->mld_querying_port = 0;
}
#endif

static void bcm_mcast_if_add_lower_port_to_if(int lowerDevIfIndex, bcm_mcast_ifdata* pif)
{
   bcm_mcast_lower_port* lowerPort = kmalloc(sizeof(bcm_mcast_lower_port), GFP_ATOMIC);
   lowerPort->port_ifindex = lowerDevIfIndex;
#if defined(CONFIG_BR_IGMP_SNOOP)
   lowerPort->querying_port = 0;
   setup_timer(&lowerPort->querying_port_timer, bcm_mcast_if_handle_lower_querying_timer, (unsigned long)lowerPort);
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   lowerPort->mld_querying_port = 0;
   setup_timer(&lowerPort->mld_querying_port_timer, bcm_mcast_if_handle_lower_mld_querying_timer, (unsigned long)lowerPort);
#endif
   hlist_add_head(&lowerPort->hlist, &pif->lower_port_list);
}

bcm_mcast_ifdata *bcm_mcast_if_create(int ifindex, enum_bcm_mcast_brtype brtype)
{
   bcm_mcast_ifdata  *pif;
   struct net_device *lowerdev;
   struct list_head  *iter;
   struct net_device *dev;

   dev = dev_get_by_index(&init_net, ifindex);
   if ( dev == NULL )
   {
       return NULL;
   }

   if ((brtype == BRTYPE_LINUX) && ( 0 == (dev->priv_flags & IFF_EBRIDGE) )) 
   {
       dev_put(dev);
       return NULL;
   }
   pif = (bcm_mcast_ifdata *)kmalloc(sizeof(*pif), GFP_ATOMIC);
   if ( pif == NULL )
   {
      dev_put(dev);
      return NULL;
   }
   memset(pif, 0, sizeof(*pif));

   spin_lock_init(&pif->config_lock);
   pif->ifindex = ifindex;
   pif->brtype = brtype;
   if (brtype == BRTYPE_LINUX) 
   {
       spin_lock_bh(&pif->config_lock);
       netdev_for_each_lower_dev(dev, lowerdev, iter)
       {
          bcm_mcast_if_add_lower_port_to_if (lowerdev->ifindex, pif);
       }
       spin_unlock_bh(&pif->config_lock);
   }
#if defined(CONFIG_BR_IGMP_SNOOP)
   bcm_mcast_igmp_init(pif);
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   bcm_mcast_mld_init(pif);
#endif
   pif->dev = dev;

   /* write lock is not needed as there is only one writer of if_list*/
   hlist_add_head_rcu(&pif->hlist, &mcast_ctrl->if_hlist);

   return pif;
}

int bcm_mcast_if_admission_process(int proto, struct net_device *dev, int packet_index, int admitted)
{
   bcm_mcast_ifdata *pif;

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(dev->ifindex);
   if (NULL == pif)
   {
      rcu_read_unlock();
      return -ENODEV;
   }

#if defined(CONFIG_BR_IGMP_SNOOP)
   if ( BCM_MCAST_PROTO_IPV4 == proto )
   {
      spin_lock_bh(&pif->mc_igmp_lock);
      bcm_mcast_igmp_admission_process(pif, packet_index, admitted);
      spin_unlock_bh(&pif->mc_igmp_lock);
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if ( BCM_MCAST_PROTO_IPV6 == proto )
   {
      spin_lock_bh(&pif->mc_mld_lock);
      bcm_mcast_mld_admission_process(pif, packet_index, admitted);
      spin_unlock_bh(&pif->mc_mld_lock);
   }
#endif
   rcu_read_unlock();
   return 0;
}

void bcm_mcast_if_admission_update_bydev(int proto, struct net_device *ndev)
{
   bcm_mcast_ifdata  *pif;
   struct net_device *pdev;

   rcu_read_lock();
   hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
   {
      /* if device is specified and device is a bridge then pif must match dev */
      if ( ndev && (ndev->priv_flags & IFF_EBRIDGE) )
      {
         if ( ndev != pif->dev )
         {
            continue;
         }
         pdev = NULL;
      }
      else
      {
         pdev = ndev;
      }
#if defined(CONFIG_BR_IGMP_SNOOP)
      if ( BCM_MCAST_PROTO_IPV4 & proto )
      {
         spin_lock_bh(&pif->mc_igmp_lock);
         bcm_mcast_igmp_admission_update_bydev(pif, pdev);
         spin_unlock_bh(&pif->mc_igmp_lock);
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if (BCM_MCAST_PROTO_IPV6 & proto )
      {
         spin_lock_bh(&pif->mc_mld_lock);
         bcm_mcast_mld_admission_update_bydev(pif, pdev);
         spin_unlock_bh(&pif->mc_mld_lock);
      }
#endif
   }
   rcu_read_unlock();
}

void bcm_mcast_if_update_bydev(int proto, struct net_device *ndev, int activate)
{
   bcm_mcast_ifdata  *pif;
   struct net_device *pdev;

   rcu_read_lock();
   hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
   {
      /* if device is specified and device is a bridge then pif must match dev */
      if ( ndev && (ndev->priv_flags & IFF_EBRIDGE) )
      {
         if ( ndev != pif->dev )
         {
            continue;
         }
         pdev = NULL;
      }
      else
      {
         pdev = ndev;
      }
#if defined(CONFIG_BR_IGMP_SNOOP)
      if ( BCM_MCAST_PROTO_IPV4 & proto )
      {
         spin_lock_bh(&pif->mc_igmp_lock);
         bcm_mcast_igmp_update_bydev(pif, pdev, activate);
         spin_unlock_bh(&pif->mc_igmp_lock);
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if (BCM_MCAST_PROTO_IPV6 & proto )
      {
         spin_lock_bh(&pif->mc_mld_lock);
         bcm_mcast_mld_update_bydev(pif, pdev, activate);
         spin_unlock_bh(&pif->mc_mld_lock);
      }
#endif
   }
   rcu_read_unlock();
}

int bcm_mcast_if_is_snooping_enabled(struct net_device *dev, int proto)
{
   bcm_mcast_ifdata *pif = NULL;
   int               rv = 0;

   rcu_read_lock();
   if ( 0 == (dev->priv_flags & IFF_EBRIDGE) )
   {
      struct net_device *upperdev;
      struct list_head  *iter;

      netdev_for_each_upper_dev_rcu(dev, upperdev, iter)
      {
         if ( upperdev->priv_flags & IFF_EBRIDGE )
         {
            pif = bcm_mcast_if_lookup(upperdev->ifindex);
            break;
         }
      }
   }
   else
   {
      pif = bcm_mcast_if_lookup(dev->ifindex);
   }

   if ( pif != NULL )
   {
      do
      {
#if defined(CONFIG_BR_IGMP_SNOOP)
         if ( BCM_MCAST_PROTO_IPV4 == proto )
         {
            if ( (pif->igmp_snooping == BCM_MCAST_SNOOPING_BLOCKING_MODE) || 
                 (pif->igmp_snooping == BCM_MCAST_SNOOPING_STANDARD_MODE) )
            {
               rv = 1;
            }
            break;
         }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
         if ( BCM_MCAST_PROTO_IPV6 == proto )
         {
            if ( (pif->mld_snooping == BCM_MCAST_SNOOPING_BLOCKING_MODE) || 
                 (pif->mld_snooping == BCM_MCAST_SNOOPING_STANDARD_MODE) )
            {
               rv = 1;
            }
            break;
         }
#endif
      } while (0);
   }
   rcu_read_unlock();
   return rv;
}

int bcm_mcast_if_is_associated_dev(struct net_device *dev, int parentifi)
{
   struct net_device *upperdev;
   struct list_head  *iter;
   int                found = 0;

   rcu_read_lock();
   netdev_for_each_upper_dev_rcu(dev, upperdev, iter)
   {
      if ( upperdev->priv_flags & IFF_EBRIDGE )
      {
         if (upperdev->ifindex == parentifi )
         {
            found = 1;
         }
      }
   }
   rcu_read_unlock();
   return found;
}

static void bcm_mcast_if_destroy(struct rcu_head *head)
{
   bcm_mcast_ifdata *pif;

   pif = container_of(head, bcm_mcast_ifdata, rcu);
#if defined(CONFIG_BR_IGMP_SNOOP)
   spin_lock_bh(&pif->mc_igmp_lock);
   bcm_mcast_igmp_update_bydev(pif, NULL, 0);
   bcm_mcast_igmp_admission_update_bydev(pif, NULL);
   spin_unlock_bh(&pif->mc_igmp_lock);
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   spin_lock_bh(&pif->mc_mld_lock);
   bcm_mcast_mld_update_bydev(pif, NULL, 0);
   bcm_mcast_mld_admission_update_bydev(pif, NULL);
   spin_unlock_bh(&pif->mc_mld_lock);
#endif
   dev_put(pif->dev);
   kfree(pif);
}

static void bcm_mcast_if_delete(int ifindex)
{
   bcm_mcast_ifdata  *pif;
   struct hlist_node *pnode;

   /* write lock is not needed as there is only one writer of if_list */
   hlist_for_each_entry_safe(pif, pnode, &mcast_ctrl->if_hlist, hlist)
   {
      if ( (pif->ifindex == ifindex) || (0 == ifindex) )
      {
         hlist_del_rcu(&pif->hlist);
         call_rcu(&pif->rcu, bcm_mcast_if_destroy);
         if ( ifindex )
         {
            /* leave loop if a specific interface was specified */
            break;
         }
      }
   }
   return;
}

/* called with pif->config_lock 
 * If the bridge in question (pif) has a lower port with the given ifindex,
 *    return a pointer to the lower_port structure
 */
bcm_mcast_lower_port* bcm_mcast_if_get_lower_port_by_ifindex(bcm_mcast_ifdata *pif, int ifindex)
{
   bcm_mcast_lower_port* lowerPort = NULL;
   hlist_for_each_entry (lowerPort, &pif->lower_port_list, hlist)
   {
      if (lowerPort->port_ifindex == ifindex)
      {
         break;
      }
   }
   return lowerPort;
}

/* When a LAN port goes down, determine its upper port
 * Remove it from the upper port's list of lower/LAN ports
 */
static void bcm_mcast_if_handle_lower_down(struct net_device *dev)
{
   bcm_mcast_ifdata  *pif = NULL;
   struct net_device *upperdev = NULL;
   struct list_head  *iter = NULL;
    
   rcu_read_lock();
   netdev_for_each_upper_dev_rcu(dev, upperdev, iter)
   {
      if ( upperdev->priv_flags & IFF_EBRIDGE )
      {
         pif = bcm_mcast_if_lookup(upperdev->ifindex);
         break;
      }
   }
   
   if (pif)
   {
      bcm_mcast_lower_port* lowerPort = NULL;
      spin_lock_bh(&pif->config_lock);
      lowerPort = bcm_mcast_if_get_lower_port_by_ifindex(pif, dev->ifindex);
      if (NULL != lowerPort)
      {
#if defined(CONFIG_BR_IGMP_SNOOP)
         del_timer(&lowerPort->querying_port_timer);
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
         del_timer(&lowerPort->mld_querying_port_timer);
#endif
         hlist_del(&lowerPort->hlist);
         kfree(lowerPort);
      }
      spin_unlock_bh(&pif->config_lock);
   }
   rcu_read_unlock();          
}

/* When a LAN port changes bridges,
 * we have to flush it out of all bridges it's in.
 */
static void bcm_mcast_if_handle_lower_flush(struct net_device *lowerDev)
{
   bcm_mcast_ifdata  *pif;

   rcu_read_lock();   
   hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
   {
      bcm_mcast_lower_port* lowerPort = NULL;
      spin_lock_bh(&pif->config_lock);
      lowerPort = bcm_mcast_if_get_lower_port_by_ifindex (pif, lowerDev->ifindex);
      if (NULL != lowerPort)
      {
#if defined(CONFIG_BR_IGMP_SNOOP)
         del_timer(&lowerPort->querying_port_timer);
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
         del_timer(&lowerPort->mld_querying_port_timer);
#endif
         hlist_del(&lowerPort->hlist);
         kfree(lowerPort);
      }
      spin_unlock_bh(&pif->config_lock);
   }
   rcu_read_unlock();
}

/* When a LAN port comes up, determine its upper port
 * Add it to the upper port's list of lower/LAN ports
 */
static void bcm_mcast_if_handle_lower_up(struct net_device *dev)
{
   bcm_mcast_ifdata *pif = NULL;
   struct net_device *upperdev = NULL;
   struct list_head *iter = NULL;

   rcu_read_lock();
   netdev_for_each_upper_dev_rcu(dev, upperdev, iter)
   {
      if ( upperdev->priv_flags & IFF_EBRIDGE )
      {
         pif = bcm_mcast_if_lookup(upperdev->ifindex);
         break;
      }
   }

   if (pif)
   {
      bcm_mcast_lower_port* lowerPort = NULL;
      spin_lock_bh(&pif->config_lock);
      lowerPort = bcm_mcast_if_get_lower_port_by_ifindex(pif, dev->ifindex);
      /* Only add the lowerPort if it doesn't exist yet */
      if (NULL == lowerPort)
      {
         bcm_mcast_if_add_lower_port_to_if(dev->ifindex, pif);
      }
      spin_unlock_bh(&pif->config_lock);

      bcm_mcast_netlink_send_query_trigger(dev->ifindex);
   }   

   if (!pif) 
   {
       /* Check if this port belongs to OvS bridge */
       bcm_mcast_ifdata  *pif;

       hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
       {
           if (pif->brtype == BRTYPE_OVS &&
               bcm_mcast_if_get_lower_port_by_ifindex(pif, dev->ifindex))
           {
               bcm_mcast_netlink_send_query_trigger(dev->ifindex);
               __logDebug("OvS port %s up, send query trigger", dev ? dev->name : "NULL");
           }
       }
   }

   rcu_read_unlock();           
}

void 
bcm_mcast_handle_dev_down(void *net) {
    if(net) {
        struct net_device *dev = (struct net_device *)net;
        if ( dev->priv_flags & IFF_EBRIDGE )
        {
            bcm_mcast_if_update_bydev(BCM_MCAST_PROTO_ALL, dev, 0);
            bcm_mcast_if_admission_update_bydev(BCM_MCAST_PROTO_ALL, dev);
        }
        else
        {
            bcm_mcast_if_handle_lower_down(dev);
        }
    }
    return;
}

EXPORT_SYMBOL(bcm_mcast_handle_dev_down);

static int bcm_mcast_if_netdev_notifier(struct notifier_block *this, unsigned long event, void *ptr)
{
   struct net_device *dev = netdev_notifier_info_to_dev(ptr);
   bcm_mcast_ifdata  *pif;

   /* process DOWN, GOING DOWN, UP and RELEASE for EBRIDGE
      process DOWN, GOING_DOWN, CHANGEUPPER and CHANGE for all other devices (not UP or RELEASE) */
   switch (event)
   {
      case NETDEV_DOWN:
      case NETDEV_GOING_DOWN:
         __logDebug("NETDEV_DOWN/NETDEV_GOING_DOWN for dev %s", dev ? dev->name : "all");
         if ( dev->priv_flags & IFF_EBRIDGE )
         {
            bcm_mcast_if_update_bydev(BCM_MCAST_PROTO_ALL, dev, 0);
            bcm_mcast_if_admission_update_bydev(BCM_MCAST_PROTO_ALL, dev);
            break;
         }
         else
         {
            bcm_mcast_if_handle_lower_down(dev);
         }
         /* fall through */
         
      case NETDEV_CHANGEUPPER:
         __logDebug("NETDEV_CHANGEUPPER for dev %s", dev ? dev->name : "all");
         bcm_mcast_if_handle_lower_flush(dev);         
      case NETDEV_CHANGE:
         __logDebug("NETDEV_CHANGE for dev %s", dev ? dev->name : "all");
         if ( 0 == (dev->priv_flags & IFF_EBRIDGE) )
         {
            bcm_mcast_if_update_bydev(BCM_MCAST_PROTO_ALL, dev, 0);
            bcm_mcast_if_admission_update_bydev(BCM_MCAST_PROTO_ALL, dev);
            if (dev->operstate == IF_OPER_UP)
            {
               bcm_mcast_if_handle_lower_up(dev);
            }
            else 
            {
               bcm_mcast_if_handle_lower_down(dev);
            }
         }
         break;

      case NETDEV_REGISTER:
      case NETDEV_UP:
         __logDebug("NETDEV_REGISTER/NETDEV_UP for dev %s", dev ? dev->name : "all");
         if ( dev->priv_flags & IFF_EBRIDGE )
         {
            /* add the interface if it does not already exist */
            rcu_read_lock();
            pif = bcm_mcast_if_lookup(dev->ifindex);
            /* pif is not used if set so we can release the rcu lock */
            rcu_read_unlock();
            if ( NULL == pif )
            {
               bcm_mcast_if_create(dev->ifindex, BRTYPE_LINUX);

               /* would need rcu_read_lock here if we used pif
                  returned by bcm_mcast_if_create */
            }
         }
         else 
         {
            /* handle lower ports coming up - add LAN ports to list */
            bcm_mcast_if_handle_lower_up(dev);
         }
         break;

      case NETDEV_UNREGISTER:
         __logDebug("NETDEV_UNREGISTER for dev %s", dev ? dev->name : "all");
         if ( dev->priv_flags & IFF_EBRIDGE )
         {
            /* remove the interface */
            bcm_mcast_if_delete(dev->ifindex);
            break;
         }
         break;
         
           
      default:
         break;
   }

   return NOTIFY_DONE;
}

static int bridge_notifier(struct notifier_block *nb, unsigned long event, void *info)
{
    struct bridge_notifier_info *info_p = info; 
    switch (event)
    {
        case BREVT_IF_CHANGED:
            if (info_p->isadd) 
            {
                /* handle lower ports added to bridge - add LAN ports to list
                   Remove from bridge is handled by NETDEV_DOWN notification */
                bcm_mcast_if_handle_lower_up(info_p->dev);
            }
            break;
    }
    return NOTIFY_DONE;
}

static int bcm_mcast_if_discover( void )
{
   struct net_device *dev;
   bcm_mcast_ifdata  *pif;

   rcu_read_lock();
   for_each_netdev_rcu(&init_net, dev)
   {
      if ( dev->priv_flags & IFF_EBRIDGE )
      {
         /* add the device if it does not already exist */
         pif = bcm_mcast_if_lookup(dev->ifindex);
         if ( NULL == pif )
         {
            bcm_mcast_if_create(dev->ifindex, BRTYPE_LINUX);
         }
      }
   }
   rcu_read_unlock();
   return 0;
}

void bcm_mcast_process_ovs_brinfo_update(t_ovs_mcpd_brcfg_info *pBrcfg)
{
#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
    int brIdx;
    int portIdx;
    bcm_mcast_ifdata  *pif;
    int deletebr = 0;

    rcu_read_lock();
    for (brIdx = 0; brIdx < pBrcfg->numbrs; brIdx++) 
    {
        pif = bcm_mcast_if_lookup(pBrcfg->ovs_br[brIdx]);
        if ( NULL == pif )
        {
            pif = bcm_mcast_if_create(pBrcfg->ovs_br[brIdx], BRTYPE_OVS);
            if (pif == NULL) 
            {
                printk("%s bcm_mcast_if_create() returned error. Unable to create pif Interface\n", __func__);
                rcu_read_unlock();
                return;
            }
            __logDebug("%s pif created %d", __func__, pBrcfg->ovs_br[brIdx]);
        }
        for (portIdx = 0; portIdx < pBrcfg->numports[brIdx]; portIdx++) 
        {
            bcm_mcast_lower_port* lowerPort = NULL;

            spin_lock_bh(&pif->config_lock);
            lowerPort = bcm_mcast_if_get_lower_port_by_ifindex(pif, pBrcfg->ovs_ports[brIdx][portIdx]);

            /* Only add the lowerPort if it doesn't exist yet */
            if (NULL == lowerPort)
            {
                bcm_mcast_if_add_lower_port_to_if(pBrcfg->ovs_ports[brIdx][portIdx], pif);
            }
            spin_unlock_bh(&pif->config_lock);
        }
    }

    /* Delete OVS bridges that are not part of the configuration update */
    hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
    {
        if (pif->brtype == BRTYPE_OVS) 
        {
            deletebr = 1;
            for (brIdx = 0; brIdx < pBrcfg->numbrs; brIdx++) 
            {
                if ( pif->ifindex == pBrcfg->ovs_br[brIdx] )
                {
                    /* Found the bridge in the configuration update,
                       no need to delete, skip */
                    deletebr = 0;
                    break;
                }
            }

            if (deletebr) 
            {
                __logDebug("%s bridge %d NOT FOUND %s, delete", __func__, pif->ifindex, pif->dev->name );
                bcm_mcast_if_delete(pif->ifindex);
            }
        }
   }
   rcu_read_unlock();
#endif /* defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP) */
}

#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
static void *bcm_mcast_if_seq_start(struct seq_file *seq, loff_t *pos)
{
   bcm_mcast_ifdata *pif;
   loff_t offs = 0;

   rcu_read_lock();
   hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
   {
      if (*pos == offs)
      {
         return pif;
      }
      ++offs;
   }
   return NULL;
}

static void *bcm_mcast_if_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
   bcm_mcast_ifdata *pif = v;
   ++*pos;
   pif = (bcm_mcast_ifdata *)rcu_dereference(hlist_next_rcu(&pif->hlist));
   return pif;
}

static void bcm_mcast_if_seq_stop(struct seq_file *seq, void *v)
{
   rcu_read_unlock();
}

#if defined(CONFIG_BR_IGMP_SNOOP)
static int bcm_mcast_if_seq_open_ipv4(struct inode *inode, struct file *file)
{
   return seq_open(file, &mcast_ctrl->ipv4_seq_ops);
}

static int bcm_mcast_if_seq_show_ipv4(struct seq_file *seq, void *v)
{
   bcm_mcast_ifdata *pif = v;

   bcm_mcast_igmp_display(seq, pif);
   
   return 0;
}
#endif

#if defined(CONFIG_BR_MLD_SNOOP)
static int bcm_mcast_if_seq_open_ipv6(struct inode *inode, struct file *file)
{
   return seq_open(file, &mcast_ctrl->ipv6_seq_ops);
}

static int bcm_mcast_if_seq_show_ipv6(struct seq_file *seq, void *v)
{
   bcm_mcast_ifdata *pif = v;

   bcm_mcast_mld_display(seq, pif);
   
   return 0;
}
#endif

#if defined(CONFIG_BLOG)
void bcm_mcast_if_process_blog_enable( int enable )
{
   bcm_mcast_ifdata *pif;
   rcu_read_lock();
   hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
      spin_lock_bh(&pif->mc_igmp_lock);
      bcm_mcast_igmp_process_blog_enable(pif, enable);
      spin_unlock_bh(&pif->mc_igmp_lock);
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      spin_lock_bh(&pif->mc_mld_lock);
      bcm_mcast_mld_process_blog_enable(pif, enable);
      spin_unlock_bh(&pif->mc_mld_lock);
#endif
   }
   rcu_read_unlock();
}

#if defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE)
int bcm_mcast_if_wlan_client_disconnect(struct net_device *dev, char *mac)
{
   bcm_mcast_ifdata *pif;

   if(dev && mac)
   {
      __logDebug("%02x%02x%02x%02x%02x%02x client disconnected from net:%s",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], 
                 dev->name );
      rcu_read_lock();
      hlist_for_each_entry_rcu(pif, &mcast_ctrl->if_hlist, hlist)
      {
#if defined(CONFIG_BR_IGMP_SNOOP)
         spin_lock_bh(&pif->mc_igmp_lock);
         bcm_mcast_igmp_wipe_reporter_by_mac(pif, mac);
         spin_unlock_bh(&pif->mc_igmp_lock);
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
         spin_lock_bh(&pif->mc_mld_lock);
         bcm_mcast_mld_wipe_reporter_by_mac(pif, mac);
         spin_unlock_bh(&pif->mc_mld_lock);
#endif
      }
      rcu_read_unlock();
   }
   return 0;
}
#endif /* CONFIG_BCM_WLAN */
#endif /* CONFIG_BLOG */
#endif /* CONFIG_BR_IGMP_SNOOP || CONFIG_BR_MLD_SNOOP */

void bcm_mcast_if_deinit( void )
{
#if defined(CONFIG_BR_IGMP_SNOOP)
   if (mcast_ctrl->ipv4_proc_entry)
   {
      proc_remove(mcast_ctrl->ipv4_proc_entry);
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if (mcast_ctrl->ipv6_proc_entry)
   {
      proc_remove(mcast_ctrl->ipv6_proc_entry);
   }
#endif
   if ( mcast_ctrl->netdev_notifier.notifier_call == bcm_mcast_if_netdev_notifier )
   {
       unregister_netdevice_notifier( &mcast_ctrl->netdev_notifier );
   }
   if ( mcast_ctrl->bridge_notifier.notifier_call == bridge_notifier )
   {
       unregister_bridge_notifier(&mcast_ctrl->bridge_notifier);
   }
   bcm_mcast_if_delete(0);
}

__init int bcm_mcast_if_init( void )
{
   /* discover existing interfaces before installing notifier to ensure
      we only have one writer of if_list */
   bcm_mcast_if_discover();
   
   mcast_ctrl->netdev_notifier.notifier_call = bcm_mcast_if_netdev_notifier,
   register_netdevice_notifier( &mcast_ctrl->netdev_notifier );

   mcast_ctrl->bridge_notifier.notifier_call = bridge_notifier,
   register_bridge_notifier( &mcast_ctrl->bridge_notifier );

#if defined(CONFIG_BR_IGMP_SNOOP)
   mcast_ctrl->ipv4_file_io.owner = THIS_MODULE;
   mcast_ctrl->ipv4_file_io.open = bcm_mcast_if_seq_open_ipv4;
   mcast_ctrl->ipv4_file_io.read = seq_read; 
   mcast_ctrl->ipv4_file_io.llseek = seq_lseek;
   mcast_ctrl->ipv4_file_io.release = seq_release;

   mcast_ctrl->ipv4_seq_ops.start = bcm_mcast_if_seq_start;
   mcast_ctrl->ipv4_seq_ops.next = bcm_mcast_if_seq_next;
   mcast_ctrl->ipv4_seq_ops.stop = bcm_mcast_if_seq_stop;
   mcast_ctrl->ipv4_seq_ops.show = bcm_mcast_if_seq_show_ipv4;

   mcast_ctrl->ipv4_proc_entry = proc_create("igmp_snooping", 0, init_net.proc_net, &mcast_ctrl->ipv4_file_io);
   if ( NULL == mcast_ctrl->ipv4_proc_entry )
   {
      return -ENOMEM;
   }
#endif

#if defined(CONFIG_BR_MLD_SNOOP)
   mcast_ctrl->ipv6_file_io.owner = THIS_MODULE;
   mcast_ctrl->ipv6_file_io.open = bcm_mcast_if_seq_open_ipv6;
   mcast_ctrl->ipv6_file_io.read = seq_read; 
   mcast_ctrl->ipv6_file_io.llseek = seq_lseek;
   mcast_ctrl->ipv6_file_io.release = seq_release;

   mcast_ctrl->ipv6_seq_ops.start = bcm_mcast_if_seq_start;
   mcast_ctrl->ipv6_seq_ops.next = bcm_mcast_if_seq_next;
   mcast_ctrl->ipv6_seq_ops.stop = bcm_mcast_if_seq_stop;
   mcast_ctrl->ipv6_seq_ops.show = bcm_mcast_if_seq_show_ipv6;

   mcast_ctrl->ipv6_proc_entry = proc_create("mld_snooping", 0, init_net.proc_net, &mcast_ctrl->ipv6_file_io);
   if ( NULL == mcast_ctrl->ipv6_proc_entry )
   {
      return -ENOMEM;
   }
#endif

   return 0;
}

