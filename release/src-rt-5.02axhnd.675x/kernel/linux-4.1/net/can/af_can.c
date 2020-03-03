/*
 * af_can.c - Protocol family CAN core module
 *            (used by different CAN protocol modules)
 *
 * Copyright (c) 2002-2007 Volkswagen Group Electronic Research
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Volkswagen nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * Alternatively, provided that this notice is retained in full, this
 * software may be distributed under the terms of the GNU General
 * Public License ("GPL") version 2, in which case the provisions of the
 * GPL apply INSTEAD OF those given above.
 *
 * The provided data structures and external interfaces from this code
 * are not restricted to be used by modules with a GPL compatible license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 */

#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/uaccess.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/socket.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/skbuff.h>
#include <linux/can.h>
#include <linux/can/core.h>
#include <linux/can/skb.h>
#include <linux/ratelimit.h>
#include <net/net_namespace.h>
#include <net/sock.h>

#include "af_can.h"

MODULE_DESCRIPTION("Controller Area Network PF_CAN core");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Urs Thuermann <urs.thuermann@volkswagen.de>, "
	      "Oliver Hartkopp <oliver.hartkopp@volkswagen.de>");

MODULE_ALIAS_NETPROTO(PF_CAN);

static int stats_timer __read_mostly = 1;
module_param(stats_timer, int, S_IRUGO);
MODULE_PARM_DESC(stats_timer, "enable timer for statistics (default:on)");

/* receive filters subscribed for 'all' CAN devices */
struct dev_rcv_lists can_rx_alldev_list;
static DEFINE_SPINLOCK(can_rcvlists_lock);

static struct kmem_cache *rcv_cache __read_mostly;

/* table of registered CAN protocols */
static const struct can_proto *proto_tab[CAN_NPROTO] __read_mostly;
static DEFINE_MUTEX(proto_tab_lock);

struct timer_list can_stattimer;   /* timer for statistics update */
struct s_stats    can_stats;       /* packet statistics */
struct s_pstats   can_pstats;      /* receive list statistics */

static atomic_t skbcounter = ATOMIC_INIT(0);

/*
 * af_can socket functions
 */

int can_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;

	switch (cmd) {

	case SIOCGSTAMP:
		return sock_get_timestamp(sk, (struct timeval __user *)arg);

	default:
		return -ENOIOCTLCMD;
	}
}
EXPORT_SYMBOL(can_ioctl);

static void can_sock_destruct(struct sock *sk)
{
	skb_queue_purge(&sk->sk_receive_queue);
}

static const struct can_proto *can_get_proto(int protocol)
{
	const struct can_proto *cp;

	rcu_read_lock();
	cp = rcu_dereference(proto_tab[protocol]);
	if (cp && !try_module_get(cp->prot->owner))
		cp = NULL;
	rcu_read_unlock();

	return cp;
}

static inline void can_put_proto(const struct can_proto *cp)
{
	module_put(cp->prot->owner);
}

static int can_create(struct net *net, struct socket *sock, int protocol,
		      int kern)
{
	struct sock *sk;
	const struct can_proto *cp;
	int err = 0;

	sock->state = SS_UNCONNECTED;

	if (protocol < 0 || protocol >= CAN_NPROTO)
		return -EINVAL;

	if (!net_eq(net, &init_net))
		return -EAFNOSUPPORT;

	cp = can_get_proto(protocol);

#ifdef CONFIG_MODULES
	if (!cp) {
		/* try to load protocol module if kernel is modular */

		err = request_module("can-proto-%d", protocol);

		/*
		 * In case of error we only print a message but don't
		 * return the error code immediately.  Below we will
		 * return -EPROTONOSUPPORT
		 */
		if (err)
			printk_ratelimited(KERN_ERR "can: request_module "
			       "(can-proto-%d) failed.\n", protocol);

		cp = can_get_proto(protocol);
	}
#endif

	/* check for available protocol and correct usage */

	if (!cp)
		return -EPROTONOSUPPORT;

	if (cp->type != sock->type) {
		err = -EPROTOTYPE;
		goto errout;
	}

	sock->ops = cp->ops;

	sk = sk_alloc(net, PF_CAN, GFP_KERNEL, cp->prot);
	if (!sk) {
		err = -ENOMEM;
		goto errout;
	}

	sock_init_data(sock, sk);
	sk->sk_destruct = can_sock_destruct;

	if (sk->sk_prot->init)
		err = sk->sk_prot->init(sk);

	if (err) {
		/* release sk on errors */
		sock_orphan(sk);
		sock_put(sk);
	}

 errout:
	can_put_proto(cp);
	return err;
}

/*
 * af_can tx path
 */

/**
 * can_send - transmit a CAN frame (optional with local loopback)
 * @skb: pointer to socket buffer with CAN frame in data section
 * @loop: loopback for listeners on local CAN sockets (recommended default!)
 *
 * Due to the loopback this routine must not be called from hardirq context.
 *
 * Return:
 *  0 on success
 *  -ENETDOWN when the selected interface is down
 *  -ENOBUFS on full driver queue (see net_xmit_errno())
 *  -ENOMEM when local loopback failed at calling skb_clone()
 *  -EPERM when trying to send on a non-CAN interface
 *  -EMSGSIZE CAN frame size is bigger than CAN interface MTU
 *  -EINVAL when the skb->data does not contain a valid CAN frame
 */
int can_send(struct sk_buff *skb, int loop)
{
	struct sk_buff *newskb = NULL;
	struct canfd_frame *cfd = (struct canfd_frame *)skb->data;
	int err = -EINVAL;

	if (skb->len == CAN_MTU) {
		skb->protocol = htons(ETH_P_CAN);
		if (unlikely(cfd->len > CAN_MAX_DLEN))
			goto inval_skb;
	} else if (skb->len == CANFD_MTU) {
		skb->protocol = htons(ETH_P_CANFD);
		if (unlikely(cfd->len > CANFD_MAX_DLEN))
			goto inval_skb;
	} else
		goto inval_skb;

	/*
	 * Make sure the CAN frame can pass the selected CAN netdevice.
	 * As structs can_frame and canfd_frame are similar, we can provide
	 * CAN FD frames to legacy CAN drivers as long as the length is <= 8
	 */
	if (unlikely(skb->len > skb->dev->mtu && cfd->len > CAN_MAX_DLEN)) {
		err = -EMSGSIZE;
		goto inval_skb;
	}

	if (unlikely(skb->dev->type != ARPHRD_CAN)) {
		err = -EPERM;
		goto inval_skb;
	}

	if (unlikely(!(skb->dev->flags & IFF_UP))) {
		err = -ENETDOWN;
		goto inval_skb;
	}

	skb->ip_summed = CHECKSUM_UNNECESSARY;

	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);

	if (loop) {
		/* local loopback of sent CAN frames */

		/* indication for the CAN driver: do loopback */
		skb->pkt_type = PACKET_LOOPBACK;

		/*
		 * The reference to the originating sock may be required
		 * by the receiving socket to check whether the frame is
		 * its own. Example: can_raw sockopt CAN_RAW_RECV_OWN_MSGS
		 * Therefore we have to ensure that skb->sk remains the
		 * reference to the originating sock by restoring skb->sk
		 * after each skb_clone() or skb_orphan() usage.
		 */

		if (!(skb->dev->flags & IFF_ECHO)) {
			/*
			 * If the interface is not capable to do loopback
			 * itself, we do it here.
			 */
			newskb = skb_clone(skb, GFP_ATOMIC);
			if (!newskb) {
				kfree_skb(skb);
				return -ENOMEM;
			}

			can_skb_set_owner(newskb, skb->sk);
			newskb->ip_summed = CHECKSUM_UNNECESSARY;
			newskb->pkt_type = PACKET_BROADCAST;
		}
	} else {
		/* indication for the CAN driver: no loopback required */
		skb->pkt_type = PACKET_HOST;
	}

	/* send to netdevice */
	err = dev_queue_xmit(skb);
	if (err > 0)
		err = net_xmit_errno(err);

	if (err) {
		kfree_skb(newskb);
		return err;
	}

	if (newskb)
		netif_rx_ni(newskb);

	/* update statistics */
	can_stats.tx_frames++;
	can_stats.tx_frames_delta++;

	return 0;

inval_skb:
	kfree_skb(skb);
	return err;
}
EXPORT_SYMBOL(can_send);

/*
 * af_can rx path
 */

static struct dev_rcv_lists *find_dev_rcv_lists(struct net_device *dev)
{
	if (!dev)
		return &can_rx_alldev_list;
	else
		return (struct dev_rcv_lists *)dev->ml_priv;
}

/**
 * effhash - hash function for 29 bit CAN identifier reduction
 * @can_id: 29 bit CAN identifier
 *
 * Description:
 *  To reduce the linear traversal in one linked list of _single_ EFF CAN
 *  frame subscriptions the 29 bit identifier is mapped to 10 bits.
 *  (see CAN_EFF_RCV_HASH_BITS definition)
 *
 * Return:
 *  Hash value from 0x000 - 0x3FF ( enforced by CAN_EFF_RCV_HASH_BITS mask )
 */
static unsigned int effhash(canid_t can_id)
{
	unsigned int hash;

	hash = can_id;
	hash ^= can_id >> CAN_EFF_RCV_HASH_BITS;
	hash ^= can_id >> (2 * CAN_EFF_RCV_HASH_BITS);

	return hash & ((1 << CAN_EFF_RCV_HASH_BITS) - 1);
}

/**
 * find_rcv_list - determine optimal filterlist inside device filter struct
 * @can_id: pointer to CAN identifier of a given can_filter
 * @mask: pointer to CAN mask of a given can_filter
 * @d: pointer to the device filter struct
 *
 * Description:
 *  Returns the optimal filterlist to reduce the filter handling in the
 *  receive path. This function is called by service functions that need
 *  to register or unregister a can_filter in the filter lists.
 *
 *  A filter matches in general, when
 *
 *          <received_can_id> & mask == can_id & mask
 *
 *  so every bit set in the mask (even CAN_EFF_FLAG, CAN_RTR_FLAG) describe
 *  relevant bits for the filter.
 *
 *  The filter can be inverted (CAN_INV_FILTER bit set in can_id) or it can
 *  filter for error messages (CAN_ERR_FLAG bit set in mask). For error msg
 *  frames there is a special filterlist and a special rx path filter handling.
 *
 * Return:
 *  Pointer to optimal filterlist for the given can_id/mask pair.
 *  Constistency checked mask.
 *  Reduced can_id to have a preprocessed filter compare value.
 */
static struct hlist_head *find_rcv_list(canid_t *can_id, canid_t *mask,
					struct dev_rcv_lists *d)
{
	canid_t inv = *can_id & CAN_INV_FILTER; /* save flag before masking */

	/* filter for error message frames in extra filterlist */
	if (*mask & CAN_ERR_FLAG) {
		/* clear CAN_ERR_FLAG in filter entry */
		*mask &= CAN_ERR_MASK;
		return &d->rx[RX_ERR];
	}

	/* with cleared CAN_ERR_FLAG we have a simple mask/value filterpair */

#define CAN_EFF_RTR_FLAGS (CAN_EFF_FLAG | CAN_RTR_FLAG)

	/* ensure valid values in can_mask for 'SFF only' frame filtering */
	if ((*mask & CAN_EFF_FLAG) && !(*can_id & CAN_EFF_FLAG))
		*mask &= (CAN_SFF_MASK | CAN_EFF_RTR_FLAGS);

	/* reduce condition testing at receive time */
	*can_id &= *mask;

	/* inverse can_id/can_mask filter */
	if (inv)
		return &d->rx[RX_INV];

	/* mask == 0 => no condition testing at receive time */
	if (!(*mask))
		return &d->rx[RX_ALL];

	/* extra filterlists for the subscription of a single non-RTR can_id */
	if (((*mask & CAN_EFF_RTR_FLAGS) == CAN_EFF_RTR_FLAGS) &&
	    !(*can_id & CAN_RTR_FLAG)) {

		if (*can_id & CAN_EFF_FLAG) {
			if (*mask == (CAN_EFF_MASK | CAN_EFF_RTR_FLAGS))
				return &d->rx_eff[effhash(*can_id)];
		} else {
			if (*mask == (CAN_SFF_MASK | CAN_EFF_RTR_FLAGS))
				return &d->rx_sff[*can_id];
		}
	}

	/* default: filter via can_id/can_mask */
	return &d->rx[RX_FIL];
}

/**
 * can_rx_register - subscribe CAN frames from a specific interface
 * @dev: pointer to netdevice (NULL => subcribe from 'all' CAN devices list)
 * @can_id: CAN identifier (see description)
 * @mask: CAN mask (see description)
 * @func: callback function on filter match
 * @data: returned parameter for callback function
 * @ident: string for calling module identification
 *
 * Description:
 *  Invokes the callback function with the received sk_buff and the given
 *  parameter 'data' on a matching receive filter. A filter matches, when
 *
 *          <received_can_id> & mask == can_id & mask
 *
 *  The filter can be inverted (CAN_INV_FILTER bit set in can_id) or it can
 *  filter for error message frames (CAN_ERR_FLAG bit set in mask).
 *
 *  The provided pointer to the sk_buff is guaranteed to be valid as long as
 *  the callback function is running. The callback function must *not* free
 *  the given sk_buff while processing it's task. When the given sk_buff is
 *  needed after the end of the callback function it must be cloned inside
 *  the callback function with skb_clone().
 *
 * Return:
 *  0 on success
 *  -ENOMEM on missing cache mem to create subscription entry
 *  -ENODEV unknown device
 */
int can_rx_register(struct net_device *dev, canid_t can_id, canid_t mask,
		    void (*func)(struct sk_buff *, void *), void *data,
		    char *ident)
{
	struct receiver *r;
	struct hlist_head *rl;
	struct dev_rcv_lists *d;
	int err = 0;

	/* insert new receiver  (dev,canid,mask) -> (func,data) */

	if (dev && dev->type != ARPHRD_CAN)
		return -ENODEV;

	r = kmem_cache_alloc(rcv_cache, GFP_KERNEL);
	if (!r)
		return -ENOMEM;

	spin_lock(&can_rcvlists_lock);

	d = find_dev_rcv_lists(dev);
	if (d) {
		rl = find_rcv_list(&can_id, &mask, d);

		r->can_id  = can_id;
		r->mask    = mask;
		r->matches = 0;
		r->func    = func;
		r->data    = data;
		r->ident   = ident;

		hlist_add_head_rcu(&r->list, rl);
		d->entries++;

		can_pstats.rcv_entries++;
		if (can_pstats.rcv_entries_max < can_pstats.rcv_entries)
			can_pstats.rcv_entries_max = can_pstats.rcv_entries;
	} else {
		kmem_cache_free(rcv_cache, r);
		err = -ENODEV;
	}

	spin_unlock(&can_rcvlists_lock);

	return err;
}
EXPORT_SYMBOL(can_rx_register);

/*
 * can_rx_delete_receiver - rcu callback for single receiver entry removal
 */
static void can_rx_delete_receiver(struct rcu_head *rp)
{
	struct receiver *r = container_of(rp, struct receiver, rcu);

	kmem_cache_free(rcv_cache, r);
}

/**
 * can_rx_unregister - unsubscribe CAN frames from a specific interface
 * @dev: pointer to netdevice (NULL => unsubscribe from 'all' CAN devices list)
 * @can_id: CAN identifier
 * @mask: CAN mask
 * @func: callback function on filter match
 * @data: returned parameter for callback function
 *
 * Description:
 *  Removes subscription entry depending on given (subscription) values.
 */
void can_rx_unregister(struct net_device *dev, canid_t can_id, canid_t mask,
		       void (*func)(struct sk_buff *, void *), void *data)
{
	struct receiver *r = NULL;
	struct hlist_head *rl;
	struct dev_rcv_lists *d;

	if (dev && dev->type != ARPHRD_CAN)
		return;

	spin_lock(&can_rcvlists_lock);

	d = find_dev_rcv_lists(dev);
	if (!d) {
		pr_err("BUG: receive list not found for "
		       "dev %s, id %03X, mask %03X\n",
		       DNAME(dev), can_id, mask);
		goto out;
	}

	rl = find_rcv_list(&can_id, &mask, d);

	/*
	 * Search the receiver list for the item to delete.  This should
	 * exist, since no receiver may be unregistered that hasn't
	 * been registered before.
	 */

	hlist_for_each_entry_rcu(r, rl, list) {
		if (r->can_id == can_id && r->mask == mask &&
		    r->func == func && r->data == data)
			break;
	}

	/*
	 * Check for bugs in CAN protocol implementations using af_can.c:
	 * 'r' will be NULL if no matching list item was found for removal.
	 */

	if (!r) {
		WARN(1, "BUG: receive list entry not found for dev %s, "
		     "id %03X, mask %03X\n", DNAME(dev), can_id, mask);
		goto out;
	}

	hlist_del_rcu(&r->list);
	d->entries--;

	if (can_pstats.rcv_entries > 0)
		can_pstats.rcv_entries--;

	/* remove device structure requested by NETDEV_UNREGISTER */
	if (d->remove_on_zero_entries && !d->entries) {
		kfree(d);
		dev->ml_priv = NULL;
	}

 out:
	spin_unlock(&can_rcvlists_lock);

	/* schedule the receiver item for deletion */
	if (r)
		call_rcu(&r->rcu, can_rx_delete_receiver);
}
EXPORT_SYMBOL(can_rx_unregister);

static inline void deliver(struct sk_buff *skb, struct receiver *r)
{
	r->func(skb, r->data);
	r->matches++;
}

static int can_rcv_filter(struct dev_rcv_lists *d, struct sk_buff *skb)
{
	struct receiver *r;
	int matches = 0;
	struct can_frame *cf = (struct can_frame *)skb->data;
	canid_t can_id = cf->can_id;

	if (d->entries == 0)
		return 0;

	if (can_id & CAN_ERR_FLAG) {
		/* check for error message frame entries only */
		hlist_for_each_entry_rcu(r, &d->rx[RX_ERR], list) {
			if (can_id & r->mask) {
				deliver(skb, r);
				matches++;
			}
		}
		return matches;
	}

	/* check for unfiltered entries */
	hlist_for_each_entry_rcu(r, &d->rx[RX_ALL], list) {
		deliver(skb, r);
		matches++;
	}

	/* check for can_id/mask entries */
	hlist_for_each_entry_rcu(r, &d->rx[RX_FIL], list) {
		if ((can_id & r->mask) == r->can_id) {
			deliver(skb, r);
			matches++;
		}
	}

	/* check for inverted can_id/mask entries */
	hlist_for_each_entry_rcu(r, &d->rx[RX_INV], list) {
		if ((can_id & r->mask) != r->can_id) {
			deliver(skb, r);
			matches++;
		}
	}

	/* check filterlists for single non-RTR can_ids */
	if (can_id & CAN_RTR_FLAG)
		return matches;

	if (can_id & CAN_EFF_FLAG) {
		hlist_for_each_entry_rcu(r, &d->rx_eff[effhash(can_id)], list) {
			if (r->can_id == can_id) {
				deliver(skb, r);
				matches++;
			}
		}
	} else {
		can_id &= CAN_SFF_MASK;
		hlist_for_each_entry_rcu(r, &d->rx_sff[can_id], list) {
			deliver(skb, r);
			matches++;
		}
	}

	return matches;
}

static void can_receive(struct sk_buff *skb, struct net_device *dev)
{
	struct dev_rcv_lists *d;
	int matches;

	/* update statistics */
	can_stats.rx_frames++;
	can_stats.rx_frames_delta++;

	/* create non-zero unique skb identifier together with *skb */
	while (!(can_skb_prv(skb)->skbcnt))
		can_skb_prv(skb)->skbcnt = atomic_inc_return(&skbcounter);

	rcu_read_lock();

	/* deliver the packet to sockets listening on all devices */
	matches = can_rcv_filter(&can_rx_alldev_list, skb);

	/* find receive list for this device */
	d = find_dev_rcv_lists(dev);
	if (d)
		matches += can_rcv_filter(d, skb);

	rcu_read_unlock();

	/* consume the skbuff allocated by the netdevice driver */
	consume_skb(skb);

	if (matches > 0) {
		can_stats.matches++;
		can_stats.matches_delta++;
	}
}

static int can_rcv(struct sk_buff *skb, struct net_device *dev,
		   struct packet_type *pt, struct net_device *orig_dev)
{
	struct canfd_frame *cfd = (struct canfd_frame *)skb->data;

	if (unlikely(!net_eq(dev_net(dev), &init_net)))
		goto drop;

	if (unlikely(dev->type != ARPHRD_CAN || skb->len != CAN_MTU ||
		     cfd->len > CAN_MAX_DLEN)) {
		pr_warn_once("PF_CAN: dropped non conform CAN skbuf: dev type %d, len %d, datalen %d\n",
			     dev->type, skb->len, cfd->len);
		goto drop;
	}

	can_receive(skb, dev);
	return NET_RX_SUCCESS;

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int canfd_rcv(struct sk_buff *skb, struct net_device *dev,
		   struct packet_type *pt, struct net_device *orig_dev)
{
	struct canfd_frame *cfd = (struct canfd_frame *)skb->data;

	if (unlikely(!net_eq(dev_net(dev), &init_net)))
		goto drop;

	if (unlikely(dev->type != ARPHRD_CAN || skb->len != CANFD_MTU ||
		     cfd->len > CANFD_MAX_DLEN)) {
		pr_warn_once("PF_CAN: dropped non conform CAN FD skbuf: dev type %d, len %d, datalen %d\n",
			     dev->type, skb->len, cfd->len);
		goto drop;
	}

	can_receive(skb, dev);
	return NET_RX_SUCCESS;

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

/*
 * af_can protocol functions
 */

/**
 * can_proto_register - register CAN transport protocol
 * @cp: pointer to CAN protocol structure
 *
 * Return:
 *  0 on success
 *  -EINVAL invalid (out of range) protocol number
 *  -EBUSY  protocol already in use
 *  -ENOBUF if proto_register() fails
 */
int can_proto_register(const struct can_proto *cp)
{
	int proto = cp->protocol;
	int err = 0;

	if (proto < 0 || proto >= CAN_NPROTO) {
		pr_err("can: protocol number %d out of range\n", proto);
		return -EINVAL;
	}

	err = proto_register(cp->prot, 0);
	if (err < 0)
		return err;

	mutex_lock(&proto_tab_lock);

	if (proto_tab[proto]) {
		pr_err("can: protocol %d already registered\n", proto);
		err = -EBUSY;
	} else
		RCU_INIT_POINTER(proto_tab[proto], cp);

	mutex_unlock(&proto_tab_lock);

	if (err < 0)
		proto_unregister(cp->prot);

	return err;
}
EXPORT_SYMBOL(can_proto_register);

/**
 * can_proto_unregister - unregister CAN transport protocol
 * @cp: pointer to CAN protocol structure
 */
void can_proto_unregister(const struct can_proto *cp)
{
	int proto = cp->protocol;

	mutex_lock(&proto_tab_lock);
	BUG_ON(proto_tab[proto] != cp);
	RCU_INIT_POINTER(proto_tab[proto], NULL);
	mutex_unlock(&proto_tab_lock);

	synchronize_rcu();

	proto_unregister(cp->prot);
}
EXPORT_SYMBOL(can_proto_unregister);

/*
 * af_can notifier to create/remove CAN netdevice specific structs
 */
static int can_notifier(struct notifier_block *nb, unsigned long msg,
			void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct dev_rcv_lists *d;

	if (!net_eq(dev_net(dev), &init_net))
		return NOTIFY_DONE;

	if (dev->type != ARPHRD_CAN)
		return NOTIFY_DONE;

	switch (msg) {

	case NETDEV_REGISTER:

		/* create new dev_rcv_lists for this device */
		d = kzalloc(sizeof(*d), GFP_KERNEL);
		if (!d)
			return NOTIFY_DONE;
		BUG_ON(dev->ml_priv);
		dev->ml_priv = d;

		break;

	case NETDEV_UNREGISTER:
		spin_lock(&can_rcvlists_lock);

		d = dev->ml_priv;
		if (d) {
			if (d->entries)
				d->remove_on_zero_entries = 1;
			else {
				kfree(d);
				dev->ml_priv = NULL;
			}
		} else
			pr_err("can: notifier: receive list not found for dev "
			       "%s\n", dev->name);

		spin_unlock(&can_rcvlists_lock);

		break;
	}

	return NOTIFY_DONE;
}

/*
 * af_can module init/exit functions
 */

static struct packet_type can_packet __read_mostly = {
	.type = cpu_to_be16(ETH_P_CAN),
	.func = can_rcv,
};

static struct packet_type canfd_packet __read_mostly = {
	.type = cpu_to_be16(ETH_P_CANFD),
	.func = canfd_rcv,
};

static const struct net_proto_family can_family_ops = {
	.family = PF_CAN,
	.create = can_create,
	.owner  = THIS_MODULE,
};

/* notifier block for netdevice event */
static struct notifier_block can_netdev_notifier __read_mostly = {
	.notifier_call = can_notifier,
};

static __init int can_init(void)
{
	/* check for correct padding to be able to use the structs similarly */
	BUILD_BUG_ON(offsetof(struct can_frame, can_dlc) !=
		     offsetof(struct canfd_frame, len) ||
		     offsetof(struct can_frame, data) !=
		     offsetof(struct canfd_frame, data));

	pr_info("can: controller area network core (" CAN_VERSION_STRING ")\n");

	memset(&can_rx_alldev_list, 0, sizeof(can_rx_alldev_list));

	rcv_cache = kmem_cache_create("can_receiver", sizeof(struct receiver),
				      0, 0, NULL);
	if (!rcv_cache)
		return -ENOMEM;

	if (stats_timer) {
		/* the statistics are updated every second (timer triggered) */
		setup_timer(&can_stattimer, can_stat_update, 0);
		mod_timer(&can_stattimer, round_jiffies(jiffies + HZ));
	} else
		can_stattimer.function = NULL;

	can_init_proc();

	/* protocol register */
	sock_register(&can_family_ops);
	register_netdevice_notifier(&can_netdev_notifier);
	dev_add_pack(&can_packet);
	dev_add_pack(&canfd_packet);

	return 0;
}

static __exit void can_exit(void)
{
	struct net_device *dev;

	if (stats_timer)
		del_timer_sync(&can_stattimer);

	can_remove_proc();

	/* protocol unregister */
	dev_remove_pack(&canfd_packet);
	dev_remove_pack(&can_packet);
	unregister_netdevice_notifier(&can_netdev_notifier);
	sock_unregister(PF_CAN);

	/* remove created dev_rcv_lists from still registered CAN devices */
	rcu_read_lock();
	for_each_netdev_rcu(&init_net, dev) {
		if (dev->type == ARPHRD_CAN && dev->ml_priv) {

			struct dev_rcv_lists *d = dev->ml_priv;

			BUG_ON(d->entries);
			kfree(d);
			dev->ml_priv = NULL;
		}
	}
	rcu_read_unlock();

	rcu_barrier(); /* Wait for completion of call_rcu()'s */

	kmem_cache_destroy(rcv_cache);
}

module_init(can_init);
module_exit(can_exit);
