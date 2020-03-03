/*
 * 	NET3	Protocol independent device support routines.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 *	Derived from the non IP parts of dev.c 1.0.19
 * 		Authors:	Ross Biro
 *				Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *				Mark Evans, <evansmp@uhura.aston.ac.uk>
 *
 *	Additional Authors:
 *		Florian la Roche <rzsfl@rz.uni-sb.de>
 *		Alan Cox <gw4pts@gw4pts.ampr.org>
 *		David Hinds <dahinds@users.sourceforge.net>
 *		Alexey Kuznetsov <kuznet@ms2.inr.ac.ru>
 *		Adam Sulmicki <adam@cfar.umd.edu>
 *              Pekka Riikonen <priikone@poesidon.pspt.fi>
 *
 *	Changes:
 *              D.J. Barrow     :       Fixed bug where dev->refcnt gets set
 *              			to 2 if register_netdev gets called
 *              			before net_dev_init & also removed a
 *              			few lines of code in the process.
 *		Alan Cox	:	device private ioctl copies fields back.
 *		Alan Cox	:	Transmit queue code does relevant
 *					stunts to keep the queue safe.
 *		Alan Cox	:	Fixed double lock.
 *		Alan Cox	:	Fixed promisc NULL pointer trap
 *		????????	:	Support the full private ioctl range
 *		Alan Cox	:	Moved ioctl permission check into
 *					drivers
 *		Tim Kordas	:	SIOCADDMULTI/SIOCDELMULTI
 *		Alan Cox	:	100 backlog just doesn't cut it when
 *					you start doing multicast video 8)
 *		Alan Cox	:	Rewrote net_bh and list manager.
 *		Alan Cox	: 	Fix ETH_P_ALL echoback lengths.
 *		Alan Cox	:	Took out transmit every packet pass
 *					Saved a few bytes in the ioctl handler
 *		Alan Cox	:	Network driver sets packet type before
 *					calling netif_rx. Saves a function
 *					call a packet.
 *		Alan Cox	:	Hashed net_bh()
 *		Richard Kooijman:	Timestamp fixes.
 *		Alan Cox	:	Wrong field in SIOCGIFDSTADDR
 *		Alan Cox	:	Device lock protection.
 *		Alan Cox	: 	Fixed nasty side effect of device close
 *					changes.
 *		Rudi Cilibrasi	:	Pass the right thing to
 *					set_mac_address()
 *		Dave Miller	:	32bit quantity for the device lock to
 *					make it work out on a Sparc.
 *		Bjorn Ekwall	:	Added KERNELD hack.
 *		Alan Cox	:	Cleaned up the backlog initialise.
 *		Craig Metz	:	SIOCGIFCONF fix if space for under
 *					1 device.
 *	    Thomas Bogendoerfer :	Return ENODEV for dev_open, if there
 *					is no device open function.
 *		Andi Kleen	:	Fix error reporting for SIOCGIFCONF
 *	    Michael Chastain	:	Fix signed/unsigned for SIOCGIFCONF
 *		Cyrus Durgin	:	Cleaned for KMOD
 *		Adam Sulmicki   :	Bug Fix : Network Device Unload
 *					A network device unload needs to purge
 *					the backlog queue.
 *	Paul Rusty Russell	:	SIOCSIFNAME
 *              Pekka Riikonen  :	Netdev boot-time settings code
 *              Andrew Morton   :       Make unregister_netdevice wait
 *              			indefinitely on dev->refcnt
 * 		J Hadi Salim	:	- Backlog queue sampling
 *				        - netif_rx() feedback
 */

#include <asm/uaccess.h>
#include <linux/bitops.h>
#include <linux/capability.h>
#include <linux/cpu.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/hash.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/notifier.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <linux/rtnetlink.h>
#include <linux/stat.h>
#include <net/dst.h>
#include <net/pkt_sched.h>
#include <net/checksum.h>
#include <net/xfrm.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netpoll.h>
#include <linux/rcupdate.h>
#include <linux/delay.h>
#include <net/iw_handler.h>
#include <asm/current.h>
#include <linux/audit.h>
#include <linux/dmaengine.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/if_arp.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/mpls.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <trace/events/napi.h>
#include <trace/events/net.h>
#include <trace/events/skb.h>
#include <linux/pci.h>
#include <linux/inetdevice.h>
#include <linux/cpu_rmap.h>
#include <linux/static_key.h>
#include <linux/hashtable.h>
#include <linux/vmalloc.h>
#include <linux/if_macvlan.h>
#include <linux/errqueue.h>
#include <linux/hrtimer.h>
#include <linux/nbuff.h>

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
#include <linux/imq.h>
#endif
#include "net-sysfs.h"

#if defined(CONFIG_BCM_KF_SKB_DEFINES)
#include <linux/kthread.h>
#include <linux/bcm_realtime.h>
#include "skb_defines.h"
#endif
#if defined(CONFIG_BCM_KF_LOG)
#include "linux/bcm_log.h"
#endif

#if defined(CONFIG_BCM_KF_SW_GSO) && defined(CONFIG_BCM_SW_GSO)
#include <net/ip6_checksum.h>
#endif

int bcm_iqos_enable_g = 0;
EXPORT_SYMBOL(bcm_iqos_enable_g);

#ifdef IQOS_DUMMY_FN
int enet_fwdcb_registered = 0;
int
enet_fwdcb_register(enet_fwdcb_t fwdcb)
{
	if (fwdcb)
		enet_fwdcb_registered = 1;
	else
		enet_fwdcb_registered = 0;
	return 0;
}
EXPORT_SYMBOL(enet_fwdcb_register);
#else /* IQOS_DUMMY_FN */
struct sk_buff *
bcm_iqoshdl_wrapper(struct net_device *dev, void *pNBuff)
{
	struct sk_buff *skb = NULL;
	FkBuff_t * pFkb = NULL;

	pFkb = PNBUFF_2_FKBUFF((pNBuff_t)pNBuff);
	if ( !pFkb)
	    return (NULL);

	/*allocate skb & initialize it using fkb */

	skb = skb_xlate_dp(pFkb, NULL);
	if (!skb)
		return (NULL);
	skb->fkb_mark = pFkb->mark;
	skb->priority = pFkb->priority;
	skb->dev = dev;

	skb->protocol = eth_type_trans(skb, dev);
	skb_push(skb, ETH_HLEN);
	return skb;
}
EXPORT_SYMBOL(bcm_iqoshdl_wrapper);
#endif /* !IQOS_DUMMY_FN */

/* Instead of increasing this, you should create a hash table. */
#define MAX_GRO_SKBS 8

/* This should be increased if a protocol with a bigger head is added. */
#define GRO_MAX_HEAD (MAX_HEADER + 128)

static DEFINE_SPINLOCK(ptype_lock);
static DEFINE_SPINLOCK(offload_lock);
struct list_head ptype_base[PTYPE_HASH_SIZE] __read_mostly;
struct list_head ptype_all __read_mostly;	/* Taps */
static struct list_head offload_base __read_mostly;

static int netif_rx_internal(struct sk_buff *skb);
static int call_netdevice_notifiers_info(unsigned long val,
					 struct net_device *dev,
					 struct netdev_notifier_info *info);

/*
 * The @dev_base_head list is protected by @dev_base_lock and the rtnl
 * semaphore.
 *
 * Pure readers hold dev_base_lock for reading, or rcu_read_lock()
 *
 * Writers must hold the rtnl semaphore while they loop through the
 * dev_base_head list, and hold dev_base_lock for writing when they do the
 * actual updates.  This allows pure readers to access the list even
 * while a writer is preparing to update it.
 *
 * To put it another way, dev_base_lock is held for writing only to
 * protect against pure readers; the rtnl semaphore provides the
 * protection against other writers.
 *
 * See, for example usages, register_netdevice() and
 * unregister_netdevice(), which must be called with the rtnl
 * semaphore held.
 */
DEFINE_RWLOCK(dev_base_lock);
EXPORT_SYMBOL(dev_base_lock);

/* protects napi_hash addition/deletion and napi_gen_id */
static DEFINE_SPINLOCK(napi_hash_lock);

static unsigned int napi_gen_id;
static DEFINE_HASHTABLE(napi_hash, 8);

static seqcount_t devnet_rename_seq;
#if defined(CONFIG_BCM_KF_SW_GSO) && defined(CONFIG_BCM_SW_GSO) && !defined(CONFIG_BCM_BPM) && !defined(CONFIG_BCM_BPM_MODULE)
static struct kmem_cache *pktBufferCache;
#endif

static inline void dev_base_seq_inc(struct net *net)
{
	while (++net->dev_base_seq == 0);
}

static inline struct hlist_head *dev_name_hash(struct net *net, const char *name)
{
	unsigned int hash = full_name_hash(name, strnlen(name, IFNAMSIZ));

	return &net->dev_name_head[hash_32(hash, NETDEV_HASHBITS)];
}

static inline struct hlist_head *dev_index_hash(struct net *net, int ifindex)
{
	return &net->dev_index_head[ifindex & (NETDEV_HASHENTRIES - 1)];
}

static inline void rps_lock(struct softnet_data *sd)
{
#ifdef CONFIG_RPS
	spin_lock(&sd->input_pkt_queue.lock);
#endif
}

static inline void rps_unlock(struct softnet_data *sd)
{
#ifdef CONFIG_RPS
	spin_unlock(&sd->input_pkt_queue.lock);
#endif
}

/* Device list insertion */
static void list_netdevice(struct net_device *dev)
{
	struct net *net = dev_net(dev);

	ASSERT_RTNL();

	write_lock_bh(&dev_base_lock);
	list_add_tail_rcu(&dev->dev_list, &net->dev_base_head);
	hlist_add_head_rcu(&dev->name_hlist, dev_name_hash(net, dev->name));
	hlist_add_head_rcu(&dev->index_hlist,
			   dev_index_hash(net, dev->ifindex));
	write_unlock_bh(&dev_base_lock);

	dev_base_seq_inc(net);
}

/* Device list removal
 * caller must respect a RCU grace period before freeing/reusing dev
 */
static void unlist_netdevice(struct net_device *dev)
{
	ASSERT_RTNL();

	/* Unlink dev from the device chain */
	write_lock_bh(&dev_base_lock);
	list_del_rcu(&dev->dev_list);
	hlist_del_rcu(&dev->name_hlist);
	hlist_del_rcu(&dev->index_hlist);
	write_unlock_bh(&dev_base_lock);

	dev_base_seq_inc(dev_net(dev));
}

/*
 *	Our notifier list
 */

static RAW_NOTIFIER_HEAD(netdev_chain);

/*
 *	Device drivers call our routines to queue packets here. We empty the
 *	queue in the local softnet handler.
 */

DEFINE_PER_CPU_ALIGNED(struct softnet_data, softnet_data);
EXPORT_PER_CPU_SYMBOL(softnet_data);

#ifdef CONFIG_LOCKDEP
/*
 * register_netdevice() inits txq->_xmit_lock and sets lockdep class
 * according to dev->type
 */
static const unsigned short netdev_lock_type[] =
	{ARPHRD_NETROM, ARPHRD_ETHER, ARPHRD_EETHER, ARPHRD_AX25,
	 ARPHRD_PRONET, ARPHRD_CHAOS, ARPHRD_IEEE802, ARPHRD_ARCNET,
	 ARPHRD_APPLETLK, ARPHRD_DLCI, ARPHRD_ATM, ARPHRD_METRICOM,
	 ARPHRD_IEEE1394, ARPHRD_EUI64, ARPHRD_INFINIBAND, ARPHRD_SLIP,
	 ARPHRD_CSLIP, ARPHRD_SLIP6, ARPHRD_CSLIP6, ARPHRD_RSRVD,
	 ARPHRD_ADAPT, ARPHRD_ROSE, ARPHRD_X25, ARPHRD_HWX25,
	 ARPHRD_PPP, ARPHRD_CISCO, ARPHRD_LAPB, ARPHRD_DDCMP,
	 ARPHRD_RAWHDLC, ARPHRD_TUNNEL, ARPHRD_TUNNEL6, ARPHRD_FRAD,
	 ARPHRD_SKIP, ARPHRD_LOOPBACK, ARPHRD_LOCALTLK, ARPHRD_FDDI,
	 ARPHRD_BIF, ARPHRD_SIT, ARPHRD_IPDDP, ARPHRD_IPGRE,
	 ARPHRD_PIMREG, ARPHRD_HIPPI, ARPHRD_ASH, ARPHRD_ECONET,
	 ARPHRD_IRDA, ARPHRD_FCPP, ARPHRD_FCAL, ARPHRD_FCPL,
	 ARPHRD_FCFABRIC, ARPHRD_IEEE80211, ARPHRD_IEEE80211_PRISM,
	 ARPHRD_IEEE80211_RADIOTAP, ARPHRD_PHONET, ARPHRD_PHONET_PIPE,
	 ARPHRD_IEEE802154, ARPHRD_VOID, ARPHRD_NONE};

static const char *const netdev_lock_name[] =
	{"_xmit_NETROM", "_xmit_ETHER", "_xmit_EETHER", "_xmit_AX25",
	 "_xmit_PRONET", "_xmit_CHAOS", "_xmit_IEEE802", "_xmit_ARCNET",
	 "_xmit_APPLETLK", "_xmit_DLCI", "_xmit_ATM", "_xmit_METRICOM",
	 "_xmit_IEEE1394", "_xmit_EUI64", "_xmit_INFINIBAND", "_xmit_SLIP",
	 "_xmit_CSLIP", "_xmit_SLIP6", "_xmit_CSLIP6", "_xmit_RSRVD",
	 "_xmit_ADAPT", "_xmit_ROSE", "_xmit_X25", "_xmit_HWX25",
	 "_xmit_PPP", "_xmit_CISCO", "_xmit_LAPB", "_xmit_DDCMP",
	 "_xmit_RAWHDLC", "_xmit_TUNNEL", "_xmit_TUNNEL6", "_xmit_FRAD",
	 "_xmit_SKIP", "_xmit_LOOPBACK", "_xmit_LOCALTLK", "_xmit_FDDI",
	 "_xmit_BIF", "_xmit_SIT", "_xmit_IPDDP", "_xmit_IPGRE",
	 "_xmit_PIMREG", "_xmit_HIPPI", "_xmit_ASH", "_xmit_ECONET",
	 "_xmit_IRDA", "_xmit_FCPP", "_xmit_FCAL", "_xmit_FCPL",
	 "_xmit_FCFABRIC", "_xmit_IEEE80211", "_xmit_IEEE80211_PRISM",
	 "_xmit_IEEE80211_RADIOTAP", "_xmit_PHONET", "_xmit_PHONET_PIPE",
	 "_xmit_IEEE802154", "_xmit_VOID", "_xmit_NONE"};

static struct lock_class_key netdev_xmit_lock_key[ARRAY_SIZE(netdev_lock_type)];
static struct lock_class_key netdev_addr_lock_key[ARRAY_SIZE(netdev_lock_type)];

static inline unsigned short netdev_lock_pos(unsigned short dev_type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(netdev_lock_type); i++)
		if (netdev_lock_type[i] == dev_type)
			return i;
	/* the last key is used by default */
	return ARRAY_SIZE(netdev_lock_type) - 1;
}

static inline void netdev_set_xmit_lockdep_class(spinlock_t *lock,
						 unsigned short dev_type)
{
	int i;

	i = netdev_lock_pos(dev_type);
	lockdep_set_class_and_name(lock, &netdev_xmit_lock_key[i],
				   netdev_lock_name[i]);
}

static inline void netdev_set_addr_lockdep_class(struct net_device *dev)
{
	int i;

	i = netdev_lock_pos(dev->type);
	lockdep_set_class_and_name(&dev->addr_list_lock,
				   &netdev_addr_lock_key[i],
				   netdev_lock_name[i]);
}
#else
static inline void netdev_set_xmit_lockdep_class(spinlock_t *lock,
						 unsigned short dev_type)
{
}
static inline void netdev_set_addr_lockdep_class(struct net_device *dev)
{
}
#endif

#if defined(CONFIG_BCM_KF_NETDEV_PATH)
/* Adds a NON-ROOT device to a path. A Root device is indirectly
   added to a path once another device points to it */
int netdev_path_add(struct net_device *new_dev, struct net_device *next_dev)
{
	/* new device already in a path, fail */
	if (netdev_path_is_linked(new_dev))
		return -EBUSY;

	netdev_path_next_dev(new_dev) = next_dev;

	next_dev->path.refcount++;

	return 0;
}
EXPORT_SYMBOL(netdev_path_add);

/* Removes a device from a path */
int netdev_path_remove(struct net_device *dev)
{
	/* device referenced by one or more interfaces, fail */
	if (!netdev_path_is_leaf(dev))
		return -EBUSY;

	/* device is the first in the list */
	/* Nothing to do */
	if (netdev_path_is_root(dev))
		return 0;

	netdev_path_next_dev(dev)->path.refcount--;

	netdev_path_next_dev(dev) = NULL;

	return 0;
}
EXPORT_SYMBOL(netdev_path_remove);

/* Prints all devices in a path */
void netdev_path_dump(struct net_device *dev)
{
	printk("netdev path : ");

	while (1) {
		printk("%s", dev->name);

		if (netdev_path_is_root(dev))
			break;

		printk(" -> ");

		dev = netdev_path_next_dev(dev);
	}

	printk("\n");
}
EXPORT_SYMBOL(netdev_path_dump);

int netdev_path_set_hw_subport_mcast_idx(struct net_device *dev,
					 unsigned int subport_idx)
{
	if (subport_idx >= NETDEV_PATH_HW_SUBPORTS_MAX) {
		printk(KERN_ERR "%s : Invalid subport <%u>, max <%u>",
		       __func__, subport_idx, NETDEV_PATH_HW_SUBPORTS_MAX);
		return -1;
	}

	dev->path.hw_subport_mcast_idx = subport_idx;

	return 0;
}
EXPORT_SYMBOL(netdev_path_set_hw_subport_mcast_idx);
#endif /* CONFIG_BCM_KF_NETDEV_PATH */


/*******************************************************************************

		Protocol management and registration routines

*******************************************************************************/

/*
 *	Add a protocol ID to the list. Now that the input handler is
 *	smarter we can dispense with all the messy stuff that used to be
 *	here.
 *
 *	BEWARE!!! Protocol handlers, mangling input packets,
 *	MUST BE last in hash buckets and checking protocol handlers
 *	MUST start from promiscuous ptype_all chain in net_bh.
 *	It is true now, do not change it.
 *	Explanation follows: if protocol handler, mangling packet, will
 *	be the first on list, it is not able to sense, that packet
 *	is cloned and should be copied-on-write, so that it will
 *	change it and subsequent readers will get broken packet.
 *							--ANK (980803)
 */

static inline struct list_head *ptype_head(const struct packet_type *pt)
{
	if (pt->type == htons(ETH_P_ALL))
		return pt->dev ? &pt->dev->ptype_all : &ptype_all;
	else
		return pt->dev ? &pt->dev->ptype_specific :
				 &ptype_base[ntohs(pt->type) & PTYPE_HASH_MASK];
}

/**
 *	dev_add_pack - add packet handler
 *	@pt: packet type declaration
 *
 *	Add a protocol handler to the networking stack. The passed &packet_type
 *	is linked into kernel lists and may not be freed until it has been
 *	removed from the kernel lists.
 *
 *	This call does not sleep therefore it can not
 *	guarantee all CPU's that are in middle of receiving packets
 *	will see the new packet type (until the next received packet).
 */

void dev_add_pack(struct packet_type *pt)
{
	struct list_head *head = ptype_head(pt);

	spin_lock(&ptype_lock);
	list_add_rcu(&pt->list, head);
	spin_unlock(&ptype_lock);
}
EXPORT_SYMBOL(dev_add_pack);

/**
 *	__dev_remove_pack	 - remove packet handler
 *	@pt: packet type declaration
 *
 *	Remove a protocol handler that was previously added to the kernel
 *	protocol handlers by dev_add_pack(). The passed &packet_type is removed
 *	from the kernel lists and can be freed or reused once this function
 *	returns.
 *
 *      The packet type might still be in use by receivers
 *	and must not be freed until after all the CPU's have gone
 *	through a quiescent state.
 */
void __dev_remove_pack(struct packet_type *pt)
{
	struct list_head *head = ptype_head(pt);
	struct packet_type *pt1;

	spin_lock(&ptype_lock);

	list_for_each_entry(pt1, head, list) {
		if (pt == pt1) {
			list_del_rcu(&pt->list);
			goto out;
		}
	}

	pr_warn("dev_remove_pack: %p not found\n", pt);
out:
	spin_unlock(&ptype_lock);
}
EXPORT_SYMBOL(__dev_remove_pack);

/**
 *	dev_remove_pack	 - remove packet handler
 *	@pt: packet type declaration
 *
 *	Remove a protocol handler that was previously added to the kernel
 *	protocol handlers by dev_add_pack(). The passed &packet_type is removed
 *	from the kernel lists and can be freed or reused once this function
 *	returns.
 *
 *	This call sleeps to guarantee that no CPU is looking at the packet
 *	type after return.
 */
void dev_remove_pack(struct packet_type *pt)
{
	__dev_remove_pack(pt);

	synchronize_net();
}
EXPORT_SYMBOL(dev_remove_pack);


/**
 *	dev_add_offload - register offload handlers
 *	@po: protocol offload declaration
 *
 *	Add protocol offload handlers to the networking stack. The passed
 *	&proto_offload is linked into kernel lists and may not be freed until
 *	it has been removed from the kernel lists.
 *
 *	This call does not sleep therefore it can not
 *	guarantee all CPU's that are in middle of receiving packets
 *	will see the new offload handlers (until the next received packet).
 */
void dev_add_offload(struct packet_offload *po)
{
	struct list_head *head = &offload_base;

	spin_lock(&offload_lock);
	list_add_rcu(&po->list, head);
	spin_unlock(&offload_lock);
}
EXPORT_SYMBOL(dev_add_offload);

/**
 *	__dev_remove_offload	 - remove offload handler
 *	@po: packet offload declaration
 *
 *	Remove a protocol offload handler that was previously added to the
 *	kernel offload handlers by dev_add_offload(). The passed &offload_type
 *	is removed from the kernel lists and can be freed or reused once this
 *	function returns.
 *
 *      The packet type might still be in use by receivers
 *	and must not be freed until after all the CPU's have gone
 *	through a quiescent state.
 */
static void __dev_remove_offload(struct packet_offload *po)
{
	struct list_head *head = &offload_base;
	struct packet_offload *po1;

	spin_lock(&offload_lock);

	list_for_each_entry(po1, head, list) {
		if (po == po1) {
			list_del_rcu(&po->list);
			goto out;
		}
	}

	pr_warn("dev_remove_offload: %p not found\n", po);
out:
	spin_unlock(&offload_lock);
}

/**
 *	dev_remove_offload	 - remove packet offload handler
 *	@po: packet offload declaration
 *
 *	Remove a packet offload handler that was previously added to the kernel
 *	offload handlers by dev_add_offload(). The passed &offload_type is
 *	removed from the kernel lists and can be freed or reused once this
 *	function returns.
 *
 *	This call sleeps to guarantee that no CPU is looking at the packet
 *	type after return.
 */
void dev_remove_offload(struct packet_offload *po)
{
	__dev_remove_offload(po);

	synchronize_net();
}
EXPORT_SYMBOL(dev_remove_offload);

/******************************************************************************

		      Device Boot-time Settings Routines

*******************************************************************************/

/* Boot time configuration table */
static struct netdev_boot_setup dev_boot_setup[NETDEV_BOOT_SETUP_MAX];

/**
 *	netdev_boot_setup_add	- add new setup entry
 *	@name: name of the device
 *	@map: configured settings for the device
 *
 *	Adds new setup entry to the dev_boot_setup list.  The function
 *	returns 0 on error and 1 on success.  This is a generic routine to
 *	all netdevices.
 */
static int netdev_boot_setup_add(char *name, struct ifmap *map)
{
	struct netdev_boot_setup *s;
	int i;

	s = dev_boot_setup;
	for (i = 0; i < NETDEV_BOOT_SETUP_MAX; i++) {
		if (s[i].name[0] == '\0' || s[i].name[0] == ' ') {
			memset(s[i].name, 0, sizeof(s[i].name));
			strlcpy(s[i].name, name, IFNAMSIZ);
			memcpy(&s[i].map, map, sizeof(s[i].map));
			break;
		}
	}

	return i >= NETDEV_BOOT_SETUP_MAX ? 0 : 1;
}

/**
 *	netdev_boot_setup_check	- check boot time settings
 *	@dev: the netdevice
 *
 * 	Check boot time settings for the device.
 *	The found settings are set for the device to be used
 *	later in the device probing.
 *	Returns 0 if no settings found, 1 if they are.
 */
int netdev_boot_setup_check(struct net_device *dev)
{
	struct netdev_boot_setup *s = dev_boot_setup;
	int i;

	for (i = 0; i < NETDEV_BOOT_SETUP_MAX; i++) {
		if (s[i].name[0] != '\0' && s[i].name[0] != ' ' &&
		    !strcmp(dev->name, s[i].name)) {
			dev->irq 	= s[i].map.irq;
			dev->base_addr 	= s[i].map.base_addr;
			dev->mem_start 	= s[i].map.mem_start;
			dev->mem_end 	= s[i].map.mem_end;
			return 1;
		}
	}
	return 0;
}
EXPORT_SYMBOL(netdev_boot_setup_check);


/**
 *	netdev_boot_base	- get address from boot time settings
 *	@prefix: prefix for network device
 *	@unit: id for network device
 *
 * 	Check boot time settings for the base address of device.
 *	The found settings are set for the device to be used
 *	later in the device probing.
 *	Returns 0 if no settings found.
 */
unsigned long netdev_boot_base(const char *prefix, int unit)
{
	const struct netdev_boot_setup *s = dev_boot_setup;
	char name[IFNAMSIZ];
	int i;

	sprintf(name, "%s%d", prefix, unit);

	/*
	 * If device already registered then return base of 1
	 * to indicate not to probe for this interface
	 */
	if (__dev_get_by_name(&init_net, name))
		return 1;

	for (i = 0; i < NETDEV_BOOT_SETUP_MAX; i++)
		if (!strcmp(name, s[i].name))
			return s[i].map.base_addr;
	return 0;
}

/*
 * Saves at boot time configured settings for any netdevice.
 */
int __init netdev_boot_setup(char *str)
{
	int ints[5];
	struct ifmap map;

	str = get_options(str, ARRAY_SIZE(ints), ints);
	if (!str || !*str)
		return 0;

	/* Save settings */
	memset(&map, 0, sizeof(map));
	if (ints[0] > 0)
		map.irq = ints[1];
	if (ints[0] > 1)
		map.base_addr = ints[2];
	if (ints[0] > 2)
		map.mem_start = ints[3];
	if (ints[0] > 3)
		map.mem_end = ints[4];

	/* Add new entry to the list */
	return netdev_boot_setup_add(str, &map);
}

__setup("netdev=", netdev_boot_setup);

/*******************************************************************************

			    Device Interface Subroutines

*******************************************************************************/

/**
 *	dev_get_iflink	- get 'iflink' value of a interface
 *	@dev: targeted interface
 *
 *	Indicates the ifindex the interface is linked to.
 *	Physical interfaces have the same 'ifindex' and 'iflink' values.
 */

int dev_get_iflink(const struct net_device *dev)
{
	if (dev->netdev_ops && dev->netdev_ops->ndo_get_iflink)
		return dev->netdev_ops->ndo_get_iflink(dev);

	return dev->ifindex;
}
EXPORT_SYMBOL(dev_get_iflink);

/**
 *	__dev_get_by_name	- find a device by its name
 *	@net: the applicable net namespace
 *	@name: name to find
 *
 *	Find an interface by name. Must be called under RTNL semaphore
 *	or @dev_base_lock. If the name is found a pointer to the device
 *	is returned. If the name is not found then %NULL is returned. The
 *	reference counters are not incremented so the caller must be
 *	careful with locks.
 */

struct net_device *__dev_get_by_name(struct net *net, const char *name)
{
	struct net_device *dev;
	struct hlist_head *head = dev_name_hash(net, name);

	hlist_for_each_entry(dev, head, name_hlist)
		if (!strncmp(dev->name, name, IFNAMSIZ))
			return dev;

	return NULL;
}
EXPORT_SYMBOL(__dev_get_by_name);

/**
 *	dev_get_by_name_rcu	- find a device by its name
 *	@net: the applicable net namespace
 *	@name: name to find
 *
 *	Find an interface by name.
 *	If the name is found a pointer to the device is returned.
 * 	If the name is not found then %NULL is returned.
 *	The reference counters are not incremented so the caller must be
 *	careful with locks. The caller must hold RCU lock.
 */

struct net_device *dev_get_by_name_rcu(struct net *net, const char *name)
{
	struct net_device *dev;
	struct hlist_head *head = dev_name_hash(net, name);

	hlist_for_each_entry_rcu(dev, head, name_hlist)
		if (!strncmp(dev->name, name, IFNAMSIZ))
			return dev;

	return NULL;
}
EXPORT_SYMBOL(dev_get_by_name_rcu);

/**
 *	dev_get_by_name		- find a device by its name
 *	@net: the applicable net namespace
 *	@name: name to find
 *
 *	Find an interface by name. This can be called from any
 *	context and does its own locking. The returned handle has
 *	the usage count incremented and the caller must use dev_put() to
 *	release it when it is no longer needed. %NULL is returned if no
 *	matching device is found.
 */

struct net_device *dev_get_by_name(struct net *net, const char *name)
{
	struct net_device *dev;

	rcu_read_lock();
	dev = dev_get_by_name_rcu(net, name);
	if (dev)
		dev_hold(dev);
	rcu_read_unlock();
	return dev;
}
EXPORT_SYMBOL(dev_get_by_name);

/**
 *	__dev_get_by_index - find a device by its ifindex
 *	@net: the applicable net namespace
 *	@ifindex: index of device
 *
 *	Search for an interface by index. Returns %NULL if the device
 *	is not found or a pointer to the device. The device has not
 *	had its reference counter increased so the caller must be careful
 *	about locking. The caller must hold either the RTNL semaphore
 *	or @dev_base_lock.
 */

struct net_device *__dev_get_by_index(struct net *net, int ifindex)
{
	struct net_device *dev;
	struct hlist_head *head = dev_index_hash(net, ifindex);

	hlist_for_each_entry(dev, head, index_hlist)
		if (dev->ifindex == ifindex)
			return dev;

	return NULL;
}
EXPORT_SYMBOL(__dev_get_by_index);

/**
 *	dev_get_by_index_rcu - find a device by its ifindex
 *	@net: the applicable net namespace
 *	@ifindex: index of device
 *
 *	Search for an interface by index. Returns %NULL if the device
 *	is not found or a pointer to the device. The device has not
 *	had its reference counter increased so the caller must be careful
 *	about locking. The caller must hold RCU lock.
 */

struct net_device *dev_get_by_index_rcu(struct net *net, int ifindex)
{
	struct net_device *dev;
	struct hlist_head *head = dev_index_hash(net, ifindex);

	hlist_for_each_entry_rcu(dev, head, index_hlist)
		if (dev->ifindex == ifindex)
			return dev;

	return NULL;
}
EXPORT_SYMBOL(dev_get_by_index_rcu);


/**
 *	dev_get_by_index - find a device by its ifindex
 *	@net: the applicable net namespace
 *	@ifindex: index of device
 *
 *	Search for an interface by index. Returns NULL if the device
 *	is not found or a pointer to the device. The device returned has
 *	had a reference added and the pointer is safe until the user calls
 *	dev_put to indicate they have finished with it.
 */

struct net_device *dev_get_by_index(struct net *net, int ifindex)
{
	struct net_device *dev;

	rcu_read_lock();
	dev = dev_get_by_index_rcu(net, ifindex);
	if (dev)
		dev_hold(dev);
	rcu_read_unlock();
	return dev;
}
EXPORT_SYMBOL(dev_get_by_index);

/**
 *	netdev_get_name - get a netdevice name, knowing its ifindex.
 *	@net: network namespace
 *	@name: a pointer to the buffer where the name will be stored.
 *	@ifindex: the ifindex of the interface to get the name from.
 *
 *	The use of raw_seqcount_begin() and cond_resched() before
 *	retrying is required as we want to give the writers a chance
 *	to complete when CONFIG_PREEMPT is not set.
 */
int netdev_get_name(struct net *net, char *name, int ifindex)
{
	struct net_device *dev;
	unsigned int seq;

retry:
	seq = raw_seqcount_begin(&devnet_rename_seq);
	rcu_read_lock();
	dev = dev_get_by_index_rcu(net, ifindex);
	if (!dev) {
		rcu_read_unlock();
		return -ENODEV;
	}

	strcpy(name, dev->name);
	rcu_read_unlock();
	if (read_seqcount_retry(&devnet_rename_seq, seq)) {
		cond_resched();
		goto retry;
	}

	return 0;
}

/**
 *	dev_getbyhwaddr_rcu - find a device by its hardware address
 *	@net: the applicable net namespace
 *	@type: media type of device
 *	@ha: hardware address
 *
 *	Search for an interface by MAC address. Returns NULL if the device
 *	is not found or a pointer to the device.
 *	The caller must hold RCU or RTNL.
 *	The returned device has not had its ref count increased
 *	and the caller must therefore be careful about locking
 *
 */

struct net_device *dev_getbyhwaddr_rcu(struct net *net, unsigned short type,
				       const char *ha)
{
	struct net_device *dev;

	for_each_netdev_rcu(net, dev)
		if (dev->type == type &&
		    !memcmp(dev->dev_addr, ha, dev->addr_len))
			return dev;

	return NULL;
}
EXPORT_SYMBOL(dev_getbyhwaddr_rcu);

struct net_device *__dev_getfirstbyhwtype(struct net *net, unsigned short type)
{
	struct net_device *dev;

	ASSERT_RTNL();
	for_each_netdev(net, dev)
		if (dev->type == type)
			return dev;

	return NULL;
}
EXPORT_SYMBOL(__dev_getfirstbyhwtype);

struct net_device *dev_getfirstbyhwtype(struct net *net, unsigned short type)
{
	struct net_device *dev, *ret = NULL;

	rcu_read_lock();
	for_each_netdev_rcu(net, dev)
		if (dev->type == type) {
			dev_hold(dev);
			ret = dev;
			break;
		}
	rcu_read_unlock();
	return ret;
}
EXPORT_SYMBOL(dev_getfirstbyhwtype);

/**
 *	__dev_get_by_flags - find any device with given flags
 *	@net: the applicable net namespace
 *	@if_flags: IFF_* values
 *	@mask: bitmask of bits in if_flags to check
 *
 *	Search for any interface with the given flags. Returns NULL if a device
 *	is not found or a pointer to the device. Must be called inside
 *	rtnl_lock(), and result refcount is unchanged.
 */

struct net_device *__dev_get_by_flags(struct net *net, unsigned short if_flags,
				      unsigned short mask)
{
	struct net_device *dev, *ret;

	ASSERT_RTNL();

	ret = NULL;
	for_each_netdev(net, dev) {
		if (((dev->flags ^ if_flags) & mask) == 0) {
			ret = dev;
			break;
		}
	}
	return ret;
}
EXPORT_SYMBOL(__dev_get_by_flags);

/**
 *	dev_valid_name - check if name is okay for network device
 *	@name: name string
 *
 *	Network device names need to be valid file names to
 *	to allow sysfs to work.  We also disallow any kind of
 *	whitespace.
 */
bool dev_valid_name(const char *name)
{
	if (*name == '\0')
		return false;
	if (strnlen(name, IFNAMSIZ) == IFNAMSIZ)
		return false;
	if (!strcmp(name, ".") || !strcmp(name, ".."))
		return false;

	while (*name) {
		if (*name == '/' || *name == ':' || isspace(*name))
			return false;
		name++;
	}
	return true;
}
EXPORT_SYMBOL(dev_valid_name);

#if defined(CONFIG_BCM_KF_FAP)
/*
 * Features changed due to FAP power up/down
 */
void dev_change_features(unsigned int features, unsigned int op)
{
	struct net *net;
	struct net_device *dev;

	write_lock_bh(&dev_base_lock);

	for_each_net(net) {
		for_each_netdev(net, dev) {
			if (dev->priv_flags & (IFF_HW_SWITCH | IFF_EBRIDGE)) {
				if (op)	/* FAP power up = add features */
					dev->features |= features;
				else	/* FAP powerdown = remove features */
					dev->features &= ~features;
			}
		}
	}

	write_unlock_bh(&dev_base_lock);
}
EXPORT_SYMBOL(dev_change_features);
#endif

/**
 *	__dev_alloc_name - allocate a name for a device
 *	@net: network namespace to allocate the device name in
 *	@name: name format string
 *	@buf:  scratch buffer and result name string
 *
 *	Passed a format string - eg "lt%d" it will try and find a suitable
 *	id. It scans list of devices to build up a free map, then chooses
 *	the first empty slot. The caller must hold the dev_base or rtnl lock
 *	while allocating the name and adding the device in order to avoid
 *	duplicates.
 *	Limited to bits_per_byte * page size devices (ie 32K on most platforms).
 *	Returns the number of the unit assigned or a negative errno code.
 */

static int __dev_alloc_name(struct net *net, const char *name, char *buf)
{
	int i = 0;
	const char *p;
	const int max_netdevices = 8*PAGE_SIZE;
	unsigned long *inuse;
	struct net_device *d;

	p = strnchr(name, IFNAMSIZ-1, '%');
	if (p) {
		/*
		 * Verify the string as this thing may have come from
		 * the user.  There must be either one "%d" and no other "%"
		 * characters.
		 */
		if (p[1] != 'd' || strchr(p + 2, '%'))
			return -EINVAL;

		/* Use one page as a bit array of possible slots */
		inuse = (unsigned long *) get_zeroed_page(GFP_ATOMIC);
		if (!inuse)
			return -ENOMEM;

		for_each_netdev(net, d) {
			if (!sscanf(d->name, name, &i))
				continue;
			if (i < 0 || i >= max_netdevices)
				continue;

			/*  avoid cases where sscanf is not exact inverse of printf */
			snprintf(buf, IFNAMSIZ, name, i);
			if (!strncmp(buf, d->name, IFNAMSIZ))
				set_bit(i, inuse);
		}

		i = find_first_zero_bit(inuse, max_netdevices);
		free_page((unsigned long) inuse);
	}

	if (buf != name)
		snprintf(buf, IFNAMSIZ, name, i);
	if (!__dev_get_by_name(net, buf))
		return i;

	/* It is possible to run out of possible slots
	 * when the name is long and there isn't enough space left
	 * for the digits, or if all bits are used.
	 */
	return -ENFILE;
}

/**
 *	dev_alloc_name - allocate a name for a device
 *	@dev: device
 *	@name: name format string
 *
 *	Passed a format string - eg "lt%d" it will try and find a suitable
 *	id. It scans list of devices to build up a free map, then chooses
 *	the first empty slot. The caller must hold the dev_base or rtnl lock
 *	while allocating the name and adding the device in order to avoid
 *	duplicates.
 *	Limited to bits_per_byte * page size devices (ie 32K on most platforms).
 *	Returns the number of the unit assigned or a negative errno code.
 */

int dev_alloc_name(struct net_device *dev, const char *name)
{
	char buf[IFNAMSIZ];
	struct net *net;
	int ret;

	BUG_ON(!dev_net(dev));
	net = dev_net(dev);
	ret = __dev_alloc_name(net, name, buf);
	if (ret >= 0)
		strlcpy(dev->name, buf, IFNAMSIZ);
	return ret;
}
EXPORT_SYMBOL(dev_alloc_name);

static int dev_alloc_name_ns(struct net *net,
			     struct net_device *dev,
			     const char *name)
{
	char buf[IFNAMSIZ];
	int ret;

	ret = __dev_alloc_name(net, name, buf);
	if (ret >= 0)
		strlcpy(dev->name, buf, IFNAMSIZ);
	return ret;
}

int dev_get_valid_name(struct net *net, struct net_device *dev,
		       const char *name)
{
	BUG_ON(!net);

	if (!dev_valid_name(name))
		return -EINVAL;

	if (strchr(name, '%'))
		return dev_alloc_name_ns(net, dev, name);
	else if (__dev_get_by_name(net, name))
		return -EEXIST;
	else if (dev->name != name)
		strlcpy(dev->name, name, IFNAMSIZ);

	return 0;
}
EXPORT_SYMBOL(dev_get_valid_name);

/**
 *	dev_change_name - change name of a device
 *	@dev: device
 *	@newname: name (or format string) must be at least IFNAMSIZ
 *
 *	Change name of a device, can pass format strings "eth%d".
 *	for wildcarding.
 */
int dev_change_name(struct net_device *dev, const char *newname)
{
	unsigned char old_assign_type;
	char oldname[IFNAMSIZ];
	int err = 0;
	int ret;
	struct net *net;

	ASSERT_RTNL();
	BUG_ON(!dev_net(dev));

	net = dev_net(dev);
	if (dev->flags & IFF_UP)
		return -EBUSY;

	write_seqcount_begin(&devnet_rename_seq);

	if (strncmp(newname, dev->name, IFNAMSIZ) == 0) {
		write_seqcount_end(&devnet_rename_seq);
		return 0;
	}

	memcpy(oldname, dev->name, IFNAMSIZ);

	err = dev_get_valid_name(net, dev, newname);
	if (err < 0) {
		write_seqcount_end(&devnet_rename_seq);
		return err;
	}

	if (oldname[0] && !strchr(oldname, '%'))
		netdev_info(dev, "renamed from %s\n", oldname);

	old_assign_type = dev->name_assign_type;
	dev->name_assign_type = NET_NAME_RENAMED;

rollback:
	ret = device_rename(&dev->dev, dev->name);
	if (ret) {
		memcpy(dev->name, oldname, IFNAMSIZ);
		dev->name_assign_type = old_assign_type;
		write_seqcount_end(&devnet_rename_seq);
		return ret;
	}

	write_seqcount_end(&devnet_rename_seq);

	netdev_adjacent_rename_links(dev, oldname);

	write_lock_bh(&dev_base_lock);
	hlist_del_rcu(&dev->name_hlist);
	write_unlock_bh(&dev_base_lock);

	synchronize_rcu();

	write_lock_bh(&dev_base_lock);
	hlist_add_head_rcu(&dev->name_hlist, dev_name_hash(net, dev->name));
	write_unlock_bh(&dev_base_lock);

	ret = call_netdevice_notifiers(NETDEV_CHANGENAME, dev);
	ret = notifier_to_errno(ret);

	if (ret) {
		/* err >= 0 after dev_alloc_name() or stores the first errno */
		if (err >= 0) {
			err = ret;
			write_seqcount_begin(&devnet_rename_seq);
			memcpy(dev->name, oldname, IFNAMSIZ);
			memcpy(oldname, newname, IFNAMSIZ);
			dev->name_assign_type = old_assign_type;
			old_assign_type = NET_NAME_RENAMED;
			goto rollback;
		} else {
			pr_err("%s: name change rollback failed: %d\n",
			       dev->name, ret);
		}
	}

	return err;
}

/**
 *	dev_set_alias - change ifalias of a device
 *	@dev: device
 *	@alias: name up to IFALIASZ
 *	@len: limit of bytes to copy from info
 *
 *	Set ifalias for a device,
 */
int dev_set_alias(struct net_device *dev, const char *alias, size_t len)
{
	char *new_ifalias;

	ASSERT_RTNL();

	if (len >= IFALIASZ)
		return -EINVAL;

	if (!len) {
		kfree(dev->ifalias);
		dev->ifalias = NULL;
		return 0;
	}

	new_ifalias = krealloc(dev->ifalias, len + 1, GFP_KERNEL);
	if (!new_ifalias)
		return -ENOMEM;
	dev->ifalias = new_ifalias;
	memcpy(dev->ifalias, alias, len);
	dev->ifalias[len] = 0;

	return len;
}


/**
 *	netdev_features_change - device changes features
 *	@dev: device to cause notification
 *
 *	Called to indicate a device has changed features.
 */
void netdev_features_change(struct net_device *dev)
{
	call_netdevice_notifiers(NETDEV_FEAT_CHANGE, dev);
}
EXPORT_SYMBOL(netdev_features_change);

/**
 *	netdev_state_change - device changes state
 *	@dev: device to cause notification
 *
 *	Called to indicate a device has changed state. This function calls
 *	the notifier chains for netdev_chain and sends a NEWLINK message
 *	to the routing socket.
 */
void netdev_state_change(struct net_device *dev)
{
	if (dev->flags & IFF_UP) {
		struct netdev_notifier_change_info change_info;

		change_info.flags_changed = 0;
		call_netdevice_notifiers_info(NETDEV_CHANGE, dev,
					      &change_info.info);
		rtmsg_ifinfo(RTM_NEWLINK, dev, 0, GFP_KERNEL);
	}
}
EXPORT_SYMBOL(netdev_state_change);

/**
 * 	netdev_notify_peers - notify network peers about existence of @dev
 * 	@dev: network device
 *
 * Generate traffic such that interested network peers are aware of
 * @dev, such as by generating a gratuitous ARP. This may be used when
 * a device wants to inform the rest of the network about some sort of
 * reconfiguration such as a failover event or virtual machine
 * migration.
 */
void netdev_notify_peers(struct net_device *dev)
{
	rtnl_lock();
	call_netdevice_notifiers(NETDEV_NOTIFY_PEERS, dev);
	call_netdevice_notifiers(NETDEV_RESEND_IGMP, dev);
	rtnl_unlock();
}
EXPORT_SYMBOL(netdev_notify_peers);

static int __dev_open(struct net_device *dev)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	int ret;

	ASSERT_RTNL();

	if (!netif_device_present(dev))
		return -ENODEV;

	/* Block netpoll from trying to do any rx path servicing.
	 * If we don't do this there is a chance ndo_poll_controller
	 * or ndo_poll may be running while we open the device
	 */
	netpoll_poll_disable(dev);

	ret = call_netdevice_notifiers(NETDEV_PRE_UP, dev);
	ret = notifier_to_errno(ret);
	if (ret)
		return ret;

	set_bit(__LINK_STATE_START, &dev->state);

	if (ops->ndo_validate_addr)
		ret = ops->ndo_validate_addr(dev);

	if (!ret && ops->ndo_open)
		ret = ops->ndo_open(dev);

	netpoll_poll_enable(dev);

	if (ret)
		clear_bit(__LINK_STATE_START, &dev->state);
	else {
		dev->flags |= IFF_UP;
		dev_set_rx_mode(dev);
		dev_activate(dev);
		add_device_randomness(dev->dev_addr, dev->addr_len);
	}

	return ret;
}

/**
 *	dev_open	- prepare an interface for use.
 *	@dev:	device to open
 *
 *	Takes a device from down to up state. The device's private open
 *	function is invoked and then the multicast lists are loaded. Finally
 *	the device is moved into the up state and a %NETDEV_UP message is
 *	sent to the netdev notifier chain.
 *
 *	Calling this function on an active interface is a nop. On a failure
 *	a negative errno code is returned.
 */
int dev_open(struct net_device *dev)
{
	int ret;

	if (dev->flags & IFF_UP)
		return 0;

	ret = __dev_open(dev);
	if (ret < 0)
		return ret;

	rtmsg_ifinfo(RTM_NEWLINK, dev, IFF_UP|IFF_RUNNING, GFP_KERNEL);
	call_netdevice_notifiers(NETDEV_UP, dev);

	return ret;
}
EXPORT_SYMBOL(dev_open);

static int __dev_close_many(struct list_head *head)
{
	struct net_device *dev;

	ASSERT_RTNL();
	might_sleep();

	list_for_each_entry(dev, head, close_list) {
		/* Temporarily disable netpoll until the interface is down */
		netpoll_poll_disable(dev);

		call_netdevice_notifiers(NETDEV_GOING_DOWN, dev);

		clear_bit(__LINK_STATE_START, &dev->state);

		/* Synchronize to scheduled poll. We cannot touch poll list, it
		 * can be even on different cpu. So just clear netif_running().
		 *
		 * dev->stop() will invoke napi_disable() on all of it's
		 * napi_struct instances on this device.
		 */
		smp_mb__after_atomic(); /* Commit netif_running(). */
	}

	dev_deactivate_many(head);

	list_for_each_entry(dev, head, close_list) {
		const struct net_device_ops *ops = dev->netdev_ops;

		/*
		 *	Call the device specific close. This cannot fail.
		 *	Only if device is UP
		 *
		 *	We allow it to be called even after a DETACH hot-plug
		 *	event.
		 */
		if (ops->ndo_stop)
			ops->ndo_stop(dev);

		dev->flags &= ~IFF_UP;
		netpoll_poll_enable(dev);
	}

	return 0;
}

static int __dev_close(struct net_device *dev)
{
	int retval;
	LIST_HEAD(single);

	list_add(&dev->close_list, &single);
	retval = __dev_close_many(&single);
	list_del(&single);

	return retval;
}

int dev_close_many(struct list_head *head, bool unlink)
{
	struct net_device *dev, *tmp;

	/* Remove the devices that don't need to be closed */
	list_for_each_entry_safe(dev, tmp, head, close_list)
		if (!(dev->flags & IFF_UP))
			list_del_init(&dev->close_list);

	__dev_close_many(head);

	list_for_each_entry_safe(dev, tmp, head, close_list) {
		rtmsg_ifinfo(RTM_NEWLINK, dev, IFF_UP|IFF_RUNNING, GFP_KERNEL);
		call_netdevice_notifiers(NETDEV_DOWN, dev);
		if (unlink)
			list_del_init(&dev->close_list);
	}

	return 0;
}
EXPORT_SYMBOL(dev_close_many);

/**
 *	dev_close - shutdown an interface.
 *	@dev: device to shutdown
 *
 *	This function moves an active device into down state. A
 *	%NETDEV_GOING_DOWN is sent to the netdev notifier chain. The device
 *	is then deactivated and finally a %NETDEV_DOWN is sent to the notifier
 *	chain.
 */
int dev_close(struct net_device *dev)
{
	if (dev->flags & IFF_UP) {
		LIST_HEAD(single);

		list_add(&dev->close_list, &single);
		dev_close_many(&single, true);
		list_del(&single);
	}
	return 0;
}
EXPORT_SYMBOL(dev_close);


/**
 *	dev_disable_lro - disable Large Receive Offload on a device
 *	@dev: device
 *
 *	Disable Large Receive Offload (LRO) on a net device.  Must be
 *	called under RTNL.  This is needed if received packets may be
 *	forwarded to another interface.
 */
void dev_disable_lro(struct net_device *dev)
{
	struct net_device *lower_dev;
	struct list_head *iter;

	dev->wanted_features &= ~NETIF_F_LRO;
	netdev_update_features(dev);

	if (unlikely(dev->features & NETIF_F_LRO))
		netdev_WARN(dev, "failed to disable LRO!\n");

	netdev_for_each_lower_dev(dev, lower_dev, iter)
		dev_disable_lro(lower_dev);
}
EXPORT_SYMBOL(dev_disable_lro);

static int call_netdevice_notifier(struct notifier_block *nb, unsigned long val,
				   struct net_device *dev)
{
	struct netdev_notifier_info info;

	netdev_notifier_info_init(&info, dev);
	return nb->notifier_call(nb, val, &info);
}

static int dev_boot_phase = 1;

/**
 *	register_netdevice_notifier - register a network notifier block
 *	@nb: notifier
 *
 *	Register a notifier to be called when network device events occur.
 *	The notifier passed is linked into the kernel structures and must
 *	not be reused until it has been unregistered. A negative errno code
 *	is returned on a failure.
 *
 * 	When registered all registration and up events are replayed
 *	to the new notifier to allow device to have a race free
 *	view of the network device list.
 */

int register_netdevice_notifier(struct notifier_block *nb)
{
	struct net_device *dev;
	struct net_device *last;
	struct net *net;
	int err;

	rtnl_lock();
	err = raw_notifier_chain_register(&netdev_chain, nb);
	if (err)
		goto unlock;
	if (dev_boot_phase)
		goto unlock;
	for_each_net(net) {
		for_each_netdev(net, dev) {
			err = call_netdevice_notifier(nb, NETDEV_REGISTER, dev);
			err = notifier_to_errno(err);
			if (err)
				goto rollback;

			if (!(dev->flags & IFF_UP))
				continue;

			call_netdevice_notifier(nb, NETDEV_UP, dev);
		}
	}

unlock:
	rtnl_unlock();
	return err;

rollback:
	last = dev;
	for_each_net(net) {
		for_each_netdev(net, dev) {
			if (dev == last)
				goto outroll;

			if (dev->flags & IFF_UP) {
				call_netdevice_notifier(nb, NETDEV_GOING_DOWN,
							dev);
				call_netdevice_notifier(nb, NETDEV_DOWN, dev);
			}
			call_netdevice_notifier(nb, NETDEV_UNREGISTER, dev);
		}
	}

outroll:
	raw_notifier_chain_unregister(&netdev_chain, nb);
	goto unlock;
}
EXPORT_SYMBOL(register_netdevice_notifier);

/**
 *	unregister_netdevice_notifier - unregister a network notifier block
 *	@nb: notifier
 *
 *	Unregister a notifier previously registered by
 *	register_netdevice_notifier(). The notifier is unlinked into the
 *	kernel structures and may then be reused. A negative errno code
 *	is returned on a failure.
 *
 * 	After unregistering unregister and down device events are synthesized
 *	for all devices on the device list to the removed notifier to remove
 *	the need for special case cleanup code.
 */

int unregister_netdevice_notifier(struct notifier_block *nb)
{
	struct net_device *dev;
	struct net *net;
	int err;

	rtnl_lock();
	err = raw_notifier_chain_unregister(&netdev_chain, nb);
	if (err)
		goto unlock;

	for_each_net(net) {
		for_each_netdev(net, dev) {
			if (dev->flags & IFF_UP) {
				call_netdevice_notifier(nb, NETDEV_GOING_DOWN,
							dev);
				call_netdevice_notifier(nb, NETDEV_DOWN, dev);
			}
			call_netdevice_notifier(nb, NETDEV_UNREGISTER, dev);
		}
	}
unlock:
	rtnl_unlock();
	return err;
}
EXPORT_SYMBOL(unregister_netdevice_notifier);

/**
 *	call_netdevice_notifiers_info - call all network notifier blocks
 *	@val: value passed unmodified to notifier function
 *	@dev: net_device pointer passed unmodified to notifier function
 *	@info: notifier information data
 *
 *	Call all network notifier blocks.  Parameters and return value
 *	are as for raw_notifier_call_chain().
 */

static int call_netdevice_notifiers_info(unsigned long val,
					 struct net_device *dev,
					 struct netdev_notifier_info *info)
{
	ASSERT_RTNL();
	netdev_notifier_info_init(info, dev);
	return raw_notifier_call_chain(&netdev_chain, val, info);
}

/**
 *	call_netdevice_notifiers - call all network notifier blocks
 *      @val: value passed unmodified to notifier function
 *      @dev: net_device pointer passed unmodified to notifier function
 *
 *	Call all network notifier blocks.  Parameters and return value
 *	are as for raw_notifier_call_chain().
 */

int call_netdevice_notifiers(unsigned long val, struct net_device *dev)
{
	struct netdev_notifier_info info;

	return call_netdevice_notifiers_info(val, dev, &info);
}
EXPORT_SYMBOL(call_netdevice_notifiers);

#ifdef CONFIG_NET_CLS_ACT
static struct static_key ingress_needed __read_mostly;

void net_inc_ingress_queue(void)
{
	static_key_slow_inc(&ingress_needed);
}
EXPORT_SYMBOL_GPL(net_inc_ingress_queue);

void net_dec_ingress_queue(void)
{
	static_key_slow_dec(&ingress_needed);
}
EXPORT_SYMBOL_GPL(net_dec_ingress_queue);
#endif

static struct static_key netstamp_needed __read_mostly;
#ifdef HAVE_JUMP_LABEL
/* We are not allowed to call static_key_slow_dec() from irq context
 * If net_disable_timestamp() is called from irq context, defer the
 * static_key_slow_dec() calls.
 */
static atomic_t netstamp_needed_deferred;
#endif

void net_enable_timestamp(void)
{
#ifdef HAVE_JUMP_LABEL
	int deferred = atomic_xchg(&netstamp_needed_deferred, 0);

	if (deferred) {
		while (--deferred)
			static_key_slow_dec(&netstamp_needed);
		return;
	}
#endif
	static_key_slow_inc(&netstamp_needed);
}
EXPORT_SYMBOL(net_enable_timestamp);

void net_disable_timestamp(void)
{
#ifdef HAVE_JUMP_LABEL
	if (in_interrupt()) {
		atomic_inc(&netstamp_needed_deferred);
		return;
	}
#endif
	static_key_slow_dec(&netstamp_needed);
}
EXPORT_SYMBOL(net_disable_timestamp);

static inline void net_timestamp_set(struct sk_buff *skb)
{
	skb->tstamp.tv64 = 0;
	if (static_key_false(&netstamp_needed))
		__net_timestamp(skb);
}

#define net_timestamp_check(COND, SKB)			\
	if (static_key_false(&netstamp_needed)) {		\
		if ((COND) && !(SKB)->tstamp.tv64)	\
			__net_timestamp(SKB);		\
	}						\

bool is_skb_forwardable(struct net_device *dev, struct sk_buff *skb)
{
	unsigned int len;

	if (!(dev->flags & IFF_UP))
		return false;

	len = dev->mtu + dev->hard_header_len + VLAN_HLEN;
	if (skb->len <= len)
		return true;

	/* if TSO is enabled, we don't care about the length as the packet
	 * could be forwarded without being segmented before
	 */
	if (skb_is_gso(skb))
		return true;

	return false;
}
EXPORT_SYMBOL_GPL(is_skb_forwardable);

int __dev_forward_skb(struct net_device *dev, struct sk_buff *skb)
{
	if (skb_orphan_frags(skb, GFP_ATOMIC) ||
	    unlikely(!is_skb_forwardable(dev, skb))) {
		atomic_long_inc(&dev->rx_dropped);
		kfree_skb(skb);
		return NET_RX_DROP;
	}

	skb_scrub_packet(skb, true);
	skb->priority = 0;
	skb->protocol = eth_type_trans(skb, dev);
	skb_postpull_rcsum(skb, eth_hdr(skb), ETH_HLEN);

	return 0;
}
EXPORT_SYMBOL_GPL(__dev_forward_skb);

/**
 * dev_forward_skb - loopback an skb to another netif
 *
 * @dev: destination network device
 * @skb: buffer to forward
 *
 * return values:
 *	NET_RX_SUCCESS	(no congestion)
 *	NET_RX_DROP     (packet was dropped, but freed)
 *
 * dev_forward_skb can be used for injecting an skb from the
 * start_xmit function of one device into the receive queue
 * of another device.
 *
 * The receiving device may be in another namespace, so
 * we have to clear all information in the skb that could
 * impact namespace isolation.
 */
int dev_forward_skb(struct net_device *dev, struct sk_buff *skb)
{
	return __dev_forward_skb(dev, skb) ?: netif_rx_internal(skb);
}
EXPORT_SYMBOL_GPL(dev_forward_skb);

static inline int deliver_skb(struct sk_buff *skb,
			      struct packet_type *pt_prev,
			      struct net_device *orig_dev)
{
	if (unlikely(skb_orphan_frags(skb, GFP_ATOMIC)))
		return -ENOMEM;
	atomic_inc(&skb->users);
	return pt_prev->func(skb, skb->dev, pt_prev, orig_dev);
}

static inline void deliver_ptype_list_skb(struct sk_buff *skb,
					  struct packet_type **pt,
					  struct net_device *orig_dev,
					  __be16 type,
					  struct list_head *ptype_list)
{
	struct packet_type *ptype, *pt_prev = *pt;

	list_for_each_entry_rcu(ptype, ptype_list, list) {
		if (ptype->type != type)
			continue;
		if (pt_prev)
			deliver_skb(skb, pt_prev, orig_dev);
		pt_prev = ptype;
	}
	*pt = pt_prev;
}

static inline bool skb_loop_sk(struct packet_type *ptype, struct sk_buff *skb)
{
	if (!ptype->af_packet_priv || !skb->sk)
		return false;

	if (ptype->id_match)
		return ptype->id_match(ptype, skb->sk);
	else if ((struct sock *)ptype->af_packet_priv == skb->sk)
		return true;

	return false;
}

/*
 *	Support routine. Sends outgoing frames to any network
 *	taps currently in use.
 */

static void dev_queue_xmit_nit(struct sk_buff *skb, struct net_device *dev)
{
	struct packet_type *ptype;
	struct sk_buff *skb2 = NULL;
	struct packet_type *pt_prev = NULL;
	struct list_head *ptype_list = &ptype_all;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	Blog_t *blog_p = blog_snull(skb);
#endif

	rcu_read_lock();
again:
	list_for_each_entry_rcu(ptype, ptype_list, list) {
		/* Never send packets back to the socket
		 * they originated from - MvS (miquels@drinkel.ow.org)
		 */
		if (skb_loop_sk(ptype, skb))
			continue;

		if (pt_prev) {
			deliver_skb(skb2, pt_prev, skb->dev);
			pt_prev = ptype;
			continue;
		}

		/* need to clone skb, done only once */
		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (!skb2)
			goto out_unlock;

		net_timestamp_set(skb2);

		/* skb->nh should be correctly
		 * set by sender, so that the second statement is
		 * just protection against buggy protocols.
		 */
		skb_reset_mac_header(skb2);

		if (skb_network_header(skb2) < skb2->data ||
		    skb_network_header(skb2) > skb_tail_pointer(skb2)) {
			net_info_ratelimited("protocol %04x is buggy, dev %s\n",
					     ntohs(skb2->protocol),
					     dev->name);
			skb_reset_network_header(skb2);
		}

		skb2->transport_header = skb2->network_header;
		skb2->pkt_type = PACKET_OUTGOING;
		pt_prev = ptype;
	}

	if (ptype_list == &ptype_all) {
		ptype_list = &dev->ptype_all;
		goto again;
	}
out_unlock:
	if (pt_prev)
		pt_prev->func(skb2, skb->dev, pt_prev, skb->dev);
	rcu_read_unlock();
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	skb->blog_p = blog_p;
#endif
}

/**
 * netif_setup_tc - Handle tc mappings on real_num_tx_queues change
 * @dev: Network device
 * @txq: number of queues available
 *
 * If real_num_tx_queues is changed the tc mappings may no longer be
 * valid. To resolve this verify the tc mapping remains valid and if
 * not NULL the mapping. With no priorities mapping to this
 * offset/count pair it will no longer be used. In the worst case TC0
 * is invalid nothing can be done so disable priority mappings. If is
 * expected that drivers will fix this mapping if they can before
 * calling netif_set_real_num_tx_queues.
 */
static void netif_setup_tc(struct net_device *dev, unsigned int txq)
{
	int i;
	struct netdev_tc_txq *tc = &dev->tc_to_txq[0];

	/* If TC0 is invalidated disable TC mapping */
	if (tc->offset + tc->count > txq) {
		pr_warn("Number of in use tx queues changed invalidating tc mappings. Priority traffic classification disabled!\n");
		dev->num_tc = 0;
		return;
	}

	/* Invalidated prio to tc mappings set to TC0 */
	for (i = 1; i < TC_BITMASK + 1; i++) {
		int q = netdev_get_prio_tc_map(dev, i);

		tc = &dev->tc_to_txq[q];
		if (tc->offset + tc->count > txq) {
			pr_warn("Number of in use tx queues changed. Priority %i to tc mapping %i is no longer valid. Setting map to 0\n",
				i, q);
			netdev_set_prio_tc_map(dev, i, 0);
		}
	}
}

#ifdef CONFIG_XPS
static DEFINE_MUTEX(xps_map_mutex);
#define xmap_dereference(P)		\
	rcu_dereference_protected((P), lockdep_is_held(&xps_map_mutex))

static struct xps_map *remove_xps_queue(struct xps_dev_maps *dev_maps,
					int cpu, u16 index)
{
	struct xps_map *map = NULL;
	int pos;

	if (dev_maps)
		map = xmap_dereference(dev_maps->cpu_map[cpu]);

	for (pos = 0; map && pos < map->len; pos++) {
		if (map->queues[pos] == index) {
			if (map->len > 1) {
				map->queues[pos] = map->queues[--map->len];
			} else {
				RCU_INIT_POINTER(dev_maps->cpu_map[cpu], NULL);
				kfree_rcu(map, rcu);
				map = NULL;
			}
			break;
		}
	}

	return map;
}

static void netif_reset_xps_queues_gt(struct net_device *dev, u16 index)
{
	struct xps_dev_maps *dev_maps;
	int cpu, i;
	bool active = false;

	mutex_lock(&xps_map_mutex);
	dev_maps = xmap_dereference(dev->xps_maps);

	if (!dev_maps)
		goto out_no_maps;

	for_each_possible_cpu(cpu) {
		for (i = index; i < dev->num_tx_queues; i++) {
			if (!remove_xps_queue(dev_maps, cpu, i))
				break;
		}
		if (i == dev->num_tx_queues)
			active = true;
	}

	if (!active) {
		RCU_INIT_POINTER(dev->xps_maps, NULL);
		kfree_rcu(dev_maps, rcu);
	}

	for (i = index; i < dev->num_tx_queues; i++)
		netdev_queue_numa_node_write(netdev_get_tx_queue(dev, i),
					     NUMA_NO_NODE);

out_no_maps:
	mutex_unlock(&xps_map_mutex);
}

static struct xps_map *expand_xps_map(struct xps_map *map,
				      int cpu, u16 index)
{
	struct xps_map *new_map;
	int alloc_len = XPS_MIN_MAP_ALLOC;
	int i, pos;

	for (pos = 0; map && pos < map->len; pos++) {
		if (map->queues[pos] != index)
			continue;
		return map;
	}

	/* Need to add queue to this CPU's existing map */
	if (map) {
		if (pos < map->alloc_len)
			return map;

		alloc_len = map->alloc_len * 2;
	}

	/* Need to allocate new map to store queue on this CPU's map */
	new_map = kzalloc_node(XPS_MAP_SIZE(alloc_len), GFP_KERNEL,
			       cpu_to_node(cpu));
	if (!new_map)
		return NULL;

	for (i = 0; i < pos; i++)
		new_map->queues[i] = map->queues[i];
	new_map->alloc_len = alloc_len;
	new_map->len = pos;

	return new_map;
}

int netif_set_xps_queue(struct net_device *dev, const struct cpumask *mask,
			u16 index)
{
	struct xps_dev_maps *dev_maps, *new_dev_maps = NULL;
	struct xps_map *map, *new_map;
	int maps_sz = max_t(unsigned int, XPS_DEV_MAPS_SIZE, L1_CACHE_BYTES);
	int cpu, numa_node_id = -2;
	bool active = false;

	mutex_lock(&xps_map_mutex);

	dev_maps = xmap_dereference(dev->xps_maps);

	/* allocate memory for queue storage */
	for_each_online_cpu(cpu) {
		if (!cpumask_test_cpu(cpu, mask))
			continue;

		if (!new_dev_maps)
			new_dev_maps = kzalloc(maps_sz, GFP_KERNEL);
		if (!new_dev_maps) {
			mutex_unlock(&xps_map_mutex);
			return -ENOMEM;
		}

		map = dev_maps ? xmap_dereference(dev_maps->cpu_map[cpu]) :
				 NULL;

		map = expand_xps_map(map, cpu, index);
		if (!map)
			goto error;

		RCU_INIT_POINTER(new_dev_maps->cpu_map[cpu], map);
	}

	if (!new_dev_maps)
		goto out_no_new_maps;

	for_each_possible_cpu(cpu) {
		if (cpumask_test_cpu(cpu, mask) && cpu_online(cpu)) {
			/* add queue to CPU maps */
			int pos = 0;

			map = xmap_dereference(new_dev_maps->cpu_map[cpu]);
			while ((pos < map->len) && (map->queues[pos] != index))
				pos++;

			if (pos == map->len)
				map->queues[map->len++] = index;
#ifdef CONFIG_NUMA
			if (numa_node_id == -2)
				numa_node_id = cpu_to_node(cpu);
			else if (numa_node_id != cpu_to_node(cpu))
				numa_node_id = -1;
#endif
		} else if (dev_maps) {
			/* fill in the new device map from the old device map */
			map = xmap_dereference(dev_maps->cpu_map[cpu]);
			RCU_INIT_POINTER(new_dev_maps->cpu_map[cpu], map);
		}

	}

	rcu_assign_pointer(dev->xps_maps, new_dev_maps);

	/* Cleanup old maps */
	if (dev_maps) {
		for_each_possible_cpu(cpu) {
			new_map = xmap_dereference(new_dev_maps->cpu_map[cpu]);
			map = xmap_dereference(dev_maps->cpu_map[cpu]);
			if (map && map != new_map)
				kfree_rcu(map, rcu);
		}

		kfree_rcu(dev_maps, rcu);
	}

	dev_maps = new_dev_maps;
	active = true;

out_no_new_maps:
	/* update Tx queue numa node */
	netdev_queue_numa_node_write(netdev_get_tx_queue(dev, index),
				     (numa_node_id >= 0) ? numa_node_id :
				     NUMA_NO_NODE);

	if (!dev_maps)
		goto out_no_maps;

	/* removes queue from unused CPUs */
	for_each_possible_cpu(cpu) {
		if (cpumask_test_cpu(cpu, mask) && cpu_online(cpu))
			continue;

		if (remove_xps_queue(dev_maps, cpu, index))
			active = true;
	}

	/* free map if not active */
	if (!active) {
		RCU_INIT_POINTER(dev->xps_maps, NULL);
		kfree_rcu(dev_maps, rcu);
	}

out_no_maps:
	mutex_unlock(&xps_map_mutex);

	return 0;
error:
	/* remove any maps that we added */
	for_each_possible_cpu(cpu) {
		new_map = xmap_dereference(new_dev_maps->cpu_map[cpu]);
		map = dev_maps ? xmap_dereference(dev_maps->cpu_map[cpu]) :
				 NULL;
		if (new_map && new_map != map)
			kfree(new_map);
	}

	mutex_unlock(&xps_map_mutex);

	kfree(new_dev_maps);
	return -ENOMEM;
}
EXPORT_SYMBOL(netif_set_xps_queue);

#endif
/*
 * Routine to help set real_num_tx_queues. To avoid skbs mapped to queues
 * greater then real_num_tx_queues stale skbs on the qdisc must be flushed.
 */
int netif_set_real_num_tx_queues(struct net_device *dev, unsigned int txq)
{
	bool disabling;
	int rc;

	disabling = txq < dev->real_num_tx_queues;

	if (txq < 1 || txq > dev->num_tx_queues)
		return -EINVAL;

	if (dev->reg_state == NETREG_REGISTERED ||
	    dev->reg_state == NETREG_UNREGISTERING) {
		ASSERT_RTNL();

		rc = netdev_queue_update_kobjects(dev, dev->real_num_tx_queues,
						  txq);
		if (rc)
			return rc;

		if (dev->num_tc)
			netif_setup_tc(dev, txq);

		dev->real_num_tx_queues = txq;

		if (disabling) {
			synchronize_net();
			qdisc_reset_all_tx_gt(dev, txq);
#ifdef CONFIG_XPS
			netif_reset_xps_queues_gt(dev, txq);
#endif
		}
	} else {
		dev->real_num_tx_queues = txq;
	}

	return 0;
}
EXPORT_SYMBOL(netif_set_real_num_tx_queues);

#ifdef CONFIG_SYSFS
/**
 *	netif_set_real_num_rx_queues - set actual number of RX queues used
 *	@dev: Network device
 *	@rxq: Actual number of RX queues
 *
 *	This must be called either with the rtnl_lock held or before
 *	registration of the net device.  Returns 0 on success, or a
 *	negative error code.  If called before registration, it always
 *	succeeds.
 */
int netif_set_real_num_rx_queues(struct net_device *dev, unsigned int rxq)
{
	int rc;

	if (rxq < 1 || rxq > dev->num_rx_queues)
		return -EINVAL;

	if (dev->reg_state == NETREG_REGISTERED) {
		ASSERT_RTNL();

		rc = net_rx_queue_update_kobjects(dev, dev->real_num_rx_queues,
						  rxq);
		if (rc)
			return rc;
	}

	dev->real_num_rx_queues = rxq;
	return 0;
}
EXPORT_SYMBOL(netif_set_real_num_rx_queues);
#endif

/**
 * netif_get_num_default_rss_queues - default number of RSS queues
 *
 * This routine should set an upper limit on the number of RSS queues
 * used by default by multiqueue devices.
 */
int netif_get_num_default_rss_queues(void)
{
	return min_t(int, DEFAULT_MAX_NUM_RSS_QUEUES, num_online_cpus());
}
EXPORT_SYMBOL(netif_get_num_default_rss_queues);

static inline void __netif_reschedule(struct Qdisc *q)
{
	struct softnet_data *sd;
	unsigned long flags;

	local_irq_save(flags);
	sd = this_cpu_ptr(&softnet_data);
	q->next_sched = NULL;
	*sd->output_queue_tailp = q;
	sd->output_queue_tailp = &q->next_sched;
	raise_softirq_irqoff(NET_TX_SOFTIRQ);
	local_irq_restore(flags);
}

void __netif_schedule(struct Qdisc *q)
{
	if (!test_and_set_bit(__QDISC_STATE_SCHED, &q->state))
		__netif_reschedule(q);
}
EXPORT_SYMBOL(__netif_schedule);

struct dev_kfree_skb_cb {
	enum skb_free_reason reason;
};

static struct dev_kfree_skb_cb *get_kfree_skb_cb(const struct sk_buff *skb)
{
	return (struct dev_kfree_skb_cb *)skb->cb;
}

void netif_schedule_queue(struct netdev_queue *txq)
{
	rcu_read_lock();
	if (!(txq->state & QUEUE_STATE_ANY_XOFF)) {
		struct Qdisc *q = rcu_dereference(txq->qdisc);

		__netif_schedule(q);
	}
	rcu_read_unlock();
}
EXPORT_SYMBOL(netif_schedule_queue);

/**
 *	netif_wake_subqueue - allow sending packets on subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Resume individual transmit queue of a device with multiple transmit queues.
 */
void netif_wake_subqueue(struct net_device *dev, u16 queue_index)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue_index);

	if (test_and_clear_bit(__QUEUE_STATE_DRV_XOFF, &txq->state)) {
		struct Qdisc *q;

		rcu_read_lock();
		q = rcu_dereference(txq->qdisc);
		__netif_schedule(q);
		rcu_read_unlock();
	}
}
EXPORT_SYMBOL(netif_wake_subqueue);

void netif_tx_wake_queue(struct netdev_queue *dev_queue)
{
	if (test_and_clear_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state)) {
		struct Qdisc *q;

		rcu_read_lock();
		q = rcu_dereference(dev_queue->qdisc);
		__netif_schedule(q);
		rcu_read_unlock();
	}
}
EXPORT_SYMBOL(netif_tx_wake_queue);

void __dev_kfree_skb_irq(struct sk_buff *skb, enum skb_free_reason reason)
{
	unsigned long flags;

	if (unlikely(!skb))
		return;

	if (likely(atomic_read(&skb->users) == 1)) {
		smp_rmb();
		atomic_set(&skb->users, 0);
	} else if (likely(!atomic_dec_and_test(&skb->users))) {
		return;
	}
	get_kfree_skb_cb(skb)->reason = reason;
	local_irq_save(flags);
	skb->next = __this_cpu_read(softnet_data.completion_queue);
	__this_cpu_write(softnet_data.completion_queue, skb);
	raise_softirq_irqoff(NET_TX_SOFTIRQ);
	local_irq_restore(flags);
}
EXPORT_SYMBOL(__dev_kfree_skb_irq);

#if defined(CONFIG_BCM_KF_SKB_DEFINES)
static struct task_struct *skb_free_task = NULL;
static struct sk_buff *skb_completion_queue = NULL;
static unsigned int skb_completion_queue_cnt = 0;
static DEFINE_SPINLOCK(skbfree_lock);

/* Setting value to WDF budget + some room for SKBs
 * freed by other threads */
#define MAX_SKB_FREE_BUDGET 256

/* the min number of skb to wake up free task */
static unsigned int skb_free_start_budget __read_mostly;

static int skb_free_thread_func(void *thread_data)
{
	unsigned int budget;
	struct sk_buff *skb;
	struct sk_buff *free_list = NULL;
	unsigned long flags;
	/* wake up periodically for every 20ms */
	unsigned timeout_jiffies = msecs_to_jiffies(20);

	while (!kthread_should_stop()) {
		budget = MAX_SKB_FREE_BUDGET;

update_list:
		spin_lock_irqsave(&skbfree_lock, flags);
		if (free_list == NULL) {
			if (skb_completion_queue) {
				free_list = skb_completion_queue;
				skb_completion_queue = NULL;
				skb_completion_queue_cnt = 0;
			}
		}
		spin_unlock_irqrestore(&skbfree_lock, flags);

		while (free_list && budget) {
			skb = free_list;
			free_list = free_list->next;
			__kfree_skb(skb);
			budget--;
		}

		if (free_list || skb_completion_queue) {
			if (budget)
				goto update_list;

			/* we still have packets in Q, reschedule the task */
			yield();
		} else {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(timeout_jiffies);
		}
	}
	return 0;
}

static ssize_t skb_free_thread_proc_rd_func(struct file *filep, char __user *page, size_t count, loff_t *offset)
{
	int len = 0;

	if( *offset != 0)
		return 0;

	len += sprintf(page, "skb_completion_queue_cnt %d \n", skb_completion_queue_cnt);

	*offset = len;

	return len;

} /* skb_free_thread_proc_rd_func */

static const struct file_operations skb_free_thread_stats_fops = {
       .owner  = THIS_MODULE,
       .read   = skb_free_thread_proc_rd_func,
};

static struct proc_dir_entry *skb_free_thread_proc_directory, *skb_free_thread_proc_file_conf;

static int skb_free_thread_proc_init(void)
{
    skb_free_thread_proc_directory = proc_mkdir("skb_free_thread", NULL) ;

    if (!skb_free_thread_proc_directory) goto fail_dir ;

    if ((skb_free_thread_proc_file_conf = proc_create("skb_free_thread/stats", 0644, NULL, &skb_free_thread_stats_fops)) == NULL) {
        goto fail_entry;
    }

    return (0) ;

fail_entry:
    printk("%s %s: Failed to create proc entry in skb_free_thread\n", __FILE__, __FUNCTION__);
    remove_proc_entry("skb_free_thread" ,NULL); /* remove already registered directory */

fail_dir:
    printk("%s %s: Failed to create directory skb_free_thread\n", __FILE__, __FUNCTION__) ;
    return (-EIO) ;
} /* skb_free_thread_proc_init */

#ifndef SZ_32M
#define SZ_32M		0x02000000
#endif
#ifndef SZ_64M
#define SZ_64M		0x04000000
#endif
#ifndef SZ_128M
#define SZ_128M		0x08000000
#endif
#ifndef SZ_256M
#define SZ_256M		0x10000000
#endif

struct task_struct *create_skb_free_task(void)
{
	struct task_struct *tsk;
	struct sched_param param;
	struct sysinfo sinfo;

	si_meminfo(&sinfo);

	if (sinfo.totalram <= (SZ_32M / sinfo.mem_unit))
		skb_free_start_budget = 16;
	else if (sinfo.totalram <= (SZ_64M / sinfo.mem_unit))
		skb_free_start_budget = 32;
	else if (sinfo.totalram <= (SZ_128M / sinfo.mem_unit))
		skb_free_start_budget = 64;
	else if (sinfo.totalram <= (SZ_256M / sinfo.mem_unit))
		skb_free_start_budget = 128;
	else
		skb_free_start_budget = 256;

	tsk = kthread_create(skb_free_thread_func, NULL, "skb_free_task");

	if (IS_ERR(tsk)) {
		printk("skb_free_task creation failed\n");
		return NULL;
	}

	/* Initialize the proc interface for debugging information */
    if (skb_free_thread_proc_init()!=0)
    {
        printk("\n%s %s: skb_free_thread_proc_init() failed\n", __FILE__, __FUNCTION__) ;
        return NULL;
    }

	param.sched_priority = BCM_RTPRIO_DATA_CONTROL;
	sched_setscheduler(tsk, SCHED_RR, &param);
	wake_up_process(tsk);

	printk("skb_free_task created successfully with start budget %d\n", skb_free_start_budget);
	return tsk;
}

/* queue the skb so it can be freed in thread context
 * note: this thread is not binded to any cpu,and we rely on scheduler to
 * run it on cpu with less load
 */
void dev_kfree_skb_thread(struct sk_buff *skb)
{
	unsigned long flags;

	if (atomic_dec_and_test(&skb->users)) {
		spin_lock_irqsave(&skbfree_lock, flags);
		skb->next = skb_completion_queue;
		skb_completion_queue = skb;
		skb_completion_queue_cnt++ ;
		spin_unlock_irqrestore(&skbfree_lock, flags);

		if ((skb_free_task->state != TASK_RUNNING) &&
				(skb_completion_queue_cnt >= skb_free_start_budget))
			wake_up_process(skb_free_task);
	}
}
EXPORT_SYMBOL(dev_kfree_skb_thread);

/* bulk queue the skb so it can be freed in thread context
 * note: this thread is not binded to any cpu,and we rely on scheduler to
 * run it on cpu with less load
 */
void dev_kfree_skb_thread_bulk(struct sk_buff *skb)
{
	unsigned long flags;
	struct sk_buff *skbfreelistend;
	unsigned int skbcnt = 1;

	/* locate last skb of the supplied skb list */
	skbfreelistend = skb;
	while (skbfreelistend->next != NULL) {
		skbfreelistend = skbfreelistend->next;
		/* +1 for first skb already done during init */
		skbcnt++;
	}

	if (atomic_dec_and_test(&skb->users)) {
		spin_lock_irqsave(&skbfree_lock, flags);
		skbfreelistend->next = skb_completion_queue;
		skb_completion_queue = skb;
		skb_completion_queue_cnt += skbcnt;
		spin_unlock_irqrestore(&skbfree_lock, flags);

		if ((skb_free_task->state != TASK_RUNNING) &&
				(skb_completion_queue_cnt >= skb_free_start_budget))
			wake_up_process(skb_free_task);
	}
}
EXPORT_SYMBOL(dev_kfree_skb_thread_bulk);

#include <linux/gbpm.h>
/* NOTE: gbpm_fap_evt_hook_g is not part of gbpm_g */
gbpm_fap_evt_hook_t gbpm_fap_evt_hook_g = (gbpm_fap_evt_hook_t)NULL;
EXPORT_SYMBOL(gbpm_fap_evt_hook_g);
static int fapdrv_thread_func(void *thread_data)
{
	while (!kthread_should_stop()) {
		static int scheduled = 0;
		local_bh_disable();

		if (!scheduled) {
			printk("gbpm_do_work scheduled\n");
			scheduled = 1;
		}
		if (gbpm_fap_evt_hook_g)
			gbpm_fap_evt_hook_g();

		local_bh_enable();

		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}
	return 0;
}

struct task_struct *fapDrvTask = NULL;
EXPORT_SYMBOL(fapDrvTask);
struct task_struct *create_fapDrvTask(void)
{
	struct sched_param param;
	/* Create FAP driver thread for dqm work handling */
	fapDrvTask = kthread_create(fapdrv_thread_func, NULL, "bcmFapDrv");

	if (IS_ERR(fapDrvTask)) {
		printk("fapDrvTask creation failed\n");
		return 0;
	}

	param.sched_priority = BCM_RTPRIO_DATA_CONTROL;
	sched_setscheduler(fapDrvTask, SCHED_RR, &param);

	wake_up_process(fapDrvTask);
	return fapDrvTask;
}

#endif

void __dev_kfree_skb_any(struct sk_buff *skb, enum skb_free_reason reason)
{
	if (in_irq() || irqs_disabled())
		__dev_kfree_skb_irq(skb, reason);
	else
		dev_kfree_skb(skb);
}
EXPORT_SYMBOL(__dev_kfree_skb_any);


/**
 * netif_device_detach - mark device as removed
 * @dev: network device
 *
 * Mark device as removed from system and therefore no longer available.
 */
void netif_device_detach(struct net_device *dev)
{
	if (test_and_clear_bit(__LINK_STATE_PRESENT, &dev->state) &&
	    netif_running(dev)) {
		netif_tx_stop_all_queues(dev);
	}
}
EXPORT_SYMBOL(netif_device_detach);

/**
 * netif_device_attach - mark device as attached
 * @dev: network device
 *
 * Mark device as attached from system and restart if needed.
 */
void netif_device_attach(struct net_device *dev)
{
	if (!test_and_set_bit(__LINK_STATE_PRESENT, &dev->state) &&
	    netif_running(dev)) {
		netif_tx_wake_all_queues(dev);
		__netdev_watchdog_up(dev);
	}
}
EXPORT_SYMBOL(netif_device_attach);

static void skb_warn_bad_offload(const struct sk_buff *skb)
{
#if defined(CONFIG_BCM_KF_DEBUGGING_DISABLED_FIX)
	static const netdev_features_t null_features __attribute__((unused)) = 0;
#else
	static const netdev_features_t null_features = 0;
#endif
	struct net_device *dev = skb->dev;
	const char *driver = "";

	if (!net_ratelimit())
		return;

	if (dev && dev->dev.parent)
		driver = dev_driver_string(dev->dev.parent);

	WARN(1, "%s: caps=(%pNF, %pNF) len=%d data_len=%d gso_size=%d "
	     "gso_type=%d ip_summed=%d\n",
	     driver, dev ? &dev->features : &null_features,
	     skb->sk ? &skb->sk->sk_route_caps : &null_features,
	     skb->len, skb->data_len, skb_shinfo(skb)->gso_size,
	     skb_shinfo(skb)->gso_type, skb->ip_summed);
}

/*
 * Invalidate hardware checksum when packet is to be mangled, and
 * complete checksum manually on outgoing path.
 */
int skb_checksum_help(struct sk_buff *skb)
{
	__wsum csum;
	int ret = 0, offset;

	if (skb->ip_summed == CHECKSUM_COMPLETE)
		goto out_set_summed;

	if (unlikely(skb_shinfo(skb)->gso_size)) {
		skb_warn_bad_offload(skb);
		return -EINVAL;
	}

	/* Before computing a checksum, we should make sure no frag could
	 * be modified by an external entity : checksum could be wrong.
	 */
	if (skb_has_shared_frag(skb)) {
		ret = __skb_linearize(skb);
		if (ret)
			goto out;
	}

	offset = skb_checksum_start_offset(skb);
	BUG_ON(offset >= skb_headlen(skb));
	csum = skb_checksum(skb, offset, skb->len - offset, 0);

	offset += skb->csum_offset;
	BUG_ON(offset + sizeof(__sum16) > skb_headlen(skb));

	if (skb_cloned(skb) &&
	    !skb_clone_writable(skb, offset + sizeof(__sum16))) {
		ret = pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
		if (ret)
			goto out;
	}

	*(__sum16 *)(skb->data + offset) = csum_fold(csum);
out_set_summed:
	skb->ip_summed = CHECKSUM_NONE;
out:
	return ret;
}
EXPORT_SYMBOL(skb_checksum_help);

__be16 skb_network_protocol(struct sk_buff *skb, int *depth)
{
	__be16 type = skb->protocol;

	/* Tunnel gso handlers can set protocol to ethernet. */
	if (type == htons(ETH_P_TEB)) {
		struct ethhdr *eth;

		if (unlikely(!pskb_may_pull(skb, sizeof(struct ethhdr))))
			return 0;

		eth = (struct ethhdr *)skb->data;
		type = eth->h_proto;
	}

	return __vlan_get_protocol(skb, type, depth);
}

/**
 *	skb_mac_gso_segment - mac layer segmentation handler.
 *	@skb: buffer to segment
 *	@features: features for the output path (see dev->features)
 */
struct sk_buff *skb_mac_gso_segment(struct sk_buff *skb,
				    netdev_features_t features)
{
	struct sk_buff *segs = ERR_PTR(-EPROTONOSUPPORT);
	struct packet_offload *ptype;
	int vlan_depth = skb->mac_len;
	__be16 type = skb_network_protocol(skb, &vlan_depth);

	if (unlikely(!type))
		return ERR_PTR(-EINVAL);

	__skb_pull(skb, vlan_depth);

	rcu_read_lock();
	list_for_each_entry_rcu(ptype, &offload_base, list) {
		if (ptype->type == type && ptype->callbacks.gso_segment) {
			segs = ptype->callbacks.gso_segment(skb, features);
			break;
		}
	}
	rcu_read_unlock();

	__skb_push(skb, skb->data - skb_mac_header(skb));

	return segs;
}
EXPORT_SYMBOL(skb_mac_gso_segment);


/* openvswitch calls this on rx path, so we need a different check.
 */
static inline bool skb_needs_check(struct sk_buff *skb, bool tx_path)
{
	if (tx_path)
		return skb->ip_summed != CHECKSUM_PARTIAL &&
		       skb->ip_summed != CHECKSUM_UNNECESSARY;

	return skb->ip_summed == CHECKSUM_NONE;
}

/**
 *	__skb_gso_segment - Perform segmentation on skb.
 *	@skb: buffer to segment
 *	@features: features for the output path (see dev->features)
 *	@tx_path: whether it is called in TX path
 *
 *	This function segments the given skb and returns a list of segments.
 *
 *	It may return NULL if the skb requires no segmentation.  This is
 *	only possible when GSO is used for verifying header integrity.
 *
 *	Segmentation preserves SKB_SGO_CB_OFFSET bytes of previous skb cb.
 */
struct sk_buff *__skb_gso_segment(struct sk_buff *skb,
				  netdev_features_t features, bool tx_path)
{
	struct sk_buff *segs;

	if (unlikely(skb_needs_check(skb, tx_path))) {
		int err;

		/* We're going to init ->check field in TCP or UDP header */
		err = skb_cow_head(skb, 0);
		if (err < 0)
			return ERR_PTR(err);
	}

	BUILD_BUG_ON(SKB_SGO_CB_OFFSET +
		     sizeof(*SKB_GSO_CB(skb)) > sizeof(skb->cb));

	SKB_GSO_CB(skb)->mac_offset = skb_headroom(skb);
	SKB_GSO_CB(skb)->encap_level = 0;

	skb_reset_mac_header(skb);
	skb_reset_mac_len(skb);

	segs = skb_mac_gso_segment(skb, features);

	if (unlikely(skb_needs_check(skb, tx_path) && !IS_ERR(segs)))
		skb_warn_bad_offload(skb);

	return segs;
}
EXPORT_SYMBOL(__skb_gso_segment);

/* Take action when hardware reception checksum errors are detected. */
#ifdef CONFIG_BUG
void netdev_rx_csum_fault(struct net_device *dev)
{
	if (net_ratelimit()) {
		pr_err("%s: hw csum failure\n", dev ? dev->name : "<unknown>");
		dump_stack();
	}
}
EXPORT_SYMBOL(netdev_rx_csum_fault);
#endif

/* Actually, we should eliminate this check as soon as we know, that:
 * 1. IOMMU is present and allows to map all the memory.
 * 2. No high memory really exists on this machine.
 */

static int illegal_highdma(struct net_device *dev, struct sk_buff *skb)
{
#ifdef CONFIG_HIGHMEM
	int i;
	if (!(dev->features & NETIF_F_HIGHDMA)) {
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
			if (PageHighMem(skb_frag_page(frag)))
				return 1;
		}
	}

	if (PCI_DMA_BUS_IS_PHYS) {
		struct device *pdev = dev->dev.parent;

		if (!pdev)
			return 0;
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
			dma_addr_t addr = page_to_phys(skb_frag_page(frag));
			if (!pdev->dma_mask || addr + PAGE_SIZE - 1 > *pdev->dma_mask)
				return 1;
		}
	}
#endif
	return 0;
}

/* If MPLS offload request, verify we are testing hardware MPLS features
 * instead of standard features for the netdev.
 */
#if IS_ENABLED(CONFIG_NET_MPLS_GSO)
static netdev_features_t net_mpls_features(struct sk_buff *skb,
					   netdev_features_t features,
					   __be16 type)
{
	if (eth_p_mpls(type))
		features &= skb->dev->mpls_features;

	return features;
}
#else
static netdev_features_t net_mpls_features(struct sk_buff *skb,
					   netdev_features_t features,
					   __be16 type)
{
	return features;
}
#endif

static netdev_features_t harmonize_features(struct sk_buff *skb,
	netdev_features_t features)
{
	int tmp;
	__be16 type;

	type = skb_network_protocol(skb, &tmp);
	features = net_mpls_features(skb, features, type);

	if (skb->ip_summed != CHECKSUM_NONE &&
	    !can_checksum_protocol(features, type)) {
		features &= ~NETIF_F_ALL_CSUM;
	} else if (illegal_highdma(skb->dev, skb)) {
		features &= ~NETIF_F_SG;
	}

	return features;
}

netdev_features_t passthru_features_check(struct sk_buff *skb,
					  struct net_device *dev,
					  netdev_features_t features)
{
	return features;
}
EXPORT_SYMBOL(passthru_features_check);

static netdev_features_t dflt_features_check(struct sk_buff *skb,
					     struct net_device *dev,
					     netdev_features_t features)
{
	return vlan_features_check(skb, features);
}

netdev_features_t netif_skb_features(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	netdev_features_t features = dev->features;
	u16 gso_segs = skb_shinfo(skb)->gso_segs;

	if (gso_segs > dev->gso_max_segs || gso_segs < dev->gso_min_segs)
		features &= ~NETIF_F_GSO_MASK;

	/* If encapsulation offload request, verify we are testing
	 * hardware encapsulation features instead of standard
	 * features for the netdev
	 */
	if (skb->encapsulation)
		features &= dev->hw_enc_features;

	if (skb_vlan_tagged(skb))
		features = netdev_intersect_features(features,
						     dev->vlan_features |
						     NETIF_F_HW_VLAN_CTAG_TX |
						     NETIF_F_HW_VLAN_STAG_TX);

	if (dev->netdev_ops->ndo_features_check)
		features &= dev->netdev_ops->ndo_features_check(skb, dev,
								features);
	else
		features &= dflt_features_check(skb, dev, features);

	return harmonize_features(skb, features);
}
EXPORT_SYMBOL(netif_skb_features);

static int xmit_one(struct sk_buff *skb, struct net_device *dev,
		    struct netdev_queue *txq, bool more)
{
	unsigned int len;
	int rc;
#if (defined(CONFIG_BCM_KF_FAP_GSO_LOOPBACK) && defined(CONFIG_BCM_FAP_GSO_LOOPBACK))
	unsigned int devId = bcm_is_gso_loopback_dev(dev);
#endif

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	if ((!list_empty(&ptype_all) || !list_empty(&dev->ptype_all)) && !(skb->imq_flags & IMQ_F_ENQUEUE))
#else
	if (!list_empty(&ptype_all) || !list_empty(&dev->ptype_all))
#endif
		dev_queue_xmit_nit(skb, dev);

	len = skb->len;
	trace_net_dev_start_xmit(skb, dev);
#if (defined(CONFIG_BCM_KF_FAP_GSO_LOOPBACK) && defined(CONFIG_BCM_FAP_GSO_LOOPBACK))
	if (devId && bcm_gso_loopback_hw_offload) {
		if (skb_shinfo(skb)->nr_frags ||
		    skb_is_gso(skb) ||
		    (skb->ip_summed == CHECKSUM_PARTIAL)) {
			skb->xmit_more = more ? 1 : 0;
			rc = bcm_gso_loopback_hw_offload(skb, devId);
			if (rc == NETDEV_TX_OK)
				txq_trans_update(txq);
		} else if (!skb->recycle_hook) {
			/* To avoid any outof order packets, send all the
			 * locally generated packets through gso loop back
			 */

			/* TODO: we are classifying the traffic as local based
			 * on recycle hook.  But cloned forwarding traffic can
			 * also have recyle_hook as NULL, so this traffic will
			 * make an extra trip through FAP unnecessarily.
			 * But we don't expecet a lot of traffic in this case,
			 * so this shoud be okay for now. Later add a flag in
			 * skb and mark the skb as local in local_out hook.
			 */
			skb->xmit_more = more ? 1 : 0;
			rc = bcm_gso_loopback_hw_offload(skb, devId);
			if (rc == NETDEV_TX_OK)
				txq_trans_update(txq);
		} else
			rc = netdev_start_xmit(skb, dev, txq, more);
	} else
		rc = netdev_start_xmit(skb, dev, txq, more);
#else
	rc = netdev_start_xmit(skb, dev, txq, more);
#endif
	trace_net_dev_xmit(skb, rc, dev, len);

	return rc;
}

struct sk_buff *dev_hard_start_xmit(struct sk_buff *first, struct net_device *dev,
				    struct netdev_queue *txq, int *ret)
{
	struct sk_buff *skb = first;
	int rc = NETDEV_TX_OK;

	while (skb) {
		struct sk_buff *next = skb->next;

		skb->next = NULL;
		rc = xmit_one(skb, dev, txq, next != NULL);
		if (unlikely(!dev_xmit_complete(rc))) {
			skb->next = next;
			goto out;
		}

		skb = next;
		if (netif_xmit_stopped(txq) && skb) {
			rc = NETDEV_TX_BUSY;
			break;
		}
	}

out:
	*ret = rc;
	return skb;
}
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
EXPORT_SYMBOL(dev_hard_start_xmit);
#endif

static struct sk_buff *validate_xmit_vlan(struct sk_buff *skb,
					  netdev_features_t features)
{
	if (skb_vlan_tag_present(skb) &&
	    !vlan_hw_offload_capable(features, skb->vlan_proto))
		skb = __vlan_hwaccel_push_inside(skb);
	return skb;
}

static struct sk_buff *validate_xmit_skb(struct sk_buff *skb, struct net_device *dev)
{
	netdev_features_t features;

	if (skb->next)
		return skb;

	features = netif_skb_features(skb);
	skb = validate_xmit_vlan(skb, features);
	if (unlikely(!skb))
		goto out_null;

	if (netif_needs_gso(skb, features)) {
		struct sk_buff *segs;

		segs = skb_gso_segment(skb, features);
		if (IS_ERR(segs)) {
			goto out_kfree_skb;
		} else if (segs) {
			consume_skb(skb);
			skb = segs;
		}
	} else {
		if (skb_needs_linearize(skb, features) &&
		    __skb_linearize(skb))
			goto out_kfree_skb;

		/* If packet is not checksummed and device does not
		 * support checksumming for this protocol, complete
		 * checksumming here.
		 */
		if (skb->ip_summed == CHECKSUM_PARTIAL) {
			if (skb->encapsulation)
				skb_set_inner_transport_header(skb,
							       skb_checksum_start_offset(skb));
			else
				skb_set_transport_header(skb,
							 skb_checksum_start_offset(skb));
			if (!(features & NETIF_F_ALL_CSUM) &&
			    skb_checksum_help(skb))
				goto out_kfree_skb;
		}
	}

	return skb;

out_kfree_skb:
	kfree_skb(skb);
out_null:
	return NULL;
}

struct sk_buff *validate_xmit_skb_list(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *next, *head = NULL, *tail;

	for (; skb != NULL; skb = next) {
		next = skb->next;
		skb->next = NULL;

		/* in case skb wont be segmented, point to itself */
		skb->prev = skb;

		skb = validate_xmit_skb(skb, dev);
		if (!skb)
			continue;

		if (!head)
			head = skb;
		else
			tail->next = skb;
		/* If skb was segmented, skb->prev points to
		 * the last segment. If not, it still contains skb.
		 */
		tail = skb->prev;
	}
	return head;
}
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
EXPORT_SYMBOL(validate_xmit_skb_list);
#endif

static void qdisc_pkt_len_init(struct sk_buff *skb)
{
	const struct skb_shared_info *shinfo = skb_shinfo(skb);

	qdisc_skb_cb(skb)->pkt_len = skb->len;

	/* To get more precise estimation of bytes sent on wire,
	 * we add to pkt_len the headers size of all segments
	 */
	if (shinfo->gso_size)  {
		unsigned int hdr_len;
		u16 gso_segs = shinfo->gso_segs;

		/* mac layer + network layer */
		hdr_len = skb_transport_header(skb) - skb_mac_header(skb);

		/* + transport layer */
		if (likely(shinfo->gso_type & (SKB_GSO_TCPV4 | SKB_GSO_TCPV6))) {
			const struct tcphdr *th;
			struct tcphdr _tcphdr;

			th = skb_header_pointer(skb, skb_transport_offset(skb),
						sizeof(_tcphdr), &_tcphdr);
			if (likely(th))
				hdr_len += __tcp_hdrlen(th);
		} else {
			struct udphdr _udphdr;

			if (skb_header_pointer(skb, skb_transport_offset(skb),
					       sizeof(_udphdr), &_udphdr))
				hdr_len += sizeof(struct udphdr);
		}

		if (shinfo->gso_type & SKB_GSO_DODGY)
			gso_segs = DIV_ROUND_UP(skb->len - hdr_len,
						shinfo->gso_size);

		qdisc_skb_cb(skb)->pkt_len += (gso_segs - 1) * hdr_len;
	}
}

static inline int __dev_xmit_skb(struct sk_buff *skb, struct Qdisc *q,
				 struct net_device *dev,
				 struct netdev_queue *txq)
{
	spinlock_t *root_lock = qdisc_lock(q);
	bool contended;
	int rc;

	qdisc_pkt_len_init(skb);
	qdisc_calculate_pkt_len(skb, q);
	/*
	 * Heuristic to force contended enqueues to serialize on a
	 * separate lock before trying to get qdisc main lock.
	 * This permits __QDISC___STATE_RUNNING owner to get the lock more
	 * often and dequeue packets faster.
	 */
	contended = qdisc_is_running(q);
	if (unlikely(contended))
		spin_lock(&q->busylock);

	spin_lock(root_lock);
	if (unlikely(test_bit(__QDISC_STATE_DEACTIVATED, &q->state))) {
		kfree_skb(skb);
		rc = NET_XMIT_DROP;
	} else if ((q->flags & TCQ_F_CAN_BYPASS) && !qdisc_qlen(q) &&
		   qdisc_run_begin(q)) {
		/*
		 * This is a work-conserving queue; there are no old skbs
		 * waiting to be sent out; and the qdisc is not running -
		 * xmit the skb directly.
		 */

		qdisc_bstats_update(q, skb);

		if (sch_direct_xmit(skb, q, dev, txq, root_lock, true)) {
			if (unlikely(contended)) {
				spin_unlock(&q->busylock);
				contended = false;
			}
			__qdisc_run(q);
		} else
			qdisc_run_end(q);

		rc = NET_XMIT_SUCCESS;
	} else {
		rc = q->enqueue(skb, q) & NET_XMIT_MASK;
		if (qdisc_run_begin(q)) {
			if (unlikely(contended)) {
				spin_unlock(&q->busylock);
				contended = false;
			}
			__qdisc_run(q);
		}
	}
	spin_unlock(root_lock);
	if (unlikely(contended))
		spin_unlock(&q->busylock);
	return rc;
}

#if IS_ENABLED(CONFIG_CGROUP_NET_PRIO)
static void skb_update_prio(struct sk_buff *skb)
{
	struct netprio_map *map = rcu_dereference_bh(skb->dev->priomap);

	if (!skb->priority && skb->sk && map) {
		unsigned int prioidx = skb->sk->sk_cgrp_prioidx;

		if (prioidx < map->priomap_len)
			skb->priority = map->priomap[prioidx];
	}
}
#else
#define skb_update_prio(skb)
#endif

DEFINE_PER_CPU(int, xmit_recursion);
EXPORT_SYMBOL(xmit_recursion);

#define RECURSION_LIMIT 10

/**
 *	dev_loopback_xmit - loop back @skb
 *	@skb: buffer to transmit
 */
int dev_loopback_xmit(struct sock *sk, struct sk_buff *skb)
{
	skb_reset_mac_header(skb);
	__skb_pull(skb, skb_network_offset(skb));
	skb->pkt_type = PACKET_LOOPBACK;
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	WARN_ON(!skb_dst(skb));
	skb_dst_force(skb);
	netif_rx_ni(skb);
	return 0;
}
EXPORT_SYMBOL(dev_loopback_xmit);

/**
 *	__dev_queue_xmit - transmit a buffer
 *	@skb: buffer to transmit
 *	@accel_priv: private data used for L2 forwarding offload
 *
 *	Queue a buffer for transmission to a network device. The caller must
 *	have set the device and priority and built the buffer before calling
 *	this function. The function can be called from an interrupt.
 *
 *	A negative errno code is returned on a failure. A success does not
 *	guarantee the frame will be transmitted as it may be dropped due
 *	to congestion or traffic shaping.
 *
 * -----------------------------------------------------------------------------------
 *      I notice this method can also return errors from the queue disciplines,
 *      including NET_XMIT_DROP, which is a positive value.  So, errors can also
 *      be positive.
 *
 *      Regardless of the return value, the skb is consumed, so it is currently
 *      difficult to retry a send to this method.  (You can bump the ref count
 *      before sending to hold a reference for retry if you are careful.)
 *
 *      When calling this method, interrupts MUST be enabled.  This is because
 *      the BH enable code must have IRQs enabled so that it will not deadlock.
 *          --BLG
 */
static int __dev_queue_xmit(struct sk_buff *skb, void *accel_priv)
{
	struct net_device *dev = skb->dev;
	struct netdev_queue *txq;
	struct Qdisc *q;
	int rc = -ENOMEM;

	skb_reset_mac_header(skb);

	if (unlikely(skb_shinfo(skb)->tx_flags & SKBTX_SCHED_TSTAMP))
		__skb_tstamp_tx(skb, NULL, skb->sk, SCM_TSTAMP_SCHED);

	/* Disable soft irqs for various locks below. Also
	 * stops preemption for RCU.
	 */
	rcu_read_lock_bh();

	skb_update_prio(skb);

	/* If device/qdisc don't need skb->dst, release it right now while
	 * its hot in this cpu cache.
	 */
	if (dev->priv_flags & IFF_XMIT_DST_RELEASE)
		skb_dst_drop(skb);
	else
		skb_dst_force(skb);

	txq = netdev_pick_tx(dev, skb, accel_priv);
	q = rcu_dereference_bh(txq->qdisc);

#ifdef CONFIG_NET_CLS_ACT
	skb->tc_verd = SET_TC_AT(skb->tc_verd, AT_EGRESS);
#endif
	trace_net_dev_queue(skb);
	if (q->enqueue) {
		rc = __dev_xmit_skb(skb, q, dev, txq);
		goto out;
	}

	/* The device has no queue. Common case for software devices:
	   loopback, all the sorts of tunnels...

	   Really, it is unlikely that netif_tx_lock protection is necessary
	   here.  (f.e. loopback and IP tunnels are clean ignoring statistics
	   counters.)
	   However, it is possible, that they rely on protection
	   made by us here.

	   Check this and shot the lock. It is not prone from deadlocks.
	   Either shot noqueue qdisc, it is even simpler 8)
	 */
	if (dev->flags & IFF_UP) {
		int cpu = smp_processor_id(); /* ok because BHs are off */

		if (txq->xmit_lock_owner != cpu) {

			if (__this_cpu_read(xmit_recursion) > RECURSION_LIMIT)
				goto recursion_alert;

			skb = validate_xmit_skb(skb, dev);
			if (!skb)
				goto drop;

			HARD_TX_LOCK(dev, txq, cpu);

			if (!netif_xmit_stopped(txq)) {
				__this_cpu_inc(xmit_recursion);
				skb = dev_hard_start_xmit(skb, dev, txq, &rc);
				__this_cpu_dec(xmit_recursion);
				if (dev_xmit_complete(rc)) {
					HARD_TX_UNLOCK(dev, txq);
					goto out;
				}
			}
			HARD_TX_UNLOCK(dev, txq);
			net_crit_ratelimited("Virtual device %s asks to queue packet!\n",
					     dev->name);
		} else {
			/* Recursion is detected! It is possible,
			 * unfortunately
			 */
recursion_alert:
			net_crit_ratelimited("Dead loop on virtual device %s, fix it urgently!\n",
					     dev->name);
		}
	}

	rc = -ENETDOWN;
drop:
	rcu_read_unlock_bh();

	atomic_long_inc(&dev->tx_dropped);
	kfree_skb_list(skb);
	return rc;
out:
	rcu_read_unlock_bh();
	return rc;
}

int dev_queue_xmit_sk(struct sock *sk, struct sk_buff *skb)
{
	return __dev_queue_xmit(skb, NULL);
}
EXPORT_SYMBOL(dev_queue_xmit_sk);

int dev_queue_xmit_accel(struct sk_buff *skb, void *accel_priv)
{
	return __dev_queue_xmit(skb, accel_priv);
}
EXPORT_SYMBOL(dev_queue_xmit_accel);


/*=======================================================================
			Receiver routines
  =======================================================================*/

int netdev_max_backlog __read_mostly = 1000;
EXPORT_SYMBOL(netdev_max_backlog);

int netdev_tstamp_prequeue __read_mostly = 1;
int netdev_budget __read_mostly = 300;
int weight_p __read_mostly = 64;            /* old backlog weight */

/* Called with irq disabled */
static inline void ____napi_schedule(struct softnet_data *sd,
				     struct napi_struct *napi)
{
	list_add_tail(&napi->poll_list, &sd->poll_list);
	__raise_softirq_irqoff(NET_RX_SOFTIRQ);
}

#ifdef CONFIG_RPS

/* One global table that all flow-based protocols share. */
struct rps_sock_flow_table __rcu *rps_sock_flow_table __read_mostly;
EXPORT_SYMBOL(rps_sock_flow_table);
u32 rps_cpu_mask __read_mostly;
EXPORT_SYMBOL(rps_cpu_mask);

struct static_key rps_needed __read_mostly;

static struct rps_dev_flow *
set_rps_cpu(struct net_device *dev, struct sk_buff *skb,
	    struct rps_dev_flow *rflow, u16 next_cpu)
{
	if (next_cpu < nr_cpu_ids) {
#ifdef CONFIG_RFS_ACCEL
		struct netdev_rx_queue *rxqueue;
		struct rps_dev_flow_table *flow_table;
		struct rps_dev_flow *old_rflow;
		u32 flow_id;
		u16 rxq_index;
		int rc;

		/* Should we steer this flow to a different hardware queue? */
		if (!skb_rx_queue_recorded(skb) || !dev->rx_cpu_rmap ||
		    !(dev->features & NETIF_F_NTUPLE))
			goto out;
		rxq_index = cpu_rmap_lookup_index(dev->rx_cpu_rmap, next_cpu);
		if (rxq_index == skb_get_rx_queue(skb))
			goto out;

		rxqueue = dev->_rx + rxq_index;
		flow_table = rcu_dereference(rxqueue->rps_flow_table);
		if (!flow_table)
			goto out;
		flow_id = skb_get_hash(skb) & flow_table->mask;
		rc = dev->netdev_ops->ndo_rx_flow_steer(dev, skb,
							rxq_index, flow_id);
		if (rc < 0)
			goto out;
		old_rflow = rflow;
		rflow = &flow_table->flows[flow_id];
		rflow->filter = rc;
		if (old_rflow->filter == rflow->filter)
			old_rflow->filter = RPS_NO_FILTER;
	out:
#endif
		rflow->last_qtail =
			per_cpu(softnet_data, next_cpu).input_queue_head;
	}

	rflow->cpu = next_cpu;
	return rflow;
}

/*
 * get_rps_cpu is called from netif_receive_skb and returns the target
 * CPU from the RPS map of the receiving queue for a given skb.
 * rcu_read_lock must be held on entry.
 */
static int get_rps_cpu(struct net_device *dev, struct sk_buff *skb,
		       struct rps_dev_flow **rflowp)
{
	const struct rps_sock_flow_table *sock_flow_table;
	struct netdev_rx_queue *rxqueue = dev->_rx;
	struct rps_dev_flow_table *flow_table;
	struct rps_map *map;
	int cpu = -1;
	u32 tcpu;
	u32 hash;

	if (skb_rx_queue_recorded(skb)) {
		u16 index = skb_get_rx_queue(skb);

		if (unlikely(index >= dev->real_num_rx_queues)) {
			WARN_ONCE(dev->real_num_rx_queues > 1,
				  "%s received packet on queue %u, but number "
				  "of RX queues is %u\n",
				  dev->name, index, dev->real_num_rx_queues);
			goto done;
		}
		rxqueue += index;
	}

	/* Avoid computing hash if RFS/RPS is not active for this rxqueue */

	flow_table = rcu_dereference(rxqueue->rps_flow_table);
	map = rcu_dereference(rxqueue->rps_map);
	if (!flow_table && !map)
		goto done;

	skb_reset_network_header(skb);
	hash = skb_get_hash(skb);
	if (!hash)
		goto done;

	sock_flow_table = rcu_dereference(rps_sock_flow_table);
	if (flow_table && sock_flow_table) {
		struct rps_dev_flow *rflow;
		u32 next_cpu;
		u32 ident;

		/* First check into global flow table if there is a match */
		ident = sock_flow_table->ents[hash & sock_flow_table->mask];
		if ((ident ^ hash) & ~rps_cpu_mask)
			goto try_rps;

		next_cpu = ident & rps_cpu_mask;

		/* OK, now we know there is a match,
		 * we can look at the local (per receive queue) flow table
		 */
		rflow = &flow_table->flows[hash & flow_table->mask];
		tcpu = rflow->cpu;

		/*
		 * If the desired CPU (where last recvmsg was done) is
		 * different from current CPU (one in the rx-queue flow
		 * table entry), switch if one of the following holds:
		 *   - Current CPU is unset (>= nr_cpu_ids).
		 *   - Current CPU is offline.
		 *   - The current CPU's queue tail has advanced beyond the
		 *     last packet that was enqueued using this table entry.
		 *     This guarantees that all previous packets for the flow
		 *     have been dequeued, thus preserving in order delivery.
		 */
		if (unlikely(tcpu != next_cpu) &&
		    (tcpu >= nr_cpu_ids || !cpu_online(tcpu) ||
		     ((int)(per_cpu(softnet_data, tcpu).input_queue_head -
		      rflow->last_qtail)) >= 0)) {
			tcpu = next_cpu;
			rflow = set_rps_cpu(dev, skb, rflow, next_cpu);
		}

		if (tcpu < nr_cpu_ids && cpu_online(tcpu)) {
			*rflowp = rflow;
			cpu = tcpu;
			goto done;
		}
	}

try_rps:

	if (map) {
		tcpu = map->cpus[reciprocal_scale(hash, map->len)];
		if (cpu_online(tcpu)) {
			cpu = tcpu;
			goto done;
		}
	}

done:
	return cpu;
}

#ifdef CONFIG_RFS_ACCEL

/**
 * rps_may_expire_flow - check whether an RFS hardware filter may be removed
 * @dev: Device on which the filter was set
 * @rxq_index: RX queue index
 * @flow_id: Flow ID passed to ndo_rx_flow_steer()
 * @filter_id: Filter ID returned by ndo_rx_flow_steer()
 *
 * Drivers that implement ndo_rx_flow_steer() should periodically call
 * this function for each installed filter and remove the filters for
 * which it returns %true.
 */
bool rps_may_expire_flow(struct net_device *dev, u16 rxq_index,
			 u32 flow_id, u16 filter_id)
{
	struct netdev_rx_queue *rxqueue = dev->_rx + rxq_index;
	struct rps_dev_flow_table *flow_table;
	struct rps_dev_flow *rflow;
	bool expire = true;
	unsigned int cpu;

	rcu_read_lock();
	flow_table = rcu_dereference(rxqueue->rps_flow_table);
	if (flow_table && flow_id <= flow_table->mask) {
		rflow = &flow_table->flows[flow_id];
		cpu = ACCESS_ONCE(rflow->cpu);
		if (rflow->filter == filter_id && cpu < nr_cpu_ids &&
		    ((int)(per_cpu(softnet_data, cpu).input_queue_head -
			   rflow->last_qtail) <
		     (int)(10 * flow_table->mask)))
			expire = false;
	}
	rcu_read_unlock();
	return expire;
}
EXPORT_SYMBOL(rps_may_expire_flow);

#endif /* CONFIG_RFS_ACCEL */

/* Called from hardirq (IPI) context */
static void rps_trigger_softirq(void *data)
{
	struct softnet_data *sd = data;

	____napi_schedule(sd, &sd->backlog);
	sd->received_rps++;
}

#endif /* CONFIG_RPS */

/*
 * Check if this softnet_data structure is another cpu one
 * If yes, queue it to our IPI list and return 1
 * If no, return 0
 */
static int rps_ipi_queued(struct softnet_data *sd)
{
#ifdef CONFIG_RPS
	struct softnet_data *mysd = this_cpu_ptr(&softnet_data);

	if (sd != mysd) {
		sd->rps_ipi_next = mysd->rps_ipi_list;
		mysd->rps_ipi_list = sd;

		__raise_softirq_irqoff(NET_RX_SOFTIRQ);
		return 1;
	}
#endif /* CONFIG_RPS */
	return 0;
}

#ifdef CONFIG_NET_FLOW_LIMIT
int netdev_flow_limit_table_len __read_mostly = (1 << 12);
#endif

static bool skb_flow_limit(struct sk_buff *skb, unsigned int qlen)
{
#ifdef CONFIG_NET_FLOW_LIMIT
	struct sd_flow_limit *fl;
	struct softnet_data *sd;
	unsigned int old_flow, new_flow;

	if (qlen < (netdev_max_backlog >> 1))
		return false;

	sd = this_cpu_ptr(&softnet_data);

	rcu_read_lock();
	fl = rcu_dereference(sd->flow_limit);
	if (fl) {
		new_flow = skb_get_hash(skb) & (fl->num_buckets - 1);
		old_flow = fl->history[fl->history_head];
		fl->history[fl->history_head] = new_flow;

		fl->history_head++;
		fl->history_head &= FLOW_LIMIT_HISTORY - 1;

		if (likely(fl->buckets[old_flow]))
			fl->buckets[old_flow]--;

		if (++fl->buckets[new_flow] > (FLOW_LIMIT_HISTORY >> 1)) {
			fl->count++;
			rcu_read_unlock();
			return true;
		}
	}
	rcu_read_unlock();
#endif
	return false;
}

/*
 * enqueue_to_backlog is called to queue an skb to a per CPU backlog
 * queue (may be a remote CPU queue).
 */
static int enqueue_to_backlog(struct sk_buff *skb, int cpu,
			      unsigned int *qtail)
{
	struct softnet_data *sd;
	unsigned long flags;
	unsigned int qlen;

	sd = &per_cpu(softnet_data, cpu);

	local_irq_save(flags);

	rps_lock(sd);
	if (!netif_running(skb->dev))
		goto drop;
	qlen = skb_queue_len(&sd->input_pkt_queue);
	if (qlen <= netdev_max_backlog && !skb_flow_limit(skb, qlen)) {
		if (qlen) {
enqueue:
			__skb_queue_tail(&sd->input_pkt_queue, skb);
			input_queue_tail_incr_save(sd, qtail);
			rps_unlock(sd);
			local_irq_restore(flags);
			return NET_RX_SUCCESS;
		}

		/* Schedule NAPI for backlog device
		 * We can use non atomic operation since we own the queue lock
		 */
		if (!__test_and_set_bit(NAPI_STATE_SCHED, &sd->backlog.state)) {
			if (!rps_ipi_queued(sd))
				____napi_schedule(sd, &sd->backlog);
		}
		goto enqueue;
	}

drop:
	sd->dropped++;
	rps_unlock(sd);

	local_irq_restore(flags);

	atomic_long_inc(&skb->dev->rx_dropped);
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int netif_rx_internal(struct sk_buff *skb)
{
	int ret;

	net_timestamp_check(netdev_tstamp_prequeue, skb);
#ifdef IQOS_DUMMY_FN	
	if (enet_fwdcb_registered)
		blog_skip(skb, blog_skip_reason_dpi);
#endif
#if defined(CONFIG_BCM_KF_WANDEV)
	/* mark IFFWAN flag in skb based on dev->priv_flags */
	if (skb->dev) {
		unsigned int mark = skb->mark;
		skb->mark |= SKBMARK_SET_IFFWAN_MARK(mark, ((skb->dev->priv_flags & IFF_WANDEV) ? 1:0));
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG_FEATURE)
		if (skb->blog_p)
			skb->blog_p->isWan = (skb->dev->priv_flags & IFF_WANDEV) ? 1 : 0;
#endif /* defined(CONFIG_BLOG_FEATURE) */
	}
#endif /* CONFIG_BCM_KF_WANDEV */
	trace_netif_rx(skb);
#ifdef CONFIG_RPS
	if (static_key_false(&rps_needed)) {
		struct rps_dev_flow voidflow, *rflow = &voidflow;
		int cpu;

		preempt_disable();
		rcu_read_lock();

		cpu = get_rps_cpu(skb->dev, skb, &rflow);
		if (cpu < 0)
			cpu = smp_processor_id();

		ret = enqueue_to_backlog(skb, cpu, &rflow->last_qtail);

		rcu_read_unlock();
		preempt_enable();
	} else
#endif
	{
		unsigned int qtail;
		ret = enqueue_to_backlog(skb, get_cpu(), &qtail);
		put_cpu();
	}
	return ret;
}

/**
 *	netif_rx	-	post buffer to the network code
 *	@skb: buffer to post
 *
 *	This function receives a packet from a device driver and queues it for
 *	the upper (protocol) levels to process.  It always succeeds. The buffer
 *	may be dropped during processing for congestion control or by the
 *	protocol layers.
 *
 *	return values:
 *	NET_RX_SUCCESS	(no congestion)
 *	NET_RX_DROP     (packet was dropped)
 *
 */

int netif_rx(struct sk_buff *skb)
{
	trace_netif_rx_entry(skb);

	return netif_rx_internal(skb);
}
EXPORT_SYMBOL(netif_rx);

int netif_rx_ni(struct sk_buff *skb)
{
	int err;

	trace_netif_rx_ni_entry(skb);

	preempt_disable();
	err = netif_rx_internal(skb);
	if (local_softirq_pending())
		do_softirq();
	preempt_enable();

	return err;
}
EXPORT_SYMBOL(netif_rx_ni);

static void net_tx_action(struct softirq_action *h)
{
	struct softnet_data *sd = this_cpu_ptr(&softnet_data);

	if (sd->completion_queue) {
		struct sk_buff *clist;

		local_irq_disable();
		clist = sd->completion_queue;
		sd->completion_queue = NULL;
		local_irq_enable();

		while (clist) {
			struct sk_buff *skb = clist;
			clist = clist->next;

			WARN_ON(atomic_read(&skb->users));
			if (likely(get_kfree_skb_cb(skb)->reason == SKB_REASON_CONSUMED))
				trace_consume_skb(skb);
			else
				trace_kfree_skb(skb, net_tx_action);
			__kfree_skb(skb);
		}
	}

	if (sd->output_queue) {
		struct Qdisc *head;

		local_irq_disable();
		head = sd->output_queue;
		sd->output_queue = NULL;
		sd->output_queue_tailp = &sd->output_queue;
		local_irq_enable();

		while (head) {
			struct Qdisc *q = head;
			spinlock_t *root_lock;

			head = head->next_sched;

			root_lock = qdisc_lock(q);
			if (spin_trylock(root_lock)) {
				smp_mb__before_atomic();
				clear_bit(__QDISC_STATE_SCHED,
					  &q->state);
				qdisc_run(q);
				spin_unlock(root_lock);
			} else {
				if (!test_bit(__QDISC_STATE_DEACTIVATED,
					      &q->state)) {
					__netif_reschedule(q);
				} else {
					smp_mb__before_atomic();
					clear_bit(__QDISC_STATE_SCHED,
						  &q->state);
				}
			}
		}
	}
}

#if (defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)) && \
    (defined(CONFIG_ATM_LANE) || defined(CONFIG_ATM_LANE_MODULE))
/* This hook is defined here for ATM LANE */
int (*br_fdb_test_addr_hook)(struct net_device *dev,
			     unsigned char *addr) __read_mostly;
EXPORT_SYMBOL_GPL(br_fdb_test_addr_hook);
#endif

#ifdef CONFIG_NET_CLS_ACT
/* TODO: Maybe we should just force sch_ingress to be compiled in
 * when CONFIG_NET_CLS_ACT is? otherwise some useless instructions
 * a compare and 2 stores extra right now if we dont have it on
 * but have CONFIG_NET_CLS_ACT
 * NOTE: This doesn't stop any functionality; if you dont have
 * the ingress scheduler, you just can't add policies on ingress.
 *
 */
static int ing_filter(struct sk_buff *skb, struct netdev_queue *rxq)
{
	struct net_device *dev = skb->dev;
	u32 ttl = G_TC_RTTL(skb->tc_verd);
	int result = TC_ACT_OK;
	struct Qdisc *q;

	if (unlikely(MAX_RED_LOOP < ttl++)) {
		net_warn_ratelimited("Redir loop detected Dropping packet (%d->%d)\n",
				     skb->skb_iif, dev->ifindex);
		return TC_ACT_SHOT;
	}

	skb->tc_verd = SET_TC_RTTL(skb->tc_verd, ttl);
	skb->tc_verd = SET_TC_AT(skb->tc_verd, AT_INGRESS);

	q = rcu_dereference(rxq->qdisc);
	if (q != &noop_qdisc) {
		spin_lock(qdisc_lock(q));
		if (likely(!test_bit(__QDISC_STATE_DEACTIVATED, &q->state)))
			result = qdisc_enqueue_root(skb, q);
		spin_unlock(qdisc_lock(q));
	}

	return result;
}

static inline struct sk_buff *handle_ing(struct sk_buff *skb,
					 struct packet_type **pt_prev,
					 int *ret, struct net_device *orig_dev)
{
	struct netdev_queue *rxq = rcu_dereference(skb->dev->ingress_queue);

	if (!rxq || rcu_access_pointer(rxq->qdisc) == &noop_qdisc)
		return skb;

	if (*pt_prev) {
		*ret = deliver_skb(skb, *pt_prev, orig_dev);
		*pt_prev = NULL;
	}

	switch (ing_filter(skb, rxq)) {
	case TC_ACT_SHOT:
	case TC_ACT_STOLEN:
		kfree_skb(skb);
		return NULL;
	}

	return skb;
}
#endif

/**
 *	netdev_rx_handler_register - register receive handler
 *	@dev: device to register a handler for
 *	@rx_handler: receive handler to register
 *	@rx_handler_data: data pointer that is used by rx handler
 *
 *	Register a receive handler for a device. This handler will then be
 *	called from __netif_receive_skb. A negative errno code is returned
 *	on a failure.
 *
 *	The caller must hold the rtnl_mutex.
 *
 *	For a general description of rx_handler, see enum rx_handler_result.
 */
int netdev_rx_handler_register(struct net_device *dev,
			       rx_handler_func_t *rx_handler,
			       void *rx_handler_data)
{
	ASSERT_RTNL();

	if (dev->rx_handler)
		return -EBUSY;

	/* Note: rx_handler_data must be set before rx_handler */
	rcu_assign_pointer(dev->rx_handler_data, rx_handler_data);
	rcu_assign_pointer(dev->rx_handler, rx_handler);

	return 0;
}
EXPORT_SYMBOL_GPL(netdev_rx_handler_register);

#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
int (*bcm_vlan_handle_frame_hook)(struct sk_buff **) = NULL;
#endif
#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
void (*bcm_mcast_def_pri_queue_hook)(struct sk_buff *) = NULL;
EXPORT_SYMBOL(bcm_mcast_def_pri_queue_hook);
#endif
#if defined(CONFIG_BCM_KF_TMS) && defined(CONFIG_BCM_TMS_MODULE)
int (*bcm_1ag_handle_frame_check_hook)(struct sk_buff *) = NULL;
int (*bcm_3ah_handle_frame_check_hook)(struct sk_buff *, struct net_device *) = NULL;
#endif

/**
 *	netdev_rx_handler_unregister - unregister receive handler
 *	@dev: device to unregister a handler from
 *
 *	Unregister a receive handler from a device.
 *
 *	The caller must hold the rtnl_mutex.
 */
void netdev_rx_handler_unregister(struct net_device *dev)
{

	ASSERT_RTNL();
	RCU_INIT_POINTER(dev->rx_handler, NULL);
	/* a reader seeing a non NULL rx_handler in a rcu_read_lock()
	 * section has a guarantee to see a non NULL rx_handler_data
	 * as well.
	 */
	synchronize_net();
	RCU_INIT_POINTER(dev->rx_handler_data, NULL);
}
EXPORT_SYMBOL_GPL(netdev_rx_handler_unregister);

/*
 * Limit the use of PFMEMALLOC reserves to those protocols that implement
 * the special handling of PFMEMALLOC skbs.
 */
static bool skb_pfmemalloc_protocol(struct sk_buff *skb)
{
	switch (skb->protocol) {
	case htons(ETH_P_ARP):
	case htons(ETH_P_IP):
	case htons(ETH_P_IPV6):
	case htons(ETH_P_8021Q):
	case htons(ETH_P_8021AD):
		return true;
	default:
		return false;
	}
}

static int __netif_receive_skb_core(struct sk_buff *skb, bool pfmemalloc)
{
	struct packet_type *ptype, *pt_prev;
	rx_handler_func_t *rx_handler;
	struct net_device *orig_dev;
	bool deliver_exact = false;
	int ret = NET_RX_DROP;
	__be16 type;
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)) && defined(CONFIG_BCM_PTP_1588)
	struct skb_shared_hwtstamps *timestamp;
#endif

	net_timestamp_check(!netdev_tstamp_prequeue, skb);

	trace_netif_receive_skb(skb);

#if defined(CONFIG_BCM_KF_WANDEV)
	/* mark IFFWAN flag in skb based on dev->priv_flags */
	if (skb->dev) {
		unsigned int mark = skb->mark;
		skb->mark |= SKBMARK_SET_IFFWAN_MARK(mark, ((skb->dev->priv_flags & IFF_WANDEV) ? 1:0));
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG_FEATURE)
		if (skb->blog_p)
			skb->blog_p->isWan = (skb->dev->priv_flags & IFF_WANDEV) ? 1 : 0;
#endif
	}
#endif

	orig_dev = skb->dev;

	skb_reset_network_header(skb);
	if (!skb_transport_header_was_set(skb))
		skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);

	pt_prev = NULL;

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
	if (bcm_mcast_def_pri_queue_hook != NULL)
		bcm_mcast_def_pri_queue_hook(skb);
#endif

#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
#if defined(CONFIG_BCM_KF_TMS) && defined(CONFIG_BCM_TMS_MODULE)
	/* Check if 802.1ag service is started. */
	if (bcm_1ag_handle_frame_check_hook) {
		/* We want to skip vlanctl for 1ag packet. */
		if (bcm_1ag_handle_frame_check_hook(skb))
			goto skip_vlanctl;
	}
	/* Check if 802.3ah service is started. */
	if (bcm_3ah_handle_frame_check_hook) {
		/* We want to skip vlanctl for 3ah packet, or for any packet
		 * when 3ah loopback was enabled. */
		if ((bcm_3ah_handle_frame_check_hook(skb, skb->dev)))
			goto skip_vlanctl;
	}
#endif /* #if defined(CONFIG_BCM_TMS_MODULE) */

#if (defined(CONFIG_BCM_PTP_1588))
	timestamp = skb_hwtstamps(skb);
	if (timestamp->hwtstamp.tv64 == 0)
#endif

another_round:
	if (bcm_vlan_handle_frame_hook && (ret = bcm_vlan_handle_frame_hook(&skb)) != 0)
		goto out;

#if defined(CONFIG_BCM_KF_TMS) && defined(CONFIG_BCM_TMS_MODULE)
skip_vlanctl:
#endif
#else
another_round:
#endif

	skb->skb_iif = skb->dev->ifindex;

	__this_cpu_inc(softnet_data.processed);

	if (skb->protocol == cpu_to_be16(ETH_P_8021Q) ||
	    skb->protocol == cpu_to_be16(ETH_P_8021AD)) {
		skb = skb_vlan_untag(skb);
		if (unlikely(!skb))
			goto out;
	}

#ifdef CONFIG_NET_CLS_ACT
	if (skb->tc_verd & TC_NCLS) {
		skb->tc_verd = CLR_TC_NCLS(skb->tc_verd);
		goto ncls;
	}
#endif

	if (pfmemalloc)
		goto skip_taps;

	list_for_each_entry_rcu(ptype, &ptype_all, list) {
		if (pt_prev)
			ret = deliver_skb(skb, pt_prev, orig_dev);
		pt_prev = ptype;
	}

	list_for_each_entry_rcu(ptype, &skb->dev->ptype_all, list) {
		if (pt_prev)
			ret = deliver_skb(skb, pt_prev, orig_dev);
		pt_prev = ptype;
	}

skip_taps:
#ifdef CONFIG_NET_CLS_ACT
	if (static_key_false(&ingress_needed)) {
		skb = handle_ing(skb, &pt_prev, &ret, orig_dev);
		if (!skb)
			goto out;
	}

	skb->tc_verd = 0;
ncls:
#endif
	if (pfmemalloc && !skb_pfmemalloc_protocol(skb))
		goto drop;

	if (skb_vlan_tag_present(skb)) {
		if (pt_prev) {
			ret = deliver_skb(skb, pt_prev, orig_dev);
			pt_prev = NULL;
		}
		if (vlan_do_receive(&skb))
			goto another_round;
		else if (unlikely(!skb))
			goto out;
	}

	rx_handler = rcu_dereference(skb->dev->rx_handler);

//#if defined(CONFIG_BCM_KF_PPP)
#if 0
	if (skb->protocol == __constant_htons(ETH_P_PPP_SES) ||
	    skb->protocol == __constant_htons(ETH_P_PPP_DISC)) {
		if (!memcmp(skb_mac_header(skb), skb->dev->dev_addr, ETH_ALEN))
#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) && defined(CONFIG_BCM_KF_LOG)
		{
			bcmFun_t *bcmFun = bcmFun_get(BCM_FUN_ID_BOND_RX_HANDLER);
			if (!bcmFun || !bcmFun(rx_handler))
#endif
				goto skip_rx_handler;
#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) && defined(CONFIG_BCM_KF_LOG)
		}
#endif
	}
#endif

	if (rx_handler) {
		if (pt_prev) {
			ret = deliver_skb(skb, pt_prev, orig_dev);
			pt_prev = NULL;
		}
		switch (rx_handler(&skb)) {
		case RX_HANDLER_CONSUMED:
			ret = NET_RX_SUCCESS;
			goto out;
		case RX_HANDLER_ANOTHER:
			goto another_round;
		case RX_HANDLER_EXACT:
			deliver_exact = true;
		case RX_HANDLER_PASS:
			break;
		default:
			BUG();
		}
	}

	if (unlikely(skb_vlan_tag_present(skb))) {
		if (skb_vlan_tag_get_id(skb))
			skb->pkt_type = PACKET_OTHERHOST;
		/* Note: we might in the future use prio bits
		 * and set skb->priority like in vlan_do_receive()
		 * For the time being, just ignore Priority Code Point
		 */
#if defined(CONFIG_BCM_KF_TMS) && defined(CONFIG_BCM_TMS_MODULE)
		if (skb->protocol != htons(ETH_P_8021AG) && skb->protocol != htons(ETH_P_8023AH) &&
			((struct vlan_hdr *)skb->data)->h_vlan_encapsulated_proto != htons(ETH_P_8021AG))
#endif
		skb->vlan_tci = 0;
	}

//#if defined(CONFIG_BCM_KF_PPP)
#if 0
skip_rx_handler:
#endif

	type = skb->protocol;

	/* deliver only exact match when indicated */
	if (likely(!deliver_exact)) {
		deliver_ptype_list_skb(skb, &pt_prev, orig_dev, type,
				       &ptype_base[ntohs(type) &
						   PTYPE_HASH_MASK]);
	}

	deliver_ptype_list_skb(skb, &pt_prev, orig_dev, type,
			       &orig_dev->ptype_specific);

	if (unlikely(skb->dev != orig_dev)) {
		deliver_ptype_list_skb(skb, &pt_prev, orig_dev, type,
				       &skb->dev->ptype_specific);
	}

	if (pt_prev) {
		if (unlikely(skb_orphan_frags(skb, GFP_ATOMIC)))
			goto drop;
		else
			ret = pt_prev->func(skb, skb->dev, pt_prev, orig_dev);
	} else {
drop:
		atomic_long_inc(&skb->dev->rx_dropped);
		kfree_skb(skb);
		/* Jamal, now you will not able to escape explaining
		 * me how you were going to use this. :-)
		 */
		ret = NET_RX_DROP;
	}

out:
	return ret;
}

static int __netif_receive_skb(struct sk_buff *skb)
{
	int ret;

	if (sk_memalloc_socks() && skb_pfmemalloc(skb)) {
		unsigned long pflags = current->flags;

		/*
		 * PFMEMALLOC skbs are special, they should
		 * - be delivered to SOCK_MEMALLOC sockets only
		 * - stay away from userspace
		 * - have bounded memory usage
		 *
		 * Use PF_MEMALLOC as this saves us from propagating the allocation
		 * context down to all allocation sites.
		 */
		current->flags |= PF_MEMALLOC;
		ret = __netif_receive_skb_core(skb, true);
		tsk_restore_flags(current, pflags, PF_MEMALLOC);
	} else
		ret = __netif_receive_skb_core(skb, false);

	return ret;
}

static int netif_receive_skb_internal(struct sk_buff *skb)
{
	int ret;

	net_timestamp_check(netdev_tstamp_prequeue, skb);

	if (skb_defer_rx_timestamp(skb))
		return NET_RX_SUCCESS;

	rcu_read_lock();

#ifdef CONFIG_RPS
	if (static_key_false(&rps_needed)) {
		struct rps_dev_flow voidflow, *rflow = &voidflow;
		int cpu = get_rps_cpu(skb->dev, skb, &rflow);

		if (cpu >= 0) {
			ret = enqueue_to_backlog(skb, cpu, &rflow->last_qtail);
			rcu_read_unlock();
			return ret;
		}
	}
#endif
	ret = __netif_receive_skb(skb);
	rcu_read_unlock();
	return ret;
}

/**
 *	netif_receive_skb - process receive buffer from network
 *	@skb: buffer to process
 *
 *	netif_receive_skb() is the main receive data processing function.
 *	It always succeeds. The buffer may be dropped during processing
 *	for congestion control or by the protocol layers.
 *
 *	This function may only be called from softirq context and interrupts
 *	should be enabled.
 *
 *	Return values (usually ignored):
 *	NET_RX_SUCCESS: no congestion
 *	NET_RX_DROP: packet was dropped
 */
int netif_receive_skb_sk(struct sock *sk, struct sk_buff *skb)
{
	trace_netif_receive_skb_entry(skb);

	return netif_receive_skb_internal(skb);
}
EXPORT_SYMBOL(netif_receive_skb_sk);

/* Network device is going away, flush any packets still pending
 * Called with irqs disabled.
 */
static void flush_backlog(void *arg)
{
	struct net_device *dev = arg;
	struct softnet_data *sd = this_cpu_ptr(&softnet_data);
	struct sk_buff *skb, *tmp;

	rps_lock(sd);
	skb_queue_walk_safe(&sd->input_pkt_queue, skb, tmp) {
		if (skb->dev == dev) {
			__skb_unlink(skb, &sd->input_pkt_queue);
			kfree_skb(skb);
			input_queue_head_incr(sd);
		}
	}
	rps_unlock(sd);

	skb_queue_walk_safe(&sd->process_queue, skb, tmp) {
		if (skb->dev == dev) {
			__skb_unlink(skb, &sd->process_queue);
			kfree_skb(skb);
			input_queue_head_incr(sd);
		}
	}
}

static int napi_gro_complete(struct sk_buff *skb)
{
	struct packet_offload *ptype;
	__be16 type = skb->protocol;
	struct list_head *head = &offload_base;
	int err = -ENOENT;

	BUILD_BUG_ON(sizeof(struct napi_gro_cb) > sizeof(skb->cb));

	if (NAPI_GRO_CB(skb)->count == 1) {
		skb_shinfo(skb)->gso_size = 0;
		goto out;
	}

	rcu_read_lock();
	list_for_each_entry_rcu(ptype, head, list) {
		if (ptype->type != type || !ptype->callbacks.gro_complete)
			continue;

		err = ptype->callbacks.gro_complete(skb, 0);
		break;
	}
	rcu_read_unlock();

	if (err) {
		WARN_ON(&ptype->list == head);
		kfree_skb(skb);
		return NET_RX_SUCCESS;
	}

out:
	return netif_receive_skb_internal(skb);
}

/* napi->gro_list contains packets ordered by age.
 * youngest packets at the head of it.
 * Complete skbs in reverse order to reduce latencies.
 */
void napi_gro_flush(struct napi_struct *napi, bool flush_old)
{
	struct sk_buff *skb, *prev = NULL;

	/* scan list and build reverse chain */
	for (skb = napi->gro_list; skb != NULL; skb = skb->next) {
		skb->prev = prev;
		prev = skb;
	}

	for (skb = prev; skb; skb = prev) {
		skb->next = NULL;

		if (flush_old && NAPI_GRO_CB(skb)->age == jiffies)
			return;

		prev = skb->prev;
		napi_gro_complete(skb);
		napi->gro_count--;
	}

	napi->gro_list = NULL;
}
EXPORT_SYMBOL(napi_gro_flush);

static void gro_list_prepare(struct napi_struct *napi, struct sk_buff *skb)
{
	struct sk_buff *p;
	unsigned int maclen = skb->dev->hard_header_len;
	u32 hash = skb_get_hash_raw(skb);

	for (p = napi->gro_list; p; p = p->next) {
		unsigned long diffs;

		NAPI_GRO_CB(p)->flush = 0;

		if (hash != skb_get_hash_raw(p)) {
			NAPI_GRO_CB(p)->same_flow = 0;
			continue;
		}

		diffs = (unsigned long)p->dev ^ (unsigned long)skb->dev;
		diffs |= p->vlan_tci ^ skb->vlan_tci;
		if (maclen == ETH_HLEN)
			diffs |= compare_ether_header(skb_mac_header(p),
						      skb_mac_header(skb));
		else if (!diffs)
			diffs = memcmp(skb_mac_header(p),
				       skb_mac_header(skb),
				       maclen);
		NAPI_GRO_CB(p)->same_flow = !diffs;
	}
}

static void skb_gro_reset_offset(struct sk_buff *skb)
{
	const struct skb_shared_info *pinfo = skb_shinfo(skb);
	const skb_frag_t *frag0 = &pinfo->frags[0];

	NAPI_GRO_CB(skb)->data_offset = 0;
	NAPI_GRO_CB(skb)->frag0 = NULL;
	NAPI_GRO_CB(skb)->frag0_len = 0;

	if (skb_mac_header(skb) == skb_tail_pointer(skb) &&
	    pinfo->nr_frags &&
	    !PageHighMem(skb_frag_page(frag0))) {
		NAPI_GRO_CB(skb)->frag0 = skb_frag_address(frag0);
		NAPI_GRO_CB(skb)->frag0_len = skb_frag_size(frag0);
	}
}

static void gro_pull_from_frag0(struct sk_buff *skb, int grow)
{
	struct skb_shared_info *pinfo = skb_shinfo(skb);

	BUG_ON(skb->end - skb->tail < grow);

	memcpy(skb_tail_pointer(skb), NAPI_GRO_CB(skb)->frag0, grow);

	skb->data_len -= grow;
	skb->tail += grow;

	pinfo->frags[0].page_offset += grow;
	skb_frag_size_sub(&pinfo->frags[0], grow);

	if (unlikely(!skb_frag_size(&pinfo->frags[0]))) {
		skb_frag_unref(skb, 0);
		memmove(pinfo->frags, pinfo->frags + 1,
			--pinfo->nr_frags * sizeof(pinfo->frags[0]));
	}
}

static enum gro_result dev_gro_receive(struct napi_struct *napi, struct sk_buff *skb)
{
	struct sk_buff **pp = NULL;
	struct packet_offload *ptype;
	__be16 type = skb->protocol;
	struct list_head *head = &offload_base;
	int same_flow;
	enum gro_result ret;
	int grow;

	if (!(skb->dev->features & NETIF_F_GRO))
		goto normal;

	if (skb_is_gso(skb) || skb_has_frag_list(skb) || skb->csum_bad)
		goto normal;

	gro_list_prepare(napi, skb);

	rcu_read_lock();
	list_for_each_entry_rcu(ptype, head, list) {
		if (ptype->type != type || !ptype->callbacks.gro_receive)
			continue;

		skb_set_network_header(skb, skb_gro_offset(skb));
		skb_reset_mac_len(skb);
		NAPI_GRO_CB(skb)->same_flow = 0;
		NAPI_GRO_CB(skb)->flush = 0;
		NAPI_GRO_CB(skb)->free = 0;
		NAPI_GRO_CB(skb)->recursion_counter = 0;
		NAPI_GRO_CB(skb)->encap_mark = 0;
		NAPI_GRO_CB(skb)->gro_remcsum_start = 0;

		/* Setup for GRO checksum validation */
		switch (skb->ip_summed) {
		case CHECKSUM_COMPLETE:
			NAPI_GRO_CB(skb)->csum = skb->csum;
			NAPI_GRO_CB(skb)->csum_valid = 1;
			NAPI_GRO_CB(skb)->csum_cnt = 0;
			break;
		case CHECKSUM_UNNECESSARY:
			NAPI_GRO_CB(skb)->csum_cnt = skb->csum_level + 1;
			NAPI_GRO_CB(skb)->csum_valid = 0;
			break;
		default:
			NAPI_GRO_CB(skb)->csum_cnt = 0;
			NAPI_GRO_CB(skb)->csum_valid = 0;
		}

		pp = ptype->callbacks.gro_receive(&napi->gro_list, skb);
		break;
	}
	rcu_read_unlock();

	if (&ptype->list == head)
		goto normal;

	same_flow = NAPI_GRO_CB(skb)->same_flow;
	ret = NAPI_GRO_CB(skb)->free ? GRO_MERGED_FREE : GRO_MERGED;

	if (pp) {
		struct sk_buff *nskb = *pp;

		*pp = nskb->next;
		nskb->next = NULL;
		napi_gro_complete(nskb);
		napi->gro_count--;
	}

	if (same_flow)
		goto ok;

	if (NAPI_GRO_CB(skb)->flush)
		goto normal;

	if (unlikely(napi->gro_count >= MAX_GRO_SKBS)) {
		struct sk_buff *nskb = napi->gro_list;

		/* locate the end of the list to select the 'oldest' flow */
		while (nskb->next) {
			pp = &nskb->next;
			nskb = *pp;
		}
		*pp = NULL;
		nskb->next = NULL;
		napi_gro_complete(nskb);
	} else {
		napi->gro_count++;
	}
	NAPI_GRO_CB(skb)->count = 1;
	NAPI_GRO_CB(skb)->age = jiffies;
	NAPI_GRO_CB(skb)->last = skb;
	skb_shinfo(skb)->gso_size = skb_gro_len(skb);
	skb->next = napi->gro_list;
	napi->gro_list = skb;
	ret = GRO_HELD;

pull:
	grow = skb_gro_offset(skb) - skb_headlen(skb);
	if (grow > 0)
		gro_pull_from_frag0(skb, grow);
ok:
	return ret;

normal:
	ret = GRO_NORMAL;
	goto pull;
}

struct packet_offload *gro_find_receive_by_type(__be16 type)
{
	struct list_head *offload_head = &offload_base;
	struct packet_offload *ptype;

	list_for_each_entry_rcu(ptype, offload_head, list) {
		if (ptype->type != type || !ptype->callbacks.gro_receive)
			continue;
		return ptype;
	}
	return NULL;
}
EXPORT_SYMBOL(gro_find_receive_by_type);

struct packet_offload *gro_find_complete_by_type(__be16 type)
{
	struct list_head *offload_head = &offload_base;
	struct packet_offload *ptype;

	list_for_each_entry_rcu(ptype, offload_head, list) {
		if (ptype->type != type || !ptype->callbacks.gro_complete)
			continue;
		return ptype;
	}
	return NULL;
}
EXPORT_SYMBOL(gro_find_complete_by_type);

static gro_result_t napi_skb_finish(gro_result_t ret, struct sk_buff *skb)
{
	switch (ret) {
	case GRO_NORMAL:
		if (netif_receive_skb_internal(skb))
			ret = GRO_DROP;
		break;

	case GRO_DROP:
		kfree_skb(skb);
		break;

	case GRO_MERGED_FREE:
		if (NAPI_GRO_CB(skb)->free == NAPI_GRO_FREE_STOLEN_HEAD)
			kmem_cache_free(skbuff_head_cache, skb);
		else
			__kfree_skb(skb);
		break;

	case GRO_HELD:
	case GRO_MERGED:
		break;
	}

	return ret;
}

gro_result_t napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb)
{
	trace_napi_gro_receive_entry(skb);

	skb_gro_reset_offset(skb);

	return napi_skb_finish(dev_gro_receive(napi, skb), skb);
}
EXPORT_SYMBOL(napi_gro_receive);

static void napi_reuse_skb(struct napi_struct *napi, struct sk_buff *skb)
{
	if (unlikely(skb->pfmemalloc)) {
		consume_skb(skb);
		return;
	}
	__skb_pull(skb, skb_headlen(skb));
	/* restore the reserve we had after netdev_alloc_skb_ip_align() */
	skb_reserve(skb, NET_SKB_PAD + NET_IP_ALIGN - skb_headroom(skb));
	skb->vlan_tci = 0;
	skb->dev = napi->dev;
	skb->skb_iif = 0;
	skb->encapsulation = 0;
	skb_shinfo(skb)->gso_type = 0;
	skb->truesize = SKB_TRUESIZE(skb_end_offset(skb));

	napi->skb = skb;
}

struct sk_buff *napi_get_frags(struct napi_struct *napi)
{
	struct sk_buff *skb = napi->skb;

	if (!skb) {
		skb = napi_alloc_skb(napi, GRO_MAX_HEAD);
		napi->skb = skb;
	}
	return skb;
}
EXPORT_SYMBOL(napi_get_frags);

static gro_result_t napi_frags_finish(struct napi_struct *napi,
				      struct sk_buff *skb,
				      gro_result_t ret)
{
	switch (ret) {
	case GRO_NORMAL:
	case GRO_HELD:
		__skb_push(skb, ETH_HLEN);
		skb->protocol = eth_type_trans(skb, skb->dev);
		if (ret == GRO_NORMAL && netif_receive_skb_internal(skb))
			ret = GRO_DROP;
		break;

	case GRO_DROP:
	case GRO_MERGED_FREE:
		napi_reuse_skb(napi, skb);
		break;

	case GRO_MERGED:
		break;
	}

	return ret;
}

/* Upper GRO stack assumes network header starts at gro_offset=0
 * Drivers could call both napi_gro_frags() and napi_gro_receive()
 * We copy ethernet header into skb->data to have a common layout.
 */
static struct sk_buff *napi_frags_skb(struct napi_struct *napi)
{
	struct sk_buff *skb = napi->skb;
	const struct ethhdr *eth;
	unsigned int hlen = sizeof(*eth);

	napi->skb = NULL;

	skb_reset_mac_header(skb);
	skb_gro_reset_offset(skb);

	eth = skb_gro_header_fast(skb, 0);
	if (unlikely(skb_gro_header_hard(skb, hlen))) {
		eth = skb_gro_header_slow(skb, hlen, 0);
		if (unlikely(!eth)) {
			napi_reuse_skb(napi, skb);
			return NULL;
		}
	} else {
		gro_pull_from_frag0(skb, hlen);
		NAPI_GRO_CB(skb)->frag0 += hlen;
		NAPI_GRO_CB(skb)->frag0_len -= hlen;
	}
	__skb_pull(skb, hlen);

	/*
	 * This works because the only protocols we care about don't require
	 * special handling.
	 * We'll fix it up properly in napi_frags_finish()
	 */
	skb->protocol = eth->h_proto;

	return skb;
}

gro_result_t napi_gro_frags(struct napi_struct *napi)
{
	struct sk_buff *skb = napi_frags_skb(napi);

	if (!skb)
		return GRO_DROP;

	trace_napi_gro_frags_entry(skb);

	return napi_frags_finish(napi, skb, dev_gro_receive(napi, skb));
}
EXPORT_SYMBOL(napi_gro_frags);

/* Compute the checksum from gro_offset and return the folded value
 * after adding in any pseudo checksum.
 */
__sum16 __skb_gro_checksum_complete(struct sk_buff *skb)
{
	__wsum wsum;
	__sum16 sum;

	wsum = skb_checksum(skb, skb_gro_offset(skb), skb_gro_len(skb), 0);

	/* NAPI_GRO_CB(skb)->csum holds pseudo checksum */
	sum = csum_fold(csum_add(NAPI_GRO_CB(skb)->csum, wsum));
	if (likely(!sum)) {
		if (unlikely(skb->ip_summed == CHECKSUM_COMPLETE) &&
		    !skb->csum_complete_sw)
			netdev_rx_csum_fault(skb->dev);
	}

	NAPI_GRO_CB(skb)->csum = wsum;
	NAPI_GRO_CB(skb)->csum_valid = 1;

	return sum;
}
EXPORT_SYMBOL(__skb_gro_checksum_complete);

/*
 * net_rps_action_and_irq_enable sends any pending IPI's for rps.
 * Note: called with local irq disabled, but exits with local irq enabled.
 */
static void net_rps_action_and_irq_enable(struct softnet_data *sd)
{
#ifdef CONFIG_RPS
	struct softnet_data *remsd = sd->rps_ipi_list;

	if (remsd) {
		sd->rps_ipi_list = NULL;

		local_irq_enable();

		/* Send pending IPI's to kick RPS processing on remote cpus. */
		while (remsd) {
			struct softnet_data *next = remsd->rps_ipi_next;

			if (cpu_online(remsd->cpu))
				smp_call_function_single_async(remsd->cpu,
							   &remsd->csd);
			remsd = next;
		}
	} else
#endif
		local_irq_enable();
}

static bool sd_has_rps_ipi_waiting(struct softnet_data *sd)
{
#ifdef CONFIG_RPS
	return sd->rps_ipi_list != NULL;
#else
	return false;
#endif
}

static int process_backlog(struct napi_struct *napi, int quota)
{
	int work = 0;
	struct softnet_data *sd = container_of(napi, struct softnet_data, backlog);

	/* Check if we have pending ipi, its better to send them now,
	 * not waiting net_rx_action() end.
	 */
	if (sd_has_rps_ipi_waiting(sd)) {
		local_irq_disable();
		net_rps_action_and_irq_enable(sd);
	}

	napi->weight = weight_p;
	local_irq_disable();
	while (1) {
		struct sk_buff *skb;

		while ((skb = __skb_dequeue(&sd->process_queue))) {
			rcu_read_lock();
			local_irq_enable();
			__netif_receive_skb(skb);
			rcu_read_unlock();
			local_irq_disable();
			input_queue_head_incr(sd);
			if (++work >= quota) {
				local_irq_enable();
				return work;
			}
		}

		rps_lock(sd);
		if (skb_queue_empty(&sd->input_pkt_queue)) {
			/*
			 * Inline a custom version of __napi_complete().
			 * only current cpu owns and manipulates this napi,
			 * and NAPI_STATE_SCHED is the only possible flag set
			 * on backlog.
			 * We can use a plain write instead of clear_bit(),
			 * and we dont need an smp_mb() memory barrier.
			 */
			napi->state = 0;
			rps_unlock(sd);

			break;
		}

		skb_queue_splice_tail_init(&sd->input_pkt_queue,
					   &sd->process_queue);
		rps_unlock(sd);
	}
	local_irq_enable();

	return work;
}

/**
 * __napi_schedule - schedule for receive
 * @n: entry to schedule
 *
 * The entry's receive function will be scheduled to run.
 * Consider using __napi_schedule_irqoff() if hard irqs are masked.
 */
void __napi_schedule(struct napi_struct *n)
{
	unsigned long flags;

	local_irq_save(flags);
	____napi_schedule(this_cpu_ptr(&softnet_data), n);
	local_irq_restore(flags);
}
EXPORT_SYMBOL(__napi_schedule);

/**
 * __napi_schedule_irqoff - schedule for receive
 * @n: entry to schedule
 *
 * Variant of __napi_schedule() assuming hard irqs are masked
 */
void __napi_schedule_irqoff(struct napi_struct *n)
{
	____napi_schedule(this_cpu_ptr(&softnet_data), n);
}
EXPORT_SYMBOL(__napi_schedule_irqoff);

void __napi_complete(struct napi_struct *n)
{
	BUG_ON(!test_bit(NAPI_STATE_SCHED, &n->state));

	list_del_init(&n->poll_list);
	smp_mb__before_atomic();
	clear_bit(NAPI_STATE_SCHED, &n->state);
}
EXPORT_SYMBOL(__napi_complete);

void napi_complete_done(struct napi_struct *n, int work_done)
{
	unsigned long flags;

	/*
	 * don't let napi dequeue from the cpu poll list
	 * just in case its running on a different cpu
	 */
	if (unlikely(test_bit(NAPI_STATE_NPSVC, &n->state)))
		return;

	if (n->gro_list) {
		unsigned long timeout = 0;

		if (work_done)
			timeout = n->dev->gro_flush_timeout;

		if (timeout)
			hrtimer_start(&n->timer, ns_to_ktime(timeout),
				      HRTIMER_MODE_REL_PINNED);
		else
			napi_gro_flush(n, false);
	}
	if (likely(list_empty(&n->poll_list))) {
		WARN_ON_ONCE(!test_and_clear_bit(NAPI_STATE_SCHED, &n->state));
	} else {
		/* If n->poll_list is not empty, we need to mask irqs */
		local_irq_save(flags);
		__napi_complete(n);
		local_irq_restore(flags);
	}
}
EXPORT_SYMBOL(napi_complete_done);

/* must be called under rcu_read_lock(), as we dont take a reference */
struct napi_struct *napi_by_id(unsigned int napi_id)
{
	unsigned int hash = napi_id % HASH_SIZE(napi_hash);
	struct napi_struct *napi;

	hlist_for_each_entry_rcu(napi, &napi_hash[hash], napi_hash_node)
		if (napi->napi_id == napi_id)
			return napi;

	return NULL;
}
EXPORT_SYMBOL_GPL(napi_by_id);

void napi_hash_add(struct napi_struct *napi)
{
	if (!test_and_set_bit(NAPI_STATE_HASHED, &napi->state)) {

		spin_lock(&napi_hash_lock);

		/* 0 is not a valid id, we also skip an id that is taken
		 * we expect both events to be extremely rare
		 */
		napi->napi_id = 0;
		while (!napi->napi_id) {
			napi->napi_id = ++napi_gen_id;
			if (napi_by_id(napi->napi_id))
				napi->napi_id = 0;
		}

		hlist_add_head_rcu(&napi->napi_hash_node,
			&napi_hash[napi->napi_id % HASH_SIZE(napi_hash)]);

		spin_unlock(&napi_hash_lock);
	}
}
EXPORT_SYMBOL_GPL(napi_hash_add);

/* Warning : caller is responsible to make sure rcu grace period
 * is respected before freeing memory containing @napi
 */
void napi_hash_del(struct napi_struct *napi)
{
	spin_lock(&napi_hash_lock);

	if (test_and_clear_bit(NAPI_STATE_HASHED, &napi->state))
		hlist_del_rcu(&napi->napi_hash_node);

	spin_unlock(&napi_hash_lock);
}
EXPORT_SYMBOL_GPL(napi_hash_del);

static enum hrtimer_restart napi_watchdog(struct hrtimer *timer)
{
	struct napi_struct *napi;

	napi = container_of(timer, struct napi_struct, timer);
	if (napi->gro_list)
		napi_schedule(napi);

	return HRTIMER_NORESTART;
}

void netif_napi_add(struct net_device *dev, struct napi_struct *napi,
		    int (*poll)(struct napi_struct *, int), int weight)
{
	INIT_LIST_HEAD(&napi->poll_list);
	hrtimer_init(&napi->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
	napi->timer.function = napi_watchdog;
	napi->gro_count = 0;
	napi->gro_list = NULL;
	napi->skb = NULL;
	napi->poll = poll;
	if (weight > NAPI_POLL_WEIGHT)
		pr_err_once("netif_napi_add() called with weight %d on device %s\n",
			    weight, dev->name);
	napi->weight = weight;
	list_add(&napi->dev_list, &dev->napi_list);
	napi->dev = dev;
#ifdef CONFIG_NETPOLL
	spin_lock_init(&napi->poll_lock);
	napi->poll_owner = -1;
#endif
	set_bit(NAPI_STATE_SCHED, &napi->state);
}
EXPORT_SYMBOL(netif_napi_add);

void napi_disable(struct napi_struct *n)
{
	might_sleep();
	set_bit(NAPI_STATE_DISABLE, &n->state);

	while (test_and_set_bit(NAPI_STATE_SCHED, &n->state))
		msleep(1);

	hrtimer_cancel(&n->timer);

	clear_bit(NAPI_STATE_DISABLE, &n->state);
}
EXPORT_SYMBOL(napi_disable);

void netif_napi_del(struct napi_struct *napi)
{
	list_del_init(&napi->dev_list);
	napi_free_frags(napi);

	kfree_skb_list(napi->gro_list);
	napi->gro_list = NULL;
	napi->gro_count = 0;
}
EXPORT_SYMBOL(netif_napi_del);

static int napi_poll(struct napi_struct *n, struct list_head *repoll)
{
	void *have;
	int work, weight;

	list_del_init(&n->poll_list);

	have = netpoll_poll_lock(n);

	weight = n->weight;

	/* This NAPI_STATE_SCHED test is for avoiding a race
	 * with netpoll's poll_napi().  Only the entity which
	 * obtains the lock and sees NAPI_STATE_SCHED set will
	 * actually make the ->poll() call.  Therefore we avoid
	 * accidentally calling ->poll() when NAPI is not scheduled.
	 */
	work = 0;
	if (test_bit(NAPI_STATE_SCHED, &n->state)) {
		work = n->poll(n, weight);
		trace_napi_poll(n);
	}

	WARN_ON_ONCE(work > weight);

	if (likely(work < weight))
		goto out_unlock;

	/* Drivers must not modify the NAPI state if they
	 * consume the entire weight.  In such cases this code
	 * still "owns" the NAPI instance and therefore can
	 * move the instance around on the list at-will.
	 */
	if (unlikely(napi_disable_pending(n))) {
		napi_complete(n);
		goto out_unlock;
	}

	if (n->gro_list) {
		/* flush too old packets
		 * If HZ < 1000, flush all packets.
		 */
		napi_gro_flush(n, HZ >= 1000);
	}

	/* Some drivers may have called napi_schedule
	 * prior to exhausting their budget.
	 */
	if (unlikely(!list_empty(&n->poll_list))) {
		pr_warn_once("%s: Budget exhausted after napi rescheduled\n",
			     n->dev ? n->dev->name : "backlog");
		goto out_unlock;
	}

	list_add_tail(&n->poll_list, repoll);

out_unlock:
	netpoll_poll_unlock(have);

	return work;
}

static void net_rx_action(struct softirq_action *h)
{
	struct softnet_data *sd = this_cpu_ptr(&softnet_data);
	unsigned long time_limit = jiffies + 2;
	int budget = netdev_budget;
	LIST_HEAD(list);
	LIST_HEAD(repoll);

	local_irq_disable();
	list_splice_init(&sd->poll_list, &list);
	local_irq_enable();

	for (;;) {
		struct napi_struct *n;

		if (list_empty(&list)) {
			if (!sd_has_rps_ipi_waiting(sd) && list_empty(&repoll))
				return;
			break;
		}

		n = list_first_entry(&list, struct napi_struct, poll_list);
		budget -= napi_poll(n, &repoll);

		/* If softirq window is exhausted then punt.
		 * Allow this to run for 2 jiffies since which will allow
		 * an average latency of 1.5/HZ.
		 */
		if (unlikely(budget <= 0 ||
			     time_after_eq(jiffies, time_limit))) {
			sd->time_squeeze++;
			break;
		}
	}

	local_irq_disable();

	list_splice_tail_init(&sd->poll_list, &list);
	list_splice_tail(&repoll, &list);
	list_splice(&list, &sd->poll_list);
	if (!list_empty(&sd->poll_list))
		__raise_softirq_irqoff(NET_RX_SOFTIRQ);

	net_rps_action_and_irq_enable(sd);
}

struct netdev_adjacent {
	struct net_device *dev;

	/* upper master flag, there can only be one master device per list */
	bool master;

	/* counter for the number of times this device was added to us */
	u16 ref_nr;

	/* private field for the users */
	void *private;

	struct list_head list;
	struct rcu_head rcu;
};

static struct netdev_adjacent *__netdev_find_adj(struct net_device *dev,
						 struct net_device *adj_dev,
						 struct list_head *adj_list)
{
	struct netdev_adjacent *adj;

	list_for_each_entry(adj, adj_list, list) {
		if (adj->dev == adj_dev)
			return adj;
	}
	return NULL;
}

/**
 * netdev_has_upper_dev - Check if device is linked to an upper device
 * @dev: device
 * @upper_dev: upper device to check
 *
 * Find out if a device is linked to specified upper device and return true
 * in case it is. Note that this checks only immediate upper device,
 * not through a complete stack of devices. The caller must hold the RTNL lock.
 */
bool netdev_has_upper_dev(struct net_device *dev,
			  struct net_device *upper_dev)
{
	ASSERT_RTNL();

	return __netdev_find_adj(dev, upper_dev, &dev->all_adj_list.upper);
}
EXPORT_SYMBOL(netdev_has_upper_dev);

/**
 * netdev_has_any_upper_dev - Check if device is linked to some device
 * @dev: device
 *
 * Find out if a device is linked to an upper device and return true in case
 * it is. The caller must hold the RTNL lock.
 */
static bool netdev_has_any_upper_dev(struct net_device *dev)
{
	ASSERT_RTNL();

	return !list_empty(&dev->all_adj_list.upper);
}

/**
 * netdev_master_upper_dev_get - Get master upper device
 * @dev: device
 *
 * Find a master upper device and return pointer to it or NULL in case
 * it's not there. The caller must hold the RTNL lock.
 */
struct net_device *netdev_master_upper_dev_get(struct net_device *dev)
{
	struct netdev_adjacent *upper;

	ASSERT_RTNL();

	if (list_empty(&dev->adj_list.upper))
		return NULL;

	upper = list_first_entry(&dev->adj_list.upper,
				 struct netdev_adjacent, list);
	if (likely(upper->master))
		return upper->dev;
	return NULL;
}
EXPORT_SYMBOL(netdev_master_upper_dev_get);

void *netdev_adjacent_get_private(struct list_head *adj_list)
{
	struct netdev_adjacent *adj;

	adj = list_entry(adj_list, struct netdev_adjacent, list);

	return adj->private;
}
EXPORT_SYMBOL(netdev_adjacent_get_private);

/**
 * netdev_upper_get_next_dev_rcu - Get the next dev from upper list
 * @dev: device
 * @iter: list_head ** of the current position
 *
 * Gets the next device from the dev's upper list, starting from iter
 * position. The caller must hold RCU read lock.
 */
struct net_device *netdev_upper_get_next_dev_rcu(struct net_device *dev,
						 struct list_head **iter)
{
	struct netdev_adjacent *upper;

	WARN_ON_ONCE(!rcu_read_lock_held() && !lockdep_rtnl_is_held());

	upper = list_entry_rcu((*iter)->next, struct netdev_adjacent, list);

	if (&upper->list == &dev->adj_list.upper)
		return NULL;

	*iter = &upper->list;

	return upper->dev;
}
EXPORT_SYMBOL(netdev_upper_get_next_dev_rcu);

/**
 * netdev_all_upper_get_next_dev_rcu - Get the next dev from upper list
 * @dev: device
 * @iter: list_head ** of the current position
 *
 * Gets the next device from the dev's upper list, starting from iter
 * position. The caller must hold RCU read lock.
 */
struct net_device *netdev_all_upper_get_next_dev_rcu(struct net_device *dev,
						     struct list_head **iter)
{
	struct netdev_adjacent *upper;

	WARN_ON_ONCE(!rcu_read_lock_held() && !lockdep_rtnl_is_held());

	upper = list_entry_rcu((*iter)->next, struct netdev_adjacent, list);

	if (&upper->list == &dev->all_adj_list.upper)
		return NULL;

	*iter = &upper->list;

	return upper->dev;
}
EXPORT_SYMBOL(netdev_all_upper_get_next_dev_rcu);

/**
 * netdev_lower_get_next_private - Get the next ->private from the
 *				   lower neighbour list
 * @dev: device
 * @iter: list_head ** of the current position
 *
 * Gets the next netdev_adjacent->private from the dev's lower neighbour
 * list, starting from iter position. The caller must hold either hold the
 * RTNL lock or its own locking that guarantees that the neighbour lower
 * list will remain unchainged.
 */
void *netdev_lower_get_next_private(struct net_device *dev,
				    struct list_head **iter)
{
	struct netdev_adjacent *lower;

	lower = list_entry(*iter, struct netdev_adjacent, list);

	if (&lower->list == &dev->adj_list.lower)
		return NULL;

	*iter = lower->list.next;

	return lower->private;
}
EXPORT_SYMBOL(netdev_lower_get_next_private);

/**
 * netdev_lower_get_next_private_rcu - Get the next ->private from the
 *				       lower neighbour list, RCU
 *				       variant
 * @dev: device
 * @iter: list_head ** of the current position
 *
 * Gets the next netdev_adjacent->private from the dev's lower neighbour
 * list, starting from iter position. The caller must hold RCU read lock.
 */
void *netdev_lower_get_next_private_rcu(struct net_device *dev,
					struct list_head **iter)
{
	struct netdev_adjacent *lower;

	WARN_ON_ONCE(!rcu_read_lock_held());

	lower = list_entry_rcu((*iter)->next, struct netdev_adjacent, list);

	if (&lower->list == &dev->adj_list.lower)
		return NULL;

	*iter = &lower->list;

	return lower->private;
}
EXPORT_SYMBOL(netdev_lower_get_next_private_rcu);

/**
 * netdev_lower_get_next - Get the next device from the lower neighbour
 *                         list
 * @dev: device
 * @iter: list_head ** of the current position
 *
 * Gets the next netdev_adjacent from the dev's lower neighbour
 * list, starting from iter position. The caller must hold RTNL lock or
 * its own locking that guarantees that the neighbour lower
 * list will remain unchainged.
 */
void *netdev_lower_get_next(struct net_device *dev, struct list_head **iter)
{
	struct netdev_adjacent *lower;

	lower = list_entry((*iter)->next, struct netdev_adjacent, list);

	if (&lower->list == &dev->adj_list.lower)
		return NULL;

	*iter = &lower->list;

	return lower->dev;
}
EXPORT_SYMBOL(netdev_lower_get_next);

/**
 * netdev_lower_get_first_private_rcu - Get the first ->private from the
 *				       lower neighbour list, RCU
 *				       variant
 * @dev: device
 *
 * Gets the first netdev_adjacent->private from the dev's lower neighbour
 * list. The caller must hold RCU read lock.
 */
void *netdev_lower_get_first_private_rcu(struct net_device *dev)
{
	struct netdev_adjacent *lower;

	lower = list_first_or_null_rcu(&dev->adj_list.lower,
			struct netdev_adjacent, list);
	if (lower)
		return lower->private;
	return NULL;
}
EXPORT_SYMBOL(netdev_lower_get_first_private_rcu);

/**
 * netdev_master_upper_dev_get_rcu - Get master upper device
 * @dev: device
 *
 * Find a master upper device and return pointer to it or NULL in case
 * it's not there. The caller must hold the RCU read lock.
 */
struct net_device *netdev_master_upper_dev_get_rcu(struct net_device *dev)
{
	struct netdev_adjacent *upper;

	upper = list_first_or_null_rcu(&dev->adj_list.upper,
				       struct netdev_adjacent, list);
	if (upper && likely(upper->master))
		return upper->dev;
	return NULL;
}
EXPORT_SYMBOL(netdev_master_upper_dev_get_rcu);

static int netdev_adjacent_sysfs_add(struct net_device *dev,
			      struct net_device *adj_dev,
			      struct list_head *dev_list)
{
	char linkname[IFNAMSIZ+7];
	sprintf(linkname, dev_list == &dev->adj_list.upper ?
		"upper_%s" : "lower_%s", adj_dev->name);
	return sysfs_create_link(&(dev->dev.kobj), &(adj_dev->dev.kobj),
				 linkname);
}
static void netdev_adjacent_sysfs_del(struct net_device *dev,
			       char *name,
			       struct list_head *dev_list)
{
	char linkname[IFNAMSIZ+7];
	sprintf(linkname, dev_list == &dev->adj_list.upper ?
		"upper_%s" : "lower_%s", name);
	sysfs_remove_link(&(dev->dev.kobj), linkname);
}

static inline bool netdev_adjacent_is_neigh_list(struct net_device *dev,
						 struct net_device *adj_dev,
						 struct list_head *dev_list)
{
	return (dev_list == &dev->adj_list.upper ||
		dev_list == &dev->adj_list.lower) &&
		net_eq(dev_net(dev), dev_net(adj_dev));
}

static int __netdev_adjacent_dev_insert(struct net_device *dev,
					struct net_device *adj_dev,
					struct list_head *dev_list,
					void *private, bool master)
{
	struct netdev_adjacent *adj;
	int ret;

	adj = __netdev_find_adj(dev, adj_dev, dev_list);

	if (adj) {
		adj->ref_nr++;
		return 0;
	}

	adj = kmalloc(sizeof(*adj), GFP_KERNEL);
	if (!adj)
		return -ENOMEM;

	adj->dev = adj_dev;
	adj->master = master;
	adj->ref_nr = 1;
	adj->private = private;
	dev_hold(adj_dev);

	pr_debug("dev_hold for %s, because of link added from %s to %s\n",
		 adj_dev->name, dev->name, adj_dev->name);

	if (netdev_adjacent_is_neigh_list(dev, adj_dev, dev_list)) {
		ret = netdev_adjacent_sysfs_add(dev, adj_dev, dev_list);
		if (ret)
			goto free_adj;
	}

	/* Ensure that master link is always the first item in list. */
	if (master) {
		ret = sysfs_create_link(&(dev->dev.kobj),
					&(adj_dev->dev.kobj), "master");
		if (ret)
			goto remove_symlinks;

		list_add_rcu(&adj->list, dev_list);
	} else {
		list_add_tail_rcu(&adj->list, dev_list);
	}

	return 0;

remove_symlinks:
	if (netdev_adjacent_is_neigh_list(dev, adj_dev, dev_list))
		netdev_adjacent_sysfs_del(dev, adj_dev->name, dev_list);
free_adj:
	kfree(adj);
	dev_put(adj_dev);

	return ret;
}

static void __netdev_adjacent_dev_remove(struct net_device *dev,
					 struct net_device *adj_dev,
					 struct list_head *dev_list)
{
	struct netdev_adjacent *adj;

	adj = __netdev_find_adj(dev, adj_dev, dev_list);

	if (!adj) {
		pr_err("tried to remove device %s from %s\n",
		       dev->name, adj_dev->name);
		BUG();
	}

	if (adj->ref_nr > 1) {
		pr_debug("%s to %s ref_nr-- = %d\n", dev->name, adj_dev->name,
			 adj->ref_nr-1);
		adj->ref_nr--;
		return;
	}

	if (adj->master)
		sysfs_remove_link(&(dev->dev.kobj), "master");

	if (netdev_adjacent_is_neigh_list(dev, adj_dev, dev_list))
		netdev_adjacent_sysfs_del(dev, adj_dev->name, dev_list);

	list_del_rcu(&adj->list);
	pr_debug("dev_put for %s, because link removed from %s to %s\n",
		 adj_dev->name, dev->name, adj_dev->name);
	dev_put(adj_dev);
	kfree_rcu(adj, rcu);
}

static int __netdev_adjacent_dev_link_lists(struct net_device *dev,
					    struct net_device *upper_dev,
					    struct list_head *up_list,
					    struct list_head *down_list,
					    void *private, bool master)
{
	int ret;

	ret = __netdev_adjacent_dev_insert(dev, upper_dev, up_list, private,
					   master);
	if (ret)
		return ret;

	ret = __netdev_adjacent_dev_insert(upper_dev, dev, down_list, private,
					   false);
	if (ret) {
		__netdev_adjacent_dev_remove(dev, upper_dev, up_list);
		return ret;
	}

	return 0;
}

static int __netdev_adjacent_dev_link(struct net_device *dev,
				      struct net_device *upper_dev)
{
	return __netdev_adjacent_dev_link_lists(dev, upper_dev,
						&dev->all_adj_list.upper,
						&upper_dev->all_adj_list.lower,
						NULL, false);
}

static void __netdev_adjacent_dev_unlink_lists(struct net_device *dev,
					       struct net_device *upper_dev,
					       struct list_head *up_list,
					       struct list_head *down_list)
{
	__netdev_adjacent_dev_remove(dev, upper_dev, up_list);
	__netdev_adjacent_dev_remove(upper_dev, dev, down_list);
}

static void __netdev_adjacent_dev_unlink(struct net_device *dev,
					 struct net_device *upper_dev)
{
	__netdev_adjacent_dev_unlink_lists(dev, upper_dev,
					   &dev->all_adj_list.upper,
					   &upper_dev->all_adj_list.lower);
}

static int __netdev_adjacent_dev_link_neighbour(struct net_device *dev,
						struct net_device *upper_dev,
						void *private, bool master)
{
	int ret = __netdev_adjacent_dev_link(dev, upper_dev);

	if (ret)
		return ret;

	ret = __netdev_adjacent_dev_link_lists(dev, upper_dev,
					       &dev->adj_list.upper,
					       &upper_dev->adj_list.lower,
					       private, master);
	if (ret) {
		__netdev_adjacent_dev_unlink(dev, upper_dev);
		return ret;
	}

	return 0;
}

static void __netdev_adjacent_dev_unlink_neighbour(struct net_device *dev,
						   struct net_device *upper_dev)
{
	__netdev_adjacent_dev_unlink(dev, upper_dev);
	__netdev_adjacent_dev_unlink_lists(dev, upper_dev,
					   &dev->adj_list.upper,
					   &upper_dev->adj_list.lower);
}

static int __netdev_upper_dev_link(struct net_device *dev,
				   struct net_device *upper_dev, bool master,
				   void *private)
{
	struct netdev_adjacent *i, *j, *to_i, *to_j;
	int ret = 0;

	ASSERT_RTNL();

	if (dev == upper_dev)
		return -EBUSY;

	/* To prevent loops, check if dev is not upper device to upper_dev. */
	if (__netdev_find_adj(upper_dev, dev, &upper_dev->all_adj_list.upper))
		return -EBUSY;

	if (__netdev_find_adj(dev, upper_dev, &dev->adj_list.upper))
		return -EEXIST;

	if (master && netdev_master_upper_dev_get(dev))
		return -EBUSY;

	ret = __netdev_adjacent_dev_link_neighbour(dev, upper_dev, private,
						   master);
	if (ret)
		return ret;

	/* Now that we linked these devs, make all the upper_dev's
	 * all_adj_list.upper visible to every dev's all_adj_list.lower an
	 * versa, and don't forget the devices itself. All of these
	 * links are non-neighbours.
	 */
	list_for_each_entry(i, &dev->all_adj_list.lower, list) {
		list_for_each_entry(j, &upper_dev->all_adj_list.upper, list) {
			pr_debug("Interlinking %s with %s, non-neighbour\n",
				 i->dev->name, j->dev->name);
			ret = __netdev_adjacent_dev_link(i->dev, j->dev);
			if (ret)
				goto rollback_mesh;
		}
	}

	/* add dev to every upper_dev's upper device */
	list_for_each_entry(i, &upper_dev->all_adj_list.upper, list) {
		pr_debug("linking %s's upper device %s with %s\n",
			 upper_dev->name, i->dev->name, dev->name);
		ret = __netdev_adjacent_dev_link(dev, i->dev);
		if (ret)
			goto rollback_upper_mesh;
	}

	/* add upper_dev to every dev's lower device */
	list_for_each_entry(i, &dev->all_adj_list.lower, list) {
		pr_debug("linking %s's lower device %s with %s\n", dev->name,
			 i->dev->name, upper_dev->name);
		ret = __netdev_adjacent_dev_link(i->dev, upper_dev);
		if (ret)
			goto rollback_lower_mesh;
	}

	call_netdevice_notifiers(NETDEV_CHANGEUPPER, dev);
	return 0;

rollback_lower_mesh:
	to_i = i;
	list_for_each_entry(i, &dev->all_adj_list.lower, list) {
		if (i == to_i)
			break;
		__netdev_adjacent_dev_unlink(i->dev, upper_dev);
	}

	i = NULL;

rollback_upper_mesh:
	to_i = i;
	list_for_each_entry(i, &upper_dev->all_adj_list.upper, list) {
		if (i == to_i)
			break;
		__netdev_adjacent_dev_unlink(dev, i->dev);
	}

	i = j = NULL;

rollback_mesh:
	to_i = i;
	to_j = j;
	list_for_each_entry(i, &dev->all_adj_list.lower, list) {
		list_for_each_entry(j, &upper_dev->all_adj_list.upper, list) {
			if (i == to_i && j == to_j)
				break;
			__netdev_adjacent_dev_unlink(i->dev, j->dev);
		}
		if (i == to_i)
			break;
	}

	__netdev_adjacent_dev_unlink_neighbour(dev, upper_dev);

	return ret;
}

/**
 * netdev_upper_dev_link - Add a link to the upper device
 * @dev: device
 * @upper_dev: new upper device
 *
 * Adds a link to device which is upper to this one. The caller must hold
 * the RTNL lock. On a failure a negative errno code is returned.
 * On success the reference counts are adjusted and the function
 * returns zero.
 */
int netdev_upper_dev_link(struct net_device *dev,
			  struct net_device *upper_dev)
{
	return __netdev_upper_dev_link(dev, upper_dev, false, NULL);
}
EXPORT_SYMBOL(netdev_upper_dev_link);

/**
 * netdev_master_upper_dev_link - Add a master link to the upper device
 * @dev: device
 * @upper_dev: new upper device
 *
 * Adds a link to device which is upper to this one. In this case, only
 * one master upper device can be linked, although other non-master devices
 * might be linked as well. The caller must hold the RTNL lock.
 * On a failure a negative errno code is returned. On success the reference
 * counts are adjusted and the function returns zero.
 */
int netdev_master_upper_dev_link(struct net_device *dev,
				 struct net_device *upper_dev)
{
	return __netdev_upper_dev_link(dev, upper_dev, true, NULL);
}
EXPORT_SYMBOL(netdev_master_upper_dev_link);

int netdev_master_upper_dev_link_private(struct net_device *dev,
					 struct net_device *upper_dev,
					 void *private)
{
	return __netdev_upper_dev_link(dev, upper_dev, true, private);
}
EXPORT_SYMBOL(netdev_master_upper_dev_link_private);

/**
 * netdev_upper_dev_unlink - Removes a link to upper device
 * @dev: device
 * @upper_dev: new upper device
 *
 * Removes a link to device which is upper to this one. The caller must hold
 * the RTNL lock.
 */
void netdev_upper_dev_unlink(struct net_device *dev,
			     struct net_device *upper_dev)
{
	struct netdev_adjacent *i, *j;
	ASSERT_RTNL();

	__netdev_adjacent_dev_unlink_neighbour(dev, upper_dev);

	/* Here is the tricky part. We must remove all dev's lower
	 * devices from all upper_dev's upper devices and vice
	 * versa, to maintain the graph relationship.
	 */
	list_for_each_entry(i, &dev->all_adj_list.lower, list)
		list_for_each_entry(j, &upper_dev->all_adj_list.upper, list)
			__netdev_adjacent_dev_unlink(i->dev, j->dev);

	/* remove also the devices itself from lower/upper device
	 * list
	 */
	list_for_each_entry(i, &dev->all_adj_list.lower, list)
		__netdev_adjacent_dev_unlink(i->dev, upper_dev);

	list_for_each_entry(i, &upper_dev->all_adj_list.upper, list)
		__netdev_adjacent_dev_unlink(dev, i->dev);

	call_netdevice_notifiers(NETDEV_CHANGEUPPER, dev);
}
EXPORT_SYMBOL(netdev_upper_dev_unlink);

/**
 * netdev_bonding_info_change - Dispatch event about slave change
 * @dev: device
 * @bonding_info: info to dispatch
 *
 * Send NETDEV_BONDING_INFO to netdev notifiers with info.
 * The caller must hold the RTNL lock.
 */
void netdev_bonding_info_change(struct net_device *dev,
				struct netdev_bonding_info *bonding_info)
{
	struct netdev_notifier_bonding_info	info;

	memcpy(&info.bonding_info, bonding_info,
	       sizeof(struct netdev_bonding_info));
	call_netdevice_notifiers_info(NETDEV_BONDING_INFO, dev,
				      &info.info);
}
EXPORT_SYMBOL(netdev_bonding_info_change);

static void netdev_adjacent_add_links(struct net_device *dev)
{
	struct netdev_adjacent *iter;

	struct net *net = dev_net(dev);

	list_for_each_entry(iter, &dev->adj_list.upper, list) {
		if (!net_eq(net,dev_net(iter->dev)))
			continue;
		netdev_adjacent_sysfs_add(iter->dev, dev,
					  &iter->dev->adj_list.lower);
		netdev_adjacent_sysfs_add(dev, iter->dev,
					  &dev->adj_list.upper);
	}

	list_for_each_entry(iter, &dev->adj_list.lower, list) {
		if (!net_eq(net,dev_net(iter->dev)))
			continue;
		netdev_adjacent_sysfs_add(iter->dev, dev,
					  &iter->dev->adj_list.upper);
		netdev_adjacent_sysfs_add(dev, iter->dev,
					  &dev->adj_list.lower);
	}
}

static void netdev_adjacent_del_links(struct net_device *dev)
{
	struct netdev_adjacent *iter;

	struct net *net = dev_net(dev);

	list_for_each_entry(iter, &dev->adj_list.upper, list) {
		if (!net_eq(net,dev_net(iter->dev)))
			continue;
		netdev_adjacent_sysfs_del(iter->dev, dev->name,
					  &iter->dev->adj_list.lower);
		netdev_adjacent_sysfs_del(dev, iter->dev->name,
					  &dev->adj_list.upper);
	}

	list_for_each_entry(iter, &dev->adj_list.lower, list) {
		if (!net_eq(net,dev_net(iter->dev)))
			continue;
		netdev_adjacent_sysfs_del(iter->dev, dev->name,
					  &iter->dev->adj_list.upper);
		netdev_adjacent_sysfs_del(dev, iter->dev->name,
					  &dev->adj_list.lower);
	}
}

void netdev_adjacent_rename_links(struct net_device *dev, char *oldname)
{
	struct netdev_adjacent *iter;

	struct net *net = dev_net(dev);

	list_for_each_entry(iter, &dev->adj_list.upper, list) {
		if (!net_eq(net,dev_net(iter->dev)))
			continue;
		netdev_adjacent_sysfs_del(iter->dev, oldname,
					  &iter->dev->adj_list.lower);
		netdev_adjacent_sysfs_add(iter->dev, dev,
					  &iter->dev->adj_list.lower);
	}

	list_for_each_entry(iter, &dev->adj_list.lower, list) {
		if (!net_eq(net,dev_net(iter->dev)))
			continue;
		netdev_adjacent_sysfs_del(iter->dev, oldname,
					  &iter->dev->adj_list.upper);
		netdev_adjacent_sysfs_add(iter->dev, dev,
					  &iter->dev->adj_list.upper);
	}
}

void *netdev_lower_dev_get_private(struct net_device *dev,
				   struct net_device *lower_dev)
{
	struct netdev_adjacent *lower;

	if (!lower_dev)
		return NULL;
	lower = __netdev_find_adj(dev, lower_dev, &dev->adj_list.lower);
	if (!lower)
		return NULL;

	return lower->private;
}
EXPORT_SYMBOL(netdev_lower_dev_get_private);


int dev_get_nest_level(struct net_device *dev,
		       bool (*type_check)(struct net_device *dev))
{
	struct net_device *lower = NULL;
	struct list_head *iter;
	int max_nest = -1;
	int nest;

	ASSERT_RTNL();

	netdev_for_each_lower_dev(dev, lower, iter) {
		nest = dev_get_nest_level(lower, type_check);
		if (max_nest < nest)
			max_nest = nest;
	}

	if (type_check(dev))
		max_nest++;

	return max_nest;
}
EXPORT_SYMBOL(dev_get_nest_level);

static void dev_change_rx_flags(struct net_device *dev, int flags)
{
	const struct net_device_ops *ops = dev->netdev_ops;

	if (ops->ndo_change_rx_flags)
		ops->ndo_change_rx_flags(dev, flags);
}

static int __dev_set_promiscuity(struct net_device *dev, int inc, bool notify)
{
	unsigned int old_flags = dev->flags;
	kuid_t uid;
	kgid_t gid;

	ASSERT_RTNL();

	dev->flags |= IFF_PROMISC;
	dev->promiscuity += inc;
	if (dev->promiscuity == 0) {
		/*
		 * Avoid overflow.
		 * If inc causes overflow, untouch promisc and return error.
		 */
		if (inc < 0)
			dev->flags &= ~IFF_PROMISC;
		else {
			dev->promiscuity -= inc;
			pr_warn("%s: promiscuity touches roof, set promiscuity failed. promiscuity feature of device might be broken.\n",
				dev->name);
			return -EOVERFLOW;
		}
	}
	if (dev->flags != old_flags) {
		pr_info("device %s %s promiscuous mode\n",
			dev->name,
			dev->flags & IFF_PROMISC ? "entered" : "left");
		if (audit_enabled) {
			current_uid_gid(&uid, &gid);
			audit_log(current->audit_context, GFP_ATOMIC,
				AUDIT_ANOM_PROMISCUOUS,
				"dev=%s prom=%d old_prom=%d auid=%u uid=%u gid=%u ses=%u",
				dev->name, (dev->flags & IFF_PROMISC),
				(old_flags & IFF_PROMISC),
				from_kuid(&init_user_ns, audit_get_loginuid(current)),
				from_kuid(&init_user_ns, uid),
				from_kgid(&init_user_ns, gid),
				audit_get_sessionid(current));
		}

		dev_change_rx_flags(dev, IFF_PROMISC);
	}
	if (notify)
		__dev_notify_flags(dev, old_flags, IFF_PROMISC);
	return 0;
}

/**
 *	dev_set_promiscuity	- update promiscuity count on a device
 *	@dev: device
 *	@inc: modifier
 *
 *	Add or remove promiscuity from a device. While the count in the device
 *	remains above zero the interface remains promiscuous. Once it hits zero
 *	the device reverts back to normal filtering operation. A negative inc
 *	value is used to drop promiscuity on the device.
 *	Return 0 if successful or a negative errno code on error.
 */
int dev_set_promiscuity(struct net_device *dev, int inc)
{
	unsigned int old_flags = dev->flags;
	int err;

	err = __dev_set_promiscuity(dev, inc, true);
	if (err < 0)
		return err;
	if (dev->flags != old_flags)
		dev_set_rx_mode(dev);
	return err;
}
EXPORT_SYMBOL(dev_set_promiscuity);

static int __dev_set_allmulti(struct net_device *dev, int inc, bool notify)
{
	unsigned int old_flags = dev->flags, old_gflags = dev->gflags;

	ASSERT_RTNL();

	dev->flags |= IFF_ALLMULTI;
	dev->allmulti += inc;
	if (dev->allmulti == 0) {
		/*
		 * Avoid overflow.
		 * If inc causes overflow, untouch allmulti and return error.
		 */
		if (inc < 0)
			dev->flags &= ~IFF_ALLMULTI;
		else {
			dev->allmulti -= inc;
			pr_warn("%s: allmulti touches roof, set allmulti failed. allmulti feature of device might be broken.\n",
				dev->name);
			return -EOVERFLOW;
		}
	}
	if (dev->flags ^ old_flags) {
		dev_change_rx_flags(dev, IFF_ALLMULTI);
		dev_set_rx_mode(dev);
		if (notify)
			__dev_notify_flags(dev, old_flags,
					   dev->gflags ^ old_gflags);
	}
	return 0;
}

/**
 *	dev_set_allmulti	- update allmulti count on a device
 *	@dev: device
 *	@inc: modifier
 *
 *	Add or remove reception of all multicast frames to a device. While the
 *	count in the device remains above zero the interface remains listening
 *	to all interfaces. Once it hits zero the device reverts back to normal
 *	filtering operation. A negative @inc value is used to drop the counter
 *	when releasing a resource needing all multicasts.
 *	Return 0 if successful or a negative errno code on error.
 */

int dev_set_allmulti(struct net_device *dev, int inc)
{
	return __dev_set_allmulti(dev, inc, true);
}
EXPORT_SYMBOL(dev_set_allmulti);

/*
 *	Upload unicast and multicast address lists to device and
 *	configure RX filtering. When the device doesn't support unicast
 *	filtering it is put in promiscuous mode while unicast addresses
 *	are present.
 */
void __dev_set_rx_mode(struct net_device *dev)
{
	const struct net_device_ops *ops = dev->netdev_ops;

	/* dev_open will call this function so the list will stay sane. */
	if (!(dev->flags&IFF_UP))
		return;

	if (!netif_device_present(dev))
		return;

	if (!(dev->priv_flags & IFF_UNICAST_FLT)) {
		/* Unicast addresses changes may only happen under the rtnl,
		 * therefore calling __dev_set_promiscuity here is safe.
		 */
		if (!netdev_uc_empty(dev) && !dev->uc_promisc) {
			__dev_set_promiscuity(dev, 1, false);
			dev->uc_promisc = true;
		} else if (netdev_uc_empty(dev) && dev->uc_promisc) {
			__dev_set_promiscuity(dev, -1, false);
			dev->uc_promisc = false;
		}
	}

	if (ops->ndo_set_rx_mode)
		ops->ndo_set_rx_mode(dev);
}

void dev_set_rx_mode(struct net_device *dev)
{
	netif_addr_lock_bh(dev);
	__dev_set_rx_mode(dev);
	netif_addr_unlock_bh(dev);
}

/**
 *	dev_get_flags - get flags reported to userspace
 *	@dev: device
 *
 *	Get the combination of flag bits exported through APIs to userspace.
 */
unsigned int dev_get_flags(const struct net_device *dev)
{
	unsigned int flags;

	flags = (dev->flags & ~(IFF_PROMISC |
				IFF_ALLMULTI |
				IFF_RUNNING |
				IFF_LOWER_UP |
				IFF_DORMANT)) |
		(dev->gflags & (IFF_PROMISC |
				IFF_ALLMULTI));

	if (netif_running(dev)) {
		if (netif_oper_up(dev))
			flags |= IFF_RUNNING;
		if (netif_carrier_ok(dev))
			flags |= IFF_LOWER_UP;
		if (netif_dormant(dev))
			flags |= IFF_DORMANT;
	}

	return flags;
}
EXPORT_SYMBOL(dev_get_flags);

int __dev_change_flags(struct net_device *dev, unsigned int flags)
{
	unsigned int old_flags = dev->flags;
	int ret;

	ASSERT_RTNL();

	/*
	 *	Set the flags on our device.
	 */

	dev->flags = (flags & (IFF_DEBUG | IFF_NOTRAILERS | IFF_NOARP |
			       IFF_DYNAMIC | IFF_MULTICAST | IFF_PORTSEL |
#if !defined(CONFIG_BCM_MPTCP) || !defined(CONFIG_BCM_KF_MPTCP)
			       IFF_AUTOMEDIA)) |
#else
			       IFF_AUTOMEDIA | IFF_NOMULTIPATH | IFF_MPBACKUP)) |
#endif
		     (dev->flags & (IFF_UP | IFF_VOLATILE | IFF_PROMISC |
				    IFF_ALLMULTI));

	/*
	 *	Load in the correct multicast list now the flags have changed.
	 */

	if ((old_flags ^ flags) & IFF_MULTICAST)
		dev_change_rx_flags(dev, IFF_MULTICAST);

	dev_set_rx_mode(dev);

	/*
	 *	Have we downed the interface. We handle IFF_UP ourselves
	 *	according to user attempts to set it, rather than blindly
	 *	setting it.
	 */

	ret = 0;
	if ((old_flags ^ flags) & IFF_UP)
		ret = ((old_flags & IFF_UP) ? __dev_close : __dev_open)(dev);

	if ((flags ^ dev->gflags) & IFF_PROMISC) {
		int inc = (flags & IFF_PROMISC) ? 1 : -1;
		unsigned int old_flags = dev->flags;

		dev->gflags ^= IFF_PROMISC;

		if (__dev_set_promiscuity(dev, inc, false) >= 0)
			if (dev->flags != old_flags)
				dev_set_rx_mode(dev);
	}

	/* NOTE: order of synchronization of IFF_PROMISC and IFF_ALLMULTI
	   is important. Some (broken) drivers set IFF_PROMISC, when
	   IFF_ALLMULTI is requested not asking us and not reporting.
	 */
	if ((flags ^ dev->gflags) & IFF_ALLMULTI) {
		int inc = (flags & IFF_ALLMULTI) ? 1 : -1;

		dev->gflags ^= IFF_ALLMULTI;
		__dev_set_allmulti(dev, inc, false);
	}

	return ret;
}

void __dev_notify_flags(struct net_device *dev, unsigned int old_flags,
			unsigned int gchanges)
{
	unsigned int changes = dev->flags ^ old_flags;

	if (gchanges)
		rtmsg_ifinfo(RTM_NEWLINK, dev, gchanges, GFP_ATOMIC);

	if (changes & IFF_UP) {
		if (dev->flags & IFF_UP)
			call_netdevice_notifiers(NETDEV_UP, dev);
		else
			call_netdevice_notifiers(NETDEV_DOWN, dev);
	}

	if (dev->flags & IFF_UP &&
	    (changes & ~(IFF_UP | IFF_PROMISC | IFF_ALLMULTI | IFF_VOLATILE))) {
		struct netdev_notifier_change_info change_info;

		change_info.flags_changed = changes;
		call_netdevice_notifiers_info(NETDEV_CHANGE, dev,
					      &change_info.info);
	}
}

/**
 *	dev_change_flags - change device settings
 *	@dev: device
 *	@flags: device state flags
 *
 *	Change settings on device based state flags. The flags are
 *	in the userspace exported format.
 */
int dev_change_flags(struct net_device *dev, unsigned int flags)
{
	int ret;
	unsigned int changes, old_flags = dev->flags, old_gflags = dev->gflags;

	ret = __dev_change_flags(dev, flags);
	if (ret < 0)
		return ret;

	changes = (old_flags ^ dev->flags) | (old_gflags ^ dev->gflags);
	__dev_notify_flags(dev, old_flags, changes);
	return ret;
}
EXPORT_SYMBOL(dev_change_flags);

static int __dev_set_mtu(struct net_device *dev, int new_mtu)
{
	const struct net_device_ops *ops = dev->netdev_ops;

	if (ops->ndo_change_mtu)
		return ops->ndo_change_mtu(dev, new_mtu);

	dev->mtu = new_mtu;
	return 0;
}

/**
 *	dev_set_mtu - Change maximum transfer unit
 *	@dev: device
 *	@new_mtu: new transfer unit
 *
 *	Change the maximum transfer size of the network device.
 */
int dev_set_mtu(struct net_device *dev, int new_mtu)
{
	int err, orig_mtu;

	if (new_mtu == dev->mtu)
		return 0;

	/*	MTU must be positive.	 */
	if (new_mtu < 0)
		return -EINVAL;

	if (!netif_device_present(dev))
		return -ENODEV;

	err = call_netdevice_notifiers(NETDEV_PRECHANGEMTU, dev);
	err = notifier_to_errno(err);
	if (err)
		return err;

	orig_mtu = dev->mtu;
	err = __dev_set_mtu(dev, new_mtu);

	if (!err) {
		err = call_netdevice_notifiers(NETDEV_CHANGEMTU, dev);
		err = notifier_to_errno(err);
		if (err) {
			/* setting mtu back and notifying everyone again,
			 * so that they have a chance to revert changes.
			 */
			__dev_set_mtu(dev, orig_mtu);
			call_netdevice_notifiers(NETDEV_CHANGEMTU, dev);
		}
	}
	return err;
}
EXPORT_SYMBOL(dev_set_mtu);

/**
 *	dev_set_group - Change group this device belongs to
 *	@dev: device
 *	@new_group: group this device should belong to
 */
void dev_set_group(struct net_device *dev, int new_group)
{
	dev->group = new_group;
}
EXPORT_SYMBOL(dev_set_group);

/**
 *	dev_set_mac_address - Change Media Access Control Address
 *	@dev: device
 *	@sa: new address
 *
 *	Change the hardware (MAC) address of the device
 */
int dev_set_mac_address(struct net_device *dev, struct sockaddr *sa)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	int err;

	if (!ops->ndo_set_mac_address)
		return -EOPNOTSUPP;
	if (sa->sa_family != dev->type)
		return -EINVAL;
	if (!netif_device_present(dev))
		return -ENODEV;
	err = ops->ndo_set_mac_address(dev, sa);
	if (err)
		return err;
	dev->addr_assign_type = NET_ADDR_SET;
	call_netdevice_notifiers(NETDEV_CHANGEADDR, dev);
	add_device_randomness(dev->dev_addr, dev->addr_len);
	return 0;
}
EXPORT_SYMBOL(dev_set_mac_address);

/**
 *	dev_change_carrier - Change device carrier
 *	@dev: device
 *	@new_carrier: new value
 *
 *	Change device carrier
 */
int dev_change_carrier(struct net_device *dev, bool new_carrier)
{
	const struct net_device_ops *ops = dev->netdev_ops;

	if (!ops->ndo_change_carrier)
		return -EOPNOTSUPP;
	if (!netif_device_present(dev))
		return -ENODEV;
	return ops->ndo_change_carrier(dev, new_carrier);
}
EXPORT_SYMBOL(dev_change_carrier);

/**
 *	dev_get_phys_port_id - Get device physical port ID
 *	@dev: device
 *	@ppid: port ID
 *
 *	Get device physical port ID
 */
int dev_get_phys_port_id(struct net_device *dev,
			 struct netdev_phys_item_id *ppid)
{
	const struct net_device_ops *ops = dev->netdev_ops;

	if (!ops->ndo_get_phys_port_id)
		return -EOPNOTSUPP;
	return ops->ndo_get_phys_port_id(dev, ppid);
}
EXPORT_SYMBOL(dev_get_phys_port_id);

/**
 *	dev_get_phys_port_name - Get device physical port name
 *	@dev: device
 *	@name: port name
 *
 *	Get device physical port name
 */
int dev_get_phys_port_name(struct net_device *dev,
			   char *name, size_t len)
{
	const struct net_device_ops *ops = dev->netdev_ops;

	if (!ops->ndo_get_phys_port_name)
		return -EOPNOTSUPP;
	return ops->ndo_get_phys_port_name(dev, name, len);
}
EXPORT_SYMBOL(dev_get_phys_port_name);

/**
 *	dev_new_index	-	allocate an ifindex
 *	@net: the applicable net namespace
 *
 *	Returns a suitable unique value for a new device interface
 *	number.  The caller must hold the rtnl semaphore or the
 *	dev_base_lock to be sure it remains unique.
 */
static int dev_new_index(struct net *net)
{
	int ifindex = net->ifindex;
#if defined(CONFIG_BCM_KF_LIMITED_IFINDEX)
	int loop = 0;
#endif

	for (;;) {
#if defined(CONFIG_BCM_KF_LIMITED_IFINDEX)
#define BCM_MAX_IFINDEX 128

		/* On DSL CPE, xtm interfaces are created/deleted when
		 * the link goes up/down. On a noisy dsl link that goes
		 * down and up frequently, the xtm ifindex may get higher
		 * and higher if we don't reuse the lower ifindex values.
		 * that were released when the interfaces were deleted.
		 * Therefore, we limit index value to BCM_MAX_IFINDEX,
		 * and try reuse ifindex that had been released.
		 */
		WARN_ONCE((++loop > BCM_MAX_IFINDEX),
			  "Cannot get new ifindex. All %d index values had "
			  "been used.\n", BCM_MAX_IFINDEX);
		/* try reuse ifindex that had been released. */
		if (ifindex >= BCM_MAX_IFINDEX)
			ifindex = 0;

#undef BCM_MAX_IFINDEX
#endif
		if (++ifindex <= 0)
			ifindex = 1;
		if (!__dev_get_by_index(net, ifindex))
			return net->ifindex = ifindex;
	}
}

/* Delayed registration/unregisteration */
static LIST_HEAD(net_todo_list);
DECLARE_WAIT_QUEUE_HEAD(netdev_unregistering_wq);

static void net_set_todo(struct net_device *dev)
{
	list_add_tail(&dev->todo_list, &net_todo_list);
	dev_net(dev)->dev_unreg_count++;
}

static void rollback_registered_many(struct list_head *head)
{
	struct net_device *dev, *tmp;
	LIST_HEAD(close_head);

	BUG_ON(dev_boot_phase);
	ASSERT_RTNL();

	list_for_each_entry_safe(dev, tmp, head, unreg_list) {
		/* Some devices call without registering
		 * for initialization unwind. Remove those
		 * devices and proceed with the remaining.
		 */
		if (dev->reg_state == NETREG_UNINITIALIZED) {
			pr_debug("unregister_netdevice: device %s/%p never was registered\n",
				 dev->name, dev);

			WARN_ON(1);
			list_del(&dev->unreg_list);
			continue;
		}
		dev->dismantle = true;
		BUG_ON(dev->reg_state != NETREG_REGISTERED);
	}

	/* If device is running, close it first. */
	list_for_each_entry(dev, head, unreg_list)
		list_add_tail(&dev->close_list, &close_head);
	dev_close_many(&close_head, true);

	list_for_each_entry(dev, head, unreg_list) {
		/* And unlink it from device chain. */
		unlist_netdevice(dev);

		dev->reg_state = NETREG_UNREGISTERING;
		on_each_cpu(flush_backlog, dev, 1);
	}

	synchronize_net();

	list_for_each_entry(dev, head, unreg_list) {
		struct sk_buff *skb = NULL;

		/* Shutdown queueing discipline. */
		dev_shutdown(dev);


		/* Notify protocols, that we are about to destroy
		   this device. They should clean all the things.
		*/
		call_netdevice_notifiers(NETDEV_UNREGISTER, dev);

		if (!dev->rtnl_link_ops ||
		    dev->rtnl_link_state == RTNL_LINK_INITIALIZED)
			skb = rtmsg_ifinfo_build_skb(RTM_DELLINK, dev, ~0U,
						     GFP_KERNEL);

		/*
		 *	Flush the unicast and multicast chains
		 */
		dev_uc_flush(dev);
		dev_mc_flush(dev);

		if (dev->netdev_ops->ndo_uninit)
			dev->netdev_ops->ndo_uninit(dev);

		if (skb)
			rtmsg_ifinfo_send(skb, dev, GFP_KERNEL);

		/* Notifier chain MUST detach us all upper devices. */
		WARN_ON(netdev_has_any_upper_dev(dev));

		/* Remove entries from kobject tree */
		netdev_unregister_kobject(dev);
#ifdef CONFIG_XPS
		/* Remove XPS queueing entries */
		netif_reset_xps_queues_gt(dev, 0);
#endif
	}

	synchronize_net();

	list_for_each_entry(dev, head, unreg_list)
		dev_put(dev);
}

static void rollback_registered(struct net_device *dev)
{
	LIST_HEAD(single);

	list_add(&dev->unreg_list, &single);
	rollback_registered_many(&single);
	list_del(&single);
}

static netdev_features_t netdev_fix_features(struct net_device *dev,
	netdev_features_t features)
{
	/* Fix illegal checksum combinations */
	if ((features & NETIF_F_HW_CSUM) &&
	    (features & (NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM))) {
		netdev_warn(dev, "mixed HW and IP checksum settings.\n");
		features &= ~(NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM);
	}

	/* TSO requires that SG is present as well. */
	if ((features & NETIF_F_ALL_TSO) && !(features & NETIF_F_SG)) {
		netdev_dbg(dev, "Dropping TSO features since no SG feature.\n");
		features &= ~NETIF_F_ALL_TSO;
	}

	if ((features & NETIF_F_TSO) && !(features & NETIF_F_HW_CSUM) &&
					!(features & NETIF_F_IP_CSUM)) {
		netdev_dbg(dev, "Dropping TSO features since no CSUM feature.\n");
		features &= ~NETIF_F_TSO;
		features &= ~NETIF_F_TSO_ECN;
	}

	if ((features & NETIF_F_TSO6) && !(features & NETIF_F_HW_CSUM) &&
					 !(features & NETIF_F_IPV6_CSUM)) {
		netdev_dbg(dev, "Dropping TSO6 features since no CSUM feature.\n");
		features &= ~NETIF_F_TSO6;
	}

	/* TSO ECN requires that TSO is present as well. */
	if ((features & NETIF_F_ALL_TSO) == NETIF_F_TSO_ECN)
		features &= ~NETIF_F_TSO_ECN;

	/* Software GSO depends on SG. */
	if ((features & NETIF_F_GSO) && !(features & NETIF_F_SG)) {
		netdev_dbg(dev, "Dropping NETIF_F_GSO since no SG feature.\n");
		features &= ~NETIF_F_GSO;
	}

	/* UFO needs SG and checksumming */
	if (features & NETIF_F_UFO) {
		/* maybe split UFO into V4 and V6? */
		if (!((features & NETIF_F_GEN_CSUM) ||
		    (features & (NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM))
			    == (NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM))) {
			netdev_dbg(dev,
				"Dropping NETIF_F_UFO since no checksum offload features.\n");
			features &= ~NETIF_F_UFO;
		}

		if (!(features & NETIF_F_SG)) {
			netdev_dbg(dev,
				"Dropping NETIF_F_UFO since no NETIF_F_SG feature.\n");
			features &= ~NETIF_F_UFO;
		}
	}

#ifdef CONFIG_NET_RX_BUSY_POLL
	if (dev->netdev_ops->ndo_busy_poll)
		features |= NETIF_F_BUSY_POLL;
	else
#endif
		features &= ~NETIF_F_BUSY_POLL;

	return features;
}

int __netdev_update_features(struct net_device *dev)
{
	netdev_features_t features;
	int err = 0;

	ASSERT_RTNL();

	features = netdev_get_wanted_features(dev);

	if (dev->netdev_ops->ndo_fix_features)
		features = dev->netdev_ops->ndo_fix_features(dev, features);

	/* driver might be less strict about feature dependencies */
	features = netdev_fix_features(dev, features);

	if (dev->features == features)
		return 0;

	netdev_dbg(dev, "Features changed: %pNF -> %pNF\n",
		&dev->features, &features);

	if (dev->netdev_ops->ndo_set_features)
		err = dev->netdev_ops->ndo_set_features(dev, features);

	if (unlikely(err < 0)) {
		netdev_err(dev,
			"set_features() failed (%d); wanted %pNF, left %pNF\n",
			err, &features, &dev->features);
		return -1;
	}

	if (!err)
		dev->features = features;

	return 1;
}

/**
 *	netdev_update_features - recalculate device features
 *	@dev: the device to check
 *
 *	Recalculate dev->features set and send notifications if it
 *	has changed. Should be called after driver or hardware dependent
 *	conditions might have changed that influence the features.
 */
void netdev_update_features(struct net_device *dev)
{
	if (__netdev_update_features(dev))
		netdev_features_change(dev);
}
EXPORT_SYMBOL(netdev_update_features);

/**
 *	netdev_change_features - recalculate device features
 *	@dev: the device to check
 *
 *	Recalculate dev->features set and send notifications even
 *	if they have not changed. Should be called instead of
 *	netdev_update_features() if also dev->vlan_features might
 *	have changed to allow the changes to be propagated to stacked
 *	VLAN devices.
 */
void netdev_change_features(struct net_device *dev)
{
	__netdev_update_features(dev);
	netdev_features_change(dev);
}
EXPORT_SYMBOL(netdev_change_features);

/**
 *	netif_stacked_transfer_operstate -	transfer operstate
 *	@rootdev: the root or lower level device to transfer state from
 *	@dev: the device to transfer operstate to
 *
 *	Transfer operational state from root to device. This is normally
 *	called when a stacking relationship exists between the root
 *	device and the device(a leaf device).
 */
void netif_stacked_transfer_operstate(const struct net_device *rootdev,
					struct net_device *dev)
{
	if (rootdev->operstate == IF_OPER_DORMANT)
		netif_dormant_on(dev);
	else
		netif_dormant_off(dev);

	if (netif_carrier_ok(rootdev)) {
		if (!netif_carrier_ok(dev))
			netif_carrier_on(dev);
	} else {
		if (netif_carrier_ok(dev))
			netif_carrier_off(dev);
	}
}
EXPORT_SYMBOL(netif_stacked_transfer_operstate);

#ifdef CONFIG_SYSFS
static int netif_alloc_rx_queues(struct net_device *dev)
{
	unsigned int i, count = dev->num_rx_queues;
	struct netdev_rx_queue *rx;
	size_t sz = count * sizeof(*rx);

	BUG_ON(count < 1);

	rx = kzalloc(sz, GFP_KERNEL | __GFP_NOWARN | __GFP_REPEAT);
	if (!rx) {
		rx = vzalloc(sz);
		if (!rx)
			return -ENOMEM;
	}
	dev->_rx = rx;

	for (i = 0; i < count; i++)
		rx[i].dev = dev;
	return 0;
}
#endif

static void netdev_init_one_queue(struct net_device *dev,
				  struct netdev_queue *queue, void *_unused)
{
	/* Initialize queue lock */
	spin_lock_init(&queue->_xmit_lock);
	netdev_set_xmit_lockdep_class(&queue->_xmit_lock, dev->type);
	queue->xmit_lock_owner = -1;
	netdev_queue_numa_node_write(queue, NUMA_NO_NODE);
	queue->dev = dev;
#ifdef CONFIG_BQL
	dql_init(&queue->dql, HZ);
#endif
}

static void netif_free_tx_queues(struct net_device *dev)
{
	kvfree(dev->_tx);
}

static int netif_alloc_netdev_queues(struct net_device *dev)
{
	unsigned int count = dev->num_tx_queues;
	struct netdev_queue *tx;
	size_t sz = count * sizeof(*tx);

	if (count < 1 || count > 0xffff)
		return -EINVAL;

	tx = kzalloc(sz, GFP_KERNEL | __GFP_NOWARN | __GFP_REPEAT);
	if (!tx) {
		tx = vzalloc(sz);
		if (!tx)
			return -ENOMEM;
	}
	dev->_tx = tx;

	netdev_for_each_tx_queue(dev, netdev_init_one_queue, NULL);
	spin_lock_init(&dev->tx_global_lock);

	return 0;
}

#if defined(CONFIG_BCM_KF_SW_GSO) && defined(CONFIG_BCM_SW_GSO)

#define BCM_SW_GSO_MAX_HEADERS  16
#define BCM_SW_GSO_MAX_SEGS  64
#define BCM_L3_PROTO_IPV4 	1
#define BCM_L3_PROTO_IPV6 	2

enum {
	BCM_SW_GSO_SEG_TYPE_SINGLE=0,
	BCM_SW_GSO_SEG_TYPE_FIRST,
	BCM_SW_GSO_SEG_TYPE_MIDDLE,
	BCM_SW_GSO_SEG_TYPE_LAST,
	BCM_SW_GSO_SEG_TYPE_MAX
};

struct gso_hdrs{
	void *ethhdr; /*including VLAN's */
	void *ppphdr;
	void *ipv4hdr;
	void *ipv6hdr;
	void *l4hdr;
	union {
		struct {
			uint8_t ethhdrlen;/*including VLAN's */
			uint8_t ppphdrlen;
			uint8_t ipv4hdrlen;
			uint8_t ipv6hdrlen;
			uint8_t l4hdrlen;
			uint8_t totlen;
			uint8_t l3proto;
			uint8_t l4proto;
		};
		uint64_t lenproto;
	};
	void *txdev;
	HardStartXmitFuncP xmit_fn;
	uint32_t ipv6_flowlbl;
	uint32_t tcpseq;
	uint32_t ipv6_fragid;
	uint16_t ipv4_id;
 	uint16_t ip_fragoff;
	uint8_t ip_fragneeded;
	uint8_t ip_lastfrag;
	uint8_t tcp_segtype;
};

static inline void bcm_push_hdr(FkBuff_t *fkb, void *src, unsigned int len)
{
	fkb->data -= len;
	fkb->len  +=len;
	memcpy(fkb->data, src, len);
}

static inline int bcm_add_l2hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	if(hdrs->ppphdrlen)
	{
		bcm_push_hdr(fkb, hdrs->ppphdr, hdrs->ppphdrlen);
		/*TODO adjust ppphdr len ip +2 */
	}
	bcm_push_hdr(fkb, hdrs->ethhdr, hdrs->ethhdrlen);
	return 0;
}

static inline int bcm_add_l3hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	if(hdrs->l3proto == BCM_L3_PROTO_IPV4){
		struct iphdr *ipv4;
		bcm_push_hdr(fkb, hdrs->ipv4hdr, hdrs->ipv4hdrlen);
		ipv4 = (struct iphdr *)fkb->data;
		/* update len   */
		 ipv4->tot_len = htons(fkb->len);

		 if(hdrs->ip_fragneeded){
			 /* update offset */
			 ipv4->frag_off = htons((hdrs->ip_fragoff >> 3));
			 /* update flags */
			 if( !hdrs->ip_lastfrag)
				 ipv4->frag_off |= htons(IP_MF);
		 }

		/* update IPID  */

		/* update csum  */
		ipv4->check = 0;
		ipv4->check = ip_fast_csum((unsigned char *)ipv4, ipv4->ihl);
	}
	else if(hdrs->l3proto == BCM_L3_PROTO_IPV6){
		struct ipv6hdr *ipv6;

		if(hdrs->ip_fragneeded){
			struct frag_hdr fh;

			fh.nexthdr = ((struct ipv6hdr *)hdrs->ipv6hdr)->nexthdr;
			fh.reserved = 0;
			fh.identification = hdrs->ipv6_fragid;
			fh.frag_off = htons(hdrs->ip_fragoff);
			bcm_push_hdr(fkb, &fh, sizeof(struct frag_hdr));
		}

		bcm_push_hdr(fkb, hdrs->ipv6hdr, hdrs->ipv6hdrlen);
		ipv6 = (struct ipv6hdr *)fkb->data;

		/* update len   */
		ipv6->payload_len = htons( fkb->len - hdrs->ipv6hdrlen);

		if(hdrs->ip_fragneeded){
			ipv6->nexthdr =NEXTHDR_FRAGMENT;
		}

		/* update IPID  */
	    /* update flags */
	}
	else{
		printk(KERN_ERR "%s:Unsuppoerted L3 protocol %d\n", __func__, hdrs->l3proto);
		return -1;
	}
	return 0;
}

static inline __sum16 bcm_sw_gso_l4_csum(struct gso_hdrs *hdrs, uint32_t len, uint16_t proto, __wsum base)
{
	if(hdrs->l3proto == BCM_L3_PROTO_IPV4 ){
		struct iphdr *ipv4= hdrs->ipv4hdr;
		return csum_tcpudp_magic(ipv4->saddr, ipv4->daddr, len, proto, base);
	} else {
		struct ipv6hdr *ipv6 = hdrs->ipv6hdr;
		return csum_ipv6_magic(&ipv6->saddr, &ipv6->daddr, len, proto, base);
	}
}

static inline void bcm_add_tcphdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	struct tcphdr *th;
	bcm_push_hdr(fkb, hdrs->l4hdr, hdrs->l4hdrlen);
	/* update seq */
	th = (struct tcphdr *)fkb->data;
	th->seq = htonl(hdrs->tcpseq);

	if(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_SINGLE){

		/* clear cwr in all packets except first */
		if(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_FIRST)
			th->cwr = 0;

		/* clear fin, psh in all packets except last */
		if((hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_LAST))
			th->fin = th->psh = 0;
	}

	th->check = 0;
	th->check = bcm_sw_gso_l4_csum(hdrs, fkb->len, IPPROTO_TCP, csum_partial(fkb->data, fkb->len, 0));
}

static void bcm_sw_gso_recycle_func(void *pNBuff, unsigned long context, uint32_t flags)
{
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
	gbpm_free_buf(PFKBUFF_TO_PDATA(PNBUFF_2_PBUF(pNBuff), BCM_PKT_HEADROOM));
#else
	kmem_cache_free(pktBufferCache, PNBUFF_2_PBUF(pNBuff));
#endif
}

static inline int bcm_sw_alloc_pkt_buffers(int npkts, void **buffer_pool)
{
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
	/* allocate buffers from BPM */
	if( gbpm_alloc_mult_buf(npkts, buffer_pool) == GBPM_ERROR ){
		return -1;
	}
	return 0;
#else
	int i;
	void *buf; 
	
	for(i=0; i < npkts; i++){
		buf = kmem_cache_alloc(pktBufferCache, GFP_ATOMIC);
		if(buf == NULL)
			break;
		buffer_pool[i] = (void *)PFKBUFF_TO_PDATA((void *)(buf), BCM_PKT_HEADROOM);
	}

	/* check if we allocated all the buffers requested */
	if( i && i != npkts){
		printk("%s %s: Not enough memory for packet buffer allocation\n", __FILE__, __FUNCTION__);
		npkts =i;	
		for(i=0; i<npkts; i++)
			kmem_cache_free(pktBufferCache, PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM));

		return -1;
	}
	return 0;
#endif
}

static inline int bcm_sw_gso_tcp_segment(struct sk_buff *skb, struct gso_hdrs *hdrs)
{
	int npkts;
	FkBuff_t *fkb;
	void *pbuf;
	unsigned short bytesleft;
	unsigned short offset;
	unsigned short mss;
	unsigned short payloadlen;
	int cur_pkt=0;
	/*TODO do kmalloc if stack size is an issue*/
	void *buffer_pool[BCM_SW_GSO_MAX_SEGS];

	/* calculate the number of packets needed to transmit this skb */

	bytesleft = skb->len - hdrs->totlen;
	offset = hdrs->totlen;

	mss = skb_shinfo(skb)->gso_size;

	if(mss == 0)
	{
		mss = bytesleft;
		npkts =1;
	}	
	else
		npkts = DIV_ROUND_UP(bytesleft, mss);

	if(npkts > BCM_SW_GSO_MAX_SEGS){
		printk(KERN_ERR "%s: npkts=%d greater than max segs(%d) \n", 
				__func__, npkts, BCM_SW_GSO_MAX_SEGS);
		return -1;
	}

	if( bcm_sw_alloc_pkt_buffers(npkts, buffer_pool) != 0 )
		return -1;

	hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_FIRST;

	do {

		if(cur_pkt > npkts){
			/* we shoud never be here,if we are then most likely there is
			 * some thing wrong with mss, allocated packets are already xmitted,
			 * no need to free them here
			 */ 
			printk(KERN_ERR "%s:error pktcount =%d > allocated buffers=%d mss=%d\n",
					__func__, cur_pkt, npkts, mss);
			return -1;
		}
		/* initialize fkb */
		pbuf = buffer_pool[cur_pkt++];

		fkb = fkb_init(pbuf, BCM_PKT_HEADROOM, pbuf + hdrs->totlen, 0);
		fkb->recycle_hook = bcm_sw_gso_recycle_func;
		fkb->recycle_context = 0;
		fkb->mark = skb->mark;
		fkb->priority = skb->priority;

		payloadlen = min(mss, bytesleft);

		/* copy data from original skb to new packet */

		/* copy tcp payload */
		skb_copy_bits(skb, offset, fkb->data, payloadlen);
		fkb->len += payloadlen;

		offset +=payloadlen;
		bytesleft -=payloadlen;

		if(bytesleft == 0){
			if(likely(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_FIRST))
				hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_LAST;
			else
				hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_SINGLE;
		}

		/* copy tcp hdr & update fields */
		bcm_add_tcphdr(fkb, hdrs);

		hdrs->tcpseq +=payloadlen;
		hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_MIDDLE;

		/* copy l3 hdr  */
		bcm_add_l3hdr(fkb, hdrs);

		/* copy l2 hdr  */
		bcm_add_l2hdr(fkb,hdrs);


		fkb->dirty_p = _to_dptr_from_kptr_(fkb->data + fkb->len);
		/* xmit packet */
		hdrs->xmit_fn(FKBUFF_2_PNBUFF(fkb), hdrs->txdev);

	} while (bytesleft);

	if(cur_pkt < npkts){
		int i;
		printk(KERN_ERR "%s:error pktcount =%d < allocated buffers=%d\n", __func__, cur_pkt, npkts);
		for(i=cur_pkt; i < npkts; i++)
			bcm_sw_gso_recycle_func(PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM), 0, 0 );
	}

	return 0;
}

static uint32_t bcm_sw_gso_gen_ipv6id(void)
{
	static atomic_t ipv6_fragmentation_id;
	uint32_t old, new;

	do {
		old = atomic_read(&ipv6_fragmentation_id);
		new = old + 1;
		if (!new)
			new = 1;
	} while (atomic_cmpxchg(&ipv6_fragmentation_id, old, new) != old);
	return new;
}

static int bcm_sw_gso_ip_fragment(struct sk_buff *skb, struct gso_hdrs *hdrs)
{
	int npkts;
	FkBuff_t *fkb;
	void *pbuf;
	unsigned short bytesleft;
	unsigned short offset;
	unsigned short mss;
	unsigned short payloadlen;
	int cur_pkt=0;
	/*TODO do kmalloc if stack size is an issue*/
	void *buffer_pool[BCM_SW_GSO_MAX_SEGS];


	/* calculate the number of packets needed to transmit this skb */
	bytesleft = skb->len - (hdrs->totlen - hdrs->l4hdrlen);
	offset = hdrs->totlen - hdrs->l4hdrlen;
	
	mss = skb_shinfo(skb)->gso_size;

	if(mss == 0){
		mss = bytesleft;
		npkts =1;
	}	
	else{
		mss = mss & ~7; /* make it multiple of 8 */
		npkts = DIV_ROUND_UP(bytesleft, mss);
	}

	if(npkts > BCM_SW_GSO_MAX_SEGS){
		printk(KERN_ERR "%s: npkts=%d greater than max segs(%d) \n", 
				__func__, npkts, BCM_SW_GSO_MAX_SEGS);
		return -1;
	}

	if( bcm_sw_alloc_pkt_buffers(npkts, buffer_pool) != 0 )
		return -1;

	hdrs->ip_fragoff = 0;
	hdrs->ip_lastfrag = 0;
	if(bytesleft - mss){
		/* perform IP fragmentation */
		hdrs->ip_fragneeded = 1;
	}

	if(hdrs->l3proto == BCM_L3_PROTO_IPV6){
		hdrs->ipv6_fragid = htonl(bcm_sw_gso_gen_ipv6id());
	}

	/* perform UDPchecksum */
	if( (hdrs->l4proto == IPPROTO_UDP) && (skb->ip_summed == CHECKSUM_PARTIAL))
	{
		struct udphdr *uh= hdrs->l4hdr;
		uh->check = 0;
		int csum_offset = hdrs->l4hdr - hdrs->ethhdr;

		uh->check = bcm_sw_gso_l4_csum(hdrs, skb->len-csum_offset, IPPROTO_UDP,
				skb_checksum(skb, csum_offset, skb->len-csum_offset, 0));
	}

	do{

		if(cur_pkt > npkts){
			/* we shoud never be here,if we are then most likely there is
			 * some thing wrong with mss, allocated packets are already xmitted,
			 * no need to free them here
			 */ 
			printk(KERN_ERR "%s:error pktcount =%d > allocated buffers=%d mss=%d\n",
					__func__, cur_pkt, npkts, mss);
			return -1;
		}
		/* initialize fkb */
		pbuf = buffer_pool[cur_pkt++];

		fkb = fkb_init(pbuf, BCM_PKT_HEADROOM, pbuf + hdrs->totlen, 0);
		fkb->recycle_hook = bcm_sw_gso_recycle_func;
		fkb->recycle_context = 0;
		fkb->mark = skb->mark;
		fkb->priority = skb->priority;

		payloadlen = min(mss, bytesleft);

		/* copy data from original skb to new packet */

		/* copy tcp payload */
		skb_copy_bits(skb, offset, fkb->data, payloadlen);
		fkb->len += payloadlen;


		offset +=payloadlen;
		bytesleft -=payloadlen;

		if(bytesleft == 0)
			hdrs->ip_lastfrag = 1;

		/* copy l3 hdr  */
		bcm_add_l3hdr(fkb, hdrs);

		hdrs->ip_fragoff +=payloadlen;

		/* copy l2 hdr  */
		bcm_add_l2hdr(fkb,hdrs);

		fkb->dirty_p = _to_dptr_from_kptr_(fkb->data + fkb->len);
		/* xmit packet */
		hdrs->xmit_fn(FKBUFF_2_PNBUFF(fkb), hdrs->txdev);

	}while (bytesleft);

	if(cur_pkt < npkts){
		int i;
		printk(KERN_ERR "%s:error pktcount =%d < allocated buffers=%d\n", __func__, cur_pkt, npkts);
		for(i=cur_pkt; i< npkts; i++)
			bcm_sw_gso_recycle_func(PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM), 0, 0 );
	}

	return 0;
}


static inline int bcm_parse_gso_hdrs(struct sk_buff *skb, struct gso_hdrs *hdrs)
{
	struct iphdr *ipv4;
	struct ipv6hdr *ipv6;
	struct tcphdr *tcp_hdr;
    int header = -1;
	void *data = skb->data;
	unsigned int skbheadlen = skb_headlen(skb);
	/*assumes first header is ETH always */
	uint16_t proto = ETH_P_802_3;

    while(1)
    {
        header++;

		if(hdrs->totlen > skbheadlen){
			printk(KERN_ERR "%s: headers not present in linear data hdrslen=%u skbheadlen=%d \n",
				 __func__, hdrs->totlen, skbheadlen);
			return -1;
		}

        if (header > BCM_SW_GSO_MAX_HEADERS){
            printk(KERN_ERR "%s:Too many headers <%d>\n", __func__, header);
			return -1;
        }

        switch(proto)
        {
            case ETH_P_802_3:  /* first encap: XYZoE */

                if(header != 0)
                {
                    return -1;
                }
				hdrs->ethhdr = data;

				proto = ntohs(((struct ethhdr *)data)->h_proto);
				hdrs->ethhdrlen = ETH_HLEN;
				hdrs->totlen += ETH_HLEN;
				data += ETH_HLEN;
                break;

            case ETH_P_8021Q: 
            case ETH_P_8021AD: 

				proto = ntohs(*((uint16_t *)(data+2)));
				hdrs->ethhdrlen += 4;
				hdrs->totlen += 4;
				data += 4;
                break;

            case ETH_P_IP:
				 
				ipv4 = data;

				hdrs->l3proto = BCM_L3_PROTO_IPV4;
				proto = ipv4->protocol;

                if ((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP)){

					hdrs->ipv4hdr = data;
					hdrs->ipv4_id = ntohs(ipv4->id);
					hdrs->ipv4hdrlen = ipv4->ihl<<2;
					hdrs->totlen += ipv4->ihl<<2;
					data += ipv4->ihl<<2;

                } else {
                    printk(KERN_ERR "%s Unsupported L4 type=%u \n", __func__, proto);
                    return -1;
                }
				break;

            case ETH_P_IPV6:

				ipv6 = data;

				hdrs->l3proto = BCM_L3_PROTO_IPV6;
				proto = ipv6->nexthdr;

                if ((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP)){

					hdrs->ipv6hdr = data;
					hdrs->ipv6_flowlbl = ntohl(ip6_flowlabel(ipv6));
					hdrs->ipv6hdrlen = sizeof(struct ipv6hdr);
					hdrs->totlen += sizeof(struct ipv6hdr);
					data += sizeof(struct ipv6hdr);

                } else {
                    printk(KERN_ERR "%s Unsupported L4 type=%u \n", __func__, proto);
                    return -1;
                }
				break;

			case IPPROTO_TCP :
				tcp_hdr = data;

				hdrs->l4hdr = data;
				hdrs->l4proto = IPPROTO_TCP;
				hdrs->l4hdrlen = tcp_hdr->doff <<2;
				hdrs->totlen += tcp_hdr->doff <<2;
				hdrs->tcpseq = ntohl(tcp_hdr->seq);
				return  0 ;

			case IPPROTO_UDP :
				hdrs->l4proto = IPPROTO_UDP;
				hdrs->l4hdr = data;
				hdrs->l4hdrlen = sizeof(struct udphdr) ;
				hdrs->totlen += sizeof(struct udphdr);
				return  0 ;

			/*TODO add PPP */

            default :
                printk(KERN_ERR "%s:UNSUPPORTED protocol %u ", __func__, proto);
                return -1;
        } /* switch (headerType) */
    }
	return -1;
}


static inline int bcm_sw_gso(struct sk_buff *skb, struct net_device *txdev, HardStartXmitFuncP xmit_fn)
{
	struct gso_hdrs hdrs;
	int ret=-1;

	/*check if this skb needs GSO processing*/
	if(skb_is_gso(skb) || skb_shinfo(skb)->nr_frags)
	{

		memset(&hdrs,0, sizeof(struct gso_hdrs));
		/* extract l2 & l3 headers and l4 proto */
		if( bcm_parse_gso_hdrs(skb, &hdrs) < 0){
			ret = -1;
			goto done;
		};

		hdrs.txdev = txdev;
		hdrs.xmit_fn =xmit_fn;

		/*perform GSO and transmit packets*/

		switch(hdrs.l4proto) {
			case IPPROTO_TCP:
				ret = bcm_sw_gso_tcp_segment(skb, &hdrs);
				break;

			case IPPROTO_UDP:
				ret = bcm_sw_gso_ip_fragment(skb, &hdrs);
				break;

			default:
				printk(KERN_ERR "%s:SW GSO not supported for protocol=%d \n",__func__, hdrs.l4proto); 
				ret= -1;
				goto done;
		}
	} else if((skb->ip_summed == CHECKSUM_PARTIAL)){
		/*only csum is needed */
		memset(&hdrs,0, sizeof(struct gso_hdrs));
		if( bcm_parse_gso_hdrs(skb, &hdrs) < 0){
			ret = -1;
			goto done;
		};

		switch(hdrs.l4proto) {
			case IPPROTO_TCP:
			{
			 	struct tcphdr *th= hdrs.l4hdr;
				th->check = 0;
				/*TODO calulate l4 len based on l3 len */
				th->check = bcm_sw_gso_l4_csum(&hdrs, skb->len-(hdrs.l4hdr - hdrs.ethhdr),
						IPPROTO_TCP, csum_partial(hdrs.l4hdr, skb->len-(hdrs.l4hdr - hdrs.ethhdr), 0));
				break;
			}

			case IPPROTO_UDP:
			{
			 	struct udphdr *uh= hdrs.l4hdr;
				uh->check = 0;
				/*TODO calulate l4 len based on l3 len */
				uh->check = bcm_sw_gso_l4_csum(&hdrs, skb->len-(hdrs.l4hdr - hdrs.ethhdr),
						IPPROTO_UDP, csum_partial(hdrs.l4hdr, skb->len-(hdrs.l4hdr - hdrs.ethhdr), 0));
				break;
			}

			default:
				printk(KERN_ERR "%s:SW GSO not supported for protocol=%d \n",__func__, hdrs.l4proto); 
				ret= -1;
				goto done;
		}

		skb->ip_summed = CHECKSUM_NONE;
		xmit_fn(SKBUFF_2_PNBUFF(skb), txdev);
		return 0;
	}else{
		/* transmit skb as is */
		xmit_fn(SKBUFF_2_PNBUFF(skb), txdev);
		return 0;
	}
done:
	/*free the original skb & return */
	dev_kfree_skb_any(skb);
	return ret;
}

int bcm_sw_gso_xmit(struct sk_buff *skb, struct net_device *txdev, HardStartXmitFuncP xmit_fn)
{
	return bcm_sw_gso(skb, txdev, xmit_fn);
}
EXPORT_SYMBOL(bcm_sw_gso_xmit);

#endif /* defined(CONFIG_BCM_KF_SW_GSO) && defined(CONFIG_BCM_SW_GSO) */

#if (defined(CONFIG_BCM_KF_FAP_GSO_LOOPBACK) && defined(CONFIG_BCM_FAP_GSO_LOOPBACK))
int (*bcm_gso_loopback_hw_offload)(struct sk_buff *skb, unsigned int txDevId) = NULL;
EXPORT_SYMBOL(bcm_gso_loopback_hw_offload);

struct net_device *bcm_gso_loopback_devs[BCM_GSO_LOOPBACK_MAXDEVS];

void bcm_gso_loopback_devs_init(void)
{
	unsigned int i;
	for (i = 0; i < BCM_GSO_LOOPBACK_MAXDEVS; i++) {
		bcm_gso_loopback_devs[i] = NULL;
	}
}

static inline void bcm_gso_loopback_add_devptr(struct net_device *dev)
{
	if (!strcmp("wl0", dev->name))
		bcm_gso_loopback_devs[BCM_GSO_LOOPBACK_WL0] = dev;
	else if (!strcmp("wl1", dev->name))
		bcm_gso_loopback_devs[BCM_GSO_LOOPBACK_WL1] = dev;
	else if (!strcmp("wl2", dev->name))
		bcm_gso_loopback_devs[BCM_GSO_LOOPBACK_WL2] = dev;
	else	/* not a know device */
		return;

	printk("+++++ Added gso loopback support for dev=%s <%p>\n",
		dev->name, dev);
}

static inline void bcm_gso_loopback_del_devptr(struct net_device *dev)
{
	if (!strcmp("wl0", dev->name))
		bcm_gso_loopback_devs[BCM_GSO_LOOPBACK_WL0] = NULL;
	else if (!strcmp("wl1", dev->name))
		bcm_gso_loopback_devs[BCM_GSO_LOOPBACK_WL1] = NULL;
	else if (!strcmp("wl2", dev->name))
		bcm_gso_loopback_devs[BCM_GSO_LOOPBACK_WL2] = NULL;
	else	/* not a know device */
		return;

	printk("------ Removed gso loopback support for dev=%s <%p>\n",
		dev->name, dev);
}

inline unsigned int bcm_is_gso_loopback_dev(void *dev)
{
	int i;

	for (i = 1; i < BCM_GSO_LOOPBACK_MAXDEVS; i++) {
		if (bcm_gso_loopback_devs[i] == dev)
			return i;
	}

	return BCM_GSO_LOOPBACK_NONE;
}

unsigned int bcm_gso_loopback_devptr2devid(void *dev)
{
	return bcm_is_gso_loopback_dev(dev);
}
EXPORT_SYMBOL(bcm_gso_loopback_devptr2devid);

struct net_device *bcm_gso_loopback_devid2devptr(unsigned int devId)
{
	if (devId < BCM_GSO_LOOPBACK_MAXDEVS)
		return bcm_gso_loopback_devs[devId];
	else {
		printk(KERN_ERR "%s: invalid devId<%d> max devs=%d\n",
				__func__, devId, BCM_GSO_LOOPBACK_MAXDEVS);
		return NULL;
	}
}
EXPORT_SYMBOL(bcm_gso_loopback_devid2devptr);
#endif

/**
 *	register_netdevice	- register a network device
 *	@dev: device to register
 *
 *	Take a completed network device structure and add it to the kernel
 *	interfaces. A %NETDEV_REGISTER message is sent to the netdev notifier
 *	chain. 0 is returned on success. A negative errno code is returned
 *	on a failure to set up the device, or if the name is a duplicate.
 *
 *	Callers must hold the rtnl semaphore. You may want
 *	register_netdev() instead of this.
 *
 *	BUGS:
 *	The locking appears insufficient to guarantee two parallel registers
 *	will not get the same name.
 */

int register_netdevice(struct net_device *dev)
{
	int ret;
	struct net *net = dev_net(dev);

	BUG_ON(dev_boot_phase);
	ASSERT_RTNL();

	might_sleep();

	/* When net_device's are persistent, this will be fatal. */
	BUG_ON(dev->reg_state != NETREG_UNINITIALIZED);
	BUG_ON(!net);

	spin_lock_init(&dev->addr_list_lock);
	netdev_set_addr_lockdep_class(dev);

	ret = dev_get_valid_name(net, dev, dev->name);
	if (ret < 0)
		goto out;

	/* Init, if this function is available */
	if (dev->netdev_ops->ndo_init) {
		ret = dev->netdev_ops->ndo_init(dev);
		if (ret) {
			if (ret > 0)
				ret = -EIO;
			goto out;
		}
	}

	if (((dev->hw_features | dev->features) &
	     NETIF_F_HW_VLAN_CTAG_FILTER) &&
	    (!dev->netdev_ops->ndo_vlan_rx_add_vid ||
	     !dev->netdev_ops->ndo_vlan_rx_kill_vid)) {
		netdev_WARN(dev, "Buggy VLAN acceleration in driver!\n");
		ret = -EINVAL;
		goto err_uninit;
	}

	ret = -EBUSY;
	if (!dev->ifindex)
		dev->ifindex = dev_new_index(net);
	else if (__dev_get_by_index(net, dev->ifindex))
		goto err_uninit;

	/* Transfer changeable features to wanted_features and enable
	 * software offloads (GSO and GRO).
	 */
	dev->hw_features |= NETIF_F_SOFT_FEATURES;
	dev->features |= NETIF_F_SOFT_FEATURES;
	dev->wanted_features = dev->features & dev->hw_features;

	if (!(dev->flags & IFF_LOOPBACK)) {
		dev->hw_features |= NETIF_F_NOCACHE_COPY;
	}

	/* Make NETIF_F_HIGHDMA inheritable to VLAN devices.
	 */
	dev->vlan_features |= NETIF_F_HIGHDMA;

	/* Make NETIF_F_SG inheritable to tunnel devices.
	 */
	dev->hw_enc_features |= NETIF_F_SG;

	/* Make NETIF_F_SG inheritable to MPLS.
	 */
	dev->mpls_features |= NETIF_F_SG;

	ret = call_netdevice_notifiers(NETDEV_POST_INIT, dev);
	ret = notifier_to_errno(ret);
	if (ret)
		goto err_uninit;

	ret = netdev_register_kobject(dev);
	if (ret)
		goto err_uninit;
	dev->reg_state = NETREG_REGISTERED;

	__netdev_update_features(dev);

	/*
	 *	Default initial state at registry is that the
	 *	device is present.
	 */

	set_bit(__LINK_STATE_PRESENT, &dev->state);

	linkwatch_init_dev(dev);

	dev_init_scheduler(dev);
	dev_hold(dev);
	list_netdevice(dev);
	add_device_randomness(dev->dev_addr, dev->addr_len);

	/* If the device has permanent device address, driver should
	 * set dev_addr and also addr_assign_type should be set to
	 * NET_ADDR_PERM (default value).
	 */
	if (dev->addr_assign_type == NET_ADDR_PERM)
		memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);

#if (defined(CONFIG_BCM_KF_FAP_GSO_LOOPBACK) && defined(CONFIG_BCM_FAP_GSO_LOOPBACK))
	bcm_gso_loopback_add_devptr(dev);
#endif

	/* Notify protocols, that a new device appeared. */
	ret = call_netdevice_notifiers(NETDEV_REGISTER, dev);
	ret = notifier_to_errno(ret);
	if (ret) {
		rollback_registered(dev);
		dev->reg_state = NETREG_UNREGISTERED;
	}
	/*
	 *	Prevent userspace races by waiting until the network
	 *	device is fully setup before sending notifications.
	 */
	if (!dev->rtnl_link_ops ||
	    dev->rtnl_link_state == RTNL_LINK_INITIALIZED)
		rtmsg_ifinfo(RTM_NEWLINK, dev, ~0U, GFP_KERNEL);

out:
	return ret;

err_uninit:
	if (dev->netdev_ops->ndo_uninit)
		dev->netdev_ops->ndo_uninit(dev);
	goto out;
}
EXPORT_SYMBOL(register_netdevice);

/**
 *	init_dummy_netdev	- init a dummy network device for NAPI
 *	@dev: device to init
 *
 *	This takes a network device structure and initialize the minimum
 *	amount of fields so it can be used to schedule NAPI polls without
 *	registering a full blown interface. This is to be used by drivers
 *	that need to tie several hardware interfaces to a single NAPI
 *	poll scheduler due to HW limitations.
 */
int init_dummy_netdev(struct net_device *dev)
{
	/* Clear everything. Note we don't initialize spinlocks
	 * are they aren't supposed to be taken by any of the
	 * NAPI code and this dummy netdev is supposed to be
	 * only ever used for NAPI polls
	 */
	memset(dev, 0, sizeof(struct net_device));

	/* make sure we BUG if trying to hit standard
	 * register/unregister code path
	 */
	dev->reg_state = NETREG_DUMMY;

	/* NAPI wants this */
	INIT_LIST_HEAD(&dev->napi_list);

	/* a dummy interface is started by default */
	set_bit(__LINK_STATE_PRESENT, &dev->state);
	set_bit(__LINK_STATE_START, &dev->state);

	/* Note : We dont allocate pcpu_refcnt for dummy devices,
	 * because users of this 'device' dont need to change
	 * its refcount.
	 */

	return 0;
}
EXPORT_SYMBOL_GPL(init_dummy_netdev);


/**
 *	register_netdev	- register a network device
 *	@dev: device to register
 *
 *	Take a completed network device structure and add it to the kernel
 *	interfaces. A %NETDEV_REGISTER message is sent to the netdev notifier
 *	chain. 0 is returned on success. A negative errno code is returned
 *	on a failure to set up the device, or if the name is a duplicate.
 *
 *	This is a wrapper around register_netdevice that takes the rtnl semaphore
 *	and expands the device name if you passed a format string to
 *	alloc_netdev.
 */
int register_netdev(struct net_device *dev)
{
	int err;

	rtnl_lock();
	err = register_netdevice(dev);
	rtnl_unlock();
	return err;
}
EXPORT_SYMBOL(register_netdev);

int netdev_refcnt_read(const struct net_device *dev)
{
	int i, refcnt = 0;

	for_each_possible_cpu(i)
		refcnt += *per_cpu_ptr(dev->pcpu_refcnt, i);
	return refcnt;
}
EXPORT_SYMBOL(netdev_refcnt_read);

/**
 * netdev_wait_allrefs - wait until all references are gone.
 * @dev: target net_device
 *
 * This is called when unregistering network devices.
 *
 * Any protocol or device that holds a reference should register
 * for netdevice notification, and cleanup and put back the
 * reference if they receive an UNREGISTER event.
 * We can get stuck here if buggy protocols don't correctly
 * call dev_put.
 */
static void netdev_wait_allrefs(struct net_device *dev)
{
	unsigned long rebroadcast_time, warning_time;
	int refcnt;

	linkwatch_forget_dev(dev);

	rebroadcast_time = warning_time = jiffies;
	refcnt = netdev_refcnt_read(dev);

	while (refcnt != 0) {
		if (time_after(jiffies, rebroadcast_time + 1 * HZ)) {
			rtnl_lock();

			/* Rebroadcast unregister notification */
			call_netdevice_notifiers(NETDEV_UNREGISTER, dev);

			__rtnl_unlock();
			rcu_barrier();
			rtnl_lock();

			call_netdevice_notifiers(NETDEV_UNREGISTER_FINAL, dev);
			if (test_bit(__LINK_STATE_LINKWATCH_PENDING,
				     &dev->state)) {
				/* We must not have linkwatch events
				 * pending on unregister. If this
				 * happens, we simply run the queue
				 * unscheduled, resulting in a noop
				 * for this device.
				 */
				linkwatch_run_queue();
			}

			__rtnl_unlock();

			rebroadcast_time = jiffies;
		}

		msleep(250);

		refcnt = netdev_refcnt_read(dev);

		if (time_after(jiffies, warning_time + 10 * HZ)) {
			pr_emerg("unregister_netdevice: waiting for %s to become free. Usage count = %d\n",
				 dev->name, refcnt);
			warning_time = jiffies;
		}
	}
}

/* The sequence is:
 *
 *	rtnl_lock();
 *	...
 *	register_netdevice(x1);
 *	register_netdevice(x2);
 *	...
 *	unregister_netdevice(y1);
 *	unregister_netdevice(y2);
 *      ...
 *	rtnl_unlock();
 *	free_netdev(y1);
 *	free_netdev(y2);
 *
 * We are invoked by rtnl_unlock().
 * This allows us to deal with problems:
 * 1) We can delete sysfs objects which invoke hotplug
 *    without deadlocking with linkwatch via keventd.
 * 2) Since we run with the RTNL semaphore not held, we can sleep
 *    safely in order to wait for the netdev refcnt to drop to zero.
 *
 * We must not return until all unregister events added during
 * the interval the lock was held have been completed.
 */
void netdev_run_todo(void)
{
	struct list_head list;

	/* Snapshot list, allow later requests */
	list_replace_init(&net_todo_list, &list);

	__rtnl_unlock();


	/* Wait for rcu callbacks to finish before next phase */
	if (!list_empty(&list))
		rcu_barrier();

	while (!list_empty(&list)) {
		struct net_device *dev
			= list_first_entry(&list, struct net_device, todo_list);
		list_del(&dev->todo_list);

		rtnl_lock();
		call_netdevice_notifiers(NETDEV_UNREGISTER_FINAL, dev);
		__rtnl_unlock();

		if (unlikely(dev->reg_state != NETREG_UNREGISTERING)) {
			pr_err("network todo '%s' but state %d\n",
			       dev->name, dev->reg_state);
			dump_stack();
			continue;
		}

		dev->reg_state = NETREG_UNREGISTERED;

		netdev_wait_allrefs(dev);

		/* paranoia */
		BUG_ON(netdev_refcnt_read(dev));
		BUG_ON(!list_empty(&dev->ptype_all));
		BUG_ON(!list_empty(&dev->ptype_specific));
		WARN_ON(rcu_access_pointer(dev->ip_ptr));
		WARN_ON(rcu_access_pointer(dev->ip6_ptr));
		WARN_ON(dev->dn_ptr);

		if (dev->destructor)
			dev->destructor(dev);

		/* Report a network device has been unregistered */
		rtnl_lock();
		dev_net(dev)->dev_unreg_count--;
		__rtnl_unlock();
		wake_up(&netdev_unregistering_wq);

		/* Free network device */
		kobject_put(&dev->dev.kobj);
	}
}

/* Convert net_device_stats to rtnl_link_stats64.  They have the same
 * fields in the same order, with only the type differing.
 */
void netdev_stats_to_stats64(struct rtnl_link_stats64 *stats64,
			     const struct net_device_stats *netdev_stats)
{
#if BITS_PER_LONG == 64
	BUILD_BUG_ON(sizeof(*stats64) != sizeof(*netdev_stats));
	memcpy(stats64, netdev_stats, sizeof(*stats64));
#else
	size_t i, n = sizeof(*stats64) / sizeof(u64);
	const unsigned long *src = (const unsigned long *)netdev_stats;
	u64 *dst = (u64 *)stats64;

	BUILD_BUG_ON(sizeof(*netdev_stats) / sizeof(unsigned long) !=
		     sizeof(*stats64) / sizeof(u64));
	for (i = 0; i < n; i++)
		dst[i] = src[i];
#endif
}
EXPORT_SYMBOL(netdev_stats_to_stats64);

/**
 *	dev_get_stats	- get network device statistics
 *	@dev: device to get statistics from
 *	@storage: place to store stats
 *
 *	Get network statistics from device. Return @storage.
 *	The device driver may provide its own method by setting
 *	dev->netdev_ops->get_stats64 or dev->netdev_ops->get_stats;
 *	otherwise the internal statistics structure is used.
 */
struct rtnl_link_stats64 *dev_get_stats(struct net_device *dev,
					struct rtnl_link_stats64 *storage)
{
	const struct net_device_ops *ops = dev->netdev_ops;

	if (ops->ndo_get_stats64) {
		memset(storage, 0, sizeof(*storage));
		ops->ndo_get_stats64(dev, storage);
	} else if (ops->ndo_get_stats) {
		netdev_stats_to_stats64(storage, ops->ndo_get_stats(dev));
	} else {
		netdev_stats_to_stats64(storage, &dev->stats);
	}
	storage->rx_dropped += (unsigned long)atomic_long_read(&dev->rx_dropped);
	storage->tx_dropped += (unsigned long)atomic_long_read(&dev->tx_dropped);
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_add_dev_accelerated_stats(dev, storage);
#endif
	return storage;
}
EXPORT_SYMBOL(dev_get_stats);

struct netdev_queue *dev_ingress_queue_create(struct net_device *dev)
{
	struct netdev_queue *queue = dev_ingress_queue(dev);

#ifdef CONFIG_NET_CLS_ACT
	if (queue)
		return queue;
	queue = kzalloc(sizeof(*queue), GFP_KERNEL);
	if (!queue)
		return NULL;
	netdev_init_one_queue(dev, queue, NULL);
	RCU_INIT_POINTER(queue->qdisc, &noop_qdisc);
	queue->qdisc_sleeping = &noop_qdisc;
	rcu_assign_pointer(dev->ingress_queue, queue);
#endif
	return queue;
}

static const struct ethtool_ops default_ethtool_ops;

void netdev_set_default_ethtool_ops(struct net_device *dev,
				    const struct ethtool_ops *ops)
{
	if (dev->ethtool_ops == &default_ethtool_ops)
		dev->ethtool_ops = ops;
}
EXPORT_SYMBOL_GPL(netdev_set_default_ethtool_ops);

void netdev_freemem(struct net_device *dev)
{
	char *addr = (char *)dev - dev->padded;

	kvfree(addr);
}

/**
 *	alloc_netdev_mqs - allocate network device
 *	@sizeof_priv:		size of private data to allocate space for
 *	@name:			device name format string
 *	@name_assign_type: 	origin of device name
 *	@setup:			callback to initialize device
 *	@txqs:			the number of TX subqueues to allocate
 *	@rxqs:			the number of RX subqueues to allocate
 *
 *	Allocates a struct net_device with private data area for driver use
 *	and performs basic initialization.  Also allocates subqueue structs
 *	for each queue on the device.
 */
struct net_device *alloc_netdev_mqs(int sizeof_priv, const char *name,
		unsigned char name_assign_type,
		void (*setup)(struct net_device *),
		unsigned int txqs, unsigned int rxqs)
{
	struct net_device *dev;
	size_t alloc_size;
	struct net_device *p;
#if defined(CONFIG_BCM_KF_MISALIGN_MQS)
	static int offset = 0;
#endif

	BUG_ON(strlen(name) >= sizeof(dev->name));

	if (txqs < 1) {
		pr_err("alloc_netdev: Unable to allocate device with zero queues\n");
		return NULL;
	}

#ifdef CONFIG_SYSFS
	if (rxqs < 1) {
		pr_err("alloc_netdev: Unable to allocate device with zero RX queues\n");
		return NULL;
	}
#endif

	alloc_size = sizeof(struct net_device);
	if (sizeof_priv) {
		/* ensure 32-byte alignment of private area */
		alloc_size = ALIGN(alloc_size, NETDEV_ALIGN);
		alloc_size += sizeof_priv;
	}
	/* ensure 32-byte alignment of whole construct */
	alloc_size += NETDEV_ALIGN - 1;

#if defined(CONFIG_BCM_KF_MISALIGN_MQS)
	/* KU_TBD - This causes the ethernet driver to panic on boot in register_netdev
	 * BUG_ON(dev->reg_state != NETREG_UNINITIALIZED); */

	/* Add an offset to break possible alignment of dev structs in cache */
	/* Note that "offset" is a static variable so it will retain its value */
	/* on each call of this function */
	alloc_size += offset;
#endif

	p = kzalloc(alloc_size, GFP_KERNEL | __GFP_NOWARN | __GFP_REPEAT);
	if (!p)
		p = vzalloc(alloc_size);
	if (!p)
		return NULL;

#if defined(CONFIG_BCM_KF_MISALIGN_MQS)
	dev = PTR_ALIGN(p, NETDEV_ALIGN) + offset;
	/* Increment offset in preparation for the next call to this function */
	/* but don't allow it to increment excessively to avoid wasting memory */
	offset += NETDEV_ALIGN;
	if (offset >= 512)
		offset -= 512;
#else
	dev = PTR_ALIGN(p, NETDEV_ALIGN);
#endif
	dev->padded = (char *)dev - (char *)p;

	dev->pcpu_refcnt = alloc_percpu(int);
	if (!dev->pcpu_refcnt)
		goto free_dev;

	if (dev_addr_init(dev))
		goto free_pcpu;

	dev_mc_init(dev);
	dev_uc_init(dev);

	dev_net_set(dev, &init_net);

	dev->gso_max_size = GSO_MAX_SIZE;
	dev->gso_max_segs = GSO_MAX_SEGS;
	dev->gso_min_segs = 0;

	INIT_LIST_HEAD(&dev->napi_list);
	INIT_LIST_HEAD(&dev->unreg_list);
	INIT_LIST_HEAD(&dev->close_list);
	INIT_LIST_HEAD(&dev->link_watch_list);
	INIT_LIST_HEAD(&dev->adj_list.upper);
	INIT_LIST_HEAD(&dev->adj_list.lower);
	INIT_LIST_HEAD(&dev->all_adj_list.upper);
	INIT_LIST_HEAD(&dev->all_adj_list.lower);
	INIT_LIST_HEAD(&dev->ptype_all);
	INIT_LIST_HEAD(&dev->ptype_specific);
	dev->priv_flags = IFF_XMIT_DST_RELEASE | IFF_XMIT_DST_RELEASE_PERM;
	setup(dev);

	dev->num_tx_queues = txqs;
	dev->real_num_tx_queues = txqs;
	if (netif_alloc_netdev_queues(dev))
		goto free_all;

#ifdef CONFIG_SYSFS
	dev->num_rx_queues = rxqs;
	dev->real_num_rx_queues = rxqs;
	if (netif_alloc_rx_queues(dev))
		goto free_all;
#endif

	strcpy(dev->name, name);
	dev->name_assign_type = name_assign_type;
	dev->group = INIT_NETDEV_GROUP;
	if (!dev->ethtool_ops)
		dev->ethtool_ops = &default_ethtool_ops;
	return dev;

free_all:
	free_netdev(dev);
	return NULL;

free_pcpu:
	free_percpu(dev->pcpu_refcnt);
free_dev:
	netdev_freemem(dev);
	return NULL;
}
EXPORT_SYMBOL(alloc_netdev_mqs);

/**
 *	free_netdev - free network device
 *	@dev: device
 *
 *	This function does the last stage of destroying an allocated device
 * 	interface. The reference to the device object is released.
 *	If this is the last reference then it will be freed.
 */
void free_netdev(struct net_device *dev)
{
	struct napi_struct *p, *n;

	netif_free_tx_queues(dev);
#ifdef CONFIG_SYSFS
	kvfree(dev->_rx);
#endif

	kfree(rcu_dereference_protected(dev->ingress_queue, 1));

	/* Flush device addresses */
	dev_addr_flush(dev);

	list_for_each_entry_safe(p, n, &dev->napi_list, dev_list)
		netif_napi_del(p);

	free_percpu(dev->pcpu_refcnt);
	dev->pcpu_refcnt = NULL;

	/*  Compatibility with error handling in drivers */
	if (dev->reg_state == NETREG_UNINITIALIZED) {
		netdev_freemem(dev);
		return;
	}

	BUG_ON(dev->reg_state != NETREG_UNREGISTERED);
	dev->reg_state = NETREG_RELEASED;

	/* will free via device release */
	put_device(&dev->dev);
}
EXPORT_SYMBOL(free_netdev);

/**
 *	synchronize_net -  Synchronize with packet receive processing
 *
 *	Wait for packets currently being received to be done.
 *	Does not block later packets from starting.
 */
void synchronize_net(void)
{
	might_sleep();
	if (rtnl_is_locked())
		synchronize_rcu_expedited();
	else
		synchronize_rcu();
}
EXPORT_SYMBOL(synchronize_net);

/**
 *	unregister_netdevice_queue - remove device from the kernel
 *	@dev: device
 *	@head: list
 *
 *	This function shuts down a device interface and removes it
 *	from the kernel tables.
 *	If head not NULL, device is queued to be unregistered later.
 *
 *	Callers must hold the rtnl semaphore.  You may want
 *	unregister_netdev() instead of this.
 */

void unregister_netdevice_queue(struct net_device *dev, struct list_head *head)
{
	ASSERT_RTNL();

	if (head) {
		list_move_tail(&dev->unreg_list, head);
	} else {
		rollback_registered(dev);
		/* Finish processing unregister after unlock */
		net_set_todo(dev);
	}
}
EXPORT_SYMBOL(unregister_netdevice_queue);

/**
 *	unregister_netdevice_many - unregister many devices
 *	@head: list of devices
 *
 *  Note: As most callers use a stack allocated list_head,
 *  we force a list_del() to make sure stack wont be corrupted later.
 */
void unregister_netdevice_many(struct list_head *head)
{
	struct net_device *dev;

	if (!list_empty(head)) {
		rollback_registered_many(head);
		list_for_each_entry(dev, head, unreg_list)
			net_set_todo(dev);
		list_del(head);
	}
}
EXPORT_SYMBOL(unregister_netdevice_many);

/**
 *	unregister_netdev - remove device from the kernel
 *	@dev: device
 *
 *	This function shuts down a device interface and removes it
 *	from the kernel tables.
 *
 *	This is just a wrapper for unregister_netdevice that takes
 *	the rtnl semaphore.  In general you want to use this and not
 *	unregister_netdevice.
 */
void unregister_netdev(struct net_device *dev)
{
	rtnl_lock();
#if (defined(CONFIG_BCM_KF_FAP_GSO_LOOPBACK) && defined(CONFIG_BCM_FAP_GSO_LOOPBACK))
	bcm_gso_loopback_del_devptr(dev);
#endif
	unregister_netdevice(dev);
	rtnl_unlock();
}
EXPORT_SYMBOL(unregister_netdev);

/**
 *	dev_change_net_namespace - move device to different nethost namespace
 *	@dev: device
 *	@net: network namespace
 *	@pat: If not NULL name pattern to try if the current device name
 *	      is already taken in the destination network namespace.
 *
 *	This function shuts down a device interface and moves it
 *	to a new network namespace. On success 0 is returned, on
 *	a failure a netagive errno code is returned.
 *
 *	Callers must hold the rtnl semaphore.
 */

int dev_change_net_namespace(struct net_device *dev, struct net *net, const char *pat)
{
	int err;

	ASSERT_RTNL();

	/* Don't allow namespace local devices to be moved. */
	err = -EINVAL;
	if (dev->features & NETIF_F_NETNS_LOCAL)
		goto out;

	/* Ensure the device has been registrered */
	if (dev->reg_state != NETREG_REGISTERED)
		goto out;

	/* Get out if there is nothing todo */
	err = 0;
	if (net_eq(dev_net(dev), net))
		goto out;

	/* Pick the destination device name, and ensure
	 * we can use it in the destination network namespace.
	 */
	err = -EEXIST;
	if (__dev_get_by_name(net, dev->name)) {
		/* We get here if we can't use the current device name */
		if (!pat)
			goto out;
		if (dev_get_valid_name(net, dev, pat) < 0)
			goto out;
	}

	/*
	 * And now a mini version of register_netdevice unregister_netdevice.
	 */

	/* If device is running close it first. */
	dev_close(dev);

	/* And unlink it from device chain */
	err = -ENODEV;
	unlist_netdevice(dev);

	synchronize_net();

	/* Shutdown queueing discipline. */
	dev_shutdown(dev);

	/* Notify protocols, that we are about to destroy
	   this device. They should clean all the things.

	   Note that dev->reg_state stays at NETREG_REGISTERED.
	   This is wanted because this way 8021q and macvlan know
	   the device is just moving and can keep their slaves up.
	*/
	call_netdevice_notifiers(NETDEV_UNREGISTER, dev);
	rcu_barrier();
	call_netdevice_notifiers(NETDEV_UNREGISTER_FINAL, dev);
	rtmsg_ifinfo(RTM_DELLINK, dev, ~0U, GFP_KERNEL);

	/*
	 *	Flush the unicast and multicast chains
	 */
	dev_uc_flush(dev);
	dev_mc_flush(dev);

	/* Send a netdev-removed uevent to the old namespace */
	kobject_uevent(&dev->dev.kobj, KOBJ_REMOVE);
	netdev_adjacent_del_links(dev);

	/* Actually switch the network namespace */
	dev_net_set(dev, net);

	/* If there is an ifindex conflict assign a new one */
	if (__dev_get_by_index(net, dev->ifindex))
		dev->ifindex = dev_new_index(net);

	/* Send a netdev-add uevent to the new namespace */
	kobject_uevent(&dev->dev.kobj, KOBJ_ADD);
	netdev_adjacent_add_links(dev);

	/* Fixup kobjects */
	err = device_rename(&dev->dev, dev->name);
	WARN_ON(err);

	/* Add the device back in the hashes */
	list_netdevice(dev);

	/* Notify protocols, that a new device appeared. */
	call_netdevice_notifiers(NETDEV_REGISTER, dev);

	/*
	 *	Prevent userspace races by waiting until the network
	 *	device is fully setup before sending notifications.
	 */
	rtmsg_ifinfo(RTM_NEWLINK, dev, ~0U, GFP_KERNEL);

	synchronize_net();
	err = 0;
out:
	return err;
}
EXPORT_SYMBOL_GPL(dev_change_net_namespace);

static int dev_cpu_callback(struct notifier_block *nfb,
			    unsigned long action,
			    void *ocpu)
{
	struct sk_buff **list_skb;
	struct sk_buff *skb;
	unsigned int cpu, oldcpu = (unsigned long)ocpu;
	struct softnet_data *sd, *oldsd;

	if (action != CPU_DEAD && action != CPU_DEAD_FROZEN)
		return NOTIFY_OK;

	local_irq_disable();
	cpu = smp_processor_id();
	sd = &per_cpu(softnet_data, cpu);
	oldsd = &per_cpu(softnet_data, oldcpu);

	/* Find end of our completion_queue. */
	list_skb = &sd->completion_queue;
	while (*list_skb)
		list_skb = &(*list_skb)->next;
	/* Append completion queue from offline CPU. */
	*list_skb = oldsd->completion_queue;
	oldsd->completion_queue = NULL;

	/* Append output queue from offline CPU. */
	if (oldsd->output_queue) {
		*sd->output_queue_tailp = oldsd->output_queue;
		sd->output_queue_tailp = oldsd->output_queue_tailp;
		oldsd->output_queue = NULL;
		oldsd->output_queue_tailp = &oldsd->output_queue;
	}
	/* Append NAPI poll list from offline CPU, with one exception :
	 * process_backlog() must be called by cpu owning percpu backlog.
	 * We properly handle process_queue & input_pkt_queue later.
	 */
	while (!list_empty(&oldsd->poll_list)) {
		struct napi_struct *napi = list_first_entry(&oldsd->poll_list,
							    struct napi_struct,
							    poll_list);

		list_del_init(&napi->poll_list);
		if (napi->poll == process_backlog)
			napi->state = 0;
		else
			____napi_schedule(sd, napi);
	}

	raise_softirq_irqoff(NET_TX_SOFTIRQ);
	local_irq_enable();

	/* Process offline CPU's input_pkt_queue */
	while ((skb = __skb_dequeue(&oldsd->process_queue))) {
		netif_rx_ni(skb);
		input_queue_head_incr(oldsd);
	}
	while ((skb = skb_dequeue(&oldsd->input_pkt_queue))) {
		netif_rx_ni(skb);
		input_queue_head_incr(oldsd);
	}

	return NOTIFY_OK;
}


/**
 *	netdev_increment_features - increment feature set by one
 *	@all: current feature set
 *	@one: new feature set
 *	@mask: mask feature set
 *
 *	Computes a new feature set after adding a device with feature set
 *	@one to the master device with current feature set @all.  Will not
 *	enable anything that is off in @mask. Returns the new feature set.
 */
netdev_features_t netdev_increment_features(netdev_features_t all,
	netdev_features_t one, netdev_features_t mask)
{
	if (mask & NETIF_F_GEN_CSUM)
		mask |= NETIF_F_ALL_CSUM;
	mask |= NETIF_F_VLAN_CHALLENGED;

	all |= one & (NETIF_F_ONE_FOR_ALL|NETIF_F_ALL_CSUM) & mask;
	all &= one | ~NETIF_F_ALL_FOR_ALL;

	/* If one device supports hw checksumming, set for all. */
	if (all & NETIF_F_GEN_CSUM)
		all &= ~(NETIF_F_ALL_CSUM & ~NETIF_F_GEN_CSUM);

	return all;
}
EXPORT_SYMBOL(netdev_increment_features);

static struct hlist_head * __net_init netdev_create_hash(void)
{
	int i;
	struct hlist_head *hash;

	hash = kmalloc(sizeof(*hash) * NETDEV_HASHENTRIES, GFP_KERNEL);
	if (hash != NULL)
		for (i = 0; i < NETDEV_HASHENTRIES; i++)
			INIT_HLIST_HEAD(&hash[i]);

	return hash;
}

/* Initialize per network namespace state */
static int __net_init netdev_init(struct net *net)
{
	if (net != &init_net)
		INIT_LIST_HEAD(&net->dev_base_head);

	net->dev_name_head = netdev_create_hash();
	if (net->dev_name_head == NULL)
		goto err_name;

	net->dev_index_head = netdev_create_hash();
	if (net->dev_index_head == NULL)
		goto err_idx;


	return 0;

err_idx:
	kfree(net->dev_name_head);
err_name:
	return -ENOMEM;
}

/**
 *	netdev_drivername - network driver for the device
 *	@dev: network device
 *
 *	Determine network driver for device.
 */
const char *netdev_drivername(const struct net_device *dev)
{
	const struct device_driver *driver;
	const struct device *parent;
	const char *empty = "";

	parent = dev->dev.parent;
	if (!parent)
		return empty;

	driver = parent->driver;
	if (driver && driver->name)
		return driver->name;
	return empty;
}

static void __netdev_printk(const char *level, const struct net_device *dev,
			    struct va_format *vaf)
{
	if (dev && dev->dev.parent) {
		dev_printk_emit(level[1] - '0',
				dev->dev.parent,
				"%s %s %s%s: %pV",
				dev_driver_string(dev->dev.parent),
				dev_name(dev->dev.parent),
				netdev_name(dev), netdev_reg_state(dev),
				vaf);
	} else if (dev) {
		printk("%s%s%s: %pV",
		       level, netdev_name(dev), netdev_reg_state(dev), vaf);
	} else {
		printk("%s(NULL net_device): %pV", level, vaf);
	}
}

void netdev_printk(const char *level, const struct net_device *dev,
		   const char *format, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, format);

	vaf.fmt = format;
	vaf.va = &args;

	__netdev_printk(level, dev, &vaf);

	va_end(args);
}
EXPORT_SYMBOL(netdev_printk);

#define define_netdev_printk_level(func, level)			\
void func(const struct net_device *dev, const char *fmt, ...)	\
{								\
	struct va_format vaf;					\
	va_list args;						\
								\
	va_start(args, fmt);					\
								\
	vaf.fmt = fmt;						\
	vaf.va = &args;						\
								\
	__netdev_printk(level, dev, &vaf);			\
								\
	va_end(args);						\
}								\
EXPORT_SYMBOL(func);

define_netdev_printk_level(netdev_emerg, KERN_EMERG);
define_netdev_printk_level(netdev_alert, KERN_ALERT);
define_netdev_printk_level(netdev_crit, KERN_CRIT);
define_netdev_printk_level(netdev_err, KERN_ERR);
define_netdev_printk_level(netdev_warn, KERN_WARNING);
define_netdev_printk_level(netdev_notice, KERN_NOTICE);
define_netdev_printk_level(netdev_info, KERN_INFO);

static void __net_exit netdev_exit(struct net *net)
{
	kfree(net->dev_name_head);
	kfree(net->dev_index_head);
}

static struct pernet_operations __net_initdata netdev_net_ops = {
	.init = netdev_init,
	.exit = netdev_exit,
};

static void __net_exit default_device_exit(struct net *net)
{
	struct net_device *dev, *aux;
	/*
	 * Push all migratable network devices back to the
	 * initial network namespace
	 */
	rtnl_lock();
	for_each_netdev_safe(net, dev, aux) {
		int err;
		char fb_name[IFNAMSIZ];

		/* Ignore unmoveable devices (i.e. loopback) */
		if (dev->features & NETIF_F_NETNS_LOCAL)
			continue;

		/* Leave virtual devices for the generic cleanup */
		if (dev->rtnl_link_ops)
			continue;

		/* Push remaining network devices to init_net */
		snprintf(fb_name, IFNAMSIZ, "dev%d", dev->ifindex);
		err = dev_change_net_namespace(dev, &init_net, fb_name);
		if (err) {
			pr_emerg("%s: failed to move %s to init_net: %d\n",
				 __func__, dev->name, err);
			BUG();
		}
	}
	rtnl_unlock();
}

static void __net_exit rtnl_lock_unregistering(struct list_head *net_list)
{
	/* Return with the rtnl_lock held when there are no network
	 * devices unregistering in any network namespace in net_list.
	 */
	struct net *net;
	bool unregistering;
	DEFINE_WAIT_FUNC(wait, woken_wake_function);

	add_wait_queue(&netdev_unregistering_wq, &wait);
	for (;;) {
		unregistering = false;
		rtnl_lock();
		list_for_each_entry(net, net_list, exit_list) {
			if (net->dev_unreg_count > 0) {
				unregistering = true;
				break;
			}
		}
		if (!unregistering)
			break;
		__rtnl_unlock();

		wait_woken(&wait, TASK_UNINTERRUPTIBLE, MAX_SCHEDULE_TIMEOUT);
	}
	remove_wait_queue(&netdev_unregistering_wq, &wait);
}

static void __net_exit default_device_exit_batch(struct list_head *net_list)
{
	/* At exit all network devices most be removed from a network
	 * namespace.  Do this in the reverse order of registration.
	 * Do this across as many network namespaces as possible to
	 * improve batching efficiency.
	 */
	struct net_device *dev;
	struct net *net;
	LIST_HEAD(dev_kill_list);

	/* To prevent network device cleanup code from dereferencing
	 * loopback devices or network devices that have been freed
	 * wait here for all pending unregistrations to complete,
	 * before unregistring the loopback device and allowing the
	 * network namespace be freed.
	 *
	 * The netdev todo list containing all network devices
	 * unregistrations that happen in default_device_exit_batch
	 * will run in the rtnl_unlock() at the end of
	 * default_device_exit_batch.
	 */
	rtnl_lock_unregistering(net_list);
	list_for_each_entry(net, net_list, exit_list) {
		for_each_netdev_reverse(net, dev) {
			if (dev->rtnl_link_ops && dev->rtnl_link_ops->dellink)
				dev->rtnl_link_ops->dellink(dev, &dev_kill_list);
			else
				unregister_netdevice_queue(dev, &dev_kill_list);
		}
	}
	unregister_netdevice_many(&dev_kill_list);
	rtnl_unlock();
}

static struct pernet_operations __net_initdata default_device_ops = {
	.exit = default_device_exit,
	.exit_batch = default_device_exit_batch,
};

/*
 *	Initialize the DEV module. At boot time this walks the device list and
 *	unhooks any devices that fail to initialise (normally hardware not
 *	present) and leaves us with a valid list of present and active devices.
 *
 */

/*
 *       This is called single threaded during boot, so no need
 *       to take the rtnl semaphore.
 */
static int __init net_dev_init(void)
{
	int i, rc = -ENOMEM;

	BUG_ON(!dev_boot_phase);

	if (dev_proc_init())
		goto out;

	if (netdev_kobject_init())
		goto out;

	INIT_LIST_HEAD(&ptype_all);
	for (i = 0; i < PTYPE_HASH_SIZE; i++)
		INIT_LIST_HEAD(&ptype_base[i]);

	INIT_LIST_HEAD(&offload_base);

	if (register_pernet_subsys(&netdev_net_ops))
		goto out;

	/*
	 *	Initialise the packet receive queues.
	 */

	for_each_possible_cpu(i) {
		struct softnet_data *sd = &per_cpu(softnet_data, i);

		skb_queue_head_init(&sd->input_pkt_queue);
		skb_queue_head_init(&sd->process_queue);
		INIT_LIST_HEAD(&sd->poll_list);
		sd->output_queue_tailp = &sd->output_queue;
#ifdef CONFIG_RPS
		sd->csd.func = rps_trigger_softirq;
		sd->csd.info = sd;
		sd->cpu = i;
#endif

		sd->backlog.poll = process_backlog;
		sd->backlog.weight = weight_p;
	}

	dev_boot_phase = 0;

	/* The loopback device is special if any other network devices
	 * is present in a network namespace the loopback device must
	 * be present. Since we now dynamically allocate and free the
	 * loopback device ensure this invariant is maintained by
	 * keeping the loopback device as the first device on the
	 * list of network devices.  Ensuring the loopback devices
	 * is the first device that appears and the last network device
	 * that disappears.
	 */
	if (register_pernet_device(&loopback_net_ops))
		goto out;

	if (register_pernet_device(&default_device_ops))
		goto out;

	open_softirq(NET_TX_SOFTIRQ, net_tx_action);
	open_softirq(NET_RX_SOFTIRQ, net_rx_action);

	hotcpu_notifier(dev_cpu_callback, 0);
	dst_init();

#if defined(CONFIG_BCM_KF_SKB_DEFINES)
	skb_free_task = create_skb_free_task();
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
	create_fapDrvTask();
#endif	
#endif

#if (defined(CONFIG_BCM_KF_FAP_GSO_LOOPBACK) && defined(CONFIG_BCM_FAP_GSO_LOOPBACK))
	bcm_gso_loopback_devs_init();
#endif

#if (defined(CONFIG_BCM_KF_SW_GSO) && defined(CONFIG_BCM_SW_GSO)) && !defined(CONFIG_BCM_BPM) && !defined(CONFIG_BCM_BPM_MODULE)
    /* create a slab cache for GSO buffers */
    pktBufferCache = kmem_cache_create("pktBufferCache",
                                       BCM_PKTBUF_SIZE,
                                       0, /* align */
                                       SLAB_HWCACHE_ALIGN, /* flags */
                                       NULL); /* ctor */
    if(pktBufferCache == NULL)
    {
        printk("%s %s: Failed to create packet buffer cache\n", __FILE__, __FUNCTION__);
        return -ENOMEM;
    }
#endif    
	rc = 0;
out:
	return rc;
}

subsys_initcall(net_dev_init);


#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
EXPORT_SYMBOL(bcm_vlan_handle_frame_hook);
#endif

#if defined(CONFIG_BCM_KF_TMS) && defined(CONFIG_BCM_TMS_MODULE)
EXPORT_SYMBOL(bcm_1ag_handle_frame_check_hook);
EXPORT_SYMBOL(bcm_3ah_handle_frame_check_hook);
#endif
