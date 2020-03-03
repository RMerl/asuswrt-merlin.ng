/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/netpoll.h>
#include <linux/u64_stats_sync.h>
#include <net/route.h>
#include <linux/if_vlan.h>

#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
#include "br_fp.h"
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */


#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
extern br_bcm_mcast_receive_hook br_bcm_mcast_receive;
extern br_bcm_mcast_should_deliver_hook br_bcm_mcast_should_deliver;
#endif

#define BR_HOLD_TIME (1*HZ)

#define BR_PORT_BITS	10
#define BR_MAX_PORTS	(1<<BR_PORT_BITS)
#define BR_VLAN_BITMAP_LEN	BITS_TO_LONGS(VLAN_N_VID)

#define BR_VERSION	"2.3"

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT)
#define BR_MAX_FDB_ENTRIES 4096
#endif

/* Control of forwarding link local multicast */
#define BR_GROUPFWD_DEFAULT	0
/* Don't allow forwarding control protocols like STP and LLDP */
#define BR_GROUPFWD_RESTRICTED	0x4007u
/* The Nearest Customer Bridge Group Address, 01-80-C2-00-00-[00,0B,0C,0D,0F] */
#define BR_GROUPFWD_8021AD	0xB801u

/* Path to usermode spanning tree program */
#define BR_STP_PROG	"/sbin/bridge-stp"

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

struct bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[ETH_ALEN];
};

struct mac_addr
{
	unsigned char	addr[ETH_ALEN];
};

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
/* our own querier */
struct bridge_mcast_own_query {
	struct timer_list	timer;
	u32			startup_sent;
};

/* other querier */
struct bridge_mcast_other_query {
	struct timer_list		timer;
	unsigned long			delay_time;
};

/* selected querier */
struct bridge_mcast_querier {
	struct br_ip addr;
	struct net_bridge_port __rcu	*port;
};
#endif

struct net_port_vlans {
	u16				port_idx;
	u16				pvid;
	union {
		struct net_bridge_port		*port;
		struct net_bridge		*br;
	}				parent;
	struct rcu_head			rcu;
	unsigned long			vlan_bitmap[BR_VLAN_BITMAP_LEN];
	unsigned long			untagged_bitmap[BR_VLAN_BITMAP_LEN];
	u16				num_vlans;
};

struct net_bridge_fdb_entry
{
	struct hlist_node		hlist;
	struct net_bridge_port		*dst;

	struct rcu_head			rcu;
	unsigned long			updated;
	unsigned long			used;
	mac_addr			addr;
	unsigned char			is_local:1,
					is_static:1,
					added_by_user:1,
					added_by_external_learn:1;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	unsigned int			fdb_key;
#endif
	__u16				vlan_id;
};
struct net_bridge_port_group {
	struct net_bridge_port		*port;
	struct net_bridge_port_group __rcu *next;
	struct hlist_node		mglist;
	struct rcu_head			rcu;
	struct timer_list		timer;
	struct br_ip			addr;
	unsigned char			state;
};

struct net_bridge_mdb_entry
{
	struct hlist_node		hlist[2];
	struct net_bridge		*br;
	struct net_bridge_port_group __rcu *ports;
	struct rcu_head			rcu;
	struct timer_list		timer;
	struct br_ip			addr;
	bool				mglist;
};

struct net_bridge_mdb_htable
{
	struct hlist_head		*mhash;
	struct rcu_head			rcu;
	struct net_bridge_mdb_htable	*old;
	u32				size;
	u32				max;
	u32				secret;
	u32				ver;
};

struct net_bridge_port
{
	struct net_bridge		*br;
	struct net_device		*dev;
	struct list_head		list;

	/* STP */
	u8				priority;
	u8				state;
	u16				port_no;
	unsigned char			topology_change_ack;
	unsigned char			config_pending;
	port_id				port_id;
	port_id				designated_port;
	bridge_id			designated_root;
	bridge_id			designated_bridge;
	u32				path_cost;
	u32				designated_cost;
	unsigned long			designated_age;
	struct timer_list		forward_delay_timer;
	struct timer_list		hold_timer;
	struct timer_list		message_age_timer;
	struct kobject			kobj;
	struct rcu_head			rcu;

	unsigned long 			flags;

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
	struct bridge_mcast_own_query	ip4_own_query;
#if IS_ENABLED(CONFIG_IPV6)
	struct bridge_mcast_own_query	ip6_own_query;
#endif /* IS_ENABLED(CONFIG_IPV6) */
	unsigned char			multicast_router;
	struct timer_list		multicast_router_timer;
	struct hlist_head		mglist;
	struct hlist_node		rlist;
#endif

#ifdef CONFIG_SYSFS
	char				sysfs_name[IFNAMSIZ];
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
	struct netpoll			*np;
#endif
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	struct net_port_vlans __rcu	*vlan_info;
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
	int                     num_port_fdb_entries;
	int                     max_port_fdb_entries;
	int                     min_port_fdb_entries;
#endif
};

#define br_auto_port(p) ((p)->flags & BR_AUTO_MASK)
#define br_promisc_port(p) ((p)->flags & BR_PROMISC)

#define br_port_exists(dev) (dev->priv_flags & IFF_BRIDGE_PORT)

static inline struct net_bridge_port *br_port_get_rcu(const struct net_device *dev)
{
	return rcu_dereference(dev->rx_handler_data);
}

static inline struct net_bridge_port *br_port_get_rtnl(const struct net_device *dev)
{
	return br_port_exists(dev) ?
		rtnl_dereference(dev->rx_handler_data) : NULL;
}

struct net_bridge
{
	spinlock_t			lock;
	struct list_head		port_list;
	struct net_device		*dev;

	struct pcpu_sw_netstats		__percpu *stats;
#if defined(CONFIG_BCM_KF_BRIDGE_COUNTERS)
	u32 mac_entry_discard_counter;
#endif
	spinlock_t			hash_lock;
	struct hlist_head		hash[BR_HASH_SIZE];
#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	struct rtable 			fake_rtable;
	bool				nf_call_iptables;
	bool				nf_call_ip6tables;
	bool				nf_call_arptables;
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) 
	int                     num_fdb_entries;
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
	int                     max_br_fdb_entries;
	int                     used_br_fdb_entries;
#endif

	u16				group_fwd_mask;
	u16				group_fwd_mask_required;

	/* STP */
	bridge_id			designated_root;
	bridge_id			bridge_id;
	u32				root_path_cost;
	unsigned long			max_age;
	unsigned long			hello_time;
	unsigned long			forward_delay;
	unsigned long			bridge_max_age;
	unsigned long			ageing_time;
	unsigned long			bridge_hello_time;
	unsigned long			bridge_forward_delay;

	u8				group_addr[ETH_ALEN];
	bool				group_addr_set;
	u16				root_port;

	enum {
		BR_NO_STP, 		/* no spanning tree */
		BR_KERNEL_STP,		/* old STP in kernel */
		BR_USER_STP,		/* new RSTP in userspace */
	} stp_enabled;

	unsigned char			topology_change;
	unsigned char			topology_change_detected;

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
	unsigned char			multicast_router;

	u8				multicast_disabled:1;
	u8				multicast_querier:1;
	u8				multicast_query_use_ifaddr:1;

	u32				hash_elasticity;
	u32				hash_max;

	u32				multicast_last_member_count;
	u32				multicast_startup_query_count;

	unsigned long			multicast_last_member_interval;
	unsigned long			multicast_membership_interval;
	unsigned long			multicast_querier_interval;
	unsigned long			multicast_query_interval;
	unsigned long			multicast_query_response_interval;
	unsigned long			multicast_startup_query_interval;

	spinlock_t			multicast_lock;
	struct net_bridge_mdb_htable __rcu *mdb;
	struct hlist_head		router_list;

	struct timer_list		multicast_router_timer;
	struct bridge_mcast_other_query	ip4_other_query;
	struct bridge_mcast_own_query	ip4_own_query;
	struct bridge_mcast_querier	ip4_querier;
#if IS_ENABLED(CONFIG_IPV6)
	struct bridge_mcast_other_query	ip6_other_query;
	struct bridge_mcast_own_query	ip6_own_query;
	struct bridge_mcast_querier	ip6_querier;
#endif /* IS_ENABLED(CONFIG_IPV6) */
#endif

	struct timer_list		hello_timer;
	struct timer_list		tcn_timer;
	struct timer_list		topology_change_timer;
	struct timer_list		gc_timer;
	struct kobject			*ifobj;
	u32				auto_cnt;
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	u8				vlan_enabled;
	__be16				vlan_proto;
	u16				default_pvid;
	struct net_port_vlans __rcu	*vlan_info;
#endif

#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
	struct br_fp_data		fp_hooks;
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */
#if defined(CONFIG_BCM_KF_INTF_BRG) && defined(CONFIG_BCM_INTF_BRG_ENABLED)
	u16				lan_port_num;
	u16				wan_port_num;
#endif /* CONFIG_BCM_KF_INTF_BRG && CONFIG_BCM_INTF_BRG_ENABLED */
#if defined(CONFIG_BCM_KF_LOCAL_SWITCHING_DISABLE)
	int local_switching_disable;
#endif
};

struct br_input_skb_cb {
	struct net_device *brdev;

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
	int igmp;
	int mrouters_only;
#endif

	u16 frag_max_size;
	bool proxyarp_replied;

#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	bool vlan_filtered;
#endif
};

#define BR_INPUT_SKB_CB(__skb)	((struct br_input_skb_cb *)(__skb)->cb)

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
# define BR_INPUT_SKB_CB_MROUTERS_ONLY(__skb)	(BR_INPUT_SKB_CB(__skb)->mrouters_only)
#else
# define BR_INPUT_SKB_CB_MROUTERS_ONLY(__skb)	(0)
#endif

#define br_printk(level, br, format, args...)	\
	printk(level "%s: " format, (br)->dev->name, ##args)

#define br_err(__br, format, args...)			\
	br_printk(KERN_ERR, __br, format, ##args)
#define br_warn(__br, format, args...)			\
	br_printk(KERN_WARNING, __br, format, ##args)
#define br_notice(__br, format, args...)		\
	br_printk(KERN_NOTICE, __br, format, ##args)
#define br_info(__br, format, args...)			\
	br_printk(KERN_INFO, __br, format, ##args)

#define br_debug(br, format, args...)			\
	pr_debug("%s: " format,  (br)->dev->name, ##args)

/* called under bridge lock */
static inline int br_is_root_bridge(const struct net_bridge *br)
{
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}

/* br_device.c */
void br_dev_setup(struct net_device *dev);
void br_dev_delete(struct net_device *dev, struct list_head *list);
netdev_tx_t br_dev_xmit(struct sk_buff *skb, struct net_device *dev);
#ifdef CONFIG_NET_POLL_CONTROLLER
static inline void br_netpoll_send_skb(const struct net_bridge_port *p,
				       struct sk_buff *skb)
{
	struct netpoll *np = p->np;

	if (np)
		netpoll_send_skb(np, skb);
}

int br_netpoll_enable(struct net_bridge_port *p);
void br_netpoll_disable(struct net_bridge_port *p);
#else
static inline void br_netpoll_send_skb(const struct net_bridge_port *p,
				       struct sk_buff *skb)
{
}

static inline int br_netpoll_enable(struct net_bridge_port *p)
{
	return 0;
}

static inline void br_netpoll_disable(struct net_bridge_port *p)
{
}
#endif

/* br_fdb.c */
int br_fdb_init(void);
void br_fdb_fini(void);
void br_fdb_flush(struct net_bridge *br);
void br_fdb_find_delete_local(struct net_bridge *br,
			      const struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid);
void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr);
void br_fdb_change_mac_address(struct net_bridge *br, const u8 *newaddr);
void br_fdb_cleanup(unsigned long arg);
void br_fdb_delete_by_port(struct net_bridge *br,
			   const struct net_bridge_port *p, int do_all);
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr, __u16 vid);
int br_fdb_test_addr(struct net_device *dev, unsigned char *addr);
int br_fdb_fillbuf(struct net_bridge *br, void *buf, unsigned long count,
		   unsigned long off);
int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, u16 vid);
void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr, u16 vid, bool added_by_user);

int br_fdb_delete(struct ndmsg *ndm, struct nlattr *tb[],
		  struct net_device *dev, const unsigned char *addr, u16 vid);
int br_fdb_add(struct ndmsg *nlh, struct nlattr *tb[], struct net_device *dev,
	       const unsigned char *addr, u16 vid, u16 nlh_flags);
int br_fdb_dump(struct sk_buff *skb, struct netlink_callback *cb,
		struct net_device *dev, struct net_device *fdev, int idx);
int br_fdb_sync_static(struct net_bridge *br, struct net_bridge_port *p);
void br_fdb_unsync_static(struct net_bridge *br, struct net_bridge_port *p);
int br_fdb_external_learn_add(struct net_bridge *br, struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid);
int br_fdb_external_learn_del(struct net_bridge *br, struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid);

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
int br_set_fdb_limit(struct net_bridge *br, 
						struct net_bridge_port *p,
						int lmt_type,
						int is_min,
						int fdb_limit);
#endif
		
/* br_forward.c */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb);
int br_dev_queue_push_xmit(struct sock *sk, struct sk_buff *skb);
void br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb, struct sk_buff *skb0);
int br_forward_finish(struct sock *sk, struct sk_buff *skb);
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, bool unicast);
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb,
		      struct sk_buff *skb2, bool unicast);

/* br_if.c */
#if defined(CONFIG_BCM_KF_INTF_BRG) && defined(CONFIG_BCM_INTF_BRG_ENABLED)
static inline void br_port_num_inc(struct net_bridge *br, struct net_bridge_port *p)
{
#if defined(CONFIG_BCM_KF_WANDEV)
    if (p->dev->priv_flags & IFF_WANDEV)
    {
        br->wan_port_num++;
    }
    else
#endif
    {
        br->lan_port_num++;
    }
}

static inline void br_port_num_dec(struct net_bridge *br, struct net_bridge_port *p)
{
#if defined(CONFIG_BCM_KF_WANDEV)
    if (p->dev->priv_flags & IFF_WANDEV)
    {
        br->wan_port_num--;
    }
    else
#endif
    {
        br->lan_port_num--;
    }
}

static inline bool is_interface_br(struct net_bridge *br, struct sk_buff *skb)
{
    struct net_bridge_port *pin;

    if (!skb->dev)
    {
        return 0;
    }
 
    pin = br_port_get_rcu(skb->dev);
    if ((!pin) || (!pin->dev))
    {
        return 0;
    }

    return ((br->lan_port_num == 1) && (br->wan_port_num == 1));
}

#endif /* CONFIG_BCM_KF_INTF_BRG && CONFIG_BCM_INTF_BRG_ENABLED */

void br_port_carrier_check(struct net_bridge_port *p);
int br_add_bridge(struct net *net, const char *name);
int br_del_bridge(struct net *net, const char *name);
int br_add_if(struct net_bridge *br, struct net_device *dev);
int br_del_if(struct net_bridge *br, struct net_device *dev);
int br_min_mtu(const struct net_bridge *br);
netdev_features_t br_features_recompute(struct net_bridge *br,
					netdev_features_t features);
void br_port_flags_change(struct net_bridge_port *port, unsigned long mask);
void br_manage_promisc(struct net_bridge *br);

/* br_input.c */
int br_handle_frame_finish(struct sock *sk, struct sk_buff *skb);
rx_handler_result_t br_handle_frame(struct sk_buff **pskb);

static inline bool br_rx_handler_check_rcu(const struct net_device *dev)
{
	return rcu_dereference(dev->rx_handler) == br_handle_frame;
}

static inline struct net_bridge_port *br_port_get_check_rcu(const struct net_device *dev)
{
	return br_rx_handler_check_rcu(dev) ? br_port_get_rcu(dev) : NULL;
}

/* br_ioctl.c */
int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
int br_ioctl_deviceless_stub(struct net *net, unsigned int cmd,
			     void __user *arg);

/* br_multicast.c */
#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
extern unsigned int br_mdb_rehash_seq;
int br_multicast_rcv(struct net_bridge *br, struct net_bridge_port *port,
		     struct sk_buff *skb, u16 vid);
struct net_bridge_mdb_entry *br_mdb_get(struct net_bridge *br,
					struct sk_buff *skb, u16 vid);
void br_multicast_add_port(struct net_bridge_port *port);
void br_multicast_del_port(struct net_bridge_port *port);
void br_multicast_enable_port(struct net_bridge_port *port);
void br_multicast_disable_port(struct net_bridge_port *port);
void br_multicast_init(struct net_bridge *br);
void br_multicast_open(struct net_bridge *br);
void br_multicast_stop(struct net_bridge *br);
void br_multicast_deliver(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb);
void br_multicast_forward(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb, struct sk_buff *skb2);
int br_multicast_set_router(struct net_bridge *br, unsigned long val);
int br_multicast_set_port_router(struct net_bridge_port *p, unsigned long val);
int br_multicast_toggle(struct net_bridge *br, unsigned long val);
int br_multicast_set_querier(struct net_bridge *br, unsigned long val);
int br_multicast_set_hash_max(struct net_bridge *br, unsigned long val);
struct net_bridge_mdb_entry *
br_mdb_ip_get(struct net_bridge_mdb_htable *mdb, struct br_ip *dst);
struct net_bridge_mdb_entry *
br_multicast_new_group(struct net_bridge *br, struct net_bridge_port *port,
		       struct br_ip *group);
void br_multicast_free_pg(struct rcu_head *head);
struct net_bridge_port_group *
br_multicast_new_port_group(struct net_bridge_port *port, struct br_ip *group,
			    struct net_bridge_port_group __rcu *next,
			    unsigned char state);
void br_mdb_init(void);
void br_mdb_uninit(void);
void br_mdb_notify(struct net_device *dev, struct net_bridge_port *port,
		   struct br_ip *group, int type);

#define mlock_dereference(X, br) \
	rcu_dereference_protected(X, lockdep_is_held(&br->multicast_lock))

static inline bool br_multicast_is_router(struct net_bridge *br)
{
	return br->multicast_router == 2 ||
	       (br->multicast_router == 1 &&
		timer_pending(&br->multicast_router_timer));
}

static inline bool
__br_multicast_querier_exists(struct net_bridge *br,
			      struct bridge_mcast_other_query *querier)
{
	return time_is_before_jiffies(querier->delay_time) &&
	       (br->multicast_querier || timer_pending(&querier->timer));
}

static inline bool br_multicast_querier_exists(struct net_bridge *br,
					       struct ethhdr *eth)
{
	switch (eth->h_proto) {
	case (htons(ETH_P_IP)):
		return __br_multicast_querier_exists(br, &br->ip4_other_query);
#if IS_ENABLED(CONFIG_IPV6)
	case (htons(ETH_P_IPV6)):
		return __br_multicast_querier_exists(br, &br->ip6_other_query);
#endif
	default:
		return false;
	}
}
#else
static inline int br_multicast_rcv(struct net_bridge *br,
				   struct net_bridge_port *port,
				   struct sk_buff *skb,
				   u16 vid)
{
	return 0;
}

static inline struct net_bridge_mdb_entry *br_mdb_get(struct net_bridge *br,
						      struct sk_buff *skb, u16 vid)
{
	return NULL;
}

static inline void br_multicast_add_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_del_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_enable_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_disable_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_init(struct net_bridge *br)
{
}

static inline void br_multicast_open(struct net_bridge *br)
{
}

static inline void br_multicast_stop(struct net_bridge *br)
{
}

static inline void br_multicast_deliver(struct net_bridge_mdb_entry *mdst,
					struct sk_buff *skb)
{
}

static inline void br_multicast_forward(struct net_bridge_mdb_entry *mdst,
					struct sk_buff *skb,
					struct sk_buff *skb2)
{
}
static inline bool br_multicast_is_router(struct net_bridge *br)
{
	return 0;
}
static inline bool br_multicast_querier_exists(struct net_bridge *br,
					       struct ethhdr *eth)
{
	return false;
}
static inline void br_mdb_init(void)
{
}
static inline void br_mdb_uninit(void)
{
}
#endif

/* br_vlan.c */
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
bool br_allowed_ingress(struct net_bridge *br, struct net_port_vlans *v,
			struct sk_buff *skb, u16 *vid);
bool br_allowed_egress(struct net_bridge *br, const struct net_port_vlans *v,
		       const struct sk_buff *skb);
bool br_should_learn(struct net_bridge_port *p, struct sk_buff *skb, u16 *vid);
struct sk_buff *br_handle_vlan(struct net_bridge *br,
			       const struct net_port_vlans *v,
			       struct sk_buff *skb);
int br_vlan_add(struct net_bridge *br, u16 vid, u16 flags);
int br_vlan_delete(struct net_bridge *br, u16 vid);
void br_vlan_flush(struct net_bridge *br);
bool br_vlan_find(struct net_bridge *br, u16 vid);
void br_recalculate_fwd_mask(struct net_bridge *br);
int br_vlan_filter_toggle(struct net_bridge *br, unsigned long val);
int br_vlan_set_proto(struct net_bridge *br, unsigned long val);
int br_vlan_init(struct net_bridge *br);
int br_vlan_set_default_pvid(struct net_bridge *br, unsigned long val);
int nbp_vlan_add(struct net_bridge_port *port, u16 vid, u16 flags);
int nbp_vlan_delete(struct net_bridge_port *port, u16 vid);
void nbp_vlan_flush(struct net_bridge_port *port);
bool nbp_vlan_find(struct net_bridge_port *port, u16 vid);
int nbp_vlan_init(struct net_bridge_port *port);

static inline struct net_port_vlans *br_get_vlan_info(
						const struct net_bridge *br)
{
	return rcu_dereference_rtnl(br->vlan_info);
}

static inline struct net_port_vlans *nbp_get_vlan_info(
						const struct net_bridge_port *p)
{
	return rcu_dereference_rtnl(p->vlan_info);
}

/* Since bridge now depends on 8021Q module, but the time bridge sees the
 * skb, the vlan tag will always be present if the frame was tagged.
 */
static inline int br_vlan_get_tag(const struct sk_buff *skb, u16 *vid)
{
	int err = 0;

	if (skb_vlan_tag_present(skb))
		*vid = skb_vlan_tag_get(skb) & VLAN_VID_MASK;
	else {
		*vid = 0;
		err = -EINVAL;
	}

	return err;
}

static inline u16 br_get_pvid(const struct net_port_vlans *v)
{
	if (!v)
		return 0;

	smp_rmb();
	return v->pvid;
}

static inline int br_vlan_enabled(struct net_bridge *br)
{
	return br->vlan_enabled;
}
#else
static inline bool br_allowed_ingress(struct net_bridge *br,
				      struct net_port_vlans *v,
				      struct sk_buff *skb,
				      u16 *vid)
{
	return true;
}

static inline bool br_allowed_egress(struct net_bridge *br,
				     const struct net_port_vlans *v,
				     const struct sk_buff *skb)
{
	return true;
}

static inline bool br_should_learn(struct net_bridge_port *p,
				   struct sk_buff *skb, u16 *vid)
{
	return true;
}

static inline struct sk_buff *br_handle_vlan(struct net_bridge *br,
					     const struct net_port_vlans *v,
					     struct sk_buff *skb)
{
	return skb;
}

static inline int br_vlan_add(struct net_bridge *br, u16 vid, u16 flags)
{
	return -EOPNOTSUPP;
}

static inline int br_vlan_delete(struct net_bridge *br, u16 vid)
{
	return -EOPNOTSUPP;
}

static inline void br_vlan_flush(struct net_bridge *br)
{
}

static inline bool br_vlan_find(struct net_bridge *br, u16 vid)
{
	return false;
}

static inline void br_recalculate_fwd_mask(struct net_bridge *br)
{
}

static inline int br_vlan_init(struct net_bridge *br)
{
	return 0;
}

static inline int nbp_vlan_add(struct net_bridge_port *port, u16 vid, u16 flags)
{
	return -EOPNOTSUPP;
}

static inline int nbp_vlan_delete(struct net_bridge_port *port, u16 vid)
{
	return -EOPNOTSUPP;
}

static inline void nbp_vlan_flush(struct net_bridge_port *port)
{
}

static inline struct net_port_vlans *br_get_vlan_info(
						const struct net_bridge *br)
{
	return NULL;
}
static inline struct net_port_vlans *nbp_get_vlan_info(
						const struct net_bridge_port *p)
{
	return NULL;
}

static inline bool nbp_vlan_find(struct net_bridge_port *port, u16 vid)
{
	return false;
}

static inline int nbp_vlan_init(struct net_bridge_port *port)
{
	return 0;
}

static inline u16 br_vlan_get_tag(const struct sk_buff *skb, u16 *tag)
{
	return 0;
}
static inline u16 br_get_pvid(const struct net_port_vlans *v)
{
	return 0;
}

static inline int br_vlan_enabled(struct net_bridge *br)
{
	return 0;
}
#endif

struct nf_br_ops {
	int (*br_dev_xmit_hook)(struct sk_buff *skb);
};
extern const struct nf_br_ops __rcu *nf_br_ops;

/* br_netfilter.c */
#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
int br_nf_core_init(void);
void br_nf_core_fini(void);
void br_netfilter_rtable_init(struct net_bridge *);
#else
static inline int br_nf_core_init(void) { return 0; }
static inline void br_nf_core_fini(void) {}
#define br_netfilter_rtable_init(x)
#endif

/* br_stp.c */
void br_log_state(const struct net_bridge_port *p);
void br_set_state(struct net_bridge_port *p, unsigned int state);
struct net_bridge_port *br_get_port(struct net_bridge *br, u16 port_no);
void br_init_port(struct net_bridge_port *p);
void br_become_designated_port(struct net_bridge_port *p);

void __br_set_forward_delay(struct net_bridge *br, unsigned long t);
int br_set_forward_delay(struct net_bridge *br, unsigned long x);
int br_set_hello_time(struct net_bridge *br, unsigned long x);
int br_set_max_age(struct net_bridge *br, unsigned long x);


/* br_stp_if.c */
void br_stp_enable_bridge(struct net_bridge *br);
void br_stp_disable_bridge(struct net_bridge *br);
void br_stp_set_enabled(struct net_bridge *br, unsigned long val);
void br_stp_enable_port(struct net_bridge_port *p);
void br_stp_disable_port(struct net_bridge_port *p);
bool br_stp_recalculate_bridge_id(struct net_bridge *br);
void br_stp_change_bridge_id(struct net_bridge *br, const unsigned char *a);
void br_stp_set_bridge_priority(struct net_bridge *br, u16 newprio);
int br_stp_set_port_priority(struct net_bridge_port *p, unsigned long newprio);
int br_stp_set_path_cost(struct net_bridge_port *p, unsigned long path_cost);
ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id);

/* br_stp_bpdu.c */
struct stp_proto;
void br_stp_rcv(const struct stp_proto *proto, struct sk_buff *skb,
		struct net_device *dev);

/* br_stp_timer.c */
void br_stp_timer_init(struct net_bridge *br);
void br_stp_port_timer_init(struct net_bridge_port *p);
unsigned long br_timer_value(const struct timer_list *timer);

/* br.c */
#if IS_ENABLED(CONFIG_ATM_LANE)
extern int (*br_fdb_test_addr_hook)(struct net_device *dev, unsigned char *addr);
#endif

/* br_netlink.c */
extern struct rtnl_link_ops br_link_ops;
int br_netlink_init(void);
void br_netlink_fini(void);
void br_ifinfo_notify(int event, struct net_bridge_port *port);
int br_setlink(struct net_device *dev, struct nlmsghdr *nlmsg, u16 flags);
int br_dellink(struct net_device *dev, struct nlmsghdr *nlmsg, u16 flags);
int br_getlink(struct sk_buff *skb, u32 pid, u32 seq, struct net_device *dev,
	       u32 filter_mask, int nlflags);

#ifdef CONFIG_SYSFS
/* br_sysfs_if.c */
extern const struct sysfs_ops brport_sysfs_ops;
int br_sysfs_addif(struct net_bridge_port *p);
int br_sysfs_renameif(struct net_bridge_port *p);

/* br_sysfs_br.c */
int br_sysfs_addbr(struct net_device *dev);
void br_sysfs_delbr(struct net_device *dev);

#else

static inline int br_sysfs_addif(struct net_bridge_port *p) { return 0; }
static inline int br_sysfs_renameif(struct net_bridge_port *p) { return 0; }
static inline int br_sysfs_addbr(struct net_device *dev) { return 0; }
static inline void br_sysfs_delbr(struct net_device *dev) { return; }
#endif /* CONFIG_SYSFS */

#if defined(CONFIG_BCM_KF_BRIDGE_STP)
/* br_notifier.c */
extern void br_stp_notify_state_port(const struct net_bridge_port *p);
extern void br_stp_notify_state_bridge(const struct net_bridge *br);
#endif
#if defined(CONFIG_BCM_KF_BRIDGE_PORT_ISOLATION)
struct bridge_notifier_info {
    struct net_device *br_dev;
	struct net_device *dev;
    int isadd;
};
extern void br_dev_notify_if_change(struct net_device *br_dev, struct net_device *dev, int isadd);
#endif

#endif
