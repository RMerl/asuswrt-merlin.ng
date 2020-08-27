/* Copyright (C) 2007-2015 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_H_
#define _NET_BATMAN_ADV_COMPAT_H_

#include <linux/version.h>	/* LINUX_VERSION_CODE */
#include <linux/kconfig.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33))
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)

#define skb_iif iif

#define batadv_softif_destroy_netlink(dev, head) batadv_softif_destroy_netlink(dev)

#endif /* < KERNEL_VERSION(2, 6, 33) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)

#include <linux/netdevice.h>

#define netdev_master_upper_dev_get_rcu(dev) \
	(dev->br_port ? dev : NULL); \
	break;

#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#include <linux/netdevice.h>

#define netdev_master_upper_dev_get_rcu(dev) \
	(dev->priv_flags & IFF_BRIDGE_PORT ? dev : NULL); \
	break;

#endif /* < KERNEL_VERSION(2, 6, 36) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)

/* hack for dev->addr_assign_type &= ~NET_ADDR_RANDOM; */
#define addr_assign_type ifindex

#endif /* < KERNEL_VERSION(2, 6, 36) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)

/* Hack for removing ndo_add/del_slave at the end of net_device_ops.
 * This is somewhat ugly because it requires that ndo_validate_addr
 * is at the end of this struct in soft-interface.c.
 */
#include <linux/netdevice.h>

#define ndo_validate_addr \
	ndo_validate_addr = eth_validate_addr, \
}; \
static const struct { \
	void *ndo_validate_addr; \
	void *ndo_add_slave; \
	void *ndo_del_slave; \
} __attribute__((unused)) __useless_ops1 = { \
	.ndo_validate_addr

#define ndo_del_slave          ndo_init
#define ndo_init(x, y)         ndo_init - master->netdev_ops->ndo_init - EBUSY

#endif /* < KERNEL_VERSION(2, 6, 39) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)

#define batadv_interface_add_vid(x, y, z) \
__batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
                          unsigned short vid);\
static void batadv_interface_add_vid(struct net_device *dev, unsigned short vid)\
{\
       __batadv_interface_add_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
                                     unsigned short vid)

#define batadv_interface_kill_vid(x, y, z) \
__batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
                           unsigned short vid);\
static void batadv_interface_kill_vid(struct net_device *dev,\
                                     unsigned short vid)\
{\
       __batadv_interface_kill_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
                                      unsigned short vid)

#endif /* < KERNEL_VERSION(3, 3, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#define batadv_interface_set_mac_addr(x, y) \
__batadv_interface_set_mac_addr(struct net_device *dev, void *p);\
static int batadv_interface_set_mac_addr(struct net_device *dev, void *p) \
{\
	int ret;\
\
	ret = __batadv_interface_set_mac_addr(dev, p);\
	if (!ret) \
		dev->addr_assign_type &= ~NET_ADDR_RANDOM;\
	return ret;\
}\
static int __batadv_interface_set_mac_addr(x, y)

#define batadv_interface_tx(x, y) \
__batadv_interface_tx(struct sk_buff *skb, struct net_device *soft_iface); \
static int batadv_interface_tx(struct sk_buff *skb, \
			       struct net_device *soft_iface) \
{ \
	skb_reset_mac_header(skb); \
	return __batadv_interface_tx(skb, soft_iface); \
} \
static int __batadv_interface_tx(struct sk_buff *skb, \
				 struct net_device *soft_iface)

#endif /* < KERNEL_VERSION(3, 9, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)

#define batadv_interface_add_vid(x, y, z) \
__batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
			   unsigned short vid);\
static int batadv_interface_add_vid(struct net_device *dev, unsigned short vid)\
{\
	return __batadv_interface_add_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
				      unsigned short vid)

#define batadv_interface_kill_vid(x, y, z) \
__batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
			    unsigned short vid);\
static int batadv_interface_kill_vid(struct net_device *dev,\
				     unsigned short vid)\
{\
	return __batadv_interface_kill_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
				       unsigned short vid)

#endif /* >= KERNEL_VERSION(3, 3, 0) */

#endif /* < KERNEL_VERSION(3, 10, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)

#define IFF_NO_QUEUE	0; dev->tx_queue_len = 0

#endif /* < KERNEL_VERSION(4, 3, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
