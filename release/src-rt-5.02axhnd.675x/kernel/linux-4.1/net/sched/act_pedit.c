/*
 * net/sched/act_pedit.c	Generic packet editor
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Jamal Hadi Salim (2002-4)
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <net/netlink.h>
#include <net/pkt_sched.h>
#include <linux/tc_act/tc_pedit.h>
#include <net/tc_act/tc_pedit.h>

#define PEDIT_TAB_MASK	15

static const struct nla_policy pedit_policy[TCA_PEDIT_MAX + 1] = {
	[TCA_PEDIT_PARMS]	= { .len = sizeof(struct tc_pedit) },
};

static int tcf_pedit_init(struct net *net, struct nlattr *nla,
			  struct nlattr *est, struct tc_action *a,
			  int ovr, int bind)
{
	struct nlattr *tb[TCA_PEDIT_MAX + 1];
	struct tc_pedit *parm;
	int ret = 0, err;
	struct tcf_pedit *p;
	struct tc_pedit_key *keys = NULL;
	int ksize;

	if (nla == NULL)
		return -EINVAL;

	err = nla_parse_nested(tb, TCA_PEDIT_MAX, nla, pedit_policy);
	if (err < 0)
		return err;

	if (tb[TCA_PEDIT_PARMS] == NULL)
		return -EINVAL;
	parm = nla_data(tb[TCA_PEDIT_PARMS]);
	ksize = parm->nkeys * sizeof(struct tc_pedit_key);
	if (nla_len(tb[TCA_PEDIT_PARMS]) < sizeof(*parm) + ksize)
		return -EINVAL;

	if (!tcf_hash_check(parm->index, a, bind)) {
		if (!parm->nkeys)
			return -EINVAL;
		ret = tcf_hash_create(parm->index, est, a, sizeof(*p), bind);
		if (ret)
			return ret;
		p = to_pedit(a);
		keys = kmalloc(ksize, GFP_KERNEL);
		if (keys == NULL) {
			tcf_hash_cleanup(a, est);
			return -ENOMEM;
		}
		ret = ACT_P_CREATED;
	} else {
		p = to_pedit(a);
		tcf_hash_release(a, bind);
		if (bind)
			return 0;
		if (!ovr)
			return -EEXIST;

		if (p->tcfp_nkeys && p->tcfp_nkeys != parm->nkeys) {
			keys = kmalloc(ksize, GFP_KERNEL);
			if (keys == NULL)
				return -ENOMEM;
		}
	}

	spin_lock_bh(&p->tcf_lock);
	p->tcfp_flags = parm->flags;
	p->tcf_action = parm->action;
	if (keys) {
		kfree(p->tcfp_keys);
		p->tcfp_keys = keys;
		p->tcfp_nkeys = parm->nkeys;
	}
	memcpy(p->tcfp_keys, parm->keys, ksize);
	spin_unlock_bh(&p->tcf_lock);
	if (ret == ACT_P_CREATED)
		tcf_hash_insert(a);
	return ret;
}

static void tcf_pedit_cleanup(struct tc_action *a, int bind)
{
	struct tcf_pedit *p = a->priv;
	struct tc_pedit_key *keys = p->tcfp_keys;
	kfree(keys);
}

static int tcf_pedit(struct sk_buff *skb, const struct tc_action *a,
		     struct tcf_result *res)
{
	struct tcf_pedit *p = a->priv;
	int i, munged = 0;
	unsigned int off;

	if (skb_unclone(skb, GFP_ATOMIC))
		return p->tcf_action;

	off = skb_network_offset(skb);

	spin_lock(&p->tcf_lock);

	p->tcf_tm.lastuse = jiffies;

	if (p->tcfp_nkeys > 0) {
		struct tc_pedit_key *tkey = p->tcfp_keys;

		for (i = p->tcfp_nkeys; i > 0; i--, tkey++) {
			u32 *ptr, _data;
			int offset = tkey->off;

			if (tkey->offmask) {
				char *d, _d;

				d = skb_header_pointer(skb, off + tkey->at, 1,
						       &_d);
				if (!d)
					goto bad;
				offset += (*d & tkey->offmask) >> tkey->shift;
			}

			if (offset % 4) {
				pr_info("tc filter pedit"
					" offset must be on 32 bit boundaries\n");
				goto bad;
			}
			if (offset > 0 && offset > skb->len) {
				pr_info("tc filter pedit"
					" offset %d can't exceed pkt length %d\n",
				       offset, skb->len);
				goto bad;
			}

			ptr = skb_header_pointer(skb, off + offset, 4, &_data);
			if (!ptr)
				goto bad;
			/* just do it, baby */
			*ptr = ((*ptr & tkey->mask) ^ tkey->val);
			if (ptr == &_data)
				skb_store_bits(skb, off + offset, ptr, 4);
			munged++;
		}

		if (munged)
			skb->tc_verd = SET_TC_MUNGED(skb->tc_verd);
		goto done;
	} else
		WARN(1, "pedit BUG: index %d\n", p->tcf_index);

bad:
	p->tcf_qstats.overlimits++;
done:
	bstats_update(&p->tcf_bstats, skb);
	spin_unlock(&p->tcf_lock);
	return p->tcf_action;
}

static int tcf_pedit_dump(struct sk_buff *skb, struct tc_action *a,
			  int bind, int ref)
{
	unsigned char *b = skb_tail_pointer(skb);
	struct tcf_pedit *p = a->priv;
	struct tc_pedit *opt;
	struct tcf_t t;
	int s;

	s = sizeof(*opt) + p->tcfp_nkeys * sizeof(struct tc_pedit_key);

	/* netlink spinlocks held above us - must use ATOMIC */
	opt = kzalloc(s, GFP_ATOMIC);
	if (unlikely(!opt))
		return -ENOBUFS;

	memcpy(opt->keys, p->tcfp_keys,
	       p->tcfp_nkeys * sizeof(struct tc_pedit_key));
	opt->index = p->tcf_index;
	opt->nkeys = p->tcfp_nkeys;
	opt->flags = p->tcfp_flags;
	opt->action = p->tcf_action;
	opt->refcnt = p->tcf_refcnt - ref;
	opt->bindcnt = p->tcf_bindcnt - bind;

	if (nla_put(skb, TCA_PEDIT_PARMS, s, opt))
		goto nla_put_failure;
	t.install = jiffies_to_clock_t(jiffies - p->tcf_tm.install);
	t.lastuse = jiffies_to_clock_t(jiffies - p->tcf_tm.lastuse);
	t.expires = jiffies_to_clock_t(p->tcf_tm.expires);
	if (nla_put(skb, TCA_PEDIT_TM, sizeof(t), &t))
		goto nla_put_failure;
	kfree(opt);
	return skb->len;

nla_put_failure:
	nlmsg_trim(skb, b);
	kfree(opt);
	return -1;
}

static struct tc_action_ops act_pedit_ops = {
	.kind		=	"pedit",
	.type		=	TCA_ACT_PEDIT,
	.owner		=	THIS_MODULE,
	.act		=	tcf_pedit,
	.dump		=	tcf_pedit_dump,
	.cleanup	=	tcf_pedit_cleanup,
	.init		=	tcf_pedit_init,
};

MODULE_AUTHOR("Jamal Hadi Salim(2002-4)");
MODULE_DESCRIPTION("Generic Packet Editor actions");
MODULE_LICENSE("GPL");

static int __init pedit_init_module(void)
{
	return tcf_register_action(&act_pedit_ops, PEDIT_TAB_MASK);
}

static void __exit pedit_cleanup_module(void)
{
	tcf_unregister_action(&act_pedit_ops);
}

module_init(pedit_init_module);
module_exit(pedit_cleanup_module);

