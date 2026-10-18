// SPDX-License-Identifier: GPL-2.0-only
/*
 * Passive Optical Network Queueing discipline.
 * Author: Stefan Chulski <stefan.chulski@broadcom.com>
 *
 */
#if defined(CONFIG_BCM_KF_QDISC_PON)
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <net/pkt_sched.h>
#include <net/inet_ecn.h>
#include <net/sch_generic.h>
#include <net/pkt_cls.h>

struct tc_pon_xstats {
	__u32 packets_in;
	__u32 dropped;
	__u32 overlimit;
};

struct pon_class {
	struct Qdisc_class_common common;

	unsigned int		children;

	struct pon_class_leaf {
		struct Qdisc	*q;
	} leaf;
};

struct pon_sched {
	struct Qdisc_class_hash clhash;

	unsigned int		children;
	u32			direct_pkts;
	u32			overlimits;
};

static int pon_qdisc_enqueue(struct sk_buff *skb, struct Qdisc *sch,
			     struct sk_buff **to_free)
{
	return qdisc_drop(skb, sch, to_free);
}

static int pon_change(struct Qdisc *sch, struct nlattr *opt,
		      struct netlink_ext_ack *extack)
{
	return 0;
}

static int pon_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	return 0;
}

static int pon_dump_stats(struct Qdisc *sch, struct gnet_dump *d)
{
	struct tc_pon_xstats st = {0};

	return gnet_stats_copy_app(d, &st, sizeof(st));
}

static struct sk_buff *pon_qdisc_dequeue(struct Qdisc *sch)
{
	return NULL;
}

static void pon_reset(struct Qdisc *sch)
{
}

/* find class in global hash table using given handle */
static inline struct pon_class *pon_find(u32 handle, struct Qdisc *sch)
{
	struct pon_sched *q = qdisc_priv(sch);
	struct Qdisc_class_common *clc;

	clc = qdisc_class_find(&q->clhash, handle);
	if (!clc)
		return NULL;
	return container_of(clc, struct pon_class, common);
}

static unsigned long pon_search(struct Qdisc *sch, u32 handle)
{
	return (unsigned long)pon_find(handle, sch);
}

static int pon_offload(struct Qdisc *sch, u32 parentid, u32 classid, u8 control,
		       enum tc_pon_command command)
{
	struct tc_pon_qopt_offload qopt = {0};
	struct net_device *dev = qdisc_dev(sch);

	if (!tc_can_offload(dev) || !dev->netdev_ops->ndo_setup_tc)
		return -EINVAL;

	qopt.parent = parentid;
	qopt.handle = classid;
	qopt.command = command;
	qopt.control = control;

	return dev->netdev_ops->ndo_setup_tc(dev, TC_SETUP_QDISC_PON, &qopt);
}

static int pon_init(struct Qdisc *sch, struct nlattr *opt,
		    struct netlink_ext_ack *extack)
{
	struct pon_sched *q = qdisc_priv(sch);
	int err;

	if (sch->parent != TC_H_ROOT) {
		pr_err("PON: parent should be root\n");
		return -EINVAL;
	}

	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0)
		return err;

	sch->flags |= TCQ_F_CAN_BYPASS;

	return pon_offload(sch, sch->parent, sch->handle, 0, TC_PON_REPLACE);
}

static int pon_dump_class(struct Qdisc *sch, unsigned long arg,
			  struct sk_buff *skb, struct tcmsg *tcm)
{
	struct pon_class *cl = (struct pon_class *)arg;
	struct nlattr *nest;
	struct tc_htb_opt opt;

	tcm->tcm_parent = TC_H_ROOT;
	tcm->tcm_handle = cl->common.classid;
	if (cl->leaf.q)
		tcm->tcm_info = cl->leaf.q->handle;

	nest = nla_nest_start_noflag(skb, TCA_OPTIONS);
	if (!nest)
		goto nla_put_failure;

	memset(&opt, 0, sizeof(opt));

	if (nla_put(skb, TCA_HTB_PARMS, sizeof(opt), &opt))
		goto nla_put_failure;

	return nla_nest_end(skb, nest);

nla_put_failure:
	nla_nest_cancel(skb, nest);
	return -1;
}

static int
pon_dump_class_stats(struct Qdisc *sch, unsigned long arg, struct gnet_dump *d)
{
	struct pon_class *cl = (struct pon_class *)arg;
	struct gnet_stats_queue qs = {
		.drops = 0,
		.overlimits = 0,
	};
	__u32 qlen = 0;

	if (cl->leaf.q)
		qdisc_qstats_qlen_backlog(cl->leaf.q, &qlen, &qs.backlog);

	if (gnet_stats_copy_queue(d, NULL, &qs, qlen) < 0)
		return -1;

	return 0;
}

static struct netdev_queue *
pon_select_queue(struct Qdisc *sch, struct tcmsg *tcm)
{
	struct net_device *dev = qdisc_dev(sch);

	return netdev_get_tx_queue(dev, 0);
}

static int pon_graft(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
		     struct Qdisc **old, struct netlink_ext_ack *extack)
{
	struct netdev_queue *dev_queue = sch->dev_queue;
	struct pon_class *cl = (struct pon_class *)arg;

	if (!new) {
		new = qdisc_create_dflt(dev_queue, &pfifo_qdisc_ops,
					cl->common.classid, extack);
		if (!new)
			return -ENOBUFS;
	}
	if (cl->leaf.q->handle)
		cl->children--;
	else
		cl->children++;

	*old = qdisc_replace(sch, new, &cl->leaf.q);

	return 0;
}

static struct Qdisc *pon_leaf(struct Qdisc *sch, unsigned long arg)
{
	struct pon_class *cl = (struct pon_class *)arg;

	return cl->leaf.q;
}

static void pon_qlen_notify(struct Qdisc *sch, unsigned long arg)
{
}

static void pon_destroy_class(struct Qdisc *sch, struct pon_class *cl)
{
	struct pon_sched *q = qdisc_priv(sch);

	if (cl->children)
		pr_warn("PON: Class %d still has childrens, please remove them",
			cl->leaf.q->parent);

	pon_offload(sch, sch->handle, cl->leaf.q->parent, 0, TC_PON_DESTROY);

	WARN_ON(!cl->leaf.q);
	qdisc_put(cl->leaf.q);
	kfree(cl);

	q->children--;
}

static void pon_destroy(struct Qdisc *sch)
{
	struct pon_sched *q = qdisc_priv(sch);
	struct hlist_node *next;
	struct pon_class *cl;
	unsigned int i;

	for (i = 0; i < q->clhash.hashsize; i++) {
		hlist_for_each_entry_safe(cl, next, &q->clhash.hash[i],
					  common.hnode) {
			qdisc_class_hash_remove(&q->clhash, &cl->common);
			pon_destroy_class(sch, cl);
		}
	}

	pon_offload(sch, sch->parent, sch->handle, 0, TC_PON_DESTROY);
}

static int pon_delete(struct Qdisc *sch, unsigned long arg,
		      struct netlink_ext_ack *extack)
{
	struct pon_sched *q = qdisc_priv(sch);
	struct pon_class *cl = (struct pon_class *)arg;

	if (cl->children)
		return -EBUSY;

	sch_tree_lock(sch);

	qdisc_purge_queue(cl->leaf.q);

	/* delete from hash and active; remainder in destroy_class */
	qdisc_class_hash_remove(&q->clhash, &cl->common);

	sch_tree_unlock(sch);

	pon_destroy_class(sch, cl);
	return 0;
}

static int pon_change_class(struct Qdisc *sch, u32 classid,
			    u32 parentid, struct nlattr **tca,
			    unsigned long *arg, struct netlink_ext_ack *extack)
{
	int err = -EINVAL;
	struct pon_sched *q = qdisc_priv(sch);
	struct pon_class *cl = (struct pon_class *)*arg;
	struct Qdisc *parent_qdisc = NULL;
	struct netdev_queue *dev_queue;
	struct tc_pon_qopt *qopt;
	struct nlattr *opt = tca[TCA_OPTIONS];
	u8 control = 0;

	if (!cl) {		/* new class */
		struct Qdisc *new_q;

		/* check for valid classid */
		if (!classid || TC_H_MAJ(classid ^ sch->handle) ||
		    pon_find(classid, sch))
			goto failure;

		err = -ENOBUFS;
		cl = kzalloc(sizeof(*cl), GFP_KERNEL);
		if (!cl)
			goto failure;

		cl->common.classid = classid;
		q->children++;

		/* Make sure nothing interrupts us in between of two
		 * ndo_setup_tc calls.
		 */
		ASSERT_RTNL();

		dev_queue = sch->dev_queue;

		new_q = qdisc_create_dflt(dev_queue, &pfifo_qdisc_ops,
					  classid, NULL);

		sch_tree_lock(sch);

		/* leaf (we) needs elementary qdisc */
		cl->leaf.q = new_q ? new_q : &noop_qdisc;

		/* attach to the hash list and parent's family */
		qdisc_class_hash_insert(&q->clhash, &cl->common);
		if (cl->leaf.q != &noop_qdisc)
			qdisc_hash_add(cl->leaf.q, true);
	} else {
		sch_tree_lock(sch);
	}

	sch_tree_unlock(sch);
	qdisc_put(parent_qdisc);

	qdisc_class_hash_grow(sch, &q->clhash);

	*arg = (unsigned long)cl;

	if (opt) {
		qopt = nla_data(opt);
		control = qopt->control;
	}

	return pon_offload(sch, parentid, classid, control, TC_PON_REPLACE);

failure:
	return err;
}

static void pon_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct pon_sched *q = qdisc_priv(sch);
	struct pon_class *cl;
	unsigned int i;

	if (arg->stop)
		return;

	for (i = 0; i < q->clhash.hashsize; i++) {
		hlist_for_each_entry(cl, &q->clhash.hash[i], common.hnode) {
			if (arg->count < arg->skip) {
				arg->count++;
				continue;
			}
			if (arg->fn(sch, (unsigned long)cl, arg) < 0) {
				arg->stop = 1;
				return;
			}
			arg->count++;
		}
	}
}

static const struct Qdisc_class_ops pon_class_ops = {
	.select_queue	= pon_select_queue,
	.change		= pon_change_class,
	.graft		= pon_graft,
	.leaf		= pon_leaf,
	.find		= pon_search,
	.qlen_notify	= pon_qlen_notify,
	.dump		= pon_dump_class,
	.dump_stats	= pon_dump_class_stats,
	.walk		= pon_walk,
	.delete		= pon_delete,
};

static struct Qdisc_ops pon_qdisc_ops __read_mostly = {
	.cl_ops		= &pon_class_ops,
	.id		= "pon",
	.priv_size	= sizeof(struct pon_sched),
	.enqueue	= pon_qdisc_enqueue,
	.dequeue	= pon_qdisc_dequeue,
	.peek		= qdisc_peek_dequeued,
	.init		= pon_init,
	.destroy	= pon_destroy,
	.reset		= pon_reset,
	.change		= pon_change,
	.dump		= pon_dump,
	.dump_stats	= pon_dump_stats,
	.owner		= THIS_MODULE,
};

static int __init pon_module_init(void)
{
	return register_qdisc(&pon_qdisc_ops);
}

static void __exit pon_module_exit(void)
{
	unregister_qdisc(&pon_qdisc_ops);
}

module_init(pon_module_init);
module_exit(pon_module_exit);

MODULE_DESCRIPTION("PON queue discipline");
MODULE_AUTHOR("Stefan Chulski");
MODULE_LICENSE("GPL");
#endif
