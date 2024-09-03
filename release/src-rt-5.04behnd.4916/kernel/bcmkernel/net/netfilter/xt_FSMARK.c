/*
*    Copyright (c) 2003-2023 Broadcom
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


#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/bcm_log.h>

#include <linux/netfilter/x_tables.h>
#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include <linux/netfilter/xt_FSMARK_sh.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Xtables: flow stat operations");
MODULE_ALIAS("ipt_FSMARK");
MODULE_ALIAS("ip6t_FSMARK");
MODULE_ALIAS("arpt_FSMARK");

static unsigned int
fsmark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_fsmark_tginfo *info = par->targinfo;

    //DEBUG_FSMARK("SKB=%px id%d flags=%x\n", skb, info->id, info->flags);
    if ((info->flags & XT_FSMARK_VALID_ID) == 0) {
        ERROR_FSMARK("query ID not specified\n");
        return XT_CONTINUE;
    }

	fsmark_update(skb, info->id);

	return XT_CONTINUE;
}

static struct xt_target fsmark_tg_reg __read_mostly = {
	.name           = "FSMARK",
	.revision       = 0,
	.family         = NFPROTO_UNSPEC,
	.target         = fsmark_tg,
	.targetsize     = sizeof(struct xt_fsmark_tginfo),
	.me             = THIS_MODULE,
};

static int __init fsmark_tg_init(void)
{
	int ret;

	ret = xt_register_target(&fsmark_tg_reg);
	if (ret < 0)
		return ret;

	return 0;
}

static void __exit fsmark_tg_exit(void)
{
	xt_unregister_target(&fsmark_tg_reg);
}

module_init(fsmark_tg_init);
module_exit(fsmark_tg_exit);
