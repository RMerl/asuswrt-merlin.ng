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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_

#include <linux/version.h>
#include_next <linux/netdevice.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)

#include <linux/netdev_features.h>

#endif /* < KERNEL_VERSION(3, 3, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)

#define unregister_netdevice_queue(dev, head) unregister_netdevice(dev)

#endif /* < KERNEL_VERSION(2, 6, 33) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)

#include <linux/etherdevice.h>

#undef  netdev_for_each_mc_addr
#define netdev_for_each_mc_addr(mclist, dev) \
	for (mclist = (struct batadv_dev_addr_list *)dev->mc_list; mclist; \
	     mclist = (struct batadv_dev_addr_list *)mclist->next)

/* Note, that this breaks the usage of the normal 'struct netdev_hw_addr'
 * for kernels < 2.6.35 in batman-adv!
 */
#define netdev_hw_addr batadv_dev_addr_list
struct batadv_dev_addr_list {
	struct dev_addr_list *next;
	u8  addr[MAX_ADDR_LEN];
	u8  da_addrlen;
	u8  da_synced;
	int da_users;
	int da_gusers;
};

#define NETDEV_PRE_TYPE_CHANGE	0x000E
#define NETDEV_POST_TYPE_CHANGE	0x000F

#endif /* < KERNEL_VERSION(2, 6, 35) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)

#define NET_ADDR_RANDOM 0

#endif /* < KERNEL_VERSION(2, 6, 36) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)

/* On older kernels net_dev->master is reserved for iface bonding. */
static inline int batadv_netdev_set_master(struct net_device *slave,
					   struct net_device *master)
{
	return 0;
}

#define netdev_set_master batadv_netdev_set_master

#endif /* < KERNEL_VERSION(2, 6, 39) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#define netdev_upper_dev_unlink(slave, master) netdev_set_master(slave, NULL)
#define netdev_master_upper_dev_get(dev) \
({\
	ASSERT_RTNL();\
	dev->master;\
})

#endif /* < KERNEL_VERSION(3, 9, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)

#define netdev_notifier_info_to_dev(ptr) ptr

#endif /* < KERNEL_VERSION(3, 11, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)

/* alloc_netdev() was defined differently before 2.6.38 */
#undef alloc_netdev
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 38)
#define alloc_netdev(sizeof_priv, name, name_assign_type, setup) \
	alloc_netdev_mq(sizeof_priv, name, setup, 1)
#else
#define alloc_netdev(sizeof_priv, name, name_assign_type, setup) \
	alloc_netdev_mqs(sizeof_priv, name, setup, 1, 1)
#endif /* nested < KERNEL_VERSION(2, 6, 38) */

#endif /* < KERNEL_VERSION(3, 17, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)

#define dev_get_iflink(_net_dev) ((_net_dev)->iflink)

#endif /* < KERNEL_VERSION(3, 19, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#define netdev_master_upper_dev_link(dev, upper_dev, upper_priv, upper_info) \
	netdev_set_master(dev, upper_dev)

#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)

#define netdev_master_upper_dev_link(dev, upper_dev, upper_priv, upper_info) \
	netdev_master_upper_dev_link(dev, upper_dev)

#endif /* < KERNEL_VERSION(4, 5, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_ */
