/*
 *	xt_mark - Netfilter module to match NFMARK value
 *
 *	(C) 1999-2001 Marc Boucher <marc@mbsi.ca>
 *	Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *	Jan Engelhardt <jengelh@medozas.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */

/* Extend from xt_mark.c.
 *  Change targets from non-terminating to terminating.
 *  BRCM, Apr, 22. 2022.
 */


#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/xt_mark.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marc Boucher <marc@mbsi.ca>");
MODULE_DESCRIPTION("Xtables: packet mark operations");
MODULE_ALIAS("ipt_mark");
MODULE_ALIAS("ip6t_mark");
MODULE_ALIAS("ipt_MARK");
MODULE_ALIAS("ip6t_MARK");
MODULE_ALIAS("arpt_MARK");

static unsigned int
mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark_tginfo2 *info = par->targinfo;

	skb->mark = (skb->mark & ~info->mask) ^ info->mark;
	return NF_ACCEPT;
}

static struct xt_target mark_tg_reg __read_mostly = {
	.name           = "MARK-EXTEND",
	.revision       = 2,
	.family         = NFPROTO_UNSPEC,
	.target         = mark_tg,
	.targetsize     = sizeof(struct xt_mark_tginfo2),
	.me             = THIS_MODULE,
};

static int __init mark_mt_init(void)
{
	int ret;

	ret = xt_register_target(&mark_tg_reg);
	if (ret < 0)
		return ret;

	return 0;
}

static void __exit mark_mt_exit(void)
{
	xt_unregister_target(&mark_tg_reg);
}

module_init(mark_mt_init);
module_exit(mark_mt_exit);
