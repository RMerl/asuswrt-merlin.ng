/*
 * net/sched/cls_cgroup.c	Control Group Classifier
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Thomas Graf <tgraf@suug.ch>
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/rcupdate.h>
#include <net/rtnetlink.h>
#include <net/pkt_cls.h>
#include <net/sock.h>
#include <net/cls_cgroup.h>

struct cls_cgroup_head {
	u32			handle;
	struct tcf_exts		exts;
	struct tcf_ematch_tree	ematches;
	struct tcf_proto	*tp;
	struct rcu_head		rcu;
};

static int cls_cgroup_classify(struct sk_buff *skb, const struct tcf_proto *tp,
			       struct tcf_result *res)
{
	struct cls_cgroup_head *head = rcu_dereference_bh(tp->root);
	u32 classid;

	classid = task_cls_state(current)->classid;

	/*
	 * Due to the nature of the classifier it is required to ignore all
	 * packets originating from softirq context as accessing `current'
	 * would lead to false results.
	 *
	 * This test assumes that all callers of dev_queue_xmit() explicitely
	 * disable bh. Knowing this, it is possible to detect softirq based
	 * calls by looking at the number of nested bh disable calls because
	 * softirqs always disables bh.
	 */
	if (in_serving_softirq()) {
		/* If there is an sk_classid we'll use that. */
		if (!skb->sk)
			return -1;
		classid = skb->sk->sk_classid;
	}

	if (!classid)
		return -1;

	if (!tcf_em_tree_match(skb, &head->ematches, NULL))
		return -1;

	res->classid = classid;
	res->class = 0;
	return tcf_exts_exec(skb, &head->exts, res);
}

static unsigned long cls_cgroup_get(struct tcf_proto *tp, u32 handle)
{
	return 0UL;
}

static int cls_cgroup_init(struct tcf_proto *tp)
{
	return 0;
}

static const struct nla_policy cgroup_policy[TCA_CGROUP_MAX + 1] = {
	[TCA_CGROUP_EMATCHES]	= { .type = NLA_NESTED },
};

static void cls_cgroup_destroy_rcu(struct rcu_head *root)
{
	struct cls_cgroup_head *head = container_of(root,
						    struct cls_cgroup_head,
						    rcu);

	tcf_exts_destroy(&head->exts);
	tcf_em_tree_destroy(&head->ematches);
	kfree(head);
}

static int cls_cgroup_change(struct net *net, struct sk_buff *in_skb,
			     struct tcf_proto *tp, unsigned long base,
			     u32 handle, struct nlattr **tca,
			     unsigned long *arg, bool ovr)
{
	struct nlattr *tb[TCA_CGROUP_MAX + 1];
	struct cls_cgroup_head *head = rtnl_dereference(tp->root);
	struct cls_cgroup_head *new;
	struct tcf_ematch_tree t;
	struct tcf_exts e;
	int err;

	if (!tca[TCA_OPTIONS])
		return -EINVAL;

	if (!head && !handle)
		return -EINVAL;

	if (head && handle != head->handle)
		return -ENOENT;

	new = kzalloc(sizeof(*head), GFP_KERNEL);
	if (!new)
		return -ENOBUFS;

	tcf_exts_init(&new->exts, TCA_CGROUP_ACT, TCA_CGROUP_POLICE);
	new->handle = handle;
	new->tp = tp;
	err = nla_parse_nested(tb, TCA_CGROUP_MAX, tca[TCA_OPTIONS],
			       cgroup_policy);
	if (err < 0)
		goto errout;

	tcf_exts_init(&e, TCA_CGROUP_ACT, TCA_CGROUP_POLICE);
	err = tcf_exts_validate(net, tp, tb, tca[TCA_RATE], &e, ovr);
	if (err < 0)
		goto errout;

	err = tcf_em_tree_validate(tp, tb[TCA_CGROUP_EMATCHES], &t);
	if (err < 0) {
		tcf_exts_destroy(&e);
		goto errout;
	}

	tcf_exts_change(tp, &new->exts, &e);
	tcf_em_tree_change(tp, &new->ematches, &t);

	rcu_assign_pointer(tp->root, new);
	if (head)
		call_rcu(&head->rcu, cls_cgroup_destroy_rcu);
	return 0;
errout:
	kfree(new);
	return err;
}

static bool cls_cgroup_destroy(struct tcf_proto *tp, bool force)
{
	struct cls_cgroup_head *head = rtnl_dereference(tp->root);

	if (!force)
		return false;

	if (head) {
		RCU_INIT_POINTER(tp->root, NULL);
		call_rcu(&head->rcu, cls_cgroup_destroy_rcu);
	}
	return true;
}

static int cls_cgroup_delete(struct tcf_proto *tp, unsigned long arg)
{
	return -EOPNOTSUPP;
}

static void cls_cgroup_walk(struct tcf_proto *tp, struct tcf_walker *arg)
{
	struct cls_cgroup_head *head = rtnl_dereference(tp->root);

	if (arg->count < arg->skip)
		goto skip;

	if (arg->fn(tp, (unsigned long) head, arg) < 0) {
		arg->stop = 1;
		return;
	}
skip:
	arg->count++;
}

static int cls_cgroup_dump(struct net *net, struct tcf_proto *tp, unsigned long fh,
			   struct sk_buff *skb, struct tcmsg *t)
{
	struct cls_cgroup_head *head = rtnl_dereference(tp->root);
	struct nlattr *nest;

	t->tcm_handle = head->handle;

	nest = nla_nest_start(skb, TCA_OPTIONS);
	if (nest == NULL)
		goto nla_put_failure;

	if (tcf_exts_dump(skb, &head->exts) < 0 ||
	    tcf_em_tree_dump(skb, &head->ematches, TCA_CGROUP_EMATCHES) < 0)
		goto nla_put_failure;

	nla_nest_end(skb, nest);

	if (tcf_exts_dump_stats(skb, &head->exts) < 0)
		goto nla_put_failure;

	return skb->len;

nla_put_failure:
	nla_nest_cancel(skb, nest);
	return -1;
}

static struct tcf_proto_ops cls_cgroup_ops __read_mostly = {
	.kind		=	"cgroup",
	.init		=	cls_cgroup_init,
	.change		=	cls_cgroup_change,
	.classify	=	cls_cgroup_classify,
	.destroy	=	cls_cgroup_destroy,
	.get		=	cls_cgroup_get,
	.delete		=	cls_cgroup_delete,
	.walk		=	cls_cgroup_walk,
	.dump		=	cls_cgroup_dump,
	.owner		=	THIS_MODULE,
};

static int __init init_cgroup_cls(void)
{
	return register_tcf_proto_ops(&cls_cgroup_ops);
}

static void __exit exit_cgroup_cls(void)
{
	unregister_tcf_proto_ops(&cls_cgroup_ops);
}

module_init(init_cgroup_cls);
module_exit(exit_cgroup_cls);
MODULE_LICENSE("GPL");
