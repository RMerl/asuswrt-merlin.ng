/*
*    Copyright (c) 2003-2023 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2023:DUAL/GPL:standard

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
 *  ebt_fsmark
 */
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebt_fsmark_t.h>
#include <linux/netfilter_bridge/ebtables.h>
#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include <linux/netfilter/xt_FSMARK_sh.h>

static unsigned int
ebt_fsmark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct ebt_fsmark_t_info *info = par->targinfo;
	
	if (fsmark_update(skb, info->id) != 0)
	    return EBT_CONTINUE;

	return info->target | ~EBT_VERDICT_BITS;
}

static struct xt_target ebt_fsmark_tg_reg __read_mostly = {
	.name		= "fsmark",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_fsmark_tg,
	.targetsize	= sizeof(struct ebt_fsmark_t_info),
	.me		= THIS_MODULE,
};

static int __init ebt_fsmark_init(void)
{
	return xt_register_target(&ebt_fsmark_tg_reg);
}

static void __exit ebt_fsmark_fini(void)
{
	xt_unregister_target(&ebt_fsmark_tg_reg);
}

module_init(ebt_fsmark_init);
module_exit(ebt_fsmark_fini);
MODULE_DESCRIPTION("Ebtables: FSMARK target");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
