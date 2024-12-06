/*
*    Copyright (c) 2003-2023 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2023:DUAL/GPL:standard

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
