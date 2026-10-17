/*
 * <:copyright-BRCM:2019:GPL/GPL:standard
 *
 *    Copyright (c) 2019 Broadcom
 *    All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * :>
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <net/netfilter/nf_conntrack_timestamp.h>
#include <linux/blog.h>
#include <linux/dpi.h>
#include <linux/ndi.h>

#include <net/netlink.h>
#include <net/netfilter/nf_conntrack.h>

struct bcm_nl_helper {
	int (*size)(const struct nf_conn *ct);
	int (*dump)(struct sk_buff *skb, const struct nf_conn *ct);
	int (*change)(struct nf_conn *ct, struct nlattr *cda[]);
};

#if IS_ENABLED(CONFIG_BCM_KQOE)
static int ctnetlink_size_qos(const struct nf_conn *ct)
{
	return nla_total_size(sizeof(int16_t))		/* CTA_QOS_ORIG_QUEUE */
	       + nla_total_size(sizeof(int16_t))	/* CTA_QOS_REPL_QUEUE */
	       + nla_total_size(sizeof(uint8_t))	/* CTA_QOS_EG_PRIO */
	       + nla_total_size(sizeof(uint8_t))	/* CTA_QOS_DSCP */
	       + nla_total_size(sizeof(uint8_t))	/* CTA_QOS_EG_TID */
	       + nla_total_size(sizeof(uint8_t))	/* CTA_QOS_IN_TID */
	       + nla_total_size(sizeof(uint64_t))	/* CTA_QOS_FLAGS */
	       ;
};

static int ctnetlink_dump_qos(struct sk_buff *skb, const struct nf_conn *ct)
{
	struct nlattr *nest_parms;
	const struct kqoe_ct_info *qos;

	nest_parms = nla_nest_start(skb, CTA_BROADCOM_QOS | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	qos = &ct->bcm_ext.qos;
	if (nla_put_s16(skb, CTA_QOS_ORIG_QUEUE, htons(qos->orig_queue)) ||
	    nla_put_s16(skb, CTA_QOS_REPL_QUEUE, htons(qos->reply_queue)) ||
	    nla_put_u8(skb, CTA_QOS_EG_PRIO, qos->eg_prio) ||
	    nla_put_u8(skb, CTA_QOS_DSCP, qos->dscp) ||
	    nla_put_u8(skb, CTA_QOS_EG_TID, qos->eg_tid) ||
	    nla_put_u8(skb, CTA_QOS_IN_TID, qos->in_tid) ||
	    nla_put_u64_64bit(skb, CTA_QOS_FLAGS, qos->flags, CTA_QOS_PAD))
		goto nla_put_failure;

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}

static const struct nla_policy qos_policy[CTA_QOS_MAX + 1] = {
	[CTA_QOS_ORIG_QUEUE]	= { .type = NLA_S16 },
	[CTA_QOS_REPL_QUEUE]	= { .type = NLA_S16 },
	[CTA_QOS_EG_PRIO]	= { .type = NLA_U8 },
	[CTA_QOS_DSCP]		= { .type = NLA_U8 },
};

static void qos_change_actions(struct nf_conn *ct, unsigned long changes)
{
	struct kqoe_ct_info *qos = &ct->bcm_ext.qos;
	int needs_eviction = 0;

	if (test_bit(CTA_QOS_ORIG_QUEUE, &changes) ||
	    test_bit(CTA_QOS_REPL_QUEUE, &changes)) {
		struct blog_qos params = {
			.orig_queue = qos->orig_queue,
			.reply_queue = qos->reply_queue,
		};

		blog_lock();
		blog_notify(SET_QOS, ct, (unsigned long)&params, 0);
		blog_unlock();
	}

	if (test_bit(CTA_QOS_DSCP, &changes) &&
	    !test_bit(CTA_QOS_F_MSCS_BIT, &qos->flags)) {
		set_bit(CTA_QOS_F_DSCP_SET_BIT, &qos->flags);
		needs_eviction = 1;
	}

	if (needs_eviction) {
		unsigned int orig, repl;

		/* evict flow if it's already programmed */
		blog_lock();
		orig = ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL];
		repl = ct->bcm_ext.blog_key[IP_CT_DIR_REPLY];
		if (orig != BLOG_KEY_FC_INVALID || repl != BLOG_KEY_FC_INVALID)
			blog_notify(DESTROY_FLOWTRACK, ct, orig, repl);
		blog_unlock();
	}
}

static int ctnetlink_change_qos(struct nf_conn *ct, struct nlattr *cda[])
{
	const struct nlattr *attr;
	struct nlattr *tb[CTA_QOS_MAX + 1];
	struct kqoe_ct_info *qos = &ct->bcm_ext.qos;
	unsigned long changes = 0;
	int err = 0;

	attr = cda[CTA_BROADCOM_QOS];
	if (!attr)
		return 0;

	err = nla_parse_nested(tb, CTA_QOS_MAX, attr, qos_policy, NULL);
	if (err < 0)
		return err;

	if (tb[CTA_QOS_ORIG_QUEUE]) {
		s16 queue = ntohs(nla_get_s16(tb[CTA_QOS_ORIG_QUEUE]));

		if (qos->orig_queue != queue) {
			qos->orig_queue = queue;
			set_bit(CTA_QOS_ORIG_QUEUE, &changes);
		}
	}
	if (tb[CTA_QOS_REPL_QUEUE]) {
		s16 queue = ntohs(nla_get_s16(tb[CTA_QOS_REPL_QUEUE]));

		if (qos->reply_queue != queue) {
			qos->reply_queue = queue;
			set_bit(CTA_QOS_REPL_QUEUE, &changes);
		}
	}
	if (tb[CTA_QOS_EG_PRIO]) {
		u8 eg_prio = nla_get_u8(tb[CTA_QOS_EG_PRIO]);

		if (qos->eg_prio != eg_prio) {
			qos->eg_prio = eg_prio;
			set_bit(CTA_QOS_EG_PRIO, &changes);
		}
	}
	if (tb[CTA_QOS_DSCP]) {
		u8 dscp = nla_get_u8(tb[CTA_QOS_DSCP]);

		if (qos->dscp != dscp) {
			qos->dscp = dscp;
			set_bit(CTA_QOS_DSCP, &changes);
		}
	}

	if (changes) {
		pr_debug("changes 0x%lX orig %d repl %d eg_prio %d dscp %d\n",
			 changes, qos->orig_queue, qos->reply_queue,
			 qos->eg_prio, qos->dscp);
	}

	/* perform actions for changed conntracks */
	qos_change_actions(ct, changes);

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_BCM_NDI)
static int ctnetlink_size_ndi(const struct nf_conn *ct)
{
	return nla_total_size(sizeof(u_int32_t))	/* CTA_NDI_ID */
	       + nla_total_size(sizeof(u_int64_t))	/* CTA_NDI_FLAGS */
	       ;
}

static int ctnetlink_dump_ndi(struct sk_buff *skb, const struct nf_conn *ct)
{
	struct nlattr *nest_parms;
	const struct ndi_info *ndi = &ct->bcm_ext.ndi;

	nest_parms = nla_nest_start(skb, CTA_BROADCOM_NDI | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	if (ndi->dev) {
		if (nla_put_u32(skb, CTA_NDI_ID, htonl(ndi->dev->id)))
			goto nla_put_failure;
	}

	if (nla_put_be64(skb, CTA_NDI_FLAGS, cpu_to_be64(ndi->flags),
			 CTA_NDI_PAD))
		goto nla_put_failure;

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}
#endif /* IS_ENABLED(CONFIG_BCM_NDI) */

#if IS_ENABLED(CONFIG_BCM_DPI)
static int ctnetlink_size_dpi(const struct nf_conn *ct)
{
	return nla_total_size(sizeof(u_int64_t))	/* CTA_DPI_APP_ID */
	       + 6 * nla_total_size(sizeof(u_int8_t))	/* CTA_DPI_DEV_MAC */
	       + nla_total_size(sizeof(u_int32_t))	/* CTA_DPI_STATUS */
	       ;
}

static int ctnetlink_dump_dpi(struct sk_buff *skb, const struct nf_conn *ct)
{
	struct nlattr *nest_parms;
	const struct dpi_info *dpi = &ct->bcm_ext.dpi;

	nest_parms = nla_nest_start(skb, CTA_BROADCOM_DPI | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	if (dpi->app) {
		if (nla_put_be64(skb, CTA_DPI_APP_ID,
				 cpu_to_be64(dpi->app->app_id), CTA_DPI_PAD))
			goto nla_put_failure;
	}

	if (dpi->dev) {
		if (nla_put(skb, CTA_DPI_DEV_MAC, sizeof(dpi->dev->mac),
			    dpi->dev->mac))
			goto nla_put_failure;
	}

	if (nla_put_be32(skb, CTA_DPI_STATUS, htonl(dpi->flags)))
		goto nla_put_failure;

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}

static const struct nla_policy dpi_policy[CTA_DPI_MAX + 1] = {
	[CTA_DPI_STATUS]	= { .type = NLA_U32 },
};

static int ctnetlink_change_dpi(struct nf_conn *ct, struct nlattr *cda[])
{
	const struct nlattr *attr;
	struct nlattr *tb[CTA_DPI_MAX + 1];
	int err = 0;

	attr = cda[CTA_BROADCOM_DPI];
	if (!attr)
		return 0;

	err = nla_parse_nested(tb, CTA_DPI_MAX, attr, dpi_policy, NULL);
	if (err < 0)
		return err;

	if (tb[CTA_DPI_STATUS]) {
		unsigned long new = ntohl(nla_get_be32(tb[CTA_DPI_STATUS]));
		unsigned long old = ct->bcm_ext.dpi.flags;
		unsigned long changed;

		ct->bcm_ext.dpi.flags = (new & DPI_NL_CHANGE_MASK) |
					(old & ~DPI_NL_CHANGE_MASK);
		changed = old ^ ct->bcm_ext.dpi.flags;

		if (test_bit(DPI_CT_BLOCK_BIT, &changed))
			dpi_block(ct);
	}

	return 0;
}
#endif /* IS_ENABLED(CONFIG_BCM_DPI) */

#if IS_ENABLED(CONFIG_BCM_SGS)
static int ctnetlink_size_sgs(const struct nf_conn *ct)
{
	return nla_total_size(sizeof(u_int64_t)) /* CTA_SGS_SES_KEY */
	       + nla_total_size(sizeof(u_int64_t)) /* CTA_SGS_SES_START */
	       ;
}

static inline int
ctnetlink_dump_sgs(struct sk_buff *skb, const struct nf_conn *ct)
{
	struct nlattr *nest_parms;
	const struct nf_conn_tstamp *tstamp;

	nest_parms = nla_nest_start(skb, CTA_BROADCOM_SGS | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	if (nla_put_be64(skb, CTA_SGS_SES_KEY, cpu_to_be64((u_int64_t)ct),
			 CTA_SGS_SES_PAD)) {
		goto nla_put_failure;
	}

	tstamp = nf_conn_tstamp_find(ct);
	if (tstamp) {
		if (nla_put_be64(skb, CTA_SGS_SES_START,
				 cpu_to_be64((u_int64_t)tstamp->start),
				 CTA_SGS_SES_PAD)) {
			goto nla_put_failure;
		}
	}

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}
#endif /* IS_ENABLED(CONFIG_BCM_SGS) */

struct bcm_nl_helper bcm_nl_helpers[] = {
#if IS_ENABLED(CONFIG_BCM_KQOE)
	{
		.size = ctnetlink_size_qos,
		.dump = ctnetlink_dump_qos,
		.change = ctnetlink_change_qos,
	},
#endif
#if IS_ENABLED(CONFIG_BCM_NDI)
	{
		.size = ctnetlink_size_ndi,
		.dump = ctnetlink_dump_ndi,
	},
#endif
#if IS_ENABLED(CONFIG_BCM_DPI)
	{
		.size = ctnetlink_size_dpi,
		.dump = ctnetlink_dump_dpi,
		.change = ctnetlink_change_dpi,
	},
#endif
#if IS_ENABLED(CONFIG_BCM_SGS)
	{
		.size = ctnetlink_size_sgs,
		.dump = ctnetlink_dump_sgs,
	},
#endif
};

/* netlink ct hooks */
int bcm_ctnetlink_size(const struct nf_conn *ct)
{
	int ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(bcm_nl_helpers); i++) {
		if (bcm_nl_helpers[i].size)
			ret += bcm_nl_helpers[i].size(ct);
	}

	return ret;
}
EXPORT_SYMBOL(bcm_ctnetlink_size);

int bcm_ctnetlink_dump(struct sk_buff *skb, const struct nf_conn *ct,
		       unsigned int events)
{
	struct nlattr *nest_parms;
	unsigned int mask;
	int err = 0;
	int i;

	mask = (1 << IPCT_NEW) | (1 << IPCT_RELATED) | (1 << IPCT_BCM);
	if (!(events & mask))
		return 0;

	nest_parms = nla_nest_start(skb, CTA_BROADCOM | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	for (i = 0; i < ARRAY_SIZE(bcm_nl_helpers); i++) {
		if (!bcm_nl_helpers[i].dump)
			continue;
		err = bcm_nl_helpers[i].dump(skb, ct);
		if (err < 0)
			return err;
	}

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}
EXPORT_SYMBOL(bcm_ctnetlink_dump);

static const struct nla_policy broadcom_policy[CTA_BROADCOM_MAX + 1] = {
	[CTA_BROADCOM_QOS]	= { .type = NLA_NESTED },
	[CTA_BROADCOM_DPI]	= { .type = NLA_NESTED },
};

int bcm_ctnetlink_change(struct nf_conn *ct, const struct nlattr * const cda[])
{
	const struct nlattr * const attr = cda[CTA_BROADCOM];
	struct nlattr *tb[CTA_BROADCOM_MAX + 1];
	int err = 0;
	int i;

	if (!cda[CTA_BROADCOM])
		return 0;

	err = nla_parse_nested(tb, CTA_BROADCOM_MAX, attr, broadcom_policy,
			       NULL);
	if (err < 0)
		return err;

	for (i = 0; i < ARRAY_SIZE(bcm_nl_helpers); i++) {
		if (!bcm_nl_helpers[i].change)
			continue;
		err = bcm_nl_helpers[i].change(ct, tb);
		if (err < 0)
			return err;
	}

	return err;
}
EXPORT_SYMBOL(bcm_ctnetlink_change);
