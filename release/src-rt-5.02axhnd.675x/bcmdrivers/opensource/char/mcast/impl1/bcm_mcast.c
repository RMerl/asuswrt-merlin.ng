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
#include <linux/if_bridge.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/ip.h>
#include <linux/igmp.h>
#include <linux/icmp.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/list.h>
#include <linux/ipv6.h>
#include <linux/blog.h>
#include <net/ip6_checksum.h>
#include <skb_defines.h>

#include "bcm_mcast_priv.h"

extern void (*bcm_mcast_def_pri_queue_hook)(struct sk_buff *);

bcm_mcast_ctrl *mcast_ctrl;

#define MICROSEC_IN_SEC	1000000

/* enable BCM_MCAST_VALIDATE_PACKET to enable IPx header and checksum validation */
#define BCM_MCAST_VALIDATE_PACKET

/* when BCM_MCAST_VALIDATE_PACKET is defined set BCM_MCAST_ERR_DISCARD
   - to 1 to log error and drop packets with checksum or header errors
   - to 0 to log error and accept packets with checksum or header errors 
   Note that if an error is detected processing stops regardless 
   of this setting */
#define BCM_MCAST_ERR_DISCARD 1

/* this module does not support non-linear buffers - if the data in the linear
   part of the buffer is not sufficient to process the packet this packet is 
   returned to Linux without processing */

#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
int bcm_mcast_is_snooping_enabled(struct net_device *dev, int proto)
{
   return bcm_mcast_if_is_snooping_enabled(dev, proto);
}
EXPORT_SYMBOL(bcm_mcast_is_snooping_enabled);

int bcm_mcast_control_filter(void *grp, int proto)
{
#if defined(CONFIG_BR_IGMP_SNOOP)
   if ( BCM_MCAST_PROTO_IPV4 == proto )
   {
      struct in_addr *grp4 = (struct in_addr *)grp;
      return bcm_mcast_igmp_control_filter(grp4->s_addr);
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if ( BCM_MCAST_PROTO_IPV6 == proto )
   {
      struct in6_addr *grp6 = (struct in6_addr *)grp;
      return bcm_mcast_mld_control_filter(grp6);
   }
#endif
   return 1;
}
EXPORT_SYMBOL(bcm_mcast_control_filter);

#if defined(CONFIG_BLOG) && (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
int bcm_mcast_wlan_client_disconnect_notifier(struct net_device *dev, char *mac)
{
   return bcm_mcast_if_wlan_client_disconnect(dev, mac
);
}
EXPORT_SYMBOL(bcm_mcast_wlan_client_disconnect_notifier);
#endif

static int bcm_mcast_fill_pkt_info(bcm_mcast_ifdata *pif, 
                                   struct sk_buff *skb, 
                                   void *pipvx, 
                                   int proto, 
                                   int len, 
                                   t_BCM_MCAST_PKT_INFO *pinfo, 
                                   int flags)
{
   int queued = 0;

   memcpy(pinfo->repMac, skb_mac_header(skb)+ ETH_ALEN, ETH_ALEN);
   pinfo->to_acceldev_ifi = skb->in_dev->ifindex;
   pinfo->rxdev_ifi = skb->dev->ifindex;
   pinfo->parent_ifi = pif->ifindex;
   pinfo->data_len = len;
   pinfo->lanppp = flags & BCM_MCAST_FLAG_PPP ? 1 : 0;

#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
   if(skb->vlan_count)
   {
      pinfo->tci = skb->vlan_header[0] >> 16;
   }
   else
#endif /* CONFIG_BCM_VLAN */
   {
      pinfo->tci = 0;
   }

   pinfo->packetIndex = -1;

   do
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
      if ( proto == BCM_MCAST_PROTO_IPV4 )
      {
         struct iphdr *pip = pipvx;
         pinfo->ipv4rep.s_addr = pip->saddr;
         if (mcast_ctrl->igmp_admission)
         {
            pinfo->packetIndex = bcm_mcast_igmp_admission_queue(pif, skb);
            queued = (pinfo->packetIndex < 0) ? -1 : 1;
         }
         break;
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if ( proto == BCM_MCAST_PROTO_IPV6 )
      {
         struct ipv6hdr *pipv6 = (struct ipv6hdr *)pipvx;
         memcpy(&pinfo->ipv6rep, &pipv6->saddr, sizeof(pinfo->ipv6rep));
         if (mcast_ctrl->mld_admission)
         {
            pinfo->packetIndex = bcm_mcast_mld_admission_queue(pif, skb);
            queued = (pinfo->packetIndex < 0) ? -1 : 1;
         }
         break;
      }
#endif
   } while (0);
   return queued;
}
#endif /* CONFIG_BR_IGMP_SNOOP || CONFIG_BR_MLD_SNOOP */

/* determine if skb contains L2 multicast or IP multicast 
   return < 0 indicates there was an error */
static int bcm_mcast_skb_is_multicast(const struct sk_buff *skb, int *flags, int *iph_offset, unsigned short *proto)
{
   unsigned int      loc_iph_offset;
   int               is_multicast = 0;
   unsigned short    loc_proto;
   int               loc_flags;
   int               read_len;;

   if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
   {
      return 0;
   }

   if ( is_broadcast_ether_addr(eth_hdr(skb)->h_dest) )
   {
      return 0;
   }

   loc_proto = skb->protocol;
   loc_flags = 0;
   loc_iph_offset = 0;
   read_len = skb_network_offset(skb);
   do
   {
      if ( loc_proto == htons(ETH_P_8021Q) )
      {
         const struct vlan_hdr *pvlan = (struct vlan_hdr *)skb_network_header(skb);
         
         /* update ipx header offset and verify that 
            we can read the VLAN header from linear
            data */
         loc_iph_offset += VLAN_HLEN;
         read_len += VLAN_HLEN;
         if ( read_len > skb_headlen(skb) )
         {
            /* non-linear skb - stop processing */
            return 0;
         }

         /* update loc_proto */
         loc_proto = pvlan->h_vlan_encapsulated_proto;

         /* parse single VLAN tag only */
      }

      if ( loc_proto == htons(ETH_P_PPP_SES))
      {
         const struct pppoe_hdr *pppoe;
         pppoe = (struct pppoe_hdr *)(skb_network_header(skb) + loc_iph_offset);

         /* update ipx header offset and verify that 
            we can read the PPP session header from 
            linear data */
         loc_iph_offset += PPPOE_SES_HLEN;
         read_len += PPPOE_SES_HLEN;
         if ( read_len > skb_headlen(skb) )
         {
            /* non-linear skb - stop processing */
            return 0;
         }

         switch (pppoe->tag[0].tag_type)
         {
            case htons(PPP_IP):
               loc_proto = htons(ETH_P_IP);
               break;

#if defined(CONFIG_IPV6)
            case htons(PPP_IPV6):
               loc_proto = htons(ETH_P_IPV6);
               break;
#endif
            default:
               /* not an IPvx packet - ignore */
               return 0;
         }
         loc_flags |= BCM_MCAST_FLAG_PPP;
      }
   } while (0);

   if ( 0 == (loc_flags & BCM_MCAST_FLAG_PPP) )
   {
      /* non PPP traffic must have multicast MAC 
         broadcast was rejected above */
      if ( !is_multicast_ether_addr(eth_hdr(skb)->h_dest) )
      {
         /* ignore packet */
         return 0;
      }
      is_multicast = 1;
   }

   *proto = loc_proto;
   *flags = loc_flags;
   *iph_offset = loc_iph_offset;
   return is_multicast;
}

static int bcm_mcast_skb_is_ip_multicast(const struct sk_buff *skb, int iph_offset, unsigned short proto)
{
   int is_ip_multicast = 0;
   int read_len;

   /* check if packet is IPx multicast */
   if ( proto == htons(ETH_P_IP) )
   {
      const struct iphdr *pip;

      /* verify we can read the IPv4 header from linear data */
      read_len = skb_network_offset(skb) + iph_offset + sizeof(*pip); 
      if ( read_len > skb_headlen(skb) )
      {
         /* non-linear skb - stop processing */
         return 0;
      }

      /* if this is not an ip multicast packet ignore it */
      pip = (struct iphdr *)(skb_network_header(skb) + iph_offset);
      if ( ipv4_is_multicast(pip->daddr) )
      {
         is_ip_multicast = 1;
      }
   }
#if defined(CONFIG_IPV6)
   else if (proto == htons(ETH_P_IPV6) )
   {
      const struct ipv6hdr *pipv6;

      /* verify we can read the IPv6 header from linear */
      read_len = skb_network_offset(skb) + iph_offset + sizeof(*pipv6); 
      if ( read_len > skb_headlen(skb) )
      {
         /* non-linear skb - stop processing */
         return 0;
      }

      /* if this is not an ip multicast packet ignore it */
      pipv6 = (struct ipv6hdr *)(skb_network_header(skb) + iph_offset);
      if ( ipv6_addr_is_multicast(&pipv6->daddr) )
      {
         is_ip_multicast = 1;
      }
   }
#endif
   return is_ip_multicast;
}

static int bcm_mcast_receive_igmp_filter(bcm_mcast_ifdata *pif, struct igmphdr *pigmp, int flags)
{
   if (pigmp != NULL)
   {
#if defined(CONFIG_BCM_GPON_MODULE)
      /* drop IGMP v1 report packets */
      if (pigmp->type == IGMP_HOST_MEMBERSHIP_REPORT)
      {
         /* discard packet */
         return -1;
      }

      /* drop IGMP v1 query packets */
      if ((pigmp->type == IGMP_HOST_MEMBERSHIP_QUERY) &&
          (pigmp->code == 0))
      {
         /* discard packet */
         return -1;
      }

      /* drop IGMP leave packets for group 0.0.0.0 */
      if ((pigmp->type == IGMP_HOST_LEAVE_MESSAGE) &&
          (0 == pigmp->group))
      {
         /* discard packet */
         return -1;
      }
#endif
#if defined(CONFIG_BR_IGMP_SNOOP)
      /* rate limit IGMP */
      if (pif->igmp_rate_limit)
      {
         ktime_t      curTime;
         u64          diffUs;
         unsigned int usPerPacket;
         unsigned int temp32;
         unsigned int burstLimit;

         /* add tokens to the bucket - compute in microseconds */
         curTime = ktime_get();
         usPerPacket = (MICROSEC_IN_SEC / pif->igmp_rate_limit);
         diffUs = ktime_to_us(ktime_sub(curTime, pif->igmp_rate_last_packet));
         pif->diffTotal += diffUs;
         diffUs += pif->igmp_rate_rem_time;

         /* allow 25% burst */
         burstLimit = pif->igmp_rate_limit >> 2;
         if (0 == burstLimit)
         {
            burstLimit = 1;
         }

         if (diffUs > MICROSEC_IN_SEC)
         {
            pif->igmp_rate_bucket = burstLimit;
            pif->igmp_rate_rem_time = 0;
            pif->diffTotal = 0;
            pif->pckInSec = 1;
         }
         else
         {
            if (pif->diffTotal >= MICROSEC_IN_SEC)
            {
                pif->diffTotal = 0;
                pif->pckInSec = 1;
            }

            temp32 = (unsigned int)diffUs / usPerPacket;
            pif->igmp_rate_bucket += temp32;
            if (temp32)
            {
               pif->igmp_rate_rem_time = diffUs - (temp32 * usPerPacket);
            }
         }

         if (pif->igmp_rate_bucket > burstLimit)
         {
            pif->igmp_rate_bucket = burstLimit;
            pif->igmp_rate_rem_time = 0;
         }

         /* if bucket is empty or packets exceed the rate - drop the packet */
         if ((0 == pif->igmp_rate_bucket) || (pif->pckInSec > pif->igmp_rate_limit))
         {
             /* discard packet */
              return -1;
         }

         pif->igmp_rate_bucket--;
         pif->igmp_rate_last_packet.tv64 = curTime.tv64;
         ++pif->pckInSec;
      }
#endif      
   }
   /* packet accepted */
   return 0;
}

static int bcm_mcast_receive_igmp(bcm_mcast_ifdata *pif, 
                                  struct sk_buff *skb_org, 
                                  int iph_offset, 
                                  int flags)
{
   struct iphdr   *pip;
   struct igmphdr *pigmp;
   int             len;
   int             rv;
   unsigned int    grpaddr=0;

   pip = (struct iphdr *)(skb_network_header(skb_org) + iph_offset);

   /* verify we can read IGMP header and data */
   len = skb_network_offset(skb_org) + iph_offset + ntohs(pip->tot_len);
   if ( len > skb_headlen(skb_org) )
   {
      /* non-linear skb - stop processing */
       __logError("non-linear skb - stop processing");
      return 0;
   }

   pigmp = (struct igmphdr *)(skb_network_header(skb_org) + iph_offset + (pip->ihl * 4));

   if (pigmp->type == IGMPV2_HOST_MEMBERSHIP_REPORT) 
   {
       grpaddr = ntohl(pigmp->group);
   }
   else if (pigmp->type == IGMPV3_HOST_MEMBERSHIP_REPORT) 
   {
       struct igmpv3_report *pigmpv3rep = (struct igmpv3_report *)pigmp;
       grpaddr = ntohl(pigmpv3rep->grec[0].grec_mca);
   }

   __logDebug("Received IGMP %s in_dev %s dstip 0x%x srcip 0x%x", 
              IGMP_TYPE_STR(pigmp->type), skb_org->in_dev->name, 
              ntohl(pip->daddr), ntohl(pip->saddr));

   switch (pigmp->type)
   {
      case IGMP_HOST_MEMBERSHIP_QUERY:
#if defined(CONFIG_BR_IGMP_SNOOP)
      {
         bcm_mcast_lower_port* sourceLowerPort = NULL;
 
         spin_lock_bh(&pif->config_lock);
         sourceLowerPort = bcm_mcast_if_get_lower_port_by_ifindex(pif, skb_org->dev->ifindex);
         if (pif->igmp_snooping)
         {
            if (sourceLowerPort)
            {
               sourceLowerPort->querying_port = 1;
               mod_timer(&sourceLowerPort->querying_port_timer, jiffies + mcast_ctrl->igmp_general_query_timeout_secs*HZ);
            }
         }      
         spin_unlock_bh(&pif->config_lock);
      }
#endif
      case IGMP_HOST_MEMBERSHIP_REPORT:
      case IGMPV2_HOST_MEMBERSHIP_REPORT:
      case IGMPV3_HOST_MEMBERSHIP_REPORT:
      case IGMP_HOST_LEAVE_MESSAGE:
         break;

      default:
         /* unrecognized type */
         return 0;
   }

   rv = bcm_mcast_receive_igmp_filter(pif, pigmp, flags);
#if defined(CONFIG_BR_IGMP_SNOOP)
   if ( 0 == rv )
   {
      struct sk_buff       *skb2;
      t_BCM_MCAST_PKT_INFO *pinfo;
#if defined(BCM_MCAST_VALIDATE_PACKET)
      __wsum                csum;
#endif

      /* valdiate icmp checksum and then allocate an skb to send icmp data to netlink
         clients. */
         
      len = ntohs(pip->tot_len) - (pip->ihl * 4); 

#if defined(BCM_MCAST_VALIDATE_PACKET)
      /* validate igmp checksum */
      csum = csum_partial(pigmp, len, 0);
      if ( csum_fold(csum) )
      {
         rv = BCM_MCAST_ERR_DISCARD ? -EINVAL : 0;
         __logInfo("Invalid IGMP checksum");
      }
      else
#endif
      {
         skb2 = bcm_mcast_netlink_alloc_skb(len + sizeof(*pinfo), BCM_MCAST_PROTO_IPV4);
         if ( NULL != skb2 )
         {
            pinfo = (t_BCM_MCAST_PKT_INFO *)skb2->data;
            rv = bcm_mcast_fill_pkt_info(pif, 
                                         skb_org, 
                                         pip, 
                                         BCM_MCAST_PROTO_IPV4, 
                                         len, 
                                         pinfo, 
                                         flags);
            skb_copy_bits(skb_org, iph_offset + (pip->ihl * 4), &pinfo->pkt[0], len);
            bcm_mcast_netlink_send_skb(skb2, BCM_MCAST_MSG_IGMP_PKT);
         }
      }
   }
#endif
   return rv;
}

#if defined(CONFIG_BR_MLD_SNOOP)
static int bcm_mcast_receive_icmp_filter(bcm_mcast_ifdata *pif, struct icmp6hdr *picmp, int flags)
{
   return 0;
}

static int bcm_mcast_receive_icmp(bcm_mcast_ifdata *pif,
                                  struct sk_buff *skb_org,
                                  int iph_offset,
                                  int icmpv6_offset,
                                  int flags)
{
   struct ipv6hdr  *pipv6;
   struct icmp6hdr *picmpv6;
   int              len;
   int              rv;

   /* ensure we can read all icmp data */
   pipv6 = (struct ipv6hdr *)(skb_network_header(skb_org) + iph_offset);
   len = skb_network_offset(skb_org) + iph_offset + ntohs(pipv6->payload_len) + sizeof(*pipv6);
   if ( len > skb_headlen(skb_org) )
   {
      /* non-linear skb - stop processing */
       __logError("non-linear skb - stop processing");
      return 0;
   }

   if (ntohs(pipv6->payload_len) < (icmpv6_offset - sizeof(*pipv6) + sizeof(struct icmp6hdr)))
   {
       /* Error, IPV6 payload length < OPT header length + ICMP header length
          icmpv6_offset - sizeof(*pipv6) provides the OPT header length */
       __logError("IPV6 payload length < OPT header length + ICMP header length");
       return -1;
   }

   /* verify that we can read the ICMP header */
   picmpv6 = (struct icmp6hdr *)(skb_network_header(skb_org) + icmpv6_offset);

   __logDebug("Received MLD %s rxdev %s dstip %x...%x srcip %x...%x", 
              MLD_TYPE_STR(picmpv6->icmp6_type),
              skb_org->in_dev->name, 
              ntohs(pipv6->daddr.s6_addr16[0]), pipv6->daddr.s6_addr[15],
              ntohs(pipv6->saddr.s6_addr16[0]), pipv6->saddr.s6_addr[15]);

   switch (picmpv6->icmp6_type)
   {
      case ICMPV6_MGM_QUERY:
      {
         bcm_mcast_lower_port* sourceLowerPort = NULL;
 
         spin_lock_bh(&pif->config_lock);
         sourceLowerPort = bcm_mcast_if_get_lower_port_by_ifindex(pif, skb_org->dev->ifindex);
         if (pif->mld_snooping)
         {
            if (sourceLowerPort)
            {
               sourceLowerPort->mld_querying_port = 1;
               mod_timer(&sourceLowerPort->mld_querying_port_timer, jiffies + mcast_ctrl->mld_general_query_timeout_secs*HZ);
            }
         }      
         spin_unlock_bh(&pif->config_lock);
      } 
      case ICMPV6_MGM_REPORT:
      case ICMPV6_MGM_REDUCTION:
      case ICMPV6_MLD2_REPORT:
         break;

      default:
         return 0;
   }

   rv = bcm_mcast_receive_icmp_filter(pif, picmpv6, flags);
   if ( 0 == rv )
   {
      struct sk_buff       *skb2;
      t_BCM_MCAST_PKT_INFO *pinfo;
#if defined(BCM_MCAST_VALIDATE_PACKET)
      __wsum                csum;
#endif
      /* valdiate icmp checksum and then allocate an skb to send icmp data to netlink
         clients. */

      len = ntohs(pipv6->payload_len) + sizeof(*pipv6) - (icmpv6_offset - iph_offset);

#if defined(BCM_MCAST_VALIDATE_PACKET)
      /* validate icmp checksum */
      csum = csum_partial(picmpv6, len, 0);
      if ( csum_ipv6_magic(&pipv6->saddr, &pipv6->daddr, len, IPPROTO_ICMPV6, csum) )
      {
         __logInfo("Invalid ICMP checksum");
         rv = BCM_MCAST_ERR_DISCARD ? -EINVAL : 0;
      }
      else
#endif
      {
         skb2 = bcm_mcast_netlink_alloc_skb(len + sizeof(*pinfo), BCM_MCAST_PROTO_IPV6);
         if ( NULL != skb2 )
         {
            pinfo = (t_BCM_MCAST_PKT_INFO *)skb2->data;
            rv = bcm_mcast_fill_pkt_info(pif,
                                         skb_org,
                                         pipv6,
                                         BCM_MCAST_PROTO_IPV6,
                                         len,
                                         pinfo,
                                         flags);
            skb_copy_bits(skb_org, icmpv6_offset, &pinfo->pkt[0], len);
            bcm_mcast_netlink_send_skb(skb2, BCM_MCAST_MSG_MLD_PKT);
         }
      }
   }
   return rv;
}
#endif  /*CONFIG_BR_MLD_SNOOP*/

static int bcm_mcast_receive(int ifindex, struct sk_buff *skb, int is_routed)
{
   unsigned int      is_multicast;
   unsigned int      iph_offset = 0;
   unsigned short    proto = 0;
   bcm_mcast_ifdata *pif;
   int               flags = 0;
   int               rv = 0;
   int               len;

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(ifindex);
   if (NULL == pif)
   {
      rcu_read_unlock();
      __logNotice("Unable to find interface with index %d", ifindex);
      return rv;
   }

   is_multicast = bcm_mcast_skb_is_multicast(skb, &flags, &iph_offset, &proto);
   if ( is_multicast || (flags & BCM_MCAST_FLAG_PPP) )
   {
      is_multicast = bcm_mcast_skb_is_ip_multicast(skb, iph_offset, proto);
      if ( !is_multicast )
      {
         /* this may not be an IPvX packet or it is an IPvX with a non multicast IP - ignore */
         rcu_read_unlock();
         return rv;
      }

      if ( !skb->in_dev ) 
      {
          __logError("skb->in_dev NULL quit processing, skb->dev %s", skb->dev->name);
          return rv;
      }

      __logInfo("Mcast pkt received from indev %s ifindex %d protocol 0x%x\n", 
                skb->in_dev->name, skb->in_dev->ifindex, htons(proto));
      do
      {
         if (proto == htons(ETH_P_IP) )
         {
            const struct iphdr *pip;

            pip = (struct iphdr *)(skb_network_header(skb) + iph_offset);

            /* verify we can read the entire IPv4 header from linear data */
            len = skb_network_offset(skb) + iph_offset + (pip->ihl * 4);
            if ( len > skb_headlen(skb) )
            {
               /* non-linear skb - stop processing */
               break;
            }

#if defined(BCM_MCAST_VALIDATE_PACKET)
            /* validate IP header fields */
            if ((pip->ihl < 5) || (pip->version != 4))
            {
               /* ignore packet */
               __logInfo("Invalid IP version or header length");
               rv = (BCM_MCAST_ERR_DISCARD ? -EINVAL : 0);
               break;
            }

            if (ip_fast_csum((u8 *)pip, pip->ihl))
            {
               /* ignore packet */
               __logInfo("Invalid IP checksum");
               rv = (BCM_MCAST_ERR_DISCARD ? -EINVAL : 0);
               break;
            }

            len = ntohs(pip->tot_len);
            if ( len < (pip->ihl * 4) )
            {
               __logInfo("Invalid length");
               rv = (BCM_MCAST_ERR_DISCARD ? -EINVAL : 0);
               break;
            }

            len += skb_network_offset(skb) + iph_offset;
            if ( skb->len < len )
            {
               __logInfo("Invalid length");
               rv = (BCM_MCAST_ERR_DISCARD ? -EINVAL : 0);
               break;
            }
#endif

            if (pip->protocol == IPPROTO_IGMP)
            {
                rv = bcm_mcast_receive_igmp(pif, skb, iph_offset, flags);
            }
         }
#if defined(CONFIG_BR_MLD_SNOOP)
         if (proto == htons(ETH_P_IPV6) )
         {
            const struct ipv6hdr *pipv6;
            struct ipv6_opt_hdr  *opt_hdr;

            pipv6 = (struct ipv6hdr *)(skb_network_header(skb) + iph_offset);

            /* make sure we can read the ipv6 header */
            len = skb_network_offset(skb) + iph_offset + sizeof(*pipv6);
            if ( len > skb_headlen(skb) )
            {
               /* non-linear skb - stop processing */
               break;
            }

            if (pipv6->payload_len == 0)
            {
               /* ignore packet */
               __logDebug("Payload length zero");
               break;
            }

#if defined(BCM_MCAST_VALIDATE_PACKET)
            /* verify the IPv6 header */
            if (pipv6->version != 6)
            {
               __logInfo("Invalid IPv6 version");
               rv = (BCM_MCAST_ERR_DISCARD ? -EINVAL : 0);
               break;
            }

            /* verify the length of the packet */
            len += ntohs(pipv6->payload_len);
            if (skb->len < len)
            {
               __logInfo("Invalid length");
               rv = (BCM_MCAST_ERR_DISCARD ? -EINVAL : 0);
               break;
            }
#endif
            /* verify that we can read the HOPOPTS header */
            if (pipv6->nexthdr == IPPROTO_HOPOPTS)
            {
               opt_hdr = (struct ipv6_opt_hdr *)(skb_network_header(skb) + iph_offset + sizeof(*pipv6));
               len = skb_network_offset(skb) + iph_offset + sizeof(*pipv6) + ipv6_optlen(opt_hdr);
               if ( len > skb_headlen(skb) )
               {
                  /* non-linear skb - stop processing */
                  break;
               }

               if (ntohs(pipv6->payload_len) < ipv6_optlen(opt_hdr))
               {
                   /* Error, IPV6 payload length < OPT header length */
                   break;
               }

               if (opt_hdr->nexthdr == IPPROTO_ICMPV6)
               {
                  int icmpv6_offset = iph_offset + sizeof(*pipv6) + ipv6_optlen(opt_hdr);
                  rv = bcm_mcast_receive_icmp(pif,
                                              skb,
                                              iph_offset,
                                              icmpv6_offset,
                                              flags);
               }
            }
         }
#endif
      } while ( 0 );
   }
   rcu_read_unlock();
   return rv;
}



/* must be called with rcu lock */
int bcm_mcast_should_deliver(int ifindex, const struct sk_buff *skb, struct net_device *dst_dev, bool dst_mrouter)
{
   unsigned int      is_multicast;
   unsigned int      iph_offset = 0;
   unsigned short    proto = 0;
   bcm_mcast_ifdata *pif;
   int               flags = 0;
   int               rv = 1;

   rcu_read_lock();
   pif = bcm_mcast_if_lookup(ifindex);
   if (NULL == pif)
   {
      rcu_read_unlock();
      __logNotice("Unable to find interface with index %d", ifindex);
      return rv;
   }

   is_multicast = bcm_mcast_skb_is_multicast(skb, &flags, &iph_offset, &proto);
   if ( is_multicast || (flags & BCM_MCAST_FLAG_PPP) )
   {
      is_multicast = bcm_mcast_skb_is_ip_multicast(skb, iph_offset, proto);
      if ( !is_multicast )
      {
         /* this may not be an IPvX packet or it is an IPvX with a non multicast IP - ignore */
         rcu_read_unlock();
         return rv;
      }

      do
      {
#if defined(CONFIG_BR_IGMP_SNOOP)
         if (proto == htons(ETH_P_IP) )
         {
            const struct iphdr *pip;
            int                 len;
            pip = (struct iphdr *)(skb_network_header(skb) + iph_offset);

            /* verify we can read the entire IPv4 header from linear data */
            len = skb_network_offset(skb) + iph_offset + (pip->ihl * 4);
            if ( len > skb_headlen(skb) )
            {
               /* non-linear skb - stop processing */
               break;
            }

            /* no need to error check in should deliver */

            if (pip->protocol == IPPROTO_IGMP)
            {
               struct igmphdr *pigmp = (struct igmphdr *)(skb_network_header(skb) + iph_offset + (pip->ihl * 4));
               if ( dst_dev->priv_flags & IFF_WANDEV )
               {
                  /* do not forward queries to a WAN interface */
                  if (pigmp->type == IGMP_HOST_MEMBERSHIP_QUERY) 
                  {
                     __logDebug("discard IGMP query destined to WAN dev %s", dst_dev->name);
                     rv = 0;
                  }
               }
               else
               {
                  bcm_mcast_lower_port* destLowerPort = NULL;

                  spin_lock_bh(&pif->config_lock);
                  destLowerPort = bcm_mcast_if_get_lower_port_by_ifindex(pif, dst_dev->ifindex);
                  /* do not forward anything to LAN ports except queries and reports toward querying LAN ports*/
                  if ( pif->igmp_snooping && 
                      (pigmp->type != IGMP_HOST_MEMBERSHIP_QUERY) && 
                      (((NULL == destLowerPort) || (0 == destLowerPort->querying_port)) && !dst_mrouter) )
                  {
                     __logDebug("do not forward IGMP %s to dev %s dstip 0x%x", 
                                IGMP_TYPE_STR(pigmp->type), dst_dev->name, ntohl(pip->daddr));
                     rv = 0;
                  }
                  spin_unlock_bh(&pif->config_lock);
               }
            }
            else
            {
               rv = bcm_mcast_igmp_should_deliver(pif, pip, skb->dev, dst_dev, skb->pkt_type);
            }
            break;
         }
#endif      
#if defined(CONFIG_BR_MLD_SNOOP)
         if (proto == htons(ETH_P_IPV6) )
         {
            const struct ipv6hdr *pipv6;
            struct ipv6_opt_hdr  *opt_hdr;
            int                   len;

            pipv6 = (struct ipv6hdr *)(skb_network_header(skb) + iph_offset);

            /* make sure we can read the ipv6 header */
            len = skb_network_offset(skb) + iph_offset + sizeof(*pipv6);
            if ( len > skb_headlen(skb) )
            {
               /* non-linear skb - stop processing */
               break;
            }
            
            if (pipv6->payload_len == 0)
            {
               /* ignore packet */
               break;
            }      

            /* verify that we can read the HOPOPTS header */
            if (pipv6->nexthdr == IPPROTO_HOPOPTS)
            {
               opt_hdr = (struct ipv6_opt_hdr *)(skb_network_header(skb) + iph_offset + sizeof(*pipv6));
               len = skb_network_offset(skb) + iph_offset + sizeof(*pipv6) +  sizeof(*opt_hdr);
               if ( len > skb_headlen(skb) )
               {
                  /* non-linear skb - stop processing */
                  break;
               }
   
               if ( opt_hdr->nexthdr == IPPROTO_ICMPV6 )
               {
                  bcm_mcast_lower_port* destLowerPort = NULL;
                  char *opt_ptr = (char *)opt_hdr;
                  /* verify that we can read the ICMP header */
                  
                  struct icmp6hdr *picmp = (struct icmp6hdr *)(opt_ptr + ipv6_optlen(opt_hdr));
                  len = skb_network_offset(skb) + iph_offset + sizeof(*pipv6) +  ipv6_optlen(opt_hdr) + sizeof(*picmp);
                  if ( len > skb_headlen(skb) )
                  {
                     /* non-linear skb - stop processing */
                     break;
                  }
                  
                  if ( dst_dev->priv_flags & IFF_WANDEV )
                  {
                     /* do not forward queries to a WAN interface */
                     if (picmp->icmp6_type == ICMPV6_MGM_QUERY) 
                     {
                        __logDebug("discard MLD query destined to WAN dev %s", dst_dev->name);
                        rv = 0;
                     }
                  }
                  else 
                  {
                     spin_lock_bh(&pif->config_lock);
                     destLowerPort = bcm_mcast_if_get_lower_port_by_ifindex(pif, dst_dev->ifindex);
                     /* do not forward anything to LAN ports except queries and reports toward querying LAN ports*/
                     if ( pif->mld_snooping && (picmp->icmp6_type != ICMPV6_MGM_QUERY) && 
                         (((NULL == destLowerPort) || (0 == destLowerPort->mld_querying_port)) && !dst_mrouter) )
                     {
                        rv = 0;
                     }
                     spin_unlock_bh(&pif->config_lock);
                  }               
               }
               else
               {
                  rv = bcm_mcast_mld_should_deliver(pif, pipv6, skb->dev, dst_dev);
               }
            }
            else
            {
               rv = bcm_mcast_mld_should_deliver(pif, pipv6, skb->dev, dst_dev);
            }
            break;
         }
#endif
      } while (0 );
   }
   rcu_read_unlock();
   return rv;
}

void bcm_mcast_set_skb_mark_queue(struct sk_buff *skb)
{
   unsigned short proto;
   int            iph_offset;
   int            is_multicast;
   int            flags;

   if ( mcast_ctrl->mcastPriQueue == -1 )
   {
      return;
   }

   if (unlikely(skb->pkt_type == PACKET_LOOPBACK) || 
       unlikely(skb->pkt_type == PACKET_HOST))
   {
      return;
   }

   is_multicast = bcm_mcast_skb_is_multicast(skb, &flags, &iph_offset, &proto);
   if ( !is_multicast && (flags & BCM_MCAST_FLAG_PPP) )
   {
      is_multicast = bcm_mcast_skb_is_ip_multicast(skb, iph_offset, proto);
   }

   if ( is_multicast )
   {
      skb->mark = SKBMARK_SET_Q(skb->mark, mcast_ctrl->mcastPriQueue);
   }
}

void bcm_mcast_set_pri_queue(int queue)
{
   mcast_ctrl->mcastPriQueue = queue;
}

#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
int bcm_mcast_get_lan2lan_snooping(int proto, bcm_mcast_ifdata *pif)
{
   int rv = 0;
   if (!mcast_ctrl->thereIsAnUplink)
   {
      return BCM_MCAST_LAN2LAN_SNOOPING_ENABLE;
   }

   do
   {
#if defined(CONFIG_BR_IGMP_SNOOP)
      if (BCM_MCAST_PROTO_IPV4 == proto)
      {
         rv = pif->igmp_lan2lan_mc_enable;
         break;
      }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
      if ( BCM_MCAST_PROTO_IPV6 == proto )
      {
         rv = pif->mld_lan2lan_mc_enable;
         break;
      }
#endif
   } while(0);

   return rv;
}

void bcm_mcast_set_uplink(int uplinkExists)
{
   mcast_ctrl->thereIsAnUplink = uplinkExists;
}

void bcm_mcast_set_admission(int proto, int enable)
{
#if defined(CONFIG_BR_IGMP_SNOOP)
   if ( BCM_MCAST_PROTO_IPV4 == proto )
   {
      if (mcast_ctrl->igmp_admission != enable)
      {
         mcast_ctrl->igmp_admission = enable;
         bcm_mcast_if_admission_update_bydev(proto, NULL);
      }
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if ( BCM_MCAST_PROTO_IPV6 == proto )
   {
      if (mcast_ctrl->mld_admission != enable)
      {
         mcast_ctrl->mld_admission = enable;
         bcm_mcast_if_admission_update_bydev(proto, NULL);
      }
   }
#endif
}

#if defined(CONFIG_BLOG)
void bcm_mcast_process_blog_enable(int enable)
{
   if ( mcast_ctrl->blog_enable != enable )
   {
      mcast_ctrl->blog_enable = enable;
      bcm_mcast_if_process_blog_enable(enable);
   }
}
#endif
#endif

void bcm_mcast_deinit(void)
{
   br_bcm_mcast_bind(NULL, NULL);

#if defined(CONFIG_BLOG) && (defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP))
   bcm_mcast_blog_deinit();
#endif

   bcm_mcast_if_deinit();
   
   bcm_mcast_netlink_deinit();

   bcm_mcast_def_pri_queue_hook = NULL;

#if defined(CONFIG_BR_IGMP_SNOOP)
   if ( mcast_ctrl->ipv4_grp_cache )
   {
      kmem_cache_destroy(mcast_ctrl->ipv4_grp_cache);
   }
   if ( mcast_ctrl->ipv4_rep_cache )
   {
      kmem_cache_destroy(mcast_ctrl->ipv4_rep_cache);
   }
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   if ( mcast_ctrl->ipv6_grp_cache )
   {
      kmem_cache_destroy(mcast_ctrl->ipv6_grp_cache);
   }
   if ( mcast_ctrl->ipv6_rep_cache )
   {
      kmem_cache_destroy(mcast_ctrl->ipv6_rep_cache);
   }
#endif
   kfree(mcast_ctrl);

   return;
}

__init int bcm_mcast_init (void)
{
   int err;
   
   mcast_ctrl = kmalloc(sizeof(*mcast_ctrl), GFP_KERNEL);
   if ( NULL == mcast_ctrl)
   {
      return -ENOMEM;
   }
   memset(mcast_ctrl, 0, sizeof(*mcast_ctrl));
   INIT_HLIST_HEAD(&mcast_ctrl->if_hlist);
   RAW_INIT_NOTIFIER_HEAD(&mcast_ctrl->mcast_snooping_chain);
   mcast_ctrl->mcastPriQueue = -1;
#if defined(CONFIG_BLOG)
   mcast_ctrl->blog_enable = 1;
#endif
   
#if (defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP))
   mcast_ctrl->cfgLock = __SPIN_LOCK_UNLOCKED (mcast_ctrl.cfgLock);
#endif

#if defined(CONFIG_BR_IGMP_SNOOP)
   get_random_bytes(&mcast_ctrl->ipv4_hash_salt, sizeof(mcast_ctrl->ipv4_hash_salt));
   mcast_ctrl->ipv4_grp_cache = kmem_cache_create("bcm_mcast_igmp_grp_cache",
                                                  sizeof(t_igmp_grp_entry),
                                                  0,
                                                  SLAB_HWCACHE_ALIGN, NULL);
   if (NULL == mcast_ctrl->ipv4_grp_cache)
   {
      printk("bcm_mcast_init: failed to allocate v4 grp cache\n");
      bcm_mcast_deinit();
      return -ENOMEM;
   }

   mcast_ctrl->ipv4_rep_cache = kmem_cache_create("bcm_mcast_igmp_rep_cache",
                                                  sizeof(t_igmp_rep_entry),
                                                  0,
                                                  SLAB_HWCACHE_ALIGN, NULL);
   if (NULL == mcast_ctrl->ipv4_rep_cache)
   {
      printk("bcm_mcast_init: failed to allocate v4 rep cache\n");
      bcm_mcast_deinit();
      return -ENOMEM;
   }

   mcast_ctrl->ipv4_exception_cache = kmem_cache_create("bcm_mcast_igmp_exception_cache",
                                                  sizeof(struct bcm_mcast_control_filter_entry),
                                                  0,
                                                  SLAB_HWCACHE_ALIGN, NULL);

   if (NULL == mcast_ctrl->ipv4_exception_cache)
   {
      printk("bcm_mcast_init: failed to allocate v4 exception cache\n");
      bcm_mcast_deinit();
      return -ENOMEM;
   }

   INIT_HLIST_HEAD(&(mcast_ctrl->igmp_snoopExceptionList));
#endif   

#if defined(CONFIG_BR_MLD_SNOOP)
   get_random_bytes(&mcast_ctrl->ipv6_hash_salt, sizeof(mcast_ctrl->ipv6_hash_salt));
   mcast_ctrl->ipv6_grp_cache = kmem_cache_create("bcm_mcast_mld_grp_cache",
                                                  sizeof(t_mld_grp_entry),
                                                  0,
                                                  SLAB_HWCACHE_ALIGN, NULL);
   if (NULL == mcast_ctrl->ipv6_grp_cache)
   {
      printk("bcm_mcast_init: failed to allocate v6 grp cache\n");
      bcm_mcast_deinit();
      return -ENOMEM;
   }

   mcast_ctrl->ipv6_rep_cache = kmem_cache_create("bcm_mcast_mld_rep_cache",
                                                  sizeof(t_mld_rep_entry),
                                                  0,
                                                  SLAB_HWCACHE_ALIGN, NULL);
   if (NULL == mcast_ctrl->ipv6_rep_cache)
   {
      printk("bcm_mcast_init: failed to allocate v6 rep cache\n");
      bcm_mcast_deinit();
      return -ENOMEM;
   }

   mcast_ctrl->ipv6_exception_cache = kmem_cache_create("bcm_mcast_mld_exception_cache",
                                                        sizeof(struct bcm_mcast_control_filter_entry),
                                                        0,
                                                        SLAB_HWCACHE_ALIGN, NULL);
   if (NULL == mcast_ctrl->ipv6_exception_cache)
   {
      printk("failed to allocate v6 exception cache\n");
      bcm_mcast_deinit();
      return -ENOMEM;
   }

#endif
   bcm_mcast_def_pri_queue_hook = bcm_mcast_set_skb_mark_queue;

   err = bcm_mcast_netlink_init();
   if ( err )
   {
      printk("bcm_mcast_init: bcm_mcast_netlink_init error\n");
      bcm_mcast_deinit();
      return err;
   }

   err = bcm_mcast_if_init();
   if ( err )
   {
      printk("bcm_mcast_init: bcm_mcast_if_init error\n");
      bcm_mcast_deinit();
      return err;
   }

#if defined(CONFIG_BLOG) && (defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP))
   err = bcm_mcast_blog_init();
   if ( err )
   {
      printk("bcm_mcast_init: bcm_mcast_blog_init error\n");
      bcm_mcast_deinit();
      return err;
   }   
#endif

   br_bcm_mcast_bind(bcm_mcast_receive, bcm_mcast_should_deliver);

   bcmLog_setLogLevel(BCM_LOG_ID_MCAST, BCM_LOG_LEVEL_ERROR);

   return 0;
}

module_init(bcm_mcast_init);
module_exit(bcm_mcast_deinit);
MODULE_LICENSE("GPL");

