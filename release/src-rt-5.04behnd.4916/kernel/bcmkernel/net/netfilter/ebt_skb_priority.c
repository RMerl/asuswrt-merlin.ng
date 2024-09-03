/*
 *
 *  Copyright (c) 2022  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2022:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/*
 *  ebt_skb_priority
 *
 */

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_skb_priority.h>

static unsigned int
ebt_skb_priority_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct ebt_skb_prioity_info *info = par->targinfo;

	skb->priority = info->prio;
	return info->target;
}

static int ebt_skb_priority_tg_check(const struct xt_tgchk_param *par)
{
	return 0;
}

static struct xt_target ebt_skb_priority_tg_reg __read_mostly = {
	.name		= "skbpriority",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_skb_priority_tg,
	.checkentry	= ebt_skb_priority_tg_check,
	.targetsize	= sizeof(struct ebt_skb_prioity_info),
	.me		= THIS_MODULE,
};

static int __init ebt_skb_priority_init(void)
{
	return xt_register_target(&ebt_skb_priority_tg_reg);
}

static void __exit ebt_skb_priority_fini(void)
{
	xt_unregister_target(&ebt_skb_priority_tg_reg);
}

module_init(ebt_skb_priority_init);
module_exit(ebt_skb_priority_fini);
MODULE_DESCRIPTION("Ebtables: Set SKB priority value");
MODULE_LICENSE("GPL");
