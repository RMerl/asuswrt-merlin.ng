#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

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
/*
 * File: l3mhdp.c
 *
 * MHDP - Modem Host Data Protocol for MHI protocol family.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#ifdef CONFIG_BLOG
#include <linux/nbuff.h>
#include <linux/blog.h>
#endif
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/mhi_l2mux.h>
#include <linux/etherdevice.h>
#include <linux/pkt_sched.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <linux/udp.h>
#include <net/mhi/sock.h>
#include <linux/skbuff.h>

/* wake_lock prevents the system from entering suspend or other
 * low power states when active.  This seems to be used in an
 * Android system, but it is not present in Vanilla Kernel */
//#define SUPPORT_WAKE_LOCK	1

#ifdef SUPPORT_WAKE_LOCK
#include <linux/wakelock.h>
#endif

#ifdef CONFIG_MHDP_BONDING_SUPPORT
#define MHDP_BONDING_SUPPORT
#endif

//#define MHDP_USE_NAPI

#ifdef MHDP_BONDING_SUPPORT
#include <linux/etherdevice.h>
#endif /*MHDP_BONDING_SUPPORT */

#include <net/netns/generic.h>
#include <net/mhi/mhdp.h>

/* MHDP device MTU limits */
#define MHDP_MTU_MAX		0x2400
#define MHDP_MTU_MIN		0x44
#define MAX_MHDP_FRAME_SIZE	16000

/* MHDP device names */
#define MHDP_IFNAME			"rmnet%d"
#define MHDP_CTL_IFNAME		"rmnetctl"

/* Print every MHDP SKB content */
/* #define MHDP_DEBUG_SKB */

/* #define CONFIG_MHI_DEBUG */

#define UDP_PROT_TYPE	17

#define EPRINTK(...)    pr_debug("MHI/MHDP: " __VA_ARGS__)

#ifdef CONFIG_MHI_DEBUG
# define DPRINTK(...)    pr_debug("MHI/MHDP: " __VA_ARGS__)
#else
# define DPRINTK(...)
#endif

#ifdef MHDP_DEBUG_SKB
# define SKBPRINT(a, b)    __print_skb_content(a, b)
#else
# define SKBPRINT(a, b)
#endif

/* IPv6 support */
#define VER_IPv4 0x04
#define VER_IPv6 0x06
#define ETH_IP_TYPE(x) (((0x00|(x>>4)) == VER_IPv4) ? ETH_P_IP : ETH_P_IPV6)

/*** Type definitions ***/

#define MAX_MHDPHDR_SIZE MAX_SKB_FRAGS

#ifdef MHDP_USE_NAPI
#define NAPI_WEIGHT 64
#endif /*MHDP_USE_NAPI */


struct mhdp_tunnel {
	struct mhdp_tunnel *next;
	struct net_device *dev;
	struct net_device *master_dev;
	struct sk_buff *skb;
	int sim_id;
	int pdn_id;
	int free_pdn;
	struct hrtimer tx_timer;
	struct tasklet_struct taskl;
	struct sk_buff *skb_to_free[MAX_MHDPHDR_SIZE];
	spinlock_t timer_lock;
};

struct mhdp_net {
	struct mhdp_tunnel *tunnels;
	struct net_device *ctl_dev;
	struct mhdp_udp_filter udp_filter;
	spinlock_t udp_lock;
#ifdef MHDP_USE_NAPI
	struct net_device *dev;
	struct napi_struct napi;
	struct sk_buff_head skb_list;
#endif				/*#ifdef MHDP_USE_NAPI */
#ifdef SUPPORT_WAKE_LOCK
	int wake_lock_time;
	struct wake_lock wakelock;
	spinlock_t wl_lock;
#endif
};

struct packet_info {
	uint32_t pdn_id;
	uint32_t packet_offset;
	uint32_t packet_length;
};

struct mhdp_hdr {
	uint32_t packet_count;
	struct packet_info info[MAX_MHDPHDR_SIZE];
};

/*** Prototypes ***/

static void mhdp_netdev_setup(struct net_device *dev);

static void mhdp_submit_queued_skb(struct mhdp_tunnel *tunnel, int force_send);

static int mhdp_netdev_event(struct notifier_block *this,
			     unsigned long event, void *ptr);

static enum hrtimer_restart tx_timer_timeout(struct hrtimer *timer);
static void tx_timer_timeout_tasklet(unsigned long arg);

#ifdef SUPPORT_WAKE_LOCK
static ssize_t mhdp_write_wakelock_value(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count);
static ssize_t mhdp_read_wakelock_value(struct device *dev,
					struct device_attribute *attr,
					char *buf);
#endif

#ifdef MHDP_USE_NAPI

static int mhdp_poll(struct napi_struct *napi, int budget);

#endif /*MHDP_USE_NAPI */

/*** Global Variables ***/

static int mhdp_net_id __read_mostly;

static struct notifier_block mhdp_netdev_notifier = {
	.notifier_call = mhdp_netdev_event,
};

#ifdef SUPPORT_WAKE_LOCK
static struct device_attribute mhdpwl_dev_attrs[] = {
	__ATTR(mhdp_wakelock_time,
	       S_IRUGO | S_IWUSR,
	       mhdp_read_wakelock_value,
	       mhdp_write_wakelock_value),
	__ATTR_NULL,
};
#endif

/*** Funtions ***/

#ifdef MHDP_DEBUG_SKB
static void __print_skb_content(struct sk_buff *skb, const char *tag)
{
	struct page *page;
	skb_frag_t *frag;
	int len;
	int i, j;
	u8 *ptr;

	/* Main SKB buffer */
	ptr = (u8 *)skb->data;
	len = skb_headlen(skb);

	pr_debug("MHDP: SKB buffer lenght %02u\n", len);
	for (i = 0; i < len; i++) {
		if (i % 8 == 0)
			pr_debug("%s DATA: ", tag);
		pr_debug(" 0x%02X", ptr[i]);
		if (i % 8 == 7 || i == len - 1)
			pr_debug("\n");
	}

	/* SKB fragments */
	for (i = 0; i < (skb_shinfo(skb)->nr_frags); i++) {
		frag = &skb_shinfo(skb)->frags[i];
		page = skb_frag_page(frag);

		ptr = page_address(page);

		for (j = 0; j < frag->size; j++) {
			if (j % 8 == 0)
				pr_debug("%s FRAG[%d]: ", tag, i);
			pr_debug(" 0x%02X", ptr[frag->page_offset + j]);
			if (j % 8 == 7 || j == frag->size - 1)
				pr_debug("\n");
		}
	}
}
#endif

/**
 * mhdp_net_dev - Get mhdp_net structure of mhdp tunnel
 */
static inline struct mhdp_net *mhdp_net_dev(struct net_device *dev)
{
	return net_generic(dev_net(dev), mhdp_net_id);
}

/**
 * mhdp_tunnel_init - Initialize MHDP tunnel
 */
static void
mhdp_tunnel_init(struct net_device *dev,
		 struct mhdp_tunnel_parm *parms, struct net_device *master_dev)
{
	struct mhdp_net *mhdpn = mhdp_net_dev(dev);
	struct mhdp_tunnel *tunnel = netdev_priv(dev);

	DPRINTK("mhdp_tunnel_init: dev:%s", dev->name);

	tunnel->next = mhdpn->tunnels;
	mhdpn->tunnels = tunnel;
#ifdef SUPPORT_WAKE_LOCK
	spin_lock_init(&mhdpn->wl_lock);
#endif

	tunnel->dev = dev;
	tunnel->master_dev = master_dev;
	tunnel->skb = NULL;
	tunnel->sim_id = parms->sim_id;
	tunnel->pdn_id = parms->pdn_id;
	tunnel->free_pdn = 0;
	netdev_path_add(dev, master_dev);

	hrtimer_init(&tunnel->tx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tunnel->tx_timer.function = &tx_timer_timeout;
	tasklet_init(&tunnel->taskl,
		     tx_timer_timeout_tasklet, (unsigned long)tunnel);

	spin_lock_init(&tunnel->timer_lock);
}

/**
 * mhdp_tunnel_destroy - Destroy MHDP tunnel
 */
static void mhdp_tunnel_destroy(struct net_device *dev)
{
	DPRINTK("mhdp_tunnel_destroy: dev:%s", dev->name);

	netdev_path_remove(dev);
	unregister_netdevice(dev);
}

/**
 * mhdp_destroy_tunnels - Initialize all MHDP tunnels
 */
static void mhdp_destroy_tunnels(struct mhdp_net *mhdpn)
{
	struct mhdp_tunnel *tunnel;

	for (tunnel = mhdpn->tunnels; (tunnel); tunnel = tunnel->next) {
		mhdp_tunnel_destroy(tunnel->dev);
		if (hrtimer_active(&tunnel->tx_timer))
			hrtimer_cancel(&tunnel->tx_timer);
		tasklet_kill(&tunnel->taskl);
	}

	mhdpn->tunnels = NULL;
}

/**
 * mhdp_locate_tunnel - Retrieve MHDP tunnel thanks to PDN
 */
static inline struct mhdp_tunnel *mhdp_locate_tunnel(struct mhdp_net *mhdpn,
					      int pdn_id)
{
	struct mhdp_tunnel *tunnel;

	for (tunnel = mhdpn->tunnels; tunnel; tunnel = tunnel->next)
		if (tunnel->pdn_id == pdn_id)
			return tunnel;

	return NULL;
}

/**
 * mhdp_add_tunnel - Add MHDP tunnel
 */
static struct net_device *mhdp_add_tunnel(struct net *net,
					  struct mhdp_tunnel_parm *parms)
{
	struct net_device *mhdp_dev, *master_dev;

	DPRINTK("mhdp_add_tunnel: adding a tunnel to %s\n", parms->master);

	master_dev = dev_get_by_name(net, parms->master);
	if (!master_dev)
		goto err_alloc_dev;

	mhdp_dev = alloc_netdev(sizeof(struct mhdp_tunnel), MHDP_IFNAME,
				NET_NAME_UNKNOWN, mhdp_netdev_setup);
	if (!mhdp_dev)
		goto err_alloc_dev;

	dev_net_set(mhdp_dev, net);

	if (dev_alloc_name(mhdp_dev, MHDP_IFNAME) < 0)
		goto err_reg_dev;

	strcpy(parms->name, mhdp_dev->name);

#if defined(CONFIG_BCM_KF_WANDEV)
	mhdp_dev->priv_flags |= IFF_WANDEV;
#endif

	if (register_netdevice(mhdp_dev)) {
		pr_err("MHDP: register_netdev failed\n");
		goto err_reg_dev;
	}

	dev_hold(mhdp_dev);

	mhdp_tunnel_init(mhdp_dev, parms, master_dev);

	dev_put(master_dev);

	return mhdp_dev;

err_reg_dev:
	netdev_path_remove(mhdp_dev);
	free_netdev(mhdp_dev);
err_alloc_dev:
	return NULL;
}

#ifdef SUPPORT_WAKE_LOCK
/**
 * mhdp_write_wakelock_value - store the wakelock value in mhdp
 * @dev: Device to be created
 * @attr: attribute of sysfs
 * @buf: output stringwait
 */
static ssize_t
mhdp_write_wakelock_value(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int retval = count;
	unsigned long flags;
	struct mhdp_net *mhdpn = dev_get_drvdata(dev);
	long int time;

	if (kstrtol(buf, 10, &time)) {
		EPRINTK("%s cannot access to wake lock time", __func__);
		return -EINVAL;
	}
	spin_lock_irqsave(&mhdpn->wl_lock, flags);
	mhdpn->wake_lock_time = (int)time;
	spin_unlock_irqrestore(&mhdpn->wl_lock, flags);

	DPRINTK("%s wake_lock_time = %d\n", __func__, mhdpn->wake_lock_time);

	if ((wake_lock_active(&mhdpn->wakelock)) &&
	    (mhdpn->wake_lock_time <= 0)) {

		wake_unlock(&mhdpn->wakelock);

	} else if ((wake_lock_active(&mhdpn->wakelock)) &&
		   (mhdpn->wake_lock_time > 0)) {

		wake_lock_timeout(&mhdpn->wakelock, mhdpn->wake_lock_time * HZ);
	}
	return retval;
}

/**
 * mhdp_read_wakelock_value - read the wakelock value in mhdp
 * @dev: Device to be created
 * @attr: attribute of sysfs
 * @buf: output stringwait
 */
static ssize_t
mhdp_read_wakelock_value(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct mhdp_net *mhdpn = dev_get_drvdata(dev);

	if (mhdpn)
		return sprintf(buf, "%d\n", mhdpn->wake_lock_time);

	return sprintf(buf, "%d\n", 0);
}

/**
 * mmhdp_check_wake_lock - check the wakelock state and restart wake lock if any
 * @dev: net Device pointer
 */
void mhdp_check_wake_lock(struct net_device *dev)
{
	unsigned long flags;
	struct mhdp_net *mhdpn = mhdp_net_dev(dev);

	spin_lock_irqsave(&mhdpn->wl_lock, flags);

	if (mhdpn->wake_lock_time != 0) {

		spin_unlock_irqrestore(&mhdpn->wl_lock, flags);

		wake_lock_timeout(&mhdpn->wakelock, mhdpn->wake_lock_time * HZ);
	} else {
		spin_unlock_irqrestore(&mhdpn->wl_lock, flags);
	}
}
#endif /* SUPPORT_WAKE_LOCK */

static void
mhdp_set_udp_filter(struct mhdp_net *mhdpn, struct mhdp_udp_filter *filter)
{
	unsigned long flags;
	spin_lock_irqsave(&mhdpn->udp_lock, flags);
	mhdpn->udp_filter.port_id = filter->port_id;
	mhdpn->udp_filter.active = 1;
	spin_unlock_irqrestore(&mhdpn->udp_lock, flags);
}

static void mhdp_reset_udp_filter(struct mhdp_net *mhdpn)
{
	unsigned long flags;
	spin_lock_irqsave(&mhdpn->udp_lock, flags);
	mhdpn->udp_filter.port_id = 0;
	mhdpn->udp_filter.active = 0;
	spin_unlock_irqrestore(&mhdpn->udp_lock, flags);

}

static int mhdp_is_filtered(struct mhdp_net *mhdpn, struct sk_buff *skb)
{
	struct ipv6hdr *ipv6header;
	struct iphdr *ipv4header;
	struct udphdr *udphdr;
	int ret = 0;
	__be16 frag_off;
	int offset = 0;
	u8 next_hdr;
	unsigned int size_of_previous_hdr;
	struct sk_buff *newskb;
	unsigned long flags;

	spin_lock_irqsave(&mhdpn->udp_lock, flags);

	if (mhdpn->udp_filter.active == 0) {
		spin_unlock_irqrestore(&mhdpn->udp_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&mhdpn->udp_lock, flags);

	/*if udp, check port number */
	if (skb->protocol == htons(ETH_P_IP)) {

		ipv4header = ip_hdr(skb);

		if (ipv4header->protocol == UDP_PROT_TYPE) {

			udphdr = (struct udphdr *)((unsigned int *)ipv4header +
						   ipv4header->ihl);

			if (htons(udphdr->dest) == mhdpn->udp_filter.port_id) {
				size_of_previous_hdr = ipv4header->ihl *
				    sizeof(unsigned int);
				ret = 1;
				DPRINTK("MHDP_FIL: IPv4 packet filtered out\n");
			}
		}

	} else if (skb->protocol == htons(ETH_P_IPV6)) {

		ipv6header = ipv6_hdr(skb);
		next_hdr = ipv6header->nexthdr;

		if ((next_hdr == NEXTHDR_TCP) || (next_hdr == NEXTHDR_ICMP))
			goto no_filter;
		else if (next_hdr == UDP_PROT_TYPE)
			goto treat_udp;

		if (!ipv6_ext_hdr(next_hdr)) {
			DPRINTK("!ipv6_ext_hdr(next_hdr): %d\n", next_hdr);
			goto no_filter;
		}

		offset = ipv6_skip_exthdr(skb,
					  sizeof(struct ipv6hdr),
					  &next_hdr, &frag_off);

		if (offset < 0) {
			DPRINTK("MHDP_FILTER offset < 0: %d\n", next_hdr);
			goto no_filter;
		}

treat_udp:
		if (next_hdr == UDP_PROT_TYPE) {

			udphdr = (struct udphdr *)((unsigned char *)ipv6header +
						   sizeof(struct ipv6hdr) +
						   offset);

			DPRINTK("MHDP_FILTER: UDP header found\n");

			if (htons(udphdr->dest) == mhdpn->udp_filter.port_id) {
				ret = 1;
				size_of_previous_hdr =
				    (unsigned int)((unsigned char *)udphdr -
						   (unsigned char *)ipv6header);
				DPRINTK("MHDP_FIL: IPv6 packet filtered out\n");
			} else {
				DPRINTK("MHDP_FILTER: wrong port %d != %d\n",
					htons(udphdr->dest),
					mhdpn->udp_filter.port_id);
			}
		}
	}

	if (ret == 1) {

		newskb = skb_clone(skb, GFP_ATOMIC);

		if (unlikely(!newskb)) {
			ret = 0;
			goto no_filter;
		}

		skb_pull(newskb, (size_of_previous_hdr + sizeof(unsigned int)));

		newskb->len = (unsigned int)htons(udphdr->len) -
		    sizeof(unsigned int);
		newskb->protocol = UDP_PROT_TYPE;
		skb_set_tail_pointer(newskb, newskb->len);

		newskb->truesize = newskb->len + sizeof(struct sk_buff);

		mhi_sock_rcv_multicast(newskb,
				       MHI_L3_MHDP_UDP_FILTER, newskb->len);

		dev_kfree_skb(skb);
	}
no_filter:

	return ret;
}

/**
 * mhdp_netdev_ioctl - I/O control on mhdp tunnel
 */
static int mhdp_netdev_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct net *net = dev_net(dev);
	struct mhdp_net *mhdpn = mhdp_net_dev(dev);
	struct mhdp_tunnel *tunnel, *pre_dev;
	struct mhdp_tunnel_parm __user *u_parms;
	struct mhdp_tunnel_parm k_parms;
	struct mhdp_udp_filter __user *u_filter;
	struct mhdp_udp_filter k_filter;

	int err = 0;

	DPRINTK("mhdp tunnel ioctl %X", cmd);

	switch (cmd) {

	case SIOCADDPDNID:
		u_parms = (struct mhdp_tunnel_parm *)ifr->ifr_data;
		if (copy_from_user(&k_parms, u_parms,
				   sizeof(struct mhdp_tunnel_parm))) {
			DPRINTK("Error: Failed to copy data from user space");
			return -EFAULT;
		}

		DPRINTK("pdn_id:%d sim_id:%d master_device:%s",
				k_parms.pdn_id,
				k_parms.sim_id,
				k_parms.master);
		tunnel = mhdp_locate_tunnel(mhdpn, k_parms.pdn_id);

		if (NULL == tunnel) {
			if (mhdp_add_tunnel(net, &k_parms)) {
				if (copy_to_user(u_parms, &k_parms,
						 sizeof(struct
							mhdp_tunnel_parm)))
					err = -EINVAL;
			} else {
				err = -EINVAL;
			}

		} else if (1 == tunnel->free_pdn) {

			tunnel->free_pdn = 0;

			tunnel->sim_id = k_parms.sim_id;
			strcpy(k_parms.name, tunnel->dev->name);

			if (copy_to_user(u_parms, &k_parms,
					 sizeof(struct mhdp_tunnel_parm)))
				err = -EINVAL;
		} else {
			err = -EBUSY;
		}
		break;

	case SIOCDELPDNID:
		u_parms = (struct mhdp_tunnel_parm *)ifr->ifr_data;

		if (copy_from_user(&k_parms, u_parms,
				   sizeof(struct mhdp_tunnel_parm))) {
			DPRINTK("Error: Failed to copy data from user space");
			return -EFAULT;
		}

		DPRINTK("pdn_id:%d sim_id:%d", k_parms.pdn_id, k_parms.sim_id);

		for (tunnel = mhdpn->tunnels, pre_dev = NULL;
		     tunnel; pre_dev = tunnel, tunnel = tunnel->next) {
			if (tunnel->pdn_id == k_parms.pdn_id)
				tunnel->free_pdn = 1;
		}
		break;

	case SIOCRESETMHDP:
		mhdp_destroy_tunnels(mhdpn);
		break;

	case SIOSETUDPFILTER:

		u_filter = (struct mhdp_udp_filter *)ifr->ifr_data;

		if (copy_from_user(&k_filter, u_filter,
				   sizeof(struct mhdp_udp_filter))) {
			DPRINTK("Err: cannot cp filter data from user space\n");
			return -EFAULT;
		}
		if (k_filter.active == 1) {
			DPRINTK("mhdp SIOSETUDPFILTER active on port %d\n",
				k_filter.port_id);
			mhdp_set_udp_filter(mhdpn, &k_filter);
		} else {
			DPRINTK("mhdp SIOSETUDPFILTER filter reset\n");
			mhdp_reset_udp_filter(mhdpn);
		}

		break;

	default:
		err = -EINVAL;
	}

	return err;
}

/**
 * mhdp_netdev_change_mtu - Change mhdp tunnel MTU
 */
static int mhdp_netdev_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < MHDP_MTU_MIN || new_mtu > MHDP_MTU_MAX)
		return -EINVAL;

	dev->mtu = new_mtu;

	return 0;
}

/**
 * mhdp_netdev_uninit - Un initialize mhdp tunnel
 */
static void mhdp_netdev_uninit(struct net_device *dev)
{
	dev_put(dev);
}

/**
 * mhdp_submit_queued_skb - Submit packets to master netdev (IPC)
 Packets can be concatenated or not
 */
static void mhdp_submit_queued_skb(struct mhdp_tunnel *tunnel, int force_send)
{
	struct sk_buff *skb = tunnel->skb;
	struct l2muxhdr *l2hdr;
	struct mhdp_hdr *p_mhdp_hdr;
	int i, nb_frags;

	BUG_ON(!tunnel->master_dev);

	if (skb) {

		p_mhdp_hdr = (struct mhdp_hdr *)tunnel->skb->data;
		nb_frags = le32_to_cpu(p_mhdp_hdr->packet_count);

		if (hrtimer_active(&tunnel->tx_timer))
			hrtimer_cancel(&tunnel->tx_timer);

		skb->protocol = htons(ETH_P_MHDP);
		skb->priority = 1;

		skb->dev = tunnel->master_dev;

		skb_reset_network_header(skb);

		skb_push(skb, L2MUX_HDR_SIZE);
		skb_reset_mac_header(skb);

		l2hdr = l2mux_hdr(skb);
		l2mux_set_proto(l2hdr, (tunnel->sim_id == 1) ?
			MHI_L3_MHDP_UL_PS2 : MHI_L3_MHDP_UL);
		l2mux_set_length(l2hdr, skb->len - L2MUX_HDR_SIZE);

		SKBPRINT(skb, "MHDP: TX");

		tunnel->dev->stats.tx_packets++;
		tunnel->dev->stats.tx_bytes += skb->len;
		tunnel->skb = NULL;

		dev_queue_xmit(skb);

		for (i = 0; i < nb_frags; i++) {
			if (tunnel->skb_to_free[i])
				dev_kfree_skb(tunnel->skb_to_free[i]);
			else
				EPRINTK("%s error no skb to free\n", __func__);
		}
	}
}

/**
 * mhdp_netdev_rx - Received packets from master netdev (IPC)
  Packets can be concatenated or not
 */
static int mhdp_netdev_rx(struct sk_buff *skb, struct net_device *dev)
{
	skb_frag_t *frag = NULL;
	struct page *page = NULL;
	struct sk_buff *newskb = NULL;
	struct mhdp_hdr *p_mhdp_hdr;
	struct mhdp_hdr *p_mhdp_hdr_tmp = NULL;
	int offset, length;
	int err = 0, i, pdn_id;
	int mhdp_header_len;
	struct mhdp_tunnel *tunnel = NULL;
#if 0
	int start = 0;
#endif
	int has_frag = skb_shinfo(skb)->nr_frags;
	uint32_t packet_count;
	unsigned char ip_ver;

#ifdef SUPPORT_WAKE_LOCK
	mhdp_check_wake_lock(dev);
#endif

#if 0
	if (has_frag) {
		frag = &skb_shinfo(skb)->frags[0];
		page = skb_frag_page(frag);
	}

	if (skb_headlen(skb) > L2MUX_HDR_SIZE)
		skb_pull(skb, L2MUX_HDR_SIZE);
	else if (has_frag)
		frag->page_offset += L2MUX_HDR_SIZE;
#else
	skb_pull(skb, L2MUX_HDR_SIZE);
#endif

	packet_count = le32_to_cpu(*((unsigned char *)skb->data));

	mhdp_header_len = sizeof(packet_count) +
	    (packet_count * sizeof(struct packet_info));

#if 0
	if (mhdp_header_len > skb_headlen(skb)) {
		int skbheadlen = skb_headlen(skb);

		DPRINTK("mhdp header length: %d, skb_headerlen: %d",
			mhdp_header_len, skbheadlen);

		p_mhdp_hdr = kmalloc(mhdp_header_len, GFP_ATOMIC);

		if (NULL == p_mhdp_hdr)
			goto error;

		p_mhdp_hdr_tmp = p_mhdp_hdr;

		if ((skbheadlen == 0) && (has_frag)) {
			memcpy((__u8 *) p_mhdp_hdr, page_address(page) +
			       frag->page_offset, mhdp_header_len);

		} else if (has_frag) {
			memcpy((__u8 *) p_mhdp_hdr, skb->data, skbheadlen);

			memcpy((__u8 *) p_mhdp_hdr + skbheadlen,
			       page_address(page) +
			       frag->page_offset, mhdp_header_len - skbheadlen);

			start = mhdp_header_len - skbheadlen;
		} else {
			EPRINTK("not a valid mhdp frame");
			goto error;
		}

		DPRINTK("page start: %d", start);
	} else {
		DPRINTK("skb->data has whole mhdp header");
		p_mhdp_hdr = (struct mhdp_hdr *)(((__u8 *) skb->data));
	}

	DPRINTK("MHDP PACKET COUNT : %d", le32_to_cpu(p_mhdp_hdr->packet_count));
#else
	p_mhdp_hdr = (struct mhdp_hdr *)(((__u8 *) skb->data));
#endif

	if (le32_to_cpu(p_mhdp_hdr->packet_count) == 1) {
		pdn_id = le32_to_cpu(p_mhdp_hdr->info[0].pdn_id);
		offset = le32_to_cpu(p_mhdp_hdr->info[0].packet_offset);
		length = le32_to_cpu(p_mhdp_hdr->info[0].packet_length);

		skb_pull(skb, mhdp_header_len + offset);
		skb_trim(skb, length);

		ip_ver = (u8)*skb->data;
		
		skb_reset_network_header(skb);
		skb->protocol = htons(ETH_IP_TYPE(ip_ver));
		skb->ip_summed = CHECKSUM_NONE;
		skb->pkt_type = PACKET_HOST;

//		rcu_read_lock();
//		if (!mhdp_is_filtered(mhdp_net_dev(dev), skb)) {
			//skb_tunnel_rx(skb, dev);
			dev->stats.rx_packets++;
			dev->stats.rx_bytes += skb->len;

			tunnel = mhdp_locate_tunnel(mhdp_net_dev(dev), pdn_id);
			if (tunnel) {
				struct net_device_stats *stats =
				    &tunnel->dev->stats;
				stats->rx_packets++;
				stats->rx_bytes += skb->len;
				skb->dev = tunnel->dev;
				SKBPRINT(skb, "SKB: RX");

#if 0
{
	/* debug purpose, dump out the packet content */
	int i;
	uint32_t *u32_ptr = (uint32_t *)skb->data;
	for (i = 0; i < (skb->len >> 2); i++) {
		printk("0x%08x: %08x\n", (uint32_t)(skb->data + (i << 2)), u32_ptr[i]);
	}
}
#endif
				netif_receive_skb(skb);
#if 0
#ifdef MHDP_USE_NAPI
				netif_receive_skb(skb);
#else
				netif_rx(skb);
#endif /*#ifdef MHDP_USE_NAPI */
#endif
			}
//		}
//		rcu_read_unlock();
		kfree(p_mhdp_hdr_tmp);
		return 0;
	}

	for (i = 0; i < le32_to_cpu(p_mhdp_hdr->packet_count); i++) {
		pdn_id = le32_to_cpu(p_mhdp_hdr->info[i].pdn_id);
		offset = le32_to_cpu(p_mhdp_hdr->info[i].packet_offset);
		length = le32_to_cpu(p_mhdp_hdr->info[i].packet_length);

		DPRINTK(" pkt_info[%d] - PDNID:%d, pkt_off: %d, pkt_len: %d\n",
		     i, pdn_id, packet_offset, packet_length);


		if (skb_headlen(skb) > (mhdp_header_len + offset)) {

			newskb = skb_clone(skb, GFP_ATOMIC);
			if (unlikely(!newskb))
				goto error;

			skb_pull(newskb, mhdp_header_len + offset);

			skb_trim(newskb, length);
			newskb->truesize = SKB_TRUESIZE(length);

			ip_ver = (u8)*newskb->data;

		} else if (has_frag) {

			newskb = netdev_alloc_skb(dev, skb_headlen(skb));

			if (unlikely(!newskb))
				goto error;

			get_page(page);
			skb_add_rx_frag(newskb,
					skb_shinfo(newskb)->nr_frags,
					page,
					frag->page_offset +
					((mhdp_header_len - skb_headlen(skb)) +
					 offset), length, length);

			ip_ver = *((unsigned char *)page_address(page) +
				   (frag->page_offset +
				    ((mhdp_header_len - skb_headlen(skb)) +
				     offset)));
			if ((ip_ver >> 4) != VER_IPv4 &&
			    (ip_ver >> 4) != VER_IPv6)
				goto error;
		} else {
			DPRINTK("Error in the data received");
			goto error;
		}

		skb_reset_network_header(newskb);

		/* IPv6 Support - Check the IP version */
		/* and set ETH_P_IP or ETH_P_IPv6 for received packets */

		newskb->protocol = htons(ETH_IP_TYPE(ip_ver));
		newskb->ip_summed = CHECKSUM_NONE;
		newskb->pkt_type = PACKET_HOST;

		rcu_read_lock();
		if (!mhdp_is_filtered(mhdp_net_dev(dev), newskb)) {

			skb_tunnel_rx(newskb, dev, dev_net(dev));

			tunnel = mhdp_locate_tunnel(mhdp_net_dev(dev), pdn_id);
			if (tunnel) {
				struct net_device_stats *stats =
				    &tunnel->dev->stats;
				stats->rx_packets++;
				stats->rx_bytes += newskb->len;
				newskb->dev = tunnel->dev;
				SKBPRINT(newskb, "NEWSKB: RX");

#ifdef MHDP_USE_NAPI
				netif_receive_skb(newskb);
#else
				netif_rx(newskb);
#endif /*#ifdef MHDP_USE_NAPI */
			}
		}
		rcu_read_unlock();
	}

	kfree(p_mhdp_hdr_tmp);

	dev_kfree_skb(skb);

	return err;

error:
	kfree(p_mhdp_hdr_tmp);

	EPRINTK("%s - error detected\n", __func__);

	dev_kfree_skb(skb);

	if (newskb)
		dev_kfree_skb(newskb);

	return err;
}

#ifdef MHDP_USE_NAPI
/*
static int mhdp_poll(struct napi_struct *napi, int budget)
function called through napi to read current ip frame received
*/
static int mhdp_poll(struct napi_struct *napi, int budget)
{
	struct mhdp_net *mhdpn = container_of(napi, struct mhdp_net, napi);
	int err = 0;
	struct sk_buff *skb;

	while (!skb_queue_empty(&mhdpn->skb_list)) {

		skb = skb_dequeue(&mhdpn->skb_list);
		err = mhdp_netdev_rx(skb, mhdpn->dev);
	}

	napi_complete(napi);

	return err;
}

/*l2mux callback*/
static int mhdp_netdev_rx_napi(struct sk_buff *skb, struct net_device *dev)
{
	struct mhdp_net *mhdpn = mhdp_net_dev(dev);

	if (mhdpn) {

		mhdpn->dev = dev;
		skb_queue_tail(&mhdpn->skb_list, skb);

		napi_schedule(&mhdpn->napi);

	} else {
		EPRINTK("mhdp_netdev_rx_napi-MHDP driver init not correct\n");
	}

	return 0;
}

#endif /*MHDP_USE_NAPI */

/**
 * tx_timer_timeout - Timer expiration function for TX packet concatenation
  => will then call mhdp_submit_queued_skb to pass concatenated packets to IPC
 */
static enum hrtimer_restart tx_timer_timeout(struct hrtimer *timer)
{
	struct mhdp_tunnel *tunnel = container_of(timer,
						  struct mhdp_tunnel,
						  tx_timer);

	tasklet_hi_schedule(&tunnel->taskl);

	return HRTIMER_NORESTART;
}

static void tx_timer_timeout_tasklet(unsigned long arg)
{
	struct mhdp_tunnel *tunnel = (struct mhdp_tunnel *)arg;

	spin_lock_bh(&tunnel->timer_lock);

	mhdp_submit_queued_skb(tunnel, 1);

	spin_unlock_bh(&tunnel->timer_lock);
}

static int mhdp_netdev_xmit_single(struct sk_buff *skb, struct net_device *dev)
{
	struct mhdp_hdr *p_mhdphdr;
	struct mhdp_tunnel *tunnel = netdev_priv(dev);
	uint32_t pkt_len = skb->len;

	skb_push(skb, sizeof(uint32_t) + sizeof(struct packet_info));
	memset(skb->data, 0, sizeof(uint32_t) + sizeof(struct packet_info));
	p_mhdphdr = (struct mhdp_hdr *)skb->data;
	p_mhdphdr->packet_count = cpu_to_le32(1);
	p_mhdphdr->info[0].pdn_id = cpu_to_le32(tunnel->pdn_id);
	p_mhdphdr->info[0].packet_length = cpu_to_le32(pkt_len);
	spin_lock_bh(&tunnel->timer_lock);
	tunnel->skb = skb;

	mhdp_submit_queued_skb(tunnel, 1);
	spin_unlock_bh(&tunnel->timer_lock);
	return NETDEV_TX_OK;
}

/* mhdp_netdev_xmit_chain
 * if TX packet doezn't fit in max MHDP frame length, send previous
 * MHDP frame asap else concatenate TX packet.
 * If nb concatenated packets reach max MHDP packets, send current
 * MHDP frame asap else start TX timer (if no further packets
 * to be transmitted, MHDP frame will be send on timer expiry) */
static int mhdp_netdev_xmit_chain(struct sk_buff *skb, struct net_device *dev)
{
	struct mhdp_hdr *p_mhdp_hdr;
	struct mhdp_tunnel *tunnel = netdev_priv(dev);
	struct net_device_stats *stats = &tunnel->dev->stats;
	struct page *page = NULL;
	int i;
	int packet_count, offset, len;

#ifdef SUPPORT_WAKE_LOCK
	mhdp_check_wake_lock(dev);
#endif

	spin_lock_bh(&tunnel->timer_lock);

	SKBPRINT(skb, "SKB: TX");

#if 0
	{
		int i;
		int len = skb->len;
		u8 *ptr = skb->data;

		for (i = 0; i < len; i++) {
			if (i % 8 == 0)
				pr_debug("MHDP mhdp_netdev_xmit:TX [%04X] ", i);
			pr_debug(" 0x%02X", ptr[i]);
			if (i % 8 == 7 || i == len - 1)
				pr_debug("\n");
		}
	}
#endif
xmit_again:

	if (tunnel->skb == NULL) {

		tunnel->skb = netdev_alloc_skb(dev,
					       L2MUX_HDR_SIZE +
					       sizeof(struct mhdp_hdr));

		if (!tunnel->skb) {
			EPRINTK("mhdp_netdev_xmit error1");
			goto tx_error;
		}

		/* Place holder for the mhdp packet count */
		len = skb_headroom(tunnel->skb) - L2MUX_HDR_SIZE;

		skb_push(tunnel->skb, len);
		len -= 4;

		memset(tunnel->skb->data, 0, len);

		/*
		 * Need to replace following logic, with something better like
		 * __pskb_pull_tail or pskb_may_pull(tunnel->skb, len);
		 */
		{
			tunnel->skb->tail -= len;
			tunnel->skb->len -= len;
		}

		p_mhdp_hdr = (struct mhdp_hdr *)tunnel->skb->data;
		p_mhdp_hdr->packet_count = 0;

		hrtimer_start(&tunnel->tx_timer,
			      ktime_set(0, NSEC_PER_SEC / 600),
			      HRTIMER_MODE_REL);
	}

	/* This new frame is to big for the current mhdp frame, */
	/* send the frame first */
	if (tunnel->skb->len + skb->len >= MAX_MHDP_FRAME_SIZE) {

		mhdp_submit_queued_skb(tunnel, 1);

		goto xmit_again;

	} else {

		/*
		 * skb_put cannot be called as the (data_len != 0)
		 */

		tunnel->skb->tail += sizeof(struct packet_info);
		tunnel->skb->len += sizeof(struct packet_info);

		DPRINTK("new - skb->tail:%lu skb->end:%lu skb->data_len:%lu",
			(unsigned long)tunnel->skb->tail,
			(unsigned long)tunnel->skb->end,
			(unsigned long)tunnel->skb->data_len);

		p_mhdp_hdr = (struct mhdp_hdr *)tunnel->skb->data;

		tunnel->skb_to_free[le32_to_cpu(p_mhdp_hdr->packet_count)] = skb;

		packet_count = le32_to_cpu(p_mhdp_hdr->packet_count);
		p_mhdp_hdr->info[packet_count].pdn_id = cpu_to_le32(tunnel->pdn_id);
		if (packet_count == 0) {
			p_mhdp_hdr->info[packet_count].packet_offset = 0;
		} else {
			p_mhdp_hdr->info[packet_count].packet_offset =
				cpu_to_le32(
					le32_to_cpu(p_mhdp_hdr->info[packet_count - 1].packet_offset) +
					le32_to_cpu(p_mhdp_hdr->info[packet_count - 1].packet_length));
		}

		p_mhdp_hdr->info[packet_count].packet_length = cpu_to_le32(skb->len);
		p_mhdp_hdr->packet_count = cpu_to_le32(le32_to_cpu(p_mhdp_hdr->packet_count) + 1);

		page = virt_to_page(skb->data);

		get_page(page);

		offset = ((unsigned long)skb->data -
			  (unsigned long)page_address(page));

		skb_add_rx_frag(tunnel->skb, skb_shinfo(tunnel->skb)->nr_frags,
				page, offset, skb_headlen(skb),
				skb_headlen(skb));

		if (skb_shinfo(skb)->nr_frags) {

			for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {

				skb_frag_t *frag =
				    &skb_shinfo(tunnel->skb)->frags[i];

				get_page(skb_frag_page(frag));

				skb_add_rx_frag(tunnel->skb,
						skb_shinfo(tunnel->skb)->
						nr_frags, skb_frag_page(frag),
						frag->page_offset, frag->size,
						frag->size);
			}
		}

		if (le32_to_cpu(p_mhdp_hdr->packet_count) >= MAX_MHDPHDR_SIZE)
			mhdp_submit_queued_skb(tunnel, 1);
	}

	spin_unlock_bh(&tunnel->timer_lock);
	return NETDEV_TX_OK;

tx_error:
	spin_unlock_bh(&tunnel->timer_lock);
	stats->tx_errors++;
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

/* mhdp_netdev_xmit - Hard xmit for MHDP tunnel net device.
 * If master device supports MHDP chain, it will use mhdp_netdev_xmit_chain.
 * otherwise, it will use mhdp_net_xmit_single */

/* Also defined in  bcm_lte_pcie.h, for net_dev->select_queue => skb->queue_mapping */
#define BCM_LTE_TX_DATAQ	0
#define BCM_LTE_TX_CTRLQ	1
#define BCM_LTE_TX_INVALID	0xffff

static int mhdp_netdev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct mhdp_tunnel *tunnel = netdev_priv(dev);

	BUG_ON(!tunnel->master_dev);

#ifdef CONFIG_BLOG
	if (skb_get_queue_mapping(skb) == BCM_LTE_TX_DATAQ) {
		struct sk_buff *orig_skb = skb;
		skb = nbuff_xlate((pNBuff_t )skb);
		if (skb == NULL) {
			nbuff_free((pNBuff_t) orig_skb);
			printk("drop packet as nbuff_xlate fail, dev_name: %s\n",
					dev->name);
			return 0;
		}

		blog_emit(skb, dev, TYPE_IP, 0, BLOG_LTEPHY);
	}
#endif /* CONFIG_BLOG */

	if (tunnel->master_dev->features & NETIF_F_SG)
		return mhdp_netdev_xmit_chain(skb, dev);
	else
		return mhdp_netdev_xmit_single(skb, dev);
}

struct net_device *mhdp_get_netdev_by_pdn_id(struct net_device *dev, int pdn_id)
{
	struct mhdp_tunnel *tunnel;
	tunnel = mhdp_locate_tunnel(mhdp_net_dev(dev), pdn_id);
	if (tunnel == NULL)
		return NULL;
	else
		return tunnel->dev;
}
EXPORT_SYMBOL(mhdp_get_netdev_by_pdn_id);

/**
 * mhdp_netdev_event -  Catch MHDP tunnel net dev states
 */
static int
mhdp_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *event_dev = (struct net_device *)ptr;

	DPRINTK("event_dev: %s, event: %lx\n",
		event_dev ? event_dev->name : "None", event);

	switch (event) {
	case NETDEV_UNREGISTER:
		{
			struct mhdp_net *mhdpn = mhdp_net_dev(event_dev);
			struct mhdp_tunnel *iter, *prev;

			DPRINTK("event_dev: %s, event: %lx\n",
				event_dev ? event_dev->name : "None", event);

			for (iter = mhdpn->tunnels, prev = NULL;
			     iter; prev = iter, iter = iter->next) {
				if (event_dev == iter->master_dev) {
					if (!prev)
						mhdpn->tunnels =
						    mhdpn->tunnels->next;
					else
						prev->next = iter->next;
					mhdp_tunnel_destroy(iter->dev);
				}
			}
		}
		break;
	}

	return NOTIFY_DONE;
}

#ifdef MHDP_BONDING_SUPPORT

static void cdma_netdev_uninit(struct net_device *dev)
{
	dev_put(dev);
}

static void mhdp_ethtool_get_drvinfo(struct net_device *dev,
				     struct ethtool_drvinfo *drvinfo)
{
	strncpy(drvinfo->driver, dev->name, 32);
}

static const struct ethtool_ops mhdp_ethtool_ops = {
	.get_drvinfo = mhdp_ethtool_get_drvinfo,
	.get_link = ethtool_op_get_link,
};
#endif /*MHDP_BONDING_SUPPORT */

static const struct net_device_ops mhdp_netdev_ops = {
	.ndo_uninit = mhdp_netdev_uninit,
	.ndo_start_xmit = mhdp_netdev_xmit,
	.ndo_do_ioctl = mhdp_netdev_ioctl,
	.ndo_change_mtu = mhdp_netdev_change_mtu,
};

/**
 * mhdp_netdev_setup -  Setup MHDP tunnel
 */
static void mhdp_netdev_setup(struct net_device *dev)
{
	dev->netdev_ops = &mhdp_netdev_ops;
#ifdef MHDP_BONDING_SUPPORT
	dev->ethtool_ops = &mhdp_ethtool_ops;
#endif /*MHDP_BONDING_SUPPORT */

	dev->destructor = free_netdev;

#ifdef MHDP_BONDING_SUPPORT
	ether_setup(dev);
	dev->flags |= IFF_NOARP;
	dev->iflink = 0;
	dev->features |= (ETIF_F_NETNS_LOCAL | NETIF_F_SG);
#else
	dev->type = ARPHRD_TUNNEL;
	dev->hard_header_len = L2MUX_HDR_SIZE + sizeof(struct mhdp_hdr);
	dev->mtu = ETH_DATA_LEN;
	dev->flags = IFF_NOARP;
	dev->group = 0;
	dev->addr_len = 4;
	dev->features |= (NETIF_F_NETNS_LOCAL);	/* temporary removing NETIF_F_SG
						 * support due to problem with
						 * skb gets freed before being
						 * transmitted */
#endif /* MHDP_BONDING_SUPPORT */

}

/**
 * mhdp_init_net -  Initalize MHDP net structure
 */
static int __net_init mhdp_init_net(struct net *net)
{
	struct mhdp_net *mhdpn = net_generic(net, mhdp_net_id);
	int err;

	mhdpn->tunnels = NULL;

	mhdpn->ctl_dev = alloc_netdev(sizeof(struct mhdp_tunnel),
				      MHDP_CTL_IFNAME, NET_NAME_UNKNOWN,
				      mhdp_netdev_setup);
	if (!mhdpn->ctl_dev)
		return -ENOMEM;

	dev_net_set(mhdpn->ctl_dev, net);
	dev_hold(mhdpn->ctl_dev);

	err = register_netdev(mhdpn->ctl_dev);
	if (err) {
		pr_err(MHDP_CTL_IFNAME " register failed");
		free_netdev(mhdpn->ctl_dev);
		return err;
	}
	spin_lock_init(&mhdpn->udp_lock);

	mhdp_reset_udp_filter(mhdpn);
#ifdef MHDP_USE_NAPI

	netif_napi_add(mhdpn->ctl_dev, &mhdpn->napi, mhdp_poll, NAPI_WEIGHT);
	napi_enable(&mhdpn->napi);
	skb_queue_head_init(&mhdpn->skb_list);

#endif /*#ifdef MHDP_USE_NAPI */

	dev_set_drvdata(&mhdpn->ctl_dev->dev, mhdpn);
#ifdef SUPPORT_WAKE_LOCK
	err = device_create_file(&mhdpn->ctl_dev->dev, &mhdpwl_dev_attrs[0]);

	if (err)
		pr_err("MHDP cannot create wakelock file");

	mhdpn->wake_lock_time = 0;

	wake_lock_init(&mhdpn->wakelock, WAKE_LOCK_SUSPEND, "mhdp_wake_lock");
#endif

	return 0;
}

/**
 * mhdp_exit_net -  destroy MHDP net structure
 */
static void __net_exit mhdp_exit_net(struct net *net)
{
	struct mhdp_net *mhdpn = net_generic(net, mhdp_net_id);

	rtnl_lock();
	mhdp_destroy_tunnels(mhdpn);
	unregister_netdevice(mhdpn->ctl_dev);
#ifdef SUPPORT_WAKE_LOCK
	device_remove_file(&mhdpn->ctl_dev->dev, &mhdpwl_dev_attrs[0]);
	wake_lock_destroy(&mhdpn->wakelock);
#endif

	rtnl_unlock();
}

static struct pernet_operations mhdp_net_ops = {
	.init = mhdp_init_net,
	.exit = mhdp_exit_net,
	.id = &mhdp_net_id,
	.size = sizeof(struct mhdp_net),
};

/**
 * mhdp_init -  Initalize MHDP
 */
static int __init mhdp_init(void)
{
	int err;

#ifdef MHDP_USE_NAPI
	err = l2mux_netif_rx_register(MHI_L3_MHDP_DL, mhdp_netdev_rx_napi);
	err = l2mux_netif_rx_register(MHI_L3_MHDP_DL_PS2, mhdp_netdev_rx_napi);
#else
	err = l2mux_netif_rx_register(MHI_L3_MHDP_DL, mhdp_netdev_rx);
	err = l2mux_netif_rx_register(MHI_L3_MHDP_DL_PS2, mhdp_netdev_rx);

#endif /*MHDP_USE_NAPI */
	if (err)
		goto rollback0;

	err = register_pernet_device(&mhdp_net_ops);
	if (err < 0)
		goto rollback1;

	err = register_netdevice_notifier(&mhdp_netdev_notifier);
	if (err < 0)
		goto rollback2;

	return 0;

rollback2:
	unregister_pernet_device(&mhdp_net_ops);
rollback1:
	l2mux_netif_rx_unregister(MHI_L3_MHDP_DL_PS2);
	l2mux_netif_rx_unregister(MHI_L3_MHDP_DL);
rollback0:
	return err;
}

static void __exit mhdp_exit(void)
{
	l2mux_netif_rx_unregister(MHI_L3_MHDP_DL_PS2);
	l2mux_netif_rx_unregister(MHI_L3_MHDP_DL);
	unregister_netdevice_notifier(&mhdp_netdev_notifier);
	unregister_pernet_device(&mhdp_net_ops);
}

module_init(mhdp_init);
module_exit(mhdp_exit);

MODULE_DESCRIPTION("Modem Host Data Protocol for MHI");
#endif /* CONFIG_BCM_KF_MHI */
