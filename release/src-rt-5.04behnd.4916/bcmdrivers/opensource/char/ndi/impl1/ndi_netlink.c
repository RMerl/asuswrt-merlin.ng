/*
 * <:copyright-BRCM:2020:DUAL/GPL:standard 
 * 
 *    Copyright (c) 2020 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <linux/bcm_netlink.h>
#include <linux/ndi.h>
#include "ndi_local.h"

/* --- function declarations --- */
static void ndi_dev_nl_process_events(struct work_struct *w);

/* --- variables --- */
struct ndi_nl_msg_hdlr {
	int	(*action)(struct sk_buff *, struct nlmsghdr *,
			  struct netlink_ext_ack *);
	int	(*dump)(struct sk_buff *, struct netlink_callback *);
};

struct ndi_event {
	int			type;
	struct ndi_dev		*dev;
	struct list_head	node;
};
static struct kmem_cache *event_cache;
static DECLARE_WORK(event_work, ndi_dev_nl_process_events);
static LIST_HEAD(events);

/* --- functions --- */
#define ndia_fill_object(skb, portid, seq, event, cb, arg) \
	__ndia_fill_object(skb, portid, seq, event, \
			   (int (*)(struct sk_buff *, void *))(cb), arg)

static int
__ndia_fill_object(struct sk_buff *skb, int portid, int seq, int event,
		   int (*cb)(struct sk_buff *, void *), void *arg)
{
	struct nlmsghdr *nlh;
	int err;
	unsigned int flags = portid ? NLM_F_MULTI : 0;

	nlh = nlmsg_put(skb, portid, seq, event, 0, flags);
	if (!nlh)
		goto nlmsg_failure;

	err = cb(skb, arg);
	if (err)
		goto nlmsg_failure;

	nlmsg_end(skb, nlh);
	return 0;

nlmsg_failure:
	nlmsg_cancel(skb, nlh);
	return -1;
}

static int ndia_dump_dev(struct sk_buff *skb, struct ndi_dev *dev)
{
	if (should_ignore_ndi_dev(dev))
		return 0;

	if (nla_put_u32(skb, NDIA_DEV_ID, dev->id))
		goto nla_put_failure;

	if (nla_put(skb, NDIA_DEV_IP4, sizeof(dev->ip4), &dev->ip4) ||
	    nla_put(skb, NDIA_DEV_IP6, sizeof(dev->ip6), &dev->ip6) ||
	    nla_put(skb, NDIA_DEV_MAC, sizeof(dev->mac), dev->mac))
		goto nla_put_failure;

	if (nla_put_string(skb, NDIA_DEV_HOSTNAME, dev->hostname))
		goto nla_put_failure;
	if (nla_put_u8(skb, NDIA_DEV_ONLINE, dev->state == DEV_OFFLINE ? 0 : 1))
		goto nla_put_failure;
	if (nla_put_u64_64bit(skb, NDIA_DEV_FLAGS, dev->flags,
			      NDIA_DEV_PADDING))
		goto nla_put_failure;

#if IS_ENABLED(CONFIG_BCM_DPI)
	if (nla_put_u16(skb, NDIA_DEV_DPI_VENDOR, dev->dpi.vendor) ||
	    nla_put_u16(skb, NDIA_DEV_DPI_OS, dev->dpi.os) ||
	    nla_put_u16(skb, NDIA_DEV_DPI_OS_CLASS, dev->dpi.os_class) ||
	    nla_put_u32(skb, NDIA_DEV_DPI_ID, dev->dpi.dev_id) ||
	    nla_put_u16(skb, NDIA_DEV_DPI_CATEGORY, dev->dpi.category) ||
	    nla_put_u16(skb, NDIA_DEV_DPI_FAMILY, dev->dpi.family) ||
	    nla_put_u16(skb, NDIA_DEV_DPI_PRIO, dev->dpi.prio))
		goto nla_put_failure;
#endif

	return 0;

nla_put_failure:
	return -1;
}

static int ndia_dump_devices(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct ndi_dev *e;
	struct ndi_dev *last = (struct ndi_dev *)cb->args[1];
	struct hlist_node *tmp;
	int i = cb->args[0];
	int err;

	spin_lock_bh(&lock);

	for (; i < devs_bucket_count(); i++) {
		hlist_for_each_entry_safe(e, tmp, devs_get_bucket(i), node) {
			if (cb->args[1]) {
				if (e != last)
					continue;
				cb->args[1] = 0;
			}

			if (should_ignore_ndi_dev(e))
				goto next;

			err = ndia_fill_object(skb, NETLINK_CB(cb->skb).portid,
					       cb->nlh->nlmsg_seq,
					       NDINL_NEWDEVICE,
					       ndia_dump_dev, e);
			if (err) {
				cb->args[1] = (unsigned long)e;
				if (e != last)
					atomic_inc(&e->refcount);
				goto out;
			}

next:
			if (e == last)
				atomic_dec(&e->refcount);
		}
	}

out:
	spin_unlock_bh(&lock);
	cb->args[0] = i;
	return err;
}

int ndi_dev_nl_event_locked(struct ndi_dev *dev, int type)
{
	struct ndi_event *e;

	if (should_ignore_ndi_dev(dev))
		return 0;

	e = kmem_cache_zalloc(event_cache, GFP_ATOMIC);
	if (!e) {
		pr_err("couldn't alloc ndi_event\n");
		goto err;
	}

	e->type	= type;
	e->dev	= dev;
	INIT_LIST_HEAD(&e->node);
	list_add_tail(&e->node, &events);

	schedule_work(&event_work);
	return 0;

err:
	kmem_cache_free(event_cache, e);
	return -ENOMEM;
}

int ndi_dev_nl_event(struct ndi_dev *dev, int type)
{
	int err;

	spin_lock_bh(&lock);
	err = ndi_dev_nl_event_locked(dev, type);
	spin_unlock_bh(&lock);

	return err;
}

static void ndi_dev_nl_process_events(struct work_struct *w)
{
	struct nl_bcast_entry *entry;
	struct sk_buff *skb = NULL;
	struct ndi_event *e, *tmp;
	int err = -ENOMEM;

	spin_lock_bh(&lock);

	list_for_each_entry_safe(e, tmp, &events, node) {
		list_for_each_entry(entry, &nl_bcast_list, node) {
			skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC);
			if (!skb)
				goto err;
			if (ndia_fill_object(skb, 0, 0, e->type, ndia_dump_dev,
					     e->dev)) {
				pr_debug("ndia_fill_object failed\n");
				goto nlmsg_failure;
			}

			err = nlmsg_notify(entry->socket, skb, 0, NDINLGRP_DEV,
					   0, GFP_ATOMIC);
			if (err == -ENOBUFS || err == -EAGAIN) {
				pr_debug("nlmsg_notify returned %d\n", err);
				goto nlmsg_failure;
			}
		}
		list_del(&e->node);
		kmem_cache_free(event_cache, e);
	}
	goto out;

nlmsg_failure:
	nlmsg_free(skb);
err:
	netlink_set_err(entry->socket, 0, NDINLGRP_DEV, err);
out:
	spin_unlock_bh(&lock);
}

int ndi_dev_nl_init(void)
{
	event_cache = KMEM_CACHE(ndi_event, 0);
	if (!event_cache) {
		pr_err("failed to create netlink event cache\n");
		goto err_free_event_cache;
	}

	return 0;

err_free_event_cache:
	kmem_cache_destroy(event_cache);
	return -ENOMEM;
}

void ndi_dev_nl_exit(void)
{
	struct ndi_event *e, *tmp;

	spin_lock_bh(&lock);
	list_for_each_entry_safe(e, tmp, &events, node) {
		list_del(&e->node);
		kmem_cache_free(event_cache, e);
	}
	spin_unlock_bh(&lock);

	kmem_cache_destroy(event_cache);
}

static const struct ndi_nl_msg_hdlr hdlrs[NDINL_MAX - NDINL_BASE + 1] = {
	[NDINL_GETDEVICE	- NDINL_BASE]	= {
		.dump		= &ndia_dump_devices,
	},
};

static int ndi_nl_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh,
			  struct netlink_ext_ack *extack)
{
	int type;
	int kind;

	type = nlh->nlmsg_type;
	if (type > NDINL_MAX)
		return -EOPNOTSUPP;

	type -= NDINL_BASE;
	kind = type & 0x3;

	if (kind == 2 && (nlh->nlmsg_flags & NLM_F_DUMP)) {
		struct netlink_dump_control c = {
			.dump	= hdlrs[type].dump,
		};

		if (!c.dump)
			return -EOPNOTSUPP;

		return netlink_dump_start(skb->sk, skb, nlh, &c);
	}

	if (!hdlrs[type].action)
		return -EOPNOTSUPP;

	return hdlrs[type].action(skb, nlh, extack);
}

void ndi_nl_rcv(struct sk_buff *skb)
{
	netlink_rcv_skb(skb, &ndi_nl_rcv_msg);
}
