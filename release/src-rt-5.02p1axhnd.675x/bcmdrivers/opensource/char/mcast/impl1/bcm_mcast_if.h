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

#ifndef _BCM_MCAST_IF_H_
#define _BCM_MCAST_IF_H_

#define BCM_MCAST_HASH_BITS 8
#define BCM_MCAST_HASH_SIZE (1 << BCM_MCAST_HASH_BITS)

#define BCM_MCAST_MAX_DELAYED_SKB_COUNT   64
#define BCM_MCAST_NETLINK_SKB_TIMEOUT_MS  2000


typedef struct {
    struct sk_buff *skb;
    unsigned long   expiryTime;
} t_BCM_MCAST_DELAYED_SKB;

typedef struct
{
  struct hlist_node       hlist;
  unsigned char           port_ifindex;
#if defined(CONFIG_BR_IGMP_SNOOP)
  unsigned char           querying_port;
  struct timer_list       querying_port_timer;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
  unsigned char           mld_querying_port;
  struct timer_list       mld_querying_port_timer;
#endif
} bcm_mcast_lower_port;

typedef enum
{
    BRTYPE_MIN,
    BRTYPE_LINUX = BRTYPE_MIN,
    BRTYPE_OVS,
    BRTYPE_MAX
}enum_bcm_mcast_brtype;

typedef struct 
{
   struct hlist_node       hlist;
   struct rcu_head         rcu;
   int                     ifindex;
   struct net_device      *dev;
   spinlock_t              config_lock;
   struct hlist_head       lower_port_list;

#if defined(CONFIG_BR_IGMP_SNOOP)
   struct timer_list       igmp_timer;
   int                     igmp_snooping;
   spinlock_t              mc_igmp_lock;
   struct hlist_head       mc_ipv4_hash[BCM_MCAST_HASH_SIZE];
   int                     igmp_lan2lan_mc_enable;
   t_BCM_MCAST_DELAYED_SKB igmp_delayed_skb[BCM_MCAST_MAX_DELAYED_SKB_COUNT];
   /* for igmp packet rate limit */
   unsigned int            igmp_rate_limit;
   unsigned int            igmp_rate_bucket;
   ktime_t                 igmp_rate_last_packet;
   unsigned int            igmp_rate_rem_time;
   u64                     diffTotal;
   unsigned int            pckInSec;

#endif

#if defined(CONFIG_BR_MLD_SNOOP)
   struct timer_list       mld_timer;
   int                     mld_snooping;
   spinlock_t              mc_mld_lock;
   struct hlist_head       mc_ipv6_hash[BCM_MCAST_HASH_SIZE];
   int                     mld_lan2lan_mc_enable;
   t_BCM_MCAST_DELAYED_SKB mld_delayed_skb[BCM_MCAST_MAX_DELAYED_SKB_COUNT];
#endif
   enum_bcm_mcast_brtype   brtype;
} bcm_mcast_ifdata;

bcm_mcast_lower_port* bcm_mcast_if_get_lower_port_by_ifindex(bcm_mcast_ifdata *pif, int ifindex);
int bcm_mcast_if_admission_process(int proto, struct net_device * dev, int packet_index, int admitted);
void bcm_mcast_if_admission_update_bydev(int proto, struct net_device *dev);
void bcm_mcast_if_update_bydev(int proto, struct net_device *dev, int activate);
int bcm_mcast_if_is_snooping_enabled(struct net_device *dev, int proto);
int bcm_mcast_if_is_associated_dev(struct net_device *dev, int parentifi);

bcm_mcast_ifdata *bcm_mcast_if_lookup(int ifindex);
void bcm_mcast_if_process_device_change(struct net_device *dev);

void bcm_mcast_process_ovs_brinfo_update(t_ovs_mcpd_brcfg_info *pBrcfg);
#if defined(CONFIG_BLOG)
void bcm_mcast_if_process_blog_enable( int enable );
#if defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE)
int bcm_mcast_if_wlan_client_disconnect(struct net_device *dev, char *mac);
#endif
#endif

__init int bcm_mcast_if_init( void );
void bcm_mcast_if_deinit( void );

#endif /* _BCM_MCAST_IF_H_ */
