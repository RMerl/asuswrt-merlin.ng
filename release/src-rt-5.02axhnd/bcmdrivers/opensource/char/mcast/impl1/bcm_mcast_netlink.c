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
#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include "bcm_mcast_priv.h"

//#define BCM_MCAST_NETLINK_MSG_DEBUG

static void bcm_mcast_dump_buf(char *msg, char *buf, int len)
{
#if defined(BCM_MCAST_NETLINK_MSG_DEBUG)
    int i;
    printk("%s\n", msg);
    for(i =0; i < len; i++) 
    {
        printk("%02x ", (unsigned char)buf[i]);
        if(0 == ((i+1)%32))
        {
            printk("\n");
        }
    }
    printk("\nmessage end\n");
#endif
}

static int bcm_mcast_netlink_get_client(void)
{
    int client = 0;
    if ( mcast_ctrl->ctrl_client[0] )
    {
        client = mcast_ctrl->ctrl_client[0];
    }
    else if ( mcast_ctrl->ctrl_client[1] )
    {
        client = mcast_ctrl->ctrl_client[1];
    }
    return client;
}

#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
static int bcm_mcast_netlink_get_filter_mode(int mode)
{
   int filter_mode;
   if((mode == BCM_MCAST_SNOOP_IN_ADD) ||
      (mode == BCM_MCAST_SNOOP_IN_CLEAR)) 
   {
       filter_mode = MCAST_INCLUDE;
   }
   else if((mode == BCM_MCAST_SNOOP_EX_ADD) ||
           (mode == BCM_MCAST_SNOOP_EX_CLEAR)) 
   {
       filter_mode = MCAST_EXCLUDE;
   }
   else
   {
       return -EINVAL;
   }
   return filter_mode;
}

struct sk_buff *bcm_mcast_netlink_alloc_skb(int len, int proto)
{
   int                     buf_size;
   struct sk_buff         *skb;
   int                     client;

   /* if there isn't a client to send to - return NULL */
   client = bcm_mcast_netlink_get_client();
   if (0 == client)
   {
      return NULL;
   }

   buf_size = NLMSG_SPACE(len);
   skb = alloc_skb(buf_size, GFP_ATOMIC);
   if ( skb == NULL )
   {
      __logError("failed to allocate skb");
      return NULL;
   }
   skb_put(skb, buf_size);

   /* leave room for nl header */
   __skb_pull(skb, NLMSG_HDRLEN);
   return skb;
} /* bcm_mcast_netlink_send_skb */

/* this function consumes the skb */
int bcm_mcast_netlink_send_skb(struct sk_buff *skb, int msg_type)
{
   struct nlmsghdr        *nlh;
   int                     client;

   client = bcm_mcast_netlink_get_client();
   if (0 == client)
   {
      kfree_skb(skb);
      return 0;
   }

   nlh = (struct nlmsghdr *)skb_push(skb, NLMSG_HDRLEN);
   nlh->nlmsg_len = skb->len;
   nlh->nlmsg_pid = 0;
   nlh->nlmsg_flags = 0;
   nlh->nlmsg_seq = 0;
   nlh->nlmsg_type = msg_type;

   NETLINK_CB(skb).dst_group = 0;
   NETLINK_CB(skb).portid = client;

   bcm_mcast_dump_buf("IGMP/MLD packet", (char *)nlh, nlh->nlmsg_len);
   netlink_unicast(mcast_ctrl->netlink_sock, skb, client, MSG_DONTWAIT);

   return 0;
} /* bcm_mcast_netlink_send_skb */
#endif

static int bcm_mcast_netlink_send(unsigned int client, t_BCM_MCAST_MSGTYPES msg_type, void *pmsg_data, int data_len)
{
   struct nlmsghdr *nlh;
   struct sk_buff  *skb;
   int              buf_size;
   unsigned char   *ptr;

   if (0 == client)
   {
      client = bcm_mcast_netlink_get_client();
      if (0 == client)
      {
         return 0;
      }
   }

   buf_size = NLMSG_SPACE(data_len);
   skb = alloc_skb(buf_size, GFP_ATOMIC);
   if ( skb == NULL )
   {
      __logError("failed to allocate skb");
      return -ENOMEM;
   }

   nlh = nlmsg_hdr(skb);
   nlh->nlmsg_type = msg_type;
   nlh->nlmsg_len = buf_size;
   nlh->nlmsg_pid = 0;
   nlh->nlmsg_flags = 0;
   nlh->nlmsg_seq = 0;
   skb_put(skb, buf_size);

   ptr = NLMSG_DATA(nlh);
   memcpy(ptr, pmsg_data, data_len);

   /* send unicast to specific client only */
   NETLINK_CB(skb).dst_group = 0;
   NETLINK_CB(skb).portid = client;
   bcm_mcast_dump_buf("Message TX start", (char *)nlh, nlh->nlmsg_len);
   netlink_unicast(mcast_ctrl->netlink_sock, skb, client, MSG_DONTWAIT);

   return 0;
} /* bcm_mcast_netlink_send */

static int bcm_mcast_netlink_process_registration(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_REGISTER *reg;
   t_BCM_MCAST_REGISTER  reg_rsp;
   int                   rv = 0;
   int                   index;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*reg))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   reg = (t_BCM_MCAST_REGISTER *)pdata;
   do
   {
      if ( 0 == nlh->nlmsg_pid )
      {
         __logError("invalid id");
         rv = -EINVAL;
         break;
      }

      if ( (reg->primary != 0) && (reg->primary != 1 ))
      {
         rv = -EINVAL;
         break;
      }
      index = reg->primary ? 0 : 1;
     
      if ( (mcast_ctrl->ctrl_client[index] != 0) && 
           (mcast_ctrl->ctrl_client[index] != nlh->nlmsg_pid) )
      {
         __logError("registration unavailable");
         rv = -EBUSY;
         break;
      }
      mcast_ctrl->ctrl_client[index] = nlh->nlmsg_pid;
   } while (0);

   /* send response to client */
   reg_rsp.result = rv;
   reg_rsp.primary = reg->primary;
   bcm_mcast_netlink_send(nlh->nlmsg_pid, BCM_MCAST_MSG_REGISTER, &reg_rsp, sizeof(reg_rsp));

   /* send response to other client */
   if ( 0 == rv )
   {
      if ( reg->primary && mcast_ctrl->ctrl_client[1] )
      {
         bcm_mcast_netlink_send(mcast_ctrl->ctrl_client[1], BCM_MCAST_MSG_REGISTER, &reg_rsp, sizeof(reg_rsp));
      }
      else if ( !reg->primary && mcast_ctrl->ctrl_client[0] )
      {
         bcm_mcast_netlink_send(mcast_ctrl->ctrl_client[0], BCM_MCAST_MSG_REGISTER, &reg_rsp, sizeof(reg_rsp));
      }
   }
   return rv;
} /* bcm_mcast_netlink_process_registration */

static int bcm_mcast_netlink_process_unregistration(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_REGISTER *reg;
   t_BCM_MCAST_REGISTER  reg_rsp;
   int                   rv = 0;
   int                   index;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*reg))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   reg = (t_BCM_MCAST_REGISTER *)pdata;
   do
   {
      if ( 0 == nlh->nlmsg_pid )
      {
         __logError("invalid id");
         rv = -1;
         break;
      }

      if ( (reg->primary != 0) && (reg->primary != 1) )
      {
         rv = -EINVAL;
         break;
      }
      index = reg->primary ? 0 : 1;

      if ( mcast_ctrl->ctrl_client[index] != nlh->nlmsg_pid)
      {
         __logError("Client is not registered");
         rv = -ENOENT;
         break;
      }
      mcast_ctrl->ctrl_client[index] = 0;
   } while (0);

   /* send response to client */
   reg_rsp.result = rv;
   reg_rsp.primary = reg->primary;
   bcm_mcast_netlink_send(nlh->nlmsg_pid, BCM_MCAST_MSG_UNREGISTER, &reg_rsp, sizeof(reg_rsp));

   /* if successful send response to other client */
   if ( 0 == rv )
   {
      if ( reg->primary && mcast_ctrl->ctrl_client[1] )
      {
         bcm_mcast_netlink_send(mcast_ctrl->ctrl_client[1], BCM_MCAST_MSG_REGISTER, &reg_rsp, sizeof(reg_rsp));
      }
      else if ( !reg->primary && mcast_ctrl->ctrl_client[0] )
      {
         bcm_mcast_netlink_send(mcast_ctrl->ctrl_client[0], BCM_MCAST_MSG_REGISTER, &reg_rsp, sizeof(reg_rsp));
      }
   }

   return rv;
} /* bcm_mcast_netlink_process_unregistration */

static int bcm_mcast_netlink_process_igmp_snoop_entry(struct nlmsghdr *nlh, unsigned char *pdata)
{
#if defined(CONFIG_BR_IGMP_SNOOP)
   bcm_mcast_ifdata             *pif;
   t_BCM_MCAST_IGMP_SNOOP_ENTRY *snoop_entry;
   struct net_device            *from_dev = NULL;
   struct net_device            *to_dev = NULL;
   int                           idx = 0;
   uint32_t                      info = 0;
   int                           filter_mode;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*snoop_entry))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
      return -EINVAL;
   }

   snoop_entry = (t_BCM_MCAST_IGMP_SNOOP_ENTRY *)pdata;
   filter_mode = bcm_mcast_netlink_get_filter_mode(snoop_entry->mode);
   if (filter_mode < 0 )
   {
      return filter_mode;
   }

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(snoop_entry->parent_ifi);
   if ( pif == NULL )
   {
      rcu_read_unlock();
      __logError("unable to find interface with index %d", snoop_entry->parent_ifi);
      return -EINVAL;
   }

   to_dev = dev_get_by_index(&init_net, snoop_entry->dstdev_ifi);
   if (NULL == to_dev)
   {
      __logError("device %d could not be found", snoop_entry->dstdev_ifi);
      rcu_read_unlock();
      return -EINVAL;
   }

   if ((0 == (to_dev->flags & IFF_UP)) ||
       (0 == bcm_mcast_if_is_associated_dev(to_dev, pif->ifindex)))
   {
      __logError("invalid device %d for snooping entry", snoop_entry->dstdev_ifi);
      dev_put(to_dev);
      rcu_read_unlock();
      return -EINVAL;
   }

#if defined(CONFIG_BLOG)
   bcm_mcast_blog_get_rep_info(to_dev, snoop_entry->repMac, &info);
#endif

   for(idx = 0; idx < BCM_MCAST_MAX_SRC_IF; idx++)
   {
       if(snoop_entry->wan_info[idx].if_ops)
       {
           from_dev = dev_get_by_index(&init_net, 
                                       snoop_entry->wan_info[idx].ifi);
           if (NULL == from_dev)
              continue;

           if (0 == (from_dev->flags & IFF_UP))
           {
               dev_put(from_dev);
               continue;
           }

           if((snoop_entry->mode == BCM_MCAST_SNOOP_IN_CLEAR) ||
              (snoop_entry->mode == BCM_MCAST_SNOOP_EX_CLEAR)) 
           {
               bcm_mcast_igmp_remove(from_dev,
                                         pif, 
                                         to_dev,
                                         &snoop_entry->rxGrp, 
                                         &snoop_entry->txGrp, 
                                         &snoop_entry->rep,
                                         filter_mode, 
                                         &snoop_entry->src,
                                         info);
           }
           else
           {
               if((snoop_entry->wan_info[idx].if_ops == BCM_MCAST_IF_BRIDGED) && 
                  (0 == bcm_mcast_if_is_associated_dev(from_dev, pif->ifindex)))
               {
                  dev_put(from_dev);
                  continue;
               }

               bcm_mcast_igmp_add(from_dev,
                                  snoop_entry->wan_info[idx].if_ops,
                                  pif, 
                                  to_dev, 
                                  &snoop_entry->rxGrp,
                                  &snoop_entry->txGrp,
                                  &snoop_entry->rep,
                                  snoop_entry->repMac,
                                  snoop_entry->rep_proto_ver,
                                  filter_mode, 
                                  snoop_entry->tci,
                                  &snoop_entry->src,
                                  snoop_entry->lanppp,
                                  snoop_entry->excludePort,
                                  snoop_entry->enRtpSeqCheck,
                                  info);
           }
           dev_put(from_dev);
       }
       else
       {
           continue;
       }
   }

   /* if LAN-2-LAN snooping enabled make an entry                         *
    * unless multicast DNAT is being used (txGrp and rxGrp are different) */
   if (bcm_mcast_get_lan2lan_snooping(BCM_MCAST_PROTO_IPV4, pif) &&
       (snoop_entry->rxGrp.s_addr == snoop_entry->txGrp.s_addr) ) 
   {
       if((snoop_entry->mode == BCM_MCAST_SNOOP_IN_CLEAR) ||
          (snoop_entry->mode == BCM_MCAST_SNOOP_EX_CLEAR)) 
       {
           bcm_mcast_igmp_remove(pif->dev,
                                 pif,
                                 to_dev, 
                                 &snoop_entry->txGrp, 
                                 &snoop_entry->txGrp, 
                                 &snoop_entry->rep, 
                                 filter_mode, 
                                 &snoop_entry->src,
                                 0);
       }
       else
       {
           bcm_mcast_igmp_add(pif->dev,
                                  BCM_MCAST_IF_BRIDGED, 
                                  pif,
                                  to_dev, 
                                  &snoop_entry->txGrp, 
                                  &snoop_entry->txGrp, 
                                  &snoop_entry->rep,
                                  snoop_entry->repMac,
                                  snoop_entry->rep_proto_ver,
                                  filter_mode, 
                                  snoop_entry->tci,
                                  &snoop_entry->src,
                                  snoop_entry->lanppp,
                                  -1,
                                  0,
                                  0);
       }
   }

   dev_put(to_dev);
   rcu_read_unlock();
#endif
   return 0;
} /* bcm_mcast_nl_process_igmp_snoop_entry*/

static int bcm_mcast_netlink_process_mld_snoop_entry(struct nlmsghdr *nlh, unsigned char *pdata)
{
#if defined(CONFIG_BR_MLD_SNOOP)
   bcm_mcast_ifdata            *pif;
   t_BCM_MCAST_MLD_SNOOP_ENTRY *snoop_entry;
   struct net_device           *from_dev= NULL;
   struct net_device           *to_dev= NULL;
   int                          idx = 0;
   uint32_t                     info = 0;
   int                          filter_mode;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*snoop_entry))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   snoop_entry = (t_BCM_MCAST_MLD_SNOOP_ENTRY *)pdata;
   filter_mode = bcm_mcast_netlink_get_filter_mode(snoop_entry->mode);
   if (filter_mode < 0 )
   {
     return filter_mode;
   }

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(snoop_entry->parent_ifi);
   if ( pif == NULL )
   {
      rcu_read_unlock();
      __logError("unable to find interface with index %d", snoop_entry->parent_ifi);
      return -EINVAL;
   }

   to_dev = dev_get_by_index(&init_net, snoop_entry->dstdev_ifi);
   if (NULL == to_dev)
   {
      rcu_read_unlock();
      __logError("device %d could not be found", snoop_entry->dstdev_ifi);
      return -EINVAL;
   }

   if ((0 == (to_dev->flags & IFF_UP)) ||
       (0 == bcm_mcast_if_is_associated_dev(to_dev, pif->ifindex)))
   {
       rcu_read_unlock();
       __logError("invalid device %d for snooping entry", snoop_entry->dstdev_ifi);
       dev_put(to_dev);
       return -EINVAL;
   }

#if defined(CONFIG_BLOG)
   bcm_mcast_blog_get_rep_info(to_dev, snoop_entry->repMac, &info);
#endif

   for(idx = 0; idx < BCM_MCAST_MAX_SRC_IF; idx++)
   {
       if(snoop_entry->wan_info[idx].if_ops)
       {
           from_dev = dev_get_by_index(&init_net, 
                                       snoop_entry->wan_info[idx].ifi);
           if(NULL == from_dev)
              continue;

           if (0 == (from_dev->flags & IFF_UP))
           {
               dev_put(from_dev);
               continue;
           }

           if((snoop_entry->mode == BCM_MCAST_SNOOP_IN_CLEAR) ||
              (snoop_entry->mode == BCM_MCAST_SNOOP_EX_CLEAR)) 
           {
               bcm_mcast_mld_remove(from_dev,
                                    pif, 
                                    to_dev, 
                                    &snoop_entry->grp,
                                    &snoop_entry->rep, 
                                    filter_mode, 
                                    &snoop_entry->src,
                                    info);
           }
           else
           {
               if ((snoop_entry->wan_info[idx].if_ops == BCM_MCAST_IF_BRIDGED) && 
                   (0 == bcm_mcast_if_is_associated_dev(from_dev, pif->ifindex)))
               {
                  dev_put(from_dev);
                  continue;
               }

               bcm_mcast_mld_add(from_dev,
                                 snoop_entry->wan_info[idx].if_ops,
                                 pif, 
                                 to_dev, 
                                 &snoop_entry->grp, 
                                 &snoop_entry->rep,
                                 snoop_entry->repMac,
                                 snoop_entry->rep_proto_ver,
                                 filter_mode, 
                                 snoop_entry->tci, 
                                 &snoop_entry->src,
                                 snoop_entry->lanppp,
                                 info);
           }
           dev_put(from_dev);
       }
       else
       {
           continue;
       }
   }

   /* if LAN-2-LAN snooping enabled make an entry */
   if(bcm_mcast_get_lan2lan_snooping(BCM_MCAST_PROTO_IPV6, pif))
   {
       if((snoop_entry->mode == BCM_MCAST_SNOOP_IN_CLEAR) ||
          (snoop_entry->mode == BCM_MCAST_SNOOP_EX_CLEAR)) 
       {
           bcm_mcast_mld_remove(pif->dev, 
                                pif,
                                to_dev, 
                                &snoop_entry->grp, 
                                &snoop_entry->rep, 
                                filter_mode, 
                                &snoop_entry->src,
                                0);
       }
       else
       {
           bcm_mcast_mld_add(pif->dev,
                             BCM_MCAST_IF_BRIDGED,
                             pif,
                             to_dev, 
                             &snoop_entry->grp, 
                             &snoop_entry->rep,
                             snoop_entry->repMac,
                             snoop_entry->rep_proto_ver,
                             filter_mode, 
                             snoop_entry->tci, 
                             &snoop_entry->src,
                             snoop_entry->lanppp,
                             0);
       }
   }
   dev_put(to_dev);
   rcu_read_unlock();
#endif
   return 0;
}

static int bcm_mcast_netlink_process_if_change(struct nlmsghdr *nlh, unsigned char *pdata)
{
   struct net_device     *ndev = NULL;
   t_BCM_MCAST_IF_CHANGE *ifChgMsg;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*ifChgMsg))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   ifChgMsg = (t_BCM_MCAST_IF_CHANGE *)pdata;
   ndev = dev_get_by_index(&init_net, ifChgMsg->ifi);
   if(!ndev)
   {
       /* device has likely been removed */
       return 0;
   }

   bcm_mcast_if_update_bydev(ifChgMsg->proto, ndev, 0);
   bcm_mcast_if_admission_update_bydev(ifChgMsg->proto, ndev);
   dev_put(ndev);

   return 0;
} /* bcm_mcast_nl_process_if_change */

static int bcm_mcast_netlink_process_mc_fdb_cleanup(struct nlmsghdr *nlh, unsigned char *pdata)
{
   /* read is not accepted */
   if ( (NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags )
   {
      return -EINVAL;
   }

   bcm_mcast_if_admission_update_bydev(BCM_MCAST_PROTO_ALL, NULL);
   bcm_mcast_if_update_bydev(BCM_MCAST_PROTO_ALL, NULL, 0);
   return 0;
}

static int bcm_mcast_netlink_process_mcast_pri_queue(struct nlmsghdr *nlh, unsigned char *pdata)
{
    t_BCM_MCAST_PRIORITY_QUEUE *pq;

    /* make sure length is correct, read is not accepted */
    if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*pq))) ||
         ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
    {
        return -EINVAL;
    }

    pq = (t_BCM_MCAST_PRIORITY_QUEUE *)pdata;
    if ( (pq->pri_queue < -1) || (pq->pri_queue > 7) )
    {
      return -EINVAL;
    }
    bcm_mcast_set_pri_queue(pq->pri_queue);

    return 0;
}

static int bcm_mcast_netlink_process_uplink_indication(struct nlmsghdr *nlh, unsigned char *pdata)
{
#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
    t_BCM_MCAST_UPLINK *pup;

    /* make sure length is correct, read is not accepted */
    if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*pup))) ||
         ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
    {
        return -EINVAL;
    }

    pup = (t_BCM_MCAST_UPLINK *)pdata;

    if ( (true != pup->uplink) &&
         (false != pup->uplink) )
    {
      return -EINVAL;
    }
    bcm_mcast_set_uplink(pup->uplink);
#endif
    return 0;
}

static int bcm_mcast_netlink_process_igmp_purge_reporter(struct nlmsghdr *nlh, unsigned char *pdata)
{
#if defined(CONFIG_BR_IGMP_SNOOP)
   t_BCM_MCAST_IGMP_PURGE_REPORTER* purge_data = NULL;
   bcm_mcast_ifdata *pif;
   struct net_device *rep_dev = NULL;   

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*purge_data))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   purge_data = (t_BCM_MCAST_IGMP_PURGE_REPORTER*)pdata;

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(purge_data->parent_ifi);
   if ( NULL == pif )
   {
      rcu_read_unlock();
      return -ENODEV;
   }

   rep_dev = dev_get_by_index(&init_net, purge_data->dstdev_ifi);
   if (rep_dev == NULL)
   {
      rcu_read_unlock();
      return -ENODEV;
   }

   bcm_mcast_igmp_wipe_reporter_for_port(pif, &purge_data->rep, rep_dev);
   dev_put(rep_dev);
   rcu_read_unlock();
#endif
   return 0;
}

static int bcm_mcast_netlink_process_mld_purge_reporter(struct nlmsghdr *nlh, unsigned char *pdata)
{
#if defined(CONFIG_BR_MLD_SNOOP)
   t_BCM_MCAST_MLD_PURGE_REPORTER* purge_data;
   bcm_mcast_ifdata *pif;
   struct net_device *rep_dev;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*purge_data))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   purge_data = (t_BCM_MCAST_MLD_PURGE_REPORTER*)pdata;
   rcu_read_lock();
   pif = bcm_mcast_if_lookup(purge_data->parent_ifi);
   if ( NULL == pif )
   {
      rcu_read_unlock();
      return -ENODEV;
   }

   rep_dev = dev_get_by_index(&init_net, purge_data->dstdev_ifi);
   if (rep_dev == NULL)
   {
      rcu_read_unlock();
      return -ENODEV;
   }

   bcm_mcast_mld_wipe_reporter_for_port(pif, &purge_data->rep, rep_dev);
   dev_put(rep_dev);
   rcu_read_unlock();
#endif
   return 0;
}

static int bcm_mcast_netlink_process_set_admission_control(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_ADMISSION_FILTER *padmission;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*padmission))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   padmission = (t_BCM_MCAST_ADMISSION_FILTER *)pdata;

#if defined(CONFIG_BR_IGMP_SNOOP)
   /* IGMP ADMISSION */
   if ((true != padmission->igmpAdmissionEn) && 
       (false != padmission->igmpAdmissionEn))
   {
      
      return -EINVAL;
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   /* MLD ADMISSION */
   if ((true != padmission->mldAdmissionEn) && 
       (false != padmission->mldAdmissionEn))
   {
      return -EINVAL;
   }
#endif

#if defined(CONFIG_BR_IGMP_SNOOP)
   bcm_mcast_set_admission(BCM_MCAST_PROTO_IPV4, padmission->igmpAdmissionEn);
#endif       
#if defined(CONFIG_BR_MLD_SNOOP)
   bcm_mcast_set_admission(BCM_MCAST_PROTO_IPV6, padmission->mldAdmissionEn);
#endif
    return 0;
}

static int bcm_mcast_netlink_process_admission_result(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_ADMISSION_RESULT *admit;
   struct net_device            *dev;

   /* make sure length is correct, read is not accepted */
   if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*admit))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
       return -EINVAL;
   }

   admit = (t_BCM_MCAST_ADMISSION_RESULT *)pdata;
   if ( (admit->admitted != true) && (admit->admitted != false) )
   {
       return -EINVAL;
   }

   /* proto cannpt be BCM_MCAST_PROTO_ALL */
   if ((admit->proto != BCM_MCAST_PROTO_IPV4) &&
       (admit->proto != BCM_MCAST_PROTO_IPV6))
   {
       return -EINVAL;
   }

   dev = dev_get_by_index(&init_net, admit->parent_ifi);
   if ( NULL == dev )
   {
      return -ENODEV;
   }

   bcm_mcast_if_admission_process(admit->proto, dev, admit->packetIndex, admit->admitted);
   dev_put(dev);
   return 0;
}


static int bcm_mcast_netlink_process_snoop_cfg(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_SNOOP_CFG *cfg;
   bcm_mcast_ifdata      *pif;
   int                    ret = 0;

   /* make sure length is correct */
   if ( nlh->nlmsg_len != NLMSG_SPACE(sizeof(*cfg)) )
   {
       return -EINVAL;
   }

   cfg = (t_BCM_MCAST_SNOOP_CFG *)pdata;
   rcu_read_lock();
   pif = bcm_mcast_if_lookup(cfg->ifi);
   if( NULL == pif )
   {
       rcu_read_unlock();
       __logError("interface %d could not be found", cfg->ifi);
       return -EINVAL;
   }

   do
   {
      /* write request */
      if ( (NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) == nlh->nlmsg_flags )
      {
         if ( (cfg->mode > BCM_MCAST_SNOOPING_BLOCKING_MODE) || 
              (cfg->l2lenable < 0) || (cfg->l2lenable > 1) ||
              ((cfg->proto != BCM_MCAST_PROTO_IPV4) &&
               (cfg->proto != BCM_MCAST_PROTO_IPV6)) )
         {
            ret = -EINVAL;
            break;
         }

         do
         {
#if defined(CONFIG_BR_IGMP_SNOOP)
            if ( cfg->proto == BCM_MCAST_PROTO_IPV4 )
            {
               pif->igmp_snooping = cfg->mode;
               pif->igmp_lan2lan_mc_enable = cfg->l2lenable;
               break;
            }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
            if ( cfg->proto == BCM_MCAST_PROTO_IPV6 )
            {
               pif->mld_snooping = cfg->mode;
               pif->mld_lan2lan_mc_enable = cfg->l2lenable;
               break;
            }
#endif
         } while (0);
      }
      else if ( NLM_F_REQUEST == nlh->nlmsg_flags )
      {
         t_BCM_MCAST_SNOOP_CFG rsp;
         if ( (cfg->proto != BCM_MCAST_PROTO_IPV4) &&
              (cfg->proto != BCM_MCAST_PROTO_IPV6) )
         {
            ret = -EINVAL;
            break;
         }

         rsp.proto = cfg->proto;
         rsp.ifi = cfg->ifi;
#if defined(CONFIG_BR_IGMP_SNOOP)
         if ( cfg->proto == BCM_MCAST_PROTO_IPV4 )
         {
            rsp.mode = pif->igmp_snooping;
            rsp.l2lenable = bcm_mcast_get_lan2lan_snooping(cfg->proto, pif);
         }
         else
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
         if ( cfg->proto == BCM_MCAST_PROTO_IPV6 )
         {

            rsp.mode = pif->mld_snooping;
            rsp.l2lenable = bcm_mcast_get_lan2lan_snooping(cfg->proto, pif);
         }
         else
#endif
         {
            rsp.mode = 0;
            rsp.l2lenable = 0;
         }
         bcm_mcast_netlink_send(nlh->nlmsg_pid, BCM_MCAST_MSG_SNOOP_CFG, &rsp, sizeof(rsp));         
      }
      else
      {
         ret = -EINVAL;
         break;
      }
   } while (0);
   rcu_read_unlock();
   return ret;
}

static int bcm_mcast_netlink_process_proto_rate_limit(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_PROTO_RATE_LIMIT *cfg;
   bcm_mcast_ifdata             *pif;
   int                           ret = 0;

   /* make sure length is correct */
   if ( nlh->nlmsg_len != NLMSG_SPACE(sizeof(*cfg)) )
   {
       return -EINVAL;
   }

   cfg = (t_BCM_MCAST_PROTO_RATE_LIMIT *)pdata;
   rcu_read_lock();
   pif = bcm_mcast_if_lookup(cfg->ifi);
   if(NULL == pif)
   {
       rcu_read_unlock();
       __logError("interface %d could not be found", cfg->ifi);
       return -EINVAL;
   }

   do
   {
      /* write request */
      if ( (NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) == nlh->nlmsg_flags )
      {
         if ( ((cfg->proto != BCM_MCAST_PROTO_IPV4) &&
               (cfg->proto != BCM_MCAST_PROTO_IPV6)) ||
              (cfg->rate > 500) )
         {
            ret = -EINVAL;
            break;
         }
#if defined(CONFIG_BR_IGMP_SNOOP)
         if ( cfg->proto == BCM_MCAST_PROTO_IPV4 )
         {
            spin_lock_bh(&pif->mc_igmp_lock);
            pif->igmp_rate_limit       = cfg->rate;
            pif->igmp_rate_last_packet = ktime_set(0,0);
            pif->igmp_rate_bucket      = 0;
            pif->igmp_rate_rem_time    = 0;
            spin_unlock_bh(&pif->mc_igmp_lock);
         }
#endif         
         /* mld not supported */
      }
      else if ( NLM_F_REQUEST == nlh->nlmsg_flags )
      {
         t_BCM_MCAST_PROTO_RATE_LIMIT rsp;
         if ( (cfg->proto != BCM_MCAST_PROTO_IPV4) &&
              (cfg->proto != BCM_MCAST_PROTO_IPV6) )
         {
            ret = -EINVAL;
            break;
         }

         rsp.proto = cfg->proto;
         rsp.ifi   = cfg->ifi;
#if defined(CONFIG_BR_IGMP_SNOOP)
         if ( cfg->proto == BCM_MCAST_PROTO_IPV4 )
         {
            rsp.rate = pif->igmp_rate_limit;
         }         
         else
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
         if ( cfg->proto == BCM_MCAST_PROTO_IPV6 )
         {
            rsp.rate = 0;
         }
         else
#endif
         {
            rsp.rate = 0;
         }
         bcm_mcast_netlink_send(nlh->nlmsg_pid, BCM_MCAST_MSG_PROTO_RATE_LIMIT_CFG, &rsp, sizeof(rsp));
      }
      else
      {
         ret = -EINVAL;
         break;
      }
   } while (0);
   rcu_read_unlock();
   return ret;
}


static int bcm_mcast_netlink_process_igmp_drop_group(struct nlmsghdr *nlh, unsigned char *pdata)
{  
#if defined(CONFIG_BR_IGMP_SNOOP)
   t_BCM_MCAST_IGMP_DROP_GROUP_ENTRY* dropMsg = (t_BCM_MCAST_IGMP_DROP_GROUP_ENTRY*)( pdata );
   int retval = -EINVAL;
   bcm_mcast_ifdata* parent_if = NULL;
 
   if ( (nlh->nlmsg_len < NLMSG_SPACE(sizeof(dropMsg[0]))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
      return retval;
   }

   rcu_read_lock();
   parent_if = bcm_mcast_if_lookup(dropMsg->parent_ifindex);
   if (NULL != parent_if)
   {
      retval = bcm_mcast_igmp_wipe_group (parent_if, dropMsg->dest_ifindex, &(dropMsg->group) );
   }
   rcu_read_unlock();

   return retval;
#else
   return 0;
#endif
}

static int bcm_mcast_netlink_process_set_timeout(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_TIMEOUT_ENTRY* timeoutMsg = (t_BCM_MCAST_TIMEOUT_ENTRY*)( pdata );
 
   if ( (nlh->nlmsg_len < NLMSG_SPACE(sizeof(timeoutMsg[0]))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
      return -EINVAL;
   }

   rcu_read_lock();
#if defined(CONFIG_BR_IGMP_SNOOP)
   if (timeoutMsg->proto == IPPROTO_IGMP) 
   {
      mcast_ctrl->igmp_general_query_timeout_secs = timeoutMsg->generalMembershipTimeoutSecs;
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if (timeoutMsg->proto == IPPROTO_ICMP)
   {
      mcast_ctrl->mld_general_query_timeout_secs = timeoutMsg->generalMembershipTimeoutSecs;
   }
#endif
   rcu_read_unlock();

   return 0;

}

static int bcm_mcast_netlink_process_mld_drop_group(struct nlmsghdr *nlh, unsigned char *pdata)
{ 
#if defined(CONFIG_BR_MLD_SNOOP)
   t_BCM_MCAST_MLD_DROP_GROUP_ENTRY* dropMsg = (t_BCM_MCAST_MLD_DROP_GROUP_ENTRY*)( pdata );
   int retval = -EINVAL;
   bcm_mcast_ifdata* parent_if = NULL;
 
   if ( (nlh->nlmsg_len < NLMSG_SPACE(sizeof(dropMsg[0]))) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
      return retval;
   }

   rcu_read_lock();
   parent_if = bcm_mcast_if_lookup(dropMsg->parent_ifindex);
   if (NULL != parent_if)
   {
      retval = bcm_mcast_mld_wipe_group (parent_if, dropMsg->dest_ifindex, &(dropMsg->group) );
   }
   rcu_read_unlock();

   return retval;
#else
   return 0;
#endif
}

  
static int bcm_mcast_netlink_process_ignore_group_list(struct nlmsghdr *nlh, unsigned char *pdata)
{
   t_BCM_MCAST_IGNORE_GROUP_MESSAGE* ignoreMsg = (t_BCM_MCAST_IGNORE_GROUP_MESSAGE*)( pdata );

   if ( (nlh->nlmsg_len < NLMSG_SPACE(sizeof(ignoreMsg[0]))) ||
        ( (nlh->nlmsg_len - NLMSG_SPACE(sizeof(ignoreMsg[0]))) != (sizeof(ignoreMsg->ignoreEntry[0]) * ignoreMsg->count)) ||
        ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
   {
      return -EINVAL;
   }
#if defined(CONFIG_BCM_KF_IGMP) && defined(CONFIG_BR_IGMP_SNOOP)  
   if (BCM_MCAST_PROTO_IPV4 == ignoreMsg->proto) 
   {
      return bcm_mcast_igmp_process_ignore_group_list (ignoreMsg->count, ignoreMsg->ignoreEntry);
   }
#endif
#if defined(CONFIG_BCM_KF_MLD) && defined(CONFIG_BR_MLD_SNOOP)
   if (BCM_MCAST_PROTO_IPV6 == ignoreMsg->proto) 
   {
      return bcm_mcast_mld_process_ignore_group_list (ignoreMsg->count, ignoreMsg->ignoreEntry);
   }
#endif
   return -EINVAL;
}

static int bcm_mcast_netlink_process_blog_enable(struct nlmsghdr *nlh, unsigned char *pdata)
{
#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
#if defined(CONFIG_BLOG)
    t_BCM_MCAST_BLOG_ENABLE *pq;

    /* make sure length is correct, read is not accepted */
    if ( (nlh->nlmsg_len != NLMSG_SPACE(sizeof(*pq))) ||
         ((NLM_F_CREATE|NLM_F_REPLACE|NLM_F_REQUEST) != nlh->nlmsg_flags) )
    {
       return -EINVAL;
    }

    pq = (t_BCM_MCAST_BLOG_ENABLE *)pdata;
    if ( (pq->blog_enable != 0) && (pq->blog_enable != 1) )
    {
       return -EINVAL;
    }
    bcm_mcast_process_blog_enable(pq->blog_enable);
#endif
#endif
    return 0;
}

void bcm_mcast_netlink_send_igmp_purge_entry(bcm_mcast_ifdata *pif,
                                              t_igmp_grp_entry *igmp_entry, 
                                              t_igmp_rep_entry *rep_entry)
{
    t_BCM_MCAST_IGMP_PURGE_ENTRY purge_entry;

    if(!igmp_entry)
        return;

    if(!rep_entry)
        return;

    purge_entry.grp.s_addr    = igmp_entry->txGrp.s_addr;
    purge_entry.src.s_addr    = igmp_entry->src_entry.src.s_addr;
    purge_entry.rep.s_addr    = rep_entry->rep.s_addr;
    purge_entry.parent_ifi    = pif->ifindex;
    purge_entry.rep_ifi       = igmp_entry->dst_dev->ifindex;
    purge_entry.tci           = igmp_entry->lan_tci;
    purge_entry.rep_proto_ver = rep_entry->rep_proto_ver;

    bcm_mcast_netlink_send(0, BCM_MCAST_MSG_IGMP_PURGE_ENTRY, &purge_entry, sizeof(purge_entry));

    return;
} /* bcm_mcast_netlink_send_igmp_purge_entry */

void bcm_mcast_netlink_send_query_trigger(int rep_ifi)
{
    t_BCM_MCAST_QUERY_TRIGGER query_trigger;

    query_trigger.rep_ifi = rep_ifi;

    bcm_mcast_netlink_send(0, BCM_MCAST_MSG_QUERY_TRIGGER, &query_trigger, sizeof(query_trigger));

    return;
} /* bcm_mcast_netlink_send_query_trigger */

typedef int (*process_rcv_func)(struct nlmsghdr *nlh, unsigned char *pdata);
process_rcv_func bcm_mcast_netlink_dispatch[BCM_MCAST_NR_MSGTYPES] =
{
   [BCM_MCAST_MSG_REGISTER             - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_registration,
   [BCM_MCAST_MSG_UNREGISTER           - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_unregistration,
   [BCM_MCAST_MSG_IGMP_PKT             - BCM_MCAST_MSG_BASE] = NULL,
   [BCM_MCAST_MSG_IGMP_SNOOP_ENTRY     - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_igmp_snoop_entry,
   [BCM_MCAST_MSG_MLD_PKT              - BCM_MCAST_MSG_BASE] = NULL,
   [BCM_MCAST_MSG_MLD_SNOOP_ENTRY      - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_mld_snoop_entry,
   [BCM_MCAST_MSG_IGMP_PURGE_ENTRY     - BCM_MCAST_MSG_BASE] = NULL,
   [BCM_MCAST_MSG_MLD_PURGE_ENTRY      - BCM_MCAST_MSG_BASE] = NULL,
   [BCM_MCAST_MSG_IF_CHANGE            - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_if_change,
   [BCM_MCAST_MSG_MC_FDB_CLEANUP       - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_mc_fdb_cleanup,
   [BCM_MCAST_MSG_SET_PRI_QUEUE        - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_mcast_pri_queue,
   [BCM_MCAST_MSG_UPLINK_INDICATION    - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_uplink_indication,
   [BCM_MCAST_MSG_IGMP_PURGE_REPORTER  - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_igmp_purge_reporter,
   [BCM_MCAST_MSG_MLD_PURGE_REPORTER   - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_mld_purge_reporter,
   [BCM_MCAST_MSG_CONTROLS_ADMISSION   - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_set_admission_control,
   [BCM_MCAST_MSG_ADMISSION_RESULT     - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_admission_result,
   [BCM_MCAST_MSG_SNOOP_CFG            - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_snoop_cfg,
   [BCM_MCAST_MSG_PROTO_RATE_LIMIT_CFG - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_proto_rate_limit,
   [BCM_MCAST_MSG_IGNORE_GROUP_LIST    - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_ignore_group_list,
   [BCM_MCAST_MSG_IGMP_DROP_GROUP      - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_igmp_drop_group,
   [BCM_MCAST_MSG_MLD_DROP_GROUP       - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_mld_drop_group,
   [BCM_MCAST_MSG_SET_TIMEOUT          - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_set_timeout,
   [BCM_MCAST_MSG_BLOG_ENABLE          - BCM_MCAST_MSG_BASE] = bcm_mcast_netlink_process_blog_enable,
};

static int bcm_mcast_netlink_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nl_msgHdr)
{
   int type;
   int err = 0;

   type = nl_msgHdr->nlmsg_type;
   bcm_mcast_dump_buf("Message RX start", (char *)nl_msgHdr, nl_msgHdr->nlmsg_len);
   if ( type >= BCM_MCAST_MSG_MAX )
   {
      return -EINVAL;
   }

   /* type minus base is index into bcm_mcast_netlink_dispatch */
   type -= BCM_MCAST_MSG_BASE;
   if ( bcm_mcast_netlink_dispatch[type] )
   {
      err = bcm_mcast_netlink_dispatch[type](nl_msgHdr, NLMSG_DATA(nl_msgHdr));
   }

   return err;
}

static void bcm_mcast_netlink_rcv_skb(struct sk_buff *skb)
{
   netlink_rcv_skb(skb, bcm_mcast_netlink_rcv_msg);
}

/* kernel level notification code */

#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
int bcm_mcast_notify_register(struct notifier_block *nb)
{
   return raw_notifier_chain_register(&mcast_ctrl->mcast_snooping_chain, nb);
}
EXPORT_SYMBOL(bcm_mcast_notify_register);

int bcm_mcast_notify_unregister(struct notifier_block *nb)
{
   return raw_notifier_chain_unregister(&mcast_ctrl->mcast_snooping_chain, nb);
}
EXPORT_SYMBOL(bcm_mcast_notify_unregister);

void bcm_mcast_notify_event(int event, int proto, void *grp_entry, void *rep_entry)
{
#if defined(CONFIG_BR_MLD_SNOOP)
   t_BCM_MCAST_NOTIFY notify;
#endif
   /* right now kernel level notifications are only used by WL for IPv6
      so only raise IPv6 events for WL */
#if 0 //defined(CONFIG_BR_IGMP_SNOOP)
   if ( BCM_MCAST_PROTO_IPV4 == proto )
   {
      t_igmp_grp_entry *grp4 = (t_igmp_grp_entry *)grp_entry;
      t_igmp_rep_entry *rep4 = (t_igmp_rep_entry *)rep_entry;

#if defined(CONFIG_BLOG)
      if (0 == grp4->root)
      {
         return;
      }
#endif 
      if ( (grp4->dst_dev->name[0] == 'w') &&
           (grp4->dst_dev->name[1] == 'l') )
      {
         notify.ifindex = grp4->dst_dev->ifindex;
         notify.proto = proto;
         memcpy(&notify.repMac[0], &rep4->repMac[0], ETH_ALEN);
         notify.ipv4grp.s_addr = grp4->rxGrp.s_addr;
         notify.ipv4src.s_addr = grp4->src_entry.src.s_addr;
         notify.ipv4rep.s_addr = rep4->rep.s_addr;
         raw_notifier_call_chain(&mcast_ctrl->mcast_snooping_chain, event, &notify);
      }
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if ( BCM_MCAST_PROTO_IPV6 == proto )
   {
      t_mld_grp_entry *grp6 = (t_mld_grp_entry *)grp_entry;
      t_mld_rep_entry *rep6 = (t_mld_rep_entry *)rep_entry;

#if defined(CONFIG_BLOG)
      if (0 == grp6->root)
      {
         return;
      }
#endif 
      if ( (grp6->dst_dev->name[0] == 'w') &&
           (grp6->dst_dev->name[1] == 'l') )
      {
         notify.ifindex = grp6->dst_dev->ifindex;
         notify.proto = proto;
         memcpy(&notify.repMac[0], &rep6->repMac[0], ETH_ALEN);
         BCM_IN6_ASSIGN_ADDR(&notify.ipv6grp, &grp6->grp);
         BCM_IN6_ASSIGN_ADDR(&notify.ipv6src, &grp6->src_entry.src);
         BCM_IN6_ASSIGN_ADDR(&notify.ipv6rep, &rep6->rep);
         raw_notifier_call_chain(&mcast_ctrl->mcast_snooping_chain, event, &notify);
      }
   }
#endif
}
#endif

__init int bcm_mcast_netlink_init(void)
{
   struct netlink_kernel_cfg cfg =
   {
      .groups = 0,
      .input  = bcm_mcast_netlink_rcv_skb,
   };

   mcast_ctrl->netlink_sock = netlink_kernel_create(&init_net, NETLINK_BCM_MCAST, &cfg);
   if(mcast_ctrl->netlink_sock == NULL) 
   {
      printk("BCM Multicast: failed to create kernel netlink socket\n");
      return -ENOMEM;
   }

   return 0;
} /* bcm_mcast_netlink_init */

void bcm_mcast_netlink_deinit(void)
{
   if ( mcast_ctrl->netlink_sock )
   {
      sock_release(mcast_ctrl->netlink_sock->sk_socket); 
   }
} /* bcm_mcast_netlink_deinit */

