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
#ifndef _LINUX_IF_BRIDGE_H
#define _LINUX_IF_BRIDGE_H


#include <linux/netdevice.h>
#include <uapi/linux/if_bridge.h>
#include <linux/bitops.h>

struct br_ip {
	union {
		__be32	ip4;
#if IS_ENABLED(CONFIG_IPV6)
		struct in6_addr ip6;
#endif
	} u;
	__be16		proto;
	__u16           vid;
};

struct br_ip_list {
	struct list_head list;
	struct br_ip addr;
};

#define BR_HAIRPIN_MODE		BIT(0)
#define BR_BPDU_GUARD		BIT(1)
#define BR_ROOT_BLOCK		BIT(2)
#define BR_MULTICAST_FAST_LEAVE	BIT(3)
#define BR_ADMIN_COST		BIT(4)
#define BR_LEARNING		BIT(5)
#define BR_FLOOD		BIT(6)
#define BR_AUTO_MASK		(BR_FLOOD | BR_LEARNING)
#define BR_PROMISC		BIT(7)
#define BR_PROXYARP		BIT(8)
#define BR_LEARNING_SYNC	BIT(9)
#define BR_PROXYARP_WIFI	BIT(10)
#define BR_ISOLATE_MODE	BIT(11)

#if defined(CONFIG_BCM_KF_BRIDGE_PORT_ISOLATION) || defined(CONFIG_BCM_KF_BRIDGE_STP)
enum {
	BREVT_IF_CHANGED,
	BREVT_STP_STATE_CHANGED
};
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_PORT_ISOLATION)
extern struct net_device *bridge_get_next_port(char *brName, unsigned int *portNum);
extern int register_bridge_notifier(struct notifier_block *nb);
extern int unregister_bridge_notifier(struct notifier_block *nb);
extern void bridge_get_br_list(char *brList, const unsigned int listSize);
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_STP)
struct stpPortInfo {
	char portName[IFNAMSIZ];
	unsigned char stpState;
};
extern int register_bridge_stp_notifier(struct notifier_block *nb);
extern int unregister_bridge_stp_notifier(struct notifier_block *nb);
#endif

extern void brioctl_set(int (*ioctl_hook)(struct net *, unsigned int, void __user *));

typedef int br_should_route_hook_t(struct sk_buff *skb);
extern br_should_route_hook_t __rcu *br_should_route_hook;
#if defined(CONFIG_BCM_KF_FBOND) && (defined(CONFIG_BCM_FBOND) || defined(CONFIG_BCM_FBOND_MODULE))
typedef struct net_device *(* br_fb_process_hook_t)(struct sk_buff *skb_p, uint16_t h_proto, struct net_device *txDev );
extern void br_fb_bind(br_fb_process_hook_t brFbProcessHook);
#endif

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
typedef int (*br_bcm_mcast_receive_hook)(int ifindex, struct sk_buff *skb, int is_routed);
typedef int (*br_bcm_mcast_should_deliver_hook)(int ifindex, const struct sk_buff *skb, struct net_device *dst_dev);
int br_bcm_mcast_flood_forward(struct net_device *dev, struct sk_buff *skb);
int br_bcm_mcast_bind(br_bcm_mcast_receive_hook bcm_rx_hook, br_bcm_mcast_should_deliver_hook bcm_should_deliver_hook);
#endif

#if IS_ENABLED(CONFIG_BRIDGE) && IS_ENABLED(CONFIG_BRIDGE_IGMP_SNOOPING)
int br_multicast_list_adjacent(struct net_device *dev,
			       struct list_head *br_ip_list);
bool br_multicast_has_querier_anywhere(struct net_device *dev, int proto);
bool br_multicast_has_querier_adjacent(struct net_device *dev, int proto);
#else
static inline int br_multicast_list_adjacent(struct net_device *dev,
					     struct list_head *br_ip_list)
{
	return 0;
}
static inline bool br_multicast_has_querier_anywhere(struct net_device *dev,
						     int proto)
{
	return false;
}
static inline bool br_multicast_has_querier_adjacent(struct net_device *dev,
						     int proto)
{
	return false;
}
#endif

#endif
