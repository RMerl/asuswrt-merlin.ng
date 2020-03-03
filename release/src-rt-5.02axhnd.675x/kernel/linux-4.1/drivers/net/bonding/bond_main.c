/*
 * originally based on the dummy device.
 *
 * Copyright 1999, Thomas Davis, tadavis@lbl.gov.
 * Licensed under the GPL. Based on dummy.c, and eql.c devices.
 *
 * bonding.c: an Ethernet Bonding driver
 *
 * This is useful to talk to a Cisco EtherChannel compatible equipment:
 *	Cisco 5500
 *	Sun Trunking (Solaris)
 *	Alteon AceDirector Trunks
 *	Linux Bonding
 *	and probably many L2 switches ...
 *
 * How it works:
 *    ifconfig bond0 ipaddress netmask up
 *      will setup a network device, with an ip address.  No mac address
 *	will be assigned at this time.  The hw mac address will come from
 *	the first slave bonded to the channel.  All slaves will then use
 *	this hw mac address.
 *
 *    ifconfig bond0 down
 *         will release all slaves, marking them as down.
 *
 *    ifenslave bond0 eth0
 *	will attach eth0 to bond0 as a slave.  eth0 hw mac address will either
 *	a: be used as initial mac address
 *	b: if a hw mac address already is there, eth0's hw mac address
 *	   will then be set from bond0.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <net/ip.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/socket.h>
#include <linux/ctype.h>
#include <linux/inet.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <asm/dma.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/igmp.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/rtnetlink.h>
#include <linux/smp.h>
#include <linux/if_ether.h>
#include <net/arp.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/if_bonding.h>
#include <linux/jiffies.h>
#include <linux/preempt.h>
#include <net/route.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/pkt_sched.h>
#include <linux/rculist.h>
#include <net/flow_keys.h>
#include <net/switchdev.h>
#include <net/bonding.h>
#include <net/bond_3ad.h>
#include <net/bond_alb.h>

#include "bonding_priv.h"

#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING)
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include <linux/bcm_log.h>
#include <linux/bcm_log_mod.h>
#endif

/*---------------------------- Module parameters ----------------------------*/

/* monitor all links that often (in milliseconds). <=0 disables monitoring */

static int max_bonds	= BOND_DEFAULT_MAX_BONDS;
static int tx_queues	= BOND_DEFAULT_TX_QUEUES;
static int num_peer_notif = 1;
static int miimon;
static int updelay;
static int downdelay;
static int use_carrier	= 1;
static char *mode;
static char *primary;
static char *primary_reselect;
static char *lacp_rate;
static int min_links;
static char *ad_select;
static char *xmit_hash_policy;
static int arp_interval;
static char *arp_ip_target[BOND_MAX_ARP_TARGETS];
static char *arp_validate;
static char *arp_all_targets;
static char *fail_over_mac;
static int all_slaves_active;
static struct bond_params bonding_defaults;
static int resend_igmp = BOND_DEFAULT_RESEND_IGMP;
static int packets_per_slave = 1;
static int lp_interval = BOND_ALB_DEFAULT_LP_INTERVAL;

module_param(max_bonds, int, 0);
MODULE_PARM_DESC(max_bonds, "Max number of bonded devices");
module_param(tx_queues, int, 0);
MODULE_PARM_DESC(tx_queues, "Max number of transmit queues (default = 16)");
module_param_named(num_grat_arp, num_peer_notif, int, 0644);
MODULE_PARM_DESC(num_grat_arp, "Number of peer notifications to send on "
			       "failover event (alias of num_unsol_na)");
module_param_named(num_unsol_na, num_peer_notif, int, 0644);
MODULE_PARM_DESC(num_unsol_na, "Number of peer notifications to send on "
			       "failover event (alias of num_grat_arp)");
module_param(miimon, int, 0);
MODULE_PARM_DESC(miimon, "Link check interval in milliseconds");
module_param(updelay, int, 0);
MODULE_PARM_DESC(updelay, "Delay before considering link up, in milliseconds");
module_param(downdelay, int, 0);
MODULE_PARM_DESC(downdelay, "Delay before considering link down, "
			    "in milliseconds");
module_param(use_carrier, int, 0);
MODULE_PARM_DESC(use_carrier, "Use netif_carrier_ok (vs MII ioctls) in miimon; "
			      "0 for off, 1 for on (default)");
module_param(mode, charp, 0);
MODULE_PARM_DESC(mode, "Mode of operation; 0 for balance-rr, "
		       "1 for active-backup, 2 for balance-xor, "
		       "3 for broadcast, 4 for 802.3ad, 5 for balance-tlb, "
		       "6 for balance-alb");
module_param(primary, charp, 0);
MODULE_PARM_DESC(primary, "Primary network device to use");
module_param(primary_reselect, charp, 0);
MODULE_PARM_DESC(primary_reselect, "Reselect primary slave "
				   "once it comes up; "
				   "0 for always (default), "
				   "1 for only if speed of primary is "
				   "better, "
				   "2 for only on active slave "
				   "failure");
module_param(lacp_rate, charp, 0);
MODULE_PARM_DESC(lacp_rate, "LACPDU tx rate to request from 802.3ad partner; "
			    "0 for slow, 1 for fast");
module_param(ad_select, charp, 0);
MODULE_PARM_DESC(ad_select, "803.ad aggregation selection logic; "
			    "0 for stable (default), 1 for bandwidth, "
			    "2 for count");
module_param(min_links, int, 0);
MODULE_PARM_DESC(min_links, "Minimum number of available links before turning on carrier");

module_param(xmit_hash_policy, charp, 0);
MODULE_PARM_DESC(xmit_hash_policy, "balance-xor and 802.3ad hashing method; "
				   "0 for layer 2 (default), 1 for layer 3+4, "
				   "2 for layer 2+3, 3 for encap layer 2+3, "
				   "4 for encap layer 3+4");
module_param(arp_interval, int, 0);
MODULE_PARM_DESC(arp_interval, "arp interval in milliseconds");
module_param_array(arp_ip_target, charp, NULL, 0);
MODULE_PARM_DESC(arp_ip_target, "arp targets in n.n.n.n form");
module_param(arp_validate, charp, 0);
MODULE_PARM_DESC(arp_validate, "validate src/dst of ARP probes; "
			       "0 for none (default), 1 for active, "
			       "2 for backup, 3 for all");
module_param(arp_all_targets, charp, 0);
MODULE_PARM_DESC(arp_all_targets, "fail on any/all arp targets timeout; 0 for any (default), 1 for all");
module_param(fail_over_mac, charp, 0);
MODULE_PARM_DESC(fail_over_mac, "For active-backup, do not set all slaves to "
				"the same MAC; 0 for none (default), "
				"1 for active, 2 for follow");
module_param(all_slaves_active, int, 0);
MODULE_PARM_DESC(all_slaves_active, "Keep all frames received on an interface "
				     "by setting active flag for all slaves; "
				     "0 for never (default), 1 for always.");
module_param(resend_igmp, int, 0);
MODULE_PARM_DESC(resend_igmp, "Number of IGMP membership reports to send on "
			      "link failure");
module_param(packets_per_slave, int, 0);
MODULE_PARM_DESC(packets_per_slave, "Packets to send per slave in balance-rr "
				    "mode; 0 for a random slave, 1 packet per "
				    "slave (default), >1 packets per slave.");
module_param(lp_interval, uint, 0);
MODULE_PARM_DESC(lp_interval, "The number of seconds between instances where "
			      "the bonding driver sends learning packets to "
			      "each slaves peer switch. The default is 1.");

/*----------------------------- Global variables ----------------------------*/

#ifdef CONFIG_NET_POLL_CONTROLLER
atomic_t netpoll_block_tx = ATOMIC_INIT(0);
#endif

int bond_net_id __read_mostly;

static __be32 arp_target[BOND_MAX_ARP_TARGETS];
static int arp_ip_count;
static int bond_mode	= BOND_MODE_ROUNDROBIN;
static int xmit_hashtype = BOND_XMIT_POLICY_LAYER2;
static int lacp_fast;

/*-------------------------- Forward declarations ---------------------------*/

static int bond_init(struct net_device *bond_dev);
static void bond_uninit(struct net_device *bond_dev);
static struct rtnl_link_stats64 *bond_get_stats(struct net_device *bond_dev,
						struct rtnl_link_stats64 *stats);
static void bond_slave_arr_handler(struct work_struct *work);
static bool bond_time_in_interval(struct bonding *bond, unsigned long last_act,
				  int mod);

/*---------------------------- General routines -----------------------------*/

const char *bond_mode_name(int mode)
{
	static const char *names[] = {
		[BOND_MODE_ROUNDROBIN] = "load balancing (round-robin)",
		[BOND_MODE_ACTIVEBACKUP] = "fault-tolerance (active-backup)",
		[BOND_MODE_XOR] = "load balancing (xor)",
		[BOND_MODE_BROADCAST] = "fault-tolerance (broadcast)",
		[BOND_MODE_8023AD] = "IEEE 802.3ad Dynamic link aggregation",
		[BOND_MODE_TLB] = "transmit load balancing",
		[BOND_MODE_ALB] = "adaptive load balancing",
	};

	if (mode < BOND_MODE_ROUNDROBIN || mode > BOND_MODE_ALB)
		return "unknown";

	return names[mode];
}

/*---------------------------------- VLAN -----------------------------------*/

/**
 * bond_dev_queue_xmit - Prepare skb for xmit.
 *
 * @bond: bond device that got this skb for tx.
 * @skb: hw accel VLAN tagged skb to transmit
 * @slave_dev: slave that is supposed to xmit this skbuff
 */
void bond_dev_queue_xmit(struct bonding *bond, struct sk_buff *skb,
			struct net_device *slave_dev)
{
	skb->dev = slave_dev;

	BUILD_BUG_ON(sizeof(skb->queue_mapping) !=
		     sizeof(qdisc_skb_cb(skb)->slave_dev_queue_mapping));
	skb->queue_mapping = qdisc_skb_cb(skb)->slave_dev_queue_mapping;

	if (unlikely(netpoll_tx_running(bond->dev)))
		bond_netpoll_send_skb(bond_get_slave_by_dev(bond, slave_dev), skb);
	else
		dev_queue_xmit(skb);
}

/* In the following 2 functions, bond_vlan_rx_add_vid and bond_vlan_rx_kill_vid,
 * We don't protect the slave list iteration with a lock because:
 * a. This operation is performed in IOCTL context,
 * b. The operation is protected by the RTNL semaphore in the 8021q code,
 * c. Holding a lock with BH disabled while directly calling a base driver
 *    entry point is generally a BAD idea.
 *
 * The design of synchronization/protection for this operation in the 8021q
 * module is good for one or more VLAN devices over a single physical device
 * and cannot be extended for a teaming solution like bonding, so there is a
 * potential race condition here where a net device from the vlan group might
 * be referenced (either by a base driver or the 8021q code) while it is being
 * removed from the system. However, it turns out we're not making matters
 * worse, and if it works for regular VLAN usage it will work here too.
*/

/**
 * bond_vlan_rx_add_vid - Propagates adding an id to slaves
 * @bond_dev: bonding net device that got called
 * @vid: vlan id being added
 */
static int bond_vlan_rx_add_vid(struct net_device *bond_dev,
				__be16 proto, u16 vid)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave, *rollback_slave;
	struct list_head *iter;
	int res;

	bond_for_each_slave(bond, slave, iter) {
		res = vlan_vid_add(slave->dev, proto, vid);
		if (res)
			goto unwind;
	}

	return 0;

unwind:
	/* unwind to the slave that failed */
	bond_for_each_slave(bond, rollback_slave, iter) {
		if (rollback_slave == slave)
			break;

		vlan_vid_del(rollback_slave->dev, proto, vid);
	}

	return res;
}

/**
 * bond_vlan_rx_kill_vid - Propagates deleting an id to slaves
 * @bond_dev: bonding net device that got called
 * @vid: vlan id being removed
 */
static int bond_vlan_rx_kill_vid(struct net_device *bond_dev,
				 __be16 proto, u16 vid)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct list_head *iter;
	struct slave *slave;

	bond_for_each_slave(bond, slave, iter)
		vlan_vid_del(slave->dev, proto, vid);

	if (bond_is_lb(bond))
		bond_alb_clear_vlan(bond, vid);

	return 0;
}

/*------------------------------- Link status -------------------------------*/

/* Set the carrier state for the master according to the state of its
 * slaves.  If any slaves are up, the master is up.  In 802.3ad mode,
 * do special 802.3ad magic.
 *
 * Returns zero if carrier state does not change, nonzero if it does.
 */
int bond_set_carrier(struct bonding *bond)
{
	struct list_head *iter;
	struct slave *slave;

	if (!bond_has_slaves(bond))
		goto down;

	if (BOND_MODE(bond) == BOND_MODE_8023AD)
		return bond_3ad_set_carrier(bond);

	bond_for_each_slave(bond, slave, iter) {
		if (slave->link == BOND_LINK_UP) {
			if (!netif_carrier_ok(bond->dev)) {
				netif_carrier_on(bond->dev);
				return 1;
			}
			return 0;
		}
	}

down:
	if (netif_carrier_ok(bond->dev)) {
		netif_carrier_off(bond->dev);
		return 1;
	}
	return 0;
}

/* Get link speed and duplex from the slave's base driver
 * using ethtool. If for some reason the call fails or the
 * values are invalid, set speed and duplex to -1,
 * and return.
 */
static void bond_update_speed_duplex(struct slave *slave)
{
	struct net_device *slave_dev = slave->dev;
	struct ethtool_cmd ecmd;
	u32 slave_speed;
	int res;

	slave->speed = SPEED_UNKNOWN;
	slave->duplex = DUPLEX_UNKNOWN;

	res = __ethtool_get_settings(slave_dev, &ecmd);
	if (res < 0)
		return;

	slave_speed = ethtool_cmd_speed(&ecmd);
	if (slave_speed == 0 || slave_speed == ((__u32) -1))
		return;

	switch (ecmd.duplex) {
	case DUPLEX_FULL:
	case DUPLEX_HALF:
		break;
	default:
		return;
	}

	slave->speed = slave_speed;
	slave->duplex = ecmd.duplex;

	return;
}

const char *bond_slave_link_status(s8 link)
{
	switch (link) {
	case BOND_LINK_UP:
		return "up";
	case BOND_LINK_FAIL:
		return "going down";
	case BOND_LINK_DOWN:
		return "down";
	case BOND_LINK_BACK:
		return "going back";
	default:
		return "unknown";
	}
}

/* if <dev> supports MII link status reporting, check its link status.
 *
 * We either do MII/ETHTOOL ioctls, or check netif_carrier_ok(),
 * depending upon the setting of the use_carrier parameter.
 *
 * Return either BMSR_LSTATUS, meaning that the link is up (or we
 * can't tell and just pretend it is), or 0, meaning that the link is
 * down.
 *
 * If reporting is non-zero, instead of faking link up, return -1 if
 * both ETHTOOL and MII ioctls fail (meaning the device does not
 * support them).  If use_carrier is set, return whatever it says.
 * It'd be nice if there was a good way to tell if a driver supports
 * netif_carrier, but there really isn't.
 */
static int bond_check_dev_link(struct bonding *bond,
			       struct net_device *slave_dev, int reporting)
{
	const struct net_device_ops *slave_ops = slave_dev->netdev_ops;
	int (*ioctl)(struct net_device *, struct ifreq *, int);
	struct ifreq ifr;
	struct mii_ioctl_data *mii;

	if (!reporting && !netif_running(slave_dev))
		return 0;

	if (bond->params.use_carrier)
		return netif_carrier_ok(slave_dev) ? BMSR_LSTATUS : 0;

	/* Try to get link status using Ethtool first. */
	if (slave_dev->ethtool_ops->get_link)
		return slave_dev->ethtool_ops->get_link(slave_dev) ?
			BMSR_LSTATUS : 0;

	/* Ethtool can't be used, fallback to MII ioctls. */
	ioctl = slave_ops->ndo_do_ioctl;
	if (ioctl) {
		/* TODO: set pointer to correct ioctl on a per team member
		 *       bases to make this more efficient. that is, once
		 *       we determine the correct ioctl, we will always
		 *       call it and not the others for that team
		 *       member.
		 */

		/* We cannot assume that SIOCGMIIPHY will also read a
		 * register; not all network drivers (e.g., e100)
		 * support that.
		 */

		/* Yes, the mii is overlaid on the ifreq.ifr_ifru */
		strncpy(ifr.ifr_name, slave_dev->name, IFNAMSIZ);
		mii = if_mii(&ifr);
		if (IOCTL(slave_dev, &ifr, SIOCGMIIPHY) == 0) {
			mii->reg_num = MII_BMSR;
			if (IOCTL(slave_dev, &ifr, SIOCGMIIREG) == 0)
				return mii->val_out & BMSR_LSTATUS;
		}
	}

	/* If reporting, report that either there's no dev->do_ioctl,
	 * or both SIOCGMIIREG and get_link failed (meaning that we
	 * cannot report link status).  If not reporting, pretend
	 * we're ok.
	 */
	return reporting ? -1 : BMSR_LSTATUS;
}

/*----------------------------- Multicast list ------------------------------*/

/* Push the promiscuity flag down to appropriate slaves */
static int bond_set_promiscuity(struct bonding *bond, int inc)
{
	struct list_head *iter;
	int err = 0;

	if (bond_uses_primary(bond)) {
		struct slave *curr_active = rtnl_dereference(bond->curr_active_slave);

		if (curr_active)
			err = dev_set_promiscuity(curr_active->dev, inc);
	} else {
		struct slave *slave;

		bond_for_each_slave(bond, slave, iter) {
			err = dev_set_promiscuity(slave->dev, inc);
			if (err)
				return err;
		}
	}
	return err;
}

/* Push the allmulti flag down to all slaves */
static int bond_set_allmulti(struct bonding *bond, int inc)
{
	struct list_head *iter;
	int err = 0;

	if (bond_uses_primary(bond)) {
		struct slave *curr_active = rtnl_dereference(bond->curr_active_slave);

		if (curr_active)
			err = dev_set_allmulti(curr_active->dev, inc);
	} else {
		struct slave *slave;

		bond_for_each_slave(bond, slave, iter) {
			err = dev_set_allmulti(slave->dev, inc);
			if (err)
				return err;
		}
	}
	return err;
}

/* Retrieve the list of registered multicast addresses for the bonding
 * device and retransmit an IGMP JOIN request to the current active
 * slave.
 */
static void bond_resend_igmp_join_requests_delayed(struct work_struct *work)
{
	struct bonding *bond = container_of(work, struct bonding,
					    mcast_work.work);

	if (!rtnl_trylock()) {
		queue_delayed_work(bond->wq, &bond->mcast_work, 1);
		return;
	}
	call_netdevice_notifiers(NETDEV_RESEND_IGMP, bond->dev);

	if (bond->igmp_retrans > 1) {
		bond->igmp_retrans--;
		queue_delayed_work(bond->wq, &bond->mcast_work, HZ/5);
	}
	rtnl_unlock();
}

/* Flush bond's hardware addresses from slave */
static void bond_hw_addr_flush(struct net_device *bond_dev,
			       struct net_device *slave_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);

	dev_uc_unsync(slave_dev, bond_dev);
	dev_mc_unsync(slave_dev, bond_dev);

	if (BOND_MODE(bond) == BOND_MODE_8023AD) {
		/* del lacpdu mc addr from mc list */
		u8 lacpdu_multicast[ETH_ALEN] = MULTICAST_LACPDU_ADDR;

		dev_mc_del(slave_dev, lacpdu_multicast);
	}
}

/*--------------------------- Active slave change ---------------------------*/

/* Update the hardware address list and promisc/allmulti for the new and
 * old active slaves (if any).  Modes that are not using primary keep all
 * slaves up date at all times; only the modes that use primary need to call
 * this function to swap these settings during a failover.
 */
static void bond_hw_addr_swap(struct bonding *bond, struct slave *new_active,
			      struct slave *old_active)
{
	if (old_active) {
		if (bond->dev->flags & IFF_PROMISC)
			dev_set_promiscuity(old_active->dev, -1);

		if (bond->dev->flags & IFF_ALLMULTI)
			dev_set_allmulti(old_active->dev, -1);

		bond_hw_addr_flush(bond->dev, old_active->dev);
	}

	if (new_active) {
		/* FIXME: Signal errors upstream. */
		if (bond->dev->flags & IFF_PROMISC)
			dev_set_promiscuity(new_active->dev, 1);

		if (bond->dev->flags & IFF_ALLMULTI)
			dev_set_allmulti(new_active->dev, 1);

		netif_addr_lock_bh(bond->dev);
		dev_uc_sync(new_active->dev, bond->dev);
		dev_mc_sync(new_active->dev, bond->dev);
		netif_addr_unlock_bh(bond->dev);
	}
}

/**
 * bond_set_dev_addr - clone slave's address to bond
 * @bond_dev: bond net device
 * @slave_dev: slave net device
 *
 * Should be called with RTNL held.
 */
static void bond_set_dev_addr(struct net_device *bond_dev,
			      struct net_device *slave_dev)
{
	netdev_dbg(bond_dev, "bond_dev=%p slave_dev=%p slave_dev->addr_len=%d\n",
		   bond_dev, slave_dev, slave_dev->addr_len);
	memcpy(bond_dev->dev_addr, slave_dev->dev_addr, slave_dev->addr_len);
	bond_dev->addr_assign_type = NET_ADDR_STOLEN;
	call_netdevice_notifiers(NETDEV_CHANGEADDR, bond_dev);
}

static struct slave *bond_get_old_active(struct bonding *bond,
					 struct slave *new_active)
{
	struct slave *slave;
	struct list_head *iter;

	bond_for_each_slave(bond, slave, iter) {
		if (slave == new_active)
			continue;

		if (ether_addr_equal(bond->dev->dev_addr, slave->dev->dev_addr))
			return slave;
	}

	return NULL;
}

/* bond_do_fail_over_mac
 *
 * Perform special MAC address swapping for fail_over_mac settings
 *
 * Called with RTNL
 */
static void bond_do_fail_over_mac(struct bonding *bond,
				  struct slave *new_active,
				  struct slave *old_active)
{
	u8 tmp_mac[ETH_ALEN];
	struct sockaddr saddr;
	int rv;

	switch (bond->params.fail_over_mac) {
	case BOND_FOM_ACTIVE:
		if (new_active)
			bond_set_dev_addr(bond->dev, new_active->dev);
		break;
	case BOND_FOM_FOLLOW:
		/* if new_active && old_active, swap them
		 * if just old_active, do nothing (going to no active slave)
		 * if just new_active, set new_active to bond's MAC
		 */
		if (!new_active)
			return;

		if (!old_active)
			old_active = bond_get_old_active(bond, new_active);

		if (old_active) {
			ether_addr_copy(tmp_mac, new_active->dev->dev_addr);
			ether_addr_copy(saddr.sa_data,
					old_active->dev->dev_addr);
			saddr.sa_family = new_active->dev->type;
		} else {
			ether_addr_copy(saddr.sa_data, bond->dev->dev_addr);
			saddr.sa_family = bond->dev->type;
		}

		rv = dev_set_mac_address(new_active->dev, &saddr);
		if (rv) {
			netdev_err(bond->dev, "Error %d setting MAC of slave %s\n",
				   -rv, new_active->dev->name);
			goto out;
		}

		if (!old_active)
			goto out;

		ether_addr_copy(saddr.sa_data, tmp_mac);
		saddr.sa_family = old_active->dev->type;

		rv = dev_set_mac_address(old_active->dev, &saddr);
		if (rv)
			netdev_err(bond->dev, "Error %d setting MAC of slave %s\n",
				   -rv, new_active->dev->name);
out:
		break;
	default:
		netdev_err(bond->dev, "bond_do_fail_over_mac impossible: bad policy %d\n",
			   bond->params.fail_over_mac);
		break;
	}

}

static bool bond_should_change_active(struct bonding *bond)
{
	struct slave *prim = rtnl_dereference(bond->primary_slave);
	struct slave *curr = rtnl_dereference(bond->curr_active_slave);

	if (!prim || !curr || curr->link != BOND_LINK_UP)
		return true;
	if (bond->force_primary) {
		bond->force_primary = false;
		return true;
	}
	if (bond->params.primary_reselect == BOND_PRI_RESELECT_BETTER &&
	    (prim->speed < curr->speed ||
	     (prim->speed == curr->speed && prim->duplex <= curr->duplex)))
		return false;
	if (bond->params.primary_reselect == BOND_PRI_RESELECT_FAILURE)
		return false;
	return true;
}

/**
 * find_best_interface - select the best available slave to be the active one
 * @bond: our bonding struct
 */
static struct slave *bond_find_best_slave(struct bonding *bond)
{
	struct slave *slave, *bestslave = NULL, *primary;
	struct list_head *iter;
	int mintime = bond->params.updelay;

	primary = rtnl_dereference(bond->primary_slave);
	if (primary && primary->link == BOND_LINK_UP &&
	    bond_should_change_active(bond))
		return primary;

	bond_for_each_slave(bond, slave, iter) {
		if (slave->link == BOND_LINK_UP)
			return slave;
		if (slave->link == BOND_LINK_BACK && bond_slave_is_up(slave) &&
		    slave->delay < mintime) {
			mintime = slave->delay;
			bestslave = slave;
		}
	}

	return bestslave;
}

static bool bond_should_notify_peers(struct bonding *bond)
{
	struct slave *slave;

	rcu_read_lock();
	slave = rcu_dereference(bond->curr_active_slave);
	rcu_read_unlock();

	netdev_dbg(bond->dev, "bond_should_notify_peers: slave %s\n",
		   slave ? slave->dev->name : "NULL");

	if (!slave || !bond->send_peer_notif ||
	    test_bit(__LINK_STATE_LINKWATCH_PENDING, &slave->dev->state))
		return false;

	return true;
}

/**
 * change_active_interface - change the active slave into the specified one
 * @bond: our bonding struct
 * @new: the new slave to make the active one
 *
 * Set the new slave to the bond's settings and unset them on the old
 * curr_active_slave.
 * Setting include flags, mc-list, promiscuity, allmulti, etc.
 *
 * If @new's link state is %BOND_LINK_BACK we'll set it to %BOND_LINK_UP,
 * because it is apparently the best available slave we have, even though its
 * updelay hasn't timed out yet.
 *
 * Caller must hold RTNL.
 */
void bond_change_active_slave(struct bonding *bond, struct slave *new_active)
{
	struct slave *old_active;

	ASSERT_RTNL();

	old_active = rtnl_dereference(bond->curr_active_slave);

	if (old_active == new_active)
		return;

	if (new_active) {
		new_active->last_link_up = jiffies;

		if (new_active->link == BOND_LINK_BACK) {
			if (bond_uses_primary(bond)) {
				netdev_info(bond->dev, "making interface %s the new active one %d ms earlier\n",
					    new_active->dev->name,
					    (bond->params.updelay - new_active->delay) * bond->params.miimon);
			}

			new_active->delay = 0;
			bond_set_slave_link_state(new_active, BOND_LINK_UP);

			if (BOND_MODE(bond) == BOND_MODE_8023AD)
				bond_3ad_handle_link_change(new_active, BOND_LINK_UP);

			if (bond_is_lb(bond))
				bond_alb_handle_link_change(bond, new_active, BOND_LINK_UP);
		} else {
			if (bond_uses_primary(bond)) {
				netdev_info(bond->dev, "making interface %s the new active one\n",
					    new_active->dev->name);
			}
		}
	}

	if (bond_uses_primary(bond))
		bond_hw_addr_swap(bond, new_active, old_active);

	if (bond_is_lb(bond)) {
		bond_alb_handle_active_change(bond, new_active);
		if (old_active)
			bond_set_slave_inactive_flags(old_active,
						      BOND_SLAVE_NOTIFY_NOW);
		if (new_active)
			bond_set_slave_active_flags(new_active,
						    BOND_SLAVE_NOTIFY_NOW);
	} else {
		rcu_assign_pointer(bond->curr_active_slave, new_active);
	}

	if (BOND_MODE(bond) == BOND_MODE_ACTIVEBACKUP) {
		if (old_active)
			bond_set_slave_inactive_flags(old_active,
						      BOND_SLAVE_NOTIFY_NOW);

		if (new_active) {
			bool should_notify_peers = false;

			bond_set_slave_active_flags(new_active,
						    BOND_SLAVE_NOTIFY_NOW);

			if (bond->params.fail_over_mac)
				bond_do_fail_over_mac(bond, new_active,
						      old_active);

			if (netif_running(bond->dev)) {
				bond->send_peer_notif =
					bond->params.num_peer_notif;
				should_notify_peers =
					bond_should_notify_peers(bond);
			}

			call_netdevice_notifiers(NETDEV_BONDING_FAILOVER, bond->dev);
			if (should_notify_peers)
				call_netdevice_notifiers(NETDEV_NOTIFY_PEERS,
							 bond->dev);
		}
	}

	/* resend IGMP joins since active slave has changed or
	 * all were sent on curr_active_slave.
	 * resend only if bond is brought up with the affected
	 * bonding modes and the retransmission is enabled
	 */
	if (netif_running(bond->dev) && (bond->params.resend_igmp > 0) &&
	    ((bond_uses_primary(bond) && new_active) ||
	     BOND_MODE(bond) == BOND_MODE_ROUNDROBIN)) {
		bond->igmp_retrans = bond->params.resend_igmp;
		queue_delayed_work(bond->wq, &bond->mcast_work, 1);
	}
}

/**
 * bond_select_active_slave - select a new active slave, if needed
 * @bond: our bonding struct
 *
 * This functions should be called when one of the following occurs:
 * - The old curr_active_slave has been released or lost its link.
 * - The primary_slave has got its link back.
 * - A slave has got its link back and there's no old curr_active_slave.
 *
 * Caller must hold RTNL.
 */
void bond_select_active_slave(struct bonding *bond)
{
	struct slave *best_slave;
	int rv;

	ASSERT_RTNL();

	best_slave = bond_find_best_slave(bond);
	if (best_slave != rtnl_dereference(bond->curr_active_slave)) {
		bond_change_active_slave(bond, best_slave);
		rv = bond_set_carrier(bond);
		if (!rv)
			return;

		if (netif_carrier_ok(bond->dev)) {
			netdev_info(bond->dev, "first active interface up!\n");
		} else {
			netdev_info(bond->dev, "now running without any active interface!\n");
		}
	}
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static inline int slave_enable_netpoll(struct slave *slave)
{
	struct netpoll *np;
	int err = 0;

	np = kzalloc(sizeof(*np), GFP_KERNEL);
	err = -ENOMEM;
	if (!np)
		goto out;

	err = __netpoll_setup(np, slave->dev);
	if (err) {
		kfree(np);
		goto out;
	}
	slave->np = np;
out:
	return err;
}
static inline void slave_disable_netpoll(struct slave *slave)
{
	struct netpoll *np = slave->np;

	if (!np)
		return;

	slave->np = NULL;
	__netpoll_free_async(np);
}

static void bond_poll_controller(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave = NULL;
	struct list_head *iter;
	struct ad_info ad_info;
	struct netpoll_info *ni;
	const struct net_device_ops *ops;

	if (BOND_MODE(bond) == BOND_MODE_8023AD)
		if (bond_3ad_get_active_agg_info(bond, &ad_info))
			return;

	rcu_read_lock_bh();
	bond_for_each_slave_rcu(bond, slave, iter) {
		ops = slave->dev->netdev_ops;
		if (!bond_slave_is_up(slave) || !ops->ndo_poll_controller)
			continue;

		if (BOND_MODE(bond) == BOND_MODE_8023AD) {
			struct aggregator *agg =
			    SLAVE_AD_INFO(slave)->port.aggregator;

			if (agg &&
			    agg->aggregator_identifier != ad_info.aggregator_id)
				continue;
		}

		ni = rcu_dereference_bh(slave->dev->npinfo);
		if (down_trylock(&ni->dev_lock))
			continue;
		ops->ndo_poll_controller(slave->dev);
		up(&ni->dev_lock);
	}
	rcu_read_unlock_bh();
}

static void bond_netpoll_cleanup(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct list_head *iter;
	struct slave *slave;

	bond_for_each_slave(bond, slave, iter)
		if (bond_slave_is_up(slave))
			slave_disable_netpoll(slave);
}

static int bond_netpoll_setup(struct net_device *dev, struct netpoll_info *ni)
{
	struct bonding *bond = netdev_priv(dev);
	struct list_head *iter;
	struct slave *slave;
	int err = 0;

	bond_for_each_slave(bond, slave, iter) {
		err = slave_enable_netpoll(slave);
		if (err) {
			bond_netpoll_cleanup(dev);
			break;
		}
	}
	return err;
}
#else
static inline int slave_enable_netpoll(struct slave *slave)
{
	return 0;
}
static inline void slave_disable_netpoll(struct slave *slave)
{
}
static void bond_netpoll_cleanup(struct net_device *bond_dev)
{
}
#endif

/*---------------------------------- IOCTL ----------------------------------*/

static netdev_features_t bond_fix_features(struct net_device *dev,
					   netdev_features_t features)
{
	struct bonding *bond = netdev_priv(dev);
	struct list_head *iter;
	netdev_features_t mask;
	struct slave *slave;

	/* If any slave has the offload feature flag set,
	 * set the offload flag on the bond.
	 */
	mask = features | NETIF_F_HW_SWITCH_OFFLOAD;

	features &= ~NETIF_F_ONE_FOR_ALL;
	features |= NETIF_F_ALL_FOR_ALL;

	bond_for_each_slave(bond, slave, iter) {
		features = netdev_increment_features(features,
						     slave->dev->features,
						     mask);
	}
	features = netdev_add_tso_features(features, mask);

	return features;
}

#define BOND_VLAN_FEATURES	(NETIF_F_ALL_CSUM | NETIF_F_SG | \
				 NETIF_F_FRAGLIST | NETIF_F_ALL_TSO | \
				 NETIF_F_HIGHDMA | NETIF_F_LRO)

#define BOND_ENC_FEATURES	(NETIF_F_ALL_CSUM | NETIF_F_SG | NETIF_F_RXCSUM |\
				 NETIF_F_TSO)

static void bond_compute_features(struct bonding *bond)
{
	unsigned int dst_release_flag = IFF_XMIT_DST_RELEASE |
					IFF_XMIT_DST_RELEASE_PERM;
	netdev_features_t vlan_features = BOND_VLAN_FEATURES;
	netdev_features_t enc_features  = BOND_ENC_FEATURES;
	struct net_device *bond_dev = bond->dev;
	struct list_head *iter;
	struct slave *slave;
	unsigned short max_hard_header_len = ETH_HLEN;
	unsigned int gso_max_size = GSO_MAX_SIZE;
	u16 gso_max_segs = GSO_MAX_SEGS;

	if (!bond_has_slaves(bond))
		goto done;
	vlan_features &= NETIF_F_ALL_FOR_ALL;

	bond_for_each_slave(bond, slave, iter) {
		vlan_features = netdev_increment_features(vlan_features,
			slave->dev->vlan_features, BOND_VLAN_FEATURES);

		enc_features = netdev_increment_features(enc_features,
							 slave->dev->hw_enc_features,
							 BOND_ENC_FEATURES);
		dst_release_flag &= slave->dev->priv_flags;
		if (slave->dev->hard_header_len > max_hard_header_len)
			max_hard_header_len = slave->dev->hard_header_len;

		gso_max_size = min(gso_max_size, slave->dev->gso_max_size);
		gso_max_segs = min(gso_max_segs, slave->dev->gso_max_segs);
	}

done:
	bond_dev->vlan_features = vlan_features;
	bond_dev->hw_enc_features = enc_features | NETIF_F_GSO_ENCAP_ALL;
	bond_dev->hard_header_len = max_hard_header_len;
	bond_dev->gso_max_segs = gso_max_segs;
	netif_set_gso_max_size(bond_dev, gso_max_size);

	bond_dev->priv_flags &= ~IFF_XMIT_DST_RELEASE;
	if ((bond_dev->priv_flags & IFF_XMIT_DST_RELEASE_PERM) &&
	    dst_release_flag == (IFF_XMIT_DST_RELEASE | IFF_XMIT_DST_RELEASE_PERM))
		bond_dev->priv_flags |= IFF_XMIT_DST_RELEASE;

	netdev_change_features(bond_dev);
}

static void bond_setup_by_slave(struct net_device *bond_dev,
				struct net_device *slave_dev)
{
	bond_dev->header_ops	    = slave_dev->header_ops;

	bond_dev->type		    = slave_dev->type;
	bond_dev->hard_header_len   = slave_dev->hard_header_len;
	bond_dev->addr_len	    = slave_dev->addr_len;

	memcpy(bond_dev->broadcast, slave_dev->broadcast,
		slave_dev->addr_len);
}

/* On bonding slaves other than the currently active slave, suppress
 * duplicates except for alb non-mcast/bcast.
 */
static bool bond_should_deliver_exact_match(struct sk_buff *skb,
					    struct slave *slave,
					    struct bonding *bond)
{
	if (bond_is_slave_inactive(slave)) {
		if (BOND_MODE(bond) == BOND_MODE_ALB &&
		    skb->pkt_type != PACKET_BROADCAST &&
		    skb->pkt_type != PACKET_MULTICAST)
			return false;
		return true;
	}
	return false;
}

static rx_handler_result_t bond_handle_frame(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct slave *slave;
	struct bonding *bond;
	int (*recv_probe)(const struct sk_buff *, struct bonding *,
			  struct slave *);
	int ret = RX_HANDLER_ANOTHER;

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (unlikely(!skb))
		return RX_HANDLER_CONSUMED;

#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING)
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	if (blog_ptr(skb)) 
	{

		/* Clarify : bond device shouldn't modify the packet;
			also it does not hold its own stats rather gets the cummulative stats from its slave devices [ see:bond_get_stats() ];
			blog->put_stats,clr_stats are also not needed for the same reason; 
			Do we need to link bond devices ; One usage could be that when bond device goes down ; FC will be able to remove the flows */

		blog_lock();
		blog_link( IF_DEVICE, blog_ptr(skb), (void*)skb->dev, DIR_RX, skb->len );
		blog_unlock();
	}
#endif /* defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG) */
#endif /* defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) */

	*pskb = skb;

	slave = bond_slave_get_rcu(skb->dev);
	bond = slave->bond;

	recv_probe = ACCESS_ONCE(bond->recv_probe);
	if (recv_probe) {
		ret = recv_probe(skb, bond, slave);
		if (ret == RX_HANDLER_CONSUMED) {
			consume_skb(skb);
			return ret;
		}
	}

	if (bond_should_deliver_exact_match(skb, slave, bond)) {
		return RX_HANDLER_EXACT;
	}

	skb->dev = bond->dev;

	if (BOND_MODE(bond) == BOND_MODE_ALB &&
	    bond->dev->priv_flags & IFF_BRIDGE_PORT &&
	    skb->pkt_type == PACKET_HOST) {

		if (unlikely(skb_cow_head(skb,
					  skb->data - skb_mac_header(skb)))) {
			kfree_skb(skb);
			return RX_HANDLER_CONSUMED;
		}
		ether_addr_copy(eth_hdr(skb)->h_dest, bond->dev->dev_addr);
	}

	return ret;
}

static int bond_master_upper_dev_link(struct net_device *bond_dev,
				      struct net_device *slave_dev,
				      struct slave *slave)
{
	int err;

	err = netdev_master_upper_dev_link_private(slave_dev, bond_dev, slave);
	if (err)
		return err;
	rtmsg_ifinfo(RTM_NEWLINK, slave_dev, IFF_SLAVE, GFP_KERNEL);
	return 0;
}

static void bond_upper_dev_unlink(struct net_device *bond_dev,
				  struct net_device *slave_dev)
{
	netdev_upper_dev_unlink(slave_dev, bond_dev);
	slave_dev->flags &= ~IFF_SLAVE;
	rtmsg_ifinfo(RTM_NEWLINK, slave_dev, IFF_SLAVE, GFP_KERNEL);
}

static struct slave *bond_alloc_slave(struct bonding *bond)
{
	struct slave *slave = NULL;

	slave = kzalloc(sizeof(struct slave), GFP_KERNEL);
	if (!slave)
		return NULL;

	if (BOND_MODE(bond) == BOND_MODE_8023AD) {
		SLAVE_AD_INFO(slave) = kzalloc(sizeof(struct ad_slave_info),
					       GFP_KERNEL);
		if (!SLAVE_AD_INFO(slave)) {
			kfree(slave);
			return NULL;
		}
	}
	return slave;
}

static void bond_free_slave(struct slave *slave)
{
	struct bonding *bond = bond_get_bond_by_slave(slave);

	if (BOND_MODE(bond) == BOND_MODE_8023AD)
		kfree(SLAVE_AD_INFO(slave));

	kfree(slave);
}

static void bond_fill_ifbond(struct bonding *bond, struct ifbond *info)
{
	info->bond_mode = BOND_MODE(bond);
	info->miimon = bond->params.miimon;
	info->num_slaves = bond->slave_cnt;
}

static void bond_fill_ifslave(struct slave *slave, struct ifslave *info)
{
	strcpy(info->slave_name, slave->dev->name);
	info->link = slave->link;
	info->state = bond_slave_state(slave);
	info->link_failure_count = slave->link_failure_count;
}

static void bond_netdev_notify(struct net_device *dev,
			       struct netdev_bonding_info *info)
{
	rtnl_lock();
	netdev_bonding_info_change(dev, info);
	rtnl_unlock();
}

static void bond_netdev_notify_work(struct work_struct *_work)
{
	struct netdev_notify_work *w =
		container_of(_work, struct netdev_notify_work, work.work);

	bond_netdev_notify(w->dev, &w->bonding_info);
	dev_put(w->dev);
	kfree(w);
}

void bond_queue_slave_event(struct slave *slave)
{
	struct bonding *bond = slave->bond;
	struct netdev_notify_work *nnw = kzalloc(sizeof(*nnw), GFP_ATOMIC);

	if (!nnw)
		return;

	dev_hold(slave->dev);
	nnw->dev = slave->dev;
	bond_fill_ifslave(slave, &nnw->bonding_info.slave);
	bond_fill_ifbond(bond, &nnw->bonding_info.master);
	INIT_DELAYED_WORK(&nnw->work, bond_netdev_notify_work);

	queue_delayed_work(slave->bond->wq, &nnw->work, 0);
}

/* enslave device <slave> to bond device <master> */
int bond_enslave(struct net_device *bond_dev, struct net_device *slave_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	const struct net_device_ops *slave_ops = slave_dev->netdev_ops;
	struct slave *new_slave = NULL, *prev_slave;
	struct sockaddr addr;
	int link_reporting;
	int res = 0, i;

	if (!bond->params.use_carrier &&
	    slave_dev->ethtool_ops->get_link == NULL &&
	    slave_ops->ndo_do_ioctl == NULL) {
		netdev_warn(bond_dev, "no link monitoring support for %s\n",
			    slave_dev->name);
	}

	/* already enslaved */
	if (slave_dev->flags & IFF_SLAVE) {
		netdev_dbg(bond_dev, "Error: Device was already enslaved\n");
		return -EBUSY;
	}

	if (bond_dev == slave_dev) {
		netdev_err(bond_dev, "cannot enslave bond to itself.\n");
		return -EPERM;
	}

	/* vlan challenged mutual exclusion */
	/* no need to lock since we're protected by rtnl_lock */
	if (slave_dev->features & NETIF_F_VLAN_CHALLENGED) {
		netdev_dbg(bond_dev, "%s is NETIF_F_VLAN_CHALLENGED\n",
			   slave_dev->name);
		if (vlan_uses_dev(bond_dev)) {
			netdev_err(bond_dev, "Error: cannot enslave VLAN challenged slave %s on VLAN enabled bond %s\n",
				   slave_dev->name, bond_dev->name);
			return -EPERM;
		} else {
			netdev_warn(bond_dev, "enslaved VLAN challenged slave %s. Adding VLANs will be blocked as long as %s is part of bond %s\n",
				    slave_dev->name, slave_dev->name,
				    bond_dev->name);
		}
	} else {
		netdev_dbg(bond_dev, "%s is !NETIF_F_VLAN_CHALLENGED\n",
			   slave_dev->name);
	}

	/* Old ifenslave binaries are no longer supported.  These can
	 * be identified with moderate accuracy by the state of the slave:
	 * the current ifenslave will set the interface down prior to
	 * enslaving it; the old ifenslave will not.
	 */
	if ((slave_dev->flags & IFF_UP)) {
		netdev_err(bond_dev, "%s is up - this may be due to an out of date ifenslave\n",
			   slave_dev->name);
		res = -EPERM;
		goto err_undo_flags;
	}

	/* set bonding device ether type by slave - bonding netdevices are
	 * created with ether_setup, so when the slave type is not ARPHRD_ETHER
	 * there is a need to override some of the type dependent attribs/funcs.
	 *
	 * bond ether type mutual exclusion - don't allow slaves of dissimilar
	 * ether type (eg ARPHRD_ETHER and ARPHRD_INFINIBAND) share the same bond
	 */
	if (!bond_has_slaves(bond)) {
		if (bond_dev->type != slave_dev->type) {
			netdev_dbg(bond_dev, "change device type from %d to %d\n",
				   bond_dev->type, slave_dev->type);

			res = call_netdevice_notifiers(NETDEV_PRE_TYPE_CHANGE,
						       bond_dev);
			res = notifier_to_errno(res);
			if (res) {
				netdev_err(bond_dev, "refused to change device type\n");
				res = -EBUSY;
				goto err_undo_flags;
			}

			/* Flush unicast and multicast addresses */
			dev_uc_flush(bond_dev);
			dev_mc_flush(bond_dev);

			if (slave_dev->type != ARPHRD_ETHER)
				bond_setup_by_slave(bond_dev, slave_dev);
			else {
				ether_setup(bond_dev);
				bond_dev->priv_flags &= ~IFF_TX_SKB_SHARING;
			}

			call_netdevice_notifiers(NETDEV_POST_TYPE_CHANGE,
						 bond_dev);
		}
	} else if (bond_dev->type != slave_dev->type) {
		netdev_err(bond_dev, "%s ether type (%d) is different from other slaves (%d), can not enslave it\n",
			   slave_dev->name, slave_dev->type, bond_dev->type);
		res = -EINVAL;
		goto err_undo_flags;
	}

	if (slave_ops->ndo_set_mac_address == NULL) {
		netdev_warn(bond_dev, "The slave device specified does not support setting the MAC address\n");
		if (BOND_MODE(bond) == BOND_MODE_ACTIVEBACKUP &&
		    bond->params.fail_over_mac != BOND_FOM_ACTIVE) {
			if (!bond_has_slaves(bond)) {
				bond->params.fail_over_mac = BOND_FOM_ACTIVE;
				netdev_warn(bond_dev, "Setting fail_over_mac to active for active-backup mode\n");
			} else {
				netdev_err(bond_dev, "The slave device specified does not support setting the MAC address, but fail_over_mac is not set to active\n");
				res = -EOPNOTSUPP;
				goto err_undo_flags;
			}
		}
	}

	call_netdevice_notifiers(NETDEV_JOIN, slave_dev);

	/* If this is the first slave, then we need to set the master's hardware
	 * address to be the same as the slave's.
	 */
	if (!bond_has_slaves(bond) &&
	    bond->dev->addr_assign_type == NET_ADDR_RANDOM)
		bond_set_dev_addr(bond->dev, slave_dev);

	new_slave = bond_alloc_slave(bond);
	if (!new_slave) {
		res = -ENOMEM;
		goto err_undo_flags;
	}

	new_slave->bond = bond;
	new_slave->dev = slave_dev;
	/* Set the new_slave's queue_id to be zero.  Queue ID mapping
	 * is set via sysfs or module option if desired.
	 */
	new_slave->queue_id = 0;

	/* Save slave's original mtu and then set it to match the bond */
	new_slave->original_mtu = slave_dev->mtu;
	res = dev_set_mtu(slave_dev, bond->dev->mtu);
	if (res) {
		netdev_dbg(bond_dev, "Error %d calling dev_set_mtu\n", res);
		goto err_free;
	}

	/* Save slave's original ("permanent") mac address for modes
	 * that need it, and for restoring it upon release, and then
	 * set it to the master's address
	 */
	ether_addr_copy(new_slave->perm_hwaddr, slave_dev->dev_addr);

	if (!bond->params.fail_over_mac ||
	    BOND_MODE(bond) != BOND_MODE_ACTIVEBACKUP) {
		/* Set slave to master's mac address.  The application already
		 * set the master's mac address to that of the first slave
		 */
		memcpy(addr.sa_data, bond_dev->dev_addr, bond_dev->addr_len);
		addr.sa_family = slave_dev->type;
		res = dev_set_mac_address(slave_dev, &addr);
		if (res) {
			netdev_dbg(bond_dev, "Error %d calling set_mac_address\n", res);
			goto err_restore_mtu;
		}
	}

	/* set slave flag before open to prevent IPv6 addrconf */
	slave_dev->flags |= IFF_SLAVE;

	/* open the slave since the application closed it */
	res = dev_open(slave_dev);
	if (res) {
		netdev_dbg(bond_dev, "Opening slave %s failed\n", slave_dev->name);
		goto err_restore_mac;
	}

	slave_dev->priv_flags |= IFF_BONDING;
	/* initialize slave stats */
	dev_get_stats(new_slave->dev, &new_slave->slave_stats);

	if (bond_is_lb(bond)) {
		/* bond_alb_init_slave() must be called before all other stages since
		 * it might fail and we do not want to have to undo everything
		 */
		res = bond_alb_init_slave(bond, new_slave);
		if (res)
			goto err_close;
	}

	res = vlan_vids_add_by_dev(slave_dev, bond_dev);
	if (res) {
		netdev_err(bond_dev, "Couldn't add bond vlan ids to %s\n",
			   slave_dev->name);
		goto err_close;
	}

	prev_slave = bond_last_slave(bond);

	new_slave->delay = 0;
	new_slave->link_failure_count = 0;

	bond_update_speed_duplex(new_slave);

	new_slave->last_rx = jiffies -
		(msecs_to_jiffies(bond->params.arp_interval) + 1);
	for (i = 0; i < BOND_MAX_ARP_TARGETS; i++)
		new_slave->target_last_arp_rx[i] = new_slave->last_rx;

	if (bond->params.miimon && !bond->params.use_carrier) {
		link_reporting = bond_check_dev_link(bond, slave_dev, 1);

		if ((link_reporting == -1) && !bond->params.arp_interval) {
			/* miimon is set but a bonded network driver
			 * does not support ETHTOOL/MII and
			 * arp_interval is not set.  Note: if
			 * use_carrier is enabled, we will never go
			 * here (because netif_carrier is always
			 * supported); thus, we don't need to change
			 * the messages for netif_carrier.
			 */
			netdev_warn(bond_dev, "MII and ETHTOOL support not available for interface %s, and arp_interval/arp_ip_target module parameters not specified, thus bonding will not detect link failures! see bonding.txt for details\n",
				    slave_dev->name);
		} else if (link_reporting == -1) {
			/* unable get link status using mii/ethtool */
			netdev_warn(bond_dev, "can't get link status from interface %s; the network driver associated with this interface does not support MII or ETHTOOL link status reporting, thus miimon has no effect on this interface\n",
				    slave_dev->name);
		}
	}

	/* check for initial state */
	if (bond->params.miimon) {
		if (bond_check_dev_link(bond, slave_dev, 0) == BMSR_LSTATUS) {
			if (bond->params.updelay) {
				bond_set_slave_link_state(new_slave,
							  BOND_LINK_BACK);
				new_slave->delay = bond->params.updelay;
			} else {
				bond_set_slave_link_state(new_slave,
							  BOND_LINK_UP);
			}
		} else {
			bond_set_slave_link_state(new_slave, BOND_LINK_DOWN);
		}
	} else if (bond->params.arp_interval) {
		bond_set_slave_link_state(new_slave,
					  (netif_carrier_ok(slave_dev) ?
					  BOND_LINK_UP : BOND_LINK_DOWN));
	} else {
		bond_set_slave_link_state(new_slave, BOND_LINK_UP);
	}

	if (new_slave->link != BOND_LINK_DOWN)
		new_slave->last_link_up = jiffies;
	netdev_dbg(bond_dev, "Initial state of slave_dev is BOND_LINK_%s\n",
		   new_slave->link == BOND_LINK_DOWN ? "DOWN" :
		   (new_slave->link == BOND_LINK_UP ? "UP" : "BACK"));

	if (bond_uses_primary(bond) && bond->params.primary[0]) {
		/* if there is a primary slave, remember it */
		if (strcmp(bond->params.primary, new_slave->dev->name) == 0) {
			rcu_assign_pointer(bond->primary_slave, new_slave);
			bond->force_primary = true;
		}
	}

	switch (BOND_MODE(bond)) {
	case BOND_MODE_ACTIVEBACKUP:
		bond_set_slave_inactive_flags(new_slave,
					      BOND_SLAVE_NOTIFY_NOW);
		break;
	case BOND_MODE_8023AD:
		/* in 802.3ad mode, the internal mechanism
		 * will activate the slaves in the selected
		 * aggregator
		 */
		bond_set_slave_inactive_flags(new_slave, BOND_SLAVE_NOTIFY_NOW);
		/* if this is the first slave */
		if (!prev_slave) {
			SLAVE_AD_INFO(new_slave)->id = 1;
			/* Initialize AD with the number of times that the AD timer is called in 1 second
			 * can be called only after the mac address of the bond is set
			 */
			bond_3ad_initialize(bond, 1000/AD_TIMER_INTERVAL);
		} else {
			SLAVE_AD_INFO(new_slave)->id =
				SLAVE_AD_INFO(prev_slave)->id + 1;
		}

		bond_3ad_bind_slave(new_slave);
		break;
	case BOND_MODE_TLB:
	case BOND_MODE_ALB:
		bond_set_active_slave(new_slave);
		bond_set_slave_inactive_flags(new_slave, BOND_SLAVE_NOTIFY_NOW);
		break;
	default:
		netdev_dbg(bond_dev, "This slave is always active in trunk mode\n");

		/* always active in trunk mode */
		bond_set_active_slave(new_slave);

		/* In trunking mode there is little meaning to curr_active_slave
		 * anyway (it holds no special properties of the bond device),
		 * so we can change it without calling change_active_interface()
		 */
		if (!rcu_access_pointer(bond->curr_active_slave) &&
		    new_slave->link == BOND_LINK_UP)
			rcu_assign_pointer(bond->curr_active_slave, new_slave);

		break;
	} /* switch(bond_mode) */

#ifdef CONFIG_NET_POLL_CONTROLLER
	if (bond->dev->npinfo) {
		if (slave_enable_netpoll(new_slave)) {
			netdev_info(bond_dev, "master_dev is using netpoll, but new slave device does not support netpoll\n");
			res = -EBUSY;
			goto err_detach;
		}
	}
#endif

	if (!(bond_dev->features & NETIF_F_LRO))
		dev_disable_lro(slave_dev);

	res = netdev_rx_handler_register(slave_dev, bond_handle_frame,
					 new_slave);
	if (res) {
		netdev_dbg(bond_dev, "Error %d calling netdev_rx_handler_register\n", res);
		goto err_detach;
	}

	res = bond_master_upper_dev_link(bond_dev, slave_dev, new_slave);
	if (res) {
		netdev_dbg(bond_dev, "Error %d calling bond_master_upper_dev_link\n", res);
		goto err_unregister;
	}

	res = bond_sysfs_slave_add(new_slave);
	if (res) {
		netdev_dbg(bond_dev, "Error %d calling bond_sysfs_slave_add\n", res);
		goto err_upper_unlink;
	}

		/* If the mode uses primary, then the following is handled by
	 * bond_change_active_slave().
	 */
	if (!bond_uses_primary(bond)) {
		/* set promiscuity level to new slave */
		if (bond_dev->flags & IFF_PROMISC) {
			res = dev_set_promiscuity(slave_dev, 1);
			if (res)
				goto err_sysfs_del;
		}

		/* set allmulti level to new slave */
		if (bond_dev->flags & IFF_ALLMULTI) {
			res = dev_set_allmulti(slave_dev, 1);
			if (res) {
				if (bond_dev->flags & IFF_PROMISC)
					dev_set_promiscuity(slave_dev, -1);
				goto err_sysfs_del;
			}
		}

		netif_addr_lock_bh(bond_dev);
		dev_mc_sync_multiple(slave_dev, bond_dev);
		dev_uc_sync_multiple(slave_dev, bond_dev);
		netif_addr_unlock_bh(bond_dev);

		if (BOND_MODE(bond) == BOND_MODE_8023AD) {
			/* add lacpdu mc addr to mc list */
			u8 lacpdu_multicast[ETH_ALEN] = MULTICAST_LACPDU_ADDR;

			dev_mc_add(slave_dev, lacpdu_multicast);
		}
	}

	bond->slave_cnt++;
	bond_compute_features(bond);
	bond_set_carrier(bond);

	if (bond_uses_primary(bond)) {
		block_netpoll_tx();
		bond_select_active_slave(bond);
		unblock_netpoll_tx();
	}

	if (bond_mode_uses_xmit_hash(bond))
		bond_update_slave_arr(bond, NULL);

	netdev_info(bond_dev, "Enslaving %s as %s interface with %s link\n",
		    slave_dev->name,
		    bond_is_active_slave(new_slave) ? "an active" : "a backup",
		    new_slave->link != BOND_LINK_DOWN ? "an up" : "a down");

	/* enslave is successful */
	bond_queue_slave_event(new_slave);

	return 0;

/* Undo stages on error */
err_sysfs_del:
	bond_sysfs_slave_del(new_slave);

err_upper_unlink:
	bond_upper_dev_unlink(bond_dev, slave_dev);

err_unregister:
	netdev_rx_handler_unregister(slave_dev);

err_detach:
	vlan_vids_del_by_dev(slave_dev, bond_dev);
	if (rcu_access_pointer(bond->primary_slave) == new_slave)
		RCU_INIT_POINTER(bond->primary_slave, NULL);
	if (rcu_access_pointer(bond->curr_active_slave) == new_slave) {
		block_netpoll_tx();
		bond_change_active_slave(bond, NULL);
		bond_select_active_slave(bond);
		unblock_netpoll_tx();
	}
	/* either primary_slave or curr_active_slave might've changed */
	synchronize_rcu();
	slave_disable_netpoll(new_slave);

err_close:
	slave_dev->priv_flags &= ~IFF_BONDING;
	dev_close(slave_dev);

err_restore_mac:
	slave_dev->flags &= ~IFF_SLAVE;
	if (!bond->params.fail_over_mac ||
	    BOND_MODE(bond) != BOND_MODE_ACTIVEBACKUP) {
		/* XXX TODO - fom follow mode needs to change master's
		 * MAC if this slave's MAC is in use by the bond, or at
		 * least print a warning.
		 */
		ether_addr_copy(addr.sa_data, new_slave->perm_hwaddr);
		addr.sa_family = slave_dev->type;
		dev_set_mac_address(slave_dev, &addr);
	}

err_restore_mtu:
	dev_set_mtu(slave_dev, new_slave->original_mtu);

err_free:
	bond_free_slave(new_slave);

err_undo_flags:
	/* Enslave of first slave has failed and we need to fix master's mac */
	if (!bond_has_slaves(bond) &&
	    ether_addr_equal_64bits(bond_dev->dev_addr, slave_dev->dev_addr))
		eth_hw_addr_random(bond_dev);

	return res;
}

/* Try to release the slave device <slave> from the bond device <master>
 * It is legal to access curr_active_slave without a lock because all the function
 * is RTNL-locked. If "all" is true it means that the function is being called
 * while destroying a bond interface and all slaves are being released.
 *
 * The rules for slave state should be:
 *   for Active/Backup:
 *     Active stays on all backups go down
 *   for Bonded connections:
 *     The first up interface should be left on and all others downed.
 */
static int __bond_release_one(struct net_device *bond_dev,
			      struct net_device *slave_dev,
			      bool all)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave, *oldcurrent;
	struct sockaddr addr;
	int old_flags = bond_dev->flags;
	netdev_features_t old_features = bond_dev->features;

	/* slave is not a slave or master is not master of this slave */
	if (!(slave_dev->flags & IFF_SLAVE) ||
	    !netdev_has_upper_dev(slave_dev, bond_dev)) {
		netdev_dbg(bond_dev, "cannot release %s\n",
			   slave_dev->name);
		return -EINVAL;
	}

	block_netpoll_tx();

	slave = bond_get_slave_by_dev(bond, slave_dev);
	if (!slave) {
		/* not a slave of this bond */
		netdev_info(bond_dev, "%s not enslaved\n",
			    slave_dev->name);
		unblock_netpoll_tx();
		return -EINVAL;
	}

	bond_sysfs_slave_del(slave);

	/* recompute stats just before removing the slave */
	bond_get_stats(bond->dev, &bond->bond_stats);

	bond_upper_dev_unlink(bond_dev, slave_dev);
	/* unregister rx_handler early so bond_handle_frame wouldn't be called
	 * for this slave anymore.
	 */
	netdev_rx_handler_unregister(slave_dev);

	if (BOND_MODE(bond) == BOND_MODE_8023AD)
		bond_3ad_unbind_slave(slave);

	if (bond_mode_uses_xmit_hash(bond))
		bond_update_slave_arr(bond, slave);

	netdev_info(bond_dev, "Releasing %s interface %s\n",
		    bond_is_active_slave(slave) ? "active" : "backup",
		    slave_dev->name);

	oldcurrent = rcu_access_pointer(bond->curr_active_slave);

	RCU_INIT_POINTER(bond->current_arp_slave, NULL);

	if (!all && (!bond->params.fail_over_mac ||
		     BOND_MODE(bond) != BOND_MODE_ACTIVEBACKUP)) {
		if (ether_addr_equal_64bits(bond_dev->dev_addr, slave->perm_hwaddr) &&
		    bond_has_slaves(bond))
			netdev_warn(bond_dev, "the permanent HWaddr of %s - %pM - is still in use by %s - set the HWaddr of %s to a different address to avoid conflicts\n",
				    slave_dev->name, slave->perm_hwaddr,
				    bond_dev->name, slave_dev->name);
	}

	if (rtnl_dereference(bond->primary_slave) == slave)
		RCU_INIT_POINTER(bond->primary_slave, NULL);

	if (oldcurrent == slave)
		bond_change_active_slave(bond, NULL);

	if (bond_is_lb(bond)) {
		/* Must be called only after the slave has been
		 * detached from the list and the curr_active_slave
		 * has been cleared (if our_slave == old_current),
		 * but before a new active slave is selected.
		 */
		bond_alb_deinit_slave(bond, slave);
	}

	if (all) {
		RCU_INIT_POINTER(bond->curr_active_slave, NULL);
	} else if (oldcurrent == slave) {
		/* Note that we hold RTNL over this sequence, so there
		 * is no concern that another slave add/remove event
		 * will interfere.
		 */
		bond_select_active_slave(bond);
	}

	if (!bond_has_slaves(bond)) {
		bond_set_carrier(bond);
		eth_hw_addr_random(bond_dev);
	}

	unblock_netpoll_tx();
	synchronize_rcu();
	bond->slave_cnt--;

	if (!bond_has_slaves(bond)) {
		call_netdevice_notifiers(NETDEV_CHANGEADDR, bond->dev);
		call_netdevice_notifiers(NETDEV_RELEASE, bond->dev);
	}

	bond_compute_features(bond);
	if (!(bond_dev->features & NETIF_F_VLAN_CHALLENGED) &&
	    (old_features & NETIF_F_VLAN_CHALLENGED))
		netdev_info(bond_dev, "last VLAN challenged slave %s left bond %s - VLAN blocking is removed\n",
			    slave_dev->name, bond_dev->name);

	vlan_vids_del_by_dev(slave_dev, bond_dev);

	/* If the mode uses primary, then this case was handled above by
	 * bond_change_active_slave(..., NULL)
	 */
	if (!bond_uses_primary(bond)) {
		/* unset promiscuity level from slave
		 * NOTE: The NETDEV_CHANGEADDR call above may change the value
		 * of the IFF_PROMISC flag in the bond_dev, but we need the
		 * value of that flag before that change, as that was the value
		 * when this slave was attached, so we cache at the start of the
		 * function and use it here. Same goes for ALLMULTI below
		 */
		if (old_flags & IFF_PROMISC)
			dev_set_promiscuity(slave_dev, -1);

		/* unset allmulti level from slave */
		if (old_flags & IFF_ALLMULTI)
			dev_set_allmulti(slave_dev, -1);

		bond_hw_addr_flush(bond_dev, slave_dev);
	}

	slave_disable_netpoll(slave);

	/* close slave before restoring its mac address */
	dev_close(slave_dev);

	if (bond->params.fail_over_mac != BOND_FOM_ACTIVE ||
	    BOND_MODE(bond) != BOND_MODE_ACTIVEBACKUP) {
		/* restore original ("permanent") mac address */
		ether_addr_copy(addr.sa_data, slave->perm_hwaddr);
		addr.sa_family = slave_dev->type;
		dev_set_mac_address(slave_dev, &addr);
	}

	dev_set_mtu(slave_dev, slave->original_mtu);

	slave_dev->priv_flags &= ~IFF_BONDING;

	bond_free_slave(slave);

	return 0;
}

/* A wrapper used because of ndo_del_link */
int bond_release(struct net_device *bond_dev, struct net_device *slave_dev)
{
	return __bond_release_one(bond_dev, slave_dev, false);
}

/* First release a slave and then destroy the bond if no more slaves are left.
 * Must be under rtnl_lock when this function is called.
 */
static int  bond_release_and_destroy(struct net_device *bond_dev,
				     struct net_device *slave_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	int ret;

	ret = bond_release(bond_dev, slave_dev);
	if (ret == 0 && !bond_has_slaves(bond)) {
		bond_dev->priv_flags |= IFF_DISABLE_NETPOLL;
		netdev_info(bond_dev, "Destroying bond %s\n",
			    bond_dev->name);
		bond_remove_proc_entry(bond);
		unregister_netdevice(bond_dev);
	}
	return ret;
}

static int bond_info_query(struct net_device *bond_dev, struct ifbond *info)
{
	struct bonding *bond = netdev_priv(bond_dev);
	bond_fill_ifbond(bond, info);
	return 0;
}

static int bond_slave_info_query(struct net_device *bond_dev, struct ifslave *info)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct list_head *iter;
	int i = 0, res = -ENODEV;
	struct slave *slave;

	bond_for_each_slave(bond, slave, iter) {
		if (i++ == (int)info->slave_id) {
			res = 0;
			bond_fill_ifslave(slave, info);
			break;
		}
	}

	return res;
}

/*-------------------------------- Monitoring -------------------------------*/

/* called with rcu_read_lock() */
static int bond_miimon_inspect(struct bonding *bond)
{
	int link_state, commit = 0;
	struct list_head *iter;
	struct slave *slave;
	bool ignore_updelay;

	ignore_updelay = !rcu_dereference(bond->curr_active_slave);

	bond_for_each_slave_rcu(bond, slave, iter) {
		slave->new_link = BOND_LINK_NOCHANGE;

		link_state = bond_check_dev_link(bond, slave->dev, 0);

		switch (slave->link) {
		case BOND_LINK_UP:
			if (link_state)
				continue;

			bond_set_slave_link_state(slave, BOND_LINK_FAIL);
			slave->delay = bond->params.downdelay;
			if (slave->delay) {
				netdev_info(bond->dev, "link status down for %sinterface %s, disabling it in %d ms\n",
					    (BOND_MODE(bond) ==
					     BOND_MODE_ACTIVEBACKUP) ?
					     (bond_is_active_slave(slave) ?
					      "active " : "backup ") : "",
					    slave->dev->name,
					    bond->params.downdelay * bond->params.miimon);
			}
			/*FALLTHRU*/
		case BOND_LINK_FAIL:
			if (link_state) {
				/* recovered before downdelay expired */
				bond_set_slave_link_state(slave, BOND_LINK_UP);
				slave->last_link_up = jiffies;
				netdev_info(bond->dev, "link status up again after %d ms for interface %s\n",
					    (bond->params.downdelay - slave->delay) *
					    bond->params.miimon,
					    slave->dev->name);
				continue;
			}

			if (slave->delay <= 0) {
				slave->new_link = BOND_LINK_DOWN;
				commit++;
				continue;
			}

			slave->delay--;
			break;

		case BOND_LINK_DOWN:
			if (!link_state)
				continue;

			bond_set_slave_link_state(slave, BOND_LINK_BACK);
			slave->delay = bond->params.updelay;

			if (slave->delay) {
				netdev_info(bond->dev, "link status up for interface %s, enabling it in %d ms\n",
					    slave->dev->name,
					    ignore_updelay ? 0 :
					    bond->params.updelay *
					    bond->params.miimon);
			}
			/*FALLTHRU*/
		case BOND_LINK_BACK:
			if (!link_state) {
				bond_set_slave_link_state(slave,
							  BOND_LINK_DOWN);
				netdev_info(bond->dev, "link status down again after %d ms for interface %s\n",
					    (bond->params.updelay - slave->delay) *
					    bond->params.miimon,
					    slave->dev->name);

				continue;
			}

			if (ignore_updelay)
				slave->delay = 0;

			if (slave->delay <= 0) {
				slave->new_link = BOND_LINK_UP;
				commit++;
				ignore_updelay = false;
				continue;
			}

			slave->delay--;
			break;
		}
	}

	return commit;
}

static void bond_miimon_commit(struct bonding *bond)
{
	struct list_head *iter;
	struct slave *slave, *primary;

	bond_for_each_slave(bond, slave, iter) {
		switch (slave->new_link) {
		case BOND_LINK_NOCHANGE:
			continue;

		case BOND_LINK_UP:
			bond_set_slave_link_state(slave, BOND_LINK_UP);
			slave->last_link_up = jiffies;

			primary = rtnl_dereference(bond->primary_slave);
			if (BOND_MODE(bond) == BOND_MODE_8023AD) {
				/* prevent it from being the active one */
				bond_set_backup_slave(slave);
			} else if (BOND_MODE(bond) != BOND_MODE_ACTIVEBACKUP) {
				/* make it immediately active */
				bond_set_active_slave(slave);
			} else if (slave != primary) {
				/* prevent it from being the active one */
				bond_set_backup_slave(slave);
			}

			netdev_info(bond->dev, "link status definitely up for interface %s, %u Mbps %s duplex\n",
				    slave->dev->name,
				    slave->speed == SPEED_UNKNOWN ? 0 : slave->speed,
				    slave->duplex ? "full" : "half");

			/* notify ad that the link status has changed */
			if (BOND_MODE(bond) == BOND_MODE_8023AD)
				bond_3ad_handle_link_change(slave, BOND_LINK_UP);

			if (bond_is_lb(bond))
				bond_alb_handle_link_change(bond, slave,
							    BOND_LINK_UP);

			if (BOND_MODE(bond) == BOND_MODE_XOR)
				bond_update_slave_arr(bond, NULL);

			if (!bond->curr_active_slave || slave == primary)
				goto do_failover;

			continue;

		case BOND_LINK_DOWN:
			if (slave->link_failure_count < UINT_MAX)
				slave->link_failure_count++;

			bond_set_slave_link_state(slave, BOND_LINK_DOWN);

			if (BOND_MODE(bond) == BOND_MODE_ACTIVEBACKUP ||
			    BOND_MODE(bond) == BOND_MODE_8023AD)
				bond_set_slave_inactive_flags(slave,
							      BOND_SLAVE_NOTIFY_NOW);

			netdev_info(bond->dev, "link status definitely down for interface %s, disabling it\n",
				    slave->dev->name);

			if (BOND_MODE(bond) == BOND_MODE_8023AD)
				bond_3ad_handle_link_change(slave,
							    BOND_LINK_DOWN);

			if (bond_is_lb(bond))
				bond_alb_handle_link_change(bond, slave,
							    BOND_LINK_DOWN);

			if (BOND_MODE(bond) == BOND_MODE_XOR)
				bond_update_slave_arr(bond, NULL);

			if (slave == rcu_access_pointer(bond->curr_active_slave))
				goto do_failover;

			continue;

		default:
			netdev_err(bond->dev, "invalid new link %d on slave %s\n",
				   slave->new_link, slave->dev->name);
			slave->new_link = BOND_LINK_NOCHANGE;

			continue;
		}

do_failover:
		block_netpoll_tx();
		bond_select_active_slave(bond);
		unblock_netpoll_tx();
	}

	bond_set_carrier(bond);
}

/* bond_mii_monitor
 *
 * Really a wrapper that splits the mii monitor into two phases: an
 * inspection, then (if inspection indicates something needs to be done)
 * an acquisition of appropriate locks followed by a commit phase to
 * implement whatever link state changes are indicated.
 */
static void bond_mii_monitor(struct work_struct *work)
{
	struct bonding *bond = container_of(work, struct bonding,
					    mii_work.work);
	bool should_notify_peers = false;
	unsigned long delay;

	delay = msecs_to_jiffies(bond->params.miimon);

	if (!bond_has_slaves(bond))
		goto re_arm;

	rcu_read_lock();

	should_notify_peers = bond_should_notify_peers(bond);

	if (bond_miimon_inspect(bond)) {
		rcu_read_unlock();

		/* Race avoidance with bond_close cancel of workqueue */
		if (!rtnl_trylock()) {
			delay = 1;
			should_notify_peers = false;
			goto re_arm;
		}

		bond_miimon_commit(bond);

		rtnl_unlock();	/* might sleep, hold no other locks */
	} else
		rcu_read_unlock();

re_arm:
	if (bond->params.miimon)
		queue_delayed_work(bond->wq, &bond->mii_work, delay);

	if (should_notify_peers) {
		if (!rtnl_trylock())
			return;
		call_netdevice_notifiers(NETDEV_NOTIFY_PEERS, bond->dev);
		rtnl_unlock();
	}
}

static bool bond_has_this_ip(struct bonding *bond, __be32 ip)
{
	struct net_device *upper;
	struct list_head *iter;
	bool ret = false;

	if (ip == bond_confirm_addr(bond->dev, 0, ip))
		return true;

	rcu_read_lock();
	netdev_for_each_all_upper_dev_rcu(bond->dev, upper, iter) {
		if (ip == bond_confirm_addr(upper, 0, ip)) {
			ret = true;
			break;
		}
	}
	rcu_read_unlock();

	return ret;
}

/* We go to the (large) trouble of VLAN tagging ARP frames because
 * switches in VLAN mode (especially if ports are configured as
 * "native" to a VLAN) might not pass non-tagged frames.
 */
static void bond_arp_send(struct net_device *slave_dev, int arp_op,
			  __be32 dest_ip, __be32 src_ip,
			  struct bond_vlan_tag *tags)
{
	struct sk_buff *skb;
	struct bond_vlan_tag *outer_tag = tags;

	netdev_dbg(slave_dev, "arp %d on slave %s: dst %pI4 src %pI4\n",
		   arp_op, slave_dev->name, &dest_ip, &src_ip);

	skb = arp_create(arp_op, ETH_P_ARP, dest_ip, slave_dev, src_ip,
			 NULL, slave_dev->dev_addr, NULL);

	if (!skb) {
		net_err_ratelimited("ARP packet allocation failed\n");
		return;
	}

	if (!tags || tags->vlan_proto == VLAN_N_VID)
		goto xmit;

	tags++;

	/* Go through all the tags backwards and add them to the packet */
	while (tags->vlan_proto != VLAN_N_VID) {
		if (!tags->vlan_id) {
			tags++;
			continue;
		}

		netdev_dbg(slave_dev, "inner tag: proto %X vid %X\n",
			   ntohs(outer_tag->vlan_proto), tags->vlan_id);
		skb = vlan_insert_tag_set_proto(skb, tags->vlan_proto,
						tags->vlan_id);
		if (!skb) {
			net_err_ratelimited("failed to insert inner VLAN tag\n");
			return;
		}

		tags++;
	}
	/* Set the outer tag */
	if (outer_tag->vlan_id) {
		netdev_dbg(slave_dev, "outer tag: proto %X vid %X\n",
			   ntohs(outer_tag->vlan_proto), outer_tag->vlan_id);
		__vlan_hwaccel_put_tag(skb, outer_tag->vlan_proto,
				       outer_tag->vlan_id);
	}

xmit:
	arp_xmit(skb);
}

/* Validate the device path between the @start_dev and the @end_dev.
 * The path is valid if the @end_dev is reachable through device
 * stacking.
 * When the path is validated, collect any vlan information in the
 * path.
 */
struct bond_vlan_tag *bond_verify_device_path(struct net_device *start_dev,
					      struct net_device *end_dev,
					      int level)
{
	struct bond_vlan_tag *tags;
	struct net_device *upper;
	struct list_head  *iter;

	if (start_dev == end_dev) {
		tags = kzalloc(sizeof(*tags) * (level + 1), GFP_ATOMIC);
		if (!tags)
			return ERR_PTR(-ENOMEM);
		tags[level].vlan_proto = VLAN_N_VID;
		return tags;
	}

	netdev_for_each_upper_dev_rcu(start_dev, upper, iter) {
		tags = bond_verify_device_path(upper, end_dev, level + 1);
		if (IS_ERR_OR_NULL(tags)) {
			if (IS_ERR(tags))
				return tags;
			continue;
		}
		if (is_vlan_dev(upper)) {
			tags[level].vlan_proto = vlan_dev_vlan_proto(upper);
			tags[level].vlan_id = vlan_dev_vlan_id(upper);
		}

		return tags;
	}

	return NULL;
}

static void bond_arp_send_all(struct bonding *bond, struct slave *slave)
{
	struct rtable *rt;
	struct bond_vlan_tag *tags;
	__be32 *targets = bond->params.arp_targets, addr;
	int i;

	for (i = 0; i < BOND_MAX_ARP_TARGETS && targets[i]; i++) {
		netdev_dbg(bond->dev, "basa: target %pI4\n", &targets[i]);
		tags = NULL;

		/* Find out through which dev should the packet go */
		rt = ip_route_output(dev_net(bond->dev), targets[i], 0,
				     RTO_ONLINK, 0);
		if (IS_ERR(rt)) {
			/* there's no route to target - try to send arp
			 * probe to generate any traffic (arp_validate=0)
			 */
			if (bond->params.arp_validate)
				net_warn_ratelimited("%s: no route to arp_ip_target %pI4 and arp_validate is set\n",
						     bond->dev->name,
						     &targets[i]);
			bond_arp_send(slave->dev, ARPOP_REQUEST, targets[i],
				      0, tags);
			continue;
		}

		/* bond device itself */
		if (rt->dst.dev == bond->dev)
			goto found;

		rcu_read_lock();
		tags = bond_verify_device_path(bond->dev, rt->dst.dev, 0);
		rcu_read_unlock();

		if (!IS_ERR_OR_NULL(tags))
			goto found;

		/* Not our device - skip */
		netdev_dbg(bond->dev, "no path to arp_ip_target %pI4 via rt.dev %s\n",
			   &targets[i], rt->dst.dev ? rt->dst.dev->name : "NULL");

		ip_rt_put(rt);
		continue;

found:
		addr = bond_confirm_addr(rt->dst.dev, targets[i], 0);
		ip_rt_put(rt);
		bond_arp_send(slave->dev, ARPOP_REQUEST, targets[i],
			      addr, tags);
		kfree(tags);
	}
}

static void bond_validate_arp(struct bonding *bond, struct slave *slave, __be32 sip, __be32 tip)
{
	int i;

	if (!sip || !bond_has_this_ip(bond, tip)) {
		netdev_dbg(bond->dev, "bva: sip %pI4 tip %pI4 not found\n",
			   &sip, &tip);
		return;
	}

	i = bond_get_targets_ip(bond->params.arp_targets, sip);
	if (i == -1) {
		netdev_dbg(bond->dev, "bva: sip %pI4 not found in targets\n",
			   &sip);
		return;
	}
	slave->last_rx = jiffies;
	slave->target_last_arp_rx[i] = jiffies;
}

int bond_arp_rcv(const struct sk_buff *skb, struct bonding *bond,
		 struct slave *slave)
{
	struct arphdr *arp = (struct arphdr *)skb->data;
	struct slave *curr_active_slave, *curr_arp_slave;
	unsigned char *arp_ptr;
	__be32 sip, tip;
	int alen, is_arp = skb->protocol == __cpu_to_be16(ETH_P_ARP);

	if (!slave_do_arp_validate(bond, slave)) {
		if ((slave_do_arp_validate_only(bond) && is_arp) ||
		    !slave_do_arp_validate_only(bond))
			slave->last_rx = jiffies;
		return RX_HANDLER_ANOTHER;
	} else if (!is_arp) {
		return RX_HANDLER_ANOTHER;
	}

	alen = arp_hdr_len(bond->dev);

	netdev_dbg(bond->dev, "bond_arp_rcv: skb->dev %s\n",
		   skb->dev->name);

	if (alen > skb_headlen(skb)) {
		arp = kmalloc(alen, GFP_ATOMIC);
		if (!arp)
			goto out_unlock;
		if (skb_copy_bits(skb, 0, arp, alen) < 0)
			goto out_unlock;
	}

	if (arp->ar_hln != bond->dev->addr_len ||
	    skb->pkt_type == PACKET_OTHERHOST ||
	    skb->pkt_type == PACKET_LOOPBACK ||
	    arp->ar_hrd != htons(ARPHRD_ETHER) ||
	    arp->ar_pro != htons(ETH_P_IP) ||
	    arp->ar_pln != 4)
		goto out_unlock;

	arp_ptr = (unsigned char *)(arp + 1);
	arp_ptr += bond->dev->addr_len;
	memcpy(&sip, arp_ptr, 4);
	arp_ptr += 4 + bond->dev->addr_len;
	memcpy(&tip, arp_ptr, 4);

	netdev_dbg(bond->dev, "bond_arp_rcv: %s/%d av %d sv %d sip %pI4 tip %pI4\n",
		   slave->dev->name, bond_slave_state(slave),
		     bond->params.arp_validate, slave_do_arp_validate(bond, slave),
		     &sip, &tip);

	curr_active_slave = rcu_dereference(bond->curr_active_slave);
	curr_arp_slave = rcu_dereference(bond->current_arp_slave);

	/* We 'trust' the received ARP enough to validate it if:
	 *
	 * (a) the slave receiving the ARP is active (which includes the
	 * current ARP slave, if any), or
	 *
	 * (b) the receiving slave isn't active, but there is a currently
	 * active slave and it received valid arp reply(s) after it became
	 * the currently active slave, or
	 *
	 * (c) there is an ARP slave that sent an ARP during the prior ARP
	 * interval, and we receive an ARP reply on any slave.  We accept
	 * these because switch FDB update delays may deliver the ARP
	 * reply to a slave other than the sender of the ARP request.
	 *
	 * Note: for (b), backup slaves are receiving the broadcast ARP
	 * request, not a reply.  This request passes from the sending
	 * slave through the L2 switch(es) to the receiving slave.  Since
	 * this is checking the request, sip/tip are swapped for
	 * validation.
	 *
	 * This is done to avoid endless looping when we can't reach the
	 * arp_ip_target and fool ourselves with our own arp requests.
	 */
	if (bond_is_active_slave(slave))
		bond_validate_arp(bond, slave, sip, tip);
	else if (curr_active_slave &&
		 time_after(slave_last_rx(bond, curr_active_slave),
			    curr_active_slave->last_link_up))
		bond_validate_arp(bond, slave, tip, sip);
	else if (curr_arp_slave && (arp->ar_op == htons(ARPOP_REPLY)) &&
		 bond_time_in_interval(bond,
				       dev_trans_start(curr_arp_slave->dev), 1))
		bond_validate_arp(bond, slave, sip, tip);

out_unlock:
	if (arp != (struct arphdr *)skb->data)
		kfree(arp);
	return RX_HANDLER_ANOTHER;
}

/* function to verify if we're in the arp_interval timeslice, returns true if
 * (last_act - arp_interval) <= jiffies <= (last_act + mod * arp_interval +
 * arp_interval/2) . the arp_interval/2 is needed for really fast networks.
 */
static bool bond_time_in_interval(struct bonding *bond, unsigned long last_act,
				  int mod)
{
	int delta_in_ticks = msecs_to_jiffies(bond->params.arp_interval);

	return time_in_range(jiffies,
			     last_act - delta_in_ticks,
			     last_act + mod * delta_in_ticks + delta_in_ticks/2);
}

/* This function is called regularly to monitor each slave's link
 * ensuring that traffic is being sent and received when arp monitoring
 * is used in load-balancing mode. if the adapter has been dormant, then an
 * arp is transmitted to generate traffic. see activebackup_arp_monitor for
 * arp monitoring in active backup mode.
 */
static void bond_loadbalance_arp_mon(struct work_struct *work)
{
	struct bonding *bond = container_of(work, struct bonding,
					    arp_work.work);
	struct slave *slave, *oldcurrent;
	struct list_head *iter;
	int do_failover = 0, slave_state_changed = 0;

	if (!bond_has_slaves(bond))
		goto re_arm;

	rcu_read_lock();

	oldcurrent = rcu_dereference(bond->curr_active_slave);
	/* see if any of the previous devices are up now (i.e. they have
	 * xmt and rcv traffic). the curr_active_slave does not come into
	 * the picture unless it is null. also, slave->last_link_up is not
	 * needed here because we send an arp on each slave and give a slave
	 * as long as it needs to get the tx/rx within the delta.
	 * TODO: what about up/down delay in arp mode? it wasn't here before
	 *       so it can wait
	 */
	bond_for_each_slave_rcu(bond, slave, iter) {
		unsigned long trans_start = dev_trans_start(slave->dev);

		slave->new_link = BOND_LINK_NOCHANGE;

		if (slave->link != BOND_LINK_UP) {
			if (bond_time_in_interval(bond, trans_start, 1) &&
			    bond_time_in_interval(bond, slave->last_rx, 1)) {

				slave->new_link = BOND_LINK_UP;
				slave_state_changed = 1;

				/* primary_slave has no meaning in round-robin
				 * mode. the window of a slave being up and
				 * curr_active_slave being null after enslaving
				 * is closed.
				 */
				if (!oldcurrent) {
					netdev_info(bond->dev, "link status definitely up for interface %s\n",
						    slave->dev->name);
					do_failover = 1;
				} else {
					netdev_info(bond->dev, "interface %s is now up\n",
						    slave->dev->name);
				}
			}
		} else {
			/* slave->link == BOND_LINK_UP */

			/* not all switches will respond to an arp request
			 * when the source ip is 0, so don't take the link down
			 * if we don't know our ip yet
			 */
			if (!bond_time_in_interval(bond, trans_start, 2) ||
			    !bond_time_in_interval(bond, slave->last_rx, 2)) {

				slave->new_link = BOND_LINK_DOWN;
				slave_state_changed = 1;

				if (slave->link_failure_count < UINT_MAX)
					slave->link_failure_count++;

				netdev_info(bond->dev, "interface %s is now down\n",
					    slave->dev->name);

				if (slave == oldcurrent)
					do_failover = 1;
			}
		}

		/* note: if switch is in round-robin mode, all links
		 * must tx arp to ensure all links rx an arp - otherwise
		 * links may oscillate or not come up at all; if switch is
		 * in something like xor mode, there is nothing we can
		 * do - all replies will be rx'ed on same link causing slaves
		 * to be unstable during low/no traffic periods
		 */
		if (bond_slave_is_up(slave))
			bond_arp_send_all(bond, slave);
	}

	rcu_read_unlock();

	if (do_failover || slave_state_changed) {
		if (!rtnl_trylock())
			goto re_arm;

		bond_for_each_slave(bond, slave, iter) {
			if (slave->new_link != BOND_LINK_NOCHANGE)
				slave->link = slave->new_link;
		}

		if (slave_state_changed) {
			bond_slave_state_change(bond);
			if (BOND_MODE(bond) == BOND_MODE_XOR)
				bond_update_slave_arr(bond, NULL);
		}
		if (do_failover) {
			block_netpoll_tx();
			bond_select_active_slave(bond);
			unblock_netpoll_tx();
		}
		rtnl_unlock();
	}

re_arm:
	if (bond->params.arp_interval)
		queue_delayed_work(bond->wq, &bond->arp_work,
				   msecs_to_jiffies(bond->params.arp_interval));
}

/* Called to inspect slaves for active-backup mode ARP monitor link state
 * changes.  Sets new_link in slaves to specify what action should take
 * place for the slave.  Returns 0 if no changes are found, >0 if changes
 * to link states must be committed.
 *
 * Called with rcu_read_lock held.
 */
static int bond_ab_arp_inspect(struct bonding *bond)
{
	unsigned long trans_start, last_rx;
	struct list_head *iter;
	struct slave *slave;
	int commit = 0;

	bond_for_each_slave_rcu(bond, slave, iter) {
		slave->new_link = BOND_LINK_NOCHANGE;
		last_rx = slave_last_rx(bond, slave);

		if (slave->link != BOND_LINK_UP) {
			if (bond_time_in_interval(bond, last_rx, 1)) {
				slave->new_link = BOND_LINK_UP;
				commit++;
			}
			continue;
		}

		/* Give slaves 2*delta after being enslaved or made
		 * active.  This avoids bouncing, as the last receive
		 * times need a full ARP monitor cycle to be updated.
		 */
		if (bond_time_in_interval(bond, slave->last_link_up, 2))
			continue;

		/* Backup slave is down if:
		 * - No current_arp_slave AND
		 * - more than 3*delta since last receive AND
		 * - the bond has an IP address
		 *
		 * Note: a non-null current_arp_slave indicates
		 * the curr_active_slave went down and we are
		 * searching for a new one; under this condition
		 * we only take the curr_active_slave down - this
		 * gives each slave a chance to tx/rx traffic
		 * before being taken out
		 */
		if (!bond_is_active_slave(slave) &&
		    !rcu_access_pointer(bond->current_arp_slave) &&
		    !bond_time_in_interval(bond, last_rx, 3)) {
			slave->new_link = BOND_LINK_DOWN;
			commit++;
		}

		/* Active slave is down if:
		 * - more than 2*delta since transmitting OR
		 * - (more than 2*delta since receive AND
		 *    the bond has an IP address)
		 */
		trans_start = dev_trans_start(slave->dev);
		if (bond_is_active_slave(slave) &&
		    (!bond_time_in_interval(bond, trans_start, 2) ||
		     !bond_time_in_interval(bond, last_rx, 2))) {
			slave->new_link = BOND_LINK_DOWN;
			commit++;
		}
	}

	return commit;
}

/* Called to commit link state changes noted by inspection step of
 * active-backup mode ARP monitor.
 *
 * Called with RTNL hold.
 */
static void bond_ab_arp_commit(struct bonding *bond)
{
	unsigned long trans_start;
	struct list_head *iter;
	struct slave *slave;

	bond_for_each_slave(bond, slave, iter) {
		switch (slave->new_link) {
		case BOND_LINK_NOCHANGE:
			continue;

		case BOND_LINK_UP:
			trans_start = dev_trans_start(slave->dev);
			if (rtnl_dereference(bond->curr_active_slave) != slave ||
			    (!rtnl_dereference(bond->curr_active_slave) &&
			     bond_time_in_interval(bond, trans_start, 1))) {
				struct slave *current_arp_slave;

				current_arp_slave = rtnl_dereference(bond->current_arp_slave);
				bond_set_slave_link_state(slave, BOND_LINK_UP);
				if (current_arp_slave) {
					bond_set_slave_inactive_flags(
						current_arp_slave,
						BOND_SLAVE_NOTIFY_NOW);
					RCU_INIT_POINTER(bond->current_arp_slave, NULL);
				}

				netdev_info(bond->dev, "link status definitely up for interface %s\n",
					    slave->dev->name);

				if (!rtnl_dereference(bond->curr_active_slave) ||
				    slave == rtnl_dereference(bond->primary_slave))
					goto do_failover;

			}

			continue;

		case BOND_LINK_DOWN:
			if (slave->link_failure_count < UINT_MAX)
				slave->link_failure_count++;

			bond_set_slave_link_state(slave, BOND_LINK_DOWN);
			bond_set_slave_inactive_flags(slave,
						      BOND_SLAVE_NOTIFY_NOW);

			netdev_info(bond->dev, "link status definitely down for interface %s, disabling it\n",
				    slave->dev->name);

			if (slave == rtnl_dereference(bond->curr_active_slave)) {
				RCU_INIT_POINTER(bond->current_arp_slave, NULL);
				goto do_failover;
			}

			continue;

		default:
			netdev_err(bond->dev, "impossible: new_link %d on slave %s\n",
				   slave->new_link, slave->dev->name);
			continue;
		}

do_failover:
		block_netpoll_tx();
		bond_select_active_slave(bond);
		unblock_netpoll_tx();
	}

	bond_set_carrier(bond);
}

/* Send ARP probes for active-backup mode ARP monitor.
 *
 * Called with rcu_read_lock held.
 */
static bool bond_ab_arp_probe(struct bonding *bond)
{
	struct slave *slave, *before = NULL, *new_slave = NULL,
		     *curr_arp_slave = rcu_dereference(bond->current_arp_slave),
		     *curr_active_slave = rcu_dereference(bond->curr_active_slave);
	struct list_head *iter;
	bool found = false;
	bool should_notify_rtnl = BOND_SLAVE_NOTIFY_LATER;

	if (curr_arp_slave && curr_active_slave)
		netdev_info(bond->dev, "PROBE: c_arp %s && cas %s BAD\n",
			    curr_arp_slave->dev->name,
			    curr_active_slave->dev->name);

	if (curr_active_slave) {
		bond_arp_send_all(bond, curr_active_slave);
		return should_notify_rtnl;
	}

	/* if we don't have a curr_active_slave, search for the next available
	 * backup slave from the current_arp_slave and make it the candidate
	 * for becoming the curr_active_slave
	 */

	if (!curr_arp_slave) {
		curr_arp_slave = bond_first_slave_rcu(bond);
		if (!curr_arp_slave)
			return should_notify_rtnl;
	}

	bond_set_slave_inactive_flags(curr_arp_slave, BOND_SLAVE_NOTIFY_LATER);

	bond_for_each_slave_rcu(bond, slave, iter) {
		if (!found && !before && bond_slave_is_up(slave))
			before = slave;

		if (found && !new_slave && bond_slave_is_up(slave))
			new_slave = slave;
		/* if the link state is up at this point, we
		 * mark it down - this can happen if we have
		 * simultaneous link failures and
		 * reselect_active_interface doesn't make this
		 * one the current slave so it is still marked
		 * up when it is actually down
		 */
		if (!bond_slave_is_up(slave) && slave->link == BOND_LINK_UP) {
			bond_set_slave_link_state(slave, BOND_LINK_DOWN);
			if (slave->link_failure_count < UINT_MAX)
				slave->link_failure_count++;

			bond_set_slave_inactive_flags(slave,
						      BOND_SLAVE_NOTIFY_LATER);

			netdev_info(bond->dev, "backup interface %s is now down\n",
				    slave->dev->name);
		}
		if (slave == curr_arp_slave)
			found = true;
	}

	if (!new_slave && before)
		new_slave = before;

	if (!new_slave)
		goto check_state;

	bond_set_slave_link_state(new_slave, BOND_LINK_BACK);
	bond_set_slave_active_flags(new_slave, BOND_SLAVE_NOTIFY_LATER);
	bond_arp_send_all(bond, new_slave);
	new_slave->last_link_up = jiffies;
	rcu_assign_pointer(bond->current_arp_slave, new_slave);

check_state:
	bond_for_each_slave_rcu(bond, slave, iter) {
		if (slave->should_notify) {
			should_notify_rtnl = BOND_SLAVE_NOTIFY_NOW;
			break;
		}
	}
	return should_notify_rtnl;
}

static void bond_activebackup_arp_mon(struct work_struct *work)
{
	struct bonding *bond = container_of(work, struct bonding,
					    arp_work.work);
	bool should_notify_peers = false;
	bool should_notify_rtnl = false;
	int delta_in_ticks;

	delta_in_ticks = msecs_to_jiffies(bond->params.arp_interval);

	if (!bond_has_slaves(bond))
		goto re_arm;

	rcu_read_lock();

	should_notify_peers = bond_should_notify_peers(bond);

	if (bond_ab_arp_inspect(bond)) {
		rcu_read_unlock();

		/* Race avoidance with bond_close flush of workqueue */
		if (!rtnl_trylock()) {
			delta_in_ticks = 1;
			should_notify_peers = false;
			goto re_arm;
		}

		bond_ab_arp_commit(bond);

		rtnl_unlock();
		rcu_read_lock();
	}

	should_notify_rtnl = bond_ab_arp_probe(bond);
	rcu_read_unlock();

re_arm:
	if (bond->params.arp_interval)
		queue_delayed_work(bond->wq, &bond->arp_work, delta_in_ticks);

	if (should_notify_peers || should_notify_rtnl) {
		if (!rtnl_trylock())
			return;

		if (should_notify_peers)
			call_netdevice_notifiers(NETDEV_NOTIFY_PEERS,
						 bond->dev);
		if (should_notify_rtnl)
			bond_slave_state_notify(bond);

		rtnl_unlock();
	}
}

/*-------------------------- netdev event handling --------------------------*/

/* Change device name */
static int bond_event_changename(struct bonding *bond)
{
	bond_remove_proc_entry(bond);
	bond_create_proc_entry(bond);

	bond_debug_reregister(bond);

	return NOTIFY_DONE;
}

static int bond_master_netdev_event(unsigned long event,
				    struct net_device *bond_dev)
{
	struct bonding *event_bond = netdev_priv(bond_dev);

	switch (event) {
	case NETDEV_CHANGENAME:
		return bond_event_changename(event_bond);
	case NETDEV_UNREGISTER:
		bond_remove_proc_entry(event_bond);
		break;
	case NETDEV_REGISTER:
		bond_create_proc_entry(event_bond);
		break;
	case NETDEV_NOTIFY_PEERS:
		if (event_bond->send_peer_notif)
			event_bond->send_peer_notif--;
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static int bond_slave_netdev_event(unsigned long event,
				   struct net_device *slave_dev)
{
	struct slave *slave = bond_slave_get_rtnl(slave_dev), *primary;
	struct bonding *bond;
	struct net_device *bond_dev;
	u32 old_speed;
	u8 old_duplex;

	/* A netdev event can be generated while enslaving a device
	 * before netdev_rx_handler_register is called in which case
	 * slave will be NULL
	 */
	if (!slave)
		return NOTIFY_DONE;
	bond_dev = slave->bond->dev;
	bond = slave->bond;
	primary = rtnl_dereference(bond->primary_slave);

	switch (event) {
	case NETDEV_UNREGISTER:
		if (bond_dev->type != ARPHRD_ETHER)
			bond_release_and_destroy(bond_dev, slave_dev);
		else
			bond_release(bond_dev, slave_dev);
		break;
	case NETDEV_UP:
	case NETDEV_CHANGE:
		old_speed = slave->speed;
		old_duplex = slave->duplex;

		bond_update_speed_duplex(slave);

		if (BOND_MODE(bond) == BOND_MODE_8023AD) {
			if (old_speed != slave->speed)
				bond_3ad_adapter_speed_changed(slave);
			if (old_duplex != slave->duplex)
				bond_3ad_adapter_duplex_changed(slave);
		}
		/* Fallthrough */
	case NETDEV_DOWN:
		/* Refresh slave-array if applicable!
		 * If the setup does not use miimon or arpmon (mode-specific!),
		 * then these events will not cause the slave-array to be
		 * refreshed. This will cause xmit to use a slave that is not
		 * usable. Avoid such situation by refeshing the array at these
		 * events. If these (miimon/arpmon) parameters are configured
		 * then array gets refreshed twice and that should be fine!
		 */
		if (bond_mode_uses_xmit_hash(bond))
			bond_update_slave_arr(bond, NULL);
		break;
	case NETDEV_CHANGEMTU:
		/* TODO: Should slaves be allowed to
		 * independently alter their MTU?  For
		 * an active-backup bond, slaves need
		 * not be the same type of device, so
		 * MTUs may vary.  For other modes,
		 * slaves arguably should have the
		 * same MTUs. To do this, we'd need to
		 * take over the slave's change_mtu
		 * function for the duration of their
		 * servitude.
		 */
		break;
	case NETDEV_CHANGENAME:
		/* we don't care if we don't have primary set */
		if (!bond_uses_primary(bond) ||
		    !bond->params.primary[0])
			break;

		if (slave == primary) {
			/* slave's name changed - he's no longer primary */
			RCU_INIT_POINTER(bond->primary_slave, NULL);
		} else if (!strcmp(slave_dev->name, bond->params.primary)) {
			/* we have a new primary slave */
			rcu_assign_pointer(bond->primary_slave, slave);
		} else { /* we didn't change primary - exit */
			break;
		}

		netdev_info(bond->dev, "Primary slave changed to %s, reselecting active slave\n",
			    primary ? slave_dev->name : "none");

		block_netpoll_tx();
		bond_select_active_slave(bond);
		unblock_netpoll_tx();
		break;
	case NETDEV_FEAT_CHANGE:
		bond_compute_features(bond);
		break;
	case NETDEV_RESEND_IGMP:
		/* Propagate to master device */
		call_netdevice_notifiers(event, slave->bond->dev);
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

/* bond_netdev_event: handle netdev notifier chain events.
 *
 * This function receives events for the netdev chain.  The caller (an
 * ioctl handler calling blocking_notifier_call_chain) holds the necessary
 * locks for us to safely manipulate the slave devices (RTNL lock,
 * dev_probe_lock).
 */
static int bond_netdev_event(struct notifier_block *this,
			     unsigned long event, void *ptr)
{
	struct net_device *event_dev = netdev_notifier_info_to_dev(ptr);

	netdev_dbg(event_dev, "event: %lx\n", event);

	if (!(event_dev->priv_flags & IFF_BONDING))
		return NOTIFY_DONE;

	if (event_dev->flags & IFF_MASTER) {
		netdev_dbg(event_dev, "IFF_MASTER\n");
		return bond_master_netdev_event(event, event_dev);
	}

	if (event_dev->flags & IFF_SLAVE) {
		netdev_dbg(event_dev, "IFF_SLAVE\n");
		return bond_slave_netdev_event(event, event_dev);
	}

	return NOTIFY_DONE;
}

static struct notifier_block bond_netdev_notifier = {
	.notifier_call = bond_netdev_event,
};

#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING)

/* bond_br_stp_event: handle bridge stp notifier chain events.
 *
 */
#include <linux/if_bridge.h>
static int bond_br_stp_event(struct notifier_block *unused,
							 unsigned long event, void *ptr)
{
	struct stpPortInfo *pInfo = (struct stpPortInfo *)ptr;
	struct net_device *bond_dev;

	bond_dev = dev_get_by_name(&init_net, pInfo->portName);
	if (!bond_dev)
		return NOTIFY_DONE;

	if ((bond_dev->priv_flags & IFF_BONDING) &&
		(bond_dev->flags & IFF_MASTER)) {
		struct bonding *bond = netdev_priv(bond_dev);
		struct slave *slave;
		struct list_head *iter;

		bond_for_each_slave(bond, slave, iter) {
			/* Propogate the STP event to slaves */
			call_br_stp_notifiers(event, slave->dev, pInfo);
		}

	}

	dev_put(bond_dev);
	return NOTIFY_DONE;
}

static struct notifier_block bond_br_stp_notifier = {
	.notifier_call = bond_br_stp_event,
};

#endif
/*---------------------------- Hashing Policies -----------------------------*/

/* L2 hash helper */
static inline u32 bond_eth_hash(struct sk_buff *skb)
{
	struct ethhdr *ep, hdr_tmp;

	ep = skb_header_pointer(skb, 0, sizeof(hdr_tmp), &hdr_tmp);
	if (ep)
		return ep->h_dest[5] ^ ep->h_source[5] ^ ep->h_proto;
	return 0;
}

/* Extract the appropriate headers based on bond's xmit policy */
static bool bond_flow_dissect(struct bonding *bond, struct sk_buff *skb,
			      struct flow_keys *fk)
{
	const struct ipv6hdr *iph6;
	const struct iphdr *iph;
	int noff, proto = -1;

	if (bond->params.xmit_policy > BOND_XMIT_POLICY_LAYER23)
		return skb_flow_dissect(skb, fk);

	fk->ports = 0;
	noff = skb_network_offset(skb);
	if (skb->protocol == htons(ETH_P_IP)) {
		if (unlikely(!pskb_may_pull(skb, noff + sizeof(*iph))))
			return false;
		iph = ip_hdr(skb);
		fk->src = iph->saddr;
		fk->dst = iph->daddr;
		noff += iph->ihl << 2;
		if (!ip_is_fragment(iph))
			proto = iph->protocol;
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		if (unlikely(!pskb_may_pull(skb, noff + sizeof(*iph6))))
			return false;
		iph6 = ipv6_hdr(skb);
		fk->src = (__force __be32)ipv6_addr_hash(&iph6->saddr);
		fk->dst = (__force __be32)ipv6_addr_hash(&iph6->daddr);
		noff += sizeof(*iph6);
		proto = iph6->nexthdr;
	} else {
		return false;
	}
	if (bond->params.xmit_policy == BOND_XMIT_POLICY_LAYER34 && proto >= 0)
		fk->ports = skb_flow_get_ports(skb, noff, proto);

	return true;
}

/**
 * bond_xmit_hash - generate a hash value based on the xmit policy
 * @bond: bonding device
 * @skb: buffer to use for headers
 *
 * This function will extract the necessary headers from the skb buffer and use
 * them to generate a hash based on the xmit_policy set in the bonding device
 */
u32 bond_xmit_hash(struct bonding *bond, struct sk_buff *skb)
{
	struct flow_keys flow;
	u32 hash;

	if (bond->params.xmit_policy == BOND_XMIT_POLICY_LAYER2 ||
	    !bond_flow_dissect(bond, skb, &flow))
		return bond_eth_hash(skb);

	if (bond->params.xmit_policy == BOND_XMIT_POLICY_LAYER23 ||
	    bond->params.xmit_policy == BOND_XMIT_POLICY_ENCAP23)
		hash = bond_eth_hash(skb);
	else
		hash = (__force u32)flow.ports;
	hash ^= (__force u32)flow.dst ^ (__force u32)flow.src;
	hash ^= (hash >> 16);
	hash ^= (hash >> 8);

	return hash >> 1;
}

/*-------------------------- Device entry points ----------------------------*/

static void bond_work_init_all(struct bonding *bond)
{
	INIT_DELAYED_WORK(&bond->mcast_work,
			  bond_resend_igmp_join_requests_delayed);
	INIT_DELAYED_WORK(&bond->alb_work, bond_alb_monitor);
	INIT_DELAYED_WORK(&bond->mii_work, bond_mii_monitor);
	if (BOND_MODE(bond) == BOND_MODE_ACTIVEBACKUP)
		INIT_DELAYED_WORK(&bond->arp_work, bond_activebackup_arp_mon);
	else
		INIT_DELAYED_WORK(&bond->arp_work, bond_loadbalance_arp_mon);
	INIT_DELAYED_WORK(&bond->ad_work, bond_3ad_state_machine_handler);
	INIT_DELAYED_WORK(&bond->slave_arr_work, bond_slave_arr_handler);
}

static void bond_work_cancel_all(struct bonding *bond)
{
	cancel_delayed_work_sync(&bond->mii_work);
	cancel_delayed_work_sync(&bond->arp_work);
	cancel_delayed_work_sync(&bond->alb_work);
	cancel_delayed_work_sync(&bond->ad_work);
	cancel_delayed_work_sync(&bond->mcast_work);
	cancel_delayed_work_sync(&bond->slave_arr_work);
}

static int bond_open(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct list_head *iter;
	struct slave *slave;

	/* reset slave->backup and slave->inactive */
	if (bond_has_slaves(bond)) {
		bond_for_each_slave(bond, slave, iter) {
			if (bond_uses_primary(bond) &&
			    slave != rcu_access_pointer(bond->curr_active_slave)) {
				bond_set_slave_inactive_flags(slave,
							      BOND_SLAVE_NOTIFY_NOW);
			} else if (BOND_MODE(bond) != BOND_MODE_8023AD) {
				bond_set_slave_active_flags(slave,
							    BOND_SLAVE_NOTIFY_NOW);
			}
		}
	}

	bond_work_init_all(bond);

	if (bond_is_lb(bond)) {
		/* bond_alb_initialize must be called before the timer
		 * is started.
		 */
		if (bond_alb_initialize(bond, (BOND_MODE(bond) == BOND_MODE_ALB)))
			return -ENOMEM;
		if (bond->params.tlb_dynamic_lb)
			queue_delayed_work(bond->wq, &bond->alb_work, 0);
	}

	if (bond->params.miimon)  /* link check interval, in milliseconds. */
		queue_delayed_work(bond->wq, &bond->mii_work, 0);

	if (bond->params.arp_interval) {  /* arp interval, in milliseconds. */
		queue_delayed_work(bond->wq, &bond->arp_work, 0);
		bond->recv_probe = bond_arp_rcv;
	}

	if (BOND_MODE(bond) == BOND_MODE_8023AD) {
		queue_delayed_work(bond->wq, &bond->ad_work, 0);
		/* register to receive LACPDUs */
		bond->recv_probe = bond_3ad_lacpdu_recv;
		bond_3ad_initiate_agg_selection(bond, 1);
	}

	if (bond_mode_uses_xmit_hash(bond))
		bond_update_slave_arr(bond, NULL);

	return 0;
}

static int bond_close(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);

	bond_work_cancel_all(bond);
	bond->send_peer_notif = 0;
	if (bond_is_lb(bond))
		bond_alb_deinitialize(bond);
	bond->recv_probe = NULL;

	return 0;
}

/* fold stats, assuming all rtnl_link_stats64 fields are u64, but
 * that some drivers can provide 32bit values only.
 */
static void bond_fold_stats(struct rtnl_link_stats64 *_res,
			    const struct rtnl_link_stats64 *_new,
			    const struct rtnl_link_stats64 *_old)
{
	const u64 *new = (const u64 *)_new;
	const u64 *old = (const u64 *)_old;
	u64 *res = (u64 *)_res;
	int i;

	for (i = 0; i < sizeof(*_res) / sizeof(u64); i++) {
		u64 nv = new[i];
		u64 ov = old[i];
		s64 delta = nv - ov;

		/* detects if this particular field is 32bit only */
		if (((nv | ov) >> 32) == 0)
			delta = (s64)(s32)((u32)nv - (u32)ov);

		/* filter anomalies, some drivers reset their stats
		 * at down/up events.
		 */
		if (delta > 0)
			res[i] += delta;
	}
}

static struct rtnl_link_stats64 *bond_get_stats(struct net_device *bond_dev,
						struct rtnl_link_stats64 *stats)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct rtnl_link_stats64 temp;
	struct list_head *iter;
	struct slave *slave;

	spin_lock(&bond->stats_lock);
	memcpy(stats, &bond->bond_stats, sizeof(*stats));

	rcu_read_lock();
	bond_for_each_slave_rcu(bond, slave, iter) {
		const struct rtnl_link_stats64 *new =
			dev_get_stats(slave->dev, &temp);

		bond_fold_stats(stats, new, &slave->slave_stats);

		/* save off the slave stats for the next run */
		memcpy(&slave->slave_stats, new, sizeof(*new));
	}
	rcu_read_unlock();

	memcpy(&bond->bond_stats, stats, sizeof(*stats));
	spin_unlock(&bond->stats_lock);

	return stats;
}

static int bond_do_ioctl(struct net_device *bond_dev, struct ifreq *ifr, int cmd)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct net_device *slave_dev = NULL;
	struct ifbond k_binfo;
	struct ifbond __user *u_binfo = NULL;
	struct ifslave k_sinfo;
	struct ifslave __user *u_sinfo = NULL;
	struct mii_ioctl_data *mii = NULL;
	struct bond_opt_value newval;
	struct net *net;
	int res = 0;

	netdev_dbg(bond_dev, "bond_ioctl: cmd=%d\n", cmd);

	switch (cmd) {
	case SIOCGMIIPHY:
		mii = if_mii(ifr);
		if (!mii)
			return -EINVAL;

		mii->phy_id = 0;
		/* Fall Through */
	case SIOCGMIIREG:
		/* We do this again just in case we were called by SIOCGMIIREG
		 * instead of SIOCGMIIPHY.
		 */
		mii = if_mii(ifr);
		if (!mii)
			return -EINVAL;

		if (mii->reg_num == 1) {
			mii->val_out = 0;
			if (netif_carrier_ok(bond->dev))
				mii->val_out = BMSR_LSTATUS;
		}

		return 0;
	case BOND_INFO_QUERY_OLD:
	case SIOCBONDINFOQUERY:
		u_binfo = (struct ifbond __user *)ifr->ifr_data;

		if (copy_from_user(&k_binfo, u_binfo, sizeof(ifbond)))
			return -EFAULT;

		res = bond_info_query(bond_dev, &k_binfo);
		if (res == 0 &&
		    copy_to_user(u_binfo, &k_binfo, sizeof(ifbond)))
			return -EFAULT;

		return res;
	case BOND_SLAVE_INFO_QUERY_OLD:
	case SIOCBONDSLAVEINFOQUERY:
		u_sinfo = (struct ifslave __user *)ifr->ifr_data;

		if (copy_from_user(&k_sinfo, u_sinfo, sizeof(ifslave)))
			return -EFAULT;

		res = bond_slave_info_query(bond_dev, &k_sinfo);
		if (res == 0 &&
		    copy_to_user(u_sinfo, &k_sinfo, sizeof(ifslave)))
			return -EFAULT;

		return res;
	default:
		break;
	}

	net = dev_net(bond_dev);

	if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
		return -EPERM;

	slave_dev = __dev_get_by_name(net, ifr->ifr_slave);

	netdev_dbg(bond_dev, "slave_dev=%p:\n", slave_dev);

	if (!slave_dev)
		return -ENODEV;

	netdev_dbg(bond_dev, "slave_dev->name=%s:\n", slave_dev->name);
	switch (cmd) {
	case BOND_ENSLAVE_OLD:
	case SIOCBONDENSLAVE:
		res = bond_enslave(bond_dev, slave_dev);
		break;
	case BOND_RELEASE_OLD:
	case SIOCBONDRELEASE:
		res = bond_release(bond_dev, slave_dev);
		break;
	case BOND_SETHWADDR_OLD:
	case SIOCBONDSETHWADDR:
		bond_set_dev_addr(bond_dev, slave_dev);
		res = 0;
		break;
	case BOND_CHANGE_ACTIVE_OLD:
	case SIOCBONDCHANGEACTIVE:
		bond_opt_initstr(&newval, slave_dev->name);
		res = __bond_opt_set(bond, BOND_OPT_ACTIVE_SLAVE, &newval);
		break;
	default:
		res = -EOPNOTSUPP;
	}

	return res;
}

static void bond_change_rx_flags(struct net_device *bond_dev, int change)
{
	struct bonding *bond = netdev_priv(bond_dev);

	if (change & IFF_PROMISC)
		bond_set_promiscuity(bond,
				     bond_dev->flags & IFF_PROMISC ? 1 : -1);

	if (change & IFF_ALLMULTI)
		bond_set_allmulti(bond,
				  bond_dev->flags & IFF_ALLMULTI ? 1 : -1);
}

static void bond_set_rx_mode(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct list_head *iter;
	struct slave *slave;

	rcu_read_lock();
	if (bond_uses_primary(bond)) {
		slave = rcu_dereference(bond->curr_active_slave);
		if (slave) {
			dev_uc_sync(slave->dev, bond_dev);
			dev_mc_sync(slave->dev, bond_dev);
		}
	} else {
		bond_for_each_slave_rcu(bond, slave, iter) {
			dev_uc_sync_multiple(slave->dev, bond_dev);
			dev_mc_sync_multiple(slave->dev, bond_dev);
		}
	}
	rcu_read_unlock();
}

static int bond_neigh_init(struct neighbour *n)
{
	struct bonding *bond = netdev_priv(n->dev);
	const struct net_device_ops *slave_ops;
	struct neigh_parms parms;
	struct slave *slave;
	int ret;

	slave = bond_first_slave(bond);
	if (!slave)
		return 0;
	slave_ops = slave->dev->netdev_ops;
	if (!slave_ops->ndo_neigh_setup)
		return 0;

	parms.neigh_setup = NULL;
	parms.neigh_cleanup = NULL;
	ret = slave_ops->ndo_neigh_setup(slave->dev, &parms);
	if (ret)
		return ret;

	/* Assign slave's neigh_cleanup to neighbour in case cleanup is called
	 * after the last slave has been detached.  Assumes that all slaves
	 * utilize the same neigh_cleanup (true at this writing as only user
	 * is ipoib).
	 */
	n->parms->neigh_cleanup = parms.neigh_cleanup;

	if (!parms.neigh_setup)
		return 0;

	return parms.neigh_setup(n);
}

/* The bonding ndo_neigh_setup is called at init time beofre any
 * slave exists. So we must declare proxy setup function which will
 * be used at run time to resolve the actual slave neigh param setup.
 *
 * It's also called by master devices (such as vlans) to setup their
 * underlying devices. In that case - do nothing, we're already set up from
 * our init.
 */
static int bond_neigh_setup(struct net_device *dev,
			    struct neigh_parms *parms)
{
	/* modify only our neigh_parms */
	if (parms->dev == dev)
		parms->neigh_setup = bond_neigh_init;

	return 0;
}

/* Change the MTU of all of a master's slaves to match the master */
static int bond_change_mtu(struct net_device *bond_dev, int new_mtu)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave, *rollback_slave;
	struct list_head *iter;
	int res = 0;

	netdev_dbg(bond_dev, "bond=%p, new_mtu=%d\n", bond, new_mtu);

	bond_for_each_slave(bond, slave, iter) {
		netdev_dbg(bond_dev, "s %p c_m %p\n",
			   slave, slave->dev->netdev_ops->ndo_change_mtu);

		res = dev_set_mtu(slave->dev, new_mtu);

		if (res) {
			/* If we failed to set the slave's mtu to the new value
			 * we must abort the operation even in ACTIVE_BACKUP
			 * mode, because if we allow the backup slaves to have
			 * different mtu values than the active slave we'll
			 * need to change their mtu when doing a failover. That
			 * means changing their mtu from timer context, which
			 * is probably not a good idea.
			 */
			netdev_dbg(bond_dev, "err %d %s\n", res,
				   slave->dev->name);
			goto unwind;
		}
	}

	bond_dev->mtu = new_mtu;

	return 0;

unwind:
	/* unwind from head to the slave that failed */
	bond_for_each_slave(bond, rollback_slave, iter) {
		int tmp_res;

		if (rollback_slave == slave)
			break;

		tmp_res = dev_set_mtu(rollback_slave->dev, bond_dev->mtu);
		if (tmp_res) {
			netdev_dbg(bond_dev, "unwind err %d dev %s\n",
				   tmp_res, rollback_slave->dev->name);
		}
	}

	return res;
}

/* Change HW address
 *
 * Note that many devices must be down to change the HW address, and
 * downing the master releases all slaves.  We can make bonds full of
 * bonding devices to test this, however.
 */
static int bond_set_mac_address(struct net_device *bond_dev, void *addr)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave, *rollback_slave;
	struct sockaddr *sa = addr, tmp_sa;
	struct list_head *iter;
	int res = 0;

	if (BOND_MODE(bond) == BOND_MODE_ALB)
		return bond_alb_set_mac_address(bond_dev, addr);


	netdev_dbg(bond_dev, "bond=%p\n", bond);

	/* If fail_over_mac is enabled, do nothing and return success.
	 * Returning an error causes ifenslave to fail.
	 */
	if (bond->params.fail_over_mac &&
	    BOND_MODE(bond) == BOND_MODE_ACTIVEBACKUP)
		return 0;

	if (!is_valid_ether_addr(sa->sa_data))
		return -EADDRNOTAVAIL;

	bond_for_each_slave(bond, slave, iter) {
		netdev_dbg(bond_dev, "slave %p %s\n", slave, slave->dev->name);
		res = dev_set_mac_address(slave->dev, addr);
		if (res) {
			/* TODO: consider downing the slave
			 * and retry ?
			 * User should expect communications
			 * breakage anyway until ARP finish
			 * updating, so...
			 */
			netdev_dbg(bond_dev, "err %d %s\n", res, slave->dev->name);
			goto unwind;
		}
	}

	/* success */
	memcpy(bond_dev->dev_addr, sa->sa_data, bond_dev->addr_len);
	return 0;

unwind:
	memcpy(tmp_sa.sa_data, bond_dev->dev_addr, bond_dev->addr_len);
	tmp_sa.sa_family = bond_dev->type;

	/* unwind from head to the slave that failed */
	bond_for_each_slave(bond, rollback_slave, iter) {
		int tmp_res;

		if (rollback_slave == slave)
			break;

		tmp_res = dev_set_mac_address(rollback_slave->dev, &tmp_sa);
		if (tmp_res) {
			netdev_dbg(bond_dev, "unwind err %d dev %s\n",
				   tmp_res, rollback_slave->dev->name);
		}
	}

	return res;
}

/**
 * bond_xmit_slave_id - transmit skb through slave with slave_id
 * @bond: bonding device that is transmitting
 * @skb: buffer to transmit
 * @slave_id: slave id up to slave_cnt-1 through which to transmit
 *
 * This function tries to transmit through slave with slave_id but in case
 * it fails, it tries to find the first available slave for transmission.
 * The skb is consumed in all cases, thus the function is void.
 */
static void bond_xmit_slave_id(struct bonding *bond, struct sk_buff *skb, int slave_id)
{
	struct list_head *iter;
	struct slave *slave;
	int i = slave_id;

	/* Here we start from the slave with slave_id */
	bond_for_each_slave_rcu(bond, slave, iter) {
		if (--i < 0) {
			if (bond_slave_can_tx(slave)) {
				bond_dev_queue_xmit(bond, skb, slave->dev);
				return;
			}
		}
	}

	/* Here we start from the first slave up to slave_id */
	i = slave_id;
	bond_for_each_slave_rcu(bond, slave, iter) {
		if (--i < 0)
			break;
		if (bond_slave_can_tx(slave)) {
			bond_dev_queue_xmit(bond, skb, slave->dev);
			return;
		}
	}
	/* no slave that can tx has been found */
	bond_tx_drop(bond->dev, skb);
}

/**
 * bond_rr_gen_slave_id - generate slave id based on packets_per_slave
 * @bond: bonding device to use
 *
 * Based on the value of the bonding device's packets_per_slave parameter
 * this function generates a slave id, which is usually used as the next
 * slave to transmit through.
 */
static u32 bond_rr_gen_slave_id(struct bonding *bond)
{
	u32 slave_id;
	struct reciprocal_value reciprocal_packets_per_slave;
	int packets_per_slave = bond->params.packets_per_slave;

	switch (packets_per_slave) {
	case 0:
		slave_id = prandom_u32();
		break;
	case 1:
		slave_id = bond->rr_tx_counter;
		break;
	default:
		reciprocal_packets_per_slave =
			bond->params.reciprocal_packets_per_slave;
		slave_id = reciprocal_divide(bond->rr_tx_counter,
					     reciprocal_packets_per_slave);
		break;
	}
	bond->rr_tx_counter++;

	return slave_id;
}

static int bond_xmit_roundrobin(struct sk_buff *skb, struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct iphdr *iph = ip_hdr(skb);
	struct slave *slave;
	u32 slave_id;

	/* Start with the curr_active_slave that joined the bond as the
	 * default for sending IGMP traffic.  For failover purposes one
	 * needs to maintain some consistency for the interface that will
	 * send the join/membership reports.  The curr_active_slave found
	 * will send all of this type of traffic.
	 */
	if (iph->protocol == IPPROTO_IGMP && skb->protocol == htons(ETH_P_IP)) {
		slave = rcu_dereference(bond->curr_active_slave);
		if (slave)
			bond_dev_queue_xmit(bond, skb, slave->dev);
		else
			bond_xmit_slave_id(bond, skb, 0);
	} else {
		int slave_cnt = ACCESS_ONCE(bond->slave_cnt);

		if (likely(slave_cnt)) {
			slave_id = bond_rr_gen_slave_id(bond);
			bond_xmit_slave_id(bond, skb, slave_id % slave_cnt);
		} else {
			bond_tx_drop(bond_dev, skb);
		}
	}

	return NETDEV_TX_OK;
}

/* In active-backup mode, we know that bond->curr_active_slave is always valid if
 * the bond has a usable interface.
 */
static int bond_xmit_activebackup(struct sk_buff *skb, struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave;

	slave = rcu_dereference(bond->curr_active_slave);
	if (slave)
		bond_dev_queue_xmit(bond, skb, slave->dev);
	else
		bond_tx_drop(bond_dev, skb);

	return NETDEV_TX_OK;
}

/* Use this to update slave_array when (a) it's not appropriate to update
 * slave_array right away (note that update_slave_array() may sleep)
 * and / or (b) RTNL is not held.
 */
void bond_slave_arr_work_rearm(struct bonding *bond, unsigned long delay)
{
	queue_delayed_work(bond->wq, &bond->slave_arr_work, delay);
}

/* Slave array work handler. Holds only RTNL */
static void bond_slave_arr_handler(struct work_struct *work)
{
	struct bonding *bond = container_of(work, struct bonding,
					    slave_arr_work.work);
	int ret;

	if (!rtnl_trylock())
		goto err;

	ret = bond_update_slave_arr(bond, NULL);
	rtnl_unlock();
	if (ret) {
		pr_warn_ratelimited("Failed to update slave array from WT\n");
		goto err;
	}
	return;

err:
	bond_slave_arr_work_rearm(bond, 1);
}

/* Build the usable slaves array in control path for modes that use xmit-hash
 * to determine the slave interface -
 * (a) BOND_MODE_8023AD
 * (b) BOND_MODE_XOR
 * (c) BOND_MODE_TLB && tlb_dynamic_lb == 0
 *
 * The caller is expected to hold RTNL only and NO other lock!
 */
int bond_update_slave_arr(struct bonding *bond, struct slave *skipslave)
{
	struct slave *slave;
	struct list_head *iter;
	struct bond_up_slave *new_arr, *old_arr;
	int slaves_in_agg;
	int agg_id = 0;
	int ret = 0;

#ifdef CONFIG_LOCKDEP
	WARN_ON(lockdep_is_held(&bond->mode_lock));
#endif

	new_arr = kzalloc(offsetof(struct bond_up_slave, arr[bond->slave_cnt]),
			  GFP_KERNEL);
	if (!new_arr) {
		ret = -ENOMEM;
		pr_err("Failed to build slave-array.\n");
		goto out;
	}
	if (BOND_MODE(bond) == BOND_MODE_8023AD) {
		struct ad_info ad_info;

		if (bond_3ad_get_active_agg_info(bond, &ad_info)) {
			pr_debug("bond_3ad_get_active_agg_info failed\n");
			kfree_rcu(new_arr, rcu);
			/* No active aggragator means it's not safe to use
			 * the previous array.
			 */
			old_arr = rtnl_dereference(bond->slave_arr);
			if (old_arr) {
				RCU_INIT_POINTER(bond->slave_arr, NULL);
				kfree_rcu(old_arr, rcu);
			}
			goto out;
		}
		slaves_in_agg = ad_info.ports;
		agg_id = ad_info.aggregator_id;
	}
	bond_for_each_slave(bond, slave, iter) {
		if (BOND_MODE(bond) == BOND_MODE_8023AD) {
			struct aggregator *agg;

			agg = SLAVE_AD_INFO(slave)->port.aggregator;
			if (!agg || agg->aggregator_identifier != agg_id)
				continue;
		}
		if (!bond_slave_can_tx(slave))
			continue;
		if (skipslave == slave)
			continue;
		new_arr->arr[new_arr->count++] = slave;
	}

	old_arr = rtnl_dereference(bond->slave_arr);
	rcu_assign_pointer(bond->slave_arr, new_arr);
	if (old_arr)
		kfree_rcu(old_arr, rcu);
out:
	if (ret != 0 && skipslave) {
		int idx;

		/* Rare situation where caller has asked to skip a specific
		 * slave but allocation failed (most likely!). BTW this is
		 * only possible when the call is initiated from
		 * __bond_release_one(). In this situation; overwrite the
		 * skipslave entry in the array with the last entry from the
		 * array to avoid a situation where the xmit path may choose
		 * this to-be-skipped slave to send a packet out.
		 */
		old_arr = rtnl_dereference(bond->slave_arr);
		for (idx = 0; idx < old_arr->count; idx++) {
			if (skipslave == old_arr->arr[idx]) {
				old_arr->arr[idx] =
				    old_arr->arr[old_arr->count-1];
				old_arr->count--;
				break;
			}
		}
	}
	return ret;
}

/* Use this Xmit function for 3AD as well as XOR modes. The current
 * usable slave array is formed in the control path. The xmit function
 * just calculates hash and sends the packet out.
 */
static int bond_3ad_xor_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct bonding *bond = netdev_priv(dev);
	struct slave *slave;
	struct bond_up_slave *slaves;
	unsigned int count;

	slaves = rcu_dereference(bond->slave_arr);
	count = slaves ? ACCESS_ONCE(slaves->count) : 0;
	if (likely(count)) {
		slave = slaves->arr[bond_xmit_hash(bond, skb) % count];
		bond_dev_queue_xmit(bond, skb, slave->dev);
	} else {
		bond_tx_drop(dev, skb);
	}

	return NETDEV_TX_OK;
}

/* in broadcast mode, we send everything to all usable interfaces. */
static int bond_xmit_broadcast(struct sk_buff *skb, struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave = NULL;
	struct list_head *iter;

	bond_for_each_slave_rcu(bond, slave, iter) {
		if (bond_is_last_slave(bond, slave))
			break;
		if (bond_slave_is_up(slave) && slave->link == BOND_LINK_UP) {
			struct sk_buff *skb2 = skb_clone(skb, GFP_ATOMIC);

			if (!skb2) {
				net_err_ratelimited("%s: Error: %s: skb_clone() failed\n",
						    bond_dev->name, __func__);
				continue;
			}
			bond_dev_queue_xmit(bond, skb2, slave->dev);
		}
	}
	if (slave && bond_slave_is_up(slave) && slave->link == BOND_LINK_UP)
		bond_dev_queue_xmit(bond, skb, slave->dev);
	else
		bond_tx_drop(bond_dev, skb);

	return NETDEV_TX_OK;
}

/*------------------------- Device initialization ---------------------------*/

/* Lookup the slave that corresponds to a qid */
static inline int bond_slave_override(struct bonding *bond,
				      struct sk_buff *skb)
{
	struct slave *slave = NULL;
	struct list_head *iter;

	if (!skb->queue_mapping)
		return 1;

	/* Find out if any slaves have the same mapping as this skb. */
	bond_for_each_slave_rcu(bond, slave, iter) {
		if (slave->queue_id == skb->queue_mapping) {
			if (bond_slave_is_up(slave) &&
			    slave->link == BOND_LINK_UP) {
				bond_dev_queue_xmit(bond, skb, slave->dev);
				return 0;
			}
			/* If the slave isn't UP, use default transmit policy. */
			break;
		}
	}

	return 1;
}


static u16 bond_select_queue(struct net_device *dev, struct sk_buff *skb,
			     void *accel_priv, select_queue_fallback_t fallback)
{
	/* This helper function exists to help dev_pick_tx get the correct
	 * destination queue.  Using a helper function skips a call to
	 * skb_tx_hash and will put the skbs in the queue we expect on their
	 * way down to the bonding driver.
	 */
	u16 txq = skb_rx_queue_recorded(skb) ? skb_get_rx_queue(skb) : 0;

	/* Save the original txq to restore before passing to the driver */
	qdisc_skb_cb(skb)->slave_dev_queue_mapping = skb->queue_mapping;

	if (unlikely(txq >= dev->real_num_tx_queues)) {
		do {
			txq -= dev->real_num_tx_queues;
		} while (txq >= dev->real_num_tx_queues);
	}
	return txq;
}

static netdev_tx_t __bond_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct bonding *bond = netdev_priv(dev);

	if (bond_should_override_tx_queue(bond) &&
	    !bond_slave_override(bond, skb))
		return NETDEV_TX_OK;

#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING)
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)

/* Only XOR or 802.3AD mode can be accelerated by flow-cache -- these are flow-based moded */
#define BOND_SUPPORTED_ACCL_MODE(mode) (((mode) == BOND_MODE_XOR) || ((mode) == BOND_MODE_8023AD))

/* IS_SKB check not needed unless we support flow-cache acceleration to bond device in other modes;
   No need to check for non-NULL blog as well -- blog_skip will take care */

    if (!BOND_SUPPORTED_ACCL_MODE(bond->params.mode))
		blog_skip(skb, blog_skip_reason_bond);
    else if (blog_ptr(skb)) 
    {
        /* Clarify : bond device shouldn't modify the packet;
           also it does not hold its own stats rather gets the cummulative stats from its slave devices [ see:bond_get_stats() ];
           blog->put_stats,clr_stats are also not needed for the same reason;
           Do we need to link bond devices ??                                                          ;
           One usage could be that when bond device goes down -- FC will be able to remove the flows */
        blog_lock();
        blog_link( IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_TX, skb->len );
        blog_unlock();
    }

#endif /* defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG) */
#endif /* defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) */

	switch (BOND_MODE(bond)) {
	case BOND_MODE_ROUNDROBIN:
		return bond_xmit_roundrobin(skb, dev);
	case BOND_MODE_ACTIVEBACKUP:
		return bond_xmit_activebackup(skb, dev);
	case BOND_MODE_8023AD:
	case BOND_MODE_XOR:
		return bond_3ad_xor_xmit(skb, dev);
	case BOND_MODE_BROADCAST:
		return bond_xmit_broadcast(skb, dev);
	case BOND_MODE_ALB:
		return bond_alb_xmit(skb, dev);
	case BOND_MODE_TLB:
		return bond_tlb_xmit(skb, dev);
	default:
		/* Should never happen, mode already checked */
		netdev_err(dev, "Unknown bonding mode %d\n", BOND_MODE(bond));
		WARN_ON_ONCE(1);
		bond_tx_drop(dev, skb);
		return NETDEV_TX_OK;
	}
}

static netdev_tx_t bond_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct bonding *bond = netdev_priv(dev);
	netdev_tx_t ret = NETDEV_TX_OK;

	/* If we risk deadlock from transmitting this in the
	 * netpoll path, tell netpoll to queue the frame for later tx
	 */
	if (unlikely(is_netpoll_tx_blocked(dev)))
		return NETDEV_TX_BUSY;

	rcu_read_lock();
	if (bond_has_slaves(bond))
		ret = __bond_start_xmit(skb, dev);
	else
		bond_tx_drop(dev, skb);
	rcu_read_unlock();

	return ret;
}

static int bond_ethtool_get_settings(struct net_device *bond_dev,
				     struct ethtool_cmd *ecmd)
{
	struct bonding *bond = netdev_priv(bond_dev);
	unsigned long speed = 0;
	struct list_head *iter;
	struct slave *slave;

	ecmd->duplex = DUPLEX_UNKNOWN;
	ecmd->port = PORT_OTHER;

	/* Since bond_slave_can_tx returns false for all inactive or down slaves, we
	 * do not need to check mode.  Though link speed might not represent
	 * the true receive or transmit bandwidth (not all modes are symmetric)
	 * this is an accurate maximum.
	 */
	bond_for_each_slave(bond, slave, iter) {
		if (bond_slave_can_tx(slave)) {
			if (slave->speed != SPEED_UNKNOWN)
				speed += slave->speed;
			if (ecmd->duplex == DUPLEX_UNKNOWN &&
			    slave->duplex != DUPLEX_UNKNOWN)
				ecmd->duplex = slave->duplex;
		}
	}
	ethtool_cmd_speed_set(ecmd, speed ? : SPEED_UNKNOWN);

	return 0;
}

static void bond_ethtool_get_drvinfo(struct net_device *bond_dev,
				     struct ethtool_drvinfo *drvinfo)
{
	strlcpy(drvinfo->driver, DRV_NAME, sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, DRV_VERSION, sizeof(drvinfo->version));
	snprintf(drvinfo->fw_version, sizeof(drvinfo->fw_version), "%d",
		 BOND_ABI_VERSION);
}

static const struct ethtool_ops bond_ethtool_ops = {
	.get_drvinfo		= bond_ethtool_get_drvinfo,
	.get_settings		= bond_ethtool_get_settings,
	.get_link		= ethtool_op_get_link,
};

static const struct net_device_ops bond_netdev_ops = {
	.ndo_init		= bond_init,
	.ndo_uninit		= bond_uninit,
	.ndo_open		= bond_open,
	.ndo_stop		= bond_close,
	.ndo_start_xmit		= bond_start_xmit,
	.ndo_select_queue	= bond_select_queue,
	.ndo_get_stats64	= bond_get_stats,
	.ndo_do_ioctl		= bond_do_ioctl,
	.ndo_change_rx_flags	= bond_change_rx_flags,
	.ndo_set_rx_mode	= bond_set_rx_mode,
	.ndo_change_mtu		= bond_change_mtu,
	.ndo_set_mac_address	= bond_set_mac_address,
	.ndo_neigh_setup	= bond_neigh_setup,
	.ndo_vlan_rx_add_vid	= bond_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= bond_vlan_rx_kill_vid,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_netpoll_setup	= bond_netpoll_setup,
	.ndo_netpoll_cleanup	= bond_netpoll_cleanup,
	.ndo_poll_controller	= bond_poll_controller,
#endif
	.ndo_add_slave		= bond_enslave,
	.ndo_del_slave		= bond_release,
	.ndo_fix_features	= bond_fix_features,
	.ndo_bridge_setlink	= ndo_dflt_netdev_switch_port_bridge_setlink,
	.ndo_bridge_dellink	= ndo_dflt_netdev_switch_port_bridge_dellink,
	.ndo_features_check	= passthru_features_check,
};

static const struct device_type bond_type = {
	.name = "bond",
};

static void bond_destructor(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	if (bond->wq)
		destroy_workqueue(bond->wq);
	free_netdev(bond_dev);
}

void bond_setup(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);

	spin_lock_init(&bond->mode_lock);
	spin_lock_init(&bond->stats_lock);
	bond->params = bonding_defaults;

	/* Initialize pointers */
	bond->dev = bond_dev;

	/* Initialize the device entry points */
	ether_setup(bond_dev);
	bond_dev->netdev_ops = &bond_netdev_ops;
	bond_dev->ethtool_ops = &bond_ethtool_ops;

	bond_dev->destructor = bond_destructor;

	SET_NETDEV_DEVTYPE(bond_dev, &bond_type);

	/* Initialize the device options */
	bond_dev->tx_queue_len = 0;
	bond_dev->flags |= IFF_MASTER|IFF_MULTICAST;
	bond_dev->priv_flags |= IFF_BONDING | IFF_UNICAST_FLT;
	bond_dev->priv_flags &= ~(IFF_XMIT_DST_RELEASE | IFF_TX_SKB_SHARING);

	/* don't acquire bond device's netif_tx_lock when transmitting */
	bond_dev->features |= NETIF_F_LLTX;

	/* By default, we declare the bond to be fully
	 * VLAN hardware accelerated capable. Special
	 * care is taken in the various xmit functions
	 * when there are slaves that are not hw accel
	 * capable
	 */

	/* Don't allow bond devices to change network namespaces. */
	bond_dev->features |= NETIF_F_NETNS_LOCAL;

	bond_dev->hw_features = BOND_VLAN_FEATURES |
				NETIF_F_HW_VLAN_CTAG_TX |
				NETIF_F_HW_VLAN_CTAG_RX |
				NETIF_F_HW_VLAN_CTAG_FILTER;

	bond_dev->hw_features &= ~(NETIF_F_ALL_CSUM & ~NETIF_F_HW_CSUM);
	bond_dev->hw_features |= NETIF_F_GSO_ENCAP_ALL;
	bond_dev->features |= bond_dev->hw_features;
}

/* Destroy a bonding device.
 * Must be under rtnl_lock when this function is called.
 */
static void bond_uninit(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct list_head *iter;
	struct slave *slave;
	struct bond_up_slave *arr;

	bond_netpoll_cleanup(bond_dev);

	/* Release the bonded slaves */
	bond_for_each_slave(bond, slave, iter)
		__bond_release_one(bond_dev, slave->dev, true);
	netdev_info(bond_dev, "Released all slaves\n");

	arr = rtnl_dereference(bond->slave_arr);
	if (arr) {
		RCU_INIT_POINTER(bond->slave_arr, NULL);
		kfree_rcu(arr, rcu);
	}

	list_del(&bond->bond_list);

	bond_debug_unregister(bond);
}

/*------------------------- Module initialization ---------------------------*/

static int bond_check_params(struct bond_params *params)
{
	int arp_validate_value, fail_over_mac_value, primary_reselect_value, i;
	struct bond_opt_value newval;
	const struct bond_opt_value *valptr;
	int arp_all_targets_value;

	/* Convert string parameters. */
	if (mode) {
		bond_opt_initstr(&newval, mode);
		valptr = bond_opt_parse(bond_opt_get(BOND_OPT_MODE), &newval);
		if (!valptr) {
			pr_err("Error: Invalid bonding mode \"%s\"\n", mode);
			return -EINVAL;
		}
		bond_mode = valptr->value;
	}

	if (xmit_hash_policy) {
		if ((bond_mode != BOND_MODE_XOR) &&
		    (bond_mode != BOND_MODE_8023AD) &&
		    (bond_mode != BOND_MODE_TLB)) {
			pr_info("xmit_hash_policy param is irrelevant in mode %s\n",
				bond_mode_name(bond_mode));
		} else {
			bond_opt_initstr(&newval, xmit_hash_policy);
			valptr = bond_opt_parse(bond_opt_get(BOND_OPT_XMIT_HASH),
						&newval);
			if (!valptr) {
				pr_err("Error: Invalid xmit_hash_policy \"%s\"\n",
				       xmit_hash_policy);
				return -EINVAL;
			}
			xmit_hashtype = valptr->value;
		}
	}

	if (lacp_rate) {
		if (bond_mode != BOND_MODE_8023AD) {
			pr_info("lacp_rate param is irrelevant in mode %s\n",
				bond_mode_name(bond_mode));
		} else {
			bond_opt_initstr(&newval, lacp_rate);
			valptr = bond_opt_parse(bond_opt_get(BOND_OPT_LACP_RATE),
						&newval);
			if (!valptr) {
				pr_err("Error: Invalid lacp rate \"%s\"\n",
				       lacp_rate);
				return -EINVAL;
			}
			lacp_fast = valptr->value;
		}
	}

	if (ad_select) {
		bond_opt_initstr(&newval, ad_select);
		valptr = bond_opt_parse(bond_opt_get(BOND_OPT_AD_SELECT),
					&newval);
		if (!valptr) {
			pr_err("Error: Invalid ad_select \"%s\"\n", ad_select);
			return -EINVAL;
		}
		params->ad_select = valptr->value;
		if (bond_mode != BOND_MODE_8023AD)
			pr_warn("ad_select param only affects 802.3ad mode\n");
	} else {
		params->ad_select = BOND_AD_STABLE;
	}

	if (max_bonds < 0) {
		pr_warn("Warning: max_bonds (%d) not in range %d-%d, so it was reset to BOND_DEFAULT_MAX_BONDS (%d)\n",
			max_bonds, 0, INT_MAX, BOND_DEFAULT_MAX_BONDS);
		max_bonds = BOND_DEFAULT_MAX_BONDS;
	}

	if (miimon < 0) {
		pr_warn("Warning: miimon module parameter (%d), not in range 0-%d, so it was reset to 0\n",
			miimon, INT_MAX);
		miimon = 0;
	}

	if (updelay < 0) {
		pr_warn("Warning: updelay module parameter (%d), not in range 0-%d, so it was reset to 0\n",
			updelay, INT_MAX);
		updelay = 0;
	}

	if (downdelay < 0) {
		pr_warn("Warning: downdelay module parameter (%d), not in range 0-%d, so it was reset to 0\n",
			downdelay, INT_MAX);
		downdelay = 0;
	}

	if ((use_carrier != 0) && (use_carrier != 1)) {
		pr_warn("Warning: use_carrier module parameter (%d), not of valid value (0/1), so it was set to 1\n",
			use_carrier);
		use_carrier = 1;
	}

	if (num_peer_notif < 0 || num_peer_notif > 255) {
		pr_warn("Warning: num_grat_arp/num_unsol_na (%d) not in range 0-255 so it was reset to 1\n",
			num_peer_notif);
		num_peer_notif = 1;
	}

	/* reset values for 802.3ad/TLB/ALB */
	if (!bond_mode_uses_arp(bond_mode)) {
		if (!miimon) {
			pr_warn("Warning: miimon must be specified, otherwise bonding will not detect link failure, speed and duplex which are essential for 802.3ad operation\n");
			pr_warn("Forcing miimon to 100msec\n");
			miimon = BOND_DEFAULT_MIIMON;
		}
	}

	if (tx_queues < 1 || tx_queues > 255) {
		pr_warn("Warning: tx_queues (%d) should be between 1 and 255, resetting to %d\n",
			tx_queues, BOND_DEFAULT_TX_QUEUES);
		tx_queues = BOND_DEFAULT_TX_QUEUES;
	}

	if ((all_slaves_active != 0) && (all_slaves_active != 1)) {
		pr_warn("Warning: all_slaves_active module parameter (%d), not of valid value (0/1), so it was set to 0\n",
			all_slaves_active);
		all_slaves_active = 0;
	}

	if (resend_igmp < 0 || resend_igmp > 255) {
		pr_warn("Warning: resend_igmp (%d) should be between 0 and 255, resetting to %d\n",
			resend_igmp, BOND_DEFAULT_RESEND_IGMP);
		resend_igmp = BOND_DEFAULT_RESEND_IGMP;
	}

	bond_opt_initval(&newval, packets_per_slave);
	if (!bond_opt_parse(bond_opt_get(BOND_OPT_PACKETS_PER_SLAVE), &newval)) {
		pr_warn("Warning: packets_per_slave (%d) should be between 0 and %u resetting to 1\n",
			packets_per_slave, USHRT_MAX);
		packets_per_slave = 1;
	}

	if (bond_mode == BOND_MODE_ALB) {
		pr_notice("In ALB mode you might experience client disconnections upon reconnection of a link if the bonding module updelay parameter (%d msec) is incompatible with the forwarding delay time of the switch\n",
			  updelay);
	}

	if (!miimon) {
		if (updelay || downdelay) {
			/* just warn the user the up/down delay will have
			 * no effect since miimon is zero...
			 */
			pr_warn("Warning: miimon module parameter not set and updelay (%d) or downdelay (%d) module parameter is set; updelay and downdelay have no effect unless miimon is set\n",
				updelay, downdelay);
		}
	} else {
		/* don't allow arp monitoring */
		if (arp_interval) {
			pr_warn("Warning: miimon (%d) and arp_interval (%d) can't be used simultaneously, disabling ARP monitoring\n",
				miimon, arp_interval);
			arp_interval = 0;
		}

		if ((updelay % miimon) != 0) {
			pr_warn("Warning: updelay (%d) is not a multiple of miimon (%d), updelay rounded to %d ms\n",
				updelay, miimon, (updelay / miimon) * miimon);
		}

		updelay /= miimon;

		if ((downdelay % miimon) != 0) {
			pr_warn("Warning: downdelay (%d) is not a multiple of miimon (%d), downdelay rounded to %d ms\n",
				downdelay, miimon,
				(downdelay / miimon) * miimon);
		}

		downdelay /= miimon;
	}

	if (arp_interval < 0) {
		pr_warn("Warning: arp_interval module parameter (%d), not in range 0-%d, so it was reset to 0\n",
			arp_interval, INT_MAX);
		arp_interval = 0;
	}

	for (arp_ip_count = 0, i = 0;
	     (arp_ip_count < BOND_MAX_ARP_TARGETS) && arp_ip_target[i]; i++) {
		__be32 ip;

		/* not a complete check, but good enough to catch mistakes */
		if (!in4_pton(arp_ip_target[i], -1, (u8 *)&ip, -1, NULL) ||
		    !bond_is_ip_target_ok(ip)) {
			pr_warn("Warning: bad arp_ip_target module parameter (%s), ARP monitoring will not be performed\n",
				arp_ip_target[i]);
			arp_interval = 0;
		} else {
			if (bond_get_targets_ip(arp_target, ip) == -1)
				arp_target[arp_ip_count++] = ip;
			else
				pr_warn("Warning: duplicate address %pI4 in arp_ip_target, skipping\n",
					&ip);
		}
	}

	if (arp_interval && !arp_ip_count) {
		/* don't allow arping if no arp_ip_target given... */
		pr_warn("Warning: arp_interval module parameter (%d) specified without providing an arp_ip_target parameter, arp_interval was reset to 0\n",
			arp_interval);
		arp_interval = 0;
	}

	if (arp_validate) {
		if (!arp_interval) {
			pr_err("arp_validate requires arp_interval\n");
			return -EINVAL;
		}

		bond_opt_initstr(&newval, arp_validate);
		valptr = bond_opt_parse(bond_opt_get(BOND_OPT_ARP_VALIDATE),
					&newval);
		if (!valptr) {
			pr_err("Error: invalid arp_validate \"%s\"\n",
			       arp_validate);
			return -EINVAL;
		}
		arp_validate_value = valptr->value;
	} else {
		arp_validate_value = 0;
	}

	arp_all_targets_value = 0;
	if (arp_all_targets) {
		bond_opt_initstr(&newval, arp_all_targets);
		valptr = bond_opt_parse(bond_opt_get(BOND_OPT_ARP_ALL_TARGETS),
					&newval);
		if (!valptr) {
			pr_err("Error: invalid arp_all_targets_value \"%s\"\n",
			       arp_all_targets);
			arp_all_targets_value = 0;
		} else {
			arp_all_targets_value = valptr->value;
		}
	}

	if (miimon) {
		pr_info("MII link monitoring set to %d ms\n", miimon);
	} else if (arp_interval) {
		valptr = bond_opt_get_val(BOND_OPT_ARP_VALIDATE,
					  arp_validate_value);
		pr_info("ARP monitoring set to %d ms, validate %s, with %d target(s):",
			arp_interval, valptr->string, arp_ip_count);

		for (i = 0; i < arp_ip_count; i++)
			pr_cont(" %s", arp_ip_target[i]);

		pr_cont("\n");

	} else if (max_bonds) {
		/* miimon and arp_interval not set, we need one so things
		 * work as expected, see bonding.txt for details
		 */
		pr_debug("Warning: either miimon or arp_interval and arp_ip_target module parameters must be specified, otherwise bonding will not detect link failures! see bonding.txt for details\n");
	}

	if (primary && !bond_mode_uses_primary(bond_mode)) {
		/* currently, using a primary only makes sense
		 * in active backup, TLB or ALB modes
		 */
		pr_warn("Warning: %s primary device specified but has no effect in %s mode\n",
			primary, bond_mode_name(bond_mode));
		primary = NULL;
	}

	if (primary && primary_reselect) {
		bond_opt_initstr(&newval, primary_reselect);
		valptr = bond_opt_parse(bond_opt_get(BOND_OPT_PRIMARY_RESELECT),
					&newval);
		if (!valptr) {
			pr_err("Error: Invalid primary_reselect \"%s\"\n",
			       primary_reselect);
			return -EINVAL;
		}
		primary_reselect_value = valptr->value;
	} else {
		primary_reselect_value = BOND_PRI_RESELECT_ALWAYS;
	}

	if (fail_over_mac) {
		bond_opt_initstr(&newval, fail_over_mac);
		valptr = bond_opt_parse(bond_opt_get(BOND_OPT_FAIL_OVER_MAC),
					&newval);
		if (!valptr) {
			pr_err("Error: invalid fail_over_mac \"%s\"\n",
			       fail_over_mac);
			return -EINVAL;
		}
		fail_over_mac_value = valptr->value;
		if (bond_mode != BOND_MODE_ACTIVEBACKUP)
			pr_warn("Warning: fail_over_mac only affects active-backup mode\n");
	} else {
		fail_over_mac_value = BOND_FOM_NONE;
	}

	if (lp_interval == 0) {
		pr_warn("Warning: ip_interval must be between 1 and %d, so it was reset to %d\n",
			INT_MAX, BOND_ALB_DEFAULT_LP_INTERVAL);
		lp_interval = BOND_ALB_DEFAULT_LP_INTERVAL;
	}

	/* fill params struct with the proper values */
	params->mode = bond_mode;
	params->xmit_policy = xmit_hashtype;
	params->miimon = miimon;
	params->num_peer_notif = num_peer_notif;
	params->arp_interval = arp_interval;
	params->arp_validate = arp_validate_value;
	params->arp_all_targets = arp_all_targets_value;
	params->updelay = updelay;
	params->downdelay = downdelay;
	params->use_carrier = use_carrier;
	params->lacp_fast = lacp_fast;
	params->primary[0] = 0;
	params->primary_reselect = primary_reselect_value;
	params->fail_over_mac = fail_over_mac_value;
	params->tx_queues = tx_queues;
	params->all_slaves_active = all_slaves_active;
	params->resend_igmp = resend_igmp;
	params->min_links = min_links;
	params->lp_interval = lp_interval;
	params->packets_per_slave = packets_per_slave;
	params->tlb_dynamic_lb = 1; /* Default value */
	if (packets_per_slave > 0) {
		params->reciprocal_packets_per_slave =
			reciprocal_value(packets_per_slave);
	} else {
		/* reciprocal_packets_per_slave is unused if
		 * packets_per_slave is 0 or 1, just initialize it
		 */
		params->reciprocal_packets_per_slave =
			(struct reciprocal_value) { 0 };
	}

	if (primary) {
		strncpy(params->primary, primary, IFNAMSIZ);
		params->primary[IFNAMSIZ - 1] = 0;
	}

	memcpy(params->arp_targets, arp_target, sizeof(arp_target));

	return 0;
}

static struct lock_class_key bonding_netdev_xmit_lock_key;
static struct lock_class_key bonding_netdev_addr_lock_key;
static struct lock_class_key bonding_tx_busylock_key;

static void bond_set_lockdep_class_one(struct net_device *dev,
				       struct netdev_queue *txq,
				       void *_unused)
{
	lockdep_set_class(&txq->_xmit_lock,
			  &bonding_netdev_xmit_lock_key);
}

static void bond_set_lockdep_class(struct net_device *dev)
{
	lockdep_set_class(&dev->addr_list_lock,
			  &bonding_netdev_addr_lock_key);
	netdev_for_each_tx_queue(dev, bond_set_lockdep_class_one, NULL);
	dev->qdisc_tx_busylock = &bonding_tx_busylock_key;
}

/* Called from registration process */
static int bond_init(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct bond_net *bn = net_generic(dev_net(bond_dev), bond_net_id);

	netdev_dbg(bond_dev, "Begin bond_init\n");

	bond->wq = create_singlethread_workqueue(bond_dev->name);
	if (!bond->wq)
		return -ENOMEM;

	bond_set_lockdep_class(bond_dev);

	list_add_tail(&bond->bond_list, &bn->dev_list);

	bond_prepare_sysfs_group(bond);

	bond_debug_register(bond);

	/* Ensure valid dev_addr */
	if (is_zero_ether_addr(bond_dev->dev_addr) &&
	    bond_dev->addr_assign_type == NET_ADDR_PERM)
		eth_hw_addr_random(bond_dev);

	return 0;
}

unsigned int bond_get_num_tx_queues(void)
{
	return tx_queues;
}

/* Create a new bond based on the specified name and bonding parameters.
 * If name is NULL, obtain a suitable "bond%d" name for us.
 * Caller must NOT hold rtnl_lock; we need to release it here before we
 * set up our sysfs entries.
 */
int bond_create(struct net *net, const char *name)
{
	struct net_device *bond_dev;
	struct bonding *bond;
	struct alb_bond_info *bond_info;
	int res;

	rtnl_lock();

	bond_dev = alloc_netdev_mq(sizeof(struct bonding),
				   name ? name : "bond%d", NET_NAME_UNKNOWN,
				   bond_setup, tx_queues);
	if (!bond_dev) {
		pr_err("%s: eek! can't alloc netdev!\n", name);
		rtnl_unlock();
		return -ENOMEM;
	}

	/*
	 * Initialize rx_hashtbl_used_head to RLB_NULL_INDEX.
	 * It is set to 0 by default which is wrong.
	 */
	bond = netdev_priv(bond_dev);
	bond_info = &(BOND_ALB_INFO(bond));
	bond_info->rx_hashtbl_used_head = RLB_NULL_INDEX;

	dev_net_set(bond_dev, net);
	bond_dev->rtnl_link_ops = &bond_link_ops;

	res = register_netdevice(bond_dev);

	netif_carrier_off(bond_dev);

	rtnl_unlock();
	if (res < 0)
		bond_destructor(bond_dev);
	return res;
}

static int __net_init bond_net_init(struct net *net)
{
	struct bond_net *bn = net_generic(net, bond_net_id);

	bn->net = net;
	INIT_LIST_HEAD(&bn->dev_list);

	bond_create_proc_dir(bn);
	bond_create_sysfs(bn);

	return 0;
}

static void __net_exit bond_net_exit(struct net *net)
{
	struct bond_net *bn = net_generic(net, bond_net_id);
	struct bonding *bond, *tmp_bond;
	LIST_HEAD(list);

	bond_destroy_sysfs(bn);

	/* Kill off any bonds created after unregistering bond rtnl ops */
	rtnl_lock();
	list_for_each_entry_safe(bond, tmp_bond, &bn->dev_list, bond_list)
		unregister_netdevice_queue(bond->dev, &list);
	unregister_netdevice_many(&list);
	rtnl_unlock();

	bond_destroy_proc_dir(bn);
}

static struct pernet_operations bond_net_ops = {
	.init = bond_net_init,
	.exit = bond_net_exit,
	.id   = &bond_net_id,
	.size = sizeof(struct bond_net),
};

#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) && defined(CONFIG_BCM_KF_LOG)
/* Callback function through bcm_log function pointer;
	Given a slave device ; update bond group stat, and clear saved slave stat */
int bcm_bond_clr_slave_stat(void *ctxt)
{
    struct net_device *slave_dev = ctxt;

    if (netif_is_bond_slave(slave_dev)) {
        struct slave *slave = bond_slave_get_rcu(slave_dev);
        if (slave) {
            rcu_read_lock();
            /* clear slave stats for the next run */
            memset(&slave->slave_stats, 0, sizeof(slave->slave_stats));
            rcu_read_unlock();
            return 0;
        }
    }
    return -1;
}

/* Callback function through bcm_log function pointer;
	Given a RX Handler function pointer for a device ; confirms if it is same as bond RX handler */
int bcm_bond_is_rx_handler(void *ctxt)
{
    return (ctxt == (void*)bond_handle_frame);
}

#endif /* defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) */

static int __init bonding_init(void)
{
	int i;
	int res;

	pr_info("%s", bond_version);

	res = bond_check_params(&bonding_defaults);
	if (res)
		goto out;

	res = register_pernet_subsys(&bond_net_ops);
	if (res)
		goto out;

	res = bond_netlink_init();
	if (res)
		goto err_link;

	bond_create_debugfs();

	for (i = 0; i < max_bonds; i++) {
		res = bond_create(&init_net, NULL);
		if (res)
			goto err;
	}

	register_netdevice_notifier(&bond_netdev_notifier);

#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) 
	register_bridge_stp_notifier(&bond_br_stp_notifier);
	bcmFun_reg(BCM_FUN_ID_BOND_CLR_SLAVE_STAT, bcm_bond_clr_slave_stat);
	bcmFun_reg(BCM_FUN_ID_BOND_RX_HANDLER, bcm_bond_is_rx_handler);
#endif /* defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) */

out:
	return res;
err:
	bond_destroy_debugfs();
	bond_netlink_fini();
err_link:
	unregister_pernet_subsys(&bond_net_ops);
	goto out;

}

static void __exit bonding_exit(void)
{
	unregister_netdevice_notifier(&bond_netdev_notifier);

#if defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING)
	unregister_bridge_stp_notifier(&bond_br_stp_notifier);
	bcmFun_dereg(BCM_FUN_ID_BOND_CLR_SLAVE_STAT);
	bcmFun_dereg(BCM_FUN_ID_BOND_RX_HANDLER);
#endif /* defined(CONFIG_BCM_KF_KBONDING) && defined(CONFIG_BCM_KERNEL_BONDING) */

	bond_destroy_debugfs();

	bond_netlink_fini();
	unregister_pernet_subsys(&bond_net_ops);

#ifdef CONFIG_NET_POLL_CONTROLLER
	/* Make sure we don't have an imbalance on our netpoll blocking */
	WARN_ON(atomic_read(&netpoll_block_tx));
#endif
}

module_init(bonding_init);
module_exit(bonding_exit);
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION(DRV_DESCRIPTION ", v" DRV_VERSION);
MODULE_AUTHOR("Thomas Davis, tadavis@lbl.gov and many others");
