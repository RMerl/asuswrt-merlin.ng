/*
*    Copyright (c) 2003-2023 Broadcom
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
