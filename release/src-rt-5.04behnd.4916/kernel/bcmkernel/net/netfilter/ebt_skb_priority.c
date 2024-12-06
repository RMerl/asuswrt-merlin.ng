/*
 *
 *  Copyright (c) 2022  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2022:DUAL/GPL:standard

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
