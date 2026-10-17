/*
 * <:copyright-BRCM:2024:DUAL/GPL:standard
 *
 *    Copyright (c) 2024 Broadcom
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
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/genetlink.h>
#include <linux/kqoe.h>
#include "kqoe_local.h"

/* --- variables --- */
static struct genl_family genl_family;

static DEFINE_SPINLOCK(nl_lock);

/* --- functions --- */
static int kq_fill_bw(struct sk_buff *skb, int attr, const struct kq_bw *bw)
{
	struct nlattr *nla;

	nla = nla_nest_start(skb, attr | NLA_F_NESTED);
	if (!nla)
		goto err;

	if (nla_put_u64_64bit(skb, KQ_A_BW_MIN, bw->min, KQ_A_BW_PAD) ||
	    nla_put_u64_64bit(skb, KQ_A_BW_MAX, bw->max, KQ_A_BW_PAD))
		goto nla_put_failure;

	nla_nest_end(skb, nla);

	return 0;

nla_put_failure:
	nla_nest_cancel(skb, nla);
err:
	return -EMSGSIZE;
}

static const struct nla_policy kq_bw_policy[KQ_A_BW_MAX + 1] = {
	[KQ_A_BW_MIN]		= { .type = NLA_U64 },
	[KQ_A_BW_MAX]		= { .type = NLA_U64 },
};

static int kqa_change_bw(struct kq_bw *bw, struct nlattr *nla,
			 struct netlink_ext_ack *extack)
{
	struct nlattr *tb[KQ_A_BW_MAX + 1];
	int err;

	err = nla_parse_nested(tb, KQ_A_BW_MAX, nla, kq_bw_policy, extack);
	if (err)
		return err;

	if (tb[KQ_A_BW_MIN])
		bw->min = nla_get_u64(tb[KQ_A_BW_MIN]);
	if (tb[KQ_A_BW_MAX])
		bw->max = nla_get_u64(tb[KQ_A_BW_MAX]);

	return 0;
}

static int
kqa_fill_queue(struct sk_buff *skb, const struct kq_queue *q, u8 dir)
{
	if (nla_put_u8(skb, KQ_A_QUEUE_DIR, dir) ||
	    nla_put_u16(skb, KQ_A_QUEUE_ID, q->id) ||
	    kq_fill_bw(skb, KQ_A_QUEUE_BW, &q->bw) ||
	    nla_put_u32(skb, KQ_A_QUEUE_STATUS, q->status))
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -1;
}

static int kq_dump_queues(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct kq_cfg_dir *cfg;
	int dir, i;
	int err = 0;

	spin_lock_bh(&nl_lock);

	for (dir = cb->args[0]; dir < KQ_DIR_MAX; dir++) {
		cfg = (dir == KQ_DIR_DS) ? &kqcfg.ds : &kqcfg.us;
		cb->args[0] = dir;

		for (i = cb->args[1]; i < cfg->num_queues; i++) {
			struct kq_queue q = { .id = i };
			void *hdr;

			err = kq_hw_queue_fill(&q, dir);
			if (err)
				goto out;

			hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).portid,
					  cb->nlh->nlmsg_seq, &genl_family,
					  NLM_F_MULTI, KQ_CMD_GET_QUEUE);
			if (!hdr) {
				err = -EMSGSIZE;
				goto out;
			}

			err = kqa_fill_queue(skb, &q, dir);
			if (err) {
				cb->args[1] = i;
				goto out;
			}

			genlmsg_end(skb, hdr);
		}

		cb->args[1] = 0;
	}

out:
	spin_unlock_bh(&nl_lock);
	return err;
}

static const struct nla_policy kq_queue_policy[KQ_A_QUEUE_MAX + 1] = {
	[KQ_A_QUEUE_DIR]	= { .type = NLA_U8 },
	[KQ_A_QUEUE_ID]		= { .type = NLA_U16 },
	[KQ_A_QUEUE_BW]		= { .type = NLA_NESTED },
	[KQ_A_QUEUE_STATUS]	= { .type = NLA_U32 },
};

static int kq_get_queue(struct sk_buff *skb, struct genl_info *info)
{
	struct kq_queue q = {};
	struct nlattr **tb = info->attrs;
	struct sk_buff *msg;
	void *reply;
	int err;
	u8 dir;

	if (!tb[KQ_A_QUEUE_DIR] || !tb[KQ_A_QUEUE_ID])
		return -EINVAL;
	dir	= nla_get_u8(tb[KQ_A_QUEUE_DIR]);
	q.id	= ntohs(nla_get_u16(tb[KQ_A_QUEUE_ID]));

	err = kq_hw_queue_fill(&q, dir);
	if (err)
		return err;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	reply = genlmsg_put_reply(msg, info, &genl_family, 0,
				  KQ_CMD_GET_QUEUE);
	if (!reply) {
		err = -EMSGSIZE;
		goto err_free_msg;
	}

	err = kqa_fill_queue(msg, &q, dir);
	if (err)
		goto err_free_msg;

	genlmsg_end(msg, reply);

	return genlmsg_reply(msg, info);

err_free_msg:
	nlmsg_free(msg);
	return err;
}

static int kq_set_queue(struct sk_buff *skb, struct genl_info *info)
{
	struct kq_queue q = { .id  = -1 };
	struct nlattr **tb = info->attrs;
	int err = 0;
	long flags = 0;
	u8 dir;

	if (!tb[KQ_A_QUEUE_DIR] || !tb[KQ_A_QUEUE_ID])
		return -EINVAL;

	dir = nla_get_u8(tb[KQ_A_QUEUE_DIR]);
	if (dir >= KQ_DIR_MAX)
		return -EINVAL;
	q.id = ntohs(nla_get_u16(tb[KQ_A_QUEUE_ID]));

	err = kq_hw_queue_fill(&q, dir);
	if (err)
		return err;

	if (tb[KQ_A_QUEUE_BW]) {
		err = kqa_change_bw(&q.bw, tb[KQ_A_QUEUE_BW], info->extack);
		if (err)
			return err;
		set_bit(KQ_A_QUEUE_BW, &flags);
	}

	if (tb[KQ_A_QUEUE_STATUS]) {
		q.status = nla_get_u32(tb[KQ_A_QUEUE_STATUS]);
		set_bit(KQ_A_QUEUE_STATUS, &flags);
	}

	if (flags) {
		pr_debug("%s queue %d [%llu %llu]\n",
			 dir == KQ_DIR_US ? "US" : "DS", q.id, q.bw.min,
			 q.bw.max);
		kq_hw_queue_set(&q, dir);
	}

	return 0;
}

/* --- config --- */
static int kq_find_config(struct kq_cfg_dir *cfg, u8 dir)
{
	struct kq_cfg_dir *c;

	if (dir >= KQ_DIR_MAX)
		return -EINVAL;

	c = (dir == KQ_DIR_US) ? &kqcfg.us : &kqcfg.ds;

	memcpy(cfg, c, sizeof(*cfg));

	return kq_hw_config_fill(cfg, dir);
}

static int kq_fill_config(struct sk_buff *skb, struct kq_cfg_dir *c, u8 dir)
{
	if (nla_put_u8(skb, KQ_A_CFG_DIR, dir) ||
	    nla_put_u8(skb, KQ_A_CFG_NUM_QUEUES, c->num_queues) ||
	    kq_fill_bw(skb, KQ_A_CFG_BW, &c->bw))
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -EMSGSIZE;
}

static int kq_dump_configs(struct sk_buff *skb, struct netlink_callback *cb)
{
	void *hdr;
	int err;

	spin_lock_bh(&nl_lock);

	for (; cb->args[0] < KQ_DIR_MAX; cb->args[0]++) {
		struct kq_cfg_dir cfg;

		err = kq_find_config(&cfg, cb->args[0]);
		if (err)
			goto out;

		hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).portid,
				  cb->nlh->nlmsg_seq, &genl_family,
				  NLM_F_MULTI, KQ_CMD_GET_CONFIG);
		if (!hdr) {
			err = -EMSGSIZE;
			goto out;
		}

		err = kq_fill_config(skb, &cfg, cb->args[0]);
		if (err) {
			genlmsg_cancel(skb, hdr);
			goto out;
		}

		genlmsg_end(skb, hdr);
	}
	err = 0;

out:
	spin_unlock_bh(&nl_lock);
	return err;
}

static const struct nla_policy kq_config_policy[KQ_A_CFG_MAX + 1] = {
	[KQ_A_CFG_DIR]		= { .type = NLA_U8 },
	[KQ_A_CFG_BW]		= { .type = NLA_NESTED },
};

static int kq_get_config(struct sk_buff *skb, struct genl_info *info)
{
	struct kq_cfg_dir cfg = {};
	struct nlattr **tb = info->attrs;
	struct sk_buff *msg;
	void *reply;
	int err;
	u8 dir;

	if (!tb[KQ_A_CFG_DIR])
		return -EINVAL;
	dir = nla_get_u8(tb[KQ_A_CFG_DIR]);
	err = kq_find_config(&cfg, dir);
	if (err)
		goto err;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg) {
		err = -ENOMEM;
		goto err;
	}

	reply = genlmsg_put_reply(msg, info, &genl_family, 0,
				  KQ_CMD_GET_CONFIG);
	if (!reply) {
		err = -EMSGSIZE;
		goto err_free_msg;
	}

	err = kq_fill_config(msg, &cfg, dir);
	if (err)
		goto err_free_msg;

	genlmsg_end(msg, reply);

	return genlmsg_reply(msg, info);

err_free_msg:
	nlmsg_free(msg);
err:
	return err;
}

static int kq_set_config(struct sk_buff *skb, struct genl_info *info)
{
	struct kq_cfg_dir cfg = {};
	struct nlattr **tb = info->attrs;
	struct kq_cfg_dir *c;
	int err;
	u8 dir;

	if (!tb[KQ_A_CFG_DIR])
		return -EINVAL;

	dir = nla_get_u8(tb[KQ_A_CFG_DIR]);
	err = kq_find_config(&cfg, dir);
	if (err)
		return err;

	c = (dir == KQ_DIR_US) ? &kqcfg.us : &kqcfg.ds;

	if (tb[KQ_A_CFG_BW]) {
		err = kqa_change_bw(&cfg.bw, tb[KQ_A_CFG_BW], info->extack);
		if (err)
			return err;

		pr_debug("%s config [%llu %llu]\n",
			 dir == KQ_DIR_US ? "US" : "DS", cfg.bw.min,
			 cfg.bw.max);
		err = kq_hw_config_set(&cfg, dir);
		if (err)
			return err;
	}

	return err;
}

static const struct genl_ops genl_ops[] = {
	{
		.cmd = KQ_CMD_GET_CONFIG,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.flags = GENL_UNS_ADMIN_PERM,
		.dumpit = kq_dump_configs,
		.doit = kq_get_config,
		.policy = kq_config_policy,
		.maxattr = ARRAY_SIZE(kq_config_policy),
	},
	{
		.cmd = KQ_CMD_SET_CONFIG,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.flags = GENL_UNS_ADMIN_PERM,
		.doit = kq_set_config,
		.policy = kq_config_policy,
		.maxattr = ARRAY_SIZE(kq_config_policy),
	},
	{
		.cmd = KQ_CMD_GET_QUEUE,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.flags = GENL_UNS_ADMIN_PERM,
		.dumpit = kq_dump_queues,
		.doit = kq_get_queue,
		.policy = kq_queue_policy,
		.maxattr = ARRAY_SIZE(kq_queue_policy),
	},
	{
		.cmd = KQ_CMD_SET_QUEUE,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.flags = GENL_UNS_ADMIN_PERM,
		.doit = kq_set_queue,
		.policy = kq_queue_policy,
		.maxattr = ARRAY_SIZE(kq_queue_policy),
	},
};

static const struct genl_multicast_group genl_mcgrps[] = {
	{ .name = KQOE_GENL_MC_GRP_NAME },
};

static struct genl_family genl_family __ro_after_init = {
	.name		= KQOE_GENL_NAME,
	.version	= KQOE_GENL_VERSION,
	.ops		= genl_ops,
	.n_ops		= ARRAY_SIZE(genl_ops),
	.mcgrps		= genl_mcgrps,
	.n_mcgrps	= ARRAY_SIZE(genl_mcgrps),
	.netnsok	= true
};

int __init kqnl_init(void)
{
	return genl_register_family(&genl_family);
}

void __exit kqnl_exit(void)
{
	genl_unregister_family(&genl_family);
}
