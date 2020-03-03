/*
 * net/sched/act_police.c	Input police filter
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 * 		J Hadi Salim (action changes)
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <net/act_api.h>
#include <net/netlink.h>

struct tcf_police {
	struct tcf_common	common;
	int			tcfp_result;
	u32			tcfp_ewma_rate;
	s64			tcfp_burst;
	u32			tcfp_mtu;
	s64			tcfp_toks;
	s64			tcfp_ptoks;
	s64			tcfp_mtu_ptoks;
	s64			tcfp_t_c;
	struct psched_ratecfg	rate;
	bool			rate_present;
	struct psched_ratecfg	peak;
	bool			peak_present;
};
#define to_police(pc)	\
	container_of(pc, struct tcf_police, common)

#define POL_TAB_MASK     15

/* old policer structure from before tc actions */
struct tc_police_compat {
	u32			index;
	int			action;
	u32			limit;
	u32			burst;
	u32			mtu;
	struct tc_ratespec	rate;
	struct tc_ratespec	peakrate;
};

/* Each policer is serialized by its individual spinlock */

static int tcf_act_police_walker(struct sk_buff *skb, struct netlink_callback *cb,
			      int type, struct tc_action *a)
{
	struct tcf_hashinfo *hinfo = a->ops->hinfo;
	struct hlist_head *head;
	struct tcf_common *p;
	int err = 0, index = -1, i = 0, s_i = 0, n_i = 0;
	struct nlattr *nest;

	spin_lock_bh(&hinfo->lock);

	s_i = cb->args[0];

	for (i = 0; i < (POL_TAB_MASK + 1); i++) {
		head = &hinfo->htab[tcf_hash(i, POL_TAB_MASK)];

		hlist_for_each_entry_rcu(p, head, tcfc_head) {
			index++;
			if (index < s_i)
				continue;
			a->priv = p;
			a->order = index;
			nest = nla_nest_start(skb, a->order);
			if (nest == NULL)
				goto nla_put_failure;
			if (type == RTM_DELACTION)
				err = tcf_action_dump_1(skb, a, 0, 1);
			else
				err = tcf_action_dump_1(skb, a, 0, 0);
			if (err < 0) {
				index--;
				nla_nest_cancel(skb, nest);
				goto done;
			}
			nla_nest_end(skb, nest);
			n_i++;
		}
	}
done:
	spin_unlock_bh(&hinfo->lock);
	if (n_i)
		cb->args[0] += n_i;
	return n_i;

nla_put_failure:
	nla_nest_cancel(skb, nest);
	goto done;
}

static const struct nla_policy police_policy[TCA_POLICE_MAX + 1] = {
	[TCA_POLICE_RATE]	= { .len = TC_RTAB_SIZE },
	[TCA_POLICE_PEAKRATE]	= { .len = TC_RTAB_SIZE },
	[TCA_POLICE_AVRATE]	= { .type = NLA_U32 },
	[TCA_POLICE_RESULT]	= { .type = NLA_U32 },
};

static int tcf_act_police_locate(struct net *net, struct nlattr *nla,
				 struct nlattr *est, struct tc_action *a,
				 int ovr, int bind)
{
	unsigned int h;
	int ret = 0, err;
	struct nlattr *tb[TCA_POLICE_MAX + 1];
	struct tc_police *parm;
	struct tcf_police *police;
	struct qdisc_rate_table *R_tab = NULL, *P_tab = NULL;
	struct tcf_hashinfo *hinfo = a->ops->hinfo;
	int size;

	if (nla == NULL)
		return -EINVAL;

	err = nla_parse_nested(tb, TCA_POLICE_MAX, nla, police_policy);
	if (err < 0)
		return err;

	if (tb[TCA_POLICE_TBF] == NULL)
		return -EINVAL;
	size = nla_len(tb[TCA_POLICE_TBF]);
	if (size != sizeof(*parm) && size != sizeof(struct tc_police_compat))
		return -EINVAL;
	parm = nla_data(tb[TCA_POLICE_TBF]);

	if (parm->index) {
		if (tcf_hash_search(a, parm->index)) {
			police = to_police(a->priv);
			if (bind) {
				police->tcf_bindcnt += 1;
				police->tcf_refcnt += 1;
				return 0;
			}
			if (ovr)
				goto override;
			/* not replacing */
			return -EEXIST;
		}
	}

	police = kzalloc(sizeof(*police), GFP_KERNEL);
	if (police == NULL)
		return -ENOMEM;
	ret = ACT_P_CREATED;
	police->tcf_refcnt = 1;
	spin_lock_init(&police->tcf_lock);
	if (bind)
		police->tcf_bindcnt = 1;
override:
	if (parm->rate.rate) {
		err = -ENOMEM;
		R_tab = qdisc_get_rtab(&parm->rate, tb[TCA_POLICE_RATE]);
		if (R_tab == NULL)
			goto failure;

		if (parm->peakrate.rate) {
			P_tab = qdisc_get_rtab(&parm->peakrate,
					       tb[TCA_POLICE_PEAKRATE]);
			if (P_tab == NULL)
				goto failure;
		}
	}

	spin_lock_bh(&police->tcf_lock);
	if (est) {
		err = gen_replace_estimator(&police->tcf_bstats, NULL,
					    &police->tcf_rate_est,
					    &police->tcf_lock, est);
		if (err)
			goto failure_unlock;
	} else if (tb[TCA_POLICE_AVRATE] &&
		   (ret == ACT_P_CREATED ||
		    !gen_estimator_active(&police->tcf_bstats,
					  &police->tcf_rate_est))) {
		err = -EINVAL;
		goto failure_unlock;
	}

	/* No failure allowed after this point */
	police->tcfp_mtu = parm->mtu;
	if (police->tcfp_mtu == 0) {
		police->tcfp_mtu = ~0;
		if (R_tab)
			police->tcfp_mtu = 255 << R_tab->rate.cell_log;
	}
	if (R_tab) {
		police->rate_present = true;
		psched_ratecfg_precompute(&police->rate, &R_tab->rate, 0);
		qdisc_put_rtab(R_tab);
	} else {
		police->rate_present = false;
	}
	if (P_tab) {
		police->peak_present = true;
		psched_ratecfg_precompute(&police->peak, &P_tab->rate, 0);
		qdisc_put_rtab(P_tab);
	} else {
		police->peak_present = false;
	}

	if (tb[TCA_POLICE_RESULT])
		police->tcfp_result = nla_get_u32(tb[TCA_POLICE_RESULT]);
	police->tcfp_burst = PSCHED_TICKS2NS(parm->burst);
	police->tcfp_toks = police->tcfp_burst;
	if (police->peak_present) {
		police->tcfp_mtu_ptoks = (s64) psched_l2t_ns(&police->peak,
							     police->tcfp_mtu);
		police->tcfp_ptoks = police->tcfp_mtu_ptoks;
	}
	police->tcf_action = parm->action;

	if (tb[TCA_POLICE_AVRATE])
		police->tcfp_ewma_rate = nla_get_u32(tb[TCA_POLICE_AVRATE]);

	spin_unlock_bh(&police->tcf_lock);
	if (ret != ACT_P_CREATED)
		return ret;

	police->tcfp_t_c = ktime_get_ns();
	police->tcf_index = parm->index ? parm->index :
		tcf_hash_new_index(hinfo);
	h = tcf_hash(police->tcf_index, POL_TAB_MASK);
	spin_lock_bh(&hinfo->lock);
	hlist_add_head(&police->tcf_head, &hinfo->htab[h]);
	spin_unlock_bh(&hinfo->lock);

	a->priv = police;
	return ret;

failure_unlock:
	spin_unlock_bh(&police->tcf_lock);
failure:
	qdisc_put_rtab(P_tab);
	qdisc_put_rtab(R_tab);
	if (ret == ACT_P_CREATED)
		kfree(police);
	return err;
}

static int tcf_act_police(struct sk_buff *skb, const struct tc_action *a,
			  struct tcf_result *res)
{
	struct tcf_police *police = a->priv;
	s64 now;
	s64 toks;
	s64 ptoks = 0;

	spin_lock(&police->tcf_lock);

	bstats_update(&police->tcf_bstats, skb);

	if (police->tcfp_ewma_rate &&
	    police->tcf_rate_est.bps >= police->tcfp_ewma_rate) {
		police->tcf_qstats.overlimits++;
		if (police->tcf_action == TC_ACT_SHOT)
			police->tcf_qstats.drops++;
		spin_unlock(&police->tcf_lock);
		return police->tcf_action;
	}

	if (qdisc_pkt_len(skb) <= police->tcfp_mtu) {
		if (!police->rate_present) {
			spin_unlock(&police->tcf_lock);
			return police->tcfp_result;
		}

		now = ktime_get_ns();
		toks = min_t(s64, now - police->tcfp_t_c,
			     police->tcfp_burst);
		if (police->peak_present) {
			ptoks = toks + police->tcfp_ptoks;
			if (ptoks > police->tcfp_mtu_ptoks)
				ptoks = police->tcfp_mtu_ptoks;
			ptoks -= (s64) psched_l2t_ns(&police->peak,
						     qdisc_pkt_len(skb));
		}
		toks += police->tcfp_toks;
		if (toks > police->tcfp_burst)
			toks = police->tcfp_burst;
		toks -= (s64) psched_l2t_ns(&police->rate, qdisc_pkt_len(skb));
		if ((toks|ptoks) >= 0) {
			police->tcfp_t_c = now;
			police->tcfp_toks = toks;
			police->tcfp_ptoks = ptoks;
			spin_unlock(&police->tcf_lock);
			return police->tcfp_result;
		}
	}

	police->tcf_qstats.overlimits++;
	if (police->tcf_action == TC_ACT_SHOT)
		police->tcf_qstats.drops++;
	spin_unlock(&police->tcf_lock);
	return police->tcf_action;
}

static int
tcf_act_police_dump(struct sk_buff *skb, struct tc_action *a, int bind, int ref)
{
	unsigned char *b = skb_tail_pointer(skb);
	struct tcf_police *police = a->priv;
	struct tc_police opt = {
		.index = police->tcf_index,
		.action = police->tcf_action,
		.mtu = police->tcfp_mtu,
		.burst = PSCHED_NS2TICKS(police->tcfp_burst),
		.refcnt = police->tcf_refcnt - ref,
		.bindcnt = police->tcf_bindcnt - bind,
	};

	if (police->rate_present)
		psched_ratecfg_getrate(&opt.rate, &police->rate);
	if (police->peak_present)
		psched_ratecfg_getrate(&opt.peakrate, &police->peak);
	if (nla_put(skb, TCA_POLICE_TBF, sizeof(opt), &opt))
		goto nla_put_failure;
	if (police->tcfp_result &&
	    nla_put_u32(skb, TCA_POLICE_RESULT, police->tcfp_result))
		goto nla_put_failure;
	if (police->tcfp_ewma_rate &&
	    nla_put_u32(skb, TCA_POLICE_AVRATE, police->tcfp_ewma_rate))
		goto nla_put_failure;
	return skb->len;

nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

MODULE_AUTHOR("Alexey Kuznetsov");
MODULE_DESCRIPTION("Policing actions");
MODULE_LICENSE("GPL");

static struct tc_action_ops act_police_ops = {
	.kind		=	"police",
	.type		=	TCA_ID_POLICE,
	.owner		=	THIS_MODULE,
	.act		=	tcf_act_police,
	.dump		=	tcf_act_police_dump,
	.init		=	tcf_act_police_locate,
	.walk		=	tcf_act_police_walker
};

static int __init
police_init_module(void)
{
	return tcf_register_action(&act_police_ops, POL_TAB_MASK);
}

static void __exit
police_cleanup_module(void)
{
	tcf_unregister_action(&act_police_ops);
}

module_init(police_init_module);
module_exit(police_cleanup_module);
