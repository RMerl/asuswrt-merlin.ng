/*
	<:copyright-BRCM:2024:DUAL/GPL:standard

	Copyright (c) 2024 Broadcom
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

/* xt_bcmmark - Netfilter module to match/update skb->bcm_ext.bcmmark value */
/* copied from xt_mark.c to update bcmmark */

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/xt_bcmmark.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Xtables: packet mark operations");
MODULE_ALIAS("ipt_bcmmark");
MODULE_ALIAS("ip6t_bcmmark");
MODULE_ALIAS("ipt_BCMMARK");
MODULE_ALIAS("ip6t_BCMMARK");
MODULE_ALIAS("arpt_BCMMARK");


static unsigned int
mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_bcmmark_tginfo2 *info = par->targinfo;

	skb->bcm_ext.bcmmark = (skb->bcm_ext.bcmmark & ~info->mask) ^ info->mark;
	return XT_CONTINUE;
}

static bool
mark_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_bcmmark_mtinfo1 *info = par->matchinfo;

	return ((skb->bcm_ext.bcmmark & info->mask) == info->mark) ^ info->invert;
}

static struct xt_target mark_tg_reg __read_mostly = {
	.name           = "BCMMARK",
	.revision       = 2,
	.family         = NFPROTO_UNSPEC,
	.target         = mark_tg,
	.targetsize     = sizeof(struct xt_bcmmark_tginfo2),
	.me             = THIS_MODULE,
};

static struct xt_match mark_mt_reg __read_mostly = {
	.name           = "bcmmark",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.match          = mark_mt,
	.matchsize      = sizeof(struct xt_bcmmark_mtinfo1),
	.me             = THIS_MODULE,
};

static int __init bcm_mark_mt_init(void)
{
	int ret;

	ret = xt_register_target(&mark_tg_reg);
	if (ret < 0)
		return ret;
	ret = xt_register_match(&mark_mt_reg);
	if (ret < 0) {
		xt_unregister_target(&mark_tg_reg);
		return ret;
	}
	return 0;
}

static void __exit bcm_mark_mt_exit(void)
{
	xt_unregister_match(&mark_mt_reg);
	xt_unregister_target(&mark_tg_reg);
}

module_init(bcm_mark_mt_init);
module_exit(bcm_mark_mt_exit);
